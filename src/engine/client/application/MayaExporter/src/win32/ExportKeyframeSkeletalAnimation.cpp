// ======================================================================
//
// ExportKeyframeSkeletalAnimation.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "ExportKeyframeSkeletalAnimation.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include "AlienbrainImporter.h"
#include "AnimationMessageCollector.h"
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "ExportSkeleton.h" // for static "getSkeletonName" func.  -TRF- let's put this function in MayaUtility and get this header out of here.
#include "KeyframeSkeletalAnimationTemplateWriter.h"
#include "MayaCompoundString.h"
#include "MayaConversions.h"
#include "MayaMisc.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "Resource.h"
#include "SetDirectoryCommand.h"
#include "VisitAnimationCommand.h"

#include "maya/MAnimControl.h"
#include "maya/MArgList.h"
#include "maya/MEulerRotation.h"
#include "maya/MFileIO.h"
#include "maya/MFnIkJoint.h"
#include "maya/MGlobal.h"
#include "maya/MSelectionList.h"
#include "maya/MTime.h"
#include "maya/MVector.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

// ======================================================================
// lint supression
// ======================================================================

// error 754: unreferenced member
//lint -esym(754, JointData::JointData)
//lint -esym(754, JointData::operator=)

// ======================================================================
// debug support
// ======================================================================

#define LOG_SPECIFIC_JOINT_OUTPUT 1

#if LOG_SPECIFIC_JOINT_OUTPUT
#include <boost/smart_ptr.hpp>
#endif

// ======================================================================
// static storage
// ======================================================================

const MString  cs_masterNodeName("master");
const int      ExportKeyframeSkeletalAnimation::cms_iffSize = 2 * 1024 * 1024;

// ----------------------------------------------------------------------

namespace
{
	typedef stdvector<int>::fwd  IntVector;
}

// ----------------------------------------------------------------------

#if LOG_SPECIFIC_JOINT_OUTPUT
namespace
{
	typedef std::set<boost::shared_ptr<CrcLowerString>, LessPointerComparator>  CrcLowerStringSet;

	CrcLowerStringSet  ms_specificJointNameSet;
}
#endif

// ======================================================================
// embedded classes
// ======================================================================

struct ExportKeyframeSkeletalAnimation::RealKeyData
{
public:

	RealKeyData();
	RealKeyData(int frameNumber, float frameData);

public:

	int    m_frameNumber;
	float  m_keyData;

};

// ----------------------------------------------------------------------

struct ExportKeyframeSkeletalAnimation::QuaternionKeyData
{
public:

	QuaternionKeyData();
	QuaternionKeyData(int frameNumber, const Quaternion &rotation);

public:

	int        m_frameNumber;
	Quaternion m_keyData;

};

// ----------------------------------------------------------------------

struct ExportKeyframeSkeletalAnimation::VectorKeyData
{
public:

	VectorKeyData();
	VectorKeyData(int frameNumber, const Vector &keyData);

public:

	int     m_frameNumber;
	Vector  m_keyData;

};

// ----------------------------------------------------------------------

struct ExportKeyframeSkeletalAnimation::JointData
{
public:

	explicit JointData(const CrcLowerString &name, int anticipatedKeyframeCount);

public:

	CrcLowerString                 m_name;

	std::vector<QuaternionKeyData> m_rotationKeys;

	std::vector<RealKeyData>       m_xTranslationKeys;
	std::vector<RealKeyData>       m_yTranslationKeys;
	std::vector<RealKeyData>       m_zTranslationKeys;

private:

	// disabled
	JointData();
	JointData(const JointData&);
	JointData &operator =(const JointData&);

};

// ----------------------------------------------------------------------

struct ExportKeyframeSkeletalAnimation::JointDataContainer
{
public:

	~JointDataContainer();

public:

	typedef std::map<CrcLowerString*, JointData*, CrcLowerString::LessPtrComparator> Container;

public:

	Container  m_container;

};

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================
// struct ExportKeyframeSkeletalAnimation::RealKeyData
// ======================================================================

inline ExportKeyframeSkeletalAnimation::RealKeyData::RealKeyData()
:
	m_frameNumber(-1),
	m_keyData(CONST_REAL(0))
{
}

// ----------------------------------------------------------------------

inline ExportKeyframeSkeletalAnimation::RealKeyData::RealKeyData(int frameNumber, real frameData)
:
	m_frameNumber(frameNumber),
	m_keyData(frameData)
{
}

// ======================================================================
// struct ExportKeyframeSkeletalAnimation::QuaternionKeyData
// ======================================================================

inline ExportKeyframeSkeletalAnimation::QuaternionKeyData::QuaternionKeyData() :
	m_frameNumber(-1),
	m_keyData()
{
}

// ----------------------------------------------------------------------

inline ExportKeyframeSkeletalAnimation::QuaternionKeyData::QuaternionKeyData(int frameNumber, const Quaternion &rotation) :
	m_frameNumber(frameNumber),
	m_keyData(rotation)
{
}

// ======================================================================
// struct ExportKeyframeSkeletalAnimation::VectorKeyData
// ======================================================================

inline ExportKeyframeSkeletalAnimation::VectorKeyData::VectorKeyData() :
	m_frameNumber(0),
	m_keyData()
{
}

// ----------------------------------------------------------------------

inline ExportKeyframeSkeletalAnimation::VectorKeyData::VectorKeyData(int frameNumber, const Vector &keyData) :
	m_frameNumber(frameNumber),
	m_keyData(keyData)
{
}

// ======================================================================
// struct ExportKeyframeSkeletalAnimation::JointData
// ======================================================================

ExportKeyframeSkeletalAnimation::JointData::JointData(const CrcLowerString &name, int anticipatedKeyframeCount)
:
	m_name(name),
	m_rotationKeys(),
	m_xTranslationKeys(),
	m_yTranslationKeys(),
	m_zTranslationKeys()
{
	m_rotationKeys.reserve(static_cast<size_t>(anticipatedKeyframeCount));
	m_xTranslationKeys.reserve(static_cast<size_t>(anticipatedKeyframeCount));
	m_yTranslationKeys.reserve(static_cast<size_t>(anticipatedKeyframeCount));
	m_zTranslationKeys.reserve(static_cast<size_t>(anticipatedKeyframeCount));
}

// ======================================================================
// struct ExportKeyframeSkeletalAnimation::JointDataContainer
// ======================================================================

ExportKeyframeSkeletalAnimation::JointDataContainer::~JointDataContainer()
{
	// delete the joint data
	Container::iterator             it    = m_container.begin();
	const Container::const_iterator itEnd = m_container.end();
	for (; it != itEnd; ++it)
	{
		JointData *const deadJointData = (*it).second;
		delete deadJointData;
	}
}

// ======================================================================
// class ExportKeyframeSkeletalAnimation
// ======================================================================

void ExportKeyframeSkeletalAnimation::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ExportKeyframeSkeletalAnimation::remove(void)
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *ExportKeyframeSkeletalAnimation::creator()
{
	return new ExportKeyframeSkeletalAnimation;
}

// ----------------------------------------------------------------------

