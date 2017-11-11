::// ======================================================================
//
// ExportSkeleton.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "ExportSkeleton.h"

#include "AlienbrainImporter.h"
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "ExportManager.h"
#include "LodSkeletonTemplateWriter.h"
#include "MayaCompoundString.h"
#include "MayaConversions.h"
#include "MayaMisc.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "SkeletonTemplateWriter.h"

#include "maya/MArgList.h"
#include "maya/MCommandResult.h"
#include "maya/MDagPath.h"
#include "maya/MEulerRotation.h"
#include "maya/MFileIO.h"
#include "maya/MFnIkJoint.h"
#include "maya/MGlobal.h"
#include "maya/MQuaternion.h"
#include "maya/MSelectionList.h"
#include "maya/MStatus.h"
#include "maya/MVector.h"

#include <vector>
#include <sstream>

// ======================================================================
// static variables
// ======================================================================

namespace
{
	static Messenger *messenger;

	const ConstCharCrcLowerString cs_lodNameArray[] =
		{
			ConstCharCrcLowerString("l0"),
			ConstCharCrcLowerString("l1"),
			ConstCharCrcLowerString("l2"),
			ConstCharCrcLowerString("l3"),
			ConstCharCrcLowerString("l4"),
			ConstCharCrcLowerString("l5"),
			ConstCharCrcLowerString("l6"),
			ConstCharCrcLowerString("l7"),
			ConstCharCrcLowerString("l8"),
			ConstCharCrcLowerString("l9")
		};

	const int cs_lodNameCount = static_cast<int>(sizeof(cs_lodNameArray) / sizeof(cs_lodNameArray[0]));
}

// ======================================================================

const int ExportSkeleton::cms_iffSize = 16 * 1024;

// ======================================================================

#define PRINT_EULER(name, r)  MESSENGER_LOG((name": <%.2f,%.2f,%.2f>\n", convertRadiansToDegrees(static_cast<real>(r.x)), convertRadiansToDegrees(static_cast<real>(r.y)), convertRadiansToDegrees(static_cast<real>(r.z))))

// ======================================================================
// class ExportSkeleton: public static member functions
// ======================================================================

void ExportSkeleton::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ExportSkeleton::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *ExportSkeleton::creator()
{
	return new ExportSkeleton();
}

// ----------------------------------------------------------------------

