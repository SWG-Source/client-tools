// ============================================================================
//
// FloatEdit.h
//
// Class for providing an editor for a float value
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FloatEdit_H
#define INCLUDED_FloatEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class FloatEdit : public DataTypeEdit
{
Q_OBJECT

public:

	FloatEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, float minLimit, float maxLimit);

	void            setValue(float value);
	virtual QString getText() const;

private slots:

	void validateCurrentValue();

private:

	QLineEdit *m_floatLineEdit;
	float      m_minLimit;
	float      m_maxLimit;

private:

	// Disabled

	FloatEdit &operator =(FloatEdit const &);
};

// ============================================================================

#endif // INCLUDED_FloatEdit_H