ExportKeyframeSkeletalAnimation::ExportKeyframeSkeletalAnimation() :
	MPxCommand(),
	m_bindPoseJointData(0),
	m_animationJointData(0),
	m_currentFrameNumber(0),
	m_frameCount(0),
	m_selectedDagPath(),
	m_locomotionRotationKeys(0),
	m_locomotionTranslationKeys(0)
{
	m_bindPoseJointData  = new JointDataContainer();
	m_animationJointData = new JointDataContainer();
}

// ----------------------------------------------------------------------

ExportKeyframeSkeletalAnimation::~ExportKeyframeSkeletalAnimation()
{
	delete m_locomotionTranslationKeys;
	delete m_locomotionRotationKeys;
	delete m_animationJointData;
	delete m_bindPoseJointData;
}

// ----------------------------------------------------------------------

void ExportKeyframeSkeletalAnimation::resetState()
{
	delete m_locomotionRotationKeys;
	m_locomotionRotationKeys = 0;

	delete m_locomotionTranslationKeys;
	m_locomotionTranslationKeys = 0;

	delete m_animationJointData;
	m_animationJointData = new JointDataContainer();

	delete m_bindPoseJointData;
	m_bindPoseJointData  = new JointDataContainer();

	m_currentFrameNumber = 0;
	m_frameCount         = 0;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::captureLocomotionData(const MDagPath &targetDagPath)
{
	MStatus status;

	MFnTransform fnTransform(targetDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to set MFnTransform for joint dag path [%s]\n", targetDagPath.partialPathName().asChar()));

	//-- retrieve a quaternion-based rotation for this transform node's rotation
	Quaternion transformRotation;

	const bool getRotationSuccess = getTransformRotation(fnTransform, transformRotation);
	MESSENGER_REJECT(!getRotationSuccess, ("getTransformRotation failed for transform [%s].\n", targetDagPath.partialPathName().asChar()));

	//-- retrieve transform's translation
	// get Maya translate
	MVector mayaJointTranslation = fnTransform.translation(MSpace::kTransform, &status);
	MESSENGER_REJECT(!status, ("failed to get joint translation\n"));

	// convert to game translation
	const Vector jointTranslation = MayaConversions::convertVector(mayaJointTranslation);

	//-- save animation's locomotion data
	if (!m_locomotionRotationKeys)
		m_locomotionRotationKeys = new QuaternionKeyDataVector;

	if (!m_locomotionTranslationKeys)
		m_locomotionTranslationKeys = new VectorKeyDataVector;

	const bool onLastFrame = (m_currentFrameNumber == m_frameCount);

	addRotationKey(m_currentFrameNumber, *m_locomotionRotationKeys, transformRotation, onLastFrame);
	addVectorKey(m_currentFrameNumber, *m_locomotionTranslationKeys, jointTranslation, onLastFrame);

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::captureTransformData(const MDagPath &targetDagPath, CaptureMode captureMode, bool &stopTraversal)
{
	MStatus status;

	stopTraversal = false;

	MFnTransform fnTransform(targetDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to set MFnTransform for joint dag path [%s]\n", targetDagPath.partialPathName().asChar()));

	//-- get joint name
	MString  jointName = fnTransform.name(&status);
	MESSENGER_REJECT(!status, ("failed to retrieve joint's name\n"));

	//-- check if we're a new skeleton template marked with "nodename__filename" name type.  if so, we skip this child
	{
		MayaCompoundString  compoundName(jointName);

		const bool hasFilename = (compoundName.getComponentCount() > 1);

		// if we've got an embedded filename, check if we're treading into the bones of a
		// skeleton template attached to our exported skeleton.
		const bool isNewSkeletonTemplate = hasFilename && !(targetDagPath == m_selectedDagPath);

		if (isNewSkeletonTemplate)
		{
			// we don't export animation for multiple skeleton templates
			// -TRF- might need to revisit this for heads, come up with a naming solution
			// for those parts that are always there in one form or another vs. transient
			// bones.
			stopTraversal = true;
			return true;
		}

		if (hasFilename)
		{
			// root node for this skeleton template will have filename.
			jointName = compoundName.getComponentString(0);
		}
	}

	//-- retrieve a quaternion-based rotation for this transform node's rotation
	Quaternion transformRotation;

	const bool getRotationSuccess = getTransformRotation(fnTransform, transformRotation);
	MESSENGER_REJECT(!getRotationSuccess, ("getTransformRotation failed for joint [%s].\n", jointName.asChar()));

#if 0
	DEBUG_REPORT_LOG(true, ("====\n"));

	DEBUG_REPORT_LOG(true, ("Original quaternion:\n"));
	transformRotation.debugDump();

	DEBUG_REPORT_LOG(true, ("Compressed quaternion:\n"));
	CompressedQuaternion cq(transformRotation);
	cq.debugDump();

	DEBUG_REPORT_LOG(true, ("Newly expanded quaternion:\n"));
	const Quaternion neq(cq.expand());
	neq.debugDump();
#endif

	//-- retrieve transform's translation
	// get Maya translate
	MVector mayaJointTranslation = fnTransform.translation(MSpace::kTransform, &status);
	MESSENGER_REJECT(!status, ("failed to get joint translation\n"));

	// convert to game translation
	const Vector jointTranslation = MayaConversions::convertVector(mayaJointTranslation);

	//-- save animation's joint data
	JointDataContainer *jointDataContainer = 0;
	switch (captureMode)
	{
		case CM_bindPose:
			jointDataContainer = m_bindPoseJointData;
			break;
		case CM_animation:
			jointDataContainer = m_animationJointData;
			break;
		default:
			FATAL(true, ("unknown CaputureMode [%u]", captureMode));
	}

	JointData *jointData = 0;

	// find or create joint data for this joint
	CrcLowerString crcJointName(jointName.asChar());
	JointDataContainer::Container::iterator it = jointDataContainer->m_container.find(&crcJointName);

	if (it != jointDataContainer->m_container.end())
		jointData = NON_NULL((*it).second);
	else
	{
		// joint data doesn't exist, create and insert
		jointData = new JointData(crcJointName, m_frameCount);

		JointDataContainer::Container::value_type value(&(jointData->m_name), jointData);
		IGNORE_RETURN(jointDataContainer->m_container.insert(value));
	}

	if (captureMode == CM_bindPose)
	{
		//-- save the bind pose information
		jointData->m_rotationKeys.push_back(QuaternionKeyData(0, transformRotation));

		jointData->m_xTranslationKeys.push_back(RealKeyData(0, jointTranslation.x));
		jointData->m_yTranslationKeys.push_back(RealKeyData(0, jointTranslation.y));
		jointData->m_zTranslationKeys.push_back(RealKeyData(0, jointTranslation.z));
	}
	else if (captureMode == CM_animation)
	{
		//-- save this frame's animation data relative to the bind pose
		const bool onLastFrame = (m_currentFrameNumber == m_frameCount);

		// find bind pose data
		JointDataContainer::Container::const_iterator bpIt = m_bindPoseJointData->m_container.find(&(jointData->m_name));
		MESSENGER_REJECT(bpIt == m_bindPoseJointData->m_container.end(), ("failed to lookup bind pose data for joint [%s]\n", jointData->m_name.getString()));

		NOT_NULL((*bpIt).second);
		const JointData *const bindPoseJointData = (*bpIt).second;

		const Quaternion rotationFromBindPose = getDeltaRotation(bindPoseJointData->m_rotationKeys.front().m_keyData, transformRotation);

		// save out animation data
		addRotationKey(m_currentFrameNumber, jointData->m_rotationKeys, rotationFromBindPose, onLastFrame);

		const real dxTranslation = jointTranslation.x - bindPoseJointData->m_xTranslationKeys.front().m_keyData;
		addRealKey(m_currentFrameNumber, jointData->m_xTranslationKeys, dxTranslation, onLastFrame);

		const real dyTranslation = jointTranslation.y - bindPoseJointData->m_yTranslationKeys.front().m_keyData;
		addRealKey(m_currentFrameNumber, jointData->m_yTranslationKeys, dyTranslation, onLastFrame);

		const real dzTranslation = jointTranslation.z - bindPoseJointData->m_zTranslationKeys.front().m_keyData;
		addRealKey(m_currentFrameNumber, jointData->m_zTranslationKeys, dzTranslation, onLastFrame);
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::processTransformNode(const MDagPath &targetDagPath, CaptureMode captureMode)
{
	MESSENGER_INDENT;
	MStatus  status;

	//-- ensure node is a joint or transform
	const MFn::Type nodeType = targetDagPath.apiType(&status);
	MESSENGER_REJECT(!status, ("failed to get target dag node's node type\n"));
	MESSENGER_REJECT((nodeType != MFn::kJoint) && (nodeType != MFn::kTransform), ("tried to add non-joint, non-transform node as part of skeleton hierarchy\n"));

	//-- ignore node if indicated.
	const bool isIgnored = MayaUtility::ignoreNode(targetDagPath);
	if (!isIgnored)
	{
		//-- if the node is not a joint, process it only if it has a joint or mesh in its child hierarchy
		if (nodeType == MFn::kTransform)
		{
			// we ignore pure transforms unless they have a joint or a mesh below them in their hierarchy
			if (!MayaMisc::hasNodeTypeInHierarachy(targetDagPath, MFn::kJoint) && !MayaMisc::hasNodeTypeInHierarachy(targetDagPath, MFn::kMesh))
			{
				// skip this node
				return true;
			}
		}

		//-- capture data for the node
		bool stopTraversal = false;

		const bool captureSuccess = captureTransformData(targetDagPath, captureMode, stopTraversal);
		MESSENGER_REJECT(!captureSuccess, ("captureTransforData() failed.\n"));

		//-- check if we should stop further traversal down this path for some reason.
		if (stopTraversal)
			return true;
	}

	//-- handle joint's children
	const unsigned childCount = targetDagPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for joint\n"));

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

			const bool childSuccess = processTransformNode(childDagPath, captureMode);
			MESSENGER_REJECT(!childSuccess, ("failed to add child joint\n"));
		}
	}

	//-- we're done
	return true;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::performExport(const char *animationBaseName, int firstFrameNumber, int lastFrameNumber, const MDagPath &targetDagPath)
{
	MESSENGER_INDENT;
	MStatus status;

	MESSENGER_REJECT(lastFrameNumber < firstFrameNumber, ("bad frame number arguments, last [%d] < first [%d]\n", lastFrameNumber, firstFrameNumber));

	//-- before we do anything, reset the state of this object since we may be doing multiple exports with it.
	resetState();

	//-- specify joints we want to track for output
#if LOG_SPECIFIC_JOINT_OUTPUT
	ms_specificJointNameSet.clear();

	// IGNORE_RETURN(ms_specificJointNameSet.insert(boost::shared_ptr<CrcLowerString>(new CrcLowerString("LShoulder"))));
	// IGNORE_RETURN(ms_specificJointNameSet.insert(boost::shared_ptr<CrcLowerString>(new CrcLowerString("RShoulder"))));

	IGNORE_RETURN(ms_specificJointNameSet.insert(boost::shared_ptr<CrcLowerString>(new CrcLowerString("r_thigh"))));

#endif

	//-- set maya's range slider to the given range
	{
		MTime mayaTime;

		//-- set min time
		status = mayaTime.setValue(static_cast<double>(firstFrameNumber));
		MESSENGER_REJECT(!status, ("setValue(%.2f) failed: [%s]\n", static_cast<double>(firstFrameNumber), status.errorString().asChar()));

		status = MAnimControl::setMinTime(mayaTime);
		MESSENGER_REJECT(!status, ("setMinTime failed: [%s]\n", status.errorString().asChar()));

		//-- set max time
		status = mayaTime.setValue(static_cast<double>(lastFrameNumber));
		MESSENGER_REJECT(!status, ("setValue(%.2f) failed: [%s]\n", static_cast<double>(lastFrameNumber), status.errorString().asChar()));

		status = MAnimControl::setMaxTime(mayaTime);
		MESSENGER_REJECT(!status, ("setMaxTime failed: [%s]\n", status.errorString().asChar()));
	}

	//-- check for duplicate short names in selected skeleton hierarchy
	{
		MStringArray duplicateNames;

		const bool checkSuccess = MayaUtility::checkForDuplicateShortNames(targetDagPath, &duplicateNames, static_cast<int>(MFn::kTransform));
		MESSENGER_REJECT(!checkSuccess, ("check for duplicate short node names failed\n"));

		const unsigned duplicateNameCount = duplicateNames.length();
		if (duplicateNameCount)
		{
			MESSENGER_LOG(("selected skeleton has dulpicate short node names, please fix:\n"));

			MESSENGER_INDENT;
			for (unsigned i = 0; i < duplicateNameCount; ++i)
			{
				const MString &nodeName = duplicateNames[i];
				MESSENGER_LOG(("[%s]\n", nodeName.asChar()));
			}
			
			//-- aborting the export
			MESSENGER_LOG_ERROR(("duplicate short node names exist, aborting\n"));
			return false;
		}
	}

	//-- check if selected skeleton segment has master node
	MDagPath  masterDagPath = targetDagPath;

	const bool doLocomotionHandling = hasMasterNode(targetDagPath);
	{
		// the master node must be immediate DAG parent of the exported node.
		status = masterDagPath.pop();
		STATUS_REJECT(status, "failed to pop from root to master node.");
	}

	//-- initialize animation message collection
	bool amcConstructionStatus = false;

	AnimationMessageCollector  animationMessageCollector(targetDagPath, amcConstructionStatus);
	MESSENGER_REJECT(!amcConstructionStatus, ("failed to construct AnimationMessageCollector\n"));

	//-- setup C++ object variables
	m_selectedDagPath = targetDagPath;
	m_frameCount      = lastFrameNumber - firstFrameNumber;

	//-- make sure the target dag path is the one and only item selected
	//   note: if we don't do this, auto export won't be able to
	//   properly set the bind pose.
	MString  executeCommand("select -r ");
	MESSENGER_LOG(("setting skeleton [%s] to bind pose\n", targetDagPath.partialPathName().asChar()));
	executeCommand += targetDagPath.partialPathName().asChar();

	status = MGlobal::executeCommand(executeCommand, true, true);
	MESSENGER_REJECT(!status, ("failed to execute selection command\n"));

	//-- capture bind pose data
	const int alternateBindPoseFrame = -10;
	const bool gtbpSuccess = MayaUtility::goToBindPose(alternateBindPoseFrame);
	MESSENGER_REJECT(!gtbpSuccess, ("failed to go to bind pose\n"));

	const bool captureBpData = processTransformNode(targetDagPath, CM_bindPose);
	MESSENGER_REJECT(!captureBpData, ("failed to capture bind pose joint data\n"));

	const bool edSuccess = MayaUtility::enableDeformers();
	MESSENGER_REJECT(!edSuccess, ("failed to enable deformers after capturing bind pose data\n"));

	//-- capture frame data
	MTime mayaFrameTime;

	for (int frameNumber = firstFrameNumber; frameNumber <= lastFrameNumber; ++frameNumber)
	{
		m_currentFrameNumber = frameNumber - firstFrameNumber;

		//-- set the animation to this frame
		status = mayaFrameTime.setValue(static_cast<double>(frameNumber));
		MESSENGER_REJECT(!status, ("mayaFrameTime.setValue(%.2f) failed\n", static_cast<double>(frameNumber)));

		status = MAnimControl::setCurrentTime(mayaFrameTime);
		MESSENGER_REJECT(!status, ("MAnimControl.setCurrentTime() failed\n", static_cast<double>(frameNumber)));

		//-- capture frame's animation data
		const bool captureSuccess = processTransformNode(targetDagPath, CM_animation);
		MESSENGER_REJECT(!captureSuccess, ("processTransformNode failed\n"));

		//-- collect this animation frame's messages
		const bool cfmResult = animationMessageCollector.collectFrameMessageSignals(m_currentFrameNumber);
		MESSENGER_REJECT(!cfmResult, ("collectFrameMessageSignals() failed\n"));

		//-- capture frame's locomotion data
		if (doLocomotionHandling)
		{
			const bool cldResult = captureLocomotionData(masterDagPath);
			MESSENGER_REJECT(!cldResult, ("captureLocomotionData() failed for master node.\n"));
		}
	}

	//-- massage / simplify collected data
	bool const edcResult = animationMessageCollector.endDataCollection();
	MESSENGER_REJECT(!edcResult, ("error during animation message processing, aborting export.\n"));

	//-- create the writer
	KeyframeSkeletalAnimationTemplateWriter writer;

	//-- fill up the writer
	// set frames per second
	int framesPerSecond = 30;
	{
		switch (mayaFrameTime.unit())
		{
			case MTime::kFilm:      framesPerSecond = 24; break;
			case MTime::kGames:     framesPerSecond = 15; break;
			case MTime::kSeconds:   framesPerSecond = 1;  break;
			case MTime::kNTSCFrame: framesPerSecond = 30; break;

			case MTime::kInvalid:
			case MTime::kHours:
			case MTime::kMinutes:
			case MTime::kMilliseconds:
			case MTime::kPALFrame:
			case MTime::kShowScan:
			case MTime::kPALField:
			case MTime::kNTSCField:
			case MTime::kUserDef:
			case MTime::kLast:
			default:
				FATAL(true, ("unsupported frame size [%u]\n", mayaFrameTime.unit()));
		}

		writer.setFramesPerSecond(static_cast<real>(framesPerSecond));
		
		// write frame count
		writer.setAnimationFrameCount(m_frameCount);

		// loop over all joint data
		JointDataContainer::Container::iterator             it    = m_animationJointData->m_container.begin();
		const JointDataContainer::Container::const_iterator itEnd = m_animationJointData->m_container.end();
		for (; it != itEnd; ++it)
		{
			const CrcLowerString *jointName = (*it).first;
			const JointData      *jointData = NON_NULL((*it).second);

			KeyframeSkeletalAnimationTemplateWriter::TransformData &transformData = writer.addTransform(jointName->getString());

			//-- write rotation data
			{
				const size_t keyCount = jointData->m_rotationKeys.size();
				MESSENGER_REJECT(!keyCount, ("no keys for rotation on [%s]\n", jointData->m_name.getString()));
				if (keyCount == 1)
				{
					// only one key, make it static
					writer.setStaticRotation(transformData, jointData->m_rotationKeys.front().m_keyData);
				}
				else
				{
					// multiple keys, add them now
					KeyframeSkeletalAnimationTemplateWriter::RotationChannelData &channelData = writer.addAnimatedRotation(transformData);
					for (size_t i = 0; i < keyCount; ++i)
					{
						const QuaternionKeyData &keyData = jointData->m_rotationKeys[i];
						writer.addRotationKey(channelData, keyData.m_frameNumber, keyData.m_keyData);
					}
				}
			}

			//-- write x translation data
			{
				const size_t keyCount = jointData->m_xTranslationKeys.size();
				MESSENGER_REJECT(!keyCount, ("no keys for x translation on [%s]\n", jointData->m_name.getString()));
				if (keyCount == 1)
				{
					// only one key, make it static
					writer.setStaticTranslationX(transformData, jointData->m_xTranslationKeys[0].m_keyData);
				}
				else
				{
					// multiple keys, add them now
					KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &channelData = writer.addChannelTranslationX(transformData);
					for (size_t i = 0; i < keyCount; ++i)
					{
						const RealKeyData &keyData = jointData->m_xTranslationKeys[i];
						writer.addTranslationKey(channelData, keyData.m_frameNumber, keyData.m_keyData);
					}
				}
			}

			//-- write y translation data
			{
				const size_t keyCount = jointData->m_yTranslationKeys.size();
				MESSENGER_REJECT(!keyCount, ("no keys for y translation on [%s]\n", jointData->m_name.getString()));
				if (keyCount == 1)
				{
					// only one key, make it static
					writer.setStaticTranslationY(transformData, jointData->m_yTranslationKeys[0].m_keyData);
				}
				else
				{
					// multiple keys, add them now
					KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &channelData = writer.addChannelTranslationY(transformData);
					for (size_t i = 0; i < keyCount; ++i)
					{
						const RealKeyData &keyData = jointData->m_yTranslationKeys[i];
						writer.addTranslationKey(channelData, keyData.m_frameNumber, keyData.m_keyData);
					}
				}
			}

			//-- write z translation data
			{
				const size_t keyCount = jointData->m_zTranslationKeys.size();
				MESSENGER_REJECT(!keyCount, ("no keys for z translation on [%s]\n", jointData->m_name.getString()));
				if (keyCount == 1)
				{
					// only one key, make it static
					writer.setStaticTranslationZ(transformData, jointData->m_zTranslationKeys[0].m_keyData);
				}
				else
				{
					// multiple keys, add them now
					KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &channelData = writer.addChannelTranslationZ(transformData);
					for (size_t i = 0; i < keyCount; ++i)
					{
						const RealKeyData &keyData = jointData->m_zTranslationKeys[i];
						writer.addTranslationKey(channelData, keyData.m_frameNumber, keyData.m_keyData);
					}
				}
			}
		}
	}

	//-- write locomotion rotation data
	if (m_locomotionRotationKeys && (m_locomotionRotationKeys->size() > 1))
	{
		KeyframeSkeletalAnimationTemplateWriter::RotationChannelData &channelData = writer.getLocomotionRotationChannelData();

		const QuaternionKeyDataVector::iterator endIt = m_locomotionRotationKeys->end();
		for (QuaternionKeyDataVector::iterator it = m_locomotionRotationKeys->begin(); it != endIt; ++it)
			writer.addRotationKey(channelData, it->m_frameNumber, it->m_keyData);
	}

	//-- write locomotion translation data
	if (m_locomotionTranslationKeys && (m_locomotionTranslationKeys->size() > 1))
	{
		//-- compute average speed
		// get delta distance vector
		const Vector deltaDistanceVector = m_locomotionTranslationKeys->back().m_keyData - m_locomotionTranslationKeys->front().m_keyData;

		// set average speed as delta distance vector divided by frame time
		const float averageTranslationSpeed = deltaDistanceVector.magnitude() / ( static_cast<float>(lastFrameNumber - firstFrameNumber) / static_cast<float>(framesPerSecond));
		writer.setAverageTranslationSpeed(averageTranslationSpeed);

		KeyframeSkeletalAnimationTemplateWriter::VectorChannelData &channelData = writer.getLocomotionTranslationChannelData();

		const VectorKeyDataVector::iterator endIt = m_locomotionTranslationKeys->end();
		for (VectorKeyDataVector::iterator it = m_locomotionTranslationKeys->begin(); it != endIt; ++it)
			writer.addVectorKey(channelData, it->m_frameNumber, it->m_keyData);
	}

	//-- add animation message data to writer
	const bool wamResult = writeAnimationMessageData(writer, animationMessageCollector);
	MESSENGER_REJECT(!wamResult, ("writeAnimationMessageData failed\n"));

	//-- write out the iff
	Iff  iff(cms_iffSize);

	writer.write(iff);
	MESSENGER_LOG(("STATS: keyframe skeletal animation template size: %d bytes\n", iff.getRawDataSize()));
	MESSENGER_LOG(("STATS: animation frame count:                     %d\n", m_frameCount));
	MESSENGER_LOG(("STATS: animation frames per second:               %d\n", framesPerSecond));
	MESSENGER_LOG(("STATS: animation transform count:                 %u\n", m_animationJointData->m_container.size()));

	//-- construct animation output filename
	char outputPath[MAX_PATH];

	// set the directory
	strcpy(outputPath, SetDirectoryCommand::getDirectoryString(ANIMATION_WRITE_DIR_INDEX));
	const size_t directoryLength = strlen(outputPath);
	if (outputPath[directoryLength-1] != '\\')
		strcat(outputPath, "\\");

	// add the base animation name
	strcat(outputPath, animationBaseName);
	strcat(outputPath, ".ans");

	//-- write iff to file
	const bool writeSuccess = iff.write(outputPath, true);
	MESSENGER_REJECT(!writeSuccess, ("failed to write file [%s]\n", outputPath));

	MESSENGER_LOG(("successfully wrote [%s]\n", outputPath));

	ExporterLog::addSkeletalAnimation(outputPath);

	return MStatus();
}

// ----------------------------------------------------------------------

MStatus ExportKeyframeSkeletalAnimation::doIt(const MArgList &args)
{
	messenger->clearWarningsAndErrors();

	MESSENGER_INDENT;

	//-- setup export based on arguments
	MString              animationOutputDirectory;
	MString              appearanceWriteDirectory;
	MString              authorName;
	MString              branch;
	std::string          newLogFilename;
	AnimationInfoVector  animations;
	bool                 userAbort             = false;
	bool                 commitToSourceControl = false;
	bool                 createNewChangelist   = false;
	bool                 interactive           = false;
	bool                 lock                  = false;
	bool                 unlock                = false;
	bool                 disableCompression    = false;

	//-- install the exporter log
	ExporterLog::install (messenger);
	ExporterLog::setSourceFilename (MFileIO::currentFile ().asChar());
	ExporterLog::setMayaCommand("exportSkeletalAnimation");

	appearanceWriteDirectory = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);

	const bool processSuccess = processArguments( 
		                                          args, 
		                                          &animationOutputDirectory, 
												  &authorName, 
												  &animations, 
												  userAbort, 
												  interactive, 
												  commitToSourceControl, 
												  createNewChangelist, 
												  lock, 
												  unlock, 
												  branch,
												  disableCompression
												  );
	if (!processSuccess)
	{
		MESSENGER_LOG_ERROR(("argument processing failed\n"));
		std::for_each(animations.begin(), animations.end(), PointerDeleter());
		return MStatus(MStatus::kFailure);
	}

	if (userAbort)
	{
		MESSENGER_LOG_ERROR(("user aborted operation\n"));
		return MStatus(MStatus::kSuccess);
	}

	ExporterLog::setAuthor (authorName.asChar());

	//get, store the base directory
	std::string baseDir = appearanceWriteDirectory.asChar();
	std::string::size_type pos = baseDir.find_last_of("appearance");
	FATAL(static_cast<int>(pos) == std::string::npos, ("malformed filename in ExportKeyframeSkeletalAnimation::doIt"));
	baseDir = baseDir.substr(0, pos-strlen("appearance"));
	baseDir += "\\";
	ExporterLog::setBaseDir(baseDir);

	//-- set the command strings
	SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, authorName.asChar());
	SetDirectoryCommand::setDirectoryString(ANIMATION_WRITE_DIR_INDEX, animationOutputDirectory.asChar());

	//-- sort animations by first frame
	std::sort(animations.begin(), animations.end(), MayaAnimationList::AnimationInfo::LessFirstFrame());

	// set compression
	KeyframeSkeletalAnimationTemplateWriter::setCompressionMode(!disableCompression);
	//-- do the export
	{
		const AnimationInfoVector::const_iterator itEnd = animations.end();
		for (AnimationInfoVector::const_iterator it = animations.begin(); it != itEnd; ++it)
		{
			const MayaAnimationList::AnimationInfo &animationInfo = *NON_NULL(*it);

			//-- get the target dag path from the specified node name
			MDagPath  targetDagPath;

			const bool getDagSuccess = MayaUtility::getDagPathFromName(animationInfo.m_exportNodeName.c_str(), &targetDagPath);

			//-- perform the export for this animation
			bool performSuccess = false;

			if (getDagSuccess)
				performSuccess = performExport(animationInfo.m_animationName.getString(), animationInfo.m_firstFrame, animationInfo.m_lastFrame, targetDagPath);
			if (!getDagSuccess || !performSuccess )
			{
				if (!getDagSuccess)
					MESSENGER_LOG_ERROR(("failed to get dag path for node [%s] -- is it in the skeleton?\n", animationInfo.m_exportNodeName.c_str()));
					
				if (!performSuccess)
					MESSENGER_LOG_ERROR(("failed to export\n"));

				std::for_each(animations.begin(), animations.end(), PointerDeleter());
				return MStatus(MStatus::kFailure);
			}

			//-- create parameters needed to reexport this
			// find skeleton for the exported node
			MString    componentValue;
			MString    skeletonNodeName;

			bool       foundIt     = false;
			const bool fawncResult = MayaUtility::findAncestorWithNameComponent(targetDagPath, 1, &componentValue, &foundIt, &skeletonNodeName, 0);
			MESSENGER_REJECT_STATUS(!fawncResult || !foundIt, ("failed to find skeleton for node [%s]\n", targetDagPath.partialPathName().asChar()));

			std::string  reexportArguments(ExportArgs::cs_skeletonArgName.asChar());
			reexportArguments += " ";
			reexportArguments += skeletonNodeName.asChar();
			reexportArguments += " ";
			reexportArguments += ExportArgs::cs_nameArgName.asChar();
			reexportArguments += " ";
			reexportArguments += animationInfo.m_animationName.getString();
			if(commitToSourceControl)
			{
				reexportArguments += " ";
				reexportArguments += ExportArgs::cs_branchArgName.asChar();
				reexportArguments += " ";
				reexportArguments += branch.asChar();
			}
			ExporterLog::setMayaExportOptions(reexportArguments);

			std::string shortLogFilename = animationInfo.m_animationName.getString();
			shortLogFilename += ".log";
			IGNORE_RETURN(ExporterLog::loadLogFile(shortLogFilename));

			newLogFilename = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
			newLogFilename += shortLogFilename;

			if (commitToSourceControl || lock || unlock)
			{
// JU_TODO: alienbrain def out
#if 0
				if (!AlienbrainImporter::connectToServer())
				{
					MESSENGER_LOG_ERROR(("Unable to connect to Alienbrain\n"));
					return MStatus::kFailure;
				}

				bool result = AlienbrainImporter::preImport(ExportSkeleton::getSkeletonName(targetDagPath).asChar(), interactive);
				IGNORE_RETURN(ExporterLog::writeSkeletalAnimation (newLogFilename.c_str(), interactive));
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
				
				IGNORE_RETURN(ExporterLog::writeSkeletalAnimation (newLogFilename.c_str(), interactive));

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
				IGNORE_RETURN(ExporterLog::writeSkeletalAnimation (newLogFilename.c_str(), interactive));
			}
		}
	}

	// reset compression
	KeyframeSkeletalAnimationTemplateWriter::setCompressionMode(true);

	//-- clean up
	ExporterLog::remove ();
	std::for_each(animations.begin(), animations.end(), PointerDeleter());

	messenger->printWarningsAndErrors();

	// JU_TODO: more conditions on this success messagebox
    std::stringstream text;
    text << "Animations Export Complete." << (disableCompression ? " [COMPRESSION DISABLED]" : "") << std::endl;
    messenger->getWarningAndErrorText(text);
    text << std::ends;

	MESSENGER_MESSAGE_BOX(NULL,text.str().c_str(),"Export",MB_OK);


	return MStatus(MStatus::kSuccess);
}

// ======================================================================
// class ExportKeyframeSkeletalAnimation: private static member functions
// ======================================================================

void ExportKeyframeSkeletalAnimation::addRealKey(int frameNumber, RealKeyDataVector &keys, real keyData, bool lastFrame)
{
	//-- only add keydata if it is not the same as last key data
	bool writeData = true;

	const real epsilon = CONST_REAL(0.0005);

	if (!keys.empty())
	{
		const real lastKeyframeData = keys.back().m_keyData;

		const real delta = keyData - lastKeyframeData;
		writeData = (delta > epsilon) || (delta < -epsilon);

		//-- Make sure we cap off the end of a constant region of the same value;
		//   otherwise we'll lerp over a range where we should be constant.
		if (writeData)
		{
			if (keys.back().m_frameNumber != (frameNumber - 1))
			{
				// This frame has a different value than the last key, and the last key did not
				// come from the last frame.  This means the last key held a constant value until
				// last frame.  To prevent us from lerping over that range, we want to add a key
				// for that value for last frame.
				keys.push_back(RealKeyData(frameNumber - 1, lastKeyframeData));
			}
		}
	}

	//-- if this is the last frame in the animation and if we have more than one keyframe total,
	//   we absolutely want to capture the key.  In this case, we'll want to capture a key on the last frame so
	//   we can loop or play through the end of the animation.  If there is only one key so far,
	//   it implies there's a very good chance the data is static and we don't need to add it.
	//   the check above will already see if the data is different, and if so will set the writeData
	//   flag to true.  for key counts two or greater, insist on a keyframe.
	if (lastFrame && (keys.size() > 1))
		writeData = true;

	if (writeData)
		keys.push_back(RealKeyData(frameNumber, keyData));
}

// ----------------------------------------------------------------------

void ExportKeyframeSkeletalAnimation::addRotationKey(int frameNumber, QuaternionKeyDataVector &keys, const Quaternion &keyData, bool lastFrame)
{
	//-- only add keydata if it is not the same as last key data
	bool writeData = true;

	const real epsilon = CONST_REAL(0.001);

	if (!keys.empty())
	{
		const Quaternion &lastRotation = keys.back().m_keyData;

		writeData =
			(fabs(static_cast<double>(keyData.w - lastRotation.w)) > epsilon) ||
			(fabs(static_cast<double>(keyData.x - lastRotation.x)) > epsilon) ||
			(fabs(static_cast<double>(keyData.y - lastRotation.y)) > epsilon) ||
			(fabs(static_cast<double>(keyData.z - lastRotation.z)) > epsilon);

		//-- Make sure we cap off the end of a constant region of the same value;
		//   otherwise we'll lerp over a range where we should be constant.
		if (writeData)
		{
			if (keys.back().m_frameNumber != (frameNumber - 1))
			{
				// This frame has a different value than the last key, and the last key did not
				// come from the last frame.  This means the last key held a constant value until
				// last frame.  To prevent us from lerping over that range, we want to add a key
				// for that value for last frame.
				keys.push_back(QuaternionKeyData(frameNumber - 1, lastRotation));
			}
		}

	}

	//-- if this is the last frame in the animation and if we have more than one keyframe total,
	//   we absolutely want to capture the key.  In this case, we'll want to capture a key on the last frame so
	//   we can loop or play through the end of the animation.  If there is only one key so far,
	//   it implies there's a very good chance the data is static and we don't need to add it.
	//   the check above will already see if the data is different, and if so will set the writeData
	//   flag to true.  for key counts two or greater, insist on a keyframe.
	if (lastFrame && (keys.size() > 1))
	{
		writeData = true;
	}

	if (writeData)
	{
		keys.push_back(QuaternionKeyData(frameNumber, keyData));
	}
}

// ----------------------------------------------------------------------

void ExportKeyframeSkeletalAnimation::addVectorKey(int frameNumber, VectorKeyDataVector &keys, const Vector &keyData, bool lastFrame)
{
	//-- only add keydata if it is not the same as last key data
	bool writeData = true;

	//-- limit locomotion to 1/2 centimeter
	const real epsilon = CONST_REAL(0.005);

	if (!keys.empty())
	{
		const Vector &lastKeyframeData = keys.back().m_keyData;

		writeData = 
			(fabs(static_cast<double>(keyData.x - lastKeyframeData.x)) > epsilon) ||
			(fabs(static_cast<double>(keyData.y - lastKeyframeData.y)) > epsilon) ||
			(fabs(static_cast<double>(keyData.z - lastKeyframeData.z)) > epsilon);
	}

	//-- if this is the last frame in the animation and if we have more than one keyframe total,
	//   we absolutely want to capture the key.  In this case, we'll want to capture a key on the last frame so
	//   we can loop or play through the end of the animation.  If there is only one key so far,
	//   it implies there's a very good chance the data is static and we don't need to add it.
	//   the check above will already see if the data is different, and if so will set the writeData
	//   flag to true.  for key counts two or greater, insist on a keyframe.
	if (lastFrame && (keys.size() > 1))
		writeData = true;

	if (writeData)
		keys.push_back(VectorKeyData(frameNumber, keyData));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the rotation required to achieve a given sum rotation when
 * the rotation is post-multiplied to an initial rotation.
 *
 * The result of this function, post multiplied to initialRotation, should
 * be equivalent to applying only the sumRotation.
 *
 * @param initialRotation  the result of post multiplying the result to
 *                         this argument should equal the given sumRotation.
 * @param sumRotation      the result of post multiplying the result to
 *                         initialRotation should be equivalent to applying
 *                         only this argument.
 *
 * @return  the rotation such that the Quaternion product (return * initialRotation) 
 *          is identical to sumRotation.
 */

Quaternion ExportKeyframeSkeletalAnimation::getDeltaRotation(const Quaternion &initialRotation, const Quaternion &sumRotation)
{
	return sumRotation * initialRotation.getComplexConjugate();
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::getTransformRotation(const MFnTransform &fnTransform, Quaternion &rotation)
{
	//-- get transform's rotation
	// get maya joint rotation
	MEulerRotation mayaJointRotation;

	const MStatus status = fnTransform.getRotation(mayaJointRotation);
	MESSENGER_REJECT(!status, ("failed to get joint rotation\n"));

	// ensure it's what we expect
	MESSENGER_REJECT(mayaJointRotation.order != MEulerRotation::kXYZ, ("FIX: joint must be in joint rotation order XYZ but is not.\n"));

	//-- convert Maya rotation to game rotation
	const Vector jointRotation = MayaConversions::convertRotationAsVector(mayaJointRotation);

	//-- convert Euler rotation to Quaternion
	const Quaternion qX = Quaternion(jointRotation.x, Vector::unitX);
	const Quaternion qY = Quaternion(jointRotation.y, Vector::unitY);
	const Quaternion qZ = Quaternion(jointRotation.z, Vector::unitZ);

	rotation = qZ * (qY * qX);

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::writeAnimationMessageData(KeyframeSkeletalAnimationTemplateWriter &writer, const AnimationMessageCollector &animationMessageCollector)
{
	MESSENGER_INDENT;

	const int messageCount = animationMessageCollector.getMessageCount();
	MESSENGER_LOG(("animation contained %d messages:\n", messageCount));

	for (int i = 0; i < messageCount; ++i)
	{
		MESSENGER_INDENT;

		//-- skip any animation message that is not signaled during this animation
		const IntVector &signalFrameNumbers = animationMessageCollector.getMessageSignalFrameNumbers(i);
		if (signalFrameNumbers.empty())
			continue;

		//-- add animation message
		const CrcLowerString &messageName = animationMessageCollector.getMessageName(i);
		writer.beginAnimationMessage(messageName);
		{
			MESSENGER_LOG(("message [%s] signaled %u times.\n", messageName.getString(), signalFrameNumbers.size()));

			const IntVector::const_iterator  endIt = signalFrameNumbers.end();
			for (IntVector::const_iterator it = signalFrameNumbers.begin(); it != endIt; ++it)
				writer.addAnimationMessageSignal(*it);
		}
		writer.endAnimationMessage();
	}

	return true;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::getSelectedAnimations(AnimationInfoVector *animations, bool *userAbort)
{
	NOT_NULL(animations);

	MStatus status;

	//-- get the skeleton root node (might be the node selected, might be an ancestor)
	MDagPath  skeletonRootDagPath;
	const bool gsrResult = MayaUtility::getSkeletonRootDagPath(&skeletonRootDagPath);
	MESSENGER_REJECT(!gsrResult, ("failed to get skeleton root\n"));

	//-- retrieve animation list associated with root node
	MayaAnimationList animationList(skeletonRootDagPath, &status);
	MESSENGER_REJECT(!status, ("failed to create animation list with storage at skeleton root dag path [%s]\n", skeletonRootDagPath.partialPathName().asChar()));

	const bool sgSuccess = showGUI();

	bool returnValue;

	if (sgSuccess)
	{
		//-- successful operation
		{
			*userAbort = false;

			//-- copy selected AnimationInfo objects into returned animation vector
			//AnimationInfoMap &animationInfoMap = animationList.getAnimationInfoMap();

			VisitAnimationCommand::SelectionContainer sc = VisitAnimationCommand::m_selectedAnimations;

			const VisitAnimationCommand::SelectionContainer::iterator itEnd = sc.end();
			for (VisitAnimationCommand::SelectionContainer::iterator it = sc.begin(); it != itEnd; ++it)
			{
				AnimationInfoMap::iterator        selectedAnimationIt = *it;
				MayaAnimationList::AnimationInfo *sourceAnimationInfo = NON_NULL(selectedAnimationIt->second);

				animations->push_back(new MayaAnimationList::AnimationInfo(*sourceAnimationInfo));
			}
		}
		returnValue = true;
	}
	else
	{
		*userAbort = true;
		returnValue = true;
	}

	//-- cleanup
	delete VisitAnimationCommand::m_animationList;
	VisitAnimationCommand::m_animationList = 0;

	return returnValue;
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::processArguments(
	const MArgList      &args, 
	MString             *animationOutputDirectory, 
	MString             *authorName, 
	AnimationInfoVector *animations,
	bool                &userAbort,
	bool                &interactive,
	bool                &commitToSourceControl,
	bool                &createNewChangelist,
	bool                &lock,
	bool                &unlock,
	MString             &branch,
	bool                &disableCompression
	)
{
	NOT_NULL(animationOutputDirectory);
	NOT_NULL(authorName);
	NOT_NULL(animations);

	MESSENGER_INDENT;
	MStatus  status;

	const unsigned argCount = args.length(&status);
	MESSENGER_REJECT(!status, ("failed to get args length\n"));
	MESSENGER_REJECT(!argCount, ("ExportKeyframeSkeletalAnimation: missing args\n"));

	//-- variables used for non-interactive export
	std::auto_ptr<MayaAnimationList> animationList;

	//-- handle each argument
	bool haveAuthor          = false;
	bool haveOutputDirectory = false;
	bool isInteractive       = false;
	bool haveSkeleton        = false;
	bool haveBranch          = false;

	userAbort                = false;
	interactive              = false;
	commitToSourceControl    = false;
	createNewChangelist      = false;
	lock                     = false;
	unlock                   = false;

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
			interactive = true;
		}
		else if (argName == ExportArgs::cs_silentArgName)
		{
			// silent mode - disable message box feedback (logs messages instead)
			messenger->startSilentExport();
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

			*animationOutputDirectory = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get filename argument\n"));

			// fixup argIndex
			++argIndex;
			haveOutputDirectory = true;
		}
		else if (argName == ExportArgs::cs_skeletonArgName)
		{
			// specify skeleton to use for following -name <animation name> arguments (non-interactive)
			// note: multiple -skeleton arguments can be given.
			MESSENGER_REJECT(isInteractive, ("cannot specify skeleton when in interactive mode"));
			haveSkeleton = true;

			MString nodeName = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get skeleton node name argument\n"));

			// fixup argIndex
			++argIndex;

			// search for node in Maya scene
			MSelectionList  nodeList;
			status = MGlobal::getSelectionListByName(nodeName, nodeList);
			MESSENGER_REJECT(!status, ("MGlobal::getSelectionListByName failure"));

			MESSENGER_REJECT(nodeList.length() < 1, ("no scene nodes match specified skeleton node [%s]\n", nodeName.asChar()));
			MESSENGER_REJECT(nodeList.length() > 1, ("multiple nodes match specified skeleton node [%s]\n", nodeName.asChar()));

			MDagPath  skeletonDagPath;
			status = nodeList.getDagPath(0, skeletonDagPath);
			MESSENGER_REJECT(!status, ("failed to get dag path for skeleton node [%s]\n", nodeName.asChar()));

			//-- load animations for node
			if (animationList.get())
			{
				delete animationList.get();
			}

			animationList.reset(new MayaAnimationList(skeletonDagPath, &status));
			MESSENGER_REJECT(!status, ("failed to initialize animation list storage from dag path [%s]\n", skeletonDagPath.partialPathName().asChar()));
		}
		else if (argName == ExportArgs::cs_nameArgName)
		{
			// specify the name of an animation to export from the currently selected skeleton (non-interactive)
			// see -skeleton.
			MESSENGER_REJECT(!haveSkeleton, ("cannot specify animation name when animation's skeleton hasn't been specified"));
			haveSkeleton = true;

			//-- get animation name
			MString animationName = args.asString(argIndex + 1, &status);
			MESSENGER_REJECT(!status, ("failed to get animation name argument\n"));

			// fixup argIndex
			++argIndex;

			//-- find animation info for specified name
			const CrcLowerString crcAnimationName(animationName.asChar());
			const MayaAnimationList::AnimationInfoMap &animationInfoMap = animationList->getAnimationInfoMap();

			MayaAnimationList::AnimationInfoMap::const_iterator findIt = animationInfoMap.find(&crcAnimationName);
			if (findIt == animationInfoMap.end())
			{
				// default behavior is to fail the whole process when animation name is missing
				MESSENGER_LOG(("animation [%s] not known for current skeleton\n", crcAnimationName.getString()));
				return MStatus(MStatus::kFailure);
			}

			//-- add copy of animation info to our return vector of animations for export
			const MayaAnimationList::AnimationInfo &animationInfo = *NON_NULL(findIt->second);
			animations->push_back(new MayaAnimationList::AnimationInfo(animationInfo));
		}
		else if (argName == ExportArgs::cs_allArgName)
		{
			// export all animations for the currently specified skeleton (non-interactive)
			MESSENGER_REJECT(!haveSkeleton, ("cannot specify -all (all animations) when animation's skeleton hasn't been specified"));
			haveSkeleton = true;

			// add all animations for export
			const MayaAnimationList::AnimationInfoMap &animationInfoMap     = animationList->getAnimationInfoMap();
			const MayaAnimationList::AnimationInfoMap::const_iterator itEnd = animationInfoMap.end();
			for (MayaAnimationList::AnimationInfoMap::const_iterator it = animationInfoMap.begin(); it != itEnd; ++it)
			{
				const MayaAnimationList::AnimationInfo &animationInfo = *NON_NULL(it->second);
				animations->push_back(new MayaAnimationList::AnimationInfo(animationInfo));
			}
		}
		else if (argName == ExportArgs::cs_createNewChangelistArgName)
		{
			createNewChangelist = true;
		}
		else if (argName == ExportArgs::cs_disableCompression)
		{
			disableCompression = true;
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
		const bool gsaResult = getSelectedAnimations(animations, &userAbort);
		MESSENGER_REJECT(!gsaResult, ("failed to get selected animations\n"));
	}

	//-- handle default author specification
	if (!haveAuthor)
	{
		// get from author command
		status = authorName->set(SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX));
		MESSENGER_REJECT(!status, ("failed to set authorName string with [%s]\n", SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX)));
		MESSENGER_LOG(("ARGS: fetched default author name [%s]\n", authorName->asChar()));
	}

	//-- handle default output directory specification
	if (!haveOutputDirectory)
	{
		// get from skeleton template write directory command
		status = animationOutputDirectory->set(SetDirectoryCommand::getDirectoryString(ANIMATION_WRITE_DIR_INDEX));
		MESSENGER_REJECT(!status, ("failed to set animationOutputDirectory string with [%s]\n", SetDirectoryCommand::getDirectoryString(ANIMATION_WRITE_DIR_INDEX)));
		MESSENGER_LOG(("ARGS: fetched default output directory name [%s]\n", animationOutputDirectory->asChar()));
	}

	MESSENGER_REJECT(commitToSourceControl && !haveBranch, ("no branch, i.e. \"-branch <branchname>\" was specified\n"));
	MESSENGER_REJECT(!isInteractive && !haveSkeleton, ("-interactive not specified, expecting -skeleton <skeleton name> and -name <animation name> args\n"));

	return MStatus();
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::showGUI()
{
	MStatus status;

	int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_SELECT_ANIMATIONS), NULL, VisitAnimationCommand::visitAnimationDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")
	if(result == IDOK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool ExportKeyframeSkeletalAnimation::hasMasterNode(const MDagPath &rootNode)
{
	MStatus  status;

	//-- check if we have a parent
	const unsigned int rootDagLength = rootNode.length(&status);
	STATUS_REJECT(status, "failed to find length of root dag node, assuming no master node.\n");

	if (rootDagLength < 1)
	{
		// no parent, this root can't have a master node.
		return false;
	}

	//-- target master node is the root's parent
	MDagPath  masterNode = rootNode;

	status = masterNode.pop();
	STATUS_REJECT(status, "failed to pop root node even though length > 0.\n");

	//-- if lowercase name is "master", we've got a master node.
	MFnDagNode  masterDagNode(masterNode, &status);
	STATUS_REJECT(status, "failed to set MFnDagNode for master node.\n");

	MString name = masterDagNode.name();
	IGNORE_RETURN(name.toLowerCase());

	return (name == cs_masterNodeName);
}

// ======================================================================
