// ============================================================================
//
// ArrayDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ArrayDialog_H
#define INCLUDED_ArrayDialog_H

#include "StaticListDialog.h"

class TemplateTableRow;

//-----------------------------------------------------------------------------
class ArrayDialog : public StaticListDialog
{
public:

	ArrayDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow);

private:

	// Disabled

	ArrayDialog();
	ArrayDialog(ArrayDialog const &);
	ArrayDialog &operator=(ArrayDialog const &);
};

// ============================================================================

#endif // INCLUDED_ArrayDialog_H
