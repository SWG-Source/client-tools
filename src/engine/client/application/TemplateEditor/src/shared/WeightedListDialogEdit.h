// ============================================================================
//
// WeightedListDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_WeightedListDialogEdit_H
#define INCLUDED_WeightedListDialogEdit_H

#include "BaseDialogEdit.h"

class TemplateTableRow;
class WeightedListDialog;

//-----------------------------------------------------------------------------
class WeightedListDialogEdit : public BaseDialogEdit
{
public:

	WeightedListDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow);

	void createRows(int const rowCount, int const parameterIndex);

protected:

	virtual QString getTextFromDialog() const;

private:

	WeightedListDialog *getWeightedListDialog() const;

private:

	// Disabled

	WeightedListDialogEdit &operator =(WeightedListDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_WeightedListDialogEdit_H
