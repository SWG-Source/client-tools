// ============================================================================
//
// BoolEdit.h
//
// Class for providing an editor for a boolean value
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_BoolEdit_H
#define INCLUDED_BoolEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class BoolEdit : public DataTypeEdit
{
public:

	BoolEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);
	virtual ~BoolEdit();

	void            setValue(bool value);
	virtual QString getText() const;

private:

	QComboBox *m_booleanComboBox;
	int        m_trueIndex;
	int        m_falseIndex;

private:

	// Disabled

	BoolEdit &operator =(BoolEdit const &);
};
// ============================================================================

#endif // INCLUDED_BoolEdit_H