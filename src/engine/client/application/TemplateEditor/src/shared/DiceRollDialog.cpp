// ============================================================================
//
// DiceRollDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"

#include "DiceRollDialog.h"

// ============================================================================
//
// DiceRollDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
DiceRollDialog::DiceRollDialog(int minLimit, int maxLimit, QWidget *parent, const char *name)
 : BaseDiceRollDialog(parent, name),
   m_minLimit(minLimit), m_maxLimit(maxLimit)
{
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	m_okButton->installEventFilter(this);
}

//-----------------------------------------------------------------------------
void DiceRollDialog::setDiceData(int numDice, int numSides, int base)
{
	m_numRollsSpinBox->setValue(numDice);
	m_numSidesSpinBox->setValue(numSides);
	m_modifierSpinBox->setValue(base);
}

//-----------------------------------------------------------------------------
void DiceRollDialog::getDiceData(int &numDice, int &numSides, int &base) const
{
	numDice = m_numRollsSpinBox->value();
	numSides = m_numSidesSpinBox->value();
	base = m_modifierSpinBox->value();
}

//-----------------------------------------------------------------------------
bool DiceRollDialog::eventFilter(QObject *watched, QEvent *event)
{
	bool handled = false;

	if(watched == m_okButton && event->type() == QEvent::MouseButtonRelease)
	{
		if(!rangeValidated())
		{
			QString text;
			
			text = QString("Dice roll range does not fall between %1 and %2!")
			               .arg(m_minLimit)
			               .arg(m_maxLimit);

			QMessageBox::warning(this, "Invalid dice roll range", text, "OK");

			m_okButton->setDown(false); // Make sure the button pops back up

			handled = true;
		}
	}

	if(!handled)
	{
		handled = BaseDiceRollDialog::eventFilter(watched, event);
	}

	return handled;
}

// Make sure the dice roll fits in the specified range
//-----------------------------------------------------------------------------
bool DiceRollDialog::rangeValidated() const
{
	int numDice, numSides, base;
	int minDiceRoll, maxDiceRoll;

	getDiceData(numDice, numSides, base);

	minDiceRoll = numDice + base; // Rolled a 1 every time
	maxDiceRoll = numDice * numSides + base; // Rolled max value every time

	if(minDiceRoll < m_minLimit || maxDiceRoll > m_maxLimit)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// ============================================================================
