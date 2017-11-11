// ======================================================================
//
// ChangeScaleAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ChangeScaleAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ChangeScaleActionTemplate.h"
#include "clientGame/CreatureObject.h"

#include <string>

// ======================================================================

namespace
{
	std::string  s_alignmentHardpointName;
	Transform    s_hardpointTransform;
	Transform    s_newTransform_o2p;
}

// ======================================================================
// inlines
// ======================================================================

inline const ChangeScaleActionTemplate &ChangeScaleAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const ChangeScaleActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class ChangeScaleAction: public member functions
// ======================================================================

ChangeScaleAction::ChangeScaleAction(const ChangeScaleActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initialized(false),
	m_deltaScaleDeltaTime(1.0f),
	m_accumulatedTime(0.0f),
	m_totalTimeDuration(1.0f),
	m_startScale(1.0f)
{
}

// ----------------------------------------------------------------------

bool ChangeScaleAction::update(float deltaTime, PlaybackScript &script)
{
	//-- Initialize first time through the loop.
	if (!m_initialized)
	{
		//-- Initialize the script.
		const bool initializeSuccess = initialize(script);
		if (!initializeSuccess)
			return false;

		m_initialized = true;
	}
	else
	{
		//-- Get the template.
		const ChangeScaleActionTemplate &actionTemplate = getOurTemplate();

		Object *const object = script.getActor(actionTemplate.getActorIndex());
		if (!object)
		{
			DEBUG_REPORT_LOG(true, ("ChangeScaleAction::update(): scaled actor object is NULL now, skipping remaining scale application.\n"));
			return false;
		}

		//-- Adjust accumulated time.
		m_accumulatedTime = std::min(m_accumulatedTime + deltaTime, m_totalTimeDuration);

		//-- Calculate current scale.
		const float currentScale = m_startScale + m_accumulatedTime * m_deltaScaleDeltaTime;

		//-- Set the Object's scale.
		object->setScale(Vector(currentScale, currentScale, currentScale));
	}

	//-- Continue as long as accumulated time is less than total runtime.
	return (m_accumulatedTime < m_totalTimeDuration);
}

// ----------------------------------------------------------------------

bool ChangeScaleAction::initialize(PlaybackScript &script)
{
	//-- Get the template.
	const ChangeScaleActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the actor that is getting scaled.
	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*>(script.getActor(actionTemplate.getActorIndex()));
	if (!creatureObject)
	{
		DEBUG_WARNING(true, ("Object instance used for scale change is not available or not a CreatureObject, skipping scale."));
		return false;
	}

	//-- Get actor object's authoratative scale.
	const float authoratativeScale = creatureObject->getScaleFactor();

	//-- Get time duration for blend.
	m_totalTimeDuration = 0.5f;

	const bool gotTimeDuration = script.getFloatVariable(actionTemplate.getTimeDurationVariable(), m_totalTimeDuration);
	if (!gotTimeDuration)
	{
		char buffer[5];

		ConvertTagToString(actionTemplate.getTimeDurationVariable(), buffer);
		DEBUG_WARNING(true, ("time duration variable [%s] was not set in the playback script, scaling ignored.", buffer));

		return false;
	}

	// Validate scale value.
	if (m_totalTimeDuration <= 0.0f)
	{
		DEBUG_WARNING(true, ("time duration for scale set to invalid value [%g], scaling ignored.", m_totalTimeDuration));
		return false;
	}

	//-- Get the target scale.
	float targetScale = 1.0f;

	const bool gotTargetScale = script.getFloatVariable(actionTemplate.getTargetScaleVariable(), targetScale);
	if (!gotTargetScale)
	{
		char buffer[5];

		ConvertTagToString(actionTemplate.getTargetScaleVariable(), buffer);
		DEBUG_WARNING(true, ("target scale variable [%s] was not set in the playback script, scaling ignored.", buffer));

		return false;
	}

	// Validate target scale.
	if (targetScale <= 0.0f)
	{
		DEBUG_FATAL(true, ("target scale set to invalid value [%g], scaling ignored.", targetScale));
		return false; //lint -e527 // Unreachable // Reachable in release.
	}

	//-- Calculate scale change per unit time.
	float  deltaScale = 0.0f;

	switch (actionTemplate.getBlendMode())
	{
		case ChangeScaleActionTemplate::BM_objectToTarget:
			m_startScale = authoratativeScale;
			deltaScale   = targetScale - authoratativeScale;
			break;

		case ChangeScaleActionTemplate::BM_targetToObject:
			m_startScale = targetScale;
			deltaScale   = authoratativeScale - targetScale;
			break;
	}

	m_deltaScaleDeltaTime = deltaScale / m_totalTimeDuration;

	m_accumulatedTime = 0.0f;

	//-- Success.
	return true;
}

// ======================================================================