MString ExportSkeleton::getSkeletonName(const MDagPath &targetDagPath)
{
	MStatus status;
	// get the root node
	MFnDagNode fnDagNode(targetDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create dagNode from selected node\n"));

	// get its name
	const MString name = fnDagNode.name(&status);
	FATAL(!status, ("failed to get name for dag node\n"));

	// break out the base name and filename
	MayaCompoundString compoundName(name);

	const bool hasFilename = (compoundName.getComponentCount() > 1);
	MESSENGER_REJECT(!hasFilename, ("root node [%s] does not have embedded filename after __\n", name.asChar()));

	return compoundName.getComponentString(1);
}

// ======================================================================
// class ExportSkeleton: public member functions
// ======================================================================

MStatus ExportSkeleton::doIt(const MArgList &args)
{
	messenger->clearWarningsAndErrors();

	MESSENGER_INDENT;

	//-- Setup export based on arguments.
	MString      skeletonTemplateDirectory;
	MString      appearanceWriteDirectory;
	MString      authorName;
	MDagPath     targetDagPath;
	std::string  newLogFilename;
	int          bindPoseFrameNumber  = 0;
	bool         commitToSourceControl = false;
	bool         createNewChangelist   = false;
	bool         interactive           = false;
	bool         lock                  = false;
	bool         unlock                = false;
	bool         showViewerAfterExport = false;
	MString      branch;

	ExporterLog::install(messenger);

	ExporterLog::setSourceFilename(MFileIO::currentFile().asChar());
	ExporterLog::setMayaCommand("exportSkeleton");
	
	appearanceWriteDirectory = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);

	// Get, store the base directory.
	std::string             baseDir = appearanceWriteDirectory.asChar();
	std::string::size_type  pos     = baseDir.find_last_of("appearance");
	FATAL(pos == static_cast<unsigned int>(std::string::npos), ("malformed filename in ExportSkeleton::doIt"));

	baseDir  = baseDir.substr(0, pos-strlen("appearance"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);
	
	// Process arguments from given command line.
	const bool processSuccess = processArguments(
		args, 
		&skeletonTemplateDirectory, 
		&authorName, 
		&bindPoseFrameNumber, 
		&targetDagPath, 
		interactive, 
		commitToSourceControl, 
		createNewChangelist, 
		lock, 
		unlock, 
		showViewerAfterExport, 
		branch
	);
	MESSENGER_REJECT_STATUS(!processSuccess, ("argument processing failed\n"));

	//-- Set command strings from args.
	SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, authorName.asChar());
	SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX, skeletonTemplateDirectory.asChar());

	ExporterLog::setAuthor(authorName.asChar());

	std::string skeletonName;

	//-- Do the export.
	if (targetDagPath.apiType() == MFn::kLodGroup)
	{
		const bool performSuccess = performLodExport(bindPoseFrameNumber, targetDagPath, skeletonName);
		if (!performSuccess)
		{
			messenger->printWarningsAndErrors();
			MESSENGER_REJECT_STATUS(!performSuccess, ("failed to export\n"));
		}
	}
	else
	{
		const bool performSuccess = performSingleSkeletonExport(bindPoseFrameNumber, targetDagPath, skeletonName);
		if (!performSuccess)
		{
			messenger->printWarningsAndErrors();
			MESSENGER_REJECT_STATUS(!performSuccess, ("failed to export\n"));
		}
	}

	//-- Build LogFile name.
	std::string shortLogFilename = skeletonName;
	shortLogFilename += "_skeleton.log";

	//-- Build re-export arguments.
	MString nodeName = targetDagPath.partialPathName();
	std::string reexportArguments = ExportArgs::cs_nodeArgName.asChar();
	reexportArguments += " ";
	reexportArguments += nodeName.asChar();
	if(commitToSourceControl)
	{
		reexportArguments += " ";
		reexportArguments += ExportArgs::cs_branchArgName.asChar();
		reexportArguments += " ";
		reexportArguments += branch.asChar();
	}
	ExporterLog::setMayaExportOptions(reexportArguments);

	IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));

	//-- Build filename for new logfile as <log base directory>\<skeletonname>.log.
	newLogFilename  = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	newLogFilename += shortLogFilename;

	//-- Handle AlienBrain support.
	if(commitToSourceControl || lock || unlock)
	{
// JU_TODO: alienbrain def out
#if 0
		if (!AlienbrainImporter::connectToServer())
		{
			MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
			return MStatus::kFailure;
		}

		bool result = AlienbrainImporter::preImport(skeletonName, interactive);

		IGNORE_RETURN(ExporterLog::writeSkeletalTemplate (newLogFilename.c_str(), interactive));

		if (result)
		{
			if(!lock && !unlock)
				AlienbrainImporter::importLogFile();
			AlienbrainImporter::storeFileProperties();

			bool branchSet = PerforceImporter::setBranch(branch.asChar());
			if(branchSet)
			{
				IGNORE_RETURN(PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock));
			}
			else
			{
				MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch.asChar()));
			}

			PerforceImporter::reset();
		}

		IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());
#else
		IGNORE_RETURN(ExporterLog::writeSkeletalTemplate (newLogFilename.c_str(), interactive));

		bool branchSet = PerforceImporter::setBranch(branch.asChar());
		if(branchSet)
		{
			IGNORE_RETURN(PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock));
		}
		else
		{
			MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch.asChar()));
		}

		PerforceImporter::reset();
#endif
// JU_TODO: end alienbrain def out
	}
	else
	{
		ExporterLog::setAssetGroup("Local export, N/A");
		ExporterLog::setAssetName("Local export, N/A");
		IGNORE_RETURN(ExporterLog::writeSkeletalTemplate(newLogFilename.c_str(), interactive));
	}
	ExporterLog::remove();

	messenger->printWarningsAndErrors();

    std::stringstream text;
    text << "Skeleton Export Complete: " << nodeName.asChar() << std::endl;
    messenger->getWarningAndErrorText(text);
    text << std::ends;

	MESSENGER_MESSAGE_BOX(NULL,text.str().c_str(),"Export",MB_OK);

	if(showViewerAfterExport)
	{
		std::stringstream assetPathAndFilename;
		assetPathAndFilename << SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX) << skeletonName.c_str() << ".skt";
		ExportManager::LaunchViewer(assetPathAndFilename.str());
	}

	return MS::kSuccess;
}

// ======================================================================
// class ExportSkeleton: private static member functions
// ======================================================================

