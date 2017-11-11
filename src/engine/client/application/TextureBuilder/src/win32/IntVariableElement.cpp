// ======================================================================
//
// IntVariableElement.cpp
// copyright 2001-2003 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "IntVariableElement.h"

#include "ElementTypeIndex.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"

#include <string>

// ======================================================================

IntVariableElement::IntVariableElement(char const *variableName):
	VariableElement(),
	m_variableName(variableName)
{
}

// ----------------------------------------------------------------------

IntVariableElement::~IntVariableElement()
{
}

// ----------------------------------------------------------------------

std::string IntVariableElement::getLabel() const
{
	// convert value to string
	char valueString[32];
	IGNORE_RETURN(_itoa(getValue(), valueString, 10));

	std::string  result(m_variableName);
	result += ": ";
	result += valueString;

	return result;
}

// ----------------------------------------------------------------------

unsigned int IntVariableElement::getTypeIndex() const
{
	return ETI_variableInt;
}

// ----------------------------------------------------------------------

std::string const &IntVariableElement::getVariableName() const
{
	return m_variableName;
}

// ======================================================================
