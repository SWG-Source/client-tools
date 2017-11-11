//======================================================================
//
// LocImportWidget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "StdAfx.h"
#include "LocImportWidget.h"
#include "LocImportWidget.moc"

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
#include <qcheckbox.h>

#include <ctime>
#include <cassert>

//======================================================================

namespace
{
	const char * const s_reg_filename_last_import = "filename_last_import";
	const char * const s_reg_unicode_last_import  = "unicode_last_import";
}

//-----------------------------------------------------------------

std::string LocImportWidget::ms_lastFilename;

//-----------------------------------------------------------------

LocImportWidget::LocImportWidget (QWidget * theParent, int fileIndex) :
ImportWidget (theParent, "ImportWidget"),
m_dialog (dynamic_cast<QDialog *> (theParent)),
m_fileIndex (fileIndex)
{
	// enable
	
	connect (m_buttonFileBrowse, SIGNAL (clicked ()), this, SLOT (doFileBrowse ()));
	connect (m_buttonOk,         SIGNAL (clicked ()), this, SLOT (ok ()));
	connect (m_buttonCancel,     SIGNAL (clicked ()), this, SLOT (cancel ()));
	
	LocalizationTool * const tool = LocalizationTool::getInstance ();
	assert (tool);
	PortableRegistry * const reg = tool->getRegistry ();
	assert (reg);
	
	if (ms_lastFilename.empty ())
	{
		Unicode::NarrowString path;
		if (reg->getStringValue (s_reg_filename_last_import, path))
			ms_lastFilename = path;
	}
	
	m_editFilename->setText (ms_lastFilename.c_str ());
	
	int b = 0;
	if (reg->getIntValue (s_reg_unicode_last_import, b))
	{
		m_checkUnicode->setChecked (b != 0);
	}	
}

//----------------------------------------------------------------------

LocImportWidget::~LocImportWidget ()
{
	m_dialog = 0;
}

//-----------------------------------------------------------------

void LocImportWidget::doFileBrowse ()
{
	const QString result = QFileDialog::getOpenFileName (ms_lastFilename.c_str ());

	if (!result.isEmpty ())
	{
		m_editFilename->setText (result);
	}
}

//-----------------------------------------------------------------

void LocImportWidget::ok ()
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

	const bool useUnicode = m_checkUnicode->isChecked ();

	LocalizedStringTableRW * table = 0;

	if (useUnicode)
		table = FileImporterExporter::importUnicode (ms_lastFilename.c_str (), StringUtils::convertQString (delimiterStr), resultMsg);
	else
		table = FileImporterExporter::importAscii (ms_lastFilename.c_str (), StringUtils::convertQStringNarrow (delimiterStr), resultMsg);

	if (!table)
	{
		char buf [1024];
		_snprintf (buf, sizeof (buf), "Could not import file:\n%s\n", resultMsg.c_str ());
		QMessageBox::warning (this, "File Error!", buf);
		return;
	}

	LocalizationTool * const tool = LocalizationTool::getInstance ();
	assert (tool);

	if (!tool->doFileCloseTranslated ())
	{
		delete table;
		return;
	}

	LocalizationData::getData ().clear ();
	bool retval_load = LocalizationData::getData ().loadFromStringTable (*table, 0);
	
	if (m_dialog)
		m_dialog->hide ();
	else
		hide ();
		
	//-----------------------------------------------------------------
	//-- save filename in registry
	{		
		PortableRegistry * const reg = tool->getRegistry ();
		assert (reg);
	
		reg->putStringValue (s_reg_filename_last_import, ms_lastFilename);
		reg->putIntValue (s_reg_unicode_last_import, useUnicode ? 1 : 0);
	}
	
	if (!retval_load)
	{
		delete table;
		QMessageBox::warning (this, "Error", "An error occured while post-processing the data.");
	}
}

//-----------------------------------------------------------------

void LocImportWidget::cancel ()
{
	if (m_dialog)
		m_dialog->hide ();
	else
		hide ();
}

//----------------------------------------------------------------------

int LocImportWidget::showDialog (QWidget * theParent, int fileIndex)
{
	QDialog * const dialog    = new QDialog (theParent, "Dialog", true);
	QBoxLayout  * const box   = new QBoxLayout (dialog, QBoxLayout::LeftToRight);
	LocImportWidget * const w = new LocImportWidget (dialog, fileIndex);
	box->addWidget (w);
	return dialog->exec ();
}

//======================================================================


