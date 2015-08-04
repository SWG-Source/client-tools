// ======================================================================
//
// CommandGroupElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "CommandGroupElement.h"

#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

CommandGroupElement::CommandGroupElement()
:	Element()
{
}

// ----------------------------------------------------------------------

CommandGroupElement::~CommandGroupElement()
{
}

// ----------------------------------------------------------------------

std::string CommandGroupElement::getLabel() const
{
	return "Commands";
}

// ----------------------------------------------------------------------

unsigned int CommandGroupElement::getTypeIndex() const
{
	return ETI_commandGroup;
}

// ======================================================================
