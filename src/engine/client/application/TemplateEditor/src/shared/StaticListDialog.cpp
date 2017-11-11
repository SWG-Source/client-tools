// ============================================================================
//
// StaticListDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StaticListDialog.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// StaticListDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
StaticListDialog::StaticListDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow)
 : BaseItemListDialog(parent, name)
 , m_templateTableRow(parentRow)
{
	connect(m_okButton, SIGNAL(clicked()), m_templateTable, SLOT(endEditing()));
	connect(m_okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(m_cancelButton, SIGNAL(clicked()), SLOT(reject()));

	// Hide the add/remove list item buttons

	m_addItemAboveButton->hide();
	m_addItemBelowButton->hide();
	m_deleteItemButton->hide();

	parentRow.setChildTable(*m_templateTable);

	m_templateTable->setParameterType(TemplateTable::PT_list);
}

//-----------------------------------------------------------------------------
int StaticListDialog::getTemplateTableRowCount() const
{
	return m_templateTable->numRows();
}

//-----------------------------------------------------------------------------
TemplateTableRow *StaticListDialog::getTemplateTableRow(int const row) const
{
	return m_templateTable->getTemplateTableRow(row);
}

//-----------------------------------------------------------------------------
TemplateTableRow const &StaticListDialog::getTemplateTableRow() const
{
	return m_templateTableRow;
}

//-----------------------------------------------------------------------------
void StaticListDialog::keyReleaseEvent(QKeyEvent *keyEvent)
{
	if (keyEvent->key() == Qt::Key_Enter)
	{
		keyEvent->accept();
	}
}

// ============================================================================
