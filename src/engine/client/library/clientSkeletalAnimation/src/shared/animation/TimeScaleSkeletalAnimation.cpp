// ======================================================================
//
// TimeScaleSkeletalAnimation.cpp
// Copyright 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimation.h"

#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================
// class TimeScaleSkeletalAnimation: PUBLIC
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(TimeScaleSkeletalAnimation, true, 0, 0, 0);

// ======================================================================

bool TimeScaleSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	TimeScaleSkeletalAnimationTemplate const &animationTemplate = getTimeScaleSkeletalAnimationTemplate();

	float const scaledDeltaTime = deltaTime * animationTemplate.getTimeScale();
	float scaledDeltaTimeRemaining;

	bool const returnValue = m_baseAnimation->alterSingleCycle(scaledDeltaTime, replacementAnimation, scaledDeltaTimeRemaining);
	deltaTimeRemaining = scaledDeltaTimeRemaining * animationTemplate.getOneOverTimeScale();

	return returnValue;
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimation::startNewCycle()
{
	m_baseAnimation->startNewCycle();
}

// ----------------------------------------------------------------------

int TimeScaleSkeletalAnimation::getTransformCount() const
{
	return m_baseAnimation->getTransformCount();
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_baseAnimation->evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

int TimeScaleSkeletalAnimation::getTransformPriority(int index) const
{
	return m_baseAnimation->getTransformPriority(index);
}

// ----------------------------------------------------------------------

int TimeScaleSkeletalAnimation::getLocomotionPriority() const
{
	return m_baseAnimation->getLocomotionPriority();
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_baseAnimation->getScaledLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

float TimeScaleSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	return m_baseAnimation->getCycleScaledLocomotionDistance();
}

// ----------------------------------------------------------------------

int TimeScaleSkeletalAnimation::getFrameCount() const
{
	return m_baseAnimation->getFrameCount();
}

// ----------------------------------------------------------------------

float TimeScaleSkeletalAnimation::getRecordedFramesPerSecond() const
{
	return m_baseAnimation->getRecordedFramesPerSecond();
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	m_baseAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float TimeScaleSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_baseAnimation->getPlaybackFramesPerSecond();
}

// ----------------------------------------------------------------------

int TimeScaleSkeletalAnimation::getMessageCount() const
{
	return m_baseAnimation->getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &TimeScaleSkeletalAnimation::getMessageName(int index) const
{
	return m_baseAnimation->getMessageName(index);
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	m_baseAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *TimeScaleSkeletalAnimation::resolveSkeletalAnimation()
{
	return m_baseAnimation->resolveSkeletalAnimation();
}

// ======================================================================

TimeScaleSkeletalAnimation::~TimeScaleSkeletalAnimation()
{
	if (m_baseAnimation)
	{
		m_baseAnimation->release();
		m_baseAnimation = 0;
	}
}

// ======================================================================

TimeScaleSkeletalAnimation::TimeScaleSkeletalAnimation(TimeScaleSkeletalAnimationTemplate const *skeletalAnimationTemplate, SkeletalAnimation *baseSkeletalAnimation) :
	SkeletalAnimation(skeletalAnimationTemplate),
	m_baseAnimation(baseSkeletalAnimation)
{
	//-- Fetch local reference to base animation.
	NOT_NULL(m_baseAnimation);
	m_baseAnimation->fetch();
}

// ----------------------------------------------------------------------

TimeScaleSkeletalAnimationTemplate const &TimeScaleSkeletalAnimation::getTimeScaleSkeletalAnimationTemplate() const
{
	TimeScaleSkeletalAnimationTemplate const *const animationTemplate = safe_cast<TimeScaleSkeletalAnimationTemplate const*>(getSkeletalAnimationTemplate());
	NOT_NULL(animationTemplate);

	return *animationTemplate;
}

// ======================================================================
