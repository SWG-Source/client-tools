// ======================================================================
//
// TextureSourceModeElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureSourceModeElement.h"

#include "DrawTextureCommandElement.h"
#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

namespace
{
	const char *const ms_modeName[] =
		{
			"1 texture",      // STM_texture1,
			"2 texture blend" // STM_textureBlend2
		};
	const int         ms_modeNameCount = sizeof(ms_modeName) / sizeof(ms_modeName[0]);
}

// ======================================================================

TextureSourceModeElement::TextureSourceModeElement(DrawTextureCommandElement &drawTextureCommandElement)
:	Element(),
	m_drawTextureCommandElement(drawTextureCommandElement)
{
}

// ----------------------------------------------------------------------

std::string TextureSourceModeElement::getLabel() const
{
	std::string  result("Texture Source Mode: ");
	
	DrawTextureCommandElement::SourceTextureMode stm = m_drawTextureCommandElement.getSourceTextureMode();
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(stm), ms_modeNameCount);

	result += ms_modeName[static_cast<size_t>(stm)];

	return result;
}

// ----------------------------------------------------------------------

unsigned int TextureSourceModeElement::getTypeIndex() const
{
	return ETI_textureSourceMode;
}

// ======================================================================
