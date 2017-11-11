// ======================================================================
//
// VariableContainerElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_VariableContainerElement_h
#define INCLUDED_VariableContainerElement_h

// ======================================================================

#include "Element.h"

class CustomizationData;
class VariableDefinitionElement;

// ======================================================================

class VariableContainerElement: public Element
{
protected:

	void                             removeAllVariableDefinitionElements();

	const VariableDefinitionElement *getVariableDefinitionElement(int variableTypeId, int index) const;
	int                              getIntVariableDefinitionValue(int variableTypeId, int index, CustomizationData const &customizationData, int defaultValue = 0) const;

};

// ======================================================================

#endif
