// ======================================================================
//
// TimerPlaybackAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TimerPlaybackAction_H
#define INCLUDED_TimerPlaybackAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class TimerPlaybackActionTemplate;

// ======================================================================

class TimerPlaybackAction: public PlaybackAction
{
public:

	explicit TimerPlaybackAction(const TimerPlaybackActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const TimerPlaybackActionTemplate &getOurTemplate() const;

	// Disabled.
	TimerPlaybackAction();
	TimerPlaybackAction(const TimerPlaybackAction&);
	TimerPlaybackAction &operator =(const TimerPlaybackAction&);

private:

	bool   m_initialized;
	float  m_totalTime;
	float  m_accumulatedTime;

};

// ======================================================================

#endif
