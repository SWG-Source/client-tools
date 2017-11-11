// ============================================================================
//
// FloatEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "FloatEdit.h"
#include "FloatEdit.moc"

#include "TableItemBase.h"

// ============================================================================
//
// FloatEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
FloatEdit::FloatEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, float minLimit, float maxLimit)
 : DataTypeEdit(parentWidget, name, parentTableItemBase),
   m_minLimit(minLimit), m_maxLimit(maxLimit)
{
	m_floatLineEdit = new QLineEdit(this);

	QDoubleValidator *validator = new QDoubleValidator(m_floatLineEdit);
	m_floatLineEdit->setValidator(validator);

	connect(m_floatLineEdit, SIGNAL(textChanged(const QString &)), SLOT(validateCurrentValue()));

	setValue(0.0f);

	setFocusTarget(*m_floatLineEdit);
}

//-----------------------------------------------------------------------------
void FloatEdit::setValue(float value)
{
	m_floatLineEdit->setText(QString::number(value));
}

//-----------------------------------------------------------------------------
// Make sure our text edit's float value is in range (as QLineEdit does not
// provide this functionality for us)
//-----------------------------------------------------------------------------
void FloatEdit::validateCurrentValue()
{
	float currValue = m_floatLineEdit->text().toFloat();;

	// Clamp float data if it is currently invalid
	if(currValue < m_minLimit || currValue > m_maxLimit)
	{
		clamp<float>(m_minLimit, currValue, m_maxLimit);

		m_floatLineEdit->setText(QString::number(currValue));
	}
}

//-----------------------------------------------------------------------------
QString FloatEdit::getText() const
{
	QString result;

	// Delta +/-
	
	if (!getTableItemBase().getTemplateTableRow().isParameterProperty(TemplateTableRow::PP_weightedItem))
	{
		if (getTableItemBase().getTemplateTableRow().isDeltaPositive())
		{
			result += "(+) ";
		}
		else if (getTableItemBase().getTemplateTableRow().isDeltaNegative())
		{
			result += "(-) ";
		}
	}

	// Current value

	result += m_floatLineEdit->text();

	// Delta percent
	
	if (getTableItemBase().getTemplateTableRow().isDeltaPercent())
	{
		result += " %";
	}

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
