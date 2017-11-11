// ============================================================================
//
// ListDialogEdit.h
//
// Table Item editor which brings up a item list dialog to edit its value.
//
// See ItemListDialog
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ListDialogEdit_H
#define INCLUDED_ListDialogEdit_H

#include "BaseDialogEdit.h"

class ListDialog;
class TableItemBase;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class ListDialogEdit : public BaseDialogEdit
{
public:

	ListDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

protected:

	virtual QString getTextFromDialog() const;

private:
	
	ListDialog *getListDialog() const;

private:

	// Disabled

	ListDialogEdit &operator =(ListDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_ListDialogEdit_H
