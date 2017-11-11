// ======================================================================
//
// SpeedSkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ControlledBlendSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TimedBlendSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"
#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

namespace SpeedSkeletalAnimationNamespace
{
	float cs_transitionToIdleBlendTime       = 0.3f;
	float cs_transitionToLocomotionBlendTime = 0.25f;
}

using namespace SpeedSkeletalAnimationNamespace;

// ======================================================================

#if TRACK_SPEED

namespace
{
	const int  cs_averagingSampleCount = 100;
}

#endif

// ======================================================================

class SpeedSkeletalAnimation::SpeedChoice
{
public:

	struct LessRecordedSpeedComparator
	{
		bool operator ()(const SpeedChoice *lhs, const SpeedChoice *rhs) const;
	};

public:

	SpeedChoice(SkeletalAnimation *animation, float cycleLocoDistance);
	~SpeedChoice();

	void   setCyclesPerSecond(float cyclesPerSecond);
	float  alterSingleCycle(float deltaTime);
	void   startNewCycle();

	float  calculateCycleFrequency(float desiredLocomotionSpeed) const;

	SkeletalAnimation *getAnimation();
	float              getRecordedSpeed() const;
	float              getCycleLocomotionDistance() const;

private:

	// disabled
	SpeedChoice();

private:

	SkeletalAnimation *m_animation;

	// speed in meters per second.
	float              m_recordedSpeed;
	float              m_factorDesiredSpeedToPlaybackFps;
	float              m_locoSpeedToCycleFrequencyFactor;
	float              m_frameCount;
	float              m_cycleLocomotionDistance;
};

// ======================================================================

SpeedSkeletalAnimation::SpeedChoice::SpeedChoice(SkeletalAnimation *animation, float cycleLocoDistance) :
	m_animation(NON_NULL(animation)),
	m_recordedSpeed(1.0f),
	m_factorDesiredSpeedToPlaybackFps(1.0f),
	m_locoSpeedToCycleFrequencyFactor(1.0f),
	m_frameCount(static_cast<float>(animation->getFrameCount())),
	m_cycleLocomotionDistance(cycleLocoDistance)
{
	//-- Calculate recorded scaled, average locomotion speed (m_recordedSpeed).
	const float recordedFps       = animation->getRecordedFramesPerSecond();
	const float recordedCycleTime = (recordedFps > 0.0f ? m_frameCount / recordedFps : 0.0f);

	m_recordedSpeed = (recordedCycleTime > 0.0f ? cycleLocoDistance / recordedCycleTime : 0.0f);

	//-- Calculate desired speed -> playback fps conversion factor.
	m_factorDesiredSpeedToPlaybackFps = (m_recordedSpeed > 0.0f ? recordedFps / m_recordedSpeed : 1.0f);

	//-- Calculate the factor to multiply against desired speed to determine required cycles per second.
	m_locoSpeedToCycleFrequencyFactor = (cycleLocoDistance > 0.0f ? (1.0f / cycleLocoDistance) : 1.0f);

	//-- Fetch local reference to animation.
	m_animation->fetch();
}

// ----------------------------------------------------------------------

SpeedSkeletalAnimation::SpeedChoice::~SpeedChoice()
{
	m_animation->release();
	m_animation = 0;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::SpeedChoice::setCyclesPerSecond(float cyclesPerSecond)
{
	//-- Set playback frame rate from cycles per second.  m_frameCount == frames per cycle.
	m_animation->setPlaybackFramesPerSecond(cyclesPerSecond * m_frameCount);
}

// ----------------------------------------------------------------------

float SpeedSkeletalAnimation::SpeedChoice::alterSingleCycle(float deltaTime)
{
	float deltaTimeRemaining = 0.0f;
	SkeletalAnimation *replacementAnimation = 0;

	const bool keepAnimation = m_animation->alterSingleCycle(deltaTime, replacementAnimation, deltaTimeRemaining);
	if (!keepAnimation)
	{
		NOT_NULL(replacementAnimation);
		// DEBUG_WARNING(!replacementAnimation, ("speed animation died but no replacement animation, don't know how to handle."));
		replacementAnimation->fetch();

		m_animation->release();
		m_animation = replacementAnimation;
	}

	return deltaTimeRemaining;
}

// ----------------------------------------------------------------------

inline void SpeedSkeletalAnimation::SpeedChoice::startNewCycle()
{
	m_animation->startNewCycle();
}

// ----------------------------------------------------------------------

inline float SpeedSkeletalAnimation::SpeedChoice::calculateCycleFrequency(float desiredLocomotionSpeed) const
{
	return desiredLocomotionSpeed * m_locoSpeedToCycleFrequencyFactor;
}

// ----------------------------------------------------------------------

inline SkeletalAnimation *SpeedSkeletalAnimation::SpeedChoice::getAnimation()
{
	return m_animation;
}

// ----------------------------------------------------------------------

inline float SpeedSkeletalAnimation::SpeedChoice::getRecordedSpeed() const
{
	return m_recordedSpeed;
}

// ----------------------------------------------------------------------

inline float SpeedSkeletalAnimation::SpeedChoice::getCycleLocomotionDistance() const
{
	return m_cycleLocomotionDistance;
}

// ======================================================================

inline bool SpeedSkeletalAnimation::SpeedChoice::LessRecordedSpeedComparator::operator ()(const SpeedChoice *lhs, const SpeedChoice *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getRecordedSpeed() < rhs->getRecordedSpeed();
}

// ======================================================================
// inlines
// ======================================================================

inline const SpeedSkeletalAnimationTemplate &SpeedSkeletalAnimation::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const SpeedSkeletalAnimationTemplate*>(getSkeletalAnimationTemplate()));
}

// ----------------------------------------------------------------------
/** 
 * Retrieve the animation that should be queried for all non-evaluation
 * purposes.
 *
 * This will either be the idle animation or the blended locomotion animation.
 */

inline SkeletalAnimation *SpeedSkeletalAnimation::getFocusAnimation()
{
	return (m_currentState == S_idle) ? m_idleAnimation : m_locomotionAnimation;
}

// ----------------------------------------------------------------------
/** 
 * Retrieve the animation that should be queried for all non-evaluation
 * purposes.
 *
 * This will either be the idle animation or the blended locomotion animation.
 */

inline const SkeletalAnimation *SpeedSkeletalAnimation::getFocusAnimation() const
{
	return (m_currentState == S_idle) ? m_idleAnimation : m_locomotionAnimation;
}

// ======================================================================

void SpeedSkeletalAnimation::install()
{
	cs_transitionToLocomotionBlendTime = ConfigClientSkeletalAnimation::getBlendTime();	
	cs_transitionToIdleBlendTime = cs_transitionToLocomotionBlendTime + 0.05f;
}

SpeedSkeletalAnimation::SpeedSkeletalAnimation(const SpeedSkeletalAnimationTemplate &animationTemplate, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) :
	SkeletalAnimation(&animationTemplate),
	m_speedChoices(new SpeedChoiceVector()),
	m_speedChoiceCount(0),
	m_locomotionAnimation(new ControlledBlendSkeletalAnimation(true, ControlledBlendSkeletalAnimation::BE_simpleBlend)),
	m_idleAnimation(0),
	m_evaluationAnimation(0),
	m_animationEnvironment(animationEnvironment),
	m_transformNameMap(transformNameMap),
	m_locomotionVelocity(animationEnvironment.getVector(AnimationEnvironmentNames::cms_locomotionVelocity)),
	m_currentState(S_idle)
#if TRACK_SPEED
	,	m_averagingIndex(0)
	, m_averagingDistance(new FloatVector(cs_averagingSampleCount))
	, m_averagingTime(new FloatVector(cs_averagingSampleCount))
#endif
{
	//-- Fetch local reference to current animation.
	m_locomotionAnimation->fetch();

	//-- Fetch an animation for each speed choice provided by the template.
	const int templateSpeedEntryCount = animationTemplate.getSpeedChoiceCount();;
	m_speedChoices->reserve(static_cast<SpeedChoiceVector::size_type>(templateSpeedEntryCount));

	const float epsilon = 0.01f;

	for (int i = 0; i < templateSpeedEntryCount; ++i)
	{
		const SkeletalAnimationTemplate *const speedChoiceTemplate = animationTemplate.fetchSpeedChoiceAnimationTemplate(i);
		if (speedChoiceTemplate)
		{
			SkeletalAnimation *const speedChoiceAnimation = speedChoiceTemplate->fetchSkeletalAnimation(animationEnvironment, transformNameMap);
			if (speedChoiceAnimation)
			{
				//-- Get distance moved during animation.
				const float cycleLocoDistance = speedChoiceAnimation->getCycleScaledLocomotionDistance();

				//-- If distance moved is nearly zero, assume this is the idle animation.
				if (WithinEpsilonInclusive(0.0f, cycleLocoDistance, epsilon))
				{
					// This should be an idle animation.
					if (m_idleAnimation)
					{
						DEBUG_WARNING(m_idleAnimation, ("speed animation appears to have multiple zero-speed animations for [%s].", 
							animationEnvironment.getSkeletalAppearance() ? animationEnvironment.getSkeletalAppearance()->getAppearanceTemplateName() : "NULL"));
						m_idleAnimation->release();
					}

					m_idleAnimation = speedChoiceAnimation;
				}
				else
				{
					// This should be a non-zero-speed animation.
					m_speedChoices->push_back(new SpeedChoice(speedChoiceAnimation, cycleLocoDistance));

					//-- Release local reference to animation.
					speedChoiceAnimation->release();
				}
			}

			//-- Release local reference to animation template.
			speedChoiceTemplate->release();
		}
	}

	m_speedChoiceCount = static_cast<int>(m_speedChoices->size());

	//-- Sort speed choice entries by recorded speed, starting from less to greater.
	std::sort(m_speedChoices->begin(), m_speedChoices->end(), SpeedChoice::LessRecordedSpeedComparator());

	//-- Create the evaluation animation, initialize it to be 100% idle.
	m_evaluationAnimation = new TimedBlendSkeletalAnimation(m_idleAnimation);
	m_evaluationAnimation->fetch();
}

// ----------------------------------------------------------------------

bool SpeedSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	//-- We never have a replacement animation.
	replacementAnimation = 0;

	//-- Update state, evaluation animation, and locomotion animation blend based on controller's desired speed.
	const float desiredSpeed            = m_locomotionVelocity.magnitude();
	const float playbackCyclesPerSecond = updateAnimationState(desiredSpeed);

	//-- Handle altering of idle or locomotion animations.
	if (m_currentState == S_idle)
	{
		if (m_idleAnimation)
		{
			//-- Alter the idle animation.
			SkeletalAnimation *idleReplacementAnimation = 0;

			const bool keepAnimation = m_idleAnimation->alterSingleCycle(deltaTime, idleReplacementAnimation, deltaTimeRemaining);
			if (!keepAnimation)
			{
				// Fetch replacement animation.
				if (idleReplacementAnimation)
					idleReplacementAnimation->fetch();

				// Release existing idle animation.
				m_idleAnimation->release();

				// Replace existing idle animation.
				m_idleAnimation = idleReplacementAnimation;
			}
		}
	}
	else
	{
		// Speed controller is locomoting.
		if (m_speedChoices->empty())
			deltaTimeRemaining = 0.0f;
		else
		{
			//-- Tell all non-idle speed choice animations to set playback rate for specified velocity.
			std::for_each(m_speedChoices->begin(), m_speedChoices->end(), VoidBindSecond(VoidMemberFunction(&SpeedChoice::setCyclesPerSecond), playbackCyclesPerSecond));

			//-- Animate the first speed choice.  If it needs to loop, loop all animations.  This keeps each
			//   animation in sync at least at the cycle granularity.
			SpeedChoiceVector::iterator it = m_speedChoices->begin();

			deltaTimeRemaining = (*it)->alterSingleCycle(deltaTime);
			std::for_each(++it, m_speedChoices->end(), std::bind2nd(std::mem_fun(&SpeedChoice::alterSingleCycle), deltaTime));
		}

#if TRACK_SPEED
		//-- Update speed averaging data.
		m_averagingIndex = (m_averagingIndex + 1) % cs_averagingSampleCount;
		(*m_averagingTime)[static_cast<FloatVector::size_type>(m_averagingIndex)] = deltaTime - deltaTimeRemaining;
#endif
	}

	//-- Alter the evaluation animation's timer, not the child animations.
	m_evaluationAnimation->alterBlendTime(deltaTime);

	//-- Indicate this animation never dies by its own choice.
	return true;
} //lint !e1764 // replacementAnimation could be made const // no, that would break the virtual contract.

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::startNewCycle()
{
	if (m_currentState == S_idle)
	{
		if (m_idleAnimation)
			m_idleAnimation->startNewCycle();
	}
	else
		std::for_each(m_speedChoices->begin(), m_speedChoices->end(), VoidMemberFunction(&SpeedChoice::startNewCycle));
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimation::getTransformCount() const
{
	return m_transformNameMap.getTransformCount();
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_evaluationAnimation->evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimation::getTransformPriority(int index) const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getTransformPriority(index);
	else
		return std::numeric_limits<int>::min();
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimation::getLocomotionPriority() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getLocomotionPriority();
	else
		return std::numeric_limits<int>::min();
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		animation->getScaledLocomotion(rotation, translation);
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}

#if TRACK_SPEED
	//-- Set current sample distance.
	float &currentDeltaDistance    = (*m_averagingDistance)[static_cast<FloatVector::size_type>(m_averagingIndex)];
	currentDeltaDistance           = translation.magnitude();

	//-- Calculate current speed.
	const float  currentDeltaTime  = (*m_averagingTime)[static_cast<FloatVector::size_type>(m_averagingIndex)];
	const float  currentSpeed      = (currentDeltaTime > 0.0f) ? (currentDeltaDistance / currentDeltaTime) : 0.0f;
	UNREF(currentSpeed);

	//-- Calculate average speed.
	float totalTime     = 0.0f;
	float totalDistance = 0.0f;

	for (int i = 0; i < cs_averagingSampleCount; ++i)
	{
		totalTime     += (*m_averagingTime)[static_cast<FloatVector::size_type>(i)];
		totalDistance += (*m_averagingDistance)[static_cast<FloatVector::size_type>(i)];
	}

	const float averageSpeed = (totalTime > 0.0f) ? (totalDistance / totalTime) : 0.0f;
	UNREF(averageSpeed);

	//-- Print results.
	DEBUG_REPORT_PRINT(true, ("actual speed:  %.2f (%.2f)\n", averageSpeed, currentSpeed));
#endif
}

// ----------------------------------------------------------------------

float SpeedSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getCycleScaledLocomotionDistance();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimation::getFrameCount() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getFrameCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

float SpeedSkeletalAnimation::getRecordedFramesPerSecond() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getRecordedFramesPerSecond();
	else
		return 0;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	// @todo if this should be here at all, it probably should change all of the animations.
	DEBUG_WARNING(true, ("SpeedSkeletalAnimation::setPlaybackFramesPerSecond() not handled properly. -TRF- find why this is called."));
	m_locomotionAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float SpeedSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getPlaybackFramesPerSecond();
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimation::getMessageCount() const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getMessageCount();
	else
		return false;
}

// ----------------------------------------------------------------------

const CrcLowerString &SpeedSkeletalAnimation::getMessageName(int index) const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		return animation->getMessageName(index);
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	const SkeletalAnimation *const animation = getFocusAnimation();

	if (animation)
		animation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *SpeedSkeletalAnimation::resolveSkeletalAnimation()
{
	//-- Select the current animation to be the animation with the closest average speed to the desired average speed.
	const float desiredSpeed            = m_locomotionVelocity.magnitude();
	const float playbackCyclesPerSecond = updateAnimationState(desiredSpeed);
	UNREF(playbackCyclesPerSecond);

	//-- Return the current animation.
	return getFocusAnimation();
}

// ======================================================================

