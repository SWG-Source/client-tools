// ======================================================================
//
// SpeedSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SpeedSkeletalAnimation_H
#define INCLUDED_SpeedSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

class AnimationEnvironment;
class ControlledBlendSkeletalAnimation;
class SpeedSkeletalAnimationTemplate;
class TimedBlendSkeletalAnimation;

// ======================================================================

#define TRACK_SPEED  0

// ======================================================================

class SpeedSkeletalAnimation: public SkeletalAnimation
{
public:

	static void                      install();

	SpeedSkeletalAnimation(const SpeedSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap);

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

	class SpeedChoice;
	typedef stdvector<SpeedChoice*>::fwd  SpeedChoiceVector;

	enum State
	{
		S_idle,
		S_locomoting
	};

private:

	virtual ~SpeedSkeletalAnimation();

	const SpeedSkeletalAnimationTemplate &getOurTemplate() const;
	float                                 updateAnimationState(float desiredLocomotionSpeed);
	float                                 prepareLocomotionAnimation(float desiredLocomotionSpeed);

	SkeletalAnimation                    *getFocusAnimation();
	const SkeletalAnimation              *getFocusAnimation() const;

	// disabled
	SpeedSkeletalAnimation();
	SpeedSkeletalAnimation(const SpeedSkeletalAnimation&);
	SpeedSkeletalAnimation &operator =(const SpeedSkeletalAnimation&);

private:

	SpeedChoiceVector                *m_speedChoices;
	int                               m_speedChoiceCount;

	/**
	 * This animation is a blend of one or more different speed animations,
	 * yielding an animation that averages to the specified velocity.
	 */
	ControlledBlendSkeletalAnimation *m_locomotionAnimation;

	/**
	 * This is the one and only logical animation representing the idle
	 * for this animation.
	 */
	SkeletalAnimation                *m_idleAnimation;

	/**
	 * This animation is the blend that is always used for evaluation.
	 * This animation always has some combination of the m_locomotionAnimation
	 * and m_idleAnimation in it, possibly 100% of one of them.
	 */
	TimedBlendSkeletalAnimation      *m_evaluationAnimation;

	AnimationEnvironment             &m_animationEnvironment;
	const TransformNameMap           &m_transformNameMap;

	const Vector                     &m_locomotionVelocity;
	State                             m_currentState;

#if TRACK_SPEED
	typedef stdvector<float>::fwd     FloatVector;

	int                               m_averagingIndex;
	FloatVector                      *m_averagingDistance;
	FloatVector                      *m_averagingTime;
#endif

};

// ======================================================================

#endif
