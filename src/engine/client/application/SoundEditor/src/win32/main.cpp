// ============================================================================
//
// main.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"

#include "SoundEditor.h"

//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	UNREF(argc);
	UNREF(argv);

	QApplication application(argc, argv);
	QRect const screenRect(QApplication::desktop()->screenGeometry());

	SoundEditor soundEditor(NULL, "MainWindow");
	application.setMainWidget(&soundEditor);
	soundEditor.show();

	// Clear all events

	QApplication::flush();

	return application.exec();
}

// ============================================================================