// ======================================================================
//
// ActionGeneratorSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionGeneratorSkeletalAnimation_H
#define INCLUDED_ActionGeneratorSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

class AnimationEnvironment;
class CrcLowerString;
class ActionGeneratorSkeletalAnimationTemplate;
class TransformNameMap;

// ======================================================================

class ActionGeneratorSkeletalAnimation: public SkeletalAnimation
{
public:

	ActionGeneratorSkeletalAnimation(const ActionGeneratorSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap);

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

	virtual ~ActionGeneratorSkeletalAnimation();

	const ActionGeneratorSkeletalAnimationTemplate &getOurTemplate() const;

	// Disabled.
	ActionGeneratorSkeletalAnimation();
	ActionGeneratorSkeletalAnimation(const ActionGeneratorSkeletalAnimation&);
	ActionGeneratorSkeletalAnimation &operator =(const ActionGeneratorSkeletalAnimation&);

private:

	AnimationEnvironment &m_animationEnvironment;

	SkeletalAnimation    *m_currentAnimation;
	float                 m_countdownToNextAction;

};

// ======================================================================

#endif
