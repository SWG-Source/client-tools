// ======================================================================
//
// ExportStaticMesh.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

//precompiled header includes
#include "FirstMayaExporter.h"

//module includes
#include "ExportStaticMesh.h"

//engine shared includes
#include "sharedFile/Iff.h"
#include "sharedDebug/PerformanceTimer.h"

//local MayaExporter includes
#include "AlienbrainImporter.h"
#if !NO_DATABASE
#include "DatabaseImporter.h"
#endif
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "ExportManager.h"
#include "MayaHierarchy.h"
#include "MayaUtility.h"
#include "MayaMeshReader.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "StaticMeshBuilder.h"

//maya includes
#include "maya/MAnimControl.h"
#include "maya/MArgList.h"
#include "maya/MCommandResult.h"
#include "maya/MFileIo.h"
#include "maya/MGlobal.h"
#include "maya/MIntArray.h"
#include "maya/MSelectionList.h"
#include "maya/MTime.h"

// STL/system includes
#include <stdio.h>
#include <stdarg.h>
#include <map>
#include <sstream>

// ======================================================================

static Messenger *messenger;

// ======================================================================

namespace ExportStaticMeshNamespace
{
	const int c_seondsPerMinute = 60;
	const int c_secondsPerHour = 3600;
}

using namespace ExportStaticMeshNamespace;

// ======================================================================

void *ExportStaticMesh::creator ()
{
	return new ExportStaticMesh;
}

// ----------------------------------------------------------------------

void ExportStaticMesh::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ExportStaticMesh::remove(void)
{
	messenger = 0;
}

// ----------------------------------------------------------------------

bool ExportStaticMesh::processArguments(
	const MArgList &args,
	MDagPath *targetDagPath,
	bool &interactive,
	bool &commitToSourceControl,
	bool &createNewChangelist,
	bool &lock,
	bool &unlock,
	bool &showViewerAfterExport,
	MString &branch,
	bool &fixPobCrc
	)
{

	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));

	//-- handle each argument
	bool haveNode            = false;
	bool isInteractive       = false;
	interactive              = false;
	commitToSourceControl    = false;
	createNewChangelist      = false;
	lock                     = false;
	unlock                   = false;
	showViewerAfterExport    = false;
	bool haveBranch          = false;

	// always non-silent unless the silent arg is present
	messenger->endSilentExport();

	// NOTE: interactive or autoexport argument needs to come first

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = args.asString(argIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get arg [%u] as string\n", argIndex));

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_interactiveArgName)
		{
			MESSENGER_REJECT(argIndex != 0, ("%s must be first argument", ExportArgs::cs_interactiveArgName));
			isInteractive = true;
			interactive   = true;
		}
		else if (argName == ExportArgs::cs_showViewerAfterExport)
		{
			showViewerAfterExport = true;
		}
		else if (argName == ExportArgs::cs_submitArgName)
		{
			commitToSourceControl = true;
		}
		else if (argName == ExportArgs::cs_lockArgName)
		{
			lock = true;
		}
		else if (argName == ExportArgs::cs_unlockArgName)
		{
			unlock = true;
		}
		else if (argName == ExportArgs::cs_nodeArgName)
		{
			if (!isInteractive)
			{
				//--handle node argument
				MESSENGER_REJECT(haveNode, ("node argument specified multiple times\n"));

				MString nodeName = args.asString(argIndex + 1, &status);
				MESSENGER_REJECT(!status, ("failed to get node name argument\n"));

				// search for node in Maya scene
				MSelectionList  nodeList;
				status = MGlobal::getSelectionListByName(nodeName, nodeList);
				MESSENGER_REJECT(!status, ("MGlobal::getSelectionListByName failure"));

				MESSENGER_REJECT(nodeList.length() < 1, ("no scene nodes match specified export node [%s]\n", nodeName.asChar()));
				MESSENGER_REJECT(nodeList.length() > 1, ("multiple nodes match specified export node [%s]\n", nodeName.asChar()));

				status = nodeList.getDagPath(0, *targetDagPath);
				MESSENGER_REJECT(!status, ("failed to get dag path for export node [%s]\n", nodeName.asChar()));

				// fixup argIndex
				++argIndex;
				haveNode = true;
			}
		}
		else if (argName == ExportArgs::cs_silentArgName)
		{
			// silent mode - disable message box feedback (logs messages instead)
			messenger->startSilentExport();
		}
		else if (argName == ExportArgs::cs_branchArgName)
		{
			//-- handle branch argument
			MESSENGER_REJECT(haveBranch, ("branch specified multiple times\n"));

			branch = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get branch argument\n"));

			// fixup argIndex
			++argIndex;
			haveBranch = true;
		}
		else if (argName == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
		}
		else if (argName == ExportArgs::cs_fixPobCrc)
		{
			fixPobCrc = true;
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s]\n", argName.asChar()));
			return false;
		}
	}

	//-- handle interactive node selection
	if (isInteractive)
	{
		MSelectionList nodeList;
		status = MGlobal::getActiveSelectionList(nodeList);
		MESSENGER_REJECT(!status,("failed to get active selection list\n"))

		// we only support export of one skeleton template into a skeleton template file
		MESSENGER_REJECT(nodeList.length() != 1, ("must have exactly one node specified, currently [%u]\n", nodeList.length()));

		status = nodeList.getDagPath(0, *targetDagPath);
		MESSENGER_REJECT(!status, ("failed to get dag path for selected node\n"));

		haveNode = true;
	}
	MESSENGER_REJECT(commitToSourceControl && !haveBranch, ("no branch, i.e. \"-branch <branchname>\" was specified\n"));
	MESSENGER_REJECT(!isInteractive && !haveNode, ("neither joint node (-node) nor (-interactive) was specified\n"));
	return MStatus();
}

