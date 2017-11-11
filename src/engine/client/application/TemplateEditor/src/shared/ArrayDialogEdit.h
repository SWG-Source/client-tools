// ============================================================================
//
// ArrayDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ArrayDialogEdit_H
#define INCLUDED_ArrayDialogEdit_H

#include "BaseDialogEdit.h"

class ArrayDialog;
class TableItemBase;

//-----------------------------------------------------------------------------
class ArrayDialogEdit : public BaseDialogEdit
{
public:

	ArrayDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

protected:

	virtual QString getTextFromDialog() const;

private:

	//QString getTextFromDialog(const QDialog &dialog);

	ArrayDialog *m_arrayDialog;

private:

	// Disabled

	ArrayDialogEdit &operator =(ArrayDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_ArrayDialogEdit_H