bool ExportSkeleton::processArguments(
	const MArgList &args,
	MString *skeletonTemplateDirectory,
	MString *authorName,
	int *bindPoseFrameNumber,
	MDagPath *targetDagPath,
	bool &interactive,
	bool &commitToSourceControl,
	bool &createNewChangelist,
	bool &lock,
	bool &unlock,
	bool &showViewerAfterExport,
  MString &branch
	)
{
	NOT_NULL(skeletonTemplateDirectory);
	NOT_NULL(authorName);
	NOT_NULL(bindPoseFrameNumber);
	NOT_NULL(targetDagPath);

	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));
	MESSENGER_REJECT(!argCount, ("ExportSkeleton: missing args\n"));

	//-- handle each argument
	bool haveAuthor          = false;
	bool haveOutputDirectory = false;
	bool haveNode            = false;
	bool haveFrame           = false;
	bool isInteractive       = false;
	bool haveBranch          = false;

	interactive              = false;
	commitToSourceControl    = false;
	createNewChangelist      = false;
	lock                     = false;
	unlock                   = false;
	showViewerAfterExport    = false;

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
			MESSENGER_REJECT(argIndex != 0, ("-interactive must be first argument"));
			isInteractive = TRUE;
			interactive  = true;
		}
		else if (argName == ExportArgs::cs_showViewerAfterExport)
		{
			showViewerAfterExport = true;
		}
		//commit flag is used to state that we should go through all the processes to send the data to the
		//source control systems (presents GUI's if necessary), etc.
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
		else if (argName == ExportArgs::cs_authorArgName)
		{
			//-- handle author argument
			MESSENGER_REJECT(haveAuthor, ("author argument specified multiple times\n"));

			*authorName = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get author name argument\n"));

			// fixup argIndex
			++argIndex;
			haveAuthor = true;
		}
		else if (argName == ExportArgs::cs_outputDirArgName)
		{
			//-- handle filename argument
			MESSENGER_REJECT(haveOutputDirectory, ("outputdir argument specified multiple times\n"));

			*skeletonTemplateDirectory = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get filename argument\n"));

			// fixup argIndex
			++argIndex;
			haveOutputDirectory = true;
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
				MESSENGER_REJECT(!status, ("MGlobal::getSelectionListByName failure for node name [%s]\n", nodeName.asChar()));

				MESSENGER_REJECT(nodeList.length() < 1, ("no scene nodes match specified export node [%s]\n", nodeName.asChar()));
				MESSENGER_REJECT(nodeList.length() > 1, ("multiple nodes match specified export node [%s]\n", nodeName.asChar()));

				status = nodeList.getDagPath(0, *targetDagPath);
				MESSENGER_REJECT(!status, ("failed to get dag path for export node [%s]\n", nodeName.asChar()));

				// fixup argIndex
				++argIndex;
				haveNode = true;
			}
		}
		else if (argName == ExportArgs::cs_frameArgName)
		{
			//--handle node argument
			MESSENGER_REJECT(haveFrame, ("frame argument specified multiple times\n"));

			*bindPoseFrameNumber = args.asInt(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get frame number argument\n"));

			haveFrame = true;
		}
		else if (argName == ExportArgs::cs_warningsArgName)
		{
			// Do nothing, warning support removed.
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

	//-- handle default author specification
	if (!haveAuthor)
	{
		// get from author command
		status = authorName->set(SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX));
		MESSENGER_REJECT(!status, ("failed to set authorName string with [%s]\n", SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX)));
	}

	//-- handle default output directory specification
	if (!haveOutputDirectory)
	{
		// get from skeleton template write directory command
		status = skeletonTemplateDirectory->set(SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX));
		MESSENGER_REJECT(!status, ("failed to set skeletonTemplateDirectory string with [%s]\n", SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX)));
	}

	//-- handle default frame specification
	if (!haveFrame)
	{
		*bindPoseFrameNumber = -10;
		MESSENGER_LOG(("ARGS: using default bind pose frame number [%d]\n", *bindPoseFrameNumber));
	}

	MESSENGER_REJECT(commitToSourceControl && !haveBranch, ("no branch, i.e. \"-branch <branchname>\" was specified\n"));
	MESSENGER_REJECT(!isInteractive && !haveNode, ("neither joint node (-node) nor (-interactive) was specified\n"));

	return MStatus();
}

// ----------------------------------------------------------------------

