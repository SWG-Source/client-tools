// ============================================================================
//
// DiceRollDialog.h
//
// Dialog that lets user select a dice roll amount (e.g. 4d6 + 20).
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_DiceRollDialog_H
#define INCLUDED_DiceRollDialog_H

#include "BaseDiceRollDialog.h"

//-----------------------------------------------------------------------------
class DiceRollDialog : public BaseDiceRollDialog
{

public:

	DiceRollDialog(int minValue, int maxValue, QWidget *parent = 0, const char *name = 0);

	void                       setDiceData(int numDice, int numSides, int base);
	void                       getDiceData(int &numDice, int &numSides, int &base) const;

	virtual bool               eventFilter(QObject *watched, QEvent *event);

private:

	bool rangeValidated() const;

	int m_minLimit;
	int m_maxLimit;
};

// ============================================================================

#endif // INCLUDED_DiceRollDialog_H
