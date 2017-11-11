// ============================================================================
//
// IntegerEdit.h
//
// Class for providing an editor for an integer value
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_IntegerEdit_H
#define INCLUDED_IntegerEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class IntegerEdit : public DataTypeEdit
{
public:

	IntegerEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, int const minLimit, int const maxLimit);

	void            setValue(int value);
	virtual QString getText() const;

private:

	QSpinBox *m_integerSpinBox;

private:

	// Disabled

	IntegerEdit &operator =(IntegerEdit const &);
};

// ============================================================================

#endif // INCLUDED_IntegerEdit_H
