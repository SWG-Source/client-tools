// ======================================================================
//
// BoolElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "BoolElement.h"

#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

BoolElement::BoolElement(bool &boolVariable, const std::string &variableName, const std::string &onString, const std::string &offString)
:	Element(),
	m_variable(boolVariable),
	m_variableName(new std::string(variableName)),
	m_onString(new std::string(onString)),
	m_offString(new std::string(offString))
{
}

// ----------------------------------------------------------------------

BoolElement::~BoolElement()
{
	delete m_offString;
	delete m_onString;
	delete m_variableName;
}

// ----------------------------------------------------------------------

std::string BoolElement::getLabel() const
{
	std::string  result(*m_variableName);
	result += ": ";

	if (m_variable)
		result += *m_onString;
	else
		result += *m_offString;

	return result;
}

// ----------------------------------------------------------------------

unsigned int BoolElement::getTypeIndex() const
{
	return ETI_bool;
}

// ----------------------------------------------------------------------

bool BoolElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool BoolElement::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	m_variable = !m_variable;
	return true;
}

// ======================================================================
