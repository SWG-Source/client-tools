// ======================================================================
//
// TimerPlaybackAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/TimerPlaybackAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/TimerPlaybackActionTemplate.h"

// ======================================================================
// inlines
// ======================================================================

inline const TimerPlaybackActionTemplate &TimerPlaybackAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const TimerPlaybackActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================

TimerPlaybackAction::TimerPlaybackAction(const TimerPlaybackActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initialized(false),
	m_totalTime(0.0f),
	m_accumulatedTime(0.0f)
{
}

// ----------------------------------------------------------------------

bool TimerPlaybackAction::update(float deltaTime, PlaybackScript &script)
{
	if (!m_initialized)
	{
		const bool gotDuration = script.getFloatVariable(getOurTemplate().getTimeDurationVariable(), m_totalTime);
		if (!gotDuration || (m_totalTime < 0.0f))
		{
			DEBUG_WARNING(true, ("TimerPlaybackAction: either time variable was not found or value is less than zero [%g], skipping.", m_totalTime));
			return false;
		}

		m_initialized = true;
	}

	//-- Accumulate the delta time.
	m_accumulatedTime += deltaTime;

	//-- This action continues as long as it has not accumulated to total wait time.
	return m_accumulatedTime < m_totalTime;
} //lint !e1764 // script could be const ref // The virtual interface cannot change.

// ======================================================================
