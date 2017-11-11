// ======================================================================
//
// LogWindow.h
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedStatusWindow/FirstSharedStatusWindow.h"
#include "sharedStatusWindow/LogWindow.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedStatusWindow/StatusWindow.h"
#include "sharedSynchronization/Gate.h"
#include "sharedThread/RunThread.h"
#include "sharedThread/Thread.h"

#include <process.h>
#include <cstdarg>
#include <cstdio>
#include <string>

// ======================================================================

// -TRF- figure out better way to disable warning 1924 for WAIT_FAILED macro
//lint -e1924 // C-style cast // WAIT_FAILED seems to be causing this, can't disable it just for that message

// ======================================================================

Gate LogWindow::gate(false);
StatusWindow *LogWindow::statusWindow;
ThreadHandle LogWindow::thread;

// ======================================================================

void LogWindow::logThreadProc (const char * title)
{
	// create status window.  it is the main window, it posts quit message to the thread's message queue when closed
	// -TRF- danger: StatusWindow better not be used anywhere else within this DLL/EXE!
	const bool disableCloseBox = true;
	const bool makeTopMost     = false;
	const bool trackNewLines   = true;
	statusWindow = new StatusWindow(true, title, disableCloseBox, makeTopMost, trackNewLines);

	// signal status window created

	gate.open();

	// do window loop
	MSG  msg;
	BOOL gmResult;

	do
	{
		gmResult = GetMessage(&msg, NULL, 0, 0);
		if (gmResult != -1)
		{
			static_cast<void>(TranslateMessage(&msg));
			static_cast<void>(SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam));
		}
	} while (gmResult > 0);

	delete statusWindow;
	statusWindow = 0;
}

// ----------------------------------------------------------------------
/**
 * install the LogWindow class.
 *
 * LogWindow requires the StatusWindow to be installed.
 *
 * LogWindow does not add its remove routine on the ExitChain.  You
 * must call it manually.
 */

void LogWindow::install (const char *title)
{
	DEBUG_FATAL (thread, ("LogWindow already installed"));

	// start up the logger thread
	//thread = runNamedThread(title, logThreadProc, title);
	thread = ThreadHandle(new FuncPtrThreadOne<const char *>(title, logThreadProc, title));
	FATAL(!thread, ("failed to create LogWindow thread"));

	// wait for it to finish initializing
	gate.wait();

	ExitChain::add(remove, "LogWindow");
}

// ----------------------------------------------------------------------

void LogWindow::closeThreads()
{
	thread->kill();
}

// ----------------------------------------------------------------------

void LogWindow::remove (void)
{
	if(statusWindow)
	{
		const LRESULT smResult = SendMessage (statusWindow->getWindowHandle(), WM_CLOSE, 0, 0);
		UNREF (smResult);
		delete statusWindow;
		statusWindow = 0;
	}
}

// ----------------------------------------------------------------------

void LogWindow::printNoArgs(const char *message)
{
#if 1
	//-- asommers changed to print out logs
	REPORT_LOG(true, (message));
#endif

	// copy the line and send it to the status window
	char *line = DuplicateString(message);

	// strip trailing '\n'
	const unsigned int lastCharIndex = strlen(line) - 1;
	if (line[lastCharIndex] == '\n')
		line[lastCharIndex] = 0;

	// this routine assumes it owns line and will delete it when done
	if(statusWindow)
		statusWindow->sendLineAsynchronous(line);
}

// ----------------------------------------------------------------------

void LogWindow::printArgList (const char *format, va_list argList)
{
	if (!statusWindow)
	{
		REPORT_LOG(true, ("LogWindow::printArgList (): statusWindow is null\n"));
		return;
	}

	DEBUG_FATAL(!format, ("null format arg"));
	DEBUG_FATAL(!argList, ("null argList arg"));

	char    buffer [1024];

	// -TRF- potential buffer overrun
	static_cast<void> (vsprintf (buffer, format, argList));

	printNoArgs(buffer);
}

// ----------------------------------------------------------------------

void LogWindow::print (const char *format, ...)
{
	va_list argList;

	va_start (argList, format);
	printArgList (format, argList);
	va_end (argList);
}

// ----------------------------------------------------------------------

void LogWindow::setWindowPlacement(const WINDOWPLACEMENT *placement)
{
	DEBUG_FATAL(!statusWindow, ("null statusWindow"));
	statusWindow->setWindowPlacement(placement);
}

// ----------------------------------------------------------------------

void LogWindow::getWindowPlacement(WINDOWPLACEMENT *placement)
{
	DEBUG_FATAL(!statusWindow, ("null statusWindow"));
	statusWindow->getWindowPlacement(placement);
}

// ======================================================================