SpeedSkeletalAnimation::~SpeedSkeletalAnimation()
{
#if TRACK_SPEED
	delete m_averagingTime;
	delete m_averagingDistance;
#endif

	if (m_idleAnimation)
	{
		m_idleAnimation->release();
		m_idleAnimation = 0;
	}

	m_locomotionAnimation->release();
	m_locomotionAnimation = 0; //lint !e423 // creation of memory leak.  No, this is reference counted.

	m_evaluationAnimation->release();
	m_evaluationAnimation = 0; //lint !e423 // creation of memory leak.  No, this is reference counted.
	
	//-- Clear out all animations
	std::for_each(m_speedChoices->begin(), m_speedChoices->end(), PointerDeleter());
	delete m_speedChoices;
}

// ----------------------------------------------------------------------
/**
 * Set m_locomotionAnimation to point to the two animations closest in speed.
 *
 * @return  the cycles per second at which each animation should run.
 */

float SpeedSkeletalAnimation::updateAnimationState(float desiredLocomotionSpeed)
{
	//-- Update state based on velocity, remember previous state.
	const State previousState = m_currentState;
	
	if (desiredLocomotionSpeed > 0.0f)
		m_currentState = S_locomoting;
	else
		m_currentState = S_idle;

	// @todo fix this confusing interface, all this can be wrapped up in the alterSingleCycle call.
	//       This function doesn't need to return anything in the case of idle.
	float cyclesPerSecond = 0.0f;

	//-- Handle state transitions.
	switch (m_currentState)
	{
		case S_idle:
			{
				if (previousState != S_idle)
				{
					//-- Transition to the idle state.
					m_evaluationAnimation->setNewBlendTarget(m_idleAnimation, cs_transitionToIdleBlendTime);
				}
			}
			break;

		case S_locomoting:
			{
				if (previousState != S_locomoting)
				{
					// This will look bad in start-stop animation, commented out for now.  When starting from a clean stop, this looks better.
#if 0
					//-- Start
					std::for_each(m_speedChoices->begin(), m_speedChoices->end(), VoidMemberFunction(&SpeedChoice::startNewCycle));
#endif

					//-- Transition to the locomotion state.
					m_evaluationAnimation->setNewBlendTarget(m_locomotionAnimation, cs_transitionToLocomotionBlendTime);
				}

				cyclesPerSecond = prepareLocomotionAnimation(desiredLocomotionSpeed);
			}
			break;

		default:
			DEBUG_FATAL(true, ("Invalid SpeedSkeletalAnimation state [%d].\n", m_currentState));
	}

	return cyclesPerSecond;
}

// ----------------------------------------------------------------------

float SpeedSkeletalAnimation::prepareLocomotionAnimation(float desiredLocomotionSpeed)
{
	//-- Leave if no animations to choose from.
	if (m_speedChoices->empty())
		return 1.0;

	SkeletalAnimation *animation1 = 0;
	SkeletalAnimation *animation2 = 0;

	//-- Find animation with closest recorded locomotion speed to the desired locomotion speed.
	float cycleFrequency1        = 1.0f;
	float deltaFromDesiredSpeed1 = std::numeric_limits<float>::max();
	int   indexAnimation1        = -1;

	int index = 0;
	const SpeedChoiceVector::iterator endIt = m_speedChoices->end();
	for (SpeedChoiceVector::iterator it = m_speedChoices->begin(); it != endIt; ++it, ++index)
	{
		//-- Get delta of this animation's natural speed from the desired speed, make delta positive.
		float newDelta = (*it)->getRecordedSpeed() - desiredLocomotionSpeed;
		if (newDelta < 0.0f)
			newDelta = -newDelta;

		if (newDelta < deltaFromDesiredSpeed1)
		{
			deltaFromDesiredSpeed1 = newDelta;
			animation1             = (*it)->getAnimation();
			cycleFrequency1        = (*it)->calculateCycleFrequency(desiredLocomotionSpeed);
			indexAnimation1        = index;
		}
		else
		{
			//-- This gets executed when the delta starts rising.  Since I can count on
			//   SpeedChoice entries ordered by increasing velocity, this should be okay.
			break;
		}
	}

	//-- Find the closest animation on the other side of the desired speed choice.
	SpeedChoice *speedChoice2 = 0;

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, indexAnimation1, m_speedChoiceCount);
	SpeedChoice &speedChoice1 = *NON_NULL((*m_speedChoices)[static_cast<SpeedChoiceVector::size_type>(indexAnimation1)]);

