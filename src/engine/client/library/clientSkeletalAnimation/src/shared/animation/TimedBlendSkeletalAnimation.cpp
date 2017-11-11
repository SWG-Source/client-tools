// ======================================================================
//
// TimedBlendSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TimedBlendSkeletalAnimation.h"

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

const float TimedBlendSkeletalAnimation::cms_minBlendTime = 0.001f;

// ======================================================================

TimedBlendSkeletalAnimation::TimedBlendSkeletalAnimation(SkeletalAnimation *animation1, SkeletalAnimation *animation2, float blendTime, TerminationPolicy terminationPolicy) :
	SkeletalAnimation(0),
	m_animation1(animation1),
	m_animation2(animation2),
	m_blendFraction(0.0f),
	m_blendRate(1.0f),
	m_currentTime(0.0f),
	m_endTime(blendTime),
	m_terminationPolicy(terminationPolicy)
{
	//-- Validate preconditions.
	if (blendTime <= cms_minBlendTime)
	{
		DEBUG_WARNING(true, ("bad blend time [%g], changed to [%g].", blendTime, cms_minBlendTime));
		blendTime = cms_minBlendTime;
	}

	//-- Setup blend change per time.
	m_blendRate = 1.0f / blendTime;

	//-- Grab local references.
	if (m_animation1)
		m_animation1->fetch();

	if (m_animation2)
		m_animation2->fetch();
}

// ----------------------------------------------------------------------
/**
 * Constructor to use when the caller only wants a single animation to
 * start playing but plans on blending in others in the near future.
 *
 * The termination policy is assumed to be TP_continueRunning.
 */

TimedBlendSkeletalAnimation::TimedBlendSkeletalAnimation(SkeletalAnimation *onlyAnimation) :
	SkeletalAnimation(0),
	m_animation1(0),
	m_animation2(onlyAnimation),
	m_blendFraction(1.0f),
	m_blendRate(1.0f),
	m_currentTime(0.0f),
	m_endTime(0.0f),
	m_terminationPolicy(TP_continueRunning)
{
	if (m_animation2)
		m_animation2->fetch();
}

// ----------------------------------------------------------------------

TimedBlendSkeletalAnimation::~TimedBlendSkeletalAnimation()
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

bool TimedBlendSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	bool continueBlend = true;

	//-- Adjust blend factor.
	m_currentTime += deltaTime;
	if (m_currentTime > m_endTime)
	{
		// Blend is fully using animation 2.
		m_currentTime   = m_endTime;
		m_blendFraction = 1.0f;
	
		if (m_animation1)
		{
			m_animation1->release();
			m_animation1 = 0;
		}

		if (m_terminationPolicy == TP_stopWhenBlendCompletes)
		{
			// Make sure next animation doesn't lose the overflow.
			deltaTimeRemaining = m_currentTime - m_endTime;
			continueBlend      = false;
		}
		else
		{
			// When we continue using this animation, we do not return any time remaining.
			deltaTimeRemaining = 0.0f;
		}
	}
	else
	{
		// Blend has more to continue after this loop.
		deltaTimeRemaining = 0.0f;
		m_blendFraction    = m_blendRate * m_currentTime;
	}

	//-- Handle animation 1 alter.  Ignore animation 1 when we're 100% using animation 2.
	const float deltaTimeToApply = deltaTime - deltaTimeRemaining;

	if (m_animation1 && (m_blendFraction < 1.0f))
		doAnimationAlter(m_animation1, deltaTimeToApply);

	//-- Handle animation 2 alter.
	if (m_animation2)
	{
		if (m_blendFraction < 1.0f)
			doAnimationAlter(m_animation2, deltaTimeToApply);
		else
		{
			// Ensure we pass on remaining delta time to parent.
			SkeletalAnimation *tempAnimation = 0;

			IGNORE_RETURN(m_animation2->alterSingleCycle(deltaTimeToApply, tempAnimation, deltaTimeRemaining));
		}
	}


	//-- Set replacement animation if blend ends.
	//   NOTE: we wait until after altering the target animations
	//         so we don't return a dead animation accidentally.
	if (continueBlend)
		replacementAnimation = 0;
	else
		replacementAnimation = m_animation2;

	//-- Indicate if blend animation should continue.
	return continueBlend;
}

