// ============================================================================
//
// main.cpp
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "MainWindow.h"

#include <qapplication.h>

// ----------------------------------------------------------------------------

int main(int argc, char ** argv)
{
	QApplication application(argc, argv);
	MainWindow mainWindow(NULL, "MainWindow");
	application.setMainWidget(&mainWindow);
	mainWindow.show();

	// Clear all events
	QApplication::flush();

	return application.exec();
}

// ============================================================================
