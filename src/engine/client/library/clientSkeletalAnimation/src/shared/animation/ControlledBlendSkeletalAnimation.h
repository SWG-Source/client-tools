// ======================================================================
//
// ControlledBlendSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ControlledBlendSkeletalAnimation_H
#define INCLUDED_ControlledBlendSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

// ======================================================================
/**
 * Implements an externally-controlled blend between two animations.
 *
 * Messages and locomotion data are chosen from the animation with the
 * largest blend factor.
 *
 * Either animation may be NULL, in which case Quaternion::identity or
 * Vector::zero will be used as that animation's target.
 */

class ControlledBlendSkeletalAnimation: public SkeletalAnimation
{
public:

	enum BlendEquation
	{
		/// blend result = (1.0 - P) * A + P * B.  Simple, standard blend.
		BE_simpleBlend,

		/// blend result = A + P * (B - A).  Result is anchored to A.  Result is guaranteed to be between A and B.
		BE_anchoredBlend
	};

	typedef Quaternion (*QuaternionBlender)(const Quaternion &a, const Quaternion &b, float blendFactor);
	typedef Vector     (*VectorBlender)(const Vector &a, const Vector &b, float blendFactor);

public:

	explicit ControlledBlendSkeletalAnimation(bool ignoreAlter, BlendEquation blendEquation);
	virtual ~ControlledBlendSkeletalAnimation();

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

	void                             setAnimation1(SkeletalAnimation *animation);
	void                             setAnimation2(SkeletalAnimation *animation);
	void                             setBlendFactor(float fractionOfAnimation2);

private:

private:

	// disabled
	ControlledBlendSkeletalAnimation();
	ControlledBlendSkeletalAnimation(const ControlledBlendSkeletalAnimation&);
	ControlledBlendSkeletalAnimation &operator =(const ControlledBlendSkeletalAnimation&);

private:

	static const float cms_minBlendTime;

private:

	SkeletalAnimation *m_animation1;
	SkeletalAnimation *m_animation2;

	/// Animates from 0 (full animation1) to 1 (full animation2) over the course of the blend.
	float              m_blendFraction;
	bool               m_ignoreAlter;

	QuaternionBlender  m_rotationBlender;
	VectorBlender      m_translationBlender;

};

// ======================================================================

inline void ControlledBlendSkeletalAnimation::setBlendFactor(float fractionOfAnimation2)
{
	m_blendFraction = fractionOfAnimation2;
}

// ======================================================================

#endif
