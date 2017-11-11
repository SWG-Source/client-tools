// ============================================================================
//
// DiceRollDialogEdit.h
//
// Table Item editor which brings up a dice roll dialog to edit its value.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_DiceRollDialogEdit_H
#define INCLUDED_DiceRollDialogEdit_H

#include "BaseDialogEdit.h"

class DiceRollDialog;
class TableItemBase;

//-----------------------------------------------------------------------------
class DiceRollDialogEdit : public BaseDialogEdit
{
public:

	DiceRollDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit);
	virtual ~DiceRollDialogEdit();

	void setDiceData(int numDice, int numSides, int base);

protected:

	virtual QString getTextFromDialog() const;

private:

	DiceRollDialog *m_diceRollDialog;

private:

	// Disabled

	DiceRollDialogEdit &operator =(DiceRollDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_DiceRollDialogEdit_H
