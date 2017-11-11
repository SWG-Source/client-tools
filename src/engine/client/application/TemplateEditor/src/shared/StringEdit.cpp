// ============================================================================
//
// StringEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StringEdit.h"

// ============================================================================
//
// StringEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
StringEdit::StringEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
{
	m_stringLineEdit = new QLineEdit(this, "QLineEdit");

	setFocusTarget(*m_stringLineEdit);
}

//-----------------------------------------------------------------------------
void StringEdit::setString(QString const &value)
{
	m_stringLineEdit->setText(value);
}

//-----------------------------------------------------------------------------
QString StringEdit::getText() const
{
	QString result;

	// Current value

	result.sprintf("\"%s\"", m_stringLineEdit->text().latin1());

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