bool ExportSkeleton::collectSkeletonNames(const MDagPath &lodGroupDagPath, CrcLowerStringVector &skeletonShortNames)
{
	//-- Retrieve the LOD 0 child node.
	MDagPath lodDagPath0;

	const bool getChildSuccess = getLodPath(lodGroupDagPath, 0, lodDagPath0);
	MESSENGER_REJECT(!getChildSuccess, ("failed to get l0 LOD node.\n"));

	//-- Walk the children of the given LOD level, collecting skeleton names.
	const bool addSuccess = addJointSkeletonNames(lodDagPath0, skeletonShortNames);
	MESSENGER_REJECT(!addSuccess, ("failed add skeleton names from LOD l0.\n"));

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeleton::addJointSkeletonNames(const MDagPath &jointPath, CrcLowerStringVector &skeletonShortNames)
{
	MStatus status;

	//-- Stop as soon as I hit a non-transform or non-joint.
	const bool isTransformCompatible = jointPath.hasFn(MFn::kTransform, &status);
	STATUS_REJECT(status, "jointPath.hasFn()");

	if (!isTransformCompatible)
		return true;

	//-- Check if this node is ignored.
	const bool isIgnored = MayaUtility::ignoreNode(jointPath);
	if (!isIgnored)
	{
		//-- Grab MFnDagPath for the path.
		MFnDagNode dagNode(jointPath, &status);
		STATUS_REJECT(status, "creating MFnDagNode from joint");

		//-- Check for a skeleton name.
		MString name = dagNode.name(&status);
		STATUS_REJECT(status, "dagNode.name()");

		MayaCompoundString  compoundName(name);
		if (compoundName.getComponentCount() > 1)
		{
			MString firstComponent = compoundName.getComponentString(0);
			bool const isMeshRelativeHardpoint = (_stricmp(firstComponent.asChar(), "hp") == 0);

			if (!isMeshRelativeHardpoint)
			{
				CrcLowerString skeletonName(compoundName.getComponentString(1).asChar());
				skeletonShortNames.push_back(skeletonName);
			}
		}
	}

	//-- Handle all children.
	const unsigned childCount = jointPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for bone\n"));

	MFnDagNode  fnDagNode;
	MDagPath    childDagPath;

	for (unsigned i = 0; i < childCount; ++i)
	{
		// Get the child MObject.
		MObject childObject = jointPath.child(i, &status);
		STATUS_REJECT(status, "failed to get child MObject");

		if (childObject.hasFn(MFn::kTransform))
		{
			// Turn it into a dag node so we can get a dag path to it.
			status = fnDagNode.setObject(childObject);
			STATUS_REJECT(status, "fnDagNode.setObject()");

			status = fnDagNode.getPath(childDagPath);
			STATUS_REJECT(status, "fnDagNode.getPath()");

			const bool childSuccess = addJointSkeletonNames(childDagPath, skeletonShortNames);
			MESSENGER_REJECT(!childSuccess, ("failed to add child joint\n"));
		}
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeleton::getLodPath(const MDagPath &lodGroupPath, int lodIndex, MDagPath &lodPath)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, cs_lodNameCount);

	//-- Get the child with the specified lod index name.
	return MayaUtility::getChildPathWithName(lodGroupPath, cs_lodNameArray[lodIndex], lodPath);
}

// ----------------------------------------------------------------------

bool ExportSkeleton::getSkeletonStartPath(const MDagPath &searchPath, const CrcLowerString &skeletonName, MDagPath &skeletonStartPath)
{
	MStatus status;

	//-- Stop as soon as I hit a non-transform or non-joint.
	const bool isTransformCompatible = searchPath.hasFn(MFn::kTransform, &status);
	STATUS_REJECT(status, "searchPath.hasFn()");

	if (!isTransformCompatible)
		return false;

	//-- Check if this node is ignored.
	const bool isIgnored = MayaUtility::ignoreNode(searchPath);
	if (!isIgnored)
	{
		//-- Grab MFnDagPath for the path.
		MFnDagNode dagNode(searchPath, &status);
		STATUS_REJECT(status, "creating MFnDagNode from joint");

		//-- Check for a skeleton name.
		MString name = dagNode.name(&status);
		STATUS_REJECT(status, "dagNode.name()");

		MayaCompoundString  compoundName(name);
		if (compoundName.getComponentCount() > 1)
		{
			CrcLowerString thisJointSkeletonName(compoundName.getComponentString(1).asChar());
			if (thisJointSkeletonName == skeletonName)
			{
				//-- Found it.
				skeletonStartPath = searchPath;
				return true;
			}
		}
	}

	//-- Handle all children.
	const unsigned childCount = searchPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for bone\n"));

	MFnDagNode  fnDagNode;
	MDagPath    childDagPath;

	for (unsigned i = 0; i < childCount; ++i)
	{
		// Get the child MObject.
		MObject childObject = searchPath.child(i, &status);
		STATUS_REJECT(status, "failed to get child MObject");

		if (childObject.hasFn(MFn::kTransform))
		{
			// Turn it into a dag node so we can get a dag path to it.
			status = fnDagNode.setObject(childObject);
			STATUS_REJECT(status, "fnDagNode.setObject()");

			status = fnDagNode.getPath(childDagPath);
			STATUS_REJECT(status, "fnDagNode.getPath()");

			const bool foundStart = getSkeletonStartPath(childDagPath, skeletonName, skeletonStartPath);
			if (foundStart)
			{
				// One of this node's children found the start of the named skeleton segment, return true.
				return true;
			}
		}
	}

	//-- Did not find the start of the skeleton segment with the given name.
	return false;
}

// ----------------------------------------------------------------------

bool ExportSkeleton::hasDuplicateNames(const MDagPath &hierarchyStartPath)
{
	MStringArray duplicateNames;

	const bool checkSuccess = MayaUtility::checkForDuplicateShortNames(hierarchyStartPath, &duplicateNames, MFn::kTransform);
	MESSENGER_REJECT(!checkSuccess, ("Check for duplicate short node names failed.\n"));

	const unsigned duplicateNameCount = duplicateNames.length();
	if (duplicateNameCount)
	{
		MESSENGER_LOG(("Skeleton starting at DAG path [%s] has dulpicate short node names, please fix:\n", hierarchyStartPath.fullPathName().asChar()));

		MESSENGER_INDENT;
		for (unsigned i = 0; i < duplicateNameCount; ++i)
		{
			const MString &nodeName = duplicateNames[i];
			MESSENGER_LOG(("[%s]\n", nodeName.asChar()));
		}
		
		//-- Indicate duplicate names exist.
		return true;
	}

	//-- No duplicates.
	return false;
}

// ======================================================================
// class ExportSkeleton: private member functions
// ======================================================================

ExportSkeleton::ExportSkeleton() :
	MPxCommand(),
	m_printDiagnostics(false)
{
}

// ----------------------------------------------------------------------

bool ExportSkeleton::performSingleSkeletonExport(int bindPoseFrameNumber, const MDagPath &targetDagPath, /*OUT*/std::string& skeletonName)
{
	MESSENGER_INDENT;
	MStatus status;

	//get skt name for return to caller (for logging and export purposes)
	skeletonName = getSkeletonName(targetDagPath).asChar();

	//-- Check for duplicate short names in selected skeleton hierarchy.
	MESSENGER_REJECT(hasDuplicateNames(targetDagPath), ("duplicate short node names exist, aborting\n"));

	//-- go to bind pose
	const bool gtbpSuccess = MayaUtility::goToBindPose(bindPoseFrameNumber);
	MESSENGER_REJECT(!gtbpSuccess, ("failed to go to bind pose\n"));

	//-- construct the skeleton template write path
	char outputPath[MAX_PATH];
	{
		const MString baseFilename = getSkeletonName(targetDagPath);

		// build the output filename
		strcpy(outputPath, SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX));

		const size_t directoryLength = strlen(outputPath);
		if (outputPath[directoryLength-1] != '\\')
			strcat(outputPath, "\\");

		strcat(outputPath, baseFilename.asChar());
		strcat(outputPath, ".skt");

		ExporterLog::addSkeletalTemplate(outputPath);

		MESSENGER_LOG(("STATS: constructed output filename [%s]\n", outputPath));
	}

	//-- build the skeleton
	SkeletonTemplateWriter  writer;
	const int               firstParentIndex = -1;

	IGNORE_RETURN(addMayaJoint(writer, targetDagPath, firstParentIndex));
	MESSENGER_LOG(("STATS: joint count: %d\n", writer.getJointCount()));

	//-- save as iff
	Iff  iff(cms_iffSize);

	writer.write(&iff);

	MESSENGER_LOG(("STATS: skeleton template size: %d bytes\n", iff.getRawDataSize()));

	//-- write iff to file
	const bool writeSuccess = iff.write(outputPath, true);
	MESSENGER_REJECT(!writeSuccess, ("failed to write file [%s]\n", outputPath));

	MESSENGER_LOG(("successfully wrote [%s]\n", outputPath));

	//-- enable all nodes (we disabled them above)
	const bool edSuccess = MayaUtility::enableDeformers();
	MESSENGER_REJECT(!edSuccess, ("failed to enable deformers after capturing bind pose data\n"));

	return MStatus();
}

