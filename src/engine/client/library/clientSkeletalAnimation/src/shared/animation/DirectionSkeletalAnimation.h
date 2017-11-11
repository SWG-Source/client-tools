// ======================================================================
//
// DirectionSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DirectionSkeletalAnimation_H
#define INCLUDED_DirectionSkeletalAnimation_H

// ======================================================================

// NOTE: I'm loading more include dependencies here to reduce memory allocation requirements.
//       I think this is acceptable because this class is included in very few places.
//       Most code that works with animations works with the base SkeletalAnimation class.

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

#include <vector>

class AnimationEnvironment;
class ControlledBlendSkeletalAnimation;
class DirectionSkeletalAnimationTemplate;
class MemoryBlockManager;

// ======================================================================

class DirectionSkeletalAnimation: public SkeletalAnimation
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	DirectionSkeletalAnimation(const DirectionSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap);

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

	typedef stdvector<SkeletalAnimation*>::fwd  SkeletalAnimationVector;

private:

	virtual ~DirectionSkeletalAnimation();

	const DirectionSkeletalAnimationTemplate &getOurTemplate() const;

	void                                      setupCurrentAnimation(float deltaTime);
	void                                      calculateCurrentDirection(float deltaTime);
	void                                      findFirstNonNullAnimationIndex();

	// disabled
	DirectionSkeletalAnimation();
	DirectionSkeletalAnimation(const DirectionSkeletalAnimation&);
	DirectionSkeletalAnimation &operator =(const DirectionSkeletalAnimation&);

private:

	static const float                      cms_maxRotationVelocity;

private:

	AnimationEnvironment             &m_animationEnvironment;
	const TransformNameMap           &m_transformNameMap;

	SkeletalAnimationVector           m_animations;
	ControlledBlendSkeletalAnimation *m_currentAnimation;

	const Vector                     &m_targetDirection;
	int                               m_firstNonNullIndex;

	float                             m_currentTheta;

};

// ======================================================================

#endif
