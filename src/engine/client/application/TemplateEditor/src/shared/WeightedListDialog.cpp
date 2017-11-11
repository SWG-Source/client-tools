// ============================================================================
//
// WeightedListDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "WeightedListDialog.h"
#include "WeightedListDialog.moc"

#include "TableItemBase.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// WeightedListDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
WeightedListDialog::WeightedListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : DynamicListDialog(parent, name, parentTemplateTableRow)
{
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOkPushButtonClicked()));
}

//-----------------------------------------------------------------------------
//bool WeightedListDialog::eventFilter(QObject *watched, QEvent *event)
//{
//	bool handled = false;
//
//	if (watched == m_okButton && event->type() == QEvent::MouseButtonRelease)
//	{
//		m_templateTable->endEditing(); // Make sure we commit our current table state, so we can make sure its weights are valid
//
//		int const weightTotal = getWeightTotal();
//
//		if (weightTotal != 100)
//		{
//			QString text;
//			text.sprintf("Please make sure the weights of all items add up to 100. Current total is %d", weightTotal);
//			QMessageBox::warning(this, "Weight Error", text, "OK");
//
//			m_okButton->setDown(false); // Make sure the button pops back up
//
//			handled = true;
//		}
//	}
//
//	if (!handled)
//	{
//		handled = DynamicListDialog::eventFilter(watched, event);
//	}
//
//	return handled;
//};

//-----------------------------------------------------------------------------
int WeightedListDialog::getWeightTotal() const
{
	int numRows = m_templateTable->numRows();
	int weightTotal = 0;

	for(int currRow = 0; currRow < numRows; ++currRow)
	{
		TemplateTableRow *rowToCheck = m_templateTable->getTemplateTableRow(currRow);

		weightTotal += rowToCheck->getColumnValue()->getWeight();
	}

	return weightTotal;
}

//-----------------------------------------------------------------------------
void WeightedListDialog::slotOkPushButtonClicked()
{
}

//-----------------------------------------------------------------------------
void WeightedListDialog::insertNewRow(int const row, int const parameterIndex)
{
	TemplateData::Parameter const *parameter = getParentTemplateTableRow().getParameter();

	if (parameter == NULL)
	{
		return;
	}

	NOT_NULL(parameter);

	TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, row, getParentTemplateTableRow(), *parameter, parameterIndex, row);

	templateTableRow->setChecked(true);
	templateTableRow->setCheckEnabled(false);
}

//-----------------------------------------------------------------------------
void WeightedListDialog::createRows(int const rowCount, int const parameterIndex)
{
	for (int index = 0; index < rowCount; ++index)
	{
		insertNewRow(index, parameterIndex);
	}
}

// ============================================================================
