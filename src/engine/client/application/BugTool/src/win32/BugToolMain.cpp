// ======================================================================
//
// BugToolMain.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "BugTool/FirstBugTool.h"

#include "BugTool/MainWindow.h"
#include "BugTool/SystemInstaller.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <windows.h>

// ======================================================================

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	UNREF(hPrevInstance);
	UNREF(nShowCmd);
	UNREF(lpCmdLine);
	UNREF(hInstance);

	//system-specific install routines
	SystemInstaller::preMainWindowInstall(hInstance, lpCmdLine);

	//-- create the Qt application object.
	int    argc = 0;
	char **argv = 0;

	QApplication a(argc, argv);

	//-- create the main window.
	//name the window
	MainWindow  mainWindow(0, "SWG Bug Submission Tool");
	//push the command line parameter into the stationId (this is not validated in any way)
	mainWindow.setStationId(lpCmdLine);

	//system-specific install routines
	SystemInstaller::postMainWindowInstall();

	//-- set the app's main window.
	a.setMainWidget(&mainWindow);
	mainWindow.show();

	//-- run the application.
	const int appReturnCode = a.exec();

	//-- shut down installed systems.
	SystemInstaller::remove();

	//-- exit the windows application.
	return (appReturnCode == 0) ? 1 : 0;
} //lint !e818 // lpCmdLine could be pointer to const // yes, but API is immutable.

// ======================================================================