// ----------------------------------------------------------------------

MStatus ExportStaticMesh::doIt(const MArgList &args)
{
	PerformanceTimer exportTimer;
	exportTimer.start();

	messenger->clearWarningsAndErrors();

	MESSENGER_INDENT;

	MStatus status;
	MDagPath  targetDagPath;

	MString arg;
	if(args.length())
		IGNORE_RETURN(args.get(0, arg));

	//-- 
	const char *const     shaderTemplateReferenceDir = SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX);
	const char *const     appearanceWriteDir         = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);
	const char *const     shaderTemplateWriteDir     = SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX);
	const char *const     effectReferenceDir         = SetDirectoryCommand::getDirectoryString(EFFECT_REFERENCE_DIR_INDEX);
	const char *const     textureReferenceDir        = SetDirectoryCommand::getDirectoryString(TEXTURE_REFERENCE_DIR_INDEX);
	const char *const     textureWriteDir            = SetDirectoryCommand::getDirectoryString(TEXTURE_WRITE_DIR_INDEX);
	const char *const     author                     = SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX);

	bool commitToSourceControl = false;
	bool createNewChangelist   = false;
	bool interactive           = false;
	bool lock                  = false;
	bool unlock                = false;
	bool showViewerAfterExport = false;
	bool fixPobCrc             = false;
	MString branch;

	const bool processSuccess = processArguments(
		args, 
		&targetDagPath, 
		interactive, 
		commitToSourceControl, 
		createNewChangelist, 
		lock, 
		unlock, 
		showViewerAfterExport,
		branch,
		fixPobCrc
	);
	MESSENGER_REJECT_STATUS(!processSuccess, ("argument processing failed\n"));

	//-- find out what is selected
	MSelectionList        transformList;
	IGNORE_RETURN(transformList.add(targetDagPath));

	//-- install the exporter log
	ExporterLog::install (messenger);
	ExporterLog::setAuthor (author);

	//get, store the base directory
	std::string baseDir = textureWriteDir;
	std::string::size_type pos = baseDir.find_last_of("texture");
	FATAL(pos == static_cast<unsigned int>(std::string::npos), ("malformed filename in ExportStaticMesh::doIt"));
	baseDir = baseDir.substr(0, pos-strlen("texture"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);

	ExporterLog::setSourceFilename (MFileIO::currentFile().asChar());
	ExporterLog::setMayaCommand("exportStaticMesh");

	//-- setup the hierarchy
	MayaHierarchy hierarchy (messenger);

	hierarchy.setAppearanceWriteDir (appearanceWriteDir);
	hierarchy.setShaderTemplateWriteDir (shaderTemplateWriteDir);
	hierarchy.setShaderTemplateReferenceDir (shaderTemplateReferenceDir);
	hierarchy.setEffectReferenceDir (effectReferenceDir);
	hierarchy.setTextureReferenceDir (textureReferenceDir);
	hierarchy.setTextureWriteDir (textureWriteDir);

	//do the actual export to disk (the slow part)
	MESSENGER_REJECT_STATUS(!hierarchy.build (transformList), ("failed to build hierarchy\n"));

	if (fixPobCrc)
	{
		std::string const pobFileName =
			PerforceImporter::findFileOnDisk("//depot/swg/" + std::string(branch.asChar()) + "/.../" + hierarchy.getBaseName() + ".pob");

		MESSENGER_REJECT_STATUS(pobFileName.empty(), ("ExportStaticMesh::doIt() - Failed to find POB file (%s) in perforce.\n", pobFileName.c_str()));

		// revert any local changes first since Fix POB CRC is intended to preserve the POB CRC from the last file submitted in perforce
		IGNORE_RETURN(PerforceImporter::revertFile(pobFileName));

		hierarchy.setPobFileName(pobFileName);
	}

	//load the log file
	MString nodeName = hierarchy.getBaseName();
	std::string shortLogFilename = nodeName.asChar();
	shortLogFilename += ".log";
	char buffer[1024];
	MObject selectedNode;
	status = transformList.getDependNode(0, selectedNode);
	const bool gnnResult = MayaUtility::getNodeName(selectedNode, buffer, 1024);
	MESSENGER_REJECT_STATUS(!gnnResult, ("getNodeName() failed\n"));
	std::string reexportArguments = ExportArgs::cs_nodeArgName.asChar();
	reexportArguments += " ";
	reexportArguments += buffer;
	if(commitToSourceControl)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_branchArgName.asChar();
		reexportArguments += " ";
		reexportArguments += branch.asChar();
	}
	ExporterLog::setMayaExportOptions(reexportArguments);
	IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));
	std::string newLogFilename = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	newLogFilename += shortLogFilename;

	//check for duplicate names (if there are dupes, maya returns a '|' as part of the node name)
	if(shortLogFilename.find("|") != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
	{
		std::string errorMsg = "duplicate node name ";
		errorMsg += shortLogFilename;
		errorMsg += " found, correct and re-export";
		MESSENGER_LOG_ERROR((errorMsg.c_str()));
		if (interactive)
		{
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK);
		}
	}

	hierarchy.dump ();
	bool writeSuccess = hierarchy.write (true);
	if (!writeSuccess)
	{
		if (interactive)
			MESSENGER_MESSAGE_BOX(NULL, "Local export failed, see output window", "Error!", MB_OK);
	}

	//only submit to source control if they want to and if the local export succeeded
	if(writeSuccess && (commitToSourceControl || lock || unlock))
	{
		bool exportToPerforceSucceeded = false;

// JU_TODO: alienbrain def out
#if 0
		if (!AlienbrainImporter::connectToServer())
		{
			MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
			return MStatus::kFailure;
		}

		bool result = AlienbrainImporter::preImport(hierarchy.getBaseName(), interactive); 
		//now that we have all the custom data, write the log file

		if(!ExportManager::validateTextureList(interactive))
		{
			messenger->printWarningsAndErrors();
			return MS::kSuccess;
		}

		IGNORE_RETURN(ExporterLog::writeStaticMesh (newLogFilename.c_str(), interactive));

		if (result)
		{
			if(!lock && !unlock)
				AlienbrainImporter::importLogFile(); 
			AlienbrainImporter::storeFileProperties();

			//import asset data into Asset Database
#if !NO_DATABASE
			IGNORE_RETURN(DatabaseImporter::startSession());
			bool gotAppearance = DatabaseImporter::selectAppearance(interactive, nodeName.asChar());
			if(!gotAppearance)
			{
				MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
				IGNORE_RETURN(DatabaseImporter::endSession());
				messenger->printWarningsAndErrors();
				return MS::kSuccess;
			}
#endif

			//import all the destination files into Perforce
			bool branchSet = PerforceImporter::setBranch(branch.asChar());
			if(branchSet)
			{
				exportToPerforceSucceeded = PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock);
			}
			else
			{
				MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch.asChar()));
			}
			PerforceImporter::reset();
			
			if(exportToPerforceSucceeded)
			{
#if !NO_DATABASE
				if(!DatabaseImporter::importStaticMeshData(interactive, lock, unlock) && interactive)
					MESSENGER_MESSAGE_BOX(NULL, "AssetDatabase import failed, see output window", "Error", MB_OK);
#endif
			}
