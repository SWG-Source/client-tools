// ============================================================================
//
// FloatRangeDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "FloatRangeDialog.h"
#include "FloatRangeDialog.moc"
#include <limits>

// ============================================================================
//
// FloatRangeDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
FloatRangeDialog::FloatRangeDialog(QWidget *parent, char const *name, float limitMin, float limitMax)
 : BaseFloatRangeDialog(parent, name)
 , m_finalValueMin(0.0f)
 , m_finalValueMax(0.0f)
 , m_finalLimitMin(false)
 , m_finalLimitMax(false)
 , m_limitMin(limitMin)
 , m_limitMax(limitMax)
{
	// Initialize float line edits

	QDoubleValidator *validator = new QDoubleValidator(this);
	m_minValueLineEdit->setValidator(validator);
	m_maxValueLineEdit->setValidator(validator);

	// Connections

	connect(m_acceptButton, SIGNAL(clicked()), this, SLOT(slotAcceptButtonClicked()));
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
	connect(m_minLimitCheckBox, SIGNAL(clicked()), this, SLOT(slotMinLimitCheckBoxClicked()));
	connect(m_maxLimitCheckBox, SIGNAL(clicked()), this, SLOT(slotMaxLimitCheckBoxClicked()));
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::setValue(float const valueMin, float const valueMax)
{
	if (valueMin > valueMax)
	{
		// Swap the values

		m_minValueLineEdit->setText(QString::number(valueMax));
		m_maxValueLineEdit->setText(QString::number(valueMin));
	}
	else
	{
		m_minValueLineEdit->setText(QString::number(valueMin));
		m_maxValueLineEdit->setText(QString::number(valueMax));
	}

	// Enable the min/max limit check boxes if needed

	float const floatMin = -std::numeric_limits<float>::max();

	if (valueMin == floatMin)
	{
		m_minLimitCheckBox->setChecked(true);
	}

	float const floatMax = std::numeric_limits<float>::max();

	if (valueMax == floatMax)
	{
		m_maxLimitCheckBox->setChecked(true);
	}
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::getValue(float &valueMin, float &valueMax) const
{
	valueMin = getMin();
	valueMax = getMax();
}

//-----------------------------------------------------------------------------
float FloatRangeDialog::getMin() const
{
	float min;

	if (m_minLimitCheckBox->isChecked())
	{
		min = std::numeric_limits<float>::min();
	}
	else
	{
		min = m_minValueLineEdit->text().toFloat();
	}

	return min;
}

//-----------------------------------------------------------------------------
float FloatRangeDialog::getMax() const
{
	float max;

	if (m_maxLimitCheckBox->isChecked())
	{
		max = std::numeric_limits<float>::max();
	}
	else
	{
		max = m_maxValueLineEdit->text().toFloat();
	}

	return max;
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::slotAcceptButtonClicked()
{
	if (getMin() > getMax())
	{
		QString text;
		text.sprintf("The min value (%f) must be <= the max value (%f).", getMin(), getMax());
		QMessageBox::warning(this, "Invalid Float Range", text, "Ok");
	}
	else if (!m_minLimitCheckBox->isChecked() && (getMin() < m_limitMin))
	{
		QString text;
		text.sprintf("The min value (%f) must be >= the min limit (%f).", getMin(), m_limitMin);
		QMessageBox::warning(this, "Invalid Float Range", text, "Ok");
	}
	else if (!m_maxLimitCheckBox->isChecked() && (getMax() > m_limitMax))
	{
		QString text;
		text.sprintf("The max value (%f) must be <= the max limit (%f).", getMax(), m_limitMax);
		QMessageBox::warning(this, "Invalid Float Range", text, "Ok");
	}
	else
	{
		accept();
	}
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::show()
{
	// Get the current state of the values

	m_finalValueMin = getMin();
	m_finalValueMax = getMax();
	m_finalLimitMin = m_minLimitCheckBox->isChecked();
	m_finalLimitMax = m_maxLimitCheckBox->isChecked();

	BaseFloatRangeDialog::show();
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::slotCancelButtonClicked()
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
void FloatRangeDialog::slotMinLimitCheckBoxClicked()
{
	if (m_minLimitCheckBox->isChecked())
	{
		m_minValueLineEdit->setEnabled(false);
	}
	else
	{
		m_minValueLineEdit->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
void FloatRangeDialog::slotMaxLimitCheckBoxClicked()
{
	if (m_maxLimitCheckBox->isChecked())
	{
		m_maxValueLineEdit->setEnabled(false);
	}
	else
	{
		m_maxValueLineEdit->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
bool FloatRangeDialog::isLimitMin()
{
	return m_minLimitCheckBox->isChecked();
}

//-----------------------------------------------------------------------------
bool FloatRangeDialog::isLimitMax()
{
	return m_maxLimitCheckBox->isChecked();
}

// ============================================================================
