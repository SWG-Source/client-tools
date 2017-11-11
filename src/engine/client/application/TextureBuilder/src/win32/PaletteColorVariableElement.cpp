// ======================================================================
//
// PaletteColorVariableElement.cpp
// copyright 2001-2003 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "PaletteColorVariableElement.h"

#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"

#include <string>

// ======================================================================

PaletteColorVariableElement::PaletteColorVariableElement(char const *variableName, char const *paletteFileName, int paletteIndex):
	IntVariableElement(variableName),
	m_paletteFileName(paletteFileName, true),
	m_palette(0),
	m_value(paletteIndex)
{
	m_palette = PaletteArgbList::fetch(m_paletteFileName);
}

// ----------------------------------------------------------------------

PaletteColorVariableElement::~PaletteColorVariableElement()
{
	if (m_palette)
	{
		m_palette->release();
		m_palette = 0;
	}
}

// ----------------------------------------------------------------------

bool PaletteColorVariableElement::isBounded() const
{
	return true;
}

// ----------------------------------------------------------------------

int PaletteColorVariableElement::getLowerBoundInclusive() const
{
	return 0;
}

// ----------------------------------------------------------------------

int PaletteColorVariableElement::getUpperBoundInclusive() const
{
	if (!m_palette)
	{
		DEBUG_WARNING(true, ("PaletteColorVariableElement::getUpperBoundInclusive(): variable [%s] can't access palette [%s], upper bound set to 0.", getVariableName().c_str(), m_paletteFileName.getString()));
		return 0;
	}
	else
		return m_palette->getEntryCount() - 1;
}

// ----------------------------------------------------------------------

int PaletteColorVariableElement::getValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void PaletteColorVariableElement::setValue(int value)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(getLowerBoundInclusive(), value, getUpperBoundInclusive());
	m_value = value;
}

// ----------------------------------------------------------------------

void PaletteColorVariableElement::saveToCustomizationData(CustomizationData &customizationData) const
{
	//-- Only handle persisting variable value if we've got a valid variable name.
	std::string const variableName = getVariableName();
	if (variableName.length() < 1)
	{
		DEBUG_REPORT_LOG(true, ("PaletteColorVariableElement::saveToCustomizationData(): variable is unnamed, will not persist value.\n"));
		return;
	}

	if (!m_palette)
	{
		DEBUG_WARNING(true, ("PaletteColorVariableElement::saveToCustomizationData(): variable [%s] exists but points to a bad palette, can't save.", variableName.c_str()));
		return;
	}

	PaletteColorCustomizationVariable *variable;

	//-- Find or add customization variable.
	CustomizationVariable *baseVariable = customizationData.findVariable(variableName);
	if (baseVariable)
	{
		// Make sure variable is of right type.
		variable = dynamic_cast<PaletteColorCustomizationVariable *>(baseVariable);
		if (!variable)
		{
			DEBUG_WARNING(true, ("PaletteColorVariableElement::saveToCustomizationData(): variable [%s] exists but is not a palette color customization variable, can't set value.", variableName.c_str()));
			return;
		}
	}
	else
	{
		// Create the variable.
		variable = new PaletteColorCustomizationVariable(m_palette, m_value);
		customizationData.addVariableTakeOwnership(variableName, variable);
	}

	//-- Set the value.
	NOT_NULL(variable);
	variable->setValue(m_value);
}

// ----------------------------------------------------------------------

void PaletteColorVariableElement::loadFromCustomizationData(CustomizationData const &customizationData)
{
	//-- Only handle depersisting if we've got a non-zero-length name.
	std::string const variableName = getVariableName();
	if (variableName.length() < 1)
	{
		DEBUG_REPORT_LOG(true, ("PaletteColorVariableElement::saveToCustomizationData(): variable is unnamed, will not depersist value.\n"));
		return;
	}

	//-- Find or add customization variable.
	PaletteColorCustomizationVariable const *const variable = dynamic_cast<PaletteColorCustomizationVariable const*>(customizationData.findConstVariable(variableName));
	if (!variable)
	{
		DEBUG_WARNING(true, ("PaletteColorVariableElement::loadFromVariableSet(): variable [%s] does not exist in customization data, can't load.", variableName.c_str()));
#ifdef _DEBUG
		customizationData.debugDump();
#endif
		return;
	}

	NOT_NULL(variable);
	m_value = variable->getValue();
}

// ======================================================================
