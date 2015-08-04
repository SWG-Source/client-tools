// ======================================================================
//
// ControlledBlendSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ControlledBlendSkeletalAnimation.h"

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

const float ControlledBlendSkeletalAnimation::cms_minBlendTime = 0.001f;

// ======================================================================

namespace ControlledBlendSkeletalAnimationNamespace
{
	Quaternion  simpleQuaternionBlender(const Quaternion &a, const Quaternion &b, float blendFactor);
	Quaternion  anchoredQuaternionBlender(const Quaternion &a, const Quaternion &b, float blendFactor);

	Vector      simpleVectorBlender(const Vector &a, const Vector &b, float blendFactor);
	Vector      anchoredVectorBlender(const Vector &a, const Vector &b, float blendFactor);

	ControlledBlendSkeletalAnimation::QuaternionBlender  selectRotationBlender(ControlledBlendSkeletalAnimation::BlendEquation blendEquation);
	ControlledBlendSkeletalAnimation::VectorBlender      selectTranslationBlender(ControlledBlendSkeletalAnimation::BlendEquation blendEquation);
}

// ======================================================================

Quaternion ControlledBlendSkeletalAnimationNamespace::simpleQuaternionBlender(const Quaternion &a, const Quaternion &b, float blendFactor)
{
	return a.slerp(b, blendFactor);
}

// ----------------------------------------------------------------------

Quaternion ControlledBlendSkeletalAnimationNamespace::anchoredQuaternionBlender(const Quaternion &a, const Quaternion &b, float blendFactor)
{
	// creating a rotation that is conceptionally:
	//   a + blendFactor * (b - a)
	return Quaternion::identity.slerp(b * a.getComplexConjugate(), blendFactor) * a;
}

// ----------------------------------------------------------------------

Vector ControlledBlendSkeletalAnimationNamespace::simpleVectorBlender(const Vector &a, const Vector &b, float blendFactor)
{
	return a * (1.0f - blendFactor) + (b * blendFactor);
}

// ----------------------------------------------------------------------

Vector ControlledBlendSkeletalAnimationNamespace::anchoredVectorBlender(const Vector &a, const Vector &b, float blendFactor)
{
	return a + blendFactor * (b - a);
}

// ----------------------------------------------------------------------

ControlledBlendSkeletalAnimation::QuaternionBlender ControlledBlendSkeletalAnimationNamespace::selectRotationBlender(ControlledBlendSkeletalAnimation::BlendEquation blendEquation)
{
	switch (blendEquation)
	{
	case ControlledBlendSkeletalAnimation::BE_simpleBlend:
		return simpleQuaternionBlender;

	case ControlledBlendSkeletalAnimation::BE_anchoredBlend:
	default:
		return anchoredQuaternionBlender;
	}
}

// ----------------------------------------------------------------------

ControlledBlendSkeletalAnimation::VectorBlender ControlledBlendSkeletalAnimationNamespace::selectTranslationBlender(ControlledBlendSkeletalAnimation::BlendEquation blendEquation)
{
	switch (blendEquation)
	{
	case ControlledBlendSkeletalAnimation::BE_simpleBlend:
		return simpleVectorBlender;

	case ControlledBlendSkeletalAnimation::BE_anchoredBlend:
	default:
		return anchoredVectorBlender;
	}
}

// ======================================================================

ControlledBlendSkeletalAnimation::ControlledBlendSkeletalAnimation(bool ignoreAlter, BlendEquation blendEquation) :
	SkeletalAnimation(0),
	m_animation1(0),
	m_animation2(0),
	m_blendFraction(0.5f),
	m_ignoreAlter(ignoreAlter),
	m_rotationBlender(ControlledBlendSkeletalAnimationNamespace::selectRotationBlender(blendEquation)),
	m_translationBlender(ControlledBlendSkeletalAnimationNamespace::selectTranslationBlender(blendEquation))
{
}

// ----------------------------------------------------------------------

