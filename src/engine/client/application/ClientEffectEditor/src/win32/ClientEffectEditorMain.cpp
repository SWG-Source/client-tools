// ======================================================================
//
// ClientEffectEditorMain.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "ClientEffectEditor/FirstClientEffectEditor.h"

#include "ClientEffectEditor/MainWindow.h"

#pragma warning(push, 3)

#include <qapplication.h>

#pragma warning(pop)

// ======================================================================

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	UNREF(hPrevInstance);
	UNREF(nShowCmd);
	UNREF(lpCmdLine);
	UNREF(hInstance);

	//create the Qt application object.
	int    argc = 0;
	char **argv = 0;

	QApplication a(argc, argv);

	//create the main window.
	MainWindow  mainWindow(0, "ClientEffect Editor");

	//-- set the app's main window.
	a.setMainWidget(&mainWindow);
	mainWindow.show();

	//-- run the application.
	const int appReturnCode = a.exec();

	//-- exit the windows application.
	return (appReturnCode == 0) ? 1 : 0;
} //lint !e818 // lpCmdLine could be pointer to const // yes, but API is immutable.

// ======================================================================
