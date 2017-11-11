// ============================================================================
//
// ArrayDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ArrayDialogEdit.h"
#include "TableItemBase.h"
#include "ArrayDialog.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// ArrayDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
ArrayDialogEdit::ArrayDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
 , m_arrayDialog(NULL)
{
	m_arrayDialog = new ArrayDialog(this, "ArrayDialog", parentTableItemBase.getTemplateTableRow());

	setInputDialog(*m_arrayDialog);
}

//-----------------------------------------------------------------------------
QString ArrayDialogEdit::getTextFromDialog() const
{
	QString resultText;

	// Begin the list

	resultText = "[ ";

	// Add the list's contents, one row at a time

	int itemCount = m_arrayDialog->getTemplateTableRowCount();

	for (int itemCurrent = 0; itemCurrent < itemCount; ++itemCurrent)
	{
		const TemplateTableRow *currRow = m_arrayDialog->getTemplateTableRow(itemCurrent);
		TableItemBase *data = currRow->getColumnValue();

		// Add current row's contents

		resultText += data->text();
		
		if (itemCurrent != itemCount - 1)
		{
			resultText += ", ";
		}
	}

	// End the list

	resultText += " ]";

	return resultText;
}

// ============================================================================
