// ======================================================================
//
// VariableContainerElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "VariableContainerElement.h"

#include "VariableDefinitionElement.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

#include <string>
#include <vector>

// ======================================================================

void VariableContainerElement::removeAllVariableDefinitionElements()
{
	ElementVector                 &children = getChildren();
	const ElementVector::iterator  itEnd    = children.end();
	for (ElementVector::iterator it = children.begin(); it != itEnd; ++it)
	{
		VariableDefinitionElement *const vdElement = dynamic_cast<VariableDefinitionElement*>(*it);
		if (vdElement)
		{
			IGNORE_RETURN(children.erase(it));
			delete vdElement;
		}
	}
}

// ----------------------------------------------------------------------

const VariableDefinitionElement *VariableContainerElement::getVariableDefinitionElement(int variableTypeId, int index) const
{
	//-- find the index-th variable definition element with the specified variable type id
	const ElementVector &children   = getChildren();
	int                  foundCount = 0;

	const ElementVector::const_iterator itEnd = children.end();
	for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
	{
		const VariableDefinitionElement *const variableDefinitionElement = dynamic_cast<const VariableDefinitionElement*>(*it);
		if (variableDefinitionElement && (variableDefinitionElement->getTypeId() == variableTypeId))
		{
			++foundCount;
			if (index == foundCount - 1)
			{
				//-- found it, return it
				return variableDefinitionElement;
			}
		}
	}

	//-- not found
	return 0;
}

// ----------------------------------------------------------------------

int VariableContainerElement::getIntVariableDefinitionValue(int variableTypeId, int index, CustomizationData const &customizationData, int defaultValue) const
{
	const VariableDefinitionElement *const vde = getVariableDefinitionElement(variableTypeId, index);
	FATAL(!vde, ("failed to find variable definition element, typeid=%d, index=%d", variableTypeId, index));

	//-- get variable id for variable element name and int type
	const std::string &variableName = vde->getVariableName();
	if (variableName.length() > 0)
	{
		RangedIntCustomizationVariable const *const variable = dynamic_cast<RangedIntCustomizationVariable const*>(customizationData.findConstVariable(variableName));
		if (variable)
			return variable->getValue();
		else
			return defaultValue;
	}
	else
	{
		//-- no variable name set
		return defaultValue;
	}
}

// ======================================================================
