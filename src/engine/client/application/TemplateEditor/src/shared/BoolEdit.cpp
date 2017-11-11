// ============================================================================
//
// BoolEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "BoolEdit.h"

// ============================================================================
//
// BoolEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
BoolEdit::BoolEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
 , m_trueIndex(0)
 , m_falseIndex(1)
{
	m_booleanComboBox = new QComboBox(this);
	NOT_NULL(m_booleanComboBox);

	m_booleanComboBox->insertItem("True", m_trueIndex);
	m_booleanComboBox->insertItem("False", m_falseIndex);
	m_booleanComboBox->setCurrentItem(m_trueIndex);

	setFocusTarget(*m_booleanComboBox);
}

//-----------------------------------------------------------------------------
BoolEdit::~BoolEdit()
{
}

//-----------------------------------------------------------------------------
void BoolEdit::setValue(bool value)
{
	if(value == true)
	{
		m_booleanComboBox->setCurrentItem(m_trueIndex);
	}
	else
	{
		m_booleanComboBox->setCurrentItem(m_falseIndex);
	}
}

//-----------------------------------------------------------------------------
QString BoolEdit::getText() const
{
	QString result;

	// Current value

	result += m_booleanComboBox->currentText();

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
