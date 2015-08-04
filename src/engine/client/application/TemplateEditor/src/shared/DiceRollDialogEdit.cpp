// ============================================================================
//
// DiceRollDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "DiceRollDialogEdit.h"

#include "DiceRollDialog.h"


// ============================================================================
//
// DiceRollDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
DiceRollDialogEdit::DiceRollDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
 , m_diceRollDialog(NULL)
{
	m_diceRollDialog = new DiceRollDialog(minLimit, maxLimit, this);

	setInputDialog(*m_diceRollDialog);
}

//-----------------------------------------------------------------------------
DiceRollDialogEdit::~DiceRollDialogEdit()
{
	delete m_diceRollDialog;
}

//-----------------------------------------------------------------------------
void DiceRollDialogEdit::setDiceData(int numDice, int numSides, int base)
{
	m_diceRollDialog->setDiceData(numDice, numSides, base);
	setText(getTextFromDialog());
}

//-----------------------------------------------------------------------------
QString DiceRollDialogEdit::getTextFromDialog() const
{
	QString resultText;

	// Text-ify dice roll info, e.g. 10d2 or 4d6

	resultText = m_diceRollDialog->m_numRollsSpinBox->text() + "d" + m_diceRollDialog->m_numSidesSpinBox->text();
	
	// Text-ify base amount, adding + or - amount to the dice roll (or nothing if base value is 0)

	int baseValue = m_diceRollDialog->m_modifierSpinBox->value();

	if(baseValue != 0)
	{
		if(baseValue < 0)
		{
			resultText += "-";
			baseValue = -baseValue;  // make positive for text display purposes
		}
		else 
		{
			resultText += "+";
		}

		resultText += QString::number(baseValue);
	}

	return resultText;
}

// ============================================================================
