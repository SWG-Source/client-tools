// ======================================================================
//
// ExportSkeletalAppearanceTemplate.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "ExportSkeletalAppearanceTemplate.h"

#include "AlienbrainImporter.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#if !NO_DATABASE
#include "DatabaseImporter.h"
#endif
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "ExportManager.h"
#include "ExportSkeletalMeshGenerator.h"
#include "MayaMeshWeighting.h"
#include "MayaUtility.h"
#include "maya/MArgList.h"
#include "maya/MDagPath.h"
#include "maya/MFileIO.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MGlobal.h"
#include "maya/MSelectionList.h"
#include "maya/MStatus.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"

#include <ctype.h>
#include <set>
#include <string>
#include <vector>
#include <sstream>

// ======================================================================

Messenger   *ExportSkeletalAppearanceTemplate::messenger;

std::string  ExportSkeletalAppearanceTemplate::ms_dialogSatShortName;
std::string  ExportSkeletalAppearanceTemplate::ms_dialogAsgTemplateReferenceName = "appearance/animationgraph/";
std::string  ExportSkeletalAppearanceTemplate::ms_dialogLastSatPathSelected;

// ======================================================================

const char         cs_attachmentSeparator = ':';
const char         cs_fileExtensionCharacter = '.';
const std::string  cs_directorySeparators("\\/");
const std::string  cs_shapeSuffix("Shape");

const int          cs_iffSize = 64 * 1024;
const int          cs_skeletonTemplateNameComponentIndex = 1;

// ======================================================================

void ExportSkeletalAppearanceTemplate::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ExportSkeletalAppearanceTemplate::remove(void)
{
	messenger = NULL;
}

// ----------------------------------------------------------------------

void *ExportSkeletalAppearanceTemplate::creator(void)
{
	NOT_NULL(messenger);
	return new ExportSkeletalAppearanceTemplate();
}

// ======================================================================
// public member functions
// ======================================================================

MStatus ExportSkeletalAppearanceTemplate::doIt(const MArgList &argList)
{
	messenger->clearWarningsAndErrors();

	//-- write out instructions if no args are provided
	if (argList.length() < 1)
	{
		MESSENGER_LOG_ERROR(("Not enough args.  Use one of the following forms:"));
		MESSENGER_LOG_ERROR(("  exportSatFile -interactive <select skeletal-bound mesh(es)> -branch <branchname>"));
		MESSENGER_LOG_ERROR(("  exportSatFile -outputfile <.sat short file name>  -branch <branchname> [-skeleton <.skt reference path>[:<attachment bone name>] ...]"));
		MESSENGER_LOG_ERROR(("               [-mesh <.mgn reference path> ...]  -branch <branchname> [-asg <.asg reference path>]"));

		return MStatus::kFailure;
	}

	ExporterLog::setSourceFilename (MFileIO::currentFile().asChar());

	//-- create the SAT file
	bool success = false;

	bool interactive = false;
	bool commitToSourceControl = false;
	bool lock = false;
	bool unlock = false;
	bool showViewerAfterExport = false;
	bool createNewChangelist = false;
	std::string branch;
	MStatus status;
	const unsigned int argCount = argList.length();
	for (unsigned int argIndex = 0; argIndex < argCount; ++argIndex)
	{
		//-- get the argument, convert to lower case
		MString arg = argList.asString(argIndex);
		IGNORE_RETURN(arg.toLowerCase());

		if (arg == ExportArgs::cs_interactiveArgName)
		{
			interactive = true;
		}
		else if (arg == ExportArgs::cs_submitArgName)
		{
			commitToSourceControl = true;
		}
		else if (arg == ExportArgs::cs_lockArgName)
		{
			lock = true;
		}
		else if (arg == ExportArgs::cs_unlockArgName)
		{
			unlock = true;
		}
		else if (arg == ExportArgs::cs_branchArgName)
		{
			branch = argList.asString(argIndex + 1, &status).asChar();
			// fixup argIndex
			++argIndex;
		}
		else if (arg == ExportArgs::cs_showViewerAfterExport)
		{
			showViewerAfterExport = true;
		}
		else if (arg == ExportArgs::cs_outputFileNameArgName)
		{
			// this is a valid arg in non-interactive mode
			// fixup argIndex
			++argIndex;
		}
		else if (arg == ExportArgs::cs_meshGeneratorArgName)
		{
			// this is a valid arg in non-interactive mode
			// fixup argIndex
			++argIndex;
		}
		else if (arg == ExportArgs::cs_skeletonArgName)
		{
			// this is a valid arg in non-interactive mode
			// fixup argIndex
			++argIndex;
		}
		else if (arg == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s].\n", arg.asChar()));
			return MStatus::kFailure;
		}
	}


/*
	//check for either "-interactive", "-interactive -submit", "-interactive -lock", or "-interactive -unlock"
	if (((args.length() >= 1) && (args.length() <= 4)) && (args.asString(0) == ExportArgs::cs_interactiveArgName))
	{	
		if (args.asString(1) == ExportArgs::cs_submitArgName)
			success = createInteractively(true, true, false, false);
		else if (args.asString(1) == ExportArgs::cs_lockArgName)
			success = createInteractively(true, true, true, false);
		else if (args.asString(1) == ExportArgs::cs_unlockArgName)
			success = createInteractively(true, true, false, true);
		else 
 			success = createInteractively(false, true, false, false);
	}
	else
		success = createFromArgList(args);
*/

	if(interactive)
    {
 		success = createInteractively(
			commitToSourceControl, 
			createNewChangelist, 
			lock ,
			unlock, 
			showViewerAfterExport, 
			branch);
    }
	else
	{
		success = createFromArgList(argList);
	}

	messenger->printWarningsAndErrors();

	std::stringstream text;
	text << "Skeleton Appearance Template Export Complete." << std::endl;
	messenger->getWarningAndErrorText(text);
	text << std::ends;

	MESSENGER_MESSAGE_BOX(NULL,text.str().c_str(),"Export",MB_OK);

	//-- return success status
	if (success)
		return MStatus::kSuccess;
	else
		return MStatus::kFailure;
}