// ----------------------------------------------------------------------

void TimedBlendSkeletalAnimation::startNewCycle()
{
	// not sure if this is the proper behavior here.
	if (m_animation1 && (m_blendFraction < 1.0f))
		m_animation1->startNewCycle();

	if (m_animation2)
		m_animation2->startNewCycle();
}

// ----------------------------------------------------------------------

int TimedBlendSkeletalAnimation::getTransformCount() const
{
	if (m_animation1 && (m_blendFraction < 1.0f))
		return m_animation1->getTransformCount();
	else if (m_animation2)
		return m_animation2->getTransformCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

void TimedBlendSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	if (m_blendFraction >= 1.0f)
	{
		//-- Only need to grab from animation 2.
		if (m_animation2)
			m_animation2->evaluateTransformComponents(index, rotation, translation);
		else
		{
			rotation    = Quaternion::identity;
			translation = Vector::zero;
		}
	}
	else
	{
		// if you don't have two different animations, don't blend.
		if (m_animation1 && m_animation2 && (m_animation1 != m_animation2)) 
		{
			//-- Evaluate animation 1.
			Quaternion rotation1(Quaternion::identity);
			Vector     translation1(Vector::zero);
			
			m_animation1->evaluateTransformComponents(index, rotation1, translation1);
			
			//-- Evaluate animation 2.
			Quaternion rotation2(Quaternion::identity);
			Vector     translation2(Vector::zero);
			
			m_animation2->evaluateTransformComponents(index, rotation2, translation2);
			
			//-- Blend the result.
			translation = (translation1 * (1.0f - m_blendFraction)) + (translation2 * m_blendFraction);
			rotation    = rotation1.slerp(rotation2, m_blendFraction);
		}
		else if(m_animation1)
		{
			m_animation1->evaluateTransformComponents(index, rotation, translation);
		}
		else if(m_animation2)
		{
			m_animation2->evaluateTransformComponents(index, rotation, translation);
		}
		else
		{
			rotation    = Quaternion::identity;
			translation = Vector::zero;
		}
	}
}

// ----------------------------------------------------------------------

int TimedBlendSkeletalAnimation::getTransformPriority(int index) const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getTransformPriority(index) : std::numeric_limits<int>::min());
	else
	{
		int const animation1Priority = (m_animation1 ? m_animation1->getTransformPriority(index) : std::numeric_limits<int>::min());
		int const animation2Priority = (m_animation2 ? m_animation2->getTransformPriority(index) : std::numeric_limits<int>::min());

		return std::min(animation1Priority, animation2Priority);
	}
}

// ----------------------------------------------------------------------

int TimedBlendSkeletalAnimation::getLocomotionPriority() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getLocomotionPriority() : std::numeric_limits<int>::min());
	else
	{
		const int animation1Priority = (m_animation1 ? m_animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
		const int animation2Priority = (m_animation2 ? m_animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

		return std::max(animation1Priority, animation2Priority);
	}
}

// ----------------------------------------------------------------------

void TimedBlendSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	if (m_blendFraction >= 1.0f)
	{
		if (m_animation2)
			m_animation2->getScaledLocomotion(rotation, translation);
		else
		{
			rotation    = Quaternion::identity;
			translation = Vector::zero;
		}
	}
	else
	{
		if (m_animation1 && m_animation2 && (m_animation1 != m_animation2))
		{
			//-- Get animation 1 locomotion.
			Quaternion rotation1(Quaternion::identity);
			Vector     translation1(Vector::zero);
			
			m_animation1->getScaledLocomotion(rotation1, translation1);
			
			//-- Get animation 2 locomotion.
			Quaternion rotation2(Quaternion::identity);
			Vector     translation2(Vector::zero);
			
			m_animation2->getScaledLocomotion(rotation2, translation2);
			
			//-- Blend the result.
			translation = (translation1 * (1.0f - m_blendFraction)) + (translation2 * m_blendFraction);
			rotation    = rotation1.slerp(rotation2, m_blendFraction);
		}
		else if(m_animation1)
		{
			m_animation1->getScaledLocomotion(rotation, translation);
		}
		else if(m_animation2)
		{
			m_animation2->getScaledLocomotion(rotation, translation);
		}
		else
		{
			rotation    = Quaternion::identity;
			translation = Vector::zero;
		}
	}
}

