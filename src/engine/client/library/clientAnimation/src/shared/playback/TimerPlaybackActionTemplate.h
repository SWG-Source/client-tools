// ======================================================================
//
// TimerPlaybackActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TimerPlaybackActionTemplate_H
#define INCLUDED_TimerPlaybackActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class TimerPlaybackActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	Tag                     getTimeDurationVariable() const;	

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	TimerPlaybackActionTemplate(Iff &iff);

	void  load_0000(Iff &iff);

	// Disabled.
	TimerPlaybackActionTemplate();
	TimerPlaybackActionTemplate(const TimerPlaybackActionTemplate&);
	TimerPlaybackActionTemplate &operator =(const TimerPlaybackActionTemplate&);

private:

	static bool ms_installed;

private:

	Tag  m_timeDurationVariable;

};

// ======================================================================

inline Tag TimerPlaybackActionTemplate::getTimeDurationVariable() const
{
	return m_timeDurationVariable;
}

// ======================================================================

#endif
