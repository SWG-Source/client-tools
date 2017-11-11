// ======================================================================
//
// SystemInstaller.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "BugTool/FirstBugTool.h"
#include "BugTool/SystemInstaller.h"

// ======================================================================

void SystemInstaller::preMainWindowInstall(HINSTANCE appInstanceHandle, const char *appCommandLine)
{
	UNREF(appInstanceHandle);
	UNREF(appCommandLine);
}

// ----------------------------------------------------------------------

void SystemInstaller::postMainWindowInstall()
{
}

// ----------------------------------------------------------------------

void SystemInstaller::remove()
{
}

// ======================================================================