#if !NO_DATABASE
			IGNORE_RETURN(DatabaseImporter::endSession());
#endif
		}
		IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());
#else
		IGNORE_RETURN(ExporterLog::writeStaticMesh (newLogFilename.c_str(), interactive));

#if !NO_DATABASE
		//import asset data into Asset Database
		IGNORE_RETURN(DatabaseImporter::startSession());
		bool gotAppearance = DatabaseImporter::selectAppearance(interactive, nodeName.asChar());
		if(!gotAppearance)
		{
			MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
			IGNORE_RETURN(DatabaseImporter::endSession());
			messenger->printWarningsAndErrors();
			return MS::kSuccess;
		}
#endif

		//import all the destination files into Perforce
		bool branchSet = PerforceImporter::setBranch(branch.asChar());
		if(branchSet)
		{
			exportToPerforceSucceeded = PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock);
		}
		else
		{
			MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch.asChar()));
		}
		PerforceImporter::reset();
			
		if(exportToPerforceSucceeded)
		{
#if !NO_DATABASE
			if(!DatabaseImporter::importStaticMeshData(interactive, lock, unlock) && interactive)
				MESSENGER_MESSAGE_BOX(NULL, "AssetDatabase import failed, see output window", "Error", MB_OK);
#endif
		}
