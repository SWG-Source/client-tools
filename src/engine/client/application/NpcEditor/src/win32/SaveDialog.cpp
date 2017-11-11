// ============================================================================
//
// SaveDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "SaveDialog.h"
#include "SaveDialog.moc"
#include "QtUtility.h"

#include "sharedFoundation/ConfigFile.h"

#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qfiledialog.h>

// ============================================================================
//
// SaveDialog
//
// ============================================================================

namespace SaveDialogNamespace
{
	char const * c_npcEditor = "NpcEditor";

	char const * c_defaultServerDsrc = "defaultServerDsrc";
	char const * c_defaultSharedDsrc = "defaultSharedDsrc";
	char const * c_defaultClientDsrc = "defaultClientDsrc";

	char const * c_defaultClientRoot = "defaultClientRoot";
	char const * c_defaultServerRoot = "defaultServerRoot";
	char const * c_defaultSharedRoot = "defaultSharedRoot";
}

using namespace SaveDialogNamespace;

// ----------------------------------------------------------------------------

SaveDialog::SaveDialog(QWidget *myParent, bool openFileMode, char const *windowName)
 : BaseSaveDialog(myParent, windowName)
{
	if(openFileMode)
	{
		m_lineEditServerFileName->setEnabled(false);
		m_lineEditServerFileName->setText("");
		m_lineEditSharedFileName->setEnabled(false);
		m_lineEditSharedFileName->setText("");
		m_pushButtonServerBrowseFileName->setEnabled(false);
		m_pushButtonSharedBrowseFileName->setEnabled(false);
	}

	IGNORE_RETURN(connect(m_pushButtonOK, SIGNAL(released()), this, SLOT(slotOkReleased())));
}

// ----------------------------------------------------------------------------

SaveDialog::~SaveDialog()
{
}

// ----------------------------------------------------------------------------

void SaveDialog::slotServerBrowseReleased()
{
	QString defaultDirectory = ConfigFile::getKeyString(c_npcEditor, c_defaultServerDsrc, 0, 0);

	QString selectedFileName(QFileDialog::getSaveFileName(defaultDirectory, "Tpf files (*.tpf)", this, "ServerFileDialog", "Open Server Object Template File"));
	
	if(!selectedFileName.isNull())
	{
		m_lineEditServerFileName->setText(selectedFileName);
	}
}

// ----------------------------------------------------------------------------

void SaveDialog::slotSharedBrowseReleased()
{
	QString defaultDirectory = ConfigFile::getKeyString(c_npcEditor, c_defaultSharedDsrc, 0, 0);

	QString selectedFileName(QFileDialog::getSaveFileName(defaultDirectory, "Tpf files (*.tpf)", this, "SharedFileDialog", "Set Shared Object Template File"));
	
	if(!selectedFileName.isNull())
	{
		m_lineEditSharedFileName->setText(selectedFileName);
	}
}

// ----------------------------------------------------------------------------

void SaveDialog::slotClientDataFileBrowseReleased()
{
	QString defaultDirectory = ConfigFile::getKeyString(c_npcEditor, c_defaultClientDsrc, 0, 0);

	QString selectedFileName(QFileDialog::getSaveFileName(defaultDirectory, "Mif files (*.mif)", this, "ClientDataFileDialog", "Set Client Data File"));
	
	if(!selectedFileName.isNull())
	{
		m_lineEditClientDataFile->setText(selectedFileName);
	}
}

// ----------------------------------------------------------------------------

void SaveDialog::slotOkReleased()
{
	QString overwriteFileNames;

	//-- if any of the files exist make sure the user wants to overwrite them
	if(m_lineEditServerFileName->isEnabled() && QFile::exists(m_lineEditServerFileName->text()))
		overwriteFileNames += m_lineEditServerFileName->text() + "\n";

	if(m_lineEditSharedFileName->isEnabled() && QFile::exists(m_lineEditSharedFileName->text()))
		overwriteFileNames += m_lineEditSharedFileName->text() + "\n";

	if(m_lineEditClientDataFile->isEnabled() && QFile::exists(m_lineEditClientDataFile->text()))
		overwriteFileNames += m_lineEditClientDataFile->text() + "\n";

	if(!overwriteFileNames.isEmpty())
	{
		int ret = QMessageBox::warning(this, "File Overwrite", "These files exist:\n" + overwriteFileNames + "Are you sure?",
			QMessageBox::Ok, QMessageBox::Cancel);

		if (ret == QMessageBox::Ok)
			accept();
	}
	else
		accept();
}

// ----------------------------------------------------------------------------

char const * SaveDialog::getClientDataFileName(bool relative) const
{
	if(relative)
	{
		QString selectedFileName = m_lineEditClientDataFile->text();

		QtUtility::makeFileNameRelative(&selectedFileName, c_npcEditor, c_defaultClientRoot);
	}

	return m_lineEditClientDataFile->text();

}

// ----------------------------------------------------------------------------

char const * SaveDialog::getServerFileName(bool relative) const
{
	if(relative)
	{
		QString selectedFileName = m_lineEditServerFileName->text();

		QtUtility::makeFileNameRelative(&selectedFileName, c_npcEditor, c_defaultServerRoot);
	}
	
	return m_lineEditServerFileName->text();
}

// ----------------------------------------------------------------------------

char const * SaveDialog::getSharedFileName(bool relative) const
{
	if(relative)
	{
		QString selectedFileName = m_lineEditSharedFileName->text();

		QtUtility::makeFileNameRelative(&selectedFileName, c_npcEditor, c_defaultSharedRoot);
	}
	
	return m_lineEditSharedFileName->text();
}

// ----------------------------------------------------------------------------

void SaveDialog::makeClientNameRelative(QString * filename)
{
	QtUtility::makeFileNameRelative(filename, c_npcEditor, c_defaultClientRoot);
}

// ----------------------------------------------------------------------------

void SaveDialog::makeServerNameRelative(QString * filename)
{
	QtUtility::makeFileNameRelative(filename, c_npcEditor, c_defaultServerRoot);
}

// ----------------------------------------------------------------------------

void SaveDialog::makeSharedNameRelative(QString * filename)
{
	QtUtility::makeFileNameRelative(filename, c_npcEditor, c_defaultSharedRoot);
}

// ============================================================================
