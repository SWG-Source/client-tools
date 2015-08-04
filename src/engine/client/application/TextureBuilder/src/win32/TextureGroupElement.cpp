// ======================================================================
//
// TextureGroupElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureGroupElement.h"

#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

TextureGroupElement::TextureGroupElement()
:	Element()
{
}

// ----------------------------------------------------------------------

TextureGroupElement::~TextureGroupElement()
{
}

// ----------------------------------------------------------------------

std::string TextureGroupElement::getLabel() const
{
	return "Source Textures";
}

// ----------------------------------------------------------------------

unsigned int TextureGroupElement::getTypeIndex() const
{
	return ETI_textureGroup;
}

// ======================================================================
