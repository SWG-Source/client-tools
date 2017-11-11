// ============================================================================
//
// ListDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ListDialog_H
#define INCLUDED_ListDialog_H

#include "DynamicListDialog.h"

class TemplateTableRow;

//-----------------------------------------------------------------------------
class ListDialog : public DynamicListDialog
{
public:

	ListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow);

private:

	// Disabled

	ListDialog();
	ListDialog(ListDialog const &);
	ListDialog &operator=(ListDialog const &);
};

// ============================================================================

#endif // INCLUDED_ListDialog_H