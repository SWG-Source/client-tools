// ======================================================================
//
// ExportKeyframeSkeletalAnimation.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef EXPORT_KEYFRAME_SKELETAL_ANIMATION_H
#define EXPORT_KEYFRAME_SKELETAL_ANIMATION_H

// ======================================================================

#include "maya/mDagPath.h"
#include "maya/MPxCommand.h"
#include "maya/MStatus.h"
#include "MayaAnimationList.h"

#include <map> //use in iterator in this file, so can't forward declare it

class AnimationMessageCollector;
class KeyframeSkeletalAnimationTemplateWriter;
class MArgList;
class Messenger;
class MFnTransform;
class MString;
class Quaternion;
class Transform;
class Vector;

// ======================================================================

class ExportKeyframeSkeletalAnimation: public MPxCommand
{
public:

	struct QuaternionKeyData;
	struct RealKeyData;

public:

	static void install(Messenger *messenger);
	static void remove(void);

	static void *creator(void);

public:

	ExportKeyframeSkeletalAnimation();
	~ExportKeyframeSkeletalAnimation();

	MStatus doIt(const MArgList &args);

private:

	struct JointData;
	struct JointDataContainer;
	struct VectorKeyData;

	typedef MayaAnimationList::AnimationInfoMap                AnimationInfoMap;
	typedef stdvector<AnimationInfoMap::iterator>::fwd         SelectionContainer;
	typedef stdvector<MayaAnimationList::AnimationInfo*>::fwd  AnimationInfoVector;
	typedef stdvector<QuaternionKeyData>::fwd                  QuaternionKeyDataVector;
	typedef stdvector<RealKeyData>::fwd                        RealKeyDataVector;
	typedef stdvector<VectorKeyData>::fwd                      VectorKeyDataVector;

	enum CaptureMode
	{
		CM_bindPose,
		CM_animation
	};

private:

	static void        addRealKey(int frameNumber, RealKeyDataVector &keys, real keyData, bool lastFrame);
	static void        addRotationKey(int frameNumber, QuaternionKeyDataVector &keys, const Quaternion &keyData, bool lastFrame);
	static void        addVectorKey(int frameNumber, VectorKeyDataVector &keys, const Vector &keyData, bool lastFrame);

	static Quaternion  getDeltaRotation(const Quaternion &initialRotation, const Quaternion &sumRotation);
	static bool        getTransformRotation(const MFnTransform &fnTransform, Quaternion &rotation);
	static bool        writeAnimationMessageData(KeyframeSkeletalAnimationTemplateWriter &writer, const AnimationMessageCollector &animationMessageCollector);
	static bool        getSelectedAnimations(AnimationInfoVector *animations, bool *userAbort);
	static bool        processArguments(
		const MArgList &args, 
		MString *animationOutputDirectory, 
		MString *authorName, 
		AnimationInfoVector *animations, 
		bool &userAbort, 
		bool &interactive, 
		bool &commitToAlienbrain, 
		bool &createNewChangelist, 
		bool &lock, 
		bool &unlock, 
		MString &branch,
		bool &disableCompression
		);

	static bool        showGUI();
	static bool        hasMasterNode(const MDagPath &rootNode);

private:

	void  resetState();

	bool  captureLocomotionData(const MDagPath &targetDagPath);
	bool  captureTransformData(const MDagPath &targetDagPath, CaptureMode captureMode, bool &stopTraversal);
	bool  processTransformNode(const MDagPath &targetDagPath, CaptureMode captureMode);
	bool  performExport(const char *animationBaseName, int firstFrameNumber, int lastFrameNumber, const MDagPath &targetDagPath);


private:

	static const int  cms_iffSize;

private:

	JointDataContainer      *m_bindPoseJointData;
	JointDataContainer      *m_animationJointData;
	int                      m_currentFrameNumber;
	int                      m_frameCount;
	MDagPath                 m_selectedDagPath;

	QuaternionKeyDataVector *m_locomotionRotationKeys;
	VectorKeyDataVector     *m_locomotionTranslationKeys;

private:

	// disabled
	ExportKeyframeSkeletalAnimation(const ExportKeyframeSkeletalAnimation&);
	ExportKeyframeSkeletalAnimation &operator =(const ExportKeyframeSkeletalAnimation&);

};

// ======================================================================

#endif
