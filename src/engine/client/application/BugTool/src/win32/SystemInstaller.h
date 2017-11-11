// ======================================================================
//
// SystemInstaller.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SystemInstaller_H
#define INCLUDED_SystemInstaller_H

// ======================================================================

class SystemInstaller
{
public:

	static void preMainWindowInstall(HINSTANCE appInstanceHandle, const char *appCommandLine);
	static void postMainWindowInstall();
	static void remove();

private:

	// disabled
	SystemInstaller();

};

// ======================================================================

#endif
