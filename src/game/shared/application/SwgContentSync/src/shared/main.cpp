// ======================================================================
//
// main.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgContentSync.h"
#include "ApplicationWindow.h"
#include <qapplication.h>

// -----------------------------------------------------------------

int main(int argc, char **argv)
{
	QApplication application( argc, argv );
	ApplicationWindow *mainWindow = new ApplicationWindow(application);
	mainWindow->setCaption("SwgContentSync");
	mainWindow->resize(800, 600);
	application.setMainWidget(mainWindow);
	mainWindow->show();

	// hook up the "quit" signal/slot
	application.connect(&application, SIGNAL(lastWindowClosed()), &application, SLOT(quit()));
	application.exec();

	return 0;
}

// -----------------------------------------------------------------
