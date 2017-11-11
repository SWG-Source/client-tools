// ============================================================================
//
// IntegerRangeDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "IntegerRangeDialogEdit.h"
#include "IntegerRangeDialog.h"

// ============================================================================
//
// IntegerRangeDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
IntegerRangeDialogEdit::IntegerRangeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new IntegerRangeDialog(this, "IntegerRangeDialog", minLimit, maxLimit);

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void IntegerRangeDialogEdit::setRangeData(int valueMin, int valueMax)
{
	getIntegerRangeDialog()->setValue(valueMin, valueMax);
	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString IntegerRangeDialogEdit::getTextFromDialog() const
{
	QString result;
	QString valueMinString("");
	QString valueMaxString("");
	int valueMin;
	int valueMax;

	getIntegerRangeDialog()->getValue(valueMin, valueMax);

	if (!getIntegerRangeDialog()->isLimitMin())
	{
		valueMinString.setNum(valueMin);
	}

	if (!getIntegerRangeDialog()->isLimitMax())
	{
		valueMaxString.setNum(valueMax);
	}

	result = valueMinString + ".." + valueMaxString;

	return result;
}

//-----------------------------------------------------------------------------
IntegerRangeDialog *IntegerRangeDialogEdit::getIntegerRangeDialog() const
{
	IntegerRangeDialog *integeRangeDialog = dynamic_cast<IntegerRangeDialog *>(m_inputDialog);
	NOT_NULL(integeRangeDialog);

	return integeRangeDialog;
}

// ============================================================================
