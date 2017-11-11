// ======================================================================
//
// AnimationEditorMain.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"

#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/MainWindow.h"
#include "AnimationEditor/SystemInstaller.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedObject/Object.h"

#pragma warning(push, 3)

#include <qapplication.h>
#include <qpushbutton.h>

#pragma warning(pop)

// ======================================================================

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	UNREF(hPrevInstance);
	UNREF(nShowCmd);

	//-- install all required systems.
	SystemInstaller::preMainWindowInstall(hInstance, lpCmdLine);

	//-- create the Qt application object.
	int    argc = 0;
	char **argv = 0;

	QApplication a(argc, argv);

	//-- create the main window.
	MainWindow  mainWindow(0, "Animation Editor");

	//-- Handle post-main-window application installation.
	//   This is done because, as currently implemented, the GameWidget loaded indirectly via the 
	//   MainWindow performs a whole mess of engine installation.  That needs to be yanked out of
	//   GameWidget.
	SystemInstaller::postMainWindowInstall();

	//-- Tell the main window to open ASH and LAT files associated with the player object (the current target).
	// ... first force target object to rebuild so that .ASH and .LAT data are setup on it.
	Object *const focusObject = AnimationEditorGameWorld::getFocusObject();
	if (focusObject)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(focusObject->getAppearance());
		if (appearance)
		{
			appearance->setUserControlledDetailLevel(true);
				appearance->setDetailLevel(0);
				appearance->rebuildMesh(0);
			appearance->setUserControlledDetailLevel(false);
		}
	}

	mainWindow.openTargetAshFiles();
	mainWindow.openTargetLatFiles();

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
