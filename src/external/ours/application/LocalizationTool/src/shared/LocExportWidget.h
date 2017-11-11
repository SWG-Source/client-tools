//======================================================================
//
// LocExportWidget.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_LocExportWidget_H
#define INCLUDED_LocExportWidget_H

#include "ui_ExportWidget.h"

class QDialog;

//======================================================================

class LocExportWidget : public ExportWidget
{
	Q_OBJECT;

public:
	static int            showDialog (QWidget * theParent, int fileIndex);
	
		                  LocExportWidget (QWidget * theParent, int fileIndex);
	                     ~LocExportWidget ();

public slots:

	void                  doFileBrowse ();
	void                  ok ();
	void                  cancel ();

private:
	                      LocExportWidget ();
	                      LocExportWidget (const LocExportWidget & rhs);
	LocExportWidget &     operator=    (const LocExportWidget & rhs);

private:

	QDialog *             m_dialog;
	int                   m_fileIndex;

	static std::string    ms_lastFilename;

};
//======================================================================

#endif
