//======================================================================
//
// LocExportWidget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "StdAfx.h"
#include "LocExportWidget.h"
#include "LocExportWidget.moc"

#include "FileImporterExporter.h"
#include "LocalizationData.h"
#include "LocalizationTool.h"
#include "LocalizedStringTableReaderWriter.h"
#include "PortableRegistry.h"
#include "StringUtils.h"

#include <qdialog.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qradiobutton.h>

#include <ctime>
#include <cassert>

//======================================================================

namespace
{
	const char * s_reg_filename_last_Export = "filename_last_Export";
}

//-----------------------------------------------------------------

std::string LocExportWidget::ms_lastFilename;

//-----------------------------------------------------------------

LocExportWidget::LocExportWidget (QWidget * theParent, int fileIndex) :
ExportWidget (theParent, "ExportWidget"),
m_dialog (dynamic_cast<QDialog *> (theParent)),
m_fileIndex (fileIndex)
{
	// enable

	connect (m_buttonFileBrowse, SIGNAL (clicked ()), this, SLOT (doFileBrowse ()));
	connect (m_buttonOk,         SIGNAL (clicked ()), this, SLOT (ok ()));
	connect (m_buttonCancel,     SIGNAL (clicked ()), this, SLOT (cancel ()));
	
	if (ms_lastFilename.empty ())
	{
		LocalizationTool * const tool = LocalizationTool::getInstance ();
		assert (tool);
		
		PortableRegistry * const reg = tool->getRegistry ();
		assert (reg);

		Unicode::NarrowString path;
		if (reg->getStringValue (s_reg_filename_last_Export, path))
			ms_lastFilename = path;
	}
	
	m_editFilename->setText (ms_lastFilename.c_str ());
}

//----------------------------------------------------------------------

LocExportWidget::~LocExportWidget ()
{
	m_dialog = 0;
}

//-----------------------------------------------------------------

void LocExportWidget::doFileBrowse ()
{
	const QString result = QFileDialog::getSaveFileName (ms_lastFilename.c_str ());

	if (!result.isEmpty ())
	{
		m_editFilename->setText (result);
	}
}

//-----------------------------------------------------------------

void LocExportWidget::ok ()
{
	const QString & fileStr = m_editFilename->text ();

	if (fileStr.isEmpty ())
	{
		QMessageBox::warning (this, "User Error!", "Please specify an input file.");
		return;
	}

	const QString & delimiterStr = m_editEntryDelimiter->text ();

	if (fileStr.isEmpty ())
	{
		QMessageBox::warning (this, "User Error!", "Please specify a delimiter string.");
		return;
	}

	ms_lastFilename = fileStr.latin1 ();

	std::string resultMsg;

	LocalizedStringTableRW table ("exporting table"); 
	if (!LocalizationData::getData ().populateStringTable (table, m_fileIndex))
	{
		QMessageBox::warning (this, "User Error!", "Could not populate string table.");
		return;
	}
	
	const bool retval = m_radioUnicode->isOn ()
		?
		FileImporterExporter::exportUnicode (ms_lastFilename.c_str (), StringUtils::convertQString (delimiterStr), table, resultMsg)
		:
		FileImporterExporter::exportAscii (ms_lastFilename.c_str (), StringUtils::convertQStringNarrow (delimiterStr), table, resultMsg);

	if (!retval)
	{
		char buf [1024];
		_snprintf (buf, 1024, "Could not Export file:\n%s\n", resultMsg.c_str ());
		QMessageBox::warning (this, "File Error!", buf);
		return;
	}
	
	//	LocalizationData::getData ().notifyListeners ();
	
	if (m_dialog)
		m_dialog->hide ();
	else
		hide ();
	
	//-----------------------------------------------------------------
	//-- save filename in registry
	{
		LocalizationTool * const tool = LocalizationTool::getInstance ();
		assert (tool);
		
		PortableRegistry * const reg = tool->getRegistry ();
		assert (reg);
	
		reg->putStringValue (s_reg_filename_last_Export, ms_lastFilename);
	}
}

//-----------------------------------------------------------------

void LocExportWidget::cancel ()
{
	if (m_dialog)
		m_dialog->hide ();
	else
		hide ();
}

//----------------------------------------------------------------------

int LocExportWidget::showDialog (QWidget * theParent, int fileIndex)
{
	QDialog * const dialog    = new QDialog (theParent, "Dialog", true);
	QBoxLayout  * const box   = new QBoxLayout (dialog, QBoxLayout::LeftToRight);
	LocExportWidget * const w = new LocExportWidget (dialog, fileIndex);
	box->addWidget (w);
	return dialog->exec (); //lint -e429
}

//======================================================================


