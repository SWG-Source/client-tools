// ======================================================================
//
// FaceTargetAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FaceTargetAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/CreatureController.h"
#include "clientGame/FaceTargetActionTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <algorithm>

// ======================================================================
// inlines
// ======================================================================

inline const FaceTargetActionTemplate &FaceTargetAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const FaceTargetActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class FaceTargetAction: public member functions
// ======================================================================

FaceTargetAction::FaceTargetAction(const FaceTargetActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initializedPlaybackData(false),
	m_maxRotationalSpeed(PI_TIMES_2 / 1.0f),
	m_onTargetThresholdAngle(10.0f * PI_OVER_180),
	m_timeToLive(1.0f)
{
}

// ----------------------------------------------------------------------

bool FaceTargetAction::update(float deltaTime, PlaybackScript &script)
{
	const FaceTargetActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the actor to rotate.
	Object *const rotateObject = script.getActor(actionTemplate.getRotationActorIndex());
	if (!rotateObject)
		return false;

	//-- Get target actor.  If target isn't available, abort the action.
	Object *const targetObject = script.getActor(actionTemplate.getTargetActorIndex());
	if (!targetObject)
		return false;

	//-- Initialize the action first time through.
	if (!m_initializedPlaybackData)
	{
		//-- Check if we should only rotate skeletal appearances.
		if (actionTemplate.getLimitRotationToSkeletalAppearance())
		{
			SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(rotateObject->getAppearance());
			if (!appearance)
			{
				// This object doesn't have a skeletal appearance, don't bother causing it to face the target.
				return false;
			}
		}

		//-- Retrieve values for script variables.
		bool success = script.getFloatVariable(actionTemplate.getMaxRotationalSpeedVariable(), m_maxRotationalSpeed);
		DEBUG_WARNING(!success, ("max rotational speed variable missing."));

		success = script.getFloatVariable(actionTemplate.getOnTargetThresholdAngleVariable(), m_onTargetThresholdAngle);
		DEBUG_WARNING(!success, ("on target threshold angle variable missing."));

		success = script.getFloatVariable(actionTemplate.getMaxTimeToLiveVariable(), m_timeToLive);
		DEBUG_WARNING(!success, ("on target threshold angle variable missing."));

		//-- Tell rotate actor to ignore dead reckoning for a specified duration.  If this wasn't done,
		//   the rotation supplied by this action would be overridden by dead reckoning.
		CreatureController *const controller = dynamic_cast<CreatureController*>(rotateObject->getController());
		if (controller)
		{
			//-- Retrieve the dead reckoning ignore time duration from the script.
			float deadReckoningIgnoreDuration = 5.0f;

			success = script.getFloatVariable(actionTemplate.getIgnoreDeadReckoningDurationVariable(), deadReckoningIgnoreDuration);
			DEBUG_WARNING(!success, ("deadReckoningIgnoreDuration variable missing."));

			//-- Disable dead reckoning for the specified amount of time.
			controller->pauseDeadReckoning(deadReckoningIgnoreDuration);
		}

		m_initializedPlaybackData = true;
	}

	//-- Get the target position in rotate object's local space.
	const Vector separationVector = rotateObject->rotateTranslate_w2o(targetObject->getPosition_w());

	const float separationTheta = separationVector.theta();
	if (WithinRangeInclusiveInclusive(-m_onTargetThresholdAngle, separationTheta, m_onTargetThresholdAngle))
	{
		//-- Rotate object is within threshold facing angle from target, end the action.
		return false;
	}

	//-- Figure out angle to rotate (yaw only).
	float yaw;

	if (separationTheta < 0.0f)
		yaw = std::max(separationTheta, -m_maxRotationalSpeed * deltaTime);
	else
		yaw = std::min(separationTheta, m_maxRotationalSpeed * deltaTime);

	//-- Yaw the object.
	rotateObject->yaw_o(yaw);

	//-- Check if time to live has expired.
	m_timeToLive -= deltaTime;
	
	//-- Continue running if time to live has not expired.
	return (m_timeToLive > 0.0f);
}

// ======================================================================