#if TRACK_SPEED
	DEBUG_REPORT_PRINT(true, ("anim 1: rec speed %.2f, delta %.2f\n", speedChoice1.getRecordedSpeed(), deltaFromDesiredSpeed1));
#endif

	const float recordedLocSpeed1 = speedChoice1.getRecordedSpeed();
	if (recordedLocSpeed1 > desiredLocomotionSpeed)
	{
		//-- The next blend speed animation should be one choice slower.
		if (indexAnimation1 > 0)
			speedChoice2 = (*m_speedChoices)[static_cast<SpeedChoiceVector::size_type>(indexAnimation1 - 1)];
	}
	else
	{
		//-- The next blend speed animation should be one choice faster.
		if (indexAnimation1 < (m_speedChoiceCount - 1))
			speedChoice2 = (*m_speedChoices)[static_cast<SpeedChoiceVector::size_type>(indexAnimation1 + 1)];
	}

	float  resultingCycleFrequency;
	float  blendFactor; // 1.0 = full animation2, 0.0 = full animation1

	if (!speedChoice2)
	{
		//-- Only one animation to consider, use it.
		resultingCycleFrequency = cycleFrequency1;
		blendFactor             = 0.0f;
	}
	else
	{
		//-- Calculate blend factor between animation1 and animation2.
		float deltaFromDesiredSpeed2 = speedChoice2->getRecordedSpeed() - desiredLocomotionSpeed;
		if (deltaFromDesiredSpeed2 < 0.0f)
			deltaFromDesiredSpeed2 = -deltaFromDesiredSpeed2;

		blendFactor = deltaFromDesiredSpeed1 / (deltaFromDesiredSpeed1 + deltaFromDesiredSpeed2);

		//-- Calculate resulting frequency
		const float blendedDistancePerCycle = (1.0f - blendFactor) * speedChoice1.getCycleLocomotionDistance() + blendFactor * speedChoice2->getCycleLocomotionDistance();
		if (blendedDistancePerCycle > 0.0f)
			resultingCycleFrequency = desiredLocomotionSpeed / blendedDistancePerCycle;
		else
		{
			// I should only get here if locomotion distances per cycle are very very small.  This most likely would be caused by data badness.
			DEBUG_WARNING(true, ("serious speed animation issues.\n"));
			resultingCycleFrequency = 10.0f;
		}

		//-- Get the second participating animation.
		animation2 = speedChoice2->getAnimation();

#if TRACK_SPEED
		DEBUG_REPORT_PRINT(true, ("anim 2: rec speed %.2f, delta %.2f, blend %.2f\n", speedChoice2->getRecordedSpeed(), deltaFromDesiredSpeed2, blendFactor));

#if 0
		const float expectedSpeed = (1.0f - blendFactor) * (speedChoice1.getRecordedSpeed() * resultingCycleFrequency / speedChoice1.calculateCycleFrequency(speedChoice1.getRecordedSpeed())) +
		                            blendFactor          * (speedChoice2->getRecordedSpeed() * resultingCycleFrequency / speedChoice2->calculateCycleFrequency(speedChoice2->getRecordedSpeed()));
		DEBUG_REPORT_PRINT(true, ("expected average speed: %.2f\n", expectedSpeed));
#endif
#endif
	}

	m_locomotionAnimation->setAnimation1(animation1);
	m_locomotionAnimation->setAnimation2(animation2);
	m_locomotionAnimation->setBlendFactor(blendFactor);

#if TRACK_SPEED
	DEBUG_REPORT_PRINT(true, ("desired speed: %.2f\n", desiredLocomotionSpeed));
#endif

	return resultingCycleFrequency;
}

// ======================================================================
