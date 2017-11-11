// ============================================================================
//
// StringIDDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StringIDDialogEdit_H
#define INCLUDED_StringIDDialogEdit_H

#include "BaseDialogEdit.h"

class TableItemBase;
class StringIDDialog;
class StringId;

//-----------------------------------------------------------------------------
class StringIDDialogEdit : public BaseDialogEdit
{
public:

	StringIDDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	void setValue(StringId const &stringId);

protected:

	virtual QString getTextFromDialog() const;

private:

	StringIDDialog *getStringIdDialog() const;

private:

	// Disabled

	StringIDDialogEdit &operator =(StringIDDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_StringIDDialogEdit_H
