// ======================================================================
//
// main.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "ApplicationWindow.h"
#include "Pixmaps.h"
#include <qapplication.h>

// -----------------------------------------------------------------

int main(int argc, char **argv)
{
	QApplication application( argc, argv );
	Pixmaps::install();
	ApplicationWindow *mainWindow = new ApplicationWindow(application);
	mainWindow->setCaption("P4Qt");
	mainWindow->resize(800, 600);
	application.setMainWidget(mainWindow);
	mainWindow->show();

#if 1
	mainWindow->depots();
	mainWindow->opened();
#endif

	// hook up the "quit" signal/slot
	application.connect(&application, SIGNAL(lastWindowClosed()), &application, SLOT(quit()));
	application.exec();

	return 0;
}

// -----------------------------------------------------------------

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}
