// ============================================================================
//
// CustomVariableWidget.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "CustomVariableWidget.h"
#include "CustomVariableWidget.moc"
#include "PaletteColorPicker.h"

#include <qspinbox.h>
#include <qlabel.h>
#include <qslider.h>

#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PackedArgb.h"

// ============================================================================
//
// CustomVariableWidget
//
// ============================================================================

namespace CustomVariableWidgetNamespace
{
	char const * c_npcEditor = "NpcEditor";
}

using namespace CustomVariableWidgetNamespace;

// ----------------------------------------------------------------------------

CustomVariableWidget::CustomVariableWidget(QWidget *myparent, char const * variableName, int minValue, int maxValue, int currentValue, PaletteArgb const * colorPalette, char const *windowName)
 : BaseCustomVariableWidget(myparent, windowName)
 , m_colors()
 , m_value(currentValue)
 , m_minValue(minValue)
 , m_maxValue(maxValue)
{
	m_textLabelCustomVariable->setText(variableName);

	m_sliderValue->setMinValue(minValue);
	m_sliderValue->setMaxValue(maxValue);

	m_spinBoxValue->setMinValue(minValue);
	m_spinBoxValue->setMaxValue(maxValue);

	if(!colorPalette)
	{
		m_pushButtonColorPicker->setHidden(true);
	}
	else
	{
		setColors(colorPalette);
		m_pushButtonColorPicker->setHidden(false);
	}

	IGNORE_RETURN(connect(m_sliderValue, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int))));
	IGNORE_RETURN(connect(m_spinBoxValue, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int))));
	IGNORE_RETURN(connect(m_pushButtonColorPicker, SIGNAL(released()), this, SLOT(slotColorButtonReleased())));

	setValue(currentValue);

	DEBUG_REPORT_LOG(false, ("CustomVariableWidget::CustomVariableWidget() - [%s] => (%d, %d)\n", variableName, minValue, maxValue));
}

// ----------------------------------------------------------------------------

CustomVariableWidget::~CustomVariableWidget()
{
}

// ----------------------------------------------------------------------------

void CustomVariableWidget::setValue(int value)
{
	m_value = value;

	m_spinBoxValue->setValue(value);
	m_sliderValue->setValue(value);

	if(!m_colors.empty())
		m_pushButtonColorPicker->setPaletteBackgroundColor(m_colors[static_cast<unsigned int>(value)]);
}

// ----------------------------------------------------------------------------

void CustomVariableWidget::slotValueChanged(int value)
{
	if(value != m_value)
	{
		const char * variable = m_textLabelCustomVariable->text();
		setValue(value);

		emit variableChanged(value, variable);
	}
}

// ----------------------------------------------------------------------------

void CustomVariableWidget::setColors(PaletteArgb const * colorPalette)
{
	NOT_NULL(colorPalette);

	m_colors.clear();

	int count = colorPalette->getEntryCount();

	for(int i=0; i<count; ++i)
	{
		bool error = false;
		PackedArgb const & rgb = colorPalette->getEntry(i, error);
		m_colors.push_back(QColor(rgb.getR(), rgb.getG(), rgb.getB()));
	}
}

// ----------------------------------------------------------------------------

void CustomVariableWidget::slotColorButtonReleased()
{
	PaletteColorPicker * colorPicker = new PaletteColorPicker(this, m_colors.begin(), static_cast<int>(m_colors.size()), m_value);

	int result = colorPicker->exec();

	if(result == static_cast<int>(QDialog::Accepted))
	{
		DEBUG_REPORT_LOG(false, ("CustomVariableWidget::slotColorButtonReleased() - ACCEPTED\n"));
		slotValueChanged(colorPicker->getSelectedColor());
	}
	else
	{
		DEBUG_REPORT_LOG(false, ("CustomVariableWidget::slotColorButtonReleased() - CANCELLED\n"));
	}
} //lint !e429

// ============================================================================
