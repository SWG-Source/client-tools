// ======================================================================
//
// YawSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_YawSkeletalAnimation_H
#define INCLUDED_YawSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

class AnimationEnvironment;
class ControlledBlendSkeletalAnimation;
class TimedBlendSkeletalAnimation;
class YawSkeletalAnimationTemplate;

// ======================================================================

class YawSkeletalAnimation: public SkeletalAnimation
{
public:

	YawSkeletalAnimation(const YawSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap);

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

	virtual ~YawSkeletalAnimation();

	void                                setupAnimation();

	const YawSkeletalAnimationTemplate &getOurTemplate() const;
	SkeletalAnimation                  *getYawAnimation();
	const SkeletalAnimation            *getYawAnimation() const;

	// disabled
	YawSkeletalAnimation();
	YawSkeletalAnimation(const YawSkeletalAnimation&);
	YawSkeletalAnimation &operator =(const YawSkeletalAnimation&);

private:

	SkeletalAnimation                *m_yawLeftAnimation;
	SkeletalAnimation                *m_yawRightAnimation;
	SkeletalAnimation                *m_noYawAnimation;

	TimedBlendSkeletalAnimation      *m_currentAnimation;
	SkeletalAnimation                *m_previousAnimation;

	const int                        &m_yawDirection;
};

// ======================================================================

#endif