// ======================================================================
// private static member functions
// ======================================================================

bool ExportSkeletalAppearanceTemplate::createFromArgList(const MArgList &argList)
{
	StringVector  skeletonReferenceNames;
	StringVector  meshGeneratorReferenceNames;
	std::string   animationStateGraphReferenceName;
	std::string   outputShortName;
	bool          commitToSourceControl = false;
	bool          createNewChangelist   = false;
	bool          lock                  = false;
	bool          unlock                = false;
	std::string   branch;
	bool          haveBranch            = false;
	MStatus       status;

	ExporterLog::install (messenger);
	ExporterLog::setSourceFilename (MFileIO::currentFile().asChar());
	ExporterLog::setMayaCommand("exportSatFile");
	MString appearanceWriteDirectory = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);

	//get, store the base directory
	std::string baseDir = appearanceWriteDirectory.asChar();
	std::string::size_type pos = baseDir.find_last_of("appearance");
	FATAL(pos == static_cast<unsigned int>(std::string::npos), ("malformed filename in ExportSkeletalAppearanceTemplate::createInteractively"));
	baseDir = baseDir.substr(0, pos-strlen("appearance"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);

	skeletonReferenceNames.reserve(16);
	meshGeneratorReferenceNames.reserve(16);

	//-- set arguments from command line
	const unsigned int argCount = argList.length();
	for (unsigned int argIndex = 0; argIndex < argCount; ++argIndex)
	{
		//-- get the argument, convert to lower case
		MString arg = argList.asString(argIndex);
		IGNORE_RETURN(arg.toLowerCase());

		//-- assign values based on argument
		if (arg == ExportArgs::cs_animationStateGraphArgName)
		{
			//-- Ignore this argument, -asg does nothing now.  Skip the following argument, which is the argument to the -asg command.
			++argIndex;
		}
		else if (arg == ExportArgs::cs_interactiveArgName)
		{
			// nothing to do.
		}
		else if (arg == ExportArgs::cs_submitArgName)
		{
			commitToSourceControl = true;
		}
		else if (arg == ExportArgs::cs_lockArgName)
		{
			lock = true;
		}
		else if (arg == ExportArgs::cs_unlockArgName)
		{
			unlock = true;
		}
		else if (arg == ExportArgs::cs_meshGeneratorArgName)
		{
			++argIndex;
			meshGeneratorReferenceNames.push_back(argList.asString(argIndex).asChar());
		}
		else if (arg == ExportArgs::cs_outputFileNameArgName)
		{
			// make sure arg is specified only once
			MESSENGER_REJECT_STATUS(!outputShortName.empty(), ("[%s] specified multiple times.\n", ExportArgs::cs_outputFileNameArgName.asChar()));

			++argIndex;
			IGNORE_RETURN(outputShortName.assign(argList.asString(argIndex).asChar()));
		}
		else if (arg == ExportArgs::cs_skeletonArgName)
		{
			++argIndex;
			skeletonReferenceNames.push_back(argList.asString(argIndex).asChar());
		}
		else if (arg == ExportArgs::cs_branchArgName)
		{
			//-- handle branch argument
			MESSENGER_REJECT(haveBranch, ("branch specified multiple times\n"));

			branch = argList.asString(argIndex + 1, &status).asChar();
			MESSENGER_REJECT(!status, ("failed to get branch argument\n"));

			// fixup argIndex
			++argIndex;
			haveBranch = true;
		}
		else if (arg == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
		}
		else if (arg == ExportArgs::cs_showViewerAfterExport)
		{
			// don't show viewer on re-exports - ignore this argument
		}
		else
		{
			MESSENGER_LOG_ERROR(("unknown argument [%s].\n", arg.asChar()));
			return MStatus::kFailure;
		}
	}

	MESSENGER_REJECT(commitToSourceControl && !haveBranch, ("no branch, i.e. \"-branch <branchname>\" was specified\n"));

	//build export options, to be stored in the log file
	//outputfile
	std::string reexportArguments;
	reexportArguments += ExportArgs::cs_outputFileNameArgName.asChar();
	reexportArguments += " ";
	reexportArguments += outputShortName;

	//skeleton
	for(StringVector::iterator i = skeletonReferenceNames.begin(); i != skeletonReferenceNames.end(); ++i)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_skeletonArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += *i + "\"";

		//TODO handle : issues (attachment bones)
	}
	//mesh
	for(StringVector::iterator i2 = meshGeneratorReferenceNames.begin(); i2 != meshGeneratorReferenceNames.end(); ++i2)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_meshGeneratorArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += *i2 + "\"";
	}

	// Handle asg argument.
	if (!animationStateGraphReferenceName.empty())
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_animationStateGraphArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += animationStateGraphReferenceName + "\"";
	}

	if(commitToSourceControl)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_branchArgName.asChar();
		reexportArguments += " ";
		reexportArguments += branch;
	}

	//set export options
	ExporterLog::setMayaExportOptions(reexportArguments);

	const std::string nodeName = outputShortName;
	MESSENGER_LOG(("ARGS: fetched selected DAG node [%s]\n", nodeName.c_str()));

	const std::string shortLogFilename = nodeName + ".log";
	IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));
	std::string newLogFilename = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	newLogFilename            += shortLogFilename;

	ExporterLog::setLogFilename(newLogFilename);

	//-- ensure required components are present
	MESSENGER_REJECT_STATUS(outputShortName.empty(), ("caller must provide output filename with [%s] <filename>.\n", ExportArgs::cs_outputFileNameArgName.asChar()));
	MESSENGER_REJECT_STATUS(meshGeneratorReferenceNames.empty() && skeletonReferenceNames.empty(), ("caller must specify at least one skeleton or mesh.\n"));

	//-- create the SAT file
	if(commitToSourceControl || lock || unlock)
	{
		bool exportToPerforceSucceeded = false;

// JU_TODO: alienbrain def out
#if 0
		if (!AlienbrainImporter::connectToServer())
		{
			MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
			return MStatus::kFailure;
		}

		bool result = AlienbrainImporter::preImport(outputShortName.c_str(), false);

		if(!ExportManager::validateTextureList(false))
		{
			messenger->printWarningsAndErrors();
			return MS::kSuccess;
		}

		//-- create the sat file
		const bool csfResult = createSatFile(outputShortName, skeletonReferenceNames, meshGeneratorReferenceNames, animationStateGraphReferenceName);
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		if (result)
		{
			if(!lock && !unlock)
				AlienbrainImporter::importLogFile();
			AlienbrainImporter::storeFileProperties();

			IGNORE_RETURN(DatabaseImporter::startSession());
			bool gotAppearance = DatabaseImporter::selectAppearance(false, nodeName.c_str());
			IGNORE_RETURN(DatabaseImporter::endSession());
			if(!gotAppearance)
			{
				MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
				return MS::kSuccess;
			}
			
			bool branchSet = PerforceImporter::setBranch(branch);
			if(branchSet)
			{
				exportToPerforceSucceeded = PerforceImporter::importCommon(false, createNewChangelist, lock, unlock);
			}
			else
			{
				MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch));
			}

			PerforceImporter::reset();

			if(!exportToPerforceSucceeded)
			{
				MESSENGER_LOG_ERROR(("Couldn't copy SAT to perforce location on local disk, aborting.\n"));
				return MS::kSuccess;
			}
		}
		IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());
