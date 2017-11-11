// ======================================================================
//
// StringSelectorSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <vector>

#ifdef _DEBUG
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedObject/AppearanceTemplate.h"
#endif

// ======================================================================
// inlines
// ======================================================================

inline const StringSelectorSkeletalAnimationTemplate &StringSelectorSkeletalAnimation::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const StringSelectorSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate()));
}

// ======================================================================

StringSelectorSkeletalAnimation::StringSelectorSkeletalAnimation(const StringSelectorSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) :
	SkeletalAnimation(&animationTemplate),
	m_value(animationEnvironment.getConstString(animationTemplate.getEnvironmentVariableName())),
	m_previousValue(new CrcLowerString("<never_use_this_name>")),
	m_animationEnvironment(animationEnvironment),
	m_transformNameMap(transformNameMap),
	m_animations(new SkeletalAnimationVector(static_cast<SkeletalAnimationVector::size_type>(animationTemplate.getSelectionAnimationTemplateCount()), static_cast<SkeletalAnimation*>(0))),
	m_currentAnimation(0),
	m_previousSelectionIndex(0)
#ifdef _DEBUG
	, m_appearance(animationEnvironment.getSkeletalAppearance())
#endif
{
}

// ----------------------------------------------------------------------

bool StringSelectorSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	UNREF(replacementAnimation);

	//-- Select the current animation based on the variable's value.
	const int selectionIndex = setupCurrentAnimation();

	replacementAnimation = 0;

	if (!m_currentAnimation)
		deltaTimeRemaining = 0.0f;
	else
	{
		SkeletalAnimation *selectionReplacementAnimation = 0;
		const bool keepAnimation = m_currentAnimation->alterSingleCycle(deltaTime, selectionReplacementAnimation, deltaTimeRemaining);

		if (!keepAnimation)
		{
			if (selectionReplacementAnimation)
				selectionReplacementAnimation->fetch();

			m_currentAnimation->release();

			m_currentAnimation = selectionReplacementAnimation;
			(*m_animations)[static_cast<SkeletalAnimationVector::size_type>(selectionIndex)] = m_currentAnimation;
		}
	}

	//-- Tell caller this animation never chooses to die on its own.
	return true;
} //lint !e1764 // replacementAnimation could be made const // no, that would break the virtual contract.

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimation::startNewCycle()
{
	if (m_currentAnimation)
		m_currentAnimation->startNewCycle();
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimation::getTransformCount() const
{
	return m_transformNameMap.getTransformCount();
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
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

int StringSelectorSkeletalAnimation::getTransformPriority(int index) const
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

int StringSelectorSkeletalAnimation::getLocomotionPriority() const
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

void StringSelectorSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
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

float StringSelectorSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getCycleScaledLocomotionDistance();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimation::getFrameCount() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getFrameCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

float StringSelectorSkeletalAnimation::getRecordedFramesPerSecond() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getRecordedFramesPerSecond();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	// @todo if this should be here at all, it probably should change all of the animations.
	if (m_currentAnimation)
		m_currentAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float StringSelectorSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getPlaybackFramesPerSecond();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimation::getMessageCount() const
{
	if (m_currentAnimation)
		return m_currentAnimation->getMessageCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

const CrcLowerString &StringSelectorSkeletalAnimation::getMessageName(int index) const
{
	if (m_currentAnimation)
		return m_currentAnimation->getMessageName(index);
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	if (m_currentAnimation)
		m_currentAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *StringSelectorSkeletalAnimation::resolveSkeletalAnimation()
{
	//-- Resolve current animation.
	IGNORE_RETURN(setupCurrentAnimation());

	//-- Return current animation.
	return m_currentAnimation;
}

// ======================================================================

StringSelectorSkeletalAnimation::~StringSelectorSkeletalAnimation()
{
#ifdef _DEBUG
	m_appearance       = 0;
#endif

	m_currentAnimation = 0;

	//-- Clear out all animations
	const SkeletalAnimationVector::iterator endIt = m_animations->end();
	for (SkeletalAnimationVector::iterator it = m_animations->begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->release();
	}

	delete m_animations;
	delete m_previousValue;
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimation::setupCurrentAnimation()
{
	int selectionIndex;

	if (*m_previousValue == m_value)
	{
		// Value hasn't changed.
		selectionIndex = m_previousSelectionIndex;
	}
	else
	{
		// Value of variable has changed, re-evaluate.

		const StringSelectorSkeletalAnimationTemplate &selectorAnimationTemplate = getOurTemplate();
	
		//-- Convert string variable's value into a selector index.
		selectionIndex = selectorAnimationTemplate.getSelectionIndexForValue(m_value);

		//-- Retrieve/create animation for selection.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, selectionIndex, static_cast<int>(m_animations->size()));
		m_currentAnimation = (*m_animations)[static_cast<SkeletalAnimationVector::size_type>(selectionIndex)];

		if (!m_currentAnimation)
		{
			//-- Retrieve the directional animation template for the given direction.
			const SkeletalAnimationTemplate *const selectionTemplate = selectorAnimationTemplate.fetchSelectionAnimationTemplate(selectionIndex);
			if (selectionTemplate)
			{
				DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStringSelectorAnimation(), ("ANIM[string sel]: appearance [%s] string [%s], index [%d] mapped to valid animation.\n", NON_NULL(m_appearance)->getAppearanceTemplate()->getName(), m_value.getString(), selectionIndex));

				// Fetch the animation.
				m_currentAnimation = selectionTemplate->fetchSkeletalAnimation(m_animationEnvironment, m_transformNameMap);
				(*m_animations)[static_cast<SkeletalAnimationVector::size_type>(selectionIndex)] = m_currentAnimation;

				// Release local reference to template.
				selectionTemplate->release();
			}
			else
			{
				DEBUG_WARNING(ConfigClientSkeletalAnimation::getLogStringSelectorAnimation(), ("ANIM[string sel]: appearance [%s] string [%s], index [%d] mapped to NULL animation.\n", NON_NULL(m_appearance)->getAppearanceTemplate()->getName(), m_value.getString(), selectionIndex));
			}
		}

		//-- Save previous value.
		*m_previousValue         = m_value;
		m_previousSelectionIndex = selectionIndex;
	}

	return selectionIndex;
}

// ======================================================================
