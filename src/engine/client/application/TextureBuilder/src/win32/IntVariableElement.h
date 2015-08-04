// ======================================================================
//
// IntVariableElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_IntVariableElement_h
#define INCLUDED_IntVariableElement_h

// ======================================================================

#include <string>
#include "VariableElement.h"

// ======================================================================

class IntVariableElement: public VariableElement
{
public:

	explicit IntVariableElement(char const *variableName);
	virtual ~IntVariableElement();

	virtual std::string        getLabel() const;
	virtual unsigned int       getTypeIndex() const;

	virtual std::string const &getVariableName() const;

	virtual void               saveToCustomizationData(CustomizationData &customizationData) const = 0;
	virtual void               loadFromCustomizationData(CustomizationData const &customizationData) = 0;

	virtual bool               isBounded() const = 0;
	virtual int                getLowerBoundInclusive() const = 0;
	virtual int                getUpperBoundInclusive() const = 0;

	virtual int                getValue() const = 0;
	virtual void               setValue(int value) = 0;

private:

	std::string  m_variableName;

private:
	// disabled
	IntVariableElement();
	IntVariableElement(const IntVariableElement&);
	IntVariableElement &operator =(const IntVariableElement&);
};

// ======================================================================

#endif
