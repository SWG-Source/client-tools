// ============================================================================
//
// IntegerRangeDialogEdit.h
//
// Table Item editor which brings up a range dialog to edit its value.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_IntegerRangeDialogEdit_H
#define INCLUDED_IntegerRangeDialogEdit_H

#include "BaseDialogEdit.h"

class IntegerRangeDialog;
class TableItemBase;

//-----------------------------------------------------------------------------
class IntegerRangeDialogEdit : public BaseDialogEdit
{
public:

	IntegerRangeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit);

	void setRangeData(int const valueMin, int const valueMax);

protected:

	virtual QString getTextFromDialog() const;

private:

	IntegerRangeDialog *getIntegerRangeDialog() const;

private:

	// Disabled

	IntegerRangeDialogEdit &operator =(IntegerRangeDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_IntegerRangeDialogEdit_H
