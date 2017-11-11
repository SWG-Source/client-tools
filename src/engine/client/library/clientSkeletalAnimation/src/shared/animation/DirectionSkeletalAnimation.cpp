// ======================================================================
//
// DirectionSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/ControlledBlendSkeletalAnimation.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h" // @todo remove this, here solely for the debug flag.  figure out better flag strategy.
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <algorithm>

// ======================================================================

/// The maximum rotational velocity at which the appearance can modify the looking.
const float DirectionSkeletalAnimation::cms_maxRotationVelocity = PI_TIMES_2 / 0.50f;
const float PI_UNDER_180                                        = 180.0f / PI;
const float cs_negativeWrapAroundThreshold                      = (-180.0f + 15.0f) * PI_OVER_180;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(DirectionSkeletalAnimation, true, 0, 0, 0);

// ======================================================================
// inlines
// ======================================================================

inline const DirectionSkeletalAnimationTemplate &DirectionSkeletalAnimation::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const DirectionSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate()));
}

// ======================================================================

DirectionSkeletalAnimation::DirectionSkeletalAnimation(const DirectionSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) :
	SkeletalAnimation(&animationTemplate),
	m_animationEnvironment(animationEnvironment),
	m_transformNameMap(transformNameMap),
	m_animations(16, static_cast<SkeletalAnimation*>(0)),
	m_currentAnimation(new ControlledBlendSkeletalAnimation(true, ControlledBlendSkeletalAnimation::BE_anchoredBlend)),
	m_targetDirection(animationEnvironment.getVector(AnimationEnvironmentNames::cms_directionToTarget)),
	m_firstNonNullIndex(-1),
	m_currentTheta(0.0f)
{
	//-- Keep local reference.
	m_currentAnimation->fetch();

	//-- Create all available direction animations.
	for (int i = 0; i < 16; ++i)
	{
		//-- Retrieve the directional animation template for the given direction.
		const SkeletalAnimationTemplate *const directionTemplate = animationTemplate.fetchDirectionalAnimationTemplate(i);
		if (directionTemplate)
		{
			// Fetch the animation.
			SkeletalAnimation *const animation = directionTemplate->fetchSkeletalAnimation(m_animationEnvironment, m_transformNameMap);
			m_animations[static_cast<SkeletalAnimationVector::size_type>(i)] = animation;

			// Release local reference to template.
			directionTemplate->release();
		}
	}

	findFirstNonNullAnimationIndex();
	setupCurrentAnimation(0.0f);
}

// ----------------------------------------------------------------------

bool DirectionSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	UNREF(replacementAnimation);

	//-- Make sure we have at least one direction.
	if (m_firstNonNullIndex == -1)
	{
		deltaTimeRemaining = 0.0f;
		return false;
	}

	//-- Animate the first non-zero direction.  If it needs to loop, loop all animations.  This keeps each
	//   animation in sync at least at the cycle granularity.
	SkeletalAnimation *const firstAnimation = m_animations[static_cast<SkeletalAnimationVector::size_type>(m_firstNonNullIndex)];
	NOT_NULL(firstAnimation);

	SkeletalAnimation *localReplacementAnimation = 0;

	bool keepAnimation = firstAnimation->alterSingleCycle(deltaTime, localReplacementAnimation, deltaTimeRemaining);
	if (!keepAnimation)
	{
		if (localReplacementAnimation)
			localReplacementAnimation->fetch();

		firstAnimation->release();
		m_animations[static_cast<SkeletalAnimationVector::size_type>(m_firstNonNullIndex)] = localReplacementAnimation;

		findFirstNonNullAnimationIndex();
	}

	//-- Animate all remaining animations.
	float alternateDeltaTimeRemaining;

	for (int i = m_firstNonNullIndex + 1; i < 16; ++i)
	{
		SkeletalAnimation *const animation = m_animations[static_cast<SkeletalAnimationVector::size_type>(i)];
		if (!animation)
			continue;

		keepAnimation = animation->alterSingleCycle(deltaTime, localReplacementAnimation, alternateDeltaTimeRemaining);
		if (!keepAnimation)
		{
			if (localReplacementAnimation)
				localReplacementAnimation->fetch();

			animation->release();
			m_animations[static_cast<SkeletalAnimationVector::size_type>(i)] = localReplacementAnimation;
		}

		//-- Must return the max delta time remaining --- if any are ready to expire, we _MUST_ allow them all to cycle.
		deltaTimeRemaining = std::max(deltaTimeRemaining, alternateDeltaTimeRemaining);
	}

	//-- Choose current animation based on direction.
	setupCurrentAnimation(deltaTime);

	//-- Return to caller.  This animation type usually doesn't try to die unless it has no direction animations.
	replacementAnimation = 0;

	return true;
} //lint !e1764 // replacementAnimation could be made const // no, that would break the virtual contract.

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::startNewCycle()
{
	for (int i = 0; i < 16; ++i)
	{
		SkeletalAnimation *const animation = m_animations[static_cast<SkeletalAnimationVector::size_type>(i)];
		if (animation)
			animation->startNewCycle();
	}
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimation::getTransformCount() const
{
	return m_transformNameMap.getTransformCount();
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_currentAnimation->evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimation::getTransformPriority(int index) const
{
	return m_currentAnimation->getTransformPriority(index);
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimation::getLocomotionPriority() const
{	
	return m_currentAnimation->getLocomotionPriority();
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_currentAnimation->getScaledLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

float DirectionSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	return m_currentAnimation->getCycleScaledLocomotionDistance();
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimation::getFrameCount() const
{
	return m_currentAnimation->getFrameCount();
}

// ----------------------------------------------------------------------

float DirectionSkeletalAnimation::getRecordedFramesPerSecond() const
{
	return m_currentAnimation->getRecordedFramesPerSecond();
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	// @todo if this should be here at all, it probably should change all of the animations.
	m_currentAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float DirectionSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_currentAnimation->getPlaybackFramesPerSecond();
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimation::getMessageCount() const
{
	return m_currentAnimation->getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &DirectionSkeletalAnimation::getMessageName(int index) const
{
	return m_currentAnimation->getMessageName(index);
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	m_currentAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *DirectionSkeletalAnimation::resolveSkeletalAnimation()
{
	//-- Choose current animation based on desired direction.
	setupCurrentAnimation(0.0f);

	//-- Return chosen direction animation.
	return m_currentAnimation; //lint !e1536 // (Warning -- Exposing low access member 'DirectionSkeletalAnimation::m_currentAnimation' -- Effective C++ #30) // Yes, this is intentional.
}

// ======================================================================

DirectionSkeletalAnimation::~DirectionSkeletalAnimation()
{
	m_currentAnimation->release();
	m_currentAnimation = 0; //lint !e423 // creation of memory leak // no, animations are reference counted.

	//-- Clear out all animations
	const SkeletalAnimationVector::iterator endIt = m_animations.end();
	for (SkeletalAnimationVector::iterator it = m_animations.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->release();
	}
}

// ----------------------------------------------------------------------
/**
 * Setup the current animation based on the desired velocity direction,
 * returning the direction index.
 *
 * @return  the direction index for the chosen direction.
 */

void DirectionSkeletalAnimation::setupCurrentAnimation(float deltaTime)
{
	const DirectionSkeletalAnimationTemplate &directionAnimationTemplate = getOurTemplate();
	
	//-- Setup current direction based on target direction and directional acceleration/velocity.
	calculateCurrentDirection(deltaTime);

	//-- Retrieve two closest animation indices for the given theta (rotation around y axis, zero = +z axis).  Also retrieve blend factor between the two.
	int   index1      = -1;
	int   index2      = -1;
	float blendFactor = 0.0f;

	directionAnimationTemplate.getClosestAvailableEncodedDirections(m_currentTheta, index1, index2, blendFactor);

	SkeletalAnimation *const animation1 = (index1 >= 0) ? m_animations[static_cast<SkeletalAnimationVector::size_type>(index1)] : 0;
	SkeletalAnimation *const animation2 = (index2 >= 0) ? m_animations[static_cast<SkeletalAnimationVector::size_type>(index2)] : 0;

	m_currentAnimation->setAnimation1(animation1);
	m_currentAnimation->setAnimation2(animation2);
	m_currentAnimation->setBlendFactor(blendFactor);

	//-- Print out selected direction if direction changed.
#if 0
	{
		static int printCount = 0;
		static int lastDirectionIndex = -1;
		static float totalTime = 0.f;

		if (directionIndex != lastDirectionIndex)
		{
			DEBUG_REPORT_LOG(true, ("DC: %1.2f #%d changed to [%d].\n", totalTime += deltaTime, ++printCount, directionIndex));
			lastDirectionIndex = directionIndex;
		}
	}
#endif
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::calculateCurrentDirection(float deltaTime)
{
	float targetTheta = m_targetDirection.theta();

	//-- Cause target theta to wrap around to positive angles when we hit the range -150 to -180 degrees.
	if (targetTheta < cs_negativeWrapAroundThreshold)
		targetTheta += (2.0f * PI);

	DEBUG_REPORT_PRINT(SkeletalAppearance2::getRenderTargetDirection(), ("direction: target (%.1f), old (%.1f), ", targetTheta * PI_UNDER_180, m_currentTheta * PI_UNDER_180));

	if (m_currentTheta > targetTheta)
	{
		//-- Decrease current theta toward targetTheta appropriately.
		m_currentTheta = std::max(m_currentTheta - cms_maxRotationVelocity * deltaTime, targetTheta);
	}
	else
	{
		//-- Increase current theta toward targetTheta appropriately.
		m_currentTheta = std::min(m_currentTheta + cms_maxRotationVelocity * deltaTime, targetTheta);
	}

	DEBUG_REPORT_PRINT(SkeletalAppearance2::getRenderTargetDirection(), ("new (%.1f).\n", m_currentTheta * PI_UNDER_180));
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimation::findFirstNonNullAnimationIndex()
{
	m_firstNonNullIndex = -1;

	for (int i = 0; i < 16; ++i)
	{
		if (m_animations[static_cast<SkeletalAnimationVector::size_type>(i)])
		{
			// Found it.
			m_firstNonNullIndex = i;
			return;
		}
	}
}

// ======================================================================
