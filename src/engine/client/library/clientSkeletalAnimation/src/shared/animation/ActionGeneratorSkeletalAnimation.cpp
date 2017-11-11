// ======================================================================
//
// ActionGeneratorSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimation.h"

#include "clientGraphics/Graphics.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <limits>
#include <vector>

// ======================================================================
// inlines
// ======================================================================

inline const ActionGeneratorSkeletalAnimationTemplate &ActionGeneratorSkeletalAnimation::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const ActionGeneratorSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate()));
}

// ======================================================================

ActionGeneratorSkeletalAnimation::ActionGeneratorSkeletalAnimation(const ActionGeneratorSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) :
	SkeletalAnimation(&animationTemplate),
	m_animationEnvironment(animationEnvironment),
	m_currentAnimation(0),
	m_countdownToNextAction(animationTemplate.getElapsedTimeUntilNextAction())
{
	//-- Retrieve the looping animation template.
	const SkeletalAnimationTemplate *const loopingTemplate = animationTemplate.fetchLoopingAnimationTemplate();
	if (loopingTemplate)
	{
		// Fetch the animation.
		m_currentAnimation = loopingTemplate->fetchSkeletalAnimation(animationEnvironment, transformNameMap);

		// Release local reference to template.
		loopingTemplate->release();
	}
}

// ----------------------------------------------------------------------

bool ActionGeneratorSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	m_countdownToNextAction -= deltaTime;
	if (m_countdownToNextAction <= 0.0f)
	{
		//-- Get the appearance template.
		SkeletalAppearance2              *const appearance  = m_animationEnvironment.getSkeletalAppearance();
		SkeletalAppearanceTemplate const *const satTemplate = appearance ? safe_cast<SkeletalAppearanceTemplate const*>(appearance->getAppearanceTemplate()) : NULL;

		//-- Check if we should bother doing animation action generation for this appearance.

		bool playAction;
		
		if (satTemplate && satTemplate->getAlwaysPlayActionGeneratorAnimations())
		{
			// Play the action because the SAT requires us to.  Useful for overhead ships.
			playAction = true;
		}
		else
		{
			// We'll skip it if we haven't rendered it recently and if the SAT doesn't specifically require us to play all action generator animations.
			int const  mruGraphicsFrameNumber = m_animationEnvironment.getMostRecentVisibleGraphicsFrameNumber();

			if ((mruGraphicsFrameNumber == std::numeric_limits<int>::min()) || ((Graphics::getFrameNumber() - mruGraphicsFrameNumber) > ConfigClientSkeletalAnimation::getSkipActionGenerationFrameCount()))
			{
				// Skip because we've never rendered or we haven't rendered recently enough.
				playAction = false;
			}
			else
			{
				// Play it because we've rendered recently.
				playAction = true;
			}
		}

		const ActionGeneratorSkeletalAnimationTemplate &agTemplate = getOurTemplate();

		if (playAction)
		{
			//-- Generate a new action.
			// Get action name from template.
			const CrcLowerString &actionName = agTemplate.randomSelectActionName();
			DEBUG_REPORT_LOG(ActionGeneratorSkeletalAnimationTemplate::shouldLogInfo(), ("AG: generating action [%s].\n", actionName.getString()));

			if (appearance)
			{
				int   animationId;
				bool  animationIsAdd;

				appearance->getAnimationResolver().playAction(actionName, animationId, animationIsAdd, NULL);
			}
		}

		//-- Reset the countdown timer.
		m_countdownToNextAction = agTemplate.getElapsedTimeUntilNextAction();
		DEBUG_REPORT_LOG(ActionGeneratorSkeletalAnimationTemplate::shouldLogInfo(), ("AG: countdown to next action [%g seconds].\n", m_countdownToNextAction));
	}

	//-- Handle looping animation alter.
	replacementAnimation = 0;

	if (!m_currentAnimation)
		deltaTimeRemaining = 0.0f;
	else
	{
		SkeletalAnimation *actionGeneratorReplacementAnimation = 0;
		const bool keepAnimation = m_currentAnimation->alterSingleCycle(deltaTime, actionGeneratorReplacementAnimation, deltaTimeRemaining);

		if (!keepAnimation)
		{
			if (actionGeneratorReplacementAnimation)
				actionGeneratorReplacementAnimation->fetch();

			m_currentAnimation->release();

			m_currentAnimation = actionGeneratorReplacementAnimation;
		}
	}

	//-- Tell caller this animation never chooses to die on its own.
	return true;
} //lint !e1764 // replacementAnimation could be made const // no, that would break the virtual contract.

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimation::startNewCycle()
{
	if (m_currentAnimation)
		m_currentAnimation->startNewCycle();
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimation::getTransformCount() const
{
	return (m_currentAnimation ? m_currentAnimation->getTransformCount() : 0);
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	if (m_currentAnimation)
		m_currentAnimation->evaluateTransformComponents(index, rotation, translation);
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimation::getTransformPriority(int index) const
{
	if (m_currentAnimation)
		return m_currentAnimation->getTransformPriority(index);
	else
	{
		// @todo work out non-specified priority levels.  This may be wrong.
		return 0;
	}
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimation::getLocomotionPriority() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getLocomotionPriority();
	else
	{
		// @todo work out non-specified priority levels.  This may be wrong.
		return 0;
	}
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	if (m_currentAnimation)
		m_currentAnimation->getScaledLocomotion(rotation, translation);
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}
}

// ----------------------------------------------------------------------

float ActionGeneratorSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getCycleScaledLocomotionDistance();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimation::getFrameCount() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getFrameCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

float ActionGeneratorSkeletalAnimation::getRecordedFramesPerSecond() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getRecordedFramesPerSecond();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	// @todo if this should be here at all, it probably should change all of the animations.
	if (m_currentAnimation)
		m_currentAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float ActionGeneratorSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getPlaybackFramesPerSecond();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimation::getMessageCount() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getMessageCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

const CrcLowerString &ActionGeneratorSkeletalAnimation::getMessageName(int index) const
{
	if (m_currentAnimation)
		return m_currentAnimation->getMessageName(index);
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	if (m_currentAnimation)
		m_currentAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *ActionGeneratorSkeletalAnimation::resolveSkeletalAnimation()
{
	return m_currentAnimation;
}

// ======================================================================

ActionGeneratorSkeletalAnimation::~ActionGeneratorSkeletalAnimation()
{
	if (m_currentAnimation)
	{
		m_currentAnimation->release();
		m_currentAnimation = 0;
	}
}

// ======================================================================