#else
		
		//-- create the sat file
		const bool csfResult = createSatFile(outputShortName, skeletonReferenceNames, meshGeneratorReferenceNames, animationStateGraphReferenceName);
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		
#if !NO_DATABASE
		IGNORE_RETURN(DatabaseImporter::startSession());
		bool gotAppearance = DatabaseImporter::selectAppearance(false, nodeName.c_str());
		IGNORE_RETURN(DatabaseImporter::endSession());
		if(!gotAppearance)
		{
			MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
			return MS::kSuccess;
		}
#endif
		
		bool branchSet = PerforceImporter::setBranch(branch);
		if(branchSet)
		{
			exportToPerforceSucceeded = PerforceImporter::importCommon(false, createNewChangelist, lock, unlock);
		}
		else
		{
			MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch));
		}

		PerforceImporter::reset();

		if(!exportToPerforceSucceeded)
		{
			MESSENGER_LOG_ERROR(("Couldn't copy SAT to perforce location on local disk, aborting.\n"));
			return MS::kSuccess;
		}
#endif
// JU_TODO: end alienbrain def out
	}
	else
	{
		//-- create the sat file
		const bool csfResult = createSatFile(outputShortName, skeletonReferenceNames, meshGeneratorReferenceNames, animationStateGraphReferenceName);
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		ExporterLog::setAssetGroup("Local export, N/A");
		ExporterLog::setAssetName("Local export, N/A");
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
	}

	//-- Export MeshGeneratorTemplate instances given MeshGeneratorTemplate reference names.
	const bool emgtSuccess = exportMeshGeneratorTemplates(meshGeneratorReferenceNames, commitToSourceControl, createNewChangelist, false, lock, unlock, branch);
	MESSENGER_REJECT(!emgtSuccess, ("failed to export MeshGeneratorTemplate data associated with this SAT."));

	ExporterLog::remove();

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalAppearanceTemplate::createInteractively(bool commitToSourceControl, bool createNewChangelist, bool lock, bool unlock, bool showViewerAfterExport, const std::string & branch)
{
	ExporterLog::install (messenger);

	ExporterLog::setSourceFilename (MFileIO::currentFile().asChar());
	ExporterLog::setMayaCommand("exportSatFile");
	
	MString appearanceWriteDirectory = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);

	//get, store the base directory
	std::string baseDir = appearanceWriteDirectory.asChar();
	std::string::size_type pos = baseDir.find_last_of("appearance");
	FATAL(pos == static_cast<unsigned int>(std::string::npos), ("malformed filename in ExportSkeletalAppearanceTemplate::createInteractively"));
	baseDir = baseDir.substr(0, pos-strlen("appearance"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);

	//-- retrieve dag paths to the mesh shapes
	MStatus        status;
	DagPathVector  meshShapeDagPaths;

	{
		// get active selections
		MSelectionList  selectionList;

		status = MGlobal::getActiveSelectionList(selectionList);
		STATUS_REJECT(status, "getActiveSelectionList");

		// Get dag path for selected nodes.
		const bool gmsResult = MayaUtility::getSelectionListMeshShapes(selectionList, meshShapeDagPaths);
		MESSENGER_REJECT(!gmsResult, ("getSelectionListMeshShapes() failed.\n"));
	}

	//-- retrieve mesh names from mesh shape DagPaths
	StringVector  meshNames;
	const bool    gmnResult = getMeshGeneratorReferenceNames(meshShapeDagPaths, meshNames);
	MESSENGER_REJECT(!gmnResult, ("getMeshNames() failed.\n"));

	//-- retrieve skeleton segments referenced by the meshes
	StringVector  skeletonTemplateNames;
	const bool    grsResult = getReferencedSkeletonSegments(meshShapeDagPaths, skeletonTemplateNames);
	MESSENGER_REJECT(!grsResult, ("getReferencedSkeletonSegments() failed.\n"));

	//get the first selected node for asset database matching
	MDagPath targetDagPath;
	MSelectionList nodeList;
	status = MGlobal::getActiveSelectionList(nodeList);
	MESSENGER_REJECT(!status,("failed to get active selection list\n"))
	status = nodeList.getDagPath(0, targetDagPath);
	MESSENGER_REJECT(!status, ("failed to get dag path for selected node\n"));
	MString nodeName = targetDagPath.partialPathName();
	MESSENGER_LOG(("ARGS: fetched selected DAG node [%s]\n", nodeName.asChar()));

	const std::string outputFileShortName = nodeName.asChar();
	const std::string  asgTemplateName;

#if 0
	//-- collect the output file name and the animation state graph name from the user

	const bool gudResult = getUserData(outputFileShortName, asgTemplateName);
	if (!gudResult)
		return false;
#endif	

	//build export options, to be stored in the log file
	//outputfile
	std::string reexportArguments;
	reexportArguments += ExportArgs::cs_outputFileNameArgName.asChar();
	reexportArguments += " ";
	reexportArguments += outputFileShortName;
	//skeleton
	for(StringVector::iterator i = skeletonTemplateNames.begin(); i != skeletonTemplateNames.end(); ++i)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_skeletonArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += *i + "\"";

		//TODO handle : issues (attachment bones)
	}
	//mesh
	for(StringVector::iterator i2 = meshNames.begin(); i2 != meshNames.end(); ++i2)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_meshGeneratorArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += *i2 + "\"";
	}

	// Handle asg arguments.
	if (!asgTemplateName.empty())
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_animationStateGraphArgName.asChar();
		reexportArguments += " \"";
		reexportArguments += asgTemplateName + "\"";
	}

	// TPERRY - add branch info to export args in log file
	if(commitToSourceControl)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_branchArgName.asChar();
		reexportArguments += " ";
		reexportArguments += branch;
	}

	IGNORE_RETURN(PerforceImporter::setBranch(branch));

	//set export options
	ExporterLog::setMayaExportOptions(reexportArguments);

	const std::string shortLogFilename = outputFileShortName + ".log";
	IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));
	std::string newLogFilename = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	newLogFilename += shortLogFilename;

	if(commitToSourceControl)
	{
		bool exportToPerforceSucceeded = false;

// JU_TODO: alienbrain def out
#if 0
		if (!AlienbrainImporter::connectToServer())
		{
			MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
			return MStatus::kFailure;
		}

		bool result = AlienbrainImporter::preImport(outputFileShortName.c_str(), true);

		if(!ExportManager::validateTextureList(true))
		{
			messenger->printWarningsAndErrors();
			return MS::kSuccess;
		}
		
		//-- create the sat file
		const bool csfResult = createSatFile(outputFileShortName, skeletonTemplateNames, meshNames, asgTemplateName);
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
		if (result)
		{
			AlienbrainImporter::importLogFile();
			AlienbrainImporter::storeFileProperties();
			exportToPerforceSucceeded = PerforceImporter::importCommon(true, createNewChangelist, lock, unlock);
			PerforceImporter::reset();
		}
		IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());
