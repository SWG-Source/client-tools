// ============================================================================
//
// WeightedListDialogEdits.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "WeightedListDialogEdit.h"

#include "TableItemBase.h"
#include "WeightedListDialog.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// WeightedListDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
WeightedListDialogEdit::WeightedListDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
{
	m_inputDialog = new WeightedListDialog(this, "WeightedListDialog", parentRow);

	setInputDialog(*m_inputDialog);
}

//-----------------------------------------------------------------------------
QString WeightedListDialogEdit::getTextFromDialog() const
{
	QString result;

	// Delta +/-
	
	if (getTableItemBase().getTemplateTableRow().isDeltaPositive())
	{
		result += "(+)";
	}
	else if (getTableItemBase().getTemplateTableRow().isDeltaNegative())
	{
		result += "(-)";
	}

	// Begin the list

	result += "{ ";

	// Add the list's contents, one row at a time

	int const rowCount = getWeightedListDialog()->getRowCount();

	for (int row = 0; row < rowCount; ++row)
	{
		TemplateTableRow *templateTableRow = getWeightedListDialog()->getTemplateTableRow(row);
		TableItemBase *valueTableItemBase = templateTableRow->getColumnValue();

		// Add current row's contents

		result += valueTableItemBase->text();
		
		if (row != (rowCount - 1))
		{
			result += ", ";
		}
	}

	// End the list

	result += " }";

	// Delta percent
	
	if (getTableItemBase().getTemplateTableRow().isDeltaPercent())
	{
		result += "%";
	}

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

//-----------------------------------------------------------------------------
WeightedListDialog *WeightedListDialogEdit::getWeightedListDialog() const
{
	WeightedListDialog *weightedListDialog = dynamic_cast<WeightedListDialog *>(m_inputDialog);
	NOT_NULL(weightedListDialog);

	return weightedListDialog;
}

//-----------------------------------------------------------------------------
void WeightedListDialogEdit::createRows(int const rowCount, int const parameterIndex)
{
	getWeightedListDialog()->createRows(rowCount, parameterIndex);
}

// ============================================================================
