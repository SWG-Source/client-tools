// ======================================================================
//
// TimerPlaybackActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/TimerPlaybackActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/TimerPlaybackAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_TIME = TAG(T,I,M,E);
const Tag TAG_TIMR = TAG(T,I,M,R);

// ======================================================================

bool TimerPlaybackActionTemplate::ms_installed;

// ======================================================================

void TimerPlaybackActionTemplate::install()
{
	InstallTimer const installTimer("TimerPlaybackActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_TIMR, create);

	ms_installed = true;
	ExitChain::add(remove, "TimerPlaybackActionTemplate");
}

// ======================================================================

PlaybackAction *TimerPlaybackActionTemplate::createPlaybackAction() const
{
	return new TimerPlaybackAction(*this);
}

// ======================================================================

void TimerPlaybackActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed."));

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_TIMR);
	ms_installed = false;
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *TimerPlaybackActionTemplate::create(Iff &iff)
{
	return new TimerPlaybackActionTemplate(iff);
}

// ======================================================================

TimerPlaybackActionTemplate::TimerPlaybackActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_timeDurationVariable(TAG_TIME)
{
	DEBUG_FATAL(!ms_installed, ("not installed."));

	iff.enterForm(TAG_TIMR);

	const Tag version = iff.getCurrentName();
	switch (version)
	{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			{
				char buffer[5];

				ConvertTagToString(version, buffer);
				FATAL(true, ("unsupported TimerPlaybackActionTemplate version [%s].", buffer));
			}
	}

	iff.exitForm(TAG_TIMR);
}

// ----------------------------------------------------------------------

void TimerPlaybackActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_timeDurationVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