ControlledBlendSkeletalAnimation::~ControlledBlendSkeletalAnimation()
{
	if (m_animation1)
	{
		m_animation1->release();
		m_animation1 = 0;
	}

	if (m_animation2)
	{
		m_animation2->release();
		m_animation2 = 0;
	}
}

// ----------------------------------------------------------------------

bool ControlledBlendSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
#if 1

	// Ignore the call.
	UNREF(deltaTime);
	replacementAnimation = 0;
	deltaTimeRemaining   = 0.0f;

	return true;

#else
	// NOTE: this is the TimedBlendSkeletalAnimation code.
	bool continueBlend = true;

	//-- Adjust blend factor.
	m_currentTime += deltaTime;
	if (m_currentTime > m_endTime)
	{
		// Blend is done after this loop.
		deltaTimeRemaining   = m_currentTime - m_endTime;
		m_currentTime        = m_endTime;
		continueBlend        = false;
	}
	else
	{
		// Blend has more to go after this loop.
		deltaTimeRemaining = 0.0f;
	}

	m_blendFraction = m_blendRate * m_currentTime;

	//-- Handle animation 1 alter.
	const float deltaTimeToApply = deltaTime - deltaTimeRemaining;

	if (m_animation1)
		doAnimationAlter(m_animation1, deltaTimeToApply);

	//-- Handle animation 2 alter.
	if (m_animation2)
		doAnimationAlter(m_animation2, deltaTimeToApply);

	//-- Set replacement animation if blend ends.
	//   NOTE: we wait until after altering the target animations
	//         so we don't return a dead animation.
	if (continueBlend)
		replacementAnimation = 0;
	else
		replacementAnimation = m_animation2;

	//-- Indicate if blend animation should continue.
	return continueBlend;
#endif
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::startNewCycle()
{
	// Not sure if this is the proper behavior here, assumes both animations are in cycle-synch.
	if (m_animation1)
		m_animation1->startNewCycle();

	if (m_animation2)
		m_animation2->startNewCycle();
}

// ----------------------------------------------------------------------

int ControlledBlendSkeletalAnimation::getTransformCount() const
{
	if (m_animation1)
		return m_animation1->getTransformCount();
	else if (m_animation2)
		return m_animation2->getTransformCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	//-- Evaluate animation 1.
	Quaternion rotation1(Quaternion::identity);
	Vector     translation1(Vector::zero);

	if (m_animation1)
		m_animation1->evaluateTransformComponents(index, rotation1, translation1);

	//-- Evaluate animation 2.
	Quaternion rotation2(Quaternion::identity);
	Vector     translation2(Vector::zero);

	if (m_animation2)
		m_animation2->evaluateTransformComponents(index, rotation2, translation2);

	//-- Blend the result.
	translation = (*m_translationBlender)(translation1, translation2, m_blendFraction);
	rotation    = (*m_rotationBlender)(rotation1, rotation2, m_blendFraction);
}

// ----------------------------------------------------------------------

int ControlledBlendSkeletalAnimation::getTransformPriority(int index) const
{
	const int animation1Priority = (m_animation1 ? m_animation1->getTransformPriority(index) : std::numeric_limits<int>::min());
	const int animation2Priority = (m_animation2 ? m_animation2->getTransformPriority(index) : std::numeric_limits<int>::min());

	return std::max(animation1Priority, animation2Priority);
}

// ----------------------------------------------------------------------

