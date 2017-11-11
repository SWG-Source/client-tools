// ======================================================================
//
// StringSelectorSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StringSelectorSkeletalAnimation_H
#define INCLUDED_StringSelectorSkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

class AnimationEnvironment;
class CrcLowerString;
class SkeletalAppearance2;
class StringSelectorSkeletalAnimationTemplate;
class TransformNameMap;

// ======================================================================

class StringSelectorSkeletalAnimation: public SkeletalAnimation
{
public:

	StringSelectorSkeletalAnimation(const StringSelectorSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap);

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

	virtual ~StringSelectorSkeletalAnimation();

	const StringSelectorSkeletalAnimationTemplate &getOurTemplate() const;
	int                                            setupCurrentAnimation();

	// Disabled.
	StringSelectorSkeletalAnimation();
	StringSelectorSkeletalAnimation(const StringSelectorSkeletalAnimation&);
	StringSelectorSkeletalAnimation &operator =(const StringSelectorSkeletalAnimation&);

private:

	const CrcLowerString          &m_value;
	CrcLowerString          *const m_previousValue;

	AnimationEnvironment          &m_animationEnvironment;
	const TransformNameMap        &m_transformNameMap;

	SkeletalAnimationVector *const m_animations;
	SkeletalAnimation       *      m_currentAnimation;
	int                            m_previousSelectionIndex;

#ifdef _DEBUG
	SkeletalAppearance2     *	     m_appearance;
#endif

};

// ======================================================================

#endif
