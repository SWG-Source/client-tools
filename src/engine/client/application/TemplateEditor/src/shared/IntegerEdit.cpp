// ============================================================================
//
// IntegerEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "IntegerEdit.h"

#include "TableItemBase.h"

// ============================================================================
//
// IntegerEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
IntegerEdit::IntegerEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
{
	m_integerSpinBox = new QSpinBox(minLimit, maxLimit, 1, this);
	m_integerSpinBox->setValue(0);

	// All DataTypeEdits should call update text immediately as part of initialization

	setFocusTarget(*m_integerSpinBox);
}

//-----------------------------------------------------------------------------
void IntegerEdit::setValue(int value)
{
	m_integerSpinBox->setValue(value);
}

//-----------------------------------------------------------------------------
QString IntegerEdit::getText() const
{
	QString result;

	// Delta +/-
	
	if (!getTableItemBase().getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
	{
		if (getTableItemBase().getTemplateTableRow().isDeltaPositive())
		{
			result += "(+) ";
		}
		else if (getTableItemBase().getTemplateTableRow().isDeltaNegative())
		{
			result += "(-) ";
		}
	}

	// Current value

	result += m_integerSpinBox->text();

	// Delta percent
	
	if (getTableItemBase().getTemplateTableRow().isDeltaPercent())
	{
		result += " %";
	}

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
