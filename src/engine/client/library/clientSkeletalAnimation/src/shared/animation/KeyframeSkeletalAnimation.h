// ======================================================================
//
// KeyframeSkeletalAnimation.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_KeyframeSkeletalAnimation_H
#define INCLUDED_KeyframeSkeletalAnimation_H

// ======================================================================

// NOTE: I am loading up include dependencies here so I can reduce internal
//       allocations.  I think this is acceptable because most code dealing
//       with animations deals with the abstract base class SkeletalAnimation.

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include <vector>

class AnimationEnvironment;
class MemoryBlockManager;

// ======================================================================

class KeyframeSkeletalAnimation: public SkeletalAnimation
{
friend class KeyframeSkeletalAnimationTemplate;

public:

	class QuaternionOperation;
	class VectorOperation;

public:

	static void install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

public:

	const KeyframeSkeletalAnimationTemplate *getOurTemplate() const;

	virtual bool                     alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	virtual void                     startNewCycle();

	virtual int                      getTransformCount() const;
	virtual void                     evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation);

	virtual int                      getTransformPriority(int index) const;
	virtual int                      getLocomotionPriority() const;

	virtual void                     getScaledLocomotion(Quaternion &rotation, Vector &translation) const;

	virtual float                    getCycleScaledLocomotionDistance() const;
	virtual int                      getFrameCount() const;
	virtual float                    getRecordedFramesPerSecond() const;

	virtual void                     setPlaybackFramesPerSecond(float playbackFramesPerSecond);
	virtual float                    getPlaybackFramesPerSecond() const;

	virtual int                      getMessageCount() const;
	virtual const CrcLowerString    &getMessageName(int index) const;
	virtual void                     getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const;

	virtual SkeletalAnimation       *resolveSkeletalAnimation();

private:

	class StaticQuaternionOperation;
	friend class StaticQuaternionOperation;

	class StaticVectorOperation;
	friend class StaticVectorOperation;

	typedef stdvector<QuaternionOperation*>::fwd  QuaternionOperationVector;
	typedef stdvector<VectorOperation*>::fwd      VectorOperationVector;

private:

	static void remove();

private:

	explicit KeyframeSkeletalAnimation(const KeyframeSkeletalAnimationTemplate *skeletalAnimationTemplate, const AnimationEnvironment &animationEnvironment, const TransformNameMap &skeletonTransformNameMap);
	virtual ~KeyframeSkeletalAnimation();

	bool  validateInvariantsWarn() const;

	// disabled
	KeyframeSkeletalAnimation();
	KeyframeSkeletalAnimation(const KeyframeSkeletalAnimation&);
	KeyframeSkeletalAnimation &operator =(const KeyframeSkeletalAnimation&);

private:

	static MemoryBlockManager *ms_skeletalAnimationMemoryBlockManager;
	static bool                              ms_testInvariants;

private:

	float                      m_currentFrameNumber;
	float                      m_previousFrameNumber;

	float                      m_currentCycleTime;
	float                      m_cyclePeriod;
	float                      m_playbackFramesPerSecond;
	float                      m_ooPlaybackFramesPerSecond;
	float                      m_frameCount;

	QuaternionOperationVector  m_rotationOperations;
	VectorOperationVector      m_translationOperations;

	mutable int                m_rotationStartKeyIndex;
	mutable int                m_translationStartKeyIndex;

	const float               &m_scale;
};

// ======================================================================

inline const KeyframeSkeletalAnimationTemplate *KeyframeSkeletalAnimation::getOurTemplate() const
{
	// normally this would be a static, but that would require #including KeyframeSkeletalAnimationTemplate.h,
	// which I consider unnecessary.
	return reinterpret_cast<const KeyframeSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate());
}

// ======================================================================

#endif
