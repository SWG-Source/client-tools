// ======================================================================
//
// BasicRangedIntVariableElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_BasicRangedIntVariableElement_h
#define INCLUDED_BasicRangedIntVariableElement_h

// ======================================================================

#include "IntVariableElement.h"

// ======================================================================

class BasicRangedIntVariableElement: public IntVariableElement
{
public:

	BasicRangedIntVariableElement(char const *variableName, int value, int lowerBoundInclusive, int upperBoundInclusive);
	virtual ~BasicRangedIntVariableElement();

	virtual void          saveToCustomizationData(CustomizationData &customizationData) const;
	virtual void          loadFromCustomizationData(CustomizationData const &customizationData);

	virtual bool          isBounded() const;
	virtual int           getLowerBoundInclusive() const;
	virtual int           getUpperBoundInclusive() const;

	virtual int           getValue() const;
	virtual void          setValue(int value);

private:

	int          m_value;

	int          m_lowerBoundInclusive;
	int          m_upperBoundInclusive;

private:
	// disabled
	BasicRangedIntVariableElement();
	BasicRangedIntVariableElement(const BasicRangedIntVariableElement&);
	BasicRangedIntVariableElement &operator =(const BasicRangedIntVariableElement&);
};

// ======================================================================

#endif
