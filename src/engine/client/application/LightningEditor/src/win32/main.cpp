// ============================================================================
//
// main.cpp
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ============================================================================

#include "FirstLightningEditor.h"
#include "MainWindow.h"
#include "sharedDebug/PerformanceTimer.h"

int main(int argc, char ** argv)
{
	PerformanceTimer::install();

	QApplication application(argc, argv);
	MainWindow mainWindow(NULL, "MainWindow");
	application.setMainWidget(&mainWindow);
	mainWindow.show();

	// Clear all events

	QApplication::flush();

	return application.exec();
}

// ============================================================================