#else
		if(!ExportManager::validateTextureList(true))
		{
			messenger->printWarningsAndErrors();
			return MS::kSuccess;
		}
		
		//-- create the sat file
		const bool csfResult = createSatFile(outputFileShortName, skeletonTemplateNames, meshNames, asgTemplateName);
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
	
		exportToPerforceSucceeded = PerforceImporter::importCommon(true, createNewChangelist, lock, unlock);
		PerforceImporter::reset();
	
#endif
// JU_TODO: end alienbrain def out

		if(!exportToPerforceSucceeded)
		{
			MESSENGER_LOG_ERROR(("Couldn't copy SAT to perforce location on local disk, aborting.\n"));
			return MS::kSuccess;
		}

#if !NO_DATABASE
		IGNORE_RETURN(DatabaseImporter::startSession());
		bool gotAppearance = DatabaseImporter::selectAppearance(true, nodeName.asChar());
		IGNORE_RETURN(DatabaseImporter::endSession());
		if(!gotAppearance)
		{
			MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
			return MS::kSuccess;
		}
#endif
	}
	else
	{
		//-- create the sat file
		const bool csfResult = createSatFile(outputFileShortName, skeletonTemplateNames, meshNames, asgTemplateName);
		MESSENGER_REJECT(!csfResult, ("createSatFile() failed.\n"));
		ExporterLog::setAssetGroup("Local export, N/A");
		ExporterLog::setAssetName("Local export, N/A");
		IGNORE_RETURN(ExporterLog::writeSkeletalAppearanceTemplate(newLogFilename, true));
	}

	ExporterLog::remove();

	//-- Export MeshGeneratorTemplate instances given MeshGeneratorTemplate reference names.
	const bool emgtSuccess = exportMeshGeneratorTemplates(meshNames, commitToSourceControl, createNewChangelist, true, lock, unlock, branch);
	MESSENGER_REJECT(!emgtSuccess, ("failed to export MeshGeneratorTemplate data associated with this SAT."));
	if(showViewerAfterExport)
	{
		std::stringstream assetPathAndFilename;
		assetPathAndFilename << SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX)  << nodeName.asChar() << ".sat";
		ExportManager::LaunchViewer(assetPathAndFilename.str());

	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Create a SAT file from the given parameters.
 *
 * @return  true upon successful construction and writing of the data,
 *          false otherwise.
 */

