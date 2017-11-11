// ============================================================================
//
// DynamicListDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "DynamicListDialog.h"
#include "DynamicListDialog.moc"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// DynamicListDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
DynamicListDialog::DynamicListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : BaseItemListDialog(parent, name)
 , m_parentTemplateTableRow(parentTemplateTableRow)
{
	connect(m_okButton, SIGNAL(clicked()), m_templateTable, SLOT(endEditing()));
	connect(m_okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(m_cancelButton, SIGNAL(clicked()), SLOT(reject()));

	connect(m_addItemAboveButton, SIGNAL(clicked()), SLOT(addNewRowAboveCurrent()));
	connect(m_addItemBelowButton, SIGNAL(clicked()), SLOT(addNewRowBelowCurrent()));
	connect(m_deleteItemButton, SIGNAL(clicked()), SLOT(deleteCurrentRow()));

	parentTemplateTableRow.setChildTable(*m_templateTable);

	m_templateTable->setParameterType(TemplateTable::PT_list);
}

//-----------------------------------------------------------------------------
DynamicListDialog::~DynamicListDialog()
{
	delete m_templateTable;
	m_templateTable = NULL;
}

//-----------------------------------------------------------------------------
int DynamicListDialog::getRowCount() const
{
	return m_templateTable->numRows();
}

//-----------------------------------------------------------------------------
TemplateTableRow *DynamicListDialog::getTemplateTableRow(int const row) const
{
	return m_templateTable->getTemplateTableRow(row);
}

//-----------------------------------------------------------------------------
void DynamicListDialog::addNewRowAboveCurrent()
{
	addNewRow(0);
}

//-----------------------------------------------------------------------------
void DynamicListDialog::addNewRowBelowCurrent()
{
	addNewRow(1);
}

//-----------------------------------------------------------------------------
void DynamicListDialog::deleteCurrentRow()
{
	int removeRowIndex = m_templateTable->currentRow();

	m_templateTable->clearTemplateRow(removeRowIndex);
}

//-----------------------------------------------------------------------------
void DynamicListDialog::addNewRow(int const row)
{
	int insertRowIndex = m_templateTable->currentRow();

	if (m_templateTable->numRows() <= 0)
	{
		insertRowIndex = 0;
	}
	else
	{
		insertRowIndex += row;
	}

	TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, insertRowIndex, getParentTemplateTableRow(), *m_parentTemplateTableRow.getParameter(), row);

	templateTableRow->setChecked(true);
	templateTableRow->setCheckEnabled(false);
}

//-----------------------------------------------------------------------------
void DynamicListDialog::insertNewRow(int const row)
{
	TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, row, getParentTemplateTableRow(), *m_parentTemplateTableRow.getParameter(), row);

	templateTableRow->setChecked(true);
	templateTableRow->setCheckEnabled(false);
}

//-----------------------------------------------------------------------------
TemplateTableRow const &DynamicListDialog::getParentTemplateTableRow()
{
	return m_parentTemplateTableRow;
}

//-----------------------------------------------------------------------------
void DynamicListDialog::createRows(int const rowCount)
{
	for (int index = 0; index < rowCount; ++index)
	{
		insertNewRow(index);
	}
}

// ============================================================================
