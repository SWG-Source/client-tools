//======================================================================
//
// LocImportWidget.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_LocImportWidget_H
#define INCLUDED_LocImportWidget_H

#include "ui_ImportWidget.h"

class QDialog;

//======================================================================

class LocImportWidget : public ImportWidget
{
	Q_OBJECT;

public:
	static int            showDialog (QWidget * theParent, int fileIndex);
	
		                  LocImportWidget (QWidget * theParent, int fileIndex);
	                     ~LocImportWidget ();

public slots:

	void                  doFileBrowse ();
	void                  ok ();
	void                  cancel ();

private:
	                      LocImportWidget ();
	                      LocImportWidget (const LocImportWidget & rhs);
	LocImportWidget &     operator=    (const LocImportWidget & rhs);

private:

	QDialog *             m_dialog;
	int                   m_fileIndex;

	static std::string    ms_lastFilename;

};
//======================================================================

#endif
