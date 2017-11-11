// ======================================================================
//
// ConfigClientAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/ConfigClientAnimation.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"

//===================================================================

namespace
{
	bool  s_logPriorityHandling;
	bool  s_enableCombatTrumping;
}

//===================================================================

#define KEY_BOOL(a,b)     (s_ ## a = ConfigFile::getKeyBool("ClientAnimation", #a, b))
#define REGISTER_BOOL(a)   DebugFlags::registerFlag(s_ ## a, "ClientAnimation", #a)

// #define KEY_INT(a,b)    (s_ ## a = ConfigFile::getKeyInt("ClientAnimation", #a,  b))
// #define KEY_FLOAT(a,b)  (s_ ## a = ConfigFile::getKeyFloat("ClientAnimation", #a, b))
// #define KEY_STRING(a,b) (s_ ## a = ConfigFile::getKeyString("ClientAnimation", #a, b))

//===================================================================

void ConfigClientAnimation::install(void)
{
	InstallTimer const installTimer("ConfigClientAnimation::install");

	KEY_BOOL      (logPriorityHandling, false);
	REGISTER_BOOL (logPriorityHandling);

	KEY_BOOL      (enableCombatTrumping, false);
	REGISTER_BOOL (enableCombatTrumping);
}

// ----------------------------------------------------------------------

bool ConfigClientAnimation::getLogPriorityHandling()
{
	return s_logPriorityHandling;
}

// ----------------------------------------------------------------------

bool ConfigClientAnimation::getEnableCombatTrumping()
{
	return s_enableCombatTrumping;
}

//===================================================================
