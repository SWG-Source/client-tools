// ============================================================================
//
// EnumEdit.h
//
// Class for providing an editor for an enumeration value
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_EnumEdit_H
#define INCLUDED_EnumEdit_H

#include "DataTypeEdit.h"
#include "sharedTemplateDefinition/TemplateData.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class EnumEdit : public DataTypeEdit
{
public:

	EnumEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, const TemplateData::EnumList &enumList);
	virtual ~EnumEdit();

	void            setValue(int index);
	virtual QString getText() const;

private:

	QComboBox *m_enumComboBox;

private:

	// Disabled

	EnumEdit &operator =(EnumEdit const &);
};

// ============================================================================

#endif // INCLUDED_EnumEdit_H
