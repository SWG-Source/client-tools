// ======================================================================
//
// DestinationTextureDimensionElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DESTINATION_TEXTURE_DIMENSION_ELEMENT_H
#define DESTINATION_TEXTURE_DIMENSION_ELEMENT_H

// ======================================================================

#include "IntSliderElement.h"

class DestinationTextureElement;

// ======================================================================

class DestinationTextureDimensionElement: public IntSliderElement
{
public:

	enum DimensionType
	{
		DT_width,
		DT_height
	};

public:

	DestinationTextureDimensionElement(DestinationTextureElement &destinationTextureElement, DimensionType dimensionType);

	virtual void          setSliderValue(int sliderValue);
	virtual int           getSliderValue() const;
	virtual int           convertSliderToDisplayValue(int sliderValue) const;
	virtual std::string   getDialogValueLabel() const;

	virtual unsigned int  getTypeIndex() const;

private:

	DestinationTextureElement &m_destinationTextureElement;
	DimensionType              m_dimensionType;

private:
	// disabled
	DestinationTextureDimensionElement();
	DestinationTextureDimensionElement(const DestinationTextureDimensionElement&);
	DestinationTextureDimensionElement &operator =(const DestinationTextureDimensionElement&);
};

// ======================================================================

#endif