// ----------------------------------------------------------------------

bool ExportSkeleton::performLodExport(int bindPoseFrameNumber, const MDagPath &targetDagPath, /*OUT*/std::string &skeletonName)
{
	MStatus  status;

	//-- Collect skeleton names to export.
	CrcLowerStringVector  skeletonNames;

	const bool result = collectSkeletonNames(targetDagPath, skeletonNames);
	MESSENGER_REJECT(!result, ("failed to collect skeleton names from LOD 0 of lod group node.\n"));

#if 1
	//-- Print the skeleton names.
	MESSENGER_LOG(("LOD-based Skeleton Export.\n"));
	MESSENGER_LOG(("-| [%u] skeleton names found in LOD 0.\n", skeletonNames.size()));

	const CrcLowerStringVector::iterator endIt2 = skeletonNames.end();
	for (CrcLowerStringVector::iterator it2 = skeletonNames.begin(); it2 != endIt2; ++it2)
	{
		MESSENGER_LOG(("-| skeleton [%s].\n", it2->getString()));
	}
#endif

	//-- Export each skeleton separately.
	const CrcLowerStringVector::iterator endIt = skeletonNames.end();
	for (CrcLowerStringVector::iterator it = skeletonNames.begin(); it != endIt; ++it)
	{
		LodSkeletonTemplateWriter  lodWriter;
		MDagPath                   lodPath;
		MDagPath                   skeletonStartPath;
		int                        lodIndex = 0;
		bool                       done;

		do
		{
			//-- Get the MDagPath to this lod.
			const bool gotLodPath = getLodPath(targetDagPath, lodIndex, lodPath);
			if (!gotLodPath)
			{
				// Done, no more LODs.
				break;
			}

			//-- Get the starting path of the named Skeleton segment.
			const bool gotSkeletonStartPath = getSkeletonStartPath(lodPath, *it, skeletonStartPath);
			if (!gotSkeletonStartPath)
			{
				// This LOD does not have any data for the skeleton segment, so the skeleton segment is done (i.e. not applied at all in this LOD).
				done = true;
			}
			else
			{
				// There is at least some skeleton segment data in this LOD, so I'm not yet done.
				done = false;

				//-- Check for duplicate node names starting at this skeleton.
				MESSENGER_REJECT(hasDuplicateNames(skeletonStartPath), ("duplicate short node names exist in lod [%d], aborting\n", lodIndex));

				//-- Set the starting skeleton path as the currently selected Maya node for the "go to bind pose" command (which acts on the selected node, I think).
				MSelectionList  newActiveList;

				status = newActiveList.add(skeletonStartPath);
				STATUS_REJECT(status, "newActiveList.add()");

				status = MGlobal::setActiveSelectionList(newActiveList);
				STATUS_REJECT(status, "MGobal::setActiveSelectionList()");

				//-- Go to bind pose.
				const bool gtbpSuccess = MayaUtility::goToBindPose(bindPoseFrameNumber);
				MESSENGER_REJECT(!gtbpSuccess, ("failed to go to bind pose\n"));

				//-- Create the Skeleton Template writer for this LOD level.
				SkeletonTemplateWriter *const levelWriter = new SkeletonTemplateWriter();

				//-- Export the SkeletonTemplate data for this LOD.
				const int firstParentIndex = -1;

				IGNORE_RETURN(addMayaJoint(*levelWriter, skeletonStartPath, firstParentIndex));
				MESSENGER_LOG(("STATS: skeleton [%s], lod [%d] joint count: %d\n", it->getString(), lodIndex, levelWriter->getJointCount()));

				//-- Add this LOD's data to the LOD writer.
				lodWriter.addDetailLevel(levelWriter);
			}

			//-- Increment the loop.
			++lodIndex;

		} while (!done);

		//-- Construct output pathname.
		std::string outputPath(SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX));

		// Ensure there's a trailing backslash on name.
		if (!outputPath.empty() && (outputPath[outputPath.length() - 1] != '\\'))
			outputPath += '\\';

		// Build, add skeleton name (save in ref param to return to caller, this is needed for logging, submission)
		skeletonName = it->getString();

		// Add skeleton name to path
		outputPath += skeletonName;
		outputPath += ".skt";

		//-- Write the LodSkeletonTemplate data.
		Iff  iff(cms_iffSize);

		lodWriter.write(iff);

		const bool writeFileSuccess = iff.write(outputPath.c_str(), true);
		MESSENGER_REJECT(!writeFileSuccess, ("failed to write LOD-based skeleton data to [%s].\n", outputPath.c_str()));

		MESSENGER_LOG(("wrote LOD skeleton file [%s], %d bytes.\n", outputPath.c_str(), iff.getRawDataSize()));

		//-- Tell ExporterLog about the exported SkeletonTemplate.
		ExporterLog::addSkeletalTemplate(outputPath);
	}

	UNREF(bindPoseFrameNumber);
	UNREF(targetDagPath);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Add a maya joint to the SkeletonTemplateWriter.
 */

