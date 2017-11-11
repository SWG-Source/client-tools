// ============================================================================
//
// StructDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StructDialogEdit_H
#define INCLUDED_StructDialogEdit_H

#include "BaseDialogEdit.h"

class EditTableItem;
class StructDialog;
class TemplateTableRow;
class TpfTemplate;

//-----------------------------------------------------------------------------
class StructDialogEdit : public BaseDialogEdit
{
public:

	StructDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow);

	void setValue(TpfTemplate &structTemplate);

protected:

	virtual QString getTextFromDialog() const;

private:

	StructDialog *getStructDialog() const;

private:

	// Disabled

	StructDialogEdit &operator =(StructDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_StructDialogEdit_H
