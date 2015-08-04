// ============================================================================
//
// main.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "PlanetWatcher.h"

//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	QApplication application(argc, argv);
	QRect const screenRect(QApplication::desktop()->screenGeometry());

	PlanetWatcher serverWatcher(NULL, "MainWindow");
	application.setMainWidget(&serverWatcher);
	serverWatcher.show();

	// Clear all events

	QApplication::flush();

	return application.exec();
}

// ============================================================================
