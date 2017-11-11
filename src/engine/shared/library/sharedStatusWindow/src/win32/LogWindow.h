// ======================================================================
//
// LogWindow.h
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_LogWindow_H
#define INCLUDED_LogWindow_H

// ======================================================================

class Gate;
class StatusWindow;

#include "sharedThread/ThreadHandle.h"

// ======================================================================

class LogWindow
{
public:

	static void install(const char *title);

	static void printNoArgs(const char *message);
	static void printArgList(const char *format, va_list argList);
	static void print(const char *format, ...);

	static void setWindowPlacement(const WINDOWPLACEMENT *placement);
	static void getWindowPlacement(WINDOWPLACEMENT *placement);

	static void closeThreads();

private:

	static void remove(void);
	static void logThreadProc (const char *title);

private:

	static Gate gate;
	static ThreadHandle thread;
	static StatusWindow *statusWindow;

};

// ======================================================================

#endif
