// ============================================================================
//
// StringIDDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StringIDDialogEdit.h"
#include "TableItemBase.h"
#include "StringIDDialog.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// StringIDDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
StringIDDialogEdit::StringIDDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new StringIDDialog(this, "StringIDDialog", parentTableItemBase.getTemplateTableRow());

	setInputDialog(*m_inputDialog);
};

//-----------------------------------------------------------------------------
void StringIDDialogEdit::setValue(StringId const &stringId)
{
	getStringIdDialog()->setValue(stringId);

	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString StringIDDialogEdit::getTextFromDialog() const
{
	QString table(getStringIdDialog()->getTableName());
	QString index(getStringIdDialog()->getIndexName());

	QString result;

	// Current value

	result += table;
	result += "  ";
	result += index;

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

//-----------------------------------------------------------------------------
StringIDDialog *StringIDDialogEdit::getStringIdDialog() const
{
	StringIDDialog *stringIdDialog = dynamic_cast<StringIDDialog *>(m_inputDialog);
	NOT_NULL(stringIdDialog);

	return stringIdDialog;
}

// ============================================================================
