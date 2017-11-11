// ============================================================================
//
// main.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstShipComponentEditor.h"

#include "ShipComponentEditor.h"

//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	UNREF(argc);
	UNREF(argv);

	QApplication application(argc, argv);
	QRect const screenRect(QApplication::desktop()->screenGeometry());

	ShipComponentEditor soundEditor(NULL, "MainWindow");
	application.setMainWidget(&soundEditor);
	soundEditor.show();

	// Clear all events

	QApplication::flush();

	return application.exec();
}


#pragma warning(disable: 4074)
#pragma init_seg(compiler)

namespace MemoryManagerNamespace
{
	void getMemoryManagerParams(int & maxBytes, bool & commit)
	{
		maxBytes = 128 * 1024 * 1024;
		commit = false;
	}
}


// ============================================================================