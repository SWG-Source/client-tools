// ======================================================================
//
// IntSliderElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INT_SLIDER_ELEMENT_H
#define INT_SLIDER_ELEMENT_H

// ======================================================================

#include "Element.h"

// ======================================================================

class IntSliderElement: public Element
{
public:

	IntSliderElement(const std::string &text, int sliderRangeFirst, int sliderRangeLast, int valuesPerTick);
	virtual ~IntSliderElement();

	virtual void          setSliderValue(int sliderValue) = 0;
	virtual int           getSliderValue() const = 0;
	virtual int           convertSliderToDisplayValue(int sliderValue) const = 0;
	virtual std::string   getDialogValueLabel() const = 0;

	virtual std::string   getLabel() const;

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

private:

	std::string *m_text;
	int          m_rangeFirst;
	int          m_rangeLast;
	int          m_valuesPerTick;
	int          m_sliderValue;

private:
	// disabled
	IntSliderElement();
	IntSliderElement(const IntSliderElement&);
	IntSliderElement &operator =(const IntSliderElement&);
};

// ======================================================================

#endif
