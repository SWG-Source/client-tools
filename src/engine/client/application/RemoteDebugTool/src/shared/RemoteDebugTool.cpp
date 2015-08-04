// ======================================================================
//
// RemoteDebugTool.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstRemoteDebugTool.h"
#include "RemoteDebugTool.h"

#include "ApplicationWindow.h"  //our starter object, owns all other Qt pieces

#include <qapplication.h> //so we can initialize Qt

ApplicationWindow *remoteDebugTool::m_mainWindow;

// ----------------------------------------------------------------------

void remoteDebugTool::run()
{
	int argc = 0;
	char **argv = 0;
	QApplication application( argc, argv );
	m_mainWindow = new ApplicationWindow();
	m_mainWindow->setCaption("remoteDebugTool");
	m_mainWindow->show();
	m_mainWindow->resize(640, 480);
	application.setMainWidget(m_mainWindow);
	application.connect(&application, SIGNAL(lastWindowClosed()), &application, SLOT(quit()));
	application.exec();
}

// ----------------------------------------------------------------------
