// ======================================================================
//
// SetupClientBugReporting.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientBugReporting/FirstClientBugReporting.h"
#include "clientBugReporting/SetupClientBugReporting.h"

#include "clientBugReporting/ClientBugReporting.h"
#include "clientBugReporting/ConfigClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

void SetupClientBugReporting::install()
{
	InstallTimer const installTimer("SetupClientBugReporting::install");

	ConfigClientBugReporting::install();

	ClientBugReporting::install();
	ToolBugReporting::install();
}

// ======================================================================
