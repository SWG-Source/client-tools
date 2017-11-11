// ============================================================================
//
// FloatRangeDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FloatRangeDialogEdit_H
#define INCLUDED_FloatRangeDialogEdit_H

#include "BaseDialogEdit.h"

class FloatRangeDialog;
class TableItemBase;

//-----------------------------------------------------------------------------
class FloatRangeDialogEdit : public BaseDialogEdit
{
public:
	
	FloatRangeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, float const limitMin, float const limitMax);

	void setRangeData(float const minValue, float const maxValue);

protected:

	virtual QString getTextFromDialog() const;

private:

	FloatRangeDialog *getFloatRangeDialog() const;

private:

	// Disabled

	FloatRangeDialogEdit &operator =(FloatRangeDialogEdit const &);
};
// ============================================================================

#endif // INCLUDED_FloatRangeDialogEdit_H
