// ============================================================================
//
// FloatRangeDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "FloatRangeDialogEdit.h"

#include "FloatRangeDialog.h"

// ============================================================================
//
// FloatRangeDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
FloatRangeDialogEdit::FloatRangeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, float const limitMin, float const limitMax)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new FloatRangeDialog(this, "FloatRangeDialog", limitMin, limitMax);

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
void FloatRangeDialogEdit::setRangeData(float const minValue, float const maxValue)
{
	getFloatRangeDialog()->setValue(minValue, maxValue);
	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString FloatRangeDialogEdit::getTextFromDialog() const
{
	QString result;
	float valueMin;
	float valueMax;

	getFloatRangeDialog()->getValue(valueMin, valueMax);

	QString valueMinString("");
	QString valueMaxString("");

	if (valueMin != std::numeric_limits<float>::min())
	{
		valueMinString.setNum(valueMin);
	}

	if (valueMax != std::numeric_limits<float>::max())
	{
		valueMaxString.setNum(valueMax);
	}

	result = valueMinString + ".." + valueMaxString;

	return result;
}

//-----------------------------------------------------------------------------
FloatRangeDialog *FloatRangeDialogEdit::getFloatRangeDialog() const
{
	FloatRangeDialog *floatRangeDialog = dynamic_cast<FloatRangeDialog *>(m_inputDialog);
	NOT_NULL(floatRangeDialog);

	return floatRangeDialog;
}

// ============================================================================
