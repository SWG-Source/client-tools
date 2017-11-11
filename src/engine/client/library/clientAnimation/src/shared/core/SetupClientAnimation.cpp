// ======================================================================
//
// SetupClientAnimation.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/SetupClientAnimation.h"

#include "sharedDebug/InstallTimer.h"
#include "clientAnimation/ConfigClientAnimation.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/PlaybackScriptTemplateList.h"
#include "clientAnimation/StopActionTemplate.h"
#include "clientAnimation/TimerPlaybackActionTemplate.h"
#include "clientAnimation/WatcherActionTemplate.h"

// ======================================================================

void SetupClientAnimation::install()
{
	InstallTimer const installTimer("SetupClientAnimation::install");

	ConfigClientAnimation::install();

	PlaybackScript::install();
	PlaybackScriptTemplate::install();
	PlaybackScriptTemplateList::install();

	StopActionTemplate::install();
	TimerPlaybackActionTemplate::install();
	WatcherActionTemplate::install();

	PlaybackScriptManager::install();
}

// ======================================================================
