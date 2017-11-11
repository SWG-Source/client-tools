// ======================================================================
//
// TrackAnimationController_PriorityBlendAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_PriorityBlendAnimation.h"

#include "clientSkeletalAnimation/TrackAnimationController_Track.h"

#include <limits>

// ======================================================================
// class TrackAnimationController::PriorityBlendAnimation: public member functions
// ======================================================================

TrackAnimationController::PriorityBlendAnimation::PriorityBlendAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController, const AnimationTrackId &trackId1, const AnimationTrackId &trackId2) :
	BasePriorityBlendAnimation(NULL, transformNameMap),
	m_controller(animationController),
	m_trackId1(trackId1),
	m_trackId2(trackId2),
	m_track1(&animationController.getTrack(trackId1)),
	m_track2(&animationController.getTrack(trackId2)),
	m_optionalTrackIndex(0)
{
}

// ----------------------------------------------------------------------

const TrackAnimationController::Track &TrackAnimationController::PriorityBlendAnimation::getLocomotionPriorityTrack() const
{
	//-- Retrieve the tracks and animations now.  Do not use cached values since this
	//   function may be called prior to alterSingleCycle().  Using cached values may
	//   use the animations set in a previous call, animations that simply may no longer
	//   exist.
	Track &track1 = m_controller.getTrack(m_trackId1);
	Track &track2 = m_controller.getTrack(m_trackId2);

	//-- Retrieve animations from track 1 and 2 for future operations.
	const SkeletalAnimation *const animation1 = track1.fetchCurrentAnimation();
	const SkeletalAnimation *const animation2 = track2.fetchCurrentAnimation();

	int animation1Priority;
	if (animation1)
	{
		animation1Priority = animation1->getLocomotionPriority();
		animation1->release();
	}
	else
		animation1Priority = std::numeric_limits<int>::min();

	int animation2Priority;
	if (animation2)
	{
		animation2Priority = animation2->getLocomotionPriority();
		animation2->release();
	}
	else
		animation2Priority = std::numeric_limits<int>::min();
	
	if (animation1Priority >= animation2Priority)
		return track1;
	else
		return track2;
}

// ----------------------------------------------------------------------

void TrackAnimationController::PriorityBlendAnimation::startNewCycle()
{
	//-- Does nothing.  We wouldn't expect this to be called, though, so lets warn on it.
	DEBUG_WARNING(true, ("PriorityBlendAnimation: unexpected: calling startNewCycle()."));
}

// ======================================================================
// class TrackAnimationController::PriorityBlendAnimation: PROTECTED
// ======================================================================

int TrackAnimationController::PriorityBlendAnimation::getAnimationCount() const
{
	return 2;
}

// ----------------------------------------------------------------------

SkeletalAnimation *TrackAnimationController::PriorityBlendAnimation::getAnimation(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, 2);

	//-- Fetch the animation since this is the available interface, then immediately release it.  We're
	//   not looking to up the ref count for the caller --- all usage is immediate.
	SkeletalAnimation *const animation = (index == 0) ? m_track1->fetchCurrentAnimation() : m_track2->fetchCurrentAnimation();
	if (animation)
		animation->release();

	return animation;
}

// ----------------------------------------------------------------------

SkeletalAnimation const *TrackAnimationController::PriorityBlendAnimation::getAnimation(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, 2);

	//-- Fetch the animation since this is the available interface, then immediately release it.  We're
	//   not looking to up the ref count for the caller --- all usage is immediate.
	SkeletalAnimation const *const animation = (index == 0) ? m_track1->fetchCurrentAnimation() : m_track2->fetchCurrentAnimation();
	if (animation)
		animation->release();

	return animation;
}

// ----------------------------------------------------------------------

bool TrackAnimationController::PriorityBlendAnimation::doAlterSingleCycle(float deltaTime, SkeletalAnimation *& /* replacementAnimation */, float &deltaTimeRemaining)
{
	//-- Get the tracks.
	m_track1 = &(m_controller.getTrack(m_trackId1));
	m_track2 = &(m_controller.getTrack(m_trackId2));

	//-- Alter child tracks.

	// Retrieve locomotion priority from current animations.  This will be used
	// to determine which dependent track will be allowed to emit animation messages.
	// NOTE: This may not be sufficient for upper body animations that need to trigger
	// some action.
	SkeletalAnimation *animation1 = m_track1->fetchCurrentAnimation();
	SkeletalAnimation *animation2 = m_track2->fetchCurrentAnimation();

	int  animation1LocPriority;
	if (animation1)
	{
		animation1LocPriority = animation1->getLocomotionPriority();
		animation1->release();
	}
	else
		animation1LocPriority = std::numeric_limits<int>::min();

	int  animation2LocPriority;
	if (animation2)
	{
		animation2LocPriority = animation2->getLocomotionPriority();
		animation2->release();
	}
	else
		animation2LocPriority = std::numeric_limits<int>::min();

	bool const useTrack1Messages = (animation1LocPriority >= animation2LocPriority);

	// Alter the child tracks.
	m_track1->alter(deltaTime, useTrack1Messages);
	m_track2->alter(deltaTime, !useTrack1Messages);

	//-- Chew up all time.
	deltaTimeRemaining = 0.0f;

	//-- Test for priority blend animation idle condition, in which case we disable the blend track.
	if (readyToDisableTrack())
	{
		disableTrack();
		return true;
	}
	//-- Never die on its own.
	return true;
}

// ----------------------------------------------------------------------

void TrackAnimationController::PriorityBlendAnimation::doEvaluateTransformComponents(int animationIndex, int transformIndex, Quaternion &rotation, Vector &translation)
{
	//-- Override this function so that we call the Track's version of evaluateTransformComponents().
	//   The track will delegate to the underlying animation if an animation is still playing.  If no
	//   animation is playing, it returns the last state of the animation prior to ending.
	//
	// @todo: think about this: feels like I may be able to pull out this facet of animations 
	//        from the track and add it to the animation concept.

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, animationIndex, 2);

	Track *const track = (animationIndex == 0) ? m_track1 : m_track2;
	NOT_NULL(track);

	track->evaluateTransformComponents(transformIndex, rotation, translation);
}

// ======================================================================
// class TrackAnimationController::PriorityBlendAnimation: PRIVATE
// ======================================================================

TrackAnimationController::PriorityBlendAnimation::~PriorityBlendAnimation()
{
	m_track2 = 0;
	m_track1 = 0;
}

// ----------------------------------------------------------------------
/**
 * Determine if the optional track can be disabled.
 *
 * The optional track can be disabled if its animation is null and if
 * all of the blends are 100% on the other animation.
 *
 * @return  true if the optional track is ready to be disabled; false if it should remain enabled.
 */

bool TrackAnimationController::PriorityBlendAnimation::readyToDisableTrack() const
{
	//-- Check if the given track animation is null.
	if (getAnimation(m_optionalTrackIndex) != NULL)
	{
		//-- The animation for the optional track is not null, so it's not time to disable the optional track.
		return false;
	}

	bool const allNonOptional = allBlendStatesMatch((m_optionalTrackIndex == 0) ? BS_playB : BS_playA);

	//-- If the blender is not playing the non-optional animation at 100%, the optional track cannot be disabled.
	return allNonOptional;
}

// ----------------------------------------------------------------------

void TrackAnimationController::PriorityBlendAnimation::disableTrack()
{
	m_controller.disableTrack((m_optionalTrackIndex == 0) ? m_trackId1 : m_trackId2);
}

// ======================================================================
