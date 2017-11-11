// ============================================================================
//
// MustDeriveEdit.h
//
// Class for providing an "editor" which marks this item as @derived
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MustDeriveEdit_H
#define INCLUDED_MustDeriveEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class MustDeriveEdit : public DataTypeEdit
{
public:

	MustDeriveEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);
	virtual ~MustDeriveEdit();

	virtual QString getText() const;

private:

	QLineEdit *m_mustDeriveLineEdit;

private:

	// Disabled

	MustDeriveEdit &operator =(MustDeriveEdit const &);
};
// ============================================================================

#endif // INCLUDED_MustDeriveEdit_H