int ControlledBlendSkeletalAnimation::getLocomotionPriority() const
{
	const int animation1Priority = (m_animation1 ? m_animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
	const int animation2Priority = (m_animation2 ? m_animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

	return std::max(animation1Priority, animation2Priority);
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	//-- Get animation 1 locomotion.
	Quaternion rotation1(Quaternion::identity);
	Vector     translation1(Vector::zero);

	if (m_animation1)
		m_animation1->getScaledLocomotion(rotation1, translation1);

	//-- Get animation 2 locomotion.
	Quaternion rotation2(Quaternion::identity);
	Vector     translation2(Vector::zero);

	if (m_animation2)
		m_animation2->getScaledLocomotion(rotation2, translation2);

	//-- Blend the result.
	translation = (*m_translationBlender)(translation1, translation2, m_blendFraction);
	rotation    = (*m_rotationBlender)(rotation1, rotation2, m_blendFraction);
}

// ----------------------------------------------------------------------

float ControlledBlendSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	//-- Get locomotion distance.
	const float distance1 = (m_animation1 ? m_animation1->getCycleScaledLocomotionDistance() : 0.0f);
	const float distance2 = (m_animation2 ? m_animation2->getCycleScaledLocomotionDistance() : 0.0f);

	//-- Blend the result.  Use the vector blender.
	// @todo clean up to use a new FloatBlender.
	return (*m_translationBlender)(Vector(distance1, 0.0f, 0.0f), Vector(distance2, 0.0f, 0.0f), m_blendFraction).x;
}

// ----------------------------------------------------------------------

int ControlledBlendSkeletalAnimation::getFrameCount() const
{
	const int count1 = (m_animation1 ? m_animation1->getFrameCount() : 0);
	const int count2 = (m_animation2 ? m_animation2->getFrameCount() : 0);

	return std::max(count1, count2);
}

// ----------------------------------------------------------------------

float ControlledBlendSkeletalAnimation::getRecordedFramesPerSecond() const
{
	const float count1 = (m_animation1 ? m_animation1->getRecordedFramesPerSecond() : 0.0f);
	const float count2 = (m_animation2 ? m_animation2->getRecordedFramesPerSecond() : 0.0f);

	return std::max(count1, count2);
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	DEBUG_WARNING(true, ("unexpected: ControlledBlendSkeletalAnimation::setPlaybackFramesPerSecond() called."));

	if (m_animation1)
		m_animation1->setPlaybackFramesPerSecond(playbackFramesPerSecond);

	if (m_animation2)
		m_animation2->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float ControlledBlendSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	const float count1 = (m_animation1 ? m_animation1->getPlaybackFramesPerSecond() : 0.0f);
	const float count2 = (m_animation2 ? m_animation2->getPlaybackFramesPerSecond() : 0.0f);

	return std::max(count1, count2);
}

// ----------------------------------------------------------------------

int ControlledBlendSkeletalAnimation::getMessageCount() const
{
	if (m_blendFraction < 0.5f)
		return (m_animation1 ? m_animation1->getMessageCount() : 0);
	else
		return (m_animation2 ? m_animation2->getMessageCount() : 0);
}

// ----------------------------------------------------------------------

const CrcLowerString &ControlledBlendSkeletalAnimation::getMessageName(int index) const
{
	if (m_blendFraction < 0.5f)
	{
		if (m_animation1)
			return m_animation1->getMessageName(index);
		else
			return CrcLowerString::empty;
	}
	else
	{
		if (m_animation2)
			return m_animation2->getMessageName(index);
		else
			return CrcLowerString::empty;
	}
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	if (m_blendFraction < 0.5f)
	{
		//-- Collect entries for animation 1.
		if (m_animation1)
			m_animation1->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
	}
	else
	{
		if (m_animation2)
			m_animation2->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
	}
}

// ----------------------------------------------------------------------

SkeletalAnimation *ControlledBlendSkeletalAnimation::resolveSkeletalAnimation()
{
	//-- Return the higher-weighted animation.
	if (m_blendFraction < 0.5f)
		return m_animation1;
	else
		return m_animation2;
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::setAnimation1(SkeletalAnimation *animation)
{
	if (m_animation1 != animation) 
	{
		if (animation)
			animation->fetch();
		
		if (m_animation1)
			m_animation1->release();
		
		m_animation1 = animation;
	}
}

// ----------------------------------------------------------------------

void ControlledBlendSkeletalAnimation::setAnimation2(SkeletalAnimation *animation)
{
	if (animation != m_animation2) 
	{
		if (animation)
			animation->fetch();
		
		if (m_animation2)
			m_animation2->release();
		
		m_animation2 = animation;
	}
}

// ======================================================================
