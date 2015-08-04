// ======================================================================
//
// TimedBlendSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TimedBlendSkeletalAnimation_H
#define INCLUDED_TimedBlendSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

// ======================================================================
/**
 * Implements a linear blend from one animation to another over a given
 * amount of time.
 *
 * Either animation may be NULL, in which case Quaternion::identity or
 * Vector::zero will be used as that animation's target.
 */

class TimedBlendSkeletalAnimation: public SkeletalAnimation
{
public:

	/// TerminationPolicy controls when the animation returns false from alterSingleCycle().
	enum TerminationPolicy
	{
		/// Terminate when the blend time is complete.
		TP_stopWhenBlendCompletes,

		/// Continue running the destination animation when the blend time elapses.
		TP_continueRunning

	};

public:

	TimedBlendSkeletalAnimation(SkeletalAnimation *animation1, SkeletalAnimation *animation2, float blendTime, TerminationPolicy terminationPolicy = TP_stopWhenBlendCompletes);
	TimedBlendSkeletalAnimation(SkeletalAnimation *onlyAnimation);
	virtual ~TimedBlendSkeletalAnimation();

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

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// New functionality.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void                             setNewBlendTarget(SkeletalAnimation *animation, float blendTime);
	void                             alterBlendTime(float deltaTime);

private:

	static void  doAnimationAlter(SkeletalAnimation *&animation, float deltaTime);

private:

	// disabled
	TimedBlendSkeletalAnimation();
	TimedBlendSkeletalAnimation(const TimedBlendSkeletalAnimation&);
	TimedBlendSkeletalAnimation &operator =(const TimedBlendSkeletalAnimation&);

private:

	static const float cms_minBlendTime;

private:

	SkeletalAnimation *m_animation1;
	SkeletalAnimation *m_animation2;

	/// Animates from 0 (full animation1) to 1 (full animation2) over the course of the blend.
	float              m_blendFraction;
	float              m_blendRate;

	float              m_currentTime;
	float              m_endTime;

	TerminationPolicy  m_terminationPolicy;

};

// ======================================================================

#endif
