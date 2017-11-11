// ======================================================================
//
// TrackAnimationController_AddAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_AddAnimation.h"

#include "clientSkeletalAnimation/TrackAnimationController_Track.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <limits>

// ======================================================================
// inlines
// ======================================================================

inline TrackAnimationController::Track &TrackAnimationController::AddAnimation::getTrack1()
{
	return m_controller.getTrack(m_trackId1);
}

// ----------------------------------------------------------------------

inline TrackAnimationController::Track &TrackAnimationController::AddAnimation::getTrack2()
{
	return m_controller.getTrack(m_trackId2);
}

// ----------------------------------------------------------------------

inline SkeletalAnimation *TrackAnimationController::AddAnimation::getAnimation1()
{
	// return getTrack1().getSkeletalAnimation();
	return m_animation1;
}

// ----------------------------------------------------------------------

inline SkeletalAnimation *TrackAnimationController::AddAnimation::getAnimation2()
{
	// return getTrack2().getSkeletalAnimation();
	return m_animation2;
}

// ----------------------------------------------------------------------

inline const SkeletalAnimation *TrackAnimationController::AddAnimation::getAnimation1() const
{
	// return getTrack1().getSkeletalAnimation();
	return m_animation1;
}

// ----------------------------------------------------------------------

inline const SkeletalAnimation *TrackAnimationController::AddAnimation::getAnimation2() const
{
	// return getTrack2().getSkeletalAnimation();
	return m_animation2;
}

// ======================================================================

TrackAnimationController::AddAnimation::AddAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController, const AnimationTrackId &trackId1, const AnimationTrackId &trackId2) :
	SkeletalAnimation(0),
	m_transformCount(transformNameMap.getTransformCount()),
	m_controller(animationController),
	m_trackId1(trackId1),
	m_trackId2(trackId2),
	m_animation1(0),
	m_animation2(0)
{
}

// ----------------------------------------------------------------------

bool TrackAnimationController::AddAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *& /* replacementAnimation */, float &deltaTimeRemaining)
{
	//-- Alter child tracks first.
	getTrack1().alter(deltaTime);
	getTrack2().alter(deltaTime);

	//-- Retrieve animations from track 1 and 2 for future operations.
	SkeletalAnimation *const animation1 = getTrack1().fetchCurrentAnimation();
	SkeletalAnimation *const animation2 = getTrack2().fetchCurrentAnimation();

	if (m_animation1)
		m_animation1->release();
	m_animation1 = animation1;

	if (m_animation2)
		m_animation2->release();
	m_animation2 = animation2;

	//-- Chew up all time.
	deltaTimeRemaining = 0.0f;

	//-- Never die on its own.
	return true;
}

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::startNewCycle()
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling startNewCycle()."));
}

// ----------------------------------------------------------------------

int TrackAnimationController::AddAnimation::getTransformCount() const
{
	return m_transformCount;
}

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	// @todo incorporate blend between two animations based on priority.

	// The code path allows this to be called in conditions where the cached animations are
	// no longer valid.  We need to ask for these directly.
	SkeletalAnimation *const animation1 = getTrack1().fetchCurrentAnimation();
	SkeletalAnimation *const animation2 = getTrack2().fetchCurrentAnimation();

	//-- Retrieve base animation's rotation, translation.
	if (animation1)
	{
		animation1->evaluateTransformComponents(index, rotation, translation);
		animation1->release();
	}
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}

	//-- Apply add-in animation.
	if (animation2)
	{
		Quaternion addRotation(Quaternion::identity);
		Vector     addTranslation(Vector::zero);

		animation2->evaluateTransformComponents(index, addRotation, addTranslation);
		animation2->release();

		rotation    =  addRotation * rotation;
		translation += addTranslation;
	}
}

// ----------------------------------------------------------------------

int TrackAnimationController::AddAnimation::getTransformPriority(int index) const
{
	const SkeletalAnimation *const animation1 = getAnimation1();
	const SkeletalAnimation *const animation2 = getAnimation2();

	const int animation1Priority = (animation1 ? animation1->getTransformPriority(index) : std::numeric_limits<int>::min());
	const int animation2Priority = (animation2 ? animation2->getTransformPriority(index) : std::numeric_limits<int>::min());

	return std::max(animation1Priority, animation2Priority);
}

// ----------------------------------------------------------------------

int TrackAnimationController::AddAnimation::getLocomotionPriority() const
{
	const SkeletalAnimation *const animation1 = getAnimation1();
	const SkeletalAnimation *const animation2 = getAnimation2();

	const int animation1Priority = (animation1 ? animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
	const int animation2Priority = (animation2 ? animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

	return std::max(animation1Priority, animation2Priority);
}

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	const SkeletalAnimation *const animation1 = getAnimation1();
	const SkeletalAnimation *const animation2 = getAnimation2();

	//-- Retrieve base animation's rotation, translation.
	if (animation1)
	{
		animation1->getScaledLocomotion(rotation, translation);
	}
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}

	//-- Apply add-in animation.
	if (animation2)
	{
		Quaternion addRotation(Quaternion::identity);
		Vector     addTranslation(Vector::zero);

		animation2->getScaledLocomotion(addRotation, addTranslation);

		rotation    =  addRotation * rotation;
		translation += addTranslation;
	}
}

// ----------------------------------------------------------------------

float TrackAnimationController::AddAnimation::getCycleScaledLocomotionDistance() const
{
	const SkeletalAnimation *const animation1 = getAnimation1();
	const SkeletalAnimation *const animation2 = getAnimation2();

	if (!animation1 && !animation2)
	{
		return 0;
	}
	else
	{
		const int animation1Priority = (animation1 ? animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
		const int animation2Priority = (animation2 ? animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

		if (animation1Priority >= animation2Priority)
		{
			NOT_NULL(animation1);
			return animation1->getCycleScaledLocomotionDistance();
		}
		else
		{
			NOT_NULL(animation2);
			return animation2->getCycleScaledLocomotionDistance();
		}
	}
}

// ----------------------------------------------------------------------

int TrackAnimationController::AddAnimation::getFrameCount() const
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling getFrameCount()."));
	return 1;
}

// ----------------------------------------------------------------------

float TrackAnimationController::AddAnimation::getRecordedFramesPerSecond() const
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling getRecordedFramesPerSecond()."));
	return 30.0f;
}

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling setPlaybackFramesPerSecond()."));
	UNREF(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float TrackAnimationController::AddAnimation::getPlaybackFramesPerSecond() const
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling getPlaybackFramesPerSecond()."));
	return 30.0f;
}

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::setFrameNumber(float frameNumber)
{
	DEBUG_WARNING(true, ("AddAnimation: unexpected: calling setFrameNumber()."));
	UNREF(frameNumber);
}

// ----------------------------------------------------------------------

int TrackAnimationController::AddAnimation::getMessageCount() const
{
	const int count1 = (m_animation1 ? m_animation1->getMessageCount() : 0);
	const int count2 = (m_animation2 ? m_animation2->getMessageCount() : 0);

	return count1 + count2;
}

// ----------------------------------------------------------------------

const CrcLowerString &TrackAnimationController::AddAnimation::getMessageName(int index) const
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

// ----------------------------------------------------------------------

void TrackAnimationController::AddAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
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

// ----------------------------------------------------------------------

SkeletalAnimation *TrackAnimationController::AddAnimation::resolveSkeletalAnimation()
{
	//-- Return the primary animation.
	return m_animation1;
}

// ======================================================================

TrackAnimationController::AddAnimation::~AddAnimation()
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

// ======================================================================
