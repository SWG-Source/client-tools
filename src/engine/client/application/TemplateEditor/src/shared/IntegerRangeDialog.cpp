// ============================================================================
//
// IntegerRangeDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "IntegerRangeDialog.h"
#include "IntegerRangeDialog.moc"

// ============================================================================
//
// IntegerRangeDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
IntegerRangeDialog::IntegerRangeDialog(QWidget *parent, const char *name, int const limitMin, int const limitMax)
 : BaseIntegerRangeDialog(parent, name)
 , m_finalValueMin(0)
 , m_finalValueMax(0)
 , m_finalLimitMin(false)
 , m_finalLimitMax(false)
{
	// Initialize the spin box min/max limit

	m_minValueSpinBox->setMinValue(limitMin);
	m_minValueSpinBox->setMaxValue(limitMax);

	m_maxValueSpinBox->setMinValue(limitMin);
	m_maxValueSpinBox->setMaxValue(limitMax);

	m_minLimitCheckBox->setChecked(false);
	m_maxLimitCheckBox->setChecked(false);

	// Connections

	connect(m_acceptButton, SIGNAL(clicked()), this, SLOT(slotAcceptButtonClicked()));
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
	connect(m_minLimitCheckBox, SIGNAL(clicked()), this, SLOT(slotMinLimitCheckBoxClicked()));
	connect(m_maxLimitCheckBox, SIGNAL(clicked()), this, SLOT(slotMaxLimitCheckBoxClicked()));
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::setValue(int const valueMin, int const valueMax)
{
	if (valueMin > valueMax)
	{
		// Swap the values

		m_minValueSpinBox->setValue(valueMax);
		m_maxValueSpinBox->setValue(valueMin);
	}
	else
	{
		m_minValueSpinBox->setValue(valueMin);
		m_maxValueSpinBox->setValue(valueMax);
	}

	// Enable the min/max limit check boxes if needed

	if (valueMin == std::numeric_limits<int>::min())
	{
		m_minLimitCheckBox->setChecked(true);
	}

	if (valueMax == std::numeric_limits<int>::max())
	{
		m_maxLimitCheckBox->setChecked(true);
	}
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::getValue(int &valueMin, int &valueMax) const
{
	valueMin = getMin();
	valueMax = getMax();
}

//-----------------------------------------------------------------------------
int IntegerRangeDialog::getMin() const
{
	int min;

	if (m_minLimitCheckBox->isChecked())
	{
		min = m_minValueSpinBox->minValue();
	}
	else
	{
		min = m_minValueSpinBox->value();
	}

	return min;
}

//-----------------------------------------------------------------------------
int IntegerRangeDialog::getMax() const
{
	int max;

	if (m_maxLimitCheckBox->isChecked())
	{
		max = m_maxValueSpinBox->maxValue();
	}
	else
	{
		max = m_maxValueSpinBox->value();
	}

	return max;
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::slotAcceptButtonClicked()
{
	if (getMin() > getMax())
	{
		QString text;
		text.sprintf("The min value (%d) must be <= the max value (%d).", getMin(), getMax());
		QMessageBox::warning(this, "Invalid Integer Range", text, "Ok");
	}
	else if (getMin() < m_minValueSpinBox->minValue())
	{
		QString text;
		text.sprintf("The min value (%d) must be >= the min limit (%d).", getMin(), m_minValueSpinBox->minValue());
		QMessageBox::warning(this, "Invalid Integer Range", text, "Ok");
	}
	else if (getMax() > m_minValueSpinBox->maxValue())
	{
		QString text;
		text.sprintf("The max value (%d) must be <= the max limit (%d).", getMax(), m_minValueSpinBox->maxValue());
		QMessageBox::warning(this, "Invalid Integer Range", text, "Ok");
	}
	else
	{
		accept();
	}
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::show()
{
	// Get the current state of the values

	m_finalValueMin = getMin();
	m_finalValueMax = getMax();
	m_finalLimitMin = m_minLimitCheckBox->isChecked();
	m_finalLimitMax = m_maxLimitCheckBox->isChecked();

	BaseIntegerRangeDialog::show();
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::slotCancelButtonClicked()
{
	// Since the values were rejected, set back the values before the editing
	// began

	setValue(m_finalValueMin, m_finalValueMax);

	m_minLimitCheckBox->setChecked(m_finalLimitMin);
	m_maxLimitCheckBox->setChecked(m_finalLimitMax);

	slotMinLimitCheckBoxClicked();
	slotMaxLimitCheckBoxClicked();

	reject();
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::slotMinLimitCheckBoxClicked()
{
	if (m_minLimitCheckBox->isChecked())
	{
		m_minValueSpinBox->setEnabled(false);
	}
	else
	{
		m_minValueSpinBox->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
void IntegerRangeDialog::slotMaxLimitCheckBoxClicked()
{
	if (m_maxLimitCheckBox->isChecked())
	{
		m_maxValueSpinBox->setEnabled(false);
	}
	else
	{
		m_maxValueSpinBox->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
bool IntegerRangeDialog::isLimitMin()
{
	return m_minLimitCheckBox->isChecked();
}

//-----------------------------------------------------------------------------
bool IntegerRangeDialog::isLimitMax()
{
	return m_maxLimitCheckBox->isChecked();
}

// ============================================================================
