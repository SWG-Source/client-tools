// ============================================================================
//
// WeightedListDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_WeightedListDialog_H
#define INCLUDED_WeightedListDialog_H

#include "DynamicListDialog.h"

class TemplateTableRow;

//-----------------------------------------------------------------------------
class WeightedListDialog : public DynamicListDialog
{
	Q_OBJECT

public:

	WeightedListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow);

	void createRows(int const rowCount, int const parameterIndex);

protected slots:

	void slotOkPushButtonClicked();

private:

	int getWeightTotal() const;

	void insertNewRow(int const row, int const parameterIndex);

private:

	// Disabled

	WeightedListDialog();
	WeightedListDialog(WeightedListDialog const &);
	WeightedListDialog &operator=(WeightedListDialog const &);
};

// ============================================================================

#endif // INCLUDED_WeightedListDialog_H
