// ======================================================================
//
// IntSliderElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "IntSliderElement.h"

#include "DialogIntSlider.h"

#include <string>

// ======================================================================

IntSliderElement::IntSliderElement(const std::string &text, int sliderRangeFirst, int sliderRangeLast, int valuesPerTick)
:	Element(),
	m_text(new std::string(text)),
	m_rangeFirst(sliderRangeFirst),
	m_rangeLast(sliderRangeLast),
	m_valuesPerTick(valuesPerTick),
	m_sliderValue(0)
{
}

// ----------------------------------------------------------------------

IntSliderElement::~IntSliderElement()
{
	delete m_text;
}

// ----------------------------------------------------------------------

std::string IntSliderElement::getLabel() const
{
	const int displayValue = convertSliderToDisplayValue(getSliderValue());

	char stringDisplayValue[32];
	IGNORE_RETURN(_itoa(displayValue, stringDisplayValue, 10));

	std::string label = *m_text;
	label += ": ";
	label += stringDisplayValue;

	return label;
}

// ----------------------------------------------------------------------

bool IntSliderElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool IntSliderElement::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	DialogIntSlider dlg(*this);
	std::string     dialogValueLabel;

	//-- set current slider value
	const int sliderValue    = getSliderValue();
	dlg.m_intSliderValue     = sliderValue;
	dlg.m_valueLabel         = getDialogValueLabel().c_str();
	dlg.m_valueLabel        += ":";

	//-- set converted display value (e.g. display texture size = 2^sliderValue)
	char      displayValue[32];
	const int convertedValue = convertSliderToDisplayValue(sliderValue);
	IGNORE_RETURN(_itoa(convertedValue, displayValue, 10));
	dlg.m_convertedValueString = displayValue;

	dlg.setSliderRange(m_rangeFirst, m_rangeLast, m_valuesPerTick);

	if (dlg.DoModal() == IDOK)
	{
		setSliderValue(dlg.m_intSliderValue);

		// value modified
		return true;
	}
	else
	{
		// value not modified
		return false;
	}
}

// ======================================================================
