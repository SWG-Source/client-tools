// ============================================================================
//
// ListDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ListDialogEdit.h"
#include "TableItemBase.h"
#include "ListDialog.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// ListDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
ListDialogEdit::ListDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new ListDialog(this, "ListDialog", parentTableItemBase.getTemplateTableRow());

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
QString ListDialogEdit::getTextFromDialog() const
{
	QString resultText;

	// Is this an appending list?

	if (getTableItemBase().getTemplateTableRow().isDeltaPositive())
	{
		resultText += "+";
	}

	// Begin the list

	resultText += "[ ";

	// Add the list's contents, one row at a time

	int rowCount = getListDialog()->getRowCount();

	for (int row = 0; row < rowCount; ++row)
	{
		TemplateTableRow const *templateTableRow = getListDialog()->getTemplateTableRow(row);
		TableItemBase *tableItemBaseValue = templateTableRow->getColumnValue();

		// Add current row's contents

		resultText += tableItemBaseValue->text();
		
		if (row != (rowCount - 1))
		{
			resultText += ", ";
		}
	}

	// End the list

	resultText += " ]";

	return resultText;
}

//-----------------------------------------------------------------------------
ListDialog *ListDialogEdit::getListDialog() const
{
	ListDialog *listDialog = dynamic_cast<ListDialog *>(m_inputDialog);
	NOT_NULL(listDialog);

	return listDialog;
}

// ============================================================================
