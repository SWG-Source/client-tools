// ======================================================================
//
// YawSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/YawSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/TimedBlendSkeletalAnimation.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

const float cs_blendTime = 0.01f;

// ======================================================================
// inlines
// ======================================================================

namespace
{
	inline SkeletalAnimation *FetchAnimationFromTemplate(const SkeletalAnimationTemplate *animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap)
	{
		if (!animationTemplate)
			return 0;

		// Create the animation.
		SkeletalAnimation *const animation = animationTemplate->fetchSkeletalAnimation(animationEnvironment, transformNameMap);

		// Release the animation template.
		animationTemplate->release();

		return animation;
	}
}

// ======================================================================

inline const YawSkeletalAnimationTemplate &YawSkeletalAnimation::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const YawSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate()));
}

// ----------------------------------------------------------------------

inline SkeletalAnimation *YawSkeletalAnimation::getYawAnimation()
{
	if ((m_yawDirection < 0) && m_yawLeftAnimation)
		return m_yawLeftAnimation;
	else if ((m_yawDirection > 0) && m_yawRightAnimation)
		return m_yawRightAnimation;
	else
		return m_noYawAnimation;
}


// ----------------------------------------------------------------------

inline const SkeletalAnimation *YawSkeletalAnimation::getYawAnimation() const
{
	if ((m_yawDirection < 0) && m_yawLeftAnimation)
		return m_yawLeftAnimation;
	else if ((m_yawDirection > 0) && m_yawRightAnimation)
		return m_yawRightAnimation;
	else
		return m_noYawAnimation;
}

// ======================================================================
// class YawSkeletalAnimation: public member functions
// ======================================================================

YawSkeletalAnimation::YawSkeletalAnimation(const YawSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) :
	SkeletalAnimation   (&animationTemplate),
	m_yawLeftAnimation  (FetchAnimationFromTemplate(animationTemplate.fetchYawLeftAnimationTemplate(), animationEnvironment, transformNameMap)),
	m_yawRightAnimation (FetchAnimationFromTemplate(animationTemplate.fetchYawRightAnimationTemplate(), animationEnvironment, transformNameMap)),
	m_noYawAnimation    (FetchAnimationFromTemplate(animationTemplate.fetchNoYawAnimationTemplate(), animationEnvironment, transformNameMap)),
	m_currentAnimation  (new TimedBlendSkeletalAnimation(m_noYawAnimation, m_noYawAnimation, 0.25f, TimedBlendSkeletalAnimation::TP_continueRunning)),
	m_previousAnimation (0),
	m_yawDirection      (animationEnvironment.getInt(AnimationEnvironmentNames::cms_yawDirection))
{
	//-- Fetch local reference to current animation.
	m_currentAnimation->fetch();
}

// ----------------------------------------------------------------------

bool YawSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *& /* replacementAnimation */, float &deltaTimeRemaining)
{
	//-- Retrieve the skeletal animation to use for the current yaw setting.
	setupAnimation();

#if 0
	if (m_currentAnimation == m_yawLeftAnimation)
		DEBUG_REPORT_PRINT(true, ("yaw animation: using yaw left\n"));
	else if (m_currentAnimation == m_yawRightAnimation)
		DEBUG_REPORT_PRINT(true, ("yaw animation: using yaw right\n"));
	else if (m_currentAnimation == m_noYawAnimation)
		DEBUG_REPORT_PRINT(true, ("yaw animation: using no yaw animation\n"));
	else
		DEBUG_REPORT_PRINT(true, ("yaw animation: NULL animation\n"));
#endif

	//-- Alter the selected animation.
	SkeletalAnimation *tempReplacementAnimation;
	IGNORE_RETURN(m_currentAnimation->alterSingleCycle(deltaTime, tempReplacementAnimation, deltaTimeRemaining));

	//-- Indicate this animation never dies by its own choice.
	return true;
} //lint !e1764 // replacementAnimation could be made const // no, that would break the virtual contract.

// ----------------------------------------------------------------------

void YawSkeletalAnimation::startNewCycle()
{
	m_currentAnimation->startNewCycle();
}

// ----------------------------------------------------------------------

int YawSkeletalAnimation::getTransformCount() const
{
	return m_currentAnimation->getTransformCount();
}

// ----------------------------------------------------------------------

void YawSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_currentAnimation->evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

int YawSkeletalAnimation::getTransformPriority(int index) const
{
	return m_currentAnimation->getTransformPriority(index);
}

// ----------------------------------------------------------------------

int YawSkeletalAnimation::getLocomotionPriority() const
{
	return m_currentAnimation->getLocomotionPriority();
}

// ----------------------------------------------------------------------

void YawSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_currentAnimation->getScaledLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

float YawSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	return m_currentAnimation->getCycleScaledLocomotionDistance();
}

// ----------------------------------------------------------------------

int YawSkeletalAnimation::getFrameCount() const
{
	return m_currentAnimation->getFrameCount();
}

// ----------------------------------------------------------------------

float YawSkeletalAnimation::getRecordedFramesPerSecond() const
{
	return m_currentAnimation->getRecordedFramesPerSecond();
}

// ----------------------------------------------------------------------

void YawSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	m_currentAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float YawSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_currentAnimation->getPlaybackFramesPerSecond();
}

// ----------------------------------------------------------------------

int YawSkeletalAnimation::getMessageCount() const
{
	return m_currentAnimation->getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &YawSkeletalAnimation::getMessageName(int index) const
{
	return m_currentAnimation->getMessageName(index);
}

// ----------------------------------------------------------------------

void YawSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	m_currentAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *YawSkeletalAnimation::resolveSkeletalAnimation()
{
	return m_currentAnimation->resolveSkeletalAnimation();
}

// ======================================================================
// class YawSkeletalAnimation: private member functions
// ======================================================================

YawSkeletalAnimation::~YawSkeletalAnimation()
{
	if (m_yawLeftAnimation)
	{
		m_yawLeftAnimation->release();
		m_yawLeftAnimation = 0;
	}

	if (m_yawRightAnimation)
	{
		m_yawRightAnimation->release();
		m_yawRightAnimation = 0;
	}

	if (m_noYawAnimation)
	{
		m_noYawAnimation->release();
		m_noYawAnimation = 0;
	}

	m_currentAnimation->release();
	m_currentAnimation = 0; //lint !e423 // Creation of memory leak. // No, animations are reference counted, so they must be released.

	m_previousAnimation = 0;
}

// ----------------------------------------------------------------------

void YawSkeletalAnimation::setupAnimation()
{
	//-- Get the new animation for the given yaw state.
	SkeletalAnimation *const newAnimation = getYawAnimation();

	//-- Reset the animation cycle if this is a newly entered animation.
	if (newAnimation && (newAnimation != m_previousAnimation))
	{
		newAnimation->startNewCycle();
		m_currentAnimation->setNewBlendTarget(newAnimation, cs_blendTime);

		m_previousAnimation = newAnimation;
	}
}

// ======================================================================
