// ======================================================================
//
// VariableElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_VariableElement_h
#define INCLUDED_VariableElement_h

// ======================================================================

#include "Element.h"

class CustomizationData;

// ======================================================================
/**
 * Base class for variable elements.
 *
 * Variable elements appear in the variable edit view.
 */

class VariableElement: public Element
{
public:

	virtual std::string const &getVariableName() const = 0;
	std::string                getFullyQualifiedVariableName(char const *defaultDirectoryName) const;

	virtual void               saveToCustomizationData(CustomizationData &customizationData) const = 0;
	virtual void               loadFromCustomizationData(CustomizationData const &cusotmizationData) = 0;

};

// ======================================================================

#endif