bool ExportSkeleton::addMayaJoint(SkeletonTemplateWriter &writer, const MDagPath &targetDagPath, int parentIndex)
{
	MESSENGER_INDENT;
	MStatus  status;

	//-- make sure node is a joint
	const MFn::Type nodeType = targetDagPath.apiType(&status);
	MESSENGER_REJECT(!status, ("failed to get target dag node's node type\n"));

	int         jointIndex;
	MString     jointName;

	const bool  isIgnored = MayaUtility::ignoreNode(targetDagPath);
	if (isIgnored)
	{
		// simply pass through this node
		jointIndex                    = parentIndex;
		jointName                     = "<ignored joint>";
		if (m_printDiagnostics)
			MESSENGER_LOG(("START joint: [%s]\n", jointName.asChar()));
	}
	else
	{
		MESSENGER_REJECT((nodeType != MFn::kJoint) && (nodeType != MFn::kTransform), ("tried to add non-joint, non-transform node to skeleton hierarchy\n"));
		if (nodeType == MFn::kTransform)
		{
			// we ignore pure transforms unless they have a joint or a mesh below them in their hierarchy
			if (!MayaMisc::hasNodeTypeInHierarachy(targetDagPath, MFn::kJoint) && !MayaMisc::hasNodeTypeInHierarachy(targetDagPath, MFn::kMesh))
			{
				// skip this node
				return true;
			}
		}

		MFnTransform fnTransform(targetDagPath, &status);
		MESSENGER_REJECT(!status, ("failed to set MFnTransform for target DagPath [%s]\n", targetDagPath.partialPathName().asChar()));

		//-- get joint name
		jointName = fnTransform.name(&status);
		MESSENGER_REJECT(!status, ("failed to retrieve joint's name\n"));

		//-- check if we're a new skeleton template marked with "nodename__filename" name type.  if so, we skip this child
		{
			MayaCompoundString  compoundName(jointName);

			const bool hasFilename           = (compoundName.getComponentCount() > 1);
			const bool isNewSkeletonTemplate = hasFilename && (writer.getJointCount() != 0);

			if (isNewSkeletonTemplate)
			{
				// we don't export multiple skeleton templates
				return true;
			}

			if (hasFilename)
			{
				//-- reset the joint name to the portion we care about (strip out filename)
				jointName = compoundName.getComponentString(0);
			}
		}

		//-- announce this joint
		if (m_printDiagnostics)
			MESSENGER_LOG(("START joint: [%s]\n", jointName.asChar()));

		{
			MESSENGER_INDENT; //lint !e578 // hiding outer indenter // yes

			//-- build pre-muliply rotation
			// according to Maya docs, this should be the Transform node's "Rotate Axis" rotation
			// get Maya transform "Rotate Axis."  this rotation takes place in time earlier
			// than the joint orient, but after the transform's rotation.
			const MQuaternion mayaRotateAxisQuaternion = fnTransform.rotateOrientation(MSpace::kTransform, &status);
			MESSENGER_REJECT(!status, ("failed to get rotateOrientation quaternion\n"));

			const MEulerRotation mayaRotateAxisEuler = mayaRotateAxisQuaternion.asEulerRotation();
			if (m_printDiagnostics)
				PRINT_EULER("maya rotate axis", mayaRotateAxisEuler);

			const Quaternion preMultiplyRotation = MayaConversions::convertRotation(mayaRotateAxisEuler);

			//-- build the post muliply rotation
			Quaternion postMultiplyRotation;

			if (nodeType == MFn::kJoint) //lint !e734 !e569 // loss of precision (10 bits to 7), loss of information // huh?
			{
				MFnIkJoint fnJoint(targetDagPath, &status);
				MESSENGER_REJECT(!status, ("failed to set MFnIkJoint for joint\n"));

				// according to Maya docs, this should  be the joint's JointOrient attribute
				MEulerRotation mayaJointOrientation;
				status = fnJoint.getOrientation(mayaJointOrientation);
				MESSENGER_REJECT(!status, ("failed to get joint orientation\n"));

				if (m_printDiagnostics)
					PRINT_EULER("maya joint orient", mayaJointOrientation);
				postMultiplyRotation = MayaConversions::convertRotation(mayaJointOrientation);
			}
			else
				postMultiplyRotation = Quaternion::identity;

			//-- get bind pose translation
			// get Maya translate
			MVector mayaJointTranslation = fnTransform.translation(MSpace::kTransform, &status);
			MESSENGER_REJECT(!status, ("failed to get joint translation\n"));

			// convert to game translation
			if (m_printDiagnostics)
				MESSENGER_LOG(("maya joint translate: <%.2f,%.2f,%.2f>\n", mayaJointTranslation.x, mayaJointTranslation.y, mayaJointTranslation.z));
			const Vector bindPoseTranslation = MayaConversions::convertVector(mayaJointTranslation);

			//-- get bind pose rotation
			// get maya joint rotation
			MEulerRotation mayaJointRotation;
			status = fnTransform.getRotation(mayaJointRotation);
			MESSENGER_REJECT(!status, ("failed to get joint rotation\n"));

			// convert to game rotation
			if (m_printDiagnostics)
				PRINT_EULER("maya joint rotation", mayaJointRotation);
			const Quaternion bindPoseRotation = MayaConversions::convertRotation(mayaJointRotation);

			//-- get transform's "rotateOrder (ro)" enum, equals jointRotationOrder
			JointRotationOrder  jointRotationOrder = JRO_COUNT;

			switch (mayaJointRotation.order)
			{
				case MEulerRotation::kXYZ: jointRotationOrder = JRO_xyz; break;
				case MEulerRotation::kYZX: jointRotationOrder = JRO_yzx; break;
				case MEulerRotation::kZXY: jointRotationOrder = JRO_zxy; break;
				case MEulerRotation::kXZY: jointRotationOrder = JRO_xzy; break;
				case MEulerRotation::kYXZ: jointRotationOrder = JRO_yxz; break;
				case MEulerRotation::kZYX: jointRotationOrder = JRO_zyx; break;

				default:
					FATAL(true, ("unexpected rotation order for mayaJointRotation"));
			}

			//-- create the joint
			jointIndex = -1;

			const bool success = writer.addJoint(
				parentIndex,
				jointName.asChar(),
				preMultiplyRotation,
				postMultiplyRotation,
				bindPoseTranslation,
				bindPoseRotation,
				jointRotationOrder,
				&jointIndex);

			MESSENGER_REJECT(!success, ("failed to add joint to SkeletonTemplateWriter\n"));
		}
	}

	//-- handle joint's children
	const unsigned childCount = targetDagPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for bone\n"));

	for (unsigned i = 0; i < childCount; ++i)
	{
		// get the object
		MObject childObject = targetDagPath.child(i, &status);
		MESSENGER_REJECT(!status, ("failed to get child index [%u]\n", i));

		const MFn::Type apiType = childObject.apiType();
		if ((apiType == MFn::kJoint) || (apiType == MFn::kTransform))
		{
			// we've got a child joint, let's process it

			// turn it into a dag node so we can get a dag path to it
			MFnDagNode  fnDagNode(childObject, &status);
			MESSENGER_REJECT(!status, ("failed to set MFnDagNode for joint's child joint\n"));

			MDagPath childDagPath;
			status = fnDagNode.getPath(childDagPath);
			MESSENGER_REJECT(!status, ("failed to get a dag path to child joint object\n"));

			const bool childSuccess = addMayaJoint(writer, childDagPath, jointIndex);
			MESSENGER_REJECT(!childSuccess, ("failed to add child joint\n"));
		}
	}

	//-- we're done
	if (m_printDiagnostics)
		MESSENGER_LOG(("END joint: [%s]\n", jointName.asChar()));

	return true;
}

// ======================================================================