bool ExportSkeletalAppearanceTemplate::createSatFile(const std::string &outputFileShortName, const StringVector &skeletonReferenceNames, const StringVector &meshGeneratorReferenceNames, const std::string &animationStateGraphReferenceName)
{
	SkeletalAppearanceTemplate  sat;

	// add mesh generators
	{
		const StringVector::const_iterator endIt = meshGeneratorReferenceNames.end();
		for (StringVector::const_iterator it = meshGeneratorReferenceNames.begin(); it != endIt; ++it)
			IGNORE_RETURN(sat.addMeshGenerator(it->c_str()));
	}

	// add skeleton templates
	{
		const StringVector::const_iterator endIt = skeletonReferenceNames.end();
		for (StringVector::const_iterator it = skeletonReferenceNames.begin(); it != endIt; ++it)
		{
			const std::string &compoundSkeletonName = *it;

			// check for skeleton with attachment name embedded.
			// embedded form looks like this: <skeleton reference name>:<attachment transform name>
			std::string::size_type attachmentIndex = compoundSkeletonName.find(cs_attachmentSeparator);
			if (static_cast<int>(attachmentIndex) == static_cast<int>(std::string::npos))
			{
				// this skeleton name doesn't have an embedded attachment.
				IGNORE_RETURN(sat.addSkeletonTemplate(compoundSkeletonName.c_str(), ""));
			}
			else
			{
				// break out skeleton reference name and attachment name
				std::string skeletonReferenceName(compoundSkeletonName, 0, attachmentIndex);
				std::string attachmentTransformName(compoundSkeletonName, attachmentIndex + 1);

				IGNORE_RETURN(sat.addSkeletonTemplate(skeletonReferenceName.c_str(), attachmentTransformName.c_str()));
			}
		}
	}

	// set AnimationStateGraphTemplate name
	if (!animationStateGraphReferenceName.empty())
		sat.setAnimationStateGraphTemplateName(CrcLowerString(animationStateGraphReferenceName.c_str()));

	//-- construct output path name
	std::string  outputPathName(SetDirectoryCommand::getDirectoryString(SAT_WRITE_DIR_INDEX));
	if (!outputPathName.empty() && !((*outputPathName.rbegin() == '\\') || (*outputPathName.rbegin() == '/')))
		outputPathName += '\\';
	outputPathName += outputFileShortName;

	// make sure extension is present
	if (static_cast<int>(outputPathName.find('.')) == static_cast<int>(std::string::npos))
	{
		// no extension, add .sat
		outputPathName += ".sat";
	}

	MESSENGER_LOG(("building SAT data for [%s].\n", outputPathName.c_str()));

	ExporterLog::addClientDestinationFile(outputPathName.c_str());

	//-- write SAT to an IFF
	Iff iff(cs_iffSize);
	sat.write(iff);

	const int iffByteCount = iff.getRawDataSize();
	MESSENGER_LOG(("SAT file [%s]: [%d] bytes.\n", outputPathName.c_str(), iffByteCount));

	//-- write IFF data to the file
	const bool writeSuccess = iff.write(outputPathName.c_str(), true);
	MESSENGER_REJECT(!writeSuccess, ("failed to write SAT data to file [%s].\n", outputPathName.c_str()));

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Construct and return the MeshGeneratorTemplate reference name for each 
 * mesh shape node referenced by the shape dag paths.
 *
 * @param shapeDagPaths                each dag path entry points to a mesh shape that
 *                                     should have its reference name constructed and
 *                                     returned.
 * @param meshGeneratorReferenceNames  a MeshGeneratorTemplate reference name
 *                                     will be constructed and returned for
 *                                     each dag path given.
 *
 * @return  true if operation occurred with no error; false if an error occurred.
 */

bool ExportSkeletalAppearanceTemplate::getMeshGeneratorReferenceNames(const DagPathVector &shapeDagPaths, StringVector &meshGeneratorReferenceNames)
{
	MStatus            status;
	MFnDependencyNode  dependencyNode;
	std::string        referenceName;
	std::string        baseShapeName;
	std::string        parentTransformName; 

	//-- prepare return vector
	meshGeneratorReferenceNames.clear();
	meshGeneratorReferenceNames.reserve(shapeDagPaths.size());

	const DagPathVector::const_iterator endIt = shapeDagPaths.end();
	for (DagPathVector::const_iterator it = shapeDagPaths.begin(); it != endIt; ++it)
	{
		//-- Get shape parent's transform name.
		MDagPath  parentDagPath = *it;
		status = parentDagPath.pop();
		STATUS_REJECT(status, "parentDagPath.pop()");

		//-- Assume shape's parent transform is a detail level if the parent node name fits the form "lX", where X = digit (e.g. "l0", "l1").
		MayaUtility::stripDagPathDirectory(std::string(parentDagPath.partialPathName().asChar()), parentTransformName);
		const bool isParentDetailLevel = (parentTransformName.length() >= 2) && (tolower(parentTransformName[0]) == 'l') && (isdigit(parentTransformName[1]));

		if (!isParentDetailLevel)
			baseShapeName = parentTransformName;
		else
		{
			//-- Get detail level's parent node, the kLodGroup node.
			status = parentDagPath.pop();
			STATUS_REJECT(status, "parentDagPath.pop()");

			//-- Use the name of the kLodGroup node as the base mesh name.
			MayaUtility::stripDagPathDirectory(std::string(parentDagPath.partialPathName().asChar()), baseShapeName);
		}

		//-- construct the MeshGeneratorTemplate reference name
		// @todo create a MESH_REFERENCE_DIR_INDEX directory rather than hardcode this.
		// @todo figure out whether it's an LOD mesh generator or a skeletal mesh generator.  For now assumes it is an LMG mesh generator.
		referenceName = "appearance/mesh/";
		referenceName += baseShapeName;
		referenceName += (isParentDetailLevel ? ".lmg" : ".mgn");

		//-- add reference name to the return vector
		meshGeneratorReferenceNames.push_back(referenceName);
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Return a list of SkeletonTemplate names referenced by the mesh shapes
 * specified in the list of dag paths.
 *
 * The returned ShaderTemplate names are encoded as follows:
 *   <skeleton template reference name>[:<attachment transform name>]
 * The attachment transform name is optional and is present only when
 * the referenced SkeletonTemplate was attached to another SkeletonTemplate.
 *
 * @param meshShapeDagPaths      the meshes for which bound SkeletonTemplate
 *                               references will be retrieved.
 * @param skeletonTemplateNames  SkeletonTemplate references bound to the
 *                               given MeshGeneratorTemplates will be returned
 *                               here.
 *
 * @return  true if operation succeeds, false otherwise.
 */

bool ExportSkeletalAppearanceTemplate::getReferencedSkeletonSegments(const DagPathVector &meshShapeDagPaths, StringVector &skeletonTemplateNames)
{
	MStatus    status;
	StringSet  skeletonTemplateNameSet;

	//-- retrieve skeleton segments referenced by each mesh
	const DagPathVector::const_iterator endIt = meshShapeDagPaths.end();
	for (DagPathVector::const_iterator it = meshShapeDagPaths.begin(); it != endIt; ++it)
	{
		//-- get the mesh dag path
		const MDagPath &meshDagPath = *it;

		//-- find joints and joint weightings for mesh
		MayaMeshWeighting  meshWeighting(meshDagPath, &status);
		STATUS_REJECT(status, "MayaMeshWeighting constructor()");
		
		const bool getMwdSuccess = MayaUtility::addMeshAffectors(meshDagPath, meshWeighting);
		MESSENGER_REJECT(!getMwdSuccess, ("failed to add mesh affectors to MayaMeshWeighting helper class.\n"));

		const bool processSuccess = meshWeighting.processAffectors();
		MESSENGER_REJECT(!processSuccess, ("MayaMeshWeighting failed to process bound mesh transforms.\n"));

		//-- add affecting SkeletonTemplate names
		const bool addResult = addRequiredSkeletonTemplateReferenceNames(meshWeighting, skeletonTemplateNameSet, skeletonTemplateNames);
		MESSENGER_REJECT(!addResult, ("failed to add required skeleton template reference names.\n"));
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Given a fully-processed MayaMeshWeighting instance, construct and retrieve
 * the reference names for the referenced SkeletonTemplates.
 *
 * The returned ShaderTemplate names are encoded as follows:
 *   <skeleton template reference name>[:<attachment transform name>]
 * The attachment transform name is optional and is present only when
 * the referenced SkeletonTemplate was attached to another SkeletonTemplate.
 *
 * @param meshWeighting               the helper class instance containing weight 
 *                                    information.
 * @param skeletonTemplateNameSet     the referenced SkeletonTemplate reference
 *                                    names will be returned in this set.  The
 *                                    set will not be cleared, so it can be used
 *                                    over repeated invocations.
 * @param skeletonTemplateNameVector  the referenced SkeletonTemplate reference
 *                                    names will be returned in proper order in this vector.
 *                                    Only names not present in the set will be added
 *                                    to the vector.
 *
 * @return  true upon successful operation, false otherwise.
 */

bool ExportSkeletalAppearanceTemplate::addRequiredSkeletonTemplateReferenceNames(const MayaMeshWeighting &meshWeighting, StringSet &skeletonTemplateNameSet, StringVector &skeletonTemplateNameVector)
{
	int        transformCount;
	const bool gatcSuccess = meshWeighting.getAffectingTransformCount(&transformCount);
	MESSENGER_REJECT(!gatcSuccess, ("meshWeighting.getAffectingTransformCount() failed\n"));

	MString             mayaTransformName;
	MString             searchSkeletonTemplateNameComponent;
	MDagPath            affectorDagPath;
	MDagPath            skeletonTemplateDagPath;
	MString             skeletonTemplateNodeName;

	std::string         skeletonTemplateReferenceName;

	for (int transformIndex = 0; transformIndex < transformCount; ++transformIndex)
	{
		//-- get the transform name
		const bool gtnSuccess = meshWeighting.getAffectingTransform(transformIndex, &mayaTransformName, &affectorDagPath);
		MESSENGER_REJECT(!gtnSuccess, ("meshWeighting.getTransformName() failed\n"));

		//-- find the skeleton template name for this transform
		bool foundSkeletonTemplateName = false;

		const bool fawncSuccess = MayaUtility::findAncestorWithNameComponent(affectorDagPath, cs_skeletonTemplateNameComponentIndex, &searchSkeletonTemplateNameComponent, &foundSkeletonTemplateName, &skeletonTemplateNodeName, &skeletonTemplateDagPath);
		MESSENGER_REJECT(!fawncSuccess, ("MayaUtility::findAncestorWithNameComponent() failed on [%s]\n", affectorDagPath.partialPathName().asChar()));

		// add skeleton template name to set of skeleton templates referenced by this mesh
		if (foundSkeletonTemplateName)
		{
			//-- construct the SkeletonTemplate reference name
			skeletonTemplateReferenceName =  SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX);
			skeletonTemplateReferenceName += searchSkeletonTemplateNameComponent.asChar();
			skeletonTemplateReferenceName += ".skt";

			//-- append attachment point transform name if skeleton is attached
			std::string  attachmentTransformName;
			bool         hasAttachmentTransform = false;

			const bool gatnResult = MayaUtility::getAttachmentTransformName(skeletonTemplateDagPath, hasAttachmentTransform, attachmentTransformName);
			MESSENGER_REJECT(!gatnResult, ("getAttachmentTransformName() failed.\n"));

			if (hasAttachmentTransform)
			{
				skeletonTemplateReferenceName += ':';
				skeletonTemplateReferenceName += attachmentTransformName;
			}

			//-- add encoded SkeletonTemplate reference name to return set
			std::pair<StringSet::iterator, bool> insertResult = skeletonTemplateNameSet.insert(skeletonTemplateReferenceName);
			if (insertResult.second)
			{
				// add skeleton template to the vector in proper order (i.e. the end)
				skeletonTemplateNameVector.push_back(skeletonTemplateReferenceName);
			}
		}
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve user input for short name for output SAT file and
 * animationStateGraphTemplateName reference name.
 *
 * @param outputFileShortName              the short name (i.e. without directory) 
 *                                         of the SAT file is returned here.
 * @param animationStateGraphTemplateName  the reference name for the ASG
 *                                         is returned here, may be empty.
 *
 * @return  true upon successful completion, false if an error occurred.
 */

#if 0 //disable dialog for now, since users don't need to pick anything useful anymore

bool ExportSkeletalAppearanceTemplate::getUserData(std::string &outputFileShortName, std::string &animationStateGraphTemplateName)
{
	ms_dialogSatShortName = outputFileShortName;
	//-- call ExportSatFile dialog
	const int dlgResult = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE(IDD_EXPORT_SAT), GetActiveWindow(), exportSatDialogProc);
	MESSENGER_REJECT(dlgResult != IDOK, ("user canceled operation.\n"));

	outputFileShortName             = ms_dialogSatShortName;
	animationStateGraphTemplateName = ms_dialogAsgTemplateReferenceName;

	return true;
}

// ----------------------------------------------------------------------

int CALLBACK ExportSkeletalAppearanceTemplate::exportSatDialogProc(HWND dialogHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREF(lParam);

	char buffer[1024];

	switch (message)
	{
		case WM_INITDIALOG:
			//initialize SAT dialog with node name
			IGNORE_RETURN(SetDlgItemText(dialogHandle, IDC_SAT_FILENAME, ms_dialogSatShortName.c_str()));
			// return true so dialog continues to initialize
			return TRUE;

		case WM_COMMAND:
			{
				const int controlId      = LOWORD(wParam);
				const int controlMessage = HIWORD(wParam);
				 
				switch (controlMessage )
				{
					case BN_CLICKED:
						if (controlId == IDC_ASG_BROWSE)
						{
							// handle browse for .asg file

							//-- setup dialog box data
							OPENFILENAME  ofn;
							char          pathName[1024];
							char          shortFileName[1024];

							memset(&ofn, 0, sizeof(ofn));
							strcpy(pathName, ms_dialogLastSatPathSelected.c_str());

							ofn.lStructSize    = sizeof(OPENFILENAME);
							ofn.hwndOwner      = dialogHandle;
							ofn.lpstrFilter    = cs_asgFilter;
							ofn.nFilterIndex   = 1;
							ofn.lpstrFile      = pathName;
							ofn.nMaxFile       = sizeof(pathName);
							ofn.lpstrFileTitle = shortFileName;
							ofn.nMaxFileTitle  = sizeof(shortFileName);
							ofn.lpstrTitle     = "Specify ASG File";
							ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
							ofn.lpstrDefExt    = ".asg";

							//-- open the dialog
							const BOOL cfdResult = GetOpenFileName(&ofn);
							if (cfdResult != 0)
							{
								// user selected a real file

								//-- save path to sat most recently selected by artist.
								ms_dialogLastSatPathSelected = pathName;

								//-- set the short filename into the text of the asg filename text
								IGNORE_RETURN(SetDlgItemText(dialogHandle, IDC_ASG_FILENAME, shortFileName));
							}

							return TRUE;
						}
						else
						{
							if (controlId == IDOK)
							{
								//-- get SAT short name
								IGNORE_RETURN(GetDlgItemText(dialogHandle, IDC_SAT_FILENAME, buffer, sizeof(buffer)));
								ms_dialogSatShortName = buffer;

								//-- get ASG short name
								IGNORE_RETURN(GetDlgItemText(dialogHandle, IDC_ASG_FILENAME, buffer, sizeof(buffer)));
								if (strlen(buffer) > 0)
								{
									// Construct the animation graph path name assuming a TreeFile-relative directory.
									ms_dialogAsgTemplateReferenceName  = "appearance/animationgraph/";
									ms_dialogAsgTemplateReferenceName += buffer;
								}
								else
								{
									// Clear the string, nothing selected.
									ms_dialogAsgTemplateReferenceName.clear();
								}
							}

							// user clicked OK or Cancel, end the dialog
							IGNORE_RETURN(EndDialog(dialogHandle, controlId));
							return TRUE;
						}
				}
			}
			return FALSE;

		default:
			return FALSE;

	}
}

#endif //end disable dialog for now, since users don't need to pick anything useful anymore

// ----------------------------------------------------------------------
/**
 * Given a list of TreeFile-relative pathnames for MeshGeneratorTemplate
 * instances, export a SkeletalMeshGeneratorTemplate for each one.
 *
 * SkeletalMeshGeneratorTemplate exporting is handled by the
 * ExportSkeletalMeshGenerator class.
 *
 * @param meshReferencePathNames  a Vector of TreeFile-relative path names
 *                                to SkeletalMeshGeneratorTemplate instances.
 *
 * @return  true if each SkeletalMeshGenerator template referenced was
 *          exported properly; false if any error occurs in the export
 *          process.
 */

 bool ExportSkeletalAppearanceTemplate::exportMeshGeneratorTemplates(const StringVector &meshReferencePathNames, bool commitToSourceControl, bool createNewChangelist, bool isInteractive, bool lock, bool unlock, const std::string & branch)
{
	//-- Save the active selection list.
	MSelectionList  initialSelectionList;
	MSelectionList  matchList;

	MStatus status = MGlobal::getActiveSelectionList(initialSelectionList);
	STATUS_REJECT(status, "MGlobal::getActiveSelectionList()");

	//-- Build up MArgList for each exportSkeletalMeshGenerator export command.
	MArgList  exportArgList;

	if(isInteractive)
	{
		status = exportArgList.addArg(ExportArgs::cs_interactiveArgName);
		STATUS_REJECT(status, "exportArgList.addArg()");
	}
	if(commitToSourceControl)
	{
		status = exportArgList.addArg(ExportArgs::cs_submitArgName);
		STATUS_REJECT(status, "exportArgList.addArg()");
	}
	if(lock)
	{
		status = exportArgList.addArg(ExportArgs::cs_lockArgName);
		STATUS_REJECT(status, "exportArgList.addArg()");
	}
	if(unlock)
	{
		status = exportArgList.addArg(ExportArgs::cs_unlockArgName);
		STATUS_REJECT(status, "exportArgList.addArg()");
	}
	if(commitToSourceControl)
	{
		status = exportArgList.addArg(ExportArgs::cs_branchArgName);
		status = exportArgList.addArg(MString(branch.c_str()));
	}
	if(createNewChangelist)
	{
		status = exportArgList.addArg(ExportArgs::cs_createNewChangelistArgName);
		STATUS_REJECT(status, "exportArgList.addArg()");
	}

	//pass a parameter saying that this export is part of a larger one, so don't write a log file, submit to AB or P4, etc.
	status = exportArgList.addArg(ExportArgs::cs_partOfOtherExportArgName);
	STATUS_REJECT(status, "exportArgList.addArg()");

	//-- Export each MeshGeneratorTemplate.
	std::string  transformNodeName;
	MDagPath     dagPath;

	const StringVector::const_iterator endIt = meshReferencePathNames.end();
	for (StringVector::const_iterator it = meshReferencePathNames.begin(); it != endIt; ++it)
	{
		//-- Retrieve the node name implied by this template path name.
		getNodeNameFromTemplatePathName(*it, transformNodeName);

		if(!isInteractive)
		{
			status = exportArgList.addArg(ExportArgs::cs_nodeArgName);
			STATUS_REJECT(status, "exportArgList.addArg()");
			status = exportArgList.addArg(MString(transformNodeName.c_str()));
			STATUS_REJECT(status, "exportArgList.addArg()");
		}

		//-- Find the seletion for the given node name.  It should be compatible with a Maya Transform node.
		status = MGlobal::selectByName(MString(transformNodeName.c_str()), MGlobal::kReplaceList);
		MESSENGER_REJECT(!status, ("failed to find Maya node for [%s]; error = [%s].", transformNodeName.c_str(), status.errorString().asChar()));

		status = MGlobal::getActiveSelectionList(matchList);
		STATUS_REJECT(status, "MGlobal::getActiveSelectionList()");

		//-- Process each Transform-compatible dag node.
		const unsigned int matchCount = matchList.length(&status);
		STATUS_REJECT(status, "matchList.length()");

		for (unsigned int entryIndex = 0; entryIndex < matchCount; ++entryIndex)
		{
			status = matchList.getDagPath(entryIndex, dagPath);
			if (status)
			{
				//-- Check if selected dag path is Transform-compatible.
				const bool isTransformCompatible = dagPath.hasFn(MFn::kTransform, &status);
				STATUS_REJECT(status, "dagPath.hasFn()");

				if (isTransformCompatible)
				{
					//-- Mark DagPath as only selected entry for the export command.
					status = MGlobal::select(dagPath, MObject::kNullObj, MGlobal::kReplaceList);
					STATUS_REJECT(status, "MGlobal::select()");

					//-- Export the DagNode via the exportSkeletalMeshGenerator command.
					ExportSkeletalMeshGenerator  exportCommand;

					status = exportCommand.doIt(exportArgList);
					STATUS_REJECT(status, "exportCommand.doIt()");
				}
			}
		}
	}

	//-- Restore the initial active selection list.
	IGNORE_RETURN(MGlobal::setActiveSelectionList(initialSelectionList));

	//-- Return success.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Convert a path name to a datafile into a Maya node name.
 *
 * This function assumes the pathname is of the form:
 *   <some directory path with '\\' or '/' separators>/<node name>.<some extension>
 *
 * The portion listed as <node name> is what will be returned.
 *
 * @param templatePathName  the path name to a data file resource.
 * @param nodeName          the node name is returned in this string.
 */

void ExportSkeletalAppearanceTemplate::getNodeNameFromTemplatePathName(const std::string &templatePathName, std::string &nodeName)
{
	std::string  fileName;

	//-- Strip off directory.
	const std::string::size_type  endOfDirPos = templatePathName.find_last_of(cs_directorySeparators);
	if (static_cast<int>(endOfDirPos) != static_cast<int>(std::string::npos))
	{
		// Copy over the portion after the last directory separator character.
		IGNORE_RETURN(fileName.assign(templatePathName.begin() + (endOfDirPos + 1), templatePathName.end()));
	}
	else
	{
		// Use the whole name, it doesn't contain a directory.
		fileName = templatePathName;
	}

	//-- Strip off extension.
	const std::string::size_type  startOfExtension = fileName.rfind(cs_fileExtensionCharacter);
	if (static_cast<int>(startOfExtension) != static_cast<int>(std::string::npos))
	{
		// Copy everything up to, but not including, the extension.
		IGNORE_RETURN(nodeName.assign(fileName, 0, startOfExtension));
	}
	else
	{
		// Odd, no extension, copy whole thing.
		nodeName = fileName;
	}
}

// ======================================================================
// private member functions
// ======================================================================

ExportSkeletalAppearanceTemplate::ExportSkeletalAppearanceTemplate() :
	MPxCommand()
{
}

// ----------------------------------------------------------------------

ExportSkeletalAppearanceTemplate::~ExportSkeletalAppearanceTemplate()
{
}

// ======================================================================
