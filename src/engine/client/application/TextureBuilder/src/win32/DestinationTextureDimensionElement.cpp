// ======================================================================
//
// DestinationTextureDimensionElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "DestinationTextureDimensionElement.h"

#include "DestinationTextureElement.h"
#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

namespace
{
	const char *const ms_widthText  = "Texture Width";
	const char *const ms_heightText = "Texture Height";
}

// ======================================================================

DestinationTextureDimensionElement::DestinationTextureDimensionElement(DestinationTextureElement &destinationTextureElement, DimensionType dimensionType)
:	IntSliderElement(dimensionType == DT_width ? ms_widthText : ms_heightText, 0, 9, 1),
	m_destinationTextureElement(destinationTextureElement),
	m_dimensionType(dimensionType)
{
}

// ----------------------------------------------------------------------

void DestinationTextureDimensionElement::setSliderValue(int sliderValue)
{
	DEBUG_FATAL(sliderValue < 0, ("negative slider value %d", sliderValue));

	const int dimension = 0x01 << sliderValue;
	if (m_dimensionType == DT_width)
		m_destinationTextureElement.setWidth(dimension);
	else
		m_destinationTextureElement.setHeight(dimension);
}

// ----------------------------------------------------------------------

int DestinationTextureDimensionElement::getSliderValue() const
{
	const int value = (m_dimensionType == DT_width ? m_destinationTextureElement.getWidth() : m_destinationTextureElement.getHeight());
	DEBUG_FATAL(!IsPowerOfTwo(value), ("dimension is not a power of two [%d]", value));

	return GetFirstBitSet(value);
}

// ----------------------------------------------------------------------

int DestinationTextureDimensionElement::convertSliderToDisplayValue(int sliderValue) const
{
	DEBUG_FATAL(sliderValue < 0, ("negative slider value %d", sliderValue));
	return 0x01 << sliderValue;
}

// ----------------------------------------------------------------------

std::string DestinationTextureDimensionElement::getDialogValueLabel() const
{
	return (m_dimensionType == DT_width ? ms_widthText : ms_heightText);
}

// ----------------------------------------------------------------------

unsigned int DestinationTextureDimensionElement::getTypeIndex() const
{
	return ETI_componentAttribute;
}

// ======================================================================
