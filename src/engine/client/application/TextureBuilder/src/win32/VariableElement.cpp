// ======================================================================
//
// VariableElement.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstTextureBuilder.h"
#include "VariableElement.h"

#include <string>

// ======================================================================

std::string VariableElement::getFullyQualifiedVariableName(char const *defaultDirectoryName) const
{
	return makeFullyQualifiedVariableName(defaultDirectoryName, getVariableName());
}

// ======================================================================