#if !NO_DATABASE
		IGNORE_RETURN(DatabaseImporter::endSession());
#endif
#endif
// JU_TODO: end alienbrain def out
	}
	else
	{
		ExporterLog::setAssetGroup("Local export, N/A");
		ExporterLog::setAssetName("Local export, N/A");
		IGNORE_RETURN(ExporterLog::writeStaticMesh (newLogFilename.c_str(), interactive));
	}
	ExporterLog::remove();

	messenger->printWarningsAndErrors();

	exportTimer.stop();
	const int exportTime = static_cast<int>(exportTimer.getElapsedTime());
	const int hours = exportTime / c_secondsPerHour;
	const int minutes = (exportTime - (hours * c_secondsPerHour)) / c_seondsPerMinute;
	const int seconds = exportTime - (hours * c_secondsPerHour) - (minutes * c_seondsPerMinute);
	MESSENGER_LOG(("Total export time: %2ih %2im %2is\n", hours, minutes, seconds));

	MESSENGER_LOG(("MemoryManager %lu/%lu=bytes %d/%d=allocs\n", MemoryManager::getCurrentNumberOfBytesAllocated(), MemoryManager::getMaximumNumberOfBytesAllocated(), MemoryManager::getCurrentNumberOfAllocations(), MemoryManager::getMaximumNumberOfAllocations()));

    std::stringstream text;
    text << "Export Complete: " << nodeName.asChar() << std::endl;
    messenger->getWarningAndErrorText(text);
    text << std::ends;

	MESSENGER_MESSAGE_BOX(NULL,text.str().c_str(),"Export",MB_OK);
	
	if (showViewerAfterExport)
	{
		std::stringstream assetPathAndFilename;
		assetPathAndFilename << SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX)  << nodeName.asChar() << ".apt";
		ExportManager::LaunchViewer(assetPathAndFilename.str());

	}

	return MS::kSuccess;
}

// ======================================================================
