// ============================================================================
//
// ObjvarTypeEdit.h
//
// Objvar values can be four different parameter datatypes (int, float, string,
// or a nested objvar list). This edit is a pull-down list which lets the user
// select which type they want their objvar value to be.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ObjvarTypeEdit_H
#define INCLUDED_ObjvarTypeEdit_H

#include "DataTypeEdit.h"

/*
class TableItemBase;

//-----------------------------------------------------------------------------
class ObjvarTypeEdit : public DataTypeEdit
{
public:

	ObjvarTypeEdit(QWidget *parentWidget, const char *name, TableItemBase const &parentTableItemBase);
	virtual ~ObjvarTypeEdit();

	void setValue(int index);

	int  currentItem() const;

private:

	void updateText();

	QComboBox *      m_objvarTypeComboBox;
	static const int ms_integerChoiceIndex;
	static const int ms_floatChoiceIndex;
	static const int ms_stringChoiceIndex;
	static const int ms_objvarListChoiceIndex;

};

*/

// ============================================================================

#endif // INCLUDED_ObjvarTypeEdit_H
