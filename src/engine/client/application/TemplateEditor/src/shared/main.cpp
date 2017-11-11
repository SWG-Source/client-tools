// ============================================================================
// Template Editor Tool
//
// This project's requirements are to provide an interface for creating,
// loading, editing, and saving template (.iff, internal file format) files.
// The work is divided up upon the following components:
//
// - QT UI Files, QT Generated Files: QT components built in QT Designer.
//
// - UI System: Interface components built on top of QT. The UI also knows
//   about the Template System.
//
// - Template loader: A system which is able to read data from .tdf (template
//   defintion file) and .obt (object template) files. The UI then organizes
//   itself around this data. Note that the template system *just* contains
//   data; it has no idea the UI even exists.
//
// - Template compiler: A system that can compile a template into its
//   binary equivalent (i.e., a .obt file)
//
// Copyright Sony Online Entertainment
//
// ============================================================================
#include "FirstTemplateEditor.h"
#include "TemplateEditor.h"

//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	// Create the main Qt applicaiton

	QApplication application(argc, argv);

	// Create the template editor and add it to the application

	TemplateEditor templateEditor(NULL, "TemplateEditor");
	application.setMainWidget(&templateEditor);
	templateEditor.show();

	// Clear all events

	QApplication::flush();

	// Start the application

	return application.exec();
}

// ============================================================================