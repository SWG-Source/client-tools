// ======================================================================
//
// WatcherActionTemplate.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/WatcherActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/WatcherAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================
// local constants
// ======================================================================

const Tag TAG_WATR = TAG(W,A,T,R);

// ======================================================================
// public static member functions
// ======================================================================

void WatcherActionTemplate::install()
{
	InstallTimer const installTimer("WatcherActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("WatcherActionTemplate already installed"));

	PlaybackScriptTemplate::registerActionTemplate(TAG_WATR, create);

#ifdef _DEBUG
	ms_installed = true;
#endif
	ExitChain::add(remove, "WatcherActionTemplate");
}

// ======================================================================
// public member functions
// ======================================================================

PlaybackAction *WatcherActionTemplate::createPlaybackAction() const
{
	return new WatcherAction(*this);
}

// ======================================================================
// private static member functions
// ======================================================================

void WatcherActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("WatcherActionTemplate not installed"));

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_WATR);

#ifdef _DEBUG
	ms_installed = false;
#endif
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *WatcherActionTemplate::create(Iff &iff)
{
	return new WatcherActionTemplate(iff);
}

// ======================================================================
// private member functions
// ======================================================================

WatcherActionTemplate::WatcherActionTemplate(Iff &iff)
:	PlaybackActionTemplate()
{
	iff.enterForm(TAG_WATR);

		const Tag version = iff.getCurrentName();
		if (version == TAG_0000)
		{
			// we're done
			iff.enterForm(TAG_0000);
			iff.exitForm(TAG_0000);
		}
		else
		{
			char name[5];

			ConvertTagToString(version, name);
			DEBUG_FATAL(true, ("unsupported WatcherActionTemplate version [%s]", name));
		}

	iff.exitForm(TAG_WATR);
}

// ======================================================================
// private static member variable definitions
// ======================================================================

#ifdef _DEBUG
bool WatcherActionTemplate::ms_installed;
#endif

// ======================================================================
