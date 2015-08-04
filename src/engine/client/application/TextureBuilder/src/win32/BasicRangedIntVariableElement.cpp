// ======================================================================
//
// BasicRangedIntVariableElement.cpp
// copyright 2001-2003 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "BasicRangedIntVariableElement.h"

#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"

#include <string>

// ======================================================================

BasicRangedIntVariableElement::BasicRangedIntVariableElement(char const *variableName, int value, int lowerBoundInclusive, int upperBoundInclusive) :
	IntVariableElement(variableName),
	m_value(value),
	m_lowerBoundInclusive(lowerBoundInclusive),
	m_upperBoundInclusive(upperBoundInclusive)
{
}

// ----------------------------------------------------------------------

BasicRangedIntVariableElement::~BasicRangedIntVariableElement()
{
}

// ----------------------------------------------------------------------

bool BasicRangedIntVariableElement::isBounded() const
{
	return true;
}

// ----------------------------------------------------------------------

int BasicRangedIntVariableElement::getLowerBoundInclusive() const
{
	return m_lowerBoundInclusive;
}

// ----------------------------------------------------------------------

int BasicRangedIntVariableElement::getUpperBoundInclusive() const
{
	return m_upperBoundInclusive;
}

// ----------------------------------------------------------------------

int BasicRangedIntVariableElement::getValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void BasicRangedIntVariableElement::setValue(int value)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(m_lowerBoundInclusive, value, m_upperBoundInclusive);
	m_value = value;
}

// ----------------------------------------------------------------------

void BasicRangedIntVariableElement::saveToCustomizationData(CustomizationData &customizationData) const
{
	RangedIntCustomizationVariable *variable;

	//-- Find or add customization variable.
	CustomizationVariable *baseVariable = customizationData.findVariable(getVariableName());
	if (baseVariable)
	{
		// Make sure variable is of right type.
		variable = dynamic_cast<RangedIntCustomizationVariable*>(baseVariable);
		if (!variable)
		{
			DEBUG_WARNING(true, ("BasicRangedIntVariableElement::saveToCustomizationData(): variable [%s] exists but is not a ranged-int customization variable, can't set value.", getVariableName().c_str()));
			return;
		}
	}
	else
	{
		// Create the variable.
		variable = new BasicRangedIntCustomizationVariable(m_lowerBoundInclusive, m_value, m_upperBoundInclusive + 1);
		customizationData.addVariableTakeOwnership(getVariableName(), variable);
	}

	//-- Set the value.
	NOT_NULL(variable);
	variable->setValue(m_value);
}

// ----------------------------------------------------------------------

void BasicRangedIntVariableElement::loadFromCustomizationData(CustomizationData const &customizationData)
{
	//-- Find or add customization variable.
	RangedIntCustomizationVariable const *const variable = dynamic_cast<RangedIntCustomizationVariable const*>(customizationData.findConstVariable(getVariableName()));
	if (!variable)
	{
		DEBUG_WARNING(true, ("BasicRangedIntVariableElement::loadFromVariableSet(): variable [%s] does not exist in customization data, can't load.", getVariableName().c_str()));
		return;
	}

	NOT_NULL(variable);
	m_value = variable->getValue();
}

// ======================================================================
