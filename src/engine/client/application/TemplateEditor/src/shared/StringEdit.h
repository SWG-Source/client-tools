// ============================================================================
//
// StringEdit.h
//
// Class for providing an editor for a string value
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StringEdit_H
#define INCLUDED_StringEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class StringEdit : public DataTypeEdit
{
public:

	StringEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	void            setString(QString const &value);
	virtual QString getText() const;

private:

	QLineEdit *m_stringLineEdit;

private:

	// Disabled

	StringEdit &operator =(StringEdit const &);
};

// ============================================================================

#endif // INCLUDED_StringEdit_H
