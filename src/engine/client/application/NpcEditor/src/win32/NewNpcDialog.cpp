// ============================================================================
//
// NewNpcDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "NewNpcDialog.h"
#include "NewNpcDialog.moc"
#include "QtUtility.h"

#include "sharedUtility/DataTable.h"

#include <qcombobox.h>

// ============================================================================
//
// NewNpcDialog
//
// ============================================================================

NewNpcDialog::NewNpcDialog(QWidget *myParent, char const *windowName)
 : BaseNewNpcDialog(myParent, windowName)
 , m_npcDataTable(0)
{
}

// ----------------------------------------------------------------------------

NewNpcDialog::~NewNpcDialog()
{
	m_npcDataTable=0;
}

// ----------------------------------------------------------------------------

void NewNpcDialog::setupControls(DataTable * npcDataTable)
{
	//-- data table driven controls
	QtUtility::fillComboBoxFromDataTable(npcDataTable, "Name", m_comboBoxSAT);

	setNpcDataTable(npcDataTable);
}

// ----------------------------------------------------------------------------

const char * NewNpcDialog::getSelectedSAT() const
{
	return m_comboBoxSAT->currentText();
}

// ----------------------------------------------------------------------------

int NewNpcDialog::getSelectedSATRow() const
{
	return m_comboBoxSAT->currentItem();
}

// ============================================================================
