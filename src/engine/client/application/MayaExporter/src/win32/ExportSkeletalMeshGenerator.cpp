// ======================================================================
//
// ExportSkeletalMeshGenerator.cpp
// copyright 2001-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "ExportSkeletalMeshGenerator.h"

//engine shared includes
#include "sharedFile/Iff.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/UniqueNameList.h"

//local MayaExporter includes
#include "AlienbrainImporter.h"
#if !NO_DATABASE
#include "DatabaseImporter.h"
#endif
#include "ExporterLog.h"
#include "ExportArgs.h"
#include "ExportManager.h"
#include "LodMeshGeneratorTemplateWriter.h"
#include "MayaCompoundString.h"
#include "MayaConversions.h"
#include "MayaMeshWeighting.h"
#include "MayaShaderTemplateBuilder.h"
#include "MayaMisc.h"
#include "Messenger.h"
#include "OccludedFaceMapGenerator.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "SkeletalMeshGeneratorWriter.h"
#include "UniqueVertexSet.h"

// STL/system includes
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

// boost includes
#include "boost/smart_ptr.hpp"

// maya includes
#include "maya/MCommandResult.h"
#include "maya/MArgList.h"
#include "maya/MDagPath.h"
#include "maya/MFileIO.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MFnBlendShapeDeformer.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnMesh.h"
#include "maya/MFnSkinCluster.h"
#include "maya/MFnTransform.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MGlobal.h"
#include "maya/MItDag.h"
#include "maya/MItDependencyNodes.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MQuaternion.h"
#include "maya/MSelectionList.h"
#include "maya/MStatus.h"
#include "maya/MStringArray.h"

// ======================================================================

const int   ExportSkeletalMeshGenerator::ms_initialIffSize                     = 1 * 1024*1024;
const int   ExportSkeletalMeshGenerator::ms_skeletonTemplateNameComponentIndex = 1;
const int   ExportSkeletalMeshGenerator::ms_maxSupportedPolygonVertexCount     = 12;
const real  ExportSkeletalMeshGenerator::ms_blendPositionEpsilon               = CONST_REAL(0.0001);
const real  ExportSkeletalMeshGenerator::ms_blendNormalEpsilon                 = CONST_REAL(0.0001);

bool        ExportSkeletalMeshGenerator::ms_installed;

static Messenger *messenger;

// ======================================================================

namespace ExportSkeletalMeshGeneratorNamespace
{
	const int c_seondsPerMinute = 60;
	const int c_secondsPerHour = 3600;
	const char *const ms_occludedFaceZonePrefix         = "OF";
	const MString     ms_zonesThisOccludesAttributeName = "SOE_OCCLUDES";
	const MString     ms_occlusionLayerAttributeName    = "SOE_LAYER";

	char const *const s_lodMeshGeneratorTemplateExtensionWithDot      = ".lmg";
	const char *const s_skeletalMeshGeneratorTemplateExtensionWithDot = ".mgn";

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

using namespace ExportSkeletalMeshGeneratorNamespace;

// ======================================================================
// embedded entity declarations
// ======================================================================

struct ExportSkeletalMeshGenerator::BlendShapeWeight
{
public:

	BlendShapeWeight(const MObject &blendShapeObject, const MObject &baseObject, int weightIndex, float weight): 
			m_blendShapeObject(blendShapeObject),
			m_baseObject(baseObject),
			m_weightIndex(weightIndex),
			m_weight(weight) {}

public:

	MObject   m_blendShapeObject;
	MObject   m_baseObject;
	int       m_weightIndex;
	float     m_weight;

private:
	// disabled
	BlendShapeWeight();
};

// ======================================================================

struct ExportSkeletalMeshGenerator::HardpointInfo
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct HardpointBlendTarget
	{
	public:

		HardpointBlendTarget(const CrcLowerString &blendShapeName, const MObject &transformObject);

	public:

		CrcLowerString  m_blendShapeName;
		MObject         m_transformObject;

	private:
		// disabled
		HardpointBlendTarget();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<const CrcLowerString*, boost::shared_ptr<HardpointBlendTarget>, LessPointerComparator>  HardpointBlendTargetMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	explicit HardpointInfo(const CrcLowerString &hardpointName);

public:

	CrcLowerString           m_hardpointName;
	MObject                  m_baseTransformObject;
	HardpointBlendTargetMap  m_hardpointBlendTargets;

private:
	// disabled
	HardpointInfo();
};

// ======================================================================
// class ExportSkeletalMeshGenerator::HardpointInfo::HardpointBlendTarget
// ======================================================================

ExportSkeletalMeshGenerator::HardpointInfo::HardpointBlendTarget::HardpointBlendTarget(const CrcLowerString &blendShapeName, const MObject &transformObject)
:	m_blendShapeName(blendShapeName),
	m_transformObject(transformObject)
{
}

// ======================================================================
// class ExportSkeletalMeshGenerator::HardpointInfo
// ======================================================================

ExportSkeletalMeshGenerator::HardpointInfo::HardpointInfo(const CrcLowerString &hardpointName)
:	m_hardpointName(hardpointName),
	m_baseTransformObject(),
	m_hardpointBlendTargets()
{
}

// ======================================================================
// class ExportSkeletalMeshGenerator: public static member functions
// ======================================================================

void ExportSkeletalMeshGenerator::install(Messenger *newMessenger)
{
	FATAL(ms_installed, ("ExportSkeletalMeshGenerator already installed"));

	messenger = newMessenger;
	ms_installed = true;
}

// ----------------------------------------------------------------------

void ExportSkeletalMeshGenerator::remove(void)
{
	FATAL(!ms_installed, ("ExportSkeletalMeshGenerator not installed"));

	messenger    = 0;
	ms_installed = false;
}

// ----------------------------------------------------------------------

void *ExportSkeletalMeshGenerator::creator(void)
{
	FATAL(!ms_installed, ("ExportSkeletalMeshGenerator not installed"));

	return new ExportSkeletalMeshGenerator();
}

// ======================================================================
// class ExportSkeletalMeshGenerator: public member functions
// ======================================================================

ExportSkeletalMeshGenerator::ExportSkeletalMeshGenerator()
:
	MPxCommand(),
	m_meshTriangleCount(0),
	m_ignoreShaders(false),
	m_ignoreBlendTargets(false),
	m_ignoreTextures(false)
{
}

// ----------------------------------------------------------------------

ExportSkeletalMeshGenerator::~ExportSkeletalMeshGenerator()
{
}

// ----------------------------------------------------------------------

MStatus ExportSkeletalMeshGenerator::doIt(const MArgList &args)
{
	PerformanceTimer exportTimer;
	exportTimer.start();

	messenger->clearWarningsAndErrors();

	MESSENGER_LOG(("========| START: Skeletal Mesh Export |========\n"));
	
	ExporterLog::install (messenger);
	ExporterLog::setMayaCommand("exportSkeletalMeshGenerator");
	ExporterLog::setSourceFilename (MFileIO::currentFile().asChar());
	std::string newLogFilename;

	MString   authorName;
	MString   generatorOutputDirectory;
	MDagPath  targetNode;
	bool commitToSourceControl = false;
	bool createNewChangelist   = false;
	bool interactive           = false;
	bool independentExport     = true;
	bool exportSuccess         = true;
	bool lock                  = false;
	bool unlock                = false;
	bool showViewerAfterExport = false;
	MString branch;

	MESSENGER_INDENT;

	//-- setup export based on arguments
	int       bindPoseFrameNumber = 0;

	const bool processSuccess = processArguments(
		args, 
		&authorName, 
		&generatorOutputDirectory, 
		&targetNode, 
		m_ignoreShaders, 
		m_ignoreBlendTargets, 
		bindPoseFrameNumber, 
		m_ignoreTextures, 
		interactive, 
		commitToSourceControl, 
		createNewChangelist, 
		independentExport, 
		lock, 
		unlock, 
		showViewerAfterExport,
		branch
	);
	MESSENGER_REJECT_STATUS(!processSuccess, ("argument processing failed\n"));

	//-- handle directory changes
	SetDirectoryCommand::setDirectoryString(APPEARANCE_WRITE_DIR_INDEX, generatorOutputDirectory.asChar());
	SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, authorName.asChar());

	//get, store the base directory
	std::string baseDir = generatorOutputDirectory.asChar();
	std::string::size_type pos = baseDir.find_last_of("appearance");
	FATAL(static_cast<int>(pos) == static_cast<int>(std::string::npos), ("malformed filename in ExportSkeletalMeshGenerator::doIt"));
	baseDir = baseDir.substr(0, pos-strlen("appearance"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);

	//-- do the export
	m_meshTriangleCount = 0;

	MString nodeName = targetNode.partialPathName();

	//build the info needed to reexport this mesh generator automatically, and store it for write to the log file
	//for this export process the only command line parameters needed to automatically export this mesh is the node name, "-node <nodename>"
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

	//try to load an old log file, for defaults and possibly non-interactive export
	std::string shortLogFilename = nodeName.asChar();

	//check for duplicate names (if there are dupes, maya returns a '|' as part of the node name)
	if(shortLogFilename.find("|") != std::string::npos) //lint !e650 !e737, lameness with find() returning an unsigned vs npos's signed
	{
		std::string errorMsg = "duplicate node name ";
		errorMsg += shortLogFilename;
		errorMsg += " found, correct and re-export";
		MESSENGER_LOG_ERROR((errorMsg.c_str()));
		if (interactive)
		{
				IGNORE_RETURN(MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK));
		}
	}

	shortLogFilename += ".log";
	IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));

	//build filename for new logfile as <log base directory>\<nodename>.log
	newLogFilename =  SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	newLogFilename += shortLogFilename;

	//-- Do the MeshGenerator export.
	if (targetNode.apiType() == MFn::kLodGroup)
		exportSuccess = performLodExport(bindPoseFrameNumber, targetNode);
	else
		exportSuccess = performSingleExport(bindPoseFrameNumber, targetNode);

	if(!exportSuccess)
	{
		if (interactive)
			IGNORE_RETURN(MESSENGER_MESSAGE_BOX(NULL, "Local export failed, see output window", "Error!", MB_OK));
	}

	MESSENGER_LOG(("========| STOP: Skeletal Mesh Export |========\n"));

	ExporterLog::setAuthor (authorName.asChar());

	if(exportSuccess)
	{
		//only send to source control if they want to and if the local export succeeded
		if(commitToSourceControl || lock || unlock)
		{
			bool exportToPerforceSucceeded = false;

			bool const setBranch = PerforceImporter::setBranch(branch.asChar());
			if(!setBranch)
				MESSENGER_LOG_ERROR(("[%s] is not a valid branch, NOT submitting to Perforce\n", branch.asChar()));

			//only connect to AB and submit to perforce if this is a stand-alone export (as opposed to part of a SAT export)
			if(independentExport)
			{
// JU_TODO: alienbrain def out
#if 0
				//gather asset database information from alienbrain, and store in the log file
				if (!AlienbrainImporter::connectToServer())
				{
					MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
					return MStatus::kFailure;
				}

				const bool result = AlienbrainImporter::preImport(targetNode.partialPathName().asChar(), interactive);

				if(!ExportManager::validateTextureList(interactive))
				{
					messenger->printWarningsAndErrors();
					return MS::kSuccess;
				}
				
				AlienbrainImporter::storeFileProperties();

				//select assetdb item, if necessary
				IGNORE_RETURN(DatabaseImporter::startSession());
				bool gotAppearance = DatabaseImporter::selectAppearance(interactive, nodeName.asChar());
				IGNORE_RETURN(DatabaseImporter::endSession());
				if(!gotAppearance)
				{
					MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
					messenger->printWarningsAndErrors();
					IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());
					return MS::kSuccess;
				}

				//submit the log file to alienbrain
				IGNORE_RETURN(ExporterLog::writeSkeletalMeshGenerator (newLogFilename.c_str(), interactive));
				if (result)
				{
					if(!lock && !unlock)
						AlienbrainImporter::importLogFile();
				}
				IGNORE_RETURN(AlienbrainImporter::disconnectFromServer());

				if(result)
				{
					exportToPerforceSucceeded = PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock);
				}
#else
				//select assetdb item, if necessary
#if !NO_DATABASE
				IGNORE_RETURN(DatabaseImporter::startSession());
				bool gotAppearance = DatabaseImporter::selectAppearance(interactive, nodeName.asChar());
				IGNORE_RETURN(DatabaseImporter::endSession());
				if(!gotAppearance)
				{
					MESSENGER_LOG_ERROR(("No appearance selected or other AssetDatabase-related error occured, aborting.\n"));
					messenger->printWarningsAndErrors();
					return MS::kSuccess;
				}
#endif

				//submit the log file to alienbrain
				IGNORE_RETURN(ExporterLog::writeSkeletalMeshGenerator (newLogFilename.c_str(), interactive));
				exportToPerforceSucceeded = PerforceImporter::importCommon(interactive, createNewChangelist, lock, unlock);
				
#endif
// JU_TODO: end alienbrain def out
			}
			//otherwise we're part of another (SAT) export, just put the files in the perforce changelist
			else
			{
				exportToPerforceSucceeded = PerforceImporter::importCommon(interactive,false /* createNewChangelist */, lock, unlock);
			}

			PerforceImporter::reset();

			if(exportToPerforceSucceeded)
			{
#if !NO_DATABASE
				//import asset data into Asset Database
				IGNORE_RETURN(DatabaseImporter::startSession());
				if(!DatabaseImporter::importSkeletalMeshData(interactive, lock, unlock) && !interactive)
					IGNORE_RETURN(MESSENGER_MESSAGE_BOX(NULL, "AssetDatabase import failed, see output window", "Error", MB_OK));
				IGNORE_RETURN(DatabaseImporter::endSession());
#endif
			}
		}
		else
		{
			if(independentExport)
			{
				ExporterLog::setAssetGroup("Local export, N/A");
				ExporterLog::setAssetName("Local export, N/A");
				IGNORE_RETURN(ExporterLog::writeSkeletalMeshGenerator (newLogFilename.c_str(), interactive));
			}
		}
	}
	ExporterLog::remove();

	messenger->printWarningsAndErrors();

	exportTimer.stop();
	const int exportTime = static_cast<int>(exportTimer.getElapsedTime());
	const int hours = exportTime / c_secondsPerHour;
	const int minutes = (exportTime - (hours * c_secondsPerHour)) / c_seondsPerMinute;
	const int seconds = exportTime - (hours * c_secondsPerHour) - (minutes * c_seondsPerMinute);
	MESSENGER_LOG(("Total export time: %2ih %2im %2is\n", hours, minutes, seconds));

    std::stringstream text;
    text << "Export Complete: " << nodeName.asChar() << std::endl;
    messenger->getWarningAndErrorText(text);
    text << std::ends;

	MESSENGER_MESSAGE_BOX(NULL,text.str().c_str(),"Export",MB_OK);

	if(showViewerAfterExport && targetNode.apiType() == MFn::kLodGroup)
	{
		MStatus status;
		MFnDagNode fnTargetNode(targetNode, &status);
		const MString meshName = fnTargetNode.name(&status);
		std::stringstream assetPathAndFilename;
		assetPathAndFilename << SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX) << "mesh\\" << meshName.asChar() << s_lodMeshGeneratorTemplateExtensionWithDot;
		ExportManager::LaunchViewer(assetPathAndFilename.str());
	}

	return MStatus(MStatus::kSuccess);
}

// ======================================================================
// class ExportSkeletalMeshGenerator: private static member functions
// ======================================================================

bool ExportSkeletalMeshGenerator::checkShaderForOccludableFaces(const OccludedFaceMapGenerator &occludedFaceMapGenerator, const MIntArray &polyShaderAssignment, int shaderIndex, bool &returnHasOccludableFaces)
{
	const int polygonCount = static_cast<int>(polyShaderAssignment.length());
	MESSENGER_REJECT(polygonCount != occludedFaceMapGenerator.getFaceCount(), ("maya's polygon => shader index map has [%d] entries, occluded face map has [%d] entries\n", polygonCount, occludedFaceMapGenerator.getFaceCount()));
	
	for (int i = 0; i < polygonCount; ++i)
	{
		//-- test if this polygon belongs to the shader we're checking
		if ((shaderIndex == -1) || (polyShaderAssignment[static_cast<unsigned int>(i)] == shaderIndex))
		{
			// this is a poly under control of the shader we're checking

			//-- test if the polygon is occluded by any combination of occlusion groups
			const int occlusionZoneCombination = occludedFaceMapGenerator.getFaceOcclusionZoneCombinationIndex(i);
			if (occlusionZoneCombination >= 0)
			{
				// This poly is occluded.  That's good enough for us to have all polys belonging to this shader group go through as occludable.
				returnHasOccludableFaces = true;
				return true;
			}
		}
	}

	// Success.  If we get here, no faces are occludable.
	returnHasOccludableFaces = false;
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::getZoneNamesMeshOccludes(const MDagPath &meshParentDagPath, stdvector<boost::shared_ptr<CrcLowerString> >::fwd &zoneNamesThisOccludes)
{
	MESSENGER_INDENT;
	MStatus status;

	zoneNamesThisOccludes.clear();

	//-- retrieve "zones this occludes" attribute value
	// get function set
	MFnDagNode  fnDagNode(meshParentDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create MFnDagNode() from [%s]: [%s]\n", meshParentDagPath.partialPathName().asChar(), status.errorString().asChar()));

	// get attribute
	MObject attributeObject = fnDagNode.attribute(ms_zonesThisOccludesAttributeName, &status);
	if (!status)
	{
		MESSENGER_LOG_WARNING(("Mesh parent [%s] has no [%s] attribute specifying zones this occludes\n", meshParentDagPath.partialPathName().asChar(), ms_zonesThisOccludesAttributeName.asChar()));
		return true;
	}

	// get plug for attribute
	MPlug attributePlug = fnDagNode.findPlug(attributeObject, &status);
	if (!status)
	{
		MESSENGER_LOG_WARNING(("Mesh parent [%s] has no [%s] attribute specifying zones this occludes\n", meshParentDagPath.partialPathName().asChar(), ms_zonesThisOccludesAttributeName.asChar()));
		return true;
	}

	// get string value
	MString  mayaValue;

	status = attributePlug.getValue(mayaValue);
	MESSENGER_REJECT(!status, ("failed to get string value for occlusion plug: [%s]\n", status.errorString().asChar()));

	//-- parse attribute value (comma separated string)
	MStringArray  mayaZoneNames;

	status = mayaValue.split(',', mayaZoneNames);
	MESSENGER_REJECT(!status, ("split() failed: [%s]\n", status.errorString().asChar()));

	//-- build "zones this occludes" return value
	const unsigned int nameCount = mayaZoneNames.length();

	zoneNamesThisOccludes.reserve(static_cast<size_t>(nameCount));

	for (unsigned int i = 0; i < nameCount; ++i)
	{
		// -TRF- handle whitespace between commas
		boost::shared_ptr<CrcLowerString> newZoneName(new CrcLowerString(mayaZoneNames[i].asChar()));
		zoneNamesThisOccludes.push_back(newZoneName);
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::getMeshLayer(const MDagPath &meshParentDagPath, int &layer)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- retrieve occlusion layer attribute value
	// get function set
	MFnDagNode  fnDagNode(meshParentDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create MFnDagNode() from [%s]: [%s]\n", meshParentDagPath.partialPathName().asChar(), status.errorString().asChar()));

	// get attribute
	MObject attributeObject = fnDagNode.attribute(ms_occlusionLayerAttributeName, &status);
	if (!status)
	{
		MESSENGER_LOG_WARNING(("Mesh parent [%s] has no [%s] node specifying occlusion layer\n", meshParentDagPath.partialPathName().asChar(), ms_occlusionLayerAttributeName.asChar()));
		return true;
	}

	// get plug for attribute
	MPlug attributePlug = fnDagNode.findPlug(attributeObject, &status);
	if (!status)
	{
		MESSENGER_LOG_WARNING(("Mesh parent [%s] has no [%s] node specifying occlusion layer\n", meshParentDagPath.partialPathName().asChar(), ms_occlusionLayerAttributeName.asChar()));
		return true;
	}

	// get value
	int mayaValue = -1;

	status = attributePlug.getValue(mayaValue);
	MESSENGER_REJECT(!status, ("failed to get integer value for occlusion layer plug: [%s]\n", status.errorString().asChar()));

	layer = mayaValue;

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::exportOcclusionData(SkeletalMeshGeneratorWriter &writer, const OccludedFaceMapGenerator &occludedFaceMapGenerator, const MDagPath &meshParentDagPath)
{
	//-- build unique name list of occlusion zones that both (1) potentially occlude
	//   this mesh and (2) this mesh occludes
	UniqueNameList  occlusionZoneNames;

	// first add occlusion zone map items, since occlusion zone combination indices depend on those oz names
	// being in that order.
	{
		const int ozCount = occludedFaceMapGenerator.getOcclusionZoneCount();
		for (int i = 0; i < ozCount; ++i)
		{
			const CrcLowerString &ozName = occludedFaceMapGenerator.getOcclusionZoneName(i);

			const int uniqueIndex = occlusionZoneNames.submitName(boost::shared_ptr<CrcLowerString>(new CrcLowerString(ozName)));
			DEBUG_FATAL(uniqueIndex != i, ("adding occlusion zone names in order, but oz index [%d] != unique name index [%d]\n", i, uniqueIndex));
			UNREF(uniqueIndex); //needed for release build
		}
	}

	// now find occlusion zones this mesh can occlude, add these names
	std::vector<int>  zonesThisOccludes;
	{
		//-- get the zone names
		typedef std::vector<boost::shared_ptr<CrcLowerString> >  SharedCrcLowerStringVector;
		SharedCrcLowerStringVector                               zoneNamesThisOccludes;

		const bool gznmoSuccess = getZoneNamesMeshOccludes(meshParentDagPath, zoneNamesThisOccludes);
		MESSENGER_REJECT(!gznmoSuccess, ("getZoneNamesMeshOccludes() failed\n"));

		//-- add names to unique occlusion zone names, retrieve occlusion zone index for them
		zonesThisOccludes.reserve(zoneNamesThisOccludes.size());

		const SharedCrcLowerStringVector::const_iterator itEnd = zoneNamesThisOccludes.end();
		for (SharedCrcLowerStringVector::const_iterator it = zoneNamesThisOccludes.begin(); it != itEnd; ++it)
		{
			const int zoneIndex = occlusionZoneNames.submitName(*it);
			zonesThisOccludes.push_back(zoneIndex);
		}
	}

	//-- submit occlusion zone names to writer for export
	{
		const int ozCount = occlusionZoneNames.getUniqueNameCount();
		for (int i = 0; i < ozCount; ++i)
			writer.addOcclusionZone(occlusionZoneNames.getName(i).getString());
	}

	//-- submit fully occluded zone combination to writer
	{
		//--  build it
		std::vector<int>  fullyOccludedZoneCombination;
		occludedFaceMapGenerator.constructFullyOccludedZoneCombination(fullyOccludedZoneCombination);

		//-- submit it
		if (!fullyOccludedZoneCombination.empty())
			writer.setFullyOccludedZoneCombination(fullyOccludedZoneCombination);
	}

	//-- submit occlusion zone combinations to writer
	{
		const int ozcCount = occludedFaceMapGenerator.getOcclusionZoneCombinationCount();
		for (int i = 0; i < ozcCount; ++i)
		{
			//-- get it
			const std::vector<int> &occlusionZoneCombination = occludedFaceMapGenerator.getOcclusionZoneCombination(i);

			//-- submit it
			writer.addOcclusionZoneCombination(occlusionZoneCombination);
		}
	}

	//-- submit zones this mesh occludes to writer
	writer.setZonesThisOccludes(zonesThisOccludes);

	//-- export layer for this mesh generator.  lower-valued layers are closer to the center.
	int layer = -1;

	const bool gmlSuccess = getMeshLayer(meshParentDagPath, layer);
	MESSENGER_REJECT(!gmlSuccess, ("getMeshLayer() failed\n"));

	writer.setLayer(layer);

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::addMeshPositions(const MFloatPointArray &mayaPositions, const MayaMeshWeighting &meshWeighting, SkeletalMeshGeneratorWriter *writer)
{
	NOT_NULL(writer);
	MESSENGER_INDENT;

	typedef std::multimap<float, int, std::greater<float> >  FloatIntMap;

	MStatus      status;
	FloatIntMap  weightPositionMap;

	// verify we found weighting information for all mesh positions	
	int positionCount = 0;
	const bool gapcSuccess = meshWeighting.getAffectedPositionCount(&positionCount);
	MESSENGER_REJECT(!gapcSuccess, ("meshWeighting.getAffectedPositionCount() failed\n"));

	const unsigned mayaPositionCount = mayaPositions.length();
	MESSENGER_REJECT(mayaPositionCount != static_cast<unsigned>(positionCount), ("maya mesh point retrieval returned different # positions than weight retrieval [%u/%d]", mayaPositionCount, positionCount));

	// add position and weighting info to the writer
	for (unsigned positionIndex = 0; positionIndex < mayaPositionCount; ++positionIndex)
	{
		// convert maya position to game position
		// -TRF- I'm assuming the points are cartesianized.
		const MFloatPoint &mayaPosition = mayaPositions[positionIndex];
		const Vector       gamePosition = MayaConversions::convertPoint(mayaPosition);

		// add position to writer
		writer->addPosition(gamePosition);

		// add position weights to writer
		const MayaMeshWeighting::VertexData *vertexData = 0;

		const bool gvdSuccess = meshWeighting.getVertexData(static_cast<int>(positionIndex), &vertexData);
		MESSENGER_REJECT(!gvdSuccess, ("failed to get weighting vertex data for position [%u]\n", positionIndex));

		//-- Sort affecting transforms by weight.  The hard skinning code in the engine needs
		//   the first affecting transform to be the most significant influence.
		weightPositionMap.clear();

		int  affectorCount;
		int  transformIndex;
		real transformWeight;
		float totalWeight = 0.0f;

		const bool gvdacSuccess = meshWeighting.getVertexDataAffectorCount(vertexData, &affectorCount);
		MESSENGER_REJECT(!gvdacSuccess, ("meshWeighting.getVertexDataAffectorCount() failed\n"));

		for (int affectorIndex = 0; affectorIndex < affectorCount; ++affectorIndex)
		{
			// Get the transform index and weight data.
			const bool gvdadSuccess = meshWeighting.getVertexDataTransformWeightData(vertexData, affectorIndex, &transformIndex, &transformWeight);
			MESSENGER_REJECT(!gvdadSuccess, ("failed to get vertex' transform weight data [%u/%d]\n", positionIndex, affectorIndex));

			// Add it to the map.
			IGNORE_RETURN(weightPositionMap.insert(FloatIntMap::value_type(transformWeight, transformIndex)));

			// Keep track of total weight so we can normalize weighting to a total factor of 1.0.
			// Failure to do so could result in erroneous position computation during rendering, particularly
			// if there are many small weights that get removed by the low-weight filter.
			totalWeight += transformWeight;
		}

		//-- Compute normalization factor.
		float const oneOverTotalWeight = (totalWeight > 0.0f) ? (1.0f / totalWeight) : 0.0f;

		//-- Add this vertex's weights in order from most significant weight.
		const FloatIntMap::iterator endIt = weightPositionMap.end();
		for (FloatIntMap::iterator it = weightPositionMap.begin(); it != endIt; ++it)
		{
			// Add it to writer.
			float const normalizedWeight = it->first * oneOverTotalWeight;
			writer->addPositionWeight(static_cast<int>(positionIndex), it->second, normalizedWeight);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::addMeshNormals(const MFloatVectorArray &mayaNormals, SkeletalMeshGeneratorWriter *writer)
{
	// add normal info to the writer
	const unsigned normalCount = mayaNormals.length();
	for (unsigned normalIndex = 0; normalIndex < normalCount; ++normalIndex)
	{
		// convert maya normal to game normal
		const MFloatVector &mayaNormal = mayaNormals[normalIndex];
		const Vector        gameNormal = MayaConversions::convertVector(mayaNormal);

		// add position to writer
		writer->addNormal(gameNormal);
	}

	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::addMeshDot3Values(const MayaPerPixelLighting::FloatVector &dot3Values, SkeletalMeshGeneratorWriter *writer)
{
	NOT_NULL(writer);

	if (dot3Values.empty())
		return true;

	//-- Ensure there are a multiple of 4 dot3 values.
	const int dot3ValuesLength = static_cast<int>(dot3Values.size());
	const int dot3ValueCount   = dot3ValuesLength / 4;

	MESSENGER_REJECT((dot3ValueCount * 4) != dot3ValuesLength, ("logic error: the dot3Values array does not contain a multiple of 4 entries [%d].", dot3ValuesLength));

	// Add dot3 info to writer.
	for (int i = 0; i < dot3ValueCount; ++i)
	{
		writer->addDot3Value(
			dot3Values[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 0)],
			dot3Values[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 1)],
			dot3Values[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 2)],
			dot3Values[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 3)]);
	}

	return true;
}

// ----------------------------------------------------------------------

void ExportSkeletalMeshGenerator::removeShapeSuffix(MString &name)
{
	const int mayaNameLength = static_cast<int>(name.length());
	if (mayaNameLength > 5)
	{
		MString suffix = name.substring(mayaNameLength - 5, mayaNameLength - 1);
		IGNORE_RETURN(suffix.toLowerCase());

		if (suffix == "shape")
			name = name.substring(0, mayaNameLength - 6);
	}
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::addTransformsToWriter(const MayaMeshWeighting &meshWeighting, SkeletalMeshGeneratorWriter *writer, MStringArray *skeletonTemplateNodeNames)
{
	NOT_NULL(skeletonTemplateNodeNames);
	MESSENGER_INDENT;

	int transformCount;

	std::set<std::string> skeletonTemplateNames;

	const bool gatcSuccess = meshWeighting.getAffectingTransformCount(&transformCount);
	MESSENGER_REJECT(!gatcSuccess, ("meshWeighting.getAffectingTransformCount() failed\n"));

	MString             mayaTransformName;
	MString             searchSkeletonTemplateNameComponent;
	MayaCompoundString  compoundName;
	MDagPath            affectorDagPath;
	MDagPath            skeletonStartPath;

	for (int transformIndex = 0; transformIndex < transformCount; ++transformIndex)
	{
		// get transform name
		const bool gtnSuccess = meshWeighting.getAffectingTransform(transformIndex, &mayaTransformName, &affectorDagPath);
		MESSENGER_REJECT(!gtnSuccess, ("meshWeighting.getTransformName() failed\n"));

		// get skeleton template name for this transform (if present)
		bool foundSkeletonTemplateName = false;

		const bool fawncSuccess = MayaUtility::findAncestorWithNameComponent(affectorDagPath, ms_skeletonTemplateNameComponentIndex, &searchSkeletonTemplateNameComponent, &foundSkeletonTemplateName, 0, &skeletonStartPath);
		MESSENGER_REJECT(!fawncSuccess, ("MayaUtility::findAncestorWithNameComponent() failed on [%s]\n", affectorDagPath.partialPathName().asChar()));

		// add skeleton template name to set of skeleton templates referenced by this mesh
		if (foundSkeletonTemplateName)
		{
			const std::string insertionString = searchSkeletonTemplateNameComponent.asChar();
			std::pair<std::set<std::string>::iterator, bool> result = skeletonTemplateNames.insert(insertionString);
			if (result.second)
			{
				// new skeleton name, add node to array of skeleton template objects
				IGNORE_RETURN(skeletonTemplateNodeNames->append(skeletonStartPath.partialPathName()));
			}
		}

		// get pure transform name
		compoundName.setCompoundString(mayaTransformName);
		const MString gameTransformName = compoundName.getComponentString(0);

		// set it for the writer
		const int newTransformIndex = writer->addTransformName(gameTransformName.asChar());
		MESSENGER_REJECT(newTransformIndex != transformIndex, ("newTransformIndex != transformIndex [%d/%d]\n", newTransformIndex, transformIndex));
	}

	// -- add skeleton template names
	MESSENGER_LOG(("the following [%u] skeleton templates were referenced:\n", skeletonTemplateNames.size()));

	std::set<std::string>::iterator       it    = skeletonTemplateNames.begin();
	std::set<std::string>::const_iterator itEnd = skeletonTemplateNames.end();
	for (; it != itEnd; ++it)
	{
		//-- build the skeleton template name from the partial filename embedded in the skeleton template's root joint name
		std::string skeletonTemplateName = SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX);
		IGNORE_RETURN(skeletonTemplateName.append(*it));
		IGNORE_RETURN(skeletonTemplateName.append(".skt"));

		//-- add skeleton template name to writer
		writer->addReferencedSkeletonTemplateName(skeletonTemplateName.c_str());
		MESSENGER_LOG(("  [%s]\n", skeletonTemplateName.c_str()));
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::captureBlendDeltas(const MString &gameBlendTargetName, const MFloatPointArray &mayaUnblendedPositions, const MFloatPointArray &mayaBlendedPositions, const MFloatVectorArray &mayaUnblendedNormals, const MFloatVectorArray &mayaBlendedNormals, const MayaPerPixelLighting::Dot3DifferenceVector &dot3DifferenceVector, SkeletalMeshGeneratorWriter *writer)
{
	MESSENGER_INDENT;

	SkeletalMeshGeneratorWriter::BlendTarget *blendTarget = 0;

	//-- validate we have arrays of same size
	const unsigned unblendedPositionCount = mayaUnblendedPositions.length();
	const unsigned blendedPositionCount = mayaBlendedPositions.length();
	MESSENGER_REJECT(unblendedPositionCount != blendedPositionCount, ("unsupported: unblended [%u] and blended [%u] array sizes differ\n", unblendedPositionCount, blendedPositionCount));

	const unsigned unblendedNormalCount = mayaUnblendedNormals.length();
	const unsigned blendedNormalCount = mayaBlendedNormals.length();
	MESSENGER_REJECT(unblendedNormalCount != blendedNormalCount, ("unsupported: unblended [%u] and blended [%u] array sizes differ\n", unblendedNormalCount, blendedNormalCount));

	//-- track largest unapplied delta for reporting.
	float largestUnappliedPositionalDelta = 0.0f;
	float largestUnappliedNormalDelta     = 0.0f;

	//-- diff positions
	int writtenPositionDeltaCount = 0;
	{
		for (unsigned i = 0; i < unblendedPositionCount; ++i)
		{
			const MFloatVector deltaPosition = mayaBlendedPositions[i] - mayaUnblendedPositions[i];
			if (
				(deltaPosition.x >= ms_blendPositionEpsilon) || (deltaPosition.x <= -ms_blendPositionEpsilon) || 
				(deltaPosition.y >= ms_blendPositionEpsilon) || (deltaPosition.y <= -ms_blendPositionEpsilon) || 
				(deltaPosition.z >= ms_blendPositionEpsilon) || (deltaPosition.z <= -ms_blendPositionEpsilon))
			{
				// this target point has changed enough to make it worthwhile exporting

				//-- create the blend target if not yet constructed.
				//   this prevents us from creating a blend target if there are no significant deltas.
				if (!blendTarget)
					blendTarget = NON_NULL(writer->addBlendTarget(gameBlendTargetName.asChar()));

				// output the delta
				const Vector gameDeltaPosition = MayaConversions::convertVector(deltaPosition);
				writer->addBlendTargetPosition(blendTarget, static_cast<int>(i), gameDeltaPosition);
				++writtenPositionDeltaCount;
			}
			else
			{
				largestUnappliedPositionalDelta = std::max(largestUnappliedPositionalDelta, abs(deltaPosition.x));
				largestUnappliedPositionalDelta = std::max(largestUnappliedPositionalDelta, abs(deltaPosition.y));
				largestUnappliedPositionalDelta = std::max(largestUnappliedPositionalDelta, abs(deltaPosition.z));
			}
		}
	}

	//-- diff target position from source position
	int writtenNormalDeltaCount = 0;
	{
		for (unsigned i = 0; i < unblendedNormalCount; ++i)
		{
			const MFloatVector deltaNormal = mayaBlendedNormals[i] - mayaUnblendedNormals[i];
			if (
				(deltaNormal.x >= ms_blendNormalEpsilon) || (deltaNormal.x <= -ms_blendNormalEpsilon) || 
				(deltaNormal.y >= ms_blendNormalEpsilon) || (deltaNormal.y <= -ms_blendNormalEpsilon) || 
				(deltaNormal.z >= ms_blendNormalEpsilon) || (deltaNormal.z <= -ms_blendNormalEpsilon))
			{
				// this target point has changed enough to make it worthwhile exporting

				//-- create the blend target if not yet constructed.
				//   this prevents us from creating a blend target if there are no significant deltas.
				if (!blendTarget)
					blendTarget = NON_NULL(writer->addBlendTarget(gameBlendTargetName.asChar()));

				// output the delta
				const Vector gameDeltaNormal = MayaConversions::convertVector(deltaNormal);
				writer->addBlendTargetNormal(blendTarget, static_cast<int>(i), gameDeltaNormal);
				++writtenNormalDeltaCount;
			}
			else
			{
				largestUnappliedNormalDelta = std::max(largestUnappliedNormalDelta, abs(deltaNormal.x));
				largestUnappliedNormalDelta = std::max(largestUnappliedNormalDelta, abs(deltaNormal.y));
				largestUnappliedNormalDelta = std::max(largestUnappliedNormalDelta, abs(deltaNormal.z));
			}
		}
	}

	//-- Write dot3 delta data.
	{
		const MayaPerPixelLighting::Dot3DifferenceVector::const_iterator endIt = dot3DifferenceVector.end();
		for (MayaPerPixelLighting::Dot3DifferenceVector::const_iterator it = dot3DifferenceVector.begin(); it != endIt; ++it)
		{
				const MayaPerPixelLighting::Dot3Difference &difference = *it;

				// Create the blend target if not yet constructed.
				if (!blendTarget)
					blendTarget = NON_NULL(writer->addBlendTarget(gameBlendTargetName.asChar()));

				// Add the blend target dot3.
				if (difference.getDeltaW() != 0.0f)
				{
					static int throwOutCount = 0;
					MESSENGER_LOG(("throwing out dot3 blend difference with different dot3 facing direction [%d]\n.", ++throwOutCount));
				}
				else
					writer->addBlendTargetDot3Value(blendTarget, difference.getIndex(), difference.getDeltaX(), difference.getDeltaY(), difference.getDeltaZ(), difference.getDeltaW());
		}
	}

	//-- announce this blend target
	if (blendTarget)
	{
		MESSENGER_LOG(("blend target: [%s], positions [%d of %u], normals [%d of %u], dot3 values [%d]\n", gameBlendTargetName.asChar(), writtenPositionDeltaCount, unblendedPositionCount, writtenNormalDeltaCount, unblendedNormalCount, static_cast<int>(dot3DifferenceVector.size())));
	}
	else
	{
		MESSENGER_LOG(("blend target: [%s] ignored, no significant positional or normal changes\n", gameBlendTargetName.asChar()));
		MESSENGER_LOG(("blend target: [%s] largest unapplied delta: position [%g];normal [%g]\n", gameBlendTargetName.asChar(), largestUnappliedPositionalDelta, largestUnappliedNormalDelta));
		MESSENGER_LOG(("---\n"));
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::setSkeletonsToBindPose(const MStringArray &skeletonTemplateNodeNames, int alternateBindPoseFrameNumber)
{
	MESSENGER_INDENT;
	MStatus status;

	const unsigned skeletonTemplateCount = skeletonTemplateNodeNames.length();
	if (!skeletonTemplateCount)
		return true;

	//-- save current selection
	MSelectionList initialSelectionList;

	status = MGlobal::getActiveSelectionList(initialSelectionList);
	MESSENGER_REJECT(!status, ("failed to get active selection list\n"));

	//-- add each referenced skeleton to the active selection, then go to bind pose
	MSelectionList skeletonSelectionList;

		//-- use select -r "node name" to select the specified skeleton

#if 1
	MString executeCommand;

	for (unsigned i = 0; i < skeletonTemplateCount; ++i)
	{
		const MString &nodeName = skeletonTemplateNodeNames[i];

		MESSENGER_LOG(("setting skeleton [%s] to bind pose\n", nodeName.asChar()));
		executeCommand = "select -r " + nodeName;

		status = MGlobal::executeCommand(executeCommand, true, true);
		MESSENGER_REJECT(!status, ("failed to execute selection command\n"));

		// set the maya frame number to the bind pose frame
		const bool gtbpSuccess = MayaUtility::goToBindPose(alternateBindPoseFrameNumber);
		MESSENGER_REJECT(!gtbpSuccess, ("failed to go to bind pose\n"));
	}
#else
	for (unsigned i = 0; i < skeletonTemplateCount; ++i)
	{
		status = skeletonSelectionList.add(skeletonTemplateObjects[i]);
		MESSENGER_REJECT(!status, ("failed to add skeleton object to selection list\n"));
	}

	// set skeleton selection list
	status = MGlobal::setActiveSelectionList(skeletonSelectionList);
	MESSENGER_REJECT(!status, ("failed to set active selection list\n"));

	// set the maya frame number to the bind pose frame
	const bool gtbpSuccess = MayaUtility::goToBindPose(alternateBindPoseFrameNumber);
	MESSENGER_REJECT(!gtbpSuccess, ("failed to go to bind pose\n"));
#endif

	//-- restore original selection
	status = MGlobal::setActiveSelectionList(initialSelectionList);
	MESSENGER_REJECT(!status, ("failed to set active selection list\n"));

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::getFaceSets(const MDagPath &meshDagPath, MObjectVector &faceSets)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- retrieve a list of all selection sets from the scene
	MItDependencyNodes  setIterator(MFn::kSet, &status);
	MESSENGER_REJECT(!status, ("MItDependencyNodes() failed [%s]\n", status.errorString().asChar()));

	bool done = setIterator.isDone(&status);
	MESSENGER_REJECT(!status, ("isDone() failed [%s]\n", status.errorString().asChar()));

	MFnDependencyNode  fnDependencyNode;
	int                setCount = 0;

	while (!done)
	{
		//-- get object
		MObject object = setIterator.item(&status);
		MESSENGER_REJECT(!status, ("item() failed [%s]\n", status.errorString().asChar()));

		//-- check if it is an occluded faces set (starts with OF__)
		status = fnDependencyNode.setObject(object);
		MESSENGER_REJECT(!status, ("failed to set MFnDependencyNode() [%s]\n", status.errorString().asChar()));

		const MString mayaName = fnDependencyNode.name(&status);
		MESSENGER_REJECT(!status, ("name() failed [%s]\n", status.errorString().asChar()));

		const MayaCompoundString  compoundName(mayaName);
		if ((compoundName.getComponentCount() > 1) && (compoundName.getComponentString(0) == ms_occludedFaceZonePrefix))
		{
			// we've got an occluded face set

			//-- add to list
			faceSets.push_back(object);
			++setCount;
		}

		//-- increment iterator
		status = setIterator.next();
		MESSENGER_REJECT(!status, ("next() failed [%s]\n", status.errorString().asChar()));

		done = setIterator.isDone(&status);
		MESSENGER_REJECT(!status, ("isDone() failed [%s]\n", status.errorString().asChar()));
	}

	MESSENGER_LOG(("Found %u selection sets that affect mesh [%s]\n", setCount, meshDagPath.partialPathName().asChar()));

	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::collectHardpoints(const MString &hardpointPrefix, const MString &meshName, HardpointInfoMap &hardpointInfoMap, int lodIndex)
{
	MStatus  status;
	bool     done;

	//-- initialize the iterator
	MItDag  dagIterator(MItDag::kDepthFirst, MFn::kTransform, &status);

	done = dagIterator.isDone(&status);
	MESSENGER_REJECT(!status, ("transform interation failed [%s]\n", status.errorString().asChar()));

	MFnTransform  fnTransform;

	// in this loop, we'll identify and collect hardpoint objects.  We'll store them in a map for
	// later processing.  We defer processing until later because we need the base hardpoint 
	// before we process the corresponding morph-target hardpoints.
	// the morph target hardpoint is stored as delta information from the base hardpoint.
	while (!done)
	{
		//-- setup MFnTransform
		MObject mayaObject = dagIterator.item(&status);
		MESSENGER_REJECT(!status, ("failed to get maya object [%s]\n", status.errorString().asChar()));

		status = fnTransform.setObject(mayaObject);
		MESSENGER_REJECT(!status, ("failed to set maya object into MFnTransform [%s]\n", status.errorString().asChar()));

		//-- check node name to see if it is one of our hardpoints
		MString mayaNodeName = fnTransform.name(&status);
		MESSENGER_REJECT(!status, ("failed to get maya node name [%s]\n", status.errorString().asChar()));

		IGNORE_RETURN(mayaNodeName.toLowerCase());

		//-- Check if this is an LOD node for an LOD that is not our own.  If so, prune traversal of this tree.
		if (lodIndex >= 0)
		{
			// NOTE: this only handles LODs labeled from l0 through l9.  If we ever have an l10, this will break.
			if ((mayaNodeName.length() == 2) && (mayaNodeName.asChar()[0] == 'l') && ((mayaNodeName.asChar()[1] - '0') != lodIndex))
			{
				status = dagIterator.prune();
				MESSENGER_REJECT(!status, ("MItDag::prune() failed for Mesh lod [%d] when traversing lod [%s],error=[%s].\n", lodIndex, mayaNodeName.asChar(), status.errorString().asChar()));
			}
		}

		MayaCompoundString compoundNodeName(mayaNodeName);

		const bool isMeshHardpoint = (compoundNodeName.getComponentCount() > 2) && (compoundNodeName.getComponentString(0) == hardpointPrefix) && (compoundNodeName.getComponentString(1) == meshName);
		if (isMeshHardpoint)
		{
			//-- find or create HardpointInfo for this hardpoint
			const CrcLowerString  crcHardpointName(compoundNodeName.getComponentString(2).asChar());
			boost::shared_ptr<HardpointInfo>  hardpointInfo;

			// find the hardpoint info node if it exists, or create a new one
			const HardpointInfoMap::iterator findIt = hardpointInfoMap.lower_bound(&crcHardpointName);
			if ((findIt != hardpointInfoMap.end()) && !hardpointInfoMap.key_comp()(&crcHardpointName, findIt->first))
			{
				// hardpoint info already exists
				hardpointInfo = findIt->second;

				// sanity check
				DEBUG_FATAL(hardpointInfo->m_hardpointName != crcHardpointName, ("hardpoint names not equal\n"));
			}
			else
			{
				// hardpoint info doesn't exist, create it
				hardpointInfo.reset(new HardpointInfo(crcHardpointName));

				// add to map
				IGNORE_RETURN(hardpointInfoMap.insert(findIt, HardpointInfoMap::value_type(&hardpointInfo->m_hardpointName, hardpointInfo)));
			}

			NOT_NULL(hardpointInfo.get());

			//-- check if it's a dynamic (morph target) hardpoint
			const bool isHardpointBlendTarget = (compoundNodeName.getComponentCount() > 3);
			if (!isHardpointBlendTarget)
			{
				// this is either a static hardpoint or the base of a dynamic hardpoint
				MESSENGER_REJECT(!hardpointInfo->m_baseTransformObject.isNull(), ("base transform object already set for hardpoint [%s]\n", crcHardpointName.getString()));

				hardpointInfo->m_baseTransformObject = mayaObject;
			}
			else
			{
				// this is a dynamic hardpoint blend target.

				// build the blend target name.  if it ends in Shape, chop that off.
				MString mayaBlendShapeName = compoundNodeName.getComponentString(3);
				removeShapeSuffix(mayaBlendShapeName);

				const CrcLowerString crcBlendTargetName(mayaBlendShapeName.asChar());

				// make sure the blend target doesn't already exist
				HardpointInfo::HardpointBlendTargetMap::iterator btFindIt = hardpointInfo->m_hardpointBlendTargets.lower_bound(&crcBlendTargetName);
				if ((btFindIt != hardpointInfo->m_hardpointBlendTargets.end()) && !hardpointInfo->m_hardpointBlendTargets.key_comp()(&crcBlendTargetName, btFindIt->first))
				{
					// hardpoint blend target already exists
					MESSENGER_LOG_ERROR(("blend target [%s] tried to assign multiple hardpoints for hardpoint [%s]\n", crcBlendTargetName.getString(), crcHardpointName.getString()));
					return false;
				}
				else
				{
					// hardpoint blend target does not exist, add it
					boost::shared_ptr<HardpointInfo::HardpointBlendTarget> blendTarget(new HardpointInfo::HardpointBlendTarget(crcBlendTargetName, mayaObject));
					IGNORE_RETURN(hardpointInfo->m_hardpointBlendTargets.insert(btFindIt, HardpointInfo::HardpointBlendTargetMap::value_type(&blendTarget->m_blendShapeName, blendTarget)));
				}
			}
		}

		//-- increment loop
		status = dagIterator.next();
		MESSENGER_REJECT(!status, ("failed to increment iterator\n"));

		done = dagIterator.isDone(&status);
		MESSENGER_REJECT(!status, ("transform interation failed [%s]\n", status.errorString().asChar()));
	}

	// no errors
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::extractHardpointTransform(const MObject &transformObject, Vector &translation, Quaternion &rotation, std::string *parentTransformName)
{
	MStatus status;

	MFnTransform  fnTransform(transformObject, &status);
	MESSENGER_REJECT(!status, ("failed to set fnTransform with object [%s]\n", status.errorString().asChar()));

	//-- extract translation
	MVector mayaTranslation = fnTransform.translation(MSpace::kTransform, &status);
	MESSENGER_REJECT(!status, ("failed to get joint translation\n"));

	// convert to game translation
	translation = MayaConversions::convertVector(mayaTranslation);

	//-- extract rotation

	// extract hardpoint pre rotation
	const MQuaternion mayaRotateAxisQuaternion = fnTransform.rotateOrientation(MSpace::kTransform, &status);
	MESSENGER_REJECT(!status, ("failed to get rotateOrientation quaternion\n"));

	const MEulerRotation  mayaRotateAxisEuler = mayaRotateAxisQuaternion.asEulerRotation();
	const Quaternion      preRotation         = MayaConversions::convertRotation(mayaRotateAxisEuler);

	// extract hardpoint rotation
	MEulerRotation mayaHardpointRotation;
	status = fnTransform.getRotation(mayaHardpointRotation);
	MESSENGER_REJECT(!status, ("failed to get hardpoint rotation\n"));
	
	const Quaternion hardpointRotation = MayaConversions::convertRotation(mayaHardpointRotation);

	// combine pre and rotate
	rotation = hardpointRotation * preRotation;

	//-- extract parent
	if (parentTransformName)
	{
		MObject mayaParentObject = fnTransform.parent(0, &status);
		MESSENGER_REJECT(!status, ("failed to get transform parent hardpoint node [%s]\n", status.errorString().asChar()));

		MFnDagNode fnParentDagNode(mayaParentObject, &status);
		MESSENGER_REJECT(!status, ("failed to set MFnDagNode for parent hardpoint object [%s]\n", status.errorString().asChar()));

		// we don't handle parenting a hardpoint to an ignored node
		MDagPath parentDagPath;
		
		status = fnParentDagNode.getPath(parentDagPath);
		MESSENGER_REJECT(!status, ("failed to get dag path for hardpoint parent node [%s]\n", status.errorString().asChar()));
		MESSENGER_REJECT(MayaUtility::ignoreNode(parentDagPath), ("we don't handle parenting a hardpoint to an ignored skeleton node\n"));

		// retrieve the name
		MayaCompoundString  parentCompoundName(fnParentDagNode.name(&status));
		MESSENGER_REJECT(!status, ("failed to get node name for hardpoint parent node [%s]\n", status.errorString().asChar()));

		IGNORE_RETURN(parentTransformName->assign(parentCompoundName.getComponentString(0).asChar()));
	}

	// no errors
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::processHardpointInfo(const HardpointInfoMap &hardpointInfoMap, SkeletalMeshGeneratorWriter &writer, bool printDebugInfo, int lodIndex)
{
	Vector       baseTranslation;
	Quaternion   baseRotation;
	std::string  hardpointParentName;

	Vector       targetTranslation;
	Quaternion   targetRotation;

	if (printDebugInfo)
		MESSENGER_LOG(("Hardpoints:\n"));

	if (hardpointInfoMap.empty())
		if (printDebugInfo)
			MESSENGER_LOG(("-- no hardpoints.\n"));

	const HardpointInfoMap::const_iterator endIt = hardpointInfoMap.end();
	for (HardpointInfoMap::const_iterator it = hardpointInfoMap.begin(); it != endIt; ++it)
	{
		MESSENGER_INDENT;

		// get the hardpoint info
		NOT_NULL(it->second.get());
		const HardpointInfo &hardpointInfo = *(it->second);

		// extract baseRotation and baseTranslation
		const bool baseExtractSuccess = extractHardpointTransform(hardpointInfo.m_baseTransformObject, baseTranslation, baseRotation, &hardpointParentName);
		MESSENGER_REJECT(!baseExtractSuccess, ("failed to extract hardpoint transform for [%s]\n", hardpointInfo.m_hardpointName.getString()));

		// add base hardpoint
		const bool isStaticHardpoint = hardpointInfo.m_hardpointBlendTargets.empty();

		if (isStaticHardpoint)
		{
			// add the hardpoint to the writer
			IGNORE_RETURN(writer.addStaticHardpoint(hardpointInfo.m_hardpointName.getString(), hardpointParentName, baseTranslation, baseRotation));

			if (printDebugInfo)
				MESSENGER_LOG(("static hardpoint [%s] attached to [%s]\n", hardpointInfo.m_hardpointName.getString(), hardpointParentName.c_str()));
		}
		else
		{
			// add the hardpoint to the writer
			const int hardpointIndex = writer.addDynamicHardpoint(hardpointInfo.m_hardpointName.getString(), hardpointParentName, baseTranslation, baseRotation);

			if (printDebugInfo)
				MESSENGER_LOG(("dynamic hardpoint [%s] attached to [%s]\n", hardpointInfo.m_hardpointName.getString(), hardpointParentName.c_str()));

			Quaternion const negativeBaseRotation = baseRotation.getComplexConjugate();

			// add hardpoint blend targets
			const HardpointInfo::HardpointBlendTargetMap::const_iterator btEndIt = hardpointInfo.m_hardpointBlendTargets.end();
			for (HardpointInfo::HardpointBlendTargetMap::const_iterator btIt = hardpointInfo.m_hardpointBlendTargets.begin(); btIt != btEndIt; ++btIt)
			{
				MESSENGER_INDENT;

				NOT_NULL(btIt->second.get());
				const HardpointInfo::HardpointBlendTarget &hardpointBt = *(btIt->second);

				SkeletalMeshGeneratorWriter::BlendTarget *const blendTarget = writer.findBlendTarget(hardpointBt.m_blendShapeName.getString());
				if (!blendTarget)
				{
					MESSENGER_LOG(("-- found hardpoint for blend shape variable [%s], lod [%d], but blend shape doesn't exist, ignoring.\n", hardpointBt.m_blendShapeName.getString(), lodIndex));
					continue;
				}

				if (printDebugInfo)
					MESSENGER_LOG(("-- hardpoint affected by blend target [%s]\n", hardpointBt.m_blendShapeName.getString()));

				// extract the hardpoint data for the blend target
				const bool btExtractSuccess = extractHardpointTransform(hardpointBt.m_transformObject, targetTranslation, targetRotation, NULL);
				MESSENGER_REJECT(!btExtractSuccess, ("failed to extract hardpoint transform for [%s], blend variable [%s]\n", hardpointInfo.m_hardpointName.getString(), hardpointBt.m_blendShapeName.getString()));

				// build the delta data
				const Vector      deltaPosition = targetTranslation - baseTranslation;
				const Quaternion  deltaRotation = targetRotation * negativeBaseRotation;

				writer.addBlendTargetHardpointTarget(blendTarget, hardpointIndex, deltaPosition, deltaRotation);
			}
		}
	}

	// no errors
	return true;
}

// ----------------------------------------------------------------------
/**
 * Collect hardpoint data and submit for export.
 *
 * @return true on success, false on non-fatal error.
 */

bool ExportSkeletalMeshGenerator::exportHardpoints(const char *meshName, SkeletalMeshGeneratorWriter &writer, int lodIndex)
{
	DEBUG_FATAL(!meshName || !*meshName, ("bad mesh name\n"));

	//-- scan skeleton for transforms starting with compound name, component 0 == "hp", component 1 == *meshName
	const MString componentString0("hp");
	MString       componentString1(meshName);
	IGNORE_RETURN(componentString1.toLowerCase());

	HardpointInfoMap  hardpointInfoMap;

	const bool collectResult = collectHardpoints(componentString0, componentString1, hardpointInfoMap, lodIndex);
	MESSENGER_REJECT(!collectResult, ("failed to collect hardpoints\n"));

	const bool processResult = processHardpointInfo(hardpointInfoMap, writer, true, lodIndex);
	MESSENGER_REJECT(!processResult, ("failed to process hardpoint\n"));

	// no errors
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::processArguments(
	const MArgList &args,
	MString *authorName,
	MString *generatorOutputDirectory,
	MDagPath *targetDagPath,
	bool &ignoreShaders,
	bool &ignoreBlendTargets,
	int &bindPoseFrameNumber,
	bool &ignoreTextures,
	bool &interactive,
	bool &commitToSourceControl,
	bool &createNewChangelist,
	bool &independentExport,
	bool &lock,
	bool &unlock,
	bool &showViewerAfterExport,
	MString &branch
)
{
	FATAL(!ms_installed, ("ExportSkeletalMeshGenerator not installed"));
	NOT_NULL(authorName);
	NOT_NULL(generatorOutputDirectory);
	NOT_NULL(targetDagPath);

	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));
	MESSENGER_REJECT(!argCount, ("No arguments passed to ExportSkeletalMeshGenerator\n"));

	//-- handle each argument
	bool haveOutputDirectory = false;
	bool haveAuthor          = false;
	bool haveNode            = false;
	bool isInteractive       = false;
	bool haveFrame           = false;
	bool haveBranch          = false;

	ignoreShaders           = false;
	ignoreBlendTargets      = false;
	ignoreTextures          = false;

	interactive             = false;
	commitToSourceControl   = false;
	createNewChangelist     = false;
	independentExport       = true;
	lock                    = false;
	unlock                  = false;

	// always non-silent unless the silent arg is present
	messenger->endSilentExport();

	// NOTE: interactive or node argument needs to come first

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = args.asString(argIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get arg [%u] as string\n", argIndex));

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_interactiveArgName)
		{
			MESSENGER_REJECT(argIndex != 0, ("-interactive must be first argument"));
			isInteractive = TRUE;
			interactive = true;
		}
		else if (argName == ExportArgs::cs_showViewerAfterExport)
		{
			showViewerAfterExport = true;
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

			*generatorOutputDirectory = args.asString(argIndex + 1, &status);
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
		else if (argName == ExportArgs::cs_ignoreShadersArgName)
		{
			ignoreShaders = true;
		}
		else if (argName == ExportArgs::cs_ignoreBlendTargetsArgName)
		{
			ignoreBlendTargets = true;
		}
		else if (argName == ExportArgs::cs_frameArgName)
		{
			//--handle node argument
			MESSENGER_REJECT(haveFrame, ("frame argument specified multiple times\n"));

			bindPoseFrameNumber = args.asInt(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get frame number argument\n"));

			haveFrame = true;
		}
		else if (argName == ExportArgs::cs_ignoreTexturesArgName)
		{
			ignoreTextures = true;
		}
		else if (argName == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
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
		else if (argName == ExportArgs::cs_partOfOtherExportArgName)
		{
			independentExport = false;
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
			MESSENGER_LOG_ERROR(("ARGS: unknown argument [%s]\n", argName.asChar()));
			return false;
		}
	}

	//-- handle interactive node selection
	if (isInteractive)
	{
		MSelectionList nodeList;
		status = MGlobal::getActiveSelectionList(nodeList);
		MESSENGER_REJECT(!status,("failed to get active selection list\n"))

		// we only support export of one mesh generator into one mesh generator file
		MESSENGER_REJECT(nodeList.length() != 1, ("must have exactly one node specified, currently [%u]\n", nodeList.length()));

		status = nodeList.getDagPath(0, *targetDagPath);
		MESSENGER_REJECT(!status, ("failed to get dag path for selected node\n"));
		MESSENGER_LOG(("ARGS: fetched selected DAG node [%s]\n", targetDagPath->partialPathName().asChar()));

		haveNode = true;
	}

	//-- handle output directory
	if (!haveOutputDirectory)
	{
		// grab from appearance output directory
		status = generatorOutputDirectory->set(SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));
		MESSENGER_REJECT(!status, ("generatorOutputDirectory->set() failed\n"));
		MESSENGER_LOG(("ARGS: fetched default output directory [%s]\n", generatorOutputDirectory->asChar()));
	}

	//-- handle default author specification
	if (!haveAuthor)
	{
		// get from author command
		status = authorName->set(SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX));
		MESSENGER_REJECT(!status, ("failed to set authorName string with [%s]\n", SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX)));
		MESSENGER_LOG(("ARGS: fetched default author name [%s]\n", authorName->asChar()));
	}

	//-- handle default frame specification
	if (!haveFrame)
	{
		bindPoseFrameNumber = -10;
		MESSENGER_LOG(("args: using default bind pose frame number [%d]\n", bindPoseFrameNumber));
	}

	//-- make sure everything required was specified
	MESSENGER_REJECT(commitToSourceControl && !haveBranch, ("no branch, i.e. \"-branch <branchname>\" was specified\n"));
	MESSENGER_REJECT(!isInteractive && !haveNode, ("no joint node (-node) was specified\n"));

	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::getShapePathFromParent(const MDagPath &parentPath, MDagPath &shapePath)
{
	MStatus status;

	const unsigned childCount = parentPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for selected DAG node\n"));

	int meshChildrenCount = 0;

	// Find the *first* MFnMesh-compatible child.
	for (unsigned childIndex = 0; childIndex < childCount; ++childIndex)
	{
		//-- Get the child object.
		MObject childObject = parentPath.child(childIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get child [%u] for node [%s]\n", childIndex, parentPath.fullPathName().asChar()));

		//-- Check if it is MFnMesh-compatible.
		if (childObject.hasFn(MFn::kMesh))
		{
			++meshChildrenCount;

			// Turn child into a dag node so we can get a dag path to it.
			MFnDagNode fnDagNode(childObject, &status);
			MESSENGER_REJECT(!status, ("failed to set MFnDagNode for child of selected dag [%s]\n", parentPath.fullPathName().asChar()));

			status = fnDagNode.getPath(shapePath);
			MESSENGER_REJECT(!status, ("failed to get Dag path for child of selected dag [%s]\n", parentPath.fullPathName().asChar()));

			// -TRF- skin-bound mesh shape is invisible but always present.
			// Hopefully the invisible original shape is always the second one,
			// because I'm using the first child.
			break;
		}
	}

	//-- there should be exactly one mesh child
	MESSENGER_REJECT(!meshChildrenCount, ("selected dag [%s] has no mesh children (no shape nodes)\n", parentPath.fullPathName().asChar()));
	MESSENGER_REJECT(meshChildrenCount > 1, ("unsupported: selected dag [%s] has more than one shape node child [%d]\n", parentPath.fullPathName().asChar(), meshChildrenCount));

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::getLodPath(const MDagPath &lodGroupPath, int lodIndex, MDagPath &lodPath)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, cs_lodNameCount);

	//-- Get the child with the specified lod index name.
	return MayaUtility::getChildPathWithName(lodGroupPath, cs_lodNameArray[lodIndex], lodPath);
}

// ----------------------------------------------------------------------
/**
 * Extract a short name and a TreeFile-relative shader template pathname
 * from a Maya shading engine object.
 */

bool ExportSkeletalMeshGenerator::extractShaderNames(bool useDefaultShader, const MObject &mayaShadingEngineObject, std::string &shaderShortName, std::string &shaderTemplateReferenceName)
{
	//-- Extract the shader's short name.
	if (useDefaultShader)
	{
		// Use a vertex color, lit, z buffer writing shader template.
		shaderShortName = "vertexcolorlz";
	}
	else
	{	
		// Construct shader template name.
		MStatus  status;

		// Ensure object is a shader engine node.
		MESSENGER_REJECT(mayaShadingEngineObject.apiType() != MFn::kShadingEngine, ("extractShaderNames(): expecting shading engine, found [%s]\n", mayaShadingEngineObject.apiTypeStr()));

		// Extract Maya name for shader group.
		MFnDependencyNode fnShadingEngineNode(mayaShadingEngineObject, &status);
		STATUS_REJECT(status, "failed to get MFnDependencyNode for shading engine object");

		const MString nodeName = fnShadingEngineNode.name(&status);
		STATUS_REJECT(status, "fnShadingEngineNode.name() failed");

		// Convert Maya node name to the shader short name.
		shaderShortName = nodeName.asChar();

		if ((shaderShortName.length() > 2) && (toupper(shaderShortName[shaderShortName.length() - 2]) == 'S') && (toupper(shaderShortName[shaderShortName.length() - 1]) == 'G'))
		{
			// Chop off the trailing SG.
			shaderShortName.resize(shaderShortName.length() - 2);
		}
	}

	//-- Build shader template reference name from the short name.
	shaderTemplateReferenceName  = SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX);
	shaderTemplateReferenceName += shaderShortName;
	shaderTemplateReferenceName += ".sht";

	//-- Indicate success.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether a specified shader has any polygons assigned to it.
 *
 * @param mayaShaderIndex       the index of the shader to check.
 * @param polyShaderAssignment  the assignment of each mesh poly to a shader index.
 *
 * @return  true if the specified shader index has at least one polygon assigned
 *          to it; false otherwise.
 */

bool ExportSkeletalMeshGenerator::arePolysAssignedToShader(int mayaShaderIndex, const MIntArray &polyShaderAssignment)
{
	const unsigned int polyCount = polyShaderAssignment.length();
	for (int unsigned i = 0; i < polyCount; ++i)
	{
		const int polyShaderIndex = polyShaderAssignment[i];
		if (polyShaderIndex == mayaShaderIndex)
			return true;
	}

	// No polys are assigned to the specified shader index.
	return false;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether a triangle with the specified inidices will be considered
 * zero area by the engine's triangle collision logic.
 *
 * Zero area triangles are bad for triangle collision as they cause the collision
 * to always evaluate to true.  That is a bad situation, so we don't want to
 * have zero-area triangles in our meshes.
 *
 * @return  true if the triangle will evaluate to zero-area for engine purposes;
 *          false otherwise.
 */

bool ExportSkeletalMeshGenerator::isZeroAreaTriangle(const Vector &position1, const Vector &position2, const Vector &position3)
{
	// Compute normal.
	//const Vector normal = (v0 - v2).cross(v1 - v0);
	const Vector normal = (position1 - position3).cross(position2 - position1);

	return (normal.magnitudeSquared () == 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the positions from the mesh converted into the game engine's
 * coordinate system.
 *
 * @param mesh           the mesh function set from which to convert.
 * @param gamePositions  the converted game positions are returned in this container, which is cleared prior to filling.
 *
 * @return  true if successful; false if some error occurred.
 */

bool ExportSkeletalMeshGenerator::getGamePositions(const MFnMesh &mesh, VectorVector &gamePositions)
{
	//-- Retrieve Maya positions from the mesh.
	MFloatPointArray  mayaPositions;

	MStatus status = mesh.getPoints(mayaPositions, MSpace::kWorld);
	STATUS_REJECT(status, "mesh.getPoints() failed");

	//-- Convert maya positions to engine positions.
	const unsigned int length = mayaPositions.length();
	gamePositions.reserve(length);
	gamePositions.clear();

	for (unsigned i = 0; i < length; ++i)
		gamePositions.push_back(MayaConversions::convertPoint(mayaPositions[i]));

	return true;
}

// ======================================================================
// class ExportSkeletalMeshGenerator: private member functions
// ======================================================================

/**
 * Write per shader data (shader vertices and primitives).
 *
 * @param mayaShaderIndex   Maya's shader index for the triangles to emit.
 *                          Use -1 if all triangles should be added regardless of assigned Shader.
 */

bool ExportSkeletalMeshGenerator::addPerShaderData(
	const MFnMesh                                       &fnMesh, 
	SkeletalMeshGeneratorWriter                         *writer, 
	const MObject                                       &mayaShadingEngineObject, 
	const MIntArray                                     &polyShaderAssignment, 
	int                                                  mayaShaderIndex, 
	const MStringArray                                  &mayaUvSetNames, 
	MayaUtility::TextureSet                             *referencedTextures, 
	const OccludedFaceMapGenerator                      &occludedFaceMapGenerator, 
	const MayaPerPixelLighting::ShaderRequestInfoVector &dot3ShaderRequestInfoVector, 
	const MayaPerPixelLighting::Dot3KeyMap              &dot3KeyMap,
	const VectorVector                                  &meshGamePositions
	)
{
	NOT_NULL(writer);
	UNREF(referencedTextures);

	MESSENGER_INDENT;
	MStatus status;

	ExporterLog::setUVSetsForCurrentMesh(fnMesh.numUVSets());

	//-- Track whether we'll use the real shaders.  This exporter is capable of exporting everything to a single built-in shader on request
	//   when m_ignoreShaders is true.
	const bool useDefaultShader = m_ignoreShaders;

	//-- Retrieve shader name.
	std::string  shaderShortName;
	std::string  shaderTemplateReferenceName;

	const bool extractName = extractShaderNames(useDefaultShader, mayaShadingEngineObject, shaderShortName, shaderTemplateReferenceName);
	MESSENGER_REJECT(!extractName, ("extractShaderNames() failed.\n"));

	//-- Check if any polys are assigned to the shader.
	const bool noPolysAssigned = (!useDefaultShader && !arePolysAssignedToShader(mayaShaderIndex, polyShaderAssignment));
	MESSENGER_REJECT_WARNING(noPolysAssigned, ("WARNING: shader [%s] has no polys assigned to selected mesh.\n", shaderShortName.c_str()));

	//-- Announce shader processing.
	MESSENGER_LOG(("Processing vertices for shader [%s]:\n", shaderTemplateReferenceName.c_str()));

	//-- Get color and alpha status for this mesh.
	bool hasVertexColor   = false;
	bool hasVertexAlpha   = false;
	const bool gcaSuccess = MayaUtility::getMeshColorAlphaStatus(fnMesh, mayaShaderIndex, polyShaderAssignment, &hasVertexColor, &hasVertexAlpha);
	MESSENGER_REJECT(!gcaSuccess, ("MayaUtility::getMeshColorAlphaStatus() failed\n"));
	const bool writeArgb  = hasVertexColor || hasVertexAlpha;

	//-- Check if shader uses dot3 lighting.
	bool usePerPixelLighting = false;

	if (mayaShaderIndex >= 0)
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, mayaShaderIndex, static_cast<int>(dot3ShaderRequestInfoVector.size()));

		const MayaPerPixelLighting::ShaderRequestInfo &info = dot3ShaderRequestInfoVector[static_cast<MayaPerPixelLighting::ShaderRequestInfoVector::size_type>(mayaShaderIndex)];
		usePerPixelLighting = info.isDot3Requested();
	}

	//-- Get writer's per shader data for this shader.
	SkeletalMeshGeneratorWriter::PerShaderData &perShaderData = writer->addShaderTemplateByName(shaderTemplateReferenceName.c_str());
	ExporterLog::addSourceShader (shaderTemplateReferenceName);

	//-- determine if this shader has occludable faces
	bool shaderHasOccludableFaces = false;

	const bool csfofResult = checkShaderForOccludableFaces(occludedFaceMapGenerator, polyShaderAssignment, mayaShaderIndex, shaderHasOccludableFaces);
	MESSENGER_REJECT(!csfofResult, ("checkShaderForOccludableFaces() failed\n"));

	//-- add all shader triangles to a single triangle list
	const int mayaPolygonCount = static_cast<int>(fnMesh.numPolygons(&status));
	MESSENGER_REJECT(!status, ("numPolygons() failed [%s]\n", status.errorString().asChar()));

	if (shaderHasOccludableFaces)
		writer->beginOccludableIndexedTriList(perShaderData, mayaPolygonCount * 2);
	else
		writer->beginIndexedTriList(perShaderData, mayaPolygonCount * 2);

	const int writerPositionCount = writer->getPositionCount();
	const int writerNormalCount   = writer->getNormalCount();

	std::vector<unsigned> shaderUvSetIndices;
	int                   shaderUvSetCount     = 0;
	MColor                mayaColor;
	int                   polygonIndex         = 0;
	int                   shaderTriangleCount  = 0;
	UniqueVertexSet       uniqueVertexSet;
	int                   totalVertexCount     = 0;
	int                   uniqueVertexCount    = 0;
	
	//-- loop over polygons in mesh
	MObject meshObject = fnMesh.object(&status);
	MESSENGER_REJECT(!status, ("failed to get mesh object\n"));

	MItMeshPolygon itPoly(meshObject, &status);
	MESSENGER_REJECT(!status, ("failed to create MItMeshPolygon for shape\n"));

	// initialize the loop iterator
	bool isDone = itPoly.isDone(&status);
	MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

	while (!isDone)
	{
		// check if this polygon belongs to this shader.  if so, export it.
		if ((mayaShaderIndex == -1) || (polyShaderAssignment[static_cast<size_t>(polygonIndex)] == mayaShaderIndex))
		{
			//-- process the polygon
			const long polyVertexCount = itPoly.polygonVertexCount(&status);
			MESSENGER_REJECT(!status, ("itPoly.polygonVertexCount() failed\n"));
			MESSENGER_REJECT(polyVertexCount > ms_maxSupportedPolygonVertexCount, ("unsupported poly vertex count [%d], max is [%d]\n", polyVertexCount, ms_maxSupportedPolygonVertexCount));

			totalVertexCount += static_cast<int>(polyVertexCount);

			//-- handle texture coordinate set count and dimensionality from stats on first polygon index
			if (shaderTriangleCount == 0)
			{
				// find out which, if any, uv sets affect this
				if (!useDefaultShader)
				{
					const unsigned meshUvSetCount = mayaUvSetNames.length();
					ExporterLog::setSourceNumberOfUVSets(static_cast<int>(meshUvSetCount));

					for (unsigned meshUvSetNameIndex = 0; meshUvSetNameIndex < meshUvSetCount; ++meshUvSetNameIndex)
					{
						const bool hasUv = itPoly.hasUVs(mayaUvSetNames[meshUvSetNameIndex], &status);
						MESSENGER_REJECT(!status, ("ItMeshPolygon::hasUVs() failed for uv set [%s]\n", mayaUvSetNames[meshUvSetNameIndex].asChar()));

						// if this polygon has uvs for this uv set, assume all polys for this same shader has these uvs set.
						if (hasUv)
							shaderUvSetIndices.push_back(meshUvSetNameIndex);
					}
				}

				// specify the texture situation for this shader
				shaderUvSetCount = static_cast<int>(shaderUvSetIndices.size());

				writer->setShaderTextureCoordinateSetCount(perShaderData, shaderUvSetCount);
				uniqueVertexSet.setTextureSetCount(shaderUvSetCount);

				for (int shaderUvSetIndex = 0; shaderUvSetIndex < shaderUvSetCount; ++shaderUvSetIndex)
				{
					// maya only supports 2 dimensional UVs
					writer->setShaderTextureCoordinateSetDimensionality(perShaderData, shaderUvSetIndex, 2);
					uniqueVertexSet.setTextureSetDimensionality(shaderUvSetIndex, 2);
				}
			}

			// Get the vertex to fill out for unique vertex set.
			int shaderVertexIndices[ms_maxSupportedPolygonVertexCount];
			int mayaPositionIndices[ms_maxSupportedPolygonVertexCount];

			for (int polyVertexIndex = 0; polyVertexIndex < static_cast<int>(polyVertexCount); ++polyVertexIndex)
			{
				UniqueVertexSet::Vertex &vertex = uniqueVertexSet.createVertex();
	
				//-- handle shader vertex position
				// get position index
				const int mayaPositionIndex = itPoly.vertexIndex(polyVertexIndex, &status);
				MESSENGER_REJECT(!status, ("itPoly.vertexIndex() failed\n"));

				// Track the mesh-relative position index so we can do zero-area triangle testing.
				mayaPositionIndices[polyVertexIndex] = mayaPositionIndex;

				// ensure position index is within range of positions provided to the writer
				MESSENGER_REJECT((mayaPositionIndex < 0) || (mayaPositionIndex >= writerPositionCount), ("Maya Data Error: vertex %d of poly %d has position index of %d (max valid is %d).\n", polyVertexIndex, polygonIndex, mayaPositionIndex, writerPositionCount - 1));

				// set shader vertex' position index
				uniqueVertexSet.setPositionIndex(vertex, mayaPositionIndex);

				//-- handle shader vertex normal
				// get normal index
				const int mayaNormalIndex = itPoly.normalIndex(polyVertexIndex, &status);
				MESSENGER_REJECT(!status, ("itPoly.normalIndex() failed\n"));

				// ensure normal index is within range of normals provided to the writer
				MESSENGER_REJECT((mayaNormalIndex < 0) || (mayaNormalIndex >= writerNormalCount), ("Maya Data Error: vertex %d of poly %d has normal index of %d (max valid is %d).\n", polyVertexIndex, polygonIndex, mayaNormalIndex, writerNormalCount - 1));

				// set shader vertex' normal index
				uniqueVertexSet.setNormalIndex(vertex, mayaNormalIndex);

				if (writeArgb)
				{
					// default the color
					PackedArgb gameArgb = PackedArgb::solidWhite;

					// check if Maya vertex has color info
					const bool mayaHasColor = itPoly.hasColor(polyVertexIndex, &status);
					MESSENGER_REJECT(!status, ("MItMeshPolygon::hasColor() failed\n"));

					if (mayaHasColor)
					{
						status = itPoly.getColor(mayaColor, polyVertexIndex);
						MESSENGER_REJECT(!status, ("MItMeshPolygon::getColor() failed\n"));

						// grab the info
						if (hasVertexColor)
						{
							gameArgb.setR(static_cast<uint8>(clamp(0, static_cast<int>(mayaColor.r * 255.0f), 255)));
							gameArgb.setG(static_cast<uint8>(clamp(0, static_cast<int>(mayaColor.g * 255.0f), 255)));
							gameArgb.setB(static_cast<uint8>(clamp(0, static_cast<int>(mayaColor.b * 255.0f), 255)));
						}
						if (hasVertexAlpha)
						{
							gameArgb.setA(static_cast<uint8>(clamp(0, static_cast<int>(mayaColor.a * 255.0f), 255)));
						}
					}
					uniqueVertexSet.setArgb(vertex, gameArgb);
				}

				// set texture coordinate data for shader vertex
				for (size_t uvSetIndex = 0; uvSetIndex < static_cast<size_t>(shaderUvSetCount); ++uvSetIndex)
				{
					// get index into maya uv set name
					const unsigned mayaUvSetNameIndex = shaderUvSetIndices[uvSetIndex];

					// get maya uv set name
					const MString &uvSetName = mayaUvSetNames[mayaUvSetNameIndex];

					// Get the uv coordinates.
					float  mayaUv[2];

					status = itPoly.getUV(polyVertexIndex, mayaUv, &uvSetName);
					MESSENGER_REJECT(!status, ("failed to get uv [%s] for vertex [%d]\n", uvSetName.asChar(), polyVertexIndex));

					// Convert Maya UVs to game UVs.
					float  foldedTc0;
					float  foldedTc1;

					// @todo we do not always want to fold UVs into the 0..1 range.
					MayaConversions::convertMayaUvToZeroOneRange(mayaUv, foldedTc0, foldedTc1);
					uniqueVertexSet.setTextureSetValue(vertex, static_cast<int>(uvSetIndex), foldedTc0, foldedTc1);
				}

				//-- check if this is a unique shader vertex.  if so, we need to add it to the writer.
				//   otherwise, we reuse the duplicate.  duplicates come about because we are asking for
				//   vertices of polys that may be meshed; hence, they might be shared.
				bool isUnique = false;
				const UniqueVertexSet::Vertex &uniqueVertex = uniqueVertexSet.submitVertex(vertex, &isUnique);

				int shaderVertexIndex;
				if (!isUnique)
				{
					// vertex already exists, grab the shader vertex index from it
					shaderVertexIndex = uniqueVertexSet.getUserInt01(uniqueVertex);
				}
				else
				{
					//-- Add this vertex to the writer.  It is known to be unique.

					// Write position.
					shaderVertexIndex = writer->addShaderVertex(perShaderData, mayaPositionIndex);

					// Write normal.
					writer->setShaderVertexNormal(perShaderData, mayaNormalIndex);

					// Handle dot3 lighting.
					if (usePerPixelLighting)
					{
						// Lookup the dot3 value vector index for this unique combination of {position index, normal index, shader index}.
						const MayaPerPixelLighting::Dot3Key key(mayaPositionIndex, mayaNormalIndex, mayaShaderIndex);

						MayaPerPixelLighting::Dot3KeyMap::const_iterator it  = dot3KeyMap.find(key);
						MESSENGER_REJECT(it == dot3KeyMap.end(), ("failed to find dot3 value index for position_idx=[%d], normal_idx=[%d], shader_idx=[%d].\n", mayaPositionIndex, mayaNormalIndex, mayaShaderIndex));

						// Write the dot3 value index.
						const int dot3ValueIndex = it->second;
						writer->setShaderVertexDot3ValueIndex(perShaderData, dot3ValueIndex);
					}

					if (writeArgb)
						writer->setShaderVertexDiffuseColor(perShaderData, uniqueVertexSet.getArgb(vertex));

					for (int tcSetIndex = 0; tcSetIndex < shaderUvSetCount; ++tcSetIndex)
					{
						const float tc0 = uniqueVertexSet.getTextureSetValue(vertex, tcSetIndex, 0);
						const float tc1 = uniqueVertexSet.getTextureSetValue(vertex, tcSetIndex, 1);
						writer->setShaderVertexTextureCoordinate(perShaderData, shaderVertexIndex, tcSetIndex, tc0, tc1);
					}

					// save the vertex index
					uniqueVertexSet.setUserInt01(vertex, shaderVertexIndex);
					++uniqueVertexCount;
				}

				// save this poly vertex's shader vertex index, we'll use it later during draw primitive building
				shaderVertexIndices[polyVertexIndex] = shaderVertexIndex;
			}

			const int polyOcclusionZoneCombination = occludedFaceMapGenerator.getFaceOcclusionZoneCombinationIndex(polygonIndex);

			// maya puts in CCW-is-visible culling order.  we need to put in CW order, so flip order of v1 and v2
			// fan the triangles --- note this is not always valid.  hence we want artists to triangulate meshes
			// before binding.
			for (size_t lastIndex = 2; lastIndex < static_cast<size_t>(polyVertexCount); ++lastIndex)
			{
				// Test the triangle for zero area.
				const bool zeroAreaTriangle = isZeroAreaTriangle(meshGamePositions[static_cast<size_t>(mayaPositionIndices[0])], meshGamePositions[static_cast<size_t>(mayaPositionIndices[lastIndex])], meshGamePositions[static_cast<size_t>(mayaPositionIndices[lastIndex - 1])]); //lint !e771 // (Info -- Symbol 'mayaPositionIndices' conceivably not initialized) // false
				if (zeroAreaTriangle)
				{
					// Skip the triangle.
					MESSENGER_LOG_WARNING(("zero area triangle detected for mesh poly index [%d], skipping tri, please remove.\n", polygonIndex));
				}
				else
				{
					// Write the triangle's shader indices.
					if (shaderHasOccludableFaces)
						writer->addOccludableIndexedTriListTri(polyOcclusionZoneCombination, shaderVertexIndices[0], shaderVertexIndices[lastIndex], shaderVertexIndices[lastIndex - 1]); //lint !e771 // (Info -- Symbol 'shaderVertexIndices' (line 373) conceivably not initialized) // false
					else
						writer->addIndexedTriListTri(shaderVertexIndices[0], shaderVertexIndices[lastIndex], shaderVertexIndices[lastIndex - 1]); //lint !e771 // (Info -- Symbol 'shaderVertexIndices' (line 373) conceivably not initialized) // false

					// count this triangle
					++shaderTriangleCount;
				}
			}
		}

		//-- increment the loop
		status = itPoly.next();
		MESSENGER_REJECT(!status, ("itPoly.next() failed\n"));

		isDone = itPoly.isDone(&status);
		MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

		++polygonIndex;
	}

	//-- cap off the tri list
	if (shaderHasOccludableFaces)
		writer->endOccludableIndexedTriList();
	else
		writer->endIndexedTriList();

	//-- Print out statistics.
	const float vertexCompressionPercentage = (totalVertexCount > 0) ? (100.0f * static_cast<float>(uniqueVertexCount)/static_cast<float>(totalVertexCount)) : (0);

	MESSENGER_LOG(("STATS: shader:           [%s]\n", shaderTemplateReferenceName.c_str()));
	MESSENGER_LOG(("STATS: triangles:        [%d]\n", shaderTriangleCount));
	MESSENGER_LOG(("STATS: shader vertices:  [%d, %d (%.2f%%)]\n", uniqueVertexCount, totalVertexCount, vertexCompressionPercentage));
	MESSENGER_LOG(("STATS: uv sets:          [%d]\n", shaderUvSetCount));
	MESSENGER_LOG(("STATS: user vertex color [%s]\n", hasVertexColor ? "true" : "false"));
	MESSENGER_LOG(("STATS: user vertex alpha [%s]\n", hasVertexAlpha ? "true" : "false"));

	//-- add shader triangle count to global triangle count
	m_meshTriangleCount += shaderTriangleCount;

	if (!useDefaultShader)
	{
		//-- construct the shader template
		// build shader template write path
		std::string shaderTemplateWritePath(SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX));

		shaderTemplateWritePath += shaderShortName;
		shaderTemplateWritePath += ".sht";

		//-- Determine bump map information for the shader.  The DOT3 texture coordinate will be constructed at runtime
		//   and will be placed as the last texture coordinate set.
		const bool useDot3TextureCoordinate   = usePerPixelLighting;
		const int  dot3TextureCoordinateIndex = shaderUvSetCount;

		// build the shader template
		typedef std::vector<std::pair<std::string, Tag> > TRReferenceContainer;
		TRReferenceContainer  referencedTextureRenderers;

		const bool result = MayaShaderTemplateBuilder::buildShaderTemplate(
			shaderTemplateWritePath,
		  mayaShadingEngineObject,
		  hasVertexAlpha,
		  referencedTextureRenderers,
		  SetDirectoryCommand::getDirectoryString(TEXTURE_REFERENCE_DIR_INDEX),
		  SetDirectoryCommand::getDirectoryString(TEXTURE_RENDERER_REFERENCE_DIR_INDEX),
		  SetDirectoryCommand::getDirectoryString(EFFECT_REFERENCE_DIR_INDEX),
		  *referencedTextures,
			useDot3TextureCoordinate,
			dot3TextureCoordinateIndex);

		MESSENGER_REJECT(!result, ("failed to write shader template for [%s]\n", shaderTemplateWritePath.c_str()));

		//-- add referenced texture renderers to writer
		TRReferenceContainer::iterator itEnd = referencedTextureRenderers.end();
		for (TRReferenceContainer::iterator it = referencedTextureRenderers.begin(); it != itEnd; ++it)
		{
			const char *const textureRendererName = it->first.c_str();
			const Tag         shaderTextureTag    = it->second;

			writer->addTextureRenderer(perShaderData, textureRendererName, shaderTextureTag);
		}

		MESSENGER_LOG(("successfully wrote shader template [%s]\n", shaderTemplateWritePath.c_str()));
	}

	// success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::handlePositionsAndNormals(
	const MDagPath                                      &meshDagPath, 
	const MayaMeshWeighting                             &meshWeighting, 
	const MayaPerPixelLighting::ShaderRequestInfoVector &dot3ShaderRequestInfoVector,
	const MayaPerPixelLighting::Dot3KeyMap              &dot3KeyMap,
	int                                                  dot3ValueCount,
	SkeletalMeshGeneratorWriter                         *writer
	) const
{
	MESSENGER_INDENT;
	MStatus status;

	std::vector<BlendShapeWeight> blendShapeWeightArray;

	//-- determine if any blend shapes control the mesh we're exporting
	MObjectArray  blendShapeObjects;

	const bool gbsSuccess = MayaMisc::getSceneDeformersAffectingMesh(meshDagPath, MFn::kBlendShape, &blendShapeObjects);
	MESSENGER_REJECT(!gbsSuccess, ("failed to get blend shapes affecting mesh\n"));

	const unsigned blendShapeCount     = blendShapeObjects.length();
	const bool exportingBlendShapeData = (blendShapeCount != 0) && !m_ignoreBlendTargets;

	MFnBlendShapeDeformer fnBlendShape;

	if (exportingBlendShapeData)
	{
		MObjectArray baseObjects;

		//-- save all blend weights affecting this mesh, then set to zero
		//   so our selected mesh is in a state unaffected by BlendShape influence
		for (unsigned blendShapeIndex = 0; blendShapeIndex < blendShapeCount; ++blendShapeIndex)
		{
			// get the blend shape
			MObject blendShapeObject = blendShapeObjects[blendShapeIndex];
			status                   = fnBlendShape.setObject(blendShapeObject);
			MESSENGER_REJECT(!status, ("failed to set blend shape object for function set\n"));

			// ensure this blend object affects only a single base object
			status = fnBlendShape.getBaseObjects(baseObjects);
			MESSENGER_REJECT(!status, ("getBaseObjects() failed\n"));
			
			const unsigned baseObjectCount = baseObjects.length();
			MESSENGER_REJECT(baseObjectCount != 1, ("unsupported: blend shape works on multiple base objects [%u]\n", baseObjectCount));
			MObject baseObject = baseObjects[0];

			// for each weight index in the blend shape, save weight info and set to zero
			const unsigned weightCount = fnBlendShape.numWeights(&status);
			MESSENGER_REJECT(!status, ("numWeights() failed\n"));

			for (unsigned weightIndex = 0; weightIndex < weightCount; ++weightIndex)
			{
				// get the current weight
				const float weight = fnBlendShape.weight(weightIndex, &status);
				MESSENGER_REJECT(!status, ("FnBlendShapeDeformer::weight() failed\n"));

				// save weight info
				blendShapeWeightArray.push_back(BlendShapeWeight(blendShapeObject, baseObject, static_cast<int>(weightIndex), weight));

				// set weight info to zero
				status = fnBlendShape.setWeight(weightIndex, 0.0f);
				MESSENGER_REJECT(!status, ("FnBlendShapeDeformer::setWeight() failed\n"));

				// clear base objects for next loop
				IGNORE_RETURN(baseObjects.clear());
			}
		}
	}

	//-- build FnMesh for our mesh
	MFnMesh  fnMesh(meshDagPath, &status);
	MESSENGER_REJECT(!status, ("export node [%s] is not a mesh shape\n", meshDagPath.partialPathName().asChar()));

	//-- export the non-blended positions
	// get mesh's position vectors
	MFloatPointArray  mayaUnblendedPositions;

	// here, world space is the model's model space
	status = fnMesh.getPoints(mayaUnblendedPositions, MSpace::kWorld);
	MESSENGER_REJECT(!status, ("failed to retrieve maya mesh positions [%s]\n", status.errorString().asChar()));

	// add positions and transform weightings
	const bool ampSuccess = addMeshPositions(mayaUnblendedPositions, meshWeighting, writer);
	MESSENGER_REJECT(!ampSuccess, ("addMeshPositions() failed\n"));

	//-- export the non-blended normals
	// get mesh's normal vectors
	MFloatVectorArray mayaUnblendedNormals;

	// here, world space is the model's model space
	status = fnMesh.getNormals(mayaUnblendedNormals, MSpace::kWorld);
	MESSENGER_REJECT(!status, ("failed to retrieve maya mesh normals\n"));

	// add normals
	const bool amnSuccess = addMeshNormals(mayaUnblendedNormals, writer);
	MESSENGER_REJECT(!amnSuccess, ("addMeshNormals() failed\n"));

	// Compute Dot3 values for the non-morphed mesh.
	MayaPerPixelLighting::FloatVector unblendedDot3Vector(static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * dot3ValueCount));

	if (dot3ValueCount > 0)
	{
		const bool computeDot3Success = MayaPerPixelLighting::computeDot3Values(fnMesh, dot3ShaderRequestInfoVector, dot3KeyMap, unblendedDot3Vector, dot3ValueCount);
		MESSENGER_REJECT(!computeDot3Success, ("MayaPerPixelLighting::computeDot3Values() failed.\n"));

		// Add Dot3 values to writer.
		for (int i = 0; i < dot3ValueCount; ++i)
		{
			writer->addDot3Value(
				unblendedDot3Vector[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 0)],
				unblendedDot3Vector[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 1)],
				unblendedDot3Vector[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 2)],
				unblendedDot3Vector[static_cast<MayaPerPixelLighting::FloatVector::size_type>(4 * i + 3)]);
		}
	}

	if (exportingBlendShapeData)
	{
		const unsigned weightCount = blendShapeWeightArray.size();

		//-- for each affecting weight, set weight to 1.0 and capture deltas.  if no deltas, don't consider
		//   the current weight.
		{
			MayaPerPixelLighting::Dot3DifferenceVector  dot3DifferenceVector;
			MayaPerPixelLighting::FloatVector           blendedDot3Vector(static_cast<MayaPerPixelLighting::FloatVector::size_type>(dot3ValueCount));

			MObjectArray                                targetObjects;
			MFnDependencyNode                           fnBlendTargetNode;
			MFloatPointArray                            mayaBlendedPositions;
			MFloatVectorArray                           mayaBlendedNormals;

			for (unsigned i = 0; i < weightCount; ++i)
			{
				BlendShapeWeight &blendShapeWeight = blendShapeWeightArray[i];

				//-- set the blend weight function set
				status = fnBlendShape.setObject(blendShapeWeight.m_blendShapeObject);
				STATUS_REJECT(status, "FnBlendShapeDeformer::setObject() failed");

				//-- set weight value to 1.0 for this weight index
				status = fnBlendShape.setWeight(static_cast<unsigned>(blendShapeWeight.m_weightIndex), 1.0f);
				STATUS_REJECT(status, "FnBlendShapeDeformer::setWeight() failed");

				//-- get the target objects that affect this weight index.  we'll use the first target
				//   object's name as the variable name for the weight index.
				status = fnBlendShape.getTargets(blendShapeWeight.m_baseObject, blendShapeWeight.m_weightIndex, targetObjects);
				// STATUS_REJECT(status, "getTargets() failed");
				if (!status)
				{
					MString nodeName;
					IGNORE_RETURN(MayaUtility::getNodeName(blendShapeWeight.m_baseObject, &nodeName));
					MESSENGER_REJECT(!status, ("getTargets() failed for base object=[%s],weightIndex=[%d],loopIndex=[%d],error=[%s].\n", nodeName.asChar(), blendShapeWeight.m_weightIndex, static_cast<int>(i), status.errorString().asChar()));
				}

				const unsigned targetObjectCount = targetObjects.length();
				MESSENGER_REJECT(targetObjectCount < 1, ("unexpected: targetObjectCount zero\n"));

				status = fnBlendTargetNode.setObject(targetObjects[0]);
				STATUS_REJECT(status, "failed to set MFnDependencyNode with blend target object");
				const MString blendTargetName = fnBlendTargetNode.name();

				// we use the last component as the name of the blend variable, then we strip off an
				// optional "Shape" string at the end.  this is not the ideal solution.  really we'd
				// like to steal the variable names of the weights within the blend shape, which default
				// to the target object name but can be changed  by the artist.  I haven't been able to
				// find a way to retrieve those (although I expect its there somewhere).
				MayaCompoundString  compoundTargetName(blendTargetName);
				const int           nameCount = compoundTargetName.getComponentCount();

				// remove "Shape" suffix from blend target name (if present)
				MString gameBlendTargetName = compoundTargetName.getComponentString(nameCount-1);
				IGNORE_RETURN(gameBlendTargetName.toLowerCase());

				removeShapeSuffix(gameBlendTargetName);

				//-- looks like MFnMesh objects become invalid in an unstable way (i.e. crash) once we
				//   modify their contents through manipulation of the blend shape weights.
				MFnMesh fnAlternateMesh(meshDagPath, &status);
				MESSENGER_REJECT(!status, ("failed to set mesh dag path\n"));

				//-- capture blend weight deltas
				// get blended positions
				status = fnAlternateMesh.getPoints(mayaBlendedPositions, MSpace::kWorld);
				MESSENGER_REJECT(!status, ("failed to retrieve maya blended mesh positions [%s]\n", status.errorString().asChar()));

				// get blended normals
				status = fnAlternateMesh.getNormals(mayaBlendedNormals, MSpace::kWorld);
				MESSENGER_REJECT(!status, ("failed to retrieve maya blended mesh normals [%s]\n", status.errorString().asChar()));

				// Dot3 handling.
				dot3DifferenceVector.clear();

				if (dot3ValueCount > 0)
				{
					// Capture dot3 values for blended mesh.
					const bool computeDot3Success = MayaPerPixelLighting::computeDot3Values(fnAlternateMesh, dot3ShaderRequestInfoVector, dot3KeyMap, blendedDot3Vector, dot3ValueCount);
					MESSENGER_REJECT(!computeDot3Success, ("MayaPerPixelLighting::computeDot3Values() failed.\n"));

					// Get differences between blended and unblended dot3 values.
					const bool computeDiffSuccess = MayaPerPixelLighting::computeDot3DifferenceVector(unblendedDot3Vector, blendedDot3Vector, dot3DifferenceVector);
					MESSENGER_REJECT(!computeDiffSuccess, ("MayaPerPixelLighting::computeDot3DifferenceVector() failed.\n"));
				}

				// capture the deltas between unblended and blended mesh
				const bool cbdSuccess = captureBlendDeltas(gameBlendTargetName, mayaUnblendedPositions, mayaBlendedPositions, mayaUnblendedNormals, mayaBlendedNormals, dot3DifferenceVector, writer);
				MESSENGER_REJECT(!cbdSuccess, ("failed to capture blend's deltas\n"));

				//-- clear weight value to remove this influence
				status = fnBlendShape.setWeight(static_cast<unsigned>(blendShapeWeight.m_weightIndex), 0.0f);
				MESSENGER_REJECT(!status, ("FnBlendShapeDeformer::setWeight() failed\n"));

				//-- clear scratch arrays for the next loop
				IGNORE_RETURN(targetObjects.clear());
				IGNORE_RETURN(mayaBlendedPositions.clear());
				IGNORE_RETURN(mayaBlendedNormals.clear());
			}
		}

		//-- be a good citizen and reset weights to original values
		{
			for (unsigned i = 0; i < weightCount; ++i)
			{
				BlendShapeWeight &blendShapeWeight = blendShapeWeightArray[i];

				//-- set the blend weight function set
				status = fnBlendShape.setObject(blendShapeWeight.m_blendShapeObject);
				MESSENGER_REJECT(!status, ("FnBlendShapeDeformer::setObject() failed\n"));

				//-- reset the weight to original value before export
				status = fnBlendShape.setWeight(static_cast<unsigned>(blendShapeWeight.m_weightIndex), blendShapeWeight.m_weight);
				MESSENGER_REJECT(!status, ("FnBlendShapeDeformer::setWeight() failed\n"));
			}
		}
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::performSingleExport(int bindPoseFrameNumber, const MDagPath &targetDagPath)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- Get the mesh shape node from the parent.  Users don't select the shape; 
	//   they select it's parent transform.
	MDagPath meshDagPath;
	MESSENGER_REJECT(!getShapePathFromParent(targetDagPath, meshDagPath), ("Failed to get shape DAG path from parent."));

	//-- Get mesh name.
	MFnDagNode fnTargetNode(targetDagPath, &status);
	STATUS_REJECT(status, "failed to set MFnDagNode from target MDagPath");

	const MString meshName = fnTargetNode.name(&status);
	STATUS_REJECT(status, "failed to get target node");

	//-- Do the export.
	SkeletalMeshGeneratorWriter writer;

	const bool exportSuccess = exportMeshGeneratorTemplate(bindPoseFrameNumber, meshDagPath, meshName.asChar(), writer, -1);
	MESSENGER_REJECT(!exportSuccess, ("Failed to export MeshGeneratorTemplate for shape DagPath [%s].", meshDagPath.fullPathName().asChar()));

	//-- Write data to Iff.
	Iff iff(ms_initialIffSize);
	writer.write(&iff);

	//-- Build output path name.
	std::string generatorOutputPath(SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));

	generatorOutputPath += "mesh\\";
	generatorOutputPath += meshName.asChar();
	generatorOutputPath += s_skeletalMeshGeneratorTemplateExtensionWithDot;

	// Write it to the file.
	const bool diskWriteSuccess = iff.write(generatorOutputPath.c_str(), true);
	MESSENGER_REJECT(!diskWriteSuccess, ("failed to write MeshGeneratorTemplate to [%s]\n", generatorOutputPath.c_str()));

	//-- write stats
	MESSENGER_LOG(("STATS: MeshGenerator size:           %d bytes.\n", iff.getRawDataSize()));
	MESSENGER_LOG(("STATS: MeshGenerator triangle count: %d.\n", m_meshTriangleCount));
	MESSENGER_LOG(("Successfully wrote [%s]\n", generatorOutputPath.c_str()));

	ExporterLog::buildSkeletalMeshGenerator(meshName.asChar(), m_meshTriangleCount * 3, m_meshTriangleCount);
	ExporterLog::addClientDestinationFile(generatorOutputPath);

	//-- Reset stats.
	m_meshTriangleCount = 0;

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool ExportSkeletalMeshGenerator::performLodExport(int bindPoseFrameNumber, const MDagPath &targetDagPath)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- Get mesh name.
	MFnDagNode fnTargetNode(targetDagPath, &status);
	STATUS_REJECT(status, "failed to set MFnDagNode from target MDagPath");

	const MString meshName = fnTargetNode.name(&status);
	STATUS_REJECT(status, "failed to get target node");

	//-- Process each detail level.
	LodMeshGeneratorTemplateWriter  lodWriter;
	MDagPath                        detailLevelPath;

	int                             totalVertexCount = 0;
	int                             totalPolyCount   = 0;
	bool                            done             = false;

	int lodIndex;
	for (lodIndex = 0; !done; ++lodIndex)
	{
		MESSENGER_LOG(("+++++ Processing Mesh LOD [%d] +++++\n", lodIndex));

		//-- Retrieve the LOD node for the given index.
		const bool hasLod = getLodPath(targetDagPath, lodIndex, detailLevelPath);
		if (!hasLod)
		{
			//-- No more to do, this LOD doesn't exist for this mesh generator.
			done = true;
			break;
		}

		//-- Get the mesh shape node from the parent.  Users don't select the shape; 
		//   they select it's parent transform.
		MDagPath meshDagPath;
		MESSENGER_REJECT(!getShapePathFromParent(detailLevelPath, meshDagPath), ("Failed to get shape DAG path from parent."));

		//-- Do the export for this detail level.
		SkeletalMeshGeneratorWriter writer;

		const bool exportSuccess = exportMeshGeneratorTemplate(bindPoseFrameNumber, meshDagPath, meshName.asChar(), writer, lodIndex);
		MESSENGER_REJECT(!exportSuccess, ("Failed to export MeshGeneratorTemplate for shape DagPath [%s].", meshDagPath.fullPathName().asChar()));

		//-- Write data to Iff.
		Iff iff(ms_initialIffSize);
		writer.write(&iff);

		//-- Build output path name.
		std::string generatorOutputPath(SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));
		std::string referenceFileName(SetDirectoryCommand::getDirectoryString(APPEARANCE_REFERENCE_DIR_INDEX));

		generatorOutputPath += "mesh\\";
		referenceFileName += "mesh/";

		generatorOutputPath += meshName.asChar();
		referenceFileName += meshName.asChar();

		// Tack LOD number on to mesh generator filename.
		char numberBuffer[32];
		sprintf(numberBuffer, "_l%d", lodIndex);
		generatorOutputPath += numberBuffer;
		referenceFileName += numberBuffer;

		generatorOutputPath += s_skeletalMeshGeneratorTemplateExtensionWithDot;
		referenceFileName += s_skeletalMeshGeneratorTemplateExtensionWithDot;

		// Write it to the file.
		const bool diskWriteSuccess = iff.write(generatorOutputPath.c_str(), true);
		MESSENGER_REJECT(!diskWriteSuccess, ("failed to write MeshGeneratorTemplate to [%s]\n", generatorOutputPath.c_str()));

		//-- Write stats for detail level.
		MESSENGER_LOG(("STATS: MeshGenerator size:           %d bytes.\n", iff.getRawDataSize()));
		MESSENGER_LOG(("STATS: MeshGenerator triangle count: %d.\n", m_meshTriangleCount));
		MESSENGER_LOG(("Successfully wrote [%s]\n", generatorOutputPath.c_str()));

		// @todo do the real thing here, need to count vertices per shader.
		const int vertexCount = m_meshTriangleCount * 3;

		ExporterLog::buildSkeletalMeshGenerator(meshName.asChar(), vertexCount, m_meshTriangleCount);
		ExporterLog::addClientDestinationFile(generatorOutputPath);

		//-- Keep track of totals.
		totalVertexCount += vertexCount;
		totalPolyCount   += m_meshTriangleCount;

		//-- Tell LodMeshGeneratorTemplate about this detail level.
		lodWriter.addDetailLevelByName(referenceFileName);

		//-- Reset stats.
		m_meshTriangleCount = 0;
	}

	//-- Write the lod writer.
	if (lodIndex < 1)
	{
		MESSENGER_LOG(("Skipping writing of LOD MeshGeneratorTemplate because no detail levels exist.\n"));
	}
	else
	{
		MESSENGER_LOG(("+++++ Processing LOD Parent +++++\n", lodIndex));

		//-- Write data to Iff.
		Iff iff(ms_initialIffSize);
		lodWriter.write(iff);

		//-- Build output path name.
		std::string generatorOutputPath(SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));

		generatorOutputPath += "mesh\\";
		generatorOutputPath += meshName.asChar();
		generatorOutputPath += s_lodMeshGeneratorTemplateExtensionWithDot;

		// Write it to the file.
		const bool diskWriteSuccess = iff.write(generatorOutputPath.c_str(), true);
		MESSENGER_REJECT(!diskWriteSuccess, ("failed to write LodMeshGeneratorTemplate to [%s]\n", generatorOutputPath.c_str()));

		//-- Write stats for detail level.
		MESSENGER_LOG(("STATS: MeshGenerator size:           %d bytes.\n", iff.getRawDataSize()));
		MESSENGER_LOG(("STATS: MeshGenerator triangle count: %d.\n", totalPolyCount));
		MESSENGER_LOG(("Successfully wrote LodMeshGeneratorTemplate [%s]\n", generatorOutputPath.c_str()));

		ExporterLog::buildSkeletalMeshGenerator(meshName.asChar(), totalVertexCount, totalPolyCount);
		ExporterLog::addClientDestinationFile(generatorOutputPath);
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Export the specified shape path into the given writer.
 *
 * @param bindPoseFrameNumber  number of the frame indicating skeleton pose in the bind position.
 * @param shapeDagPath         a MDagPath to the shape (not the shape's transform node).
 * @param writer               the shape is exported to this writer on successful exit.
 *
 * @return  true if the export succeeded; false otherwise.
 */

bool ExportSkeletalMeshGenerator::exportMeshGeneratorTemplate(int bindPoseFrameNumber, const MDagPath &shapeDagPath, const char *meshName, SkeletalMeshGeneratorWriter &writer, int lodIndex)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- Get Maya MFnMesh for the shape.
	MFnMesh  fnMesh(shapeDagPath, &status);
	MESSENGER_REJECT(!status, ("export node [%s] is not a mesh shape\n", shapeDagPath.fullPathName().asChar()));

	//--
	//-- Handle occlusion export requirements.
	//--
	// Generate occluded face map data.
	std::vector<MObject>  faceSets;

	const bool gfsResult = getFaceSets(shapeDagPath, faceSets);
	MESSENGER_REJECT(!gfsResult, ("Failed to get face sets.\n"));

	OccludedFaceMapGenerator  occludedFaceMapGenerator(faceSets, shapeDagPath, status);
	MESSENGER_REJECT(!status, ("Failed to construct occludedFaceMapGenerator.\n"));

	// Generate a dag path for the shape parent.  The shape parent node has the occlusion attributes on it.
	MDagPath  shapeParentDagPath(shapeDagPath);

	status = shapeParentDagPath.pop();
	STATUS_REJECT(status, "shapeParentDagPath.pop()");
	
	// Export occlusion-related data.
	const bool eodResult = exportOcclusionData(writer, occludedFaceMapGenerator, shapeParentDagPath);
	MESSENGER_REJECT(!eodResult, ("ExportMeshOcclusionData() failed.\n"));

	//--
	//-- Retrieve and process skeleton weighting information.
	//-- 

	// Find joints and joint weightings for mesh.
	MayaMeshWeighting  meshWeighting(shapeDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create meshWeighting\n"));
	
	const bool getMwdSuccess = MayaUtility::addMeshAffectors(shapeDagPath, meshWeighting);
	MESSENGER_REJECT(!getMwdSuccess, ("failed to add mesh affectors to MayaMeshWeighting helper class\n"));

	const bool processSuccess = meshWeighting.processAffectors();
	MESSENGER_REJECT(!processSuccess, ("MayaMeshWeighting failed to process affectors\n"));

	// Add affecting joint transforms to writer.
	MStringArray  skeletonTemplateNodeNames;
	const bool attwSuccess = addTransformsToWriter(meshWeighting, &writer, &skeletonTemplateNodeNames);
	MESSENGER_REJECT(!attwSuccess, ("failed to add affecting transforms to writer\n"));

	//--
	//-- Collect baseline dot3 data.
	//--

	MayaPerPixelLighting::ShaderRequestInfoVector  dot3ShaderRequestInfoVector;
	MayaPerPixelLighting::Dot3KeyMap               dot3KeyMap;
	int                                            dot3ValueCount;

	// Retrieve per-shader dot3 request information.
	const bool getSriSuccess = MayaPerPixelLighting::getShaderRequestInfo(fnMesh, dot3ShaderRequestInfoVector);
	MESSENGER_REJECT(!getSriSuccess, ("MayaPerPixelLighting::getShaderRequestInfo() failed.\n"));

	// Generate the dot3 key map (maps {position index, normal index, shader index} to a specific dot3 value).
	const bool generateKmSuccess = MayaPerPixelLighting::generateDot3KeyMap(fnMesh, dot3ShaderRequestInfoVector, dot3KeyMap, dot3ValueCount);
	MESSENGER_REJECT(!generateKmSuccess, ("MayaPerPixelLighting::generateDot3KeyMap() failed.\n"));

	//--
	//-- Retrieve position, normal and dot3 data.
	//--

	// Go to bind pose so we get proper position and normal data.
	const bool sstbp = setSkeletonsToBindPose(skeletonTemplateNodeNames, bindPoseFrameNumber);
	MESSENGER_REJECT(!sstbp, ("failed to set skeletons to bind pose\n"));

	// Handle writing position and normal data.
	const bool hpanSuccess = handlePositionsAndNormals(shapeDagPath, meshWeighting, dot3ShaderRequestInfoVector, dot3KeyMap, dot3ValueCount, &writer);
	MESSENGER_REJECT(!hpanSuccess, ("failed to handle writing positions and normals\n"));

	//--
	//-- Handle per-shader data generation.
	//--

	// Create a new MFnMesh --- handlePositionsAndNormals may invalidate our old one in an unstable manner.
	MFnMesh fnMeshStage2(shapeDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to initialize MFnMesh\n"));

	// Get the maya mesh UV set names that affect this mesh.
	MStringArray mayaUvSetNames;
	status = fnMeshStage2.getUVSetNames(mayaUvSetNames);
	MESSENGER_REJECT(!status, ("failed to get mesh UV set names\n"));

	// Warn if there is more than one UV set for character system meshes.
	const unsigned uvSetCount = mayaUvSetNames.length();
	if (uvSetCount > 1)
	{
		MESSENGER_LOG_WARNING(("Skeletal mesh has [%d] UV sets associated with it, remove unnecessary sets.\n", uvSetCount));
	}

	bool exportSuccess = true;

	// Handle per shader data (draw primitives are formed here).
	{
		//-- Get mesh polygon to shader assignment.
		MObjectArray  mayaShaderObjects;
		MIntArray     mayaPolygonShaderAssignments;

		status = fnMeshStage2.getConnectedShaders(0, mayaShaderObjects, mayaPolygonShaderAssignments);
		MESSENGER_REJECT(!status, ("fnMeshStage2.getConnectedShaders() failed\n"));

		//-- Get Maya positions.  Note these are possibly morphed.
		// @todo collect the non-morphed data only once.  Currently I collect and convert this in handlePositionsAndNormals as well.
		VectorVector  meshGamePositions;

		const bool ggpResult = getGamePositions(fnMeshStage2, meshGamePositions);
		MESSENGER_REJECT(!ggpResult, ("getGamePositions() failed.\n"));

		//-- Add per shader data to the writer.
		if (m_ignoreShaders)
		{
			// We're going to ignore shaders, throw all mesh polys into one shader group.
			const bool apsdSuccess = addPerShaderData(fnMeshStage2, &writer, mayaShaderObjects[0], mayaPolygonShaderAssignments, -1, mayaUvSetNames, 0, occludedFaceMapGenerator, dot3ShaderRequestInfoVector, dot3KeyMap, meshGamePositions);
			MESSENGER_REJECT(!apsdSuccess, ("addPerShaderData() failed\n"));
		}
		else
		{
			// Normal export --- build mesh data one shader at a time.
			MESSENGER_LOG(("=== [HANDLING PER SHADER DATA] ===\n"));
			MayaUtility::TextureSet referencedTextures;

			const unsigned mayaShaderCount = mayaShaderObjects.length();
			for (unsigned i = 0; i < mayaShaderCount; ++i)
			{
				MESSENGER_LOG(("- - - - - [SHADER] - - - - - - -\n"));
				const bool apsdSuccess = addPerShaderData(fnMeshStage2, &writer, mayaShaderObjects[i], mayaPolygonShaderAssignments, static_cast<int>(i), mayaUvSetNames, &referencedTextures, occludedFaceMapGenerator, dot3ShaderRequestInfoVector, dot3KeyMap, meshGamePositions);
				MESSENGER_REJECT(!apsdSuccess, ("addPerShaderData() failed\n"));
			}

			//-- Handle texture export.
			if (!m_ignoreTextures)
			{
				// fill up existing crc map from ExporterLog
				MayaUtility::FileCrcMap existingCrcMap;
				MayaUtility::loadFileCrcMap(&existingCrcMap, referencedTextures);

				// export source-modified textures
				MayaUtility::FileCrcMap revisedCrcMap;

				const char *const textureWriteDir = SetDirectoryCommand::getDirectoryString(TEXTURE_WRITE_DIR_INDEX);
				const bool textureExportSuccess   = MayaUtility::generateTextures(&referencedTextures, &existingCrcMap, textureWriteDir, &revisedCrcMap, false);

				// allow export to continue even though something failed (generating some of the dds files)
				if (!textureExportSuccess)
					exportSuccess = false;

				// Tell exporter log about new crc values.
				MayaUtility::saveFileCrcMap(existingCrcMap, revisedCrcMap, referencedTextures);
			}
		}
	}

	//-- Handle hardpoints.
	const bool ehResult = exportHardpoints(meshName, writer, lodIndex);
	MESSENGER_REJECT(!ehResult, ("failed to export hardpoint data.\n"));

	//-- Enable deformerss (may have disabled when setting to bind pose).
	const bool edSuccess = MayaUtility::enableDeformers();
	MESSENGER_REJECT(!edSuccess, ("failed to enable deformers after capturing bind pose data\n"));

	//-- Success.
	return exportSuccess;
}

// ======================================================================