// ----------------------------------------------------------------------

float TimedBlendSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getCycleScaledLocomotionDistance() : 0.0f);
	else
	{
		//-- Get locomotion distance.
		const float distance1 = (m_animation1 ? m_animation1->getCycleScaledLocomotionDistance() : 0.0f);
		const float distance2 = (m_animation2 ? m_animation2->getCycleScaledLocomotionDistance() : 0.0f);

		//-- Blend the result.
		return (1.0f - m_blendFraction) * distance1 + m_blendFraction * distance2;
	}
}

// ----------------------------------------------------------------------

int TimedBlendSkeletalAnimation::getFrameCount() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getFrameCount() : 0);
	else
	{
		const int count1 = (m_animation1 ? m_animation1->getFrameCount() : 0);
		const int count2 = (m_animation2 ? m_animation2->getFrameCount() : 0);

		return std::max(count1, count2);
	}
}

// ----------------------------------------------------------------------

float TimedBlendSkeletalAnimation::getRecordedFramesPerSecond() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getRecordedFramesPerSecond() : 0.0f);
	else
	{
		const float count1 = (m_animation1 ? m_animation1->getRecordedFramesPerSecond() : 0.0f);
		const float count2 = (m_animation2 ? m_animation2->getRecordedFramesPerSecond() : 0.0f);

		return std::max(count1, count2);
	}
}

// ----------------------------------------------------------------------

void TimedBlendSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	DEBUG_WARNING(true, ("unexpected: TimedBlendSkeletalAnimation::setPlaybackFramesPerSecond() called."));

	if (m_animation1)
		m_animation1->setPlaybackFramesPerSecond(playbackFramesPerSecond);

	if (m_animation2)
		m_animation2->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float TimedBlendSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getPlaybackFramesPerSecond() : 0.0f);
	else
	{
		const float count1 = (m_animation1 ? m_animation1->getPlaybackFramesPerSecond() : 0.0f);
		const float count2 = (m_animation2 ? m_animation2->getPlaybackFramesPerSecond() : 0.0f);

		return std::max(count1, count2);
	}
}

// ----------------------------------------------------------------------

int TimedBlendSkeletalAnimation::getMessageCount() const
{
	if (m_blendFraction >= 1.0f)
		return (m_animation2 ? m_animation2->getMessageCount() : 0);
	else
	{
		const int count1 = (m_animation1 ? m_animation1->getMessageCount() : 0);
		const int count2 = (m_animation2 ? m_animation2->getMessageCount() : 0);

		return count1 + count2;
	}
}

// ----------------------------------------------------------------------

const CrcLowerString &TimedBlendSkeletalAnimation::getMessageName(int index) const
{
	if (m_blendFraction >= 1.0f)
	{
		if (m_animation2)
			return m_animation2->getMessageName(index);
		else
			return CrcLowerString::empty;
	}
	else
	{
		const int count1 = (m_animation1 ? m_animation1->getMessageCount() : 0);

		if (index >= count1)
		{
			if (m_animation2)
				return m_animation2->getMessageName(index - count1);
			else
				return CrcLowerString::empty;
		}
		else
		{
			if (m_animation1)
				return m_animation1->getMessageName(index);
			else
				return CrcLowerString::empty;
		}
	}
}

// ----------------------------------------------------------------------

void TimedBlendSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	if (m_blendFraction >= 1.0f)
	{
		if (m_animation2)
			m_animation2->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
	}
	else
	{
		//-- Collect entries for animation 1.
		if (m_animation1)
			m_animation1->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
		
		std::vector<int>::size_type startAdjustIndex = signaledMessageIndices.size();

		//-- Collect and adjust entries for animation 2.
		if (m_animation2)
		{
			m_animation2->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);

			std::vector<int>::size_type endAdjustIndex = signaledMessageIndices.size();
			const int count1 = (m_animation1 ? m_animation1->getMessageCount() : 0);
			
			for (std::vector<int>::size_type i = startAdjustIndex; i < endAdjustIndex; ++i)
				signaledMessageIndices[i] += count1;
		}
	}
}

// ----------------------------------------------------------------------

SkeletalAnimation *TimedBlendSkeletalAnimation::resolveSkeletalAnimation()
{
	//-- Return the animation that is more weighted.
	if (m_blendFraction < 0.50)
		return m_animation1;
	else
		return m_animation2;
}

// ----------------------------------------------------------------------
/**
 * Specify a new animation as the blend target, blending between the previous
 * blend target and the new one.
 */

void TimedBlendSkeletalAnimation::setNewBlendTarget(SkeletalAnimation *animation, float blendTime)
{
	if (blendTime <= 0.0f)
	{
		DEBUG_WARNING(true, ("setNewBlendTarget() called with invalid blend time [%g], skipping.", blendTime));
		return;
	}

	// Fetch local reference on the new animation.
	if (animation)
		animation->fetch();

	// Release local reference on the outgoing animation.
	if (m_animation1)
		m_animation1->release();

	// Move animation 2 to animation 1.
	m_animation1 = m_animation2;
	m_animation2 = animation;

	if (m_animation1 == m_animation2)
	{
		m_blendFraction = 1.0f;
		m_currentTime   = blendTime;
		m_endTime       = blendTime;
		m_blendRate     = 1.0f;
	}
	else
	{
		m_blendFraction = 1.0f - m_blendFraction;

		m_currentTime   = 0.0f;
		m_endTime       = blendTime;
		m_blendRate     = (1.0f - m_blendFraction) / blendTime;
	}
}

// ----------------------------------------------------------------------
/**
 * Provide a mechanism to alter this animation's blend time without propagating
 * the alter to any child animations.
 *
 * @param deltaTime  the amount of new time to apply to the blend.
 */

void TimedBlendSkeletalAnimation::alterBlendTime(float deltaTime)
{
	DEBUG_FATAL(m_terminationPolicy != TP_continueRunning, ("This is invalid to do in any mode other than TP_continueRunning."));

	m_currentTime += deltaTime;
	if (m_currentTime > m_endTime)
	{
		// Blend is fully using animation 2.
		m_currentTime   = m_endTime;
		m_blendFraction = 1.0f;
	
		if (m_animation1)
		{
			m_animation1->release();
			m_animation1 = 0;
		}
	}
	else
	{
		// Blend has more to continue after this loop.
		m_blendFraction    = m_blendRate * m_currentTime;
	}
}

// ======================================================================
// class TimedBlendSkeletalAnimation: private static member functions
// ======================================================================

void TimedBlendSkeletalAnimation::doAnimationAlter(SkeletalAnimation *&animation, float deltaTime)
{
	NOT_NULL(animation);

	SkeletalAnimation *replacementAnimation = 0;

	//-- Alter the animation, ignoring if it requests a loop via remainingDeltaTime > 0.
	//   The net result: I peg an animation such that it stays in its last frame when
	//   the cycle ends.
	// 
	//   Later we may want to allow the behavior at end-of-frame to be configurable.
	const bool keepAnimation = animation->alterSingleCycle(deltaTime, replacementAnimation, deltaTime);
	if (!keepAnimation)
	{
		//-- Animation requested to die.
		if (replacementAnimation)
		{
			// Fetch local reference to new animation.
			replacementAnimation->fetch();
		}

		//-- Release existing animation reference.
		animation->release();

		//-- Assign replacement animation.
		animation = replacementAnimation;
	}
}

// ======================================================================
