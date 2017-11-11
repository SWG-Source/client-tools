// ======================================================================
//
// TextureWriteModeElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureWriteModeElement.h"

#include "DrawTextureCommandElement.h"
#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

namespace
{
	const char *const ms_modeName[] =
		{
			"overwrite",   // WTM_overwrite,
			"alpha blend"  // WTM_alphaBlend
		};
	const int         ms_modeNameCount = sizeof(ms_modeName) / sizeof(ms_modeName[0]);
}

// ======================================================================

TextureWriteModeElement::TextureWriteModeElement(DrawTextureCommandElement &drawTextureCommandElement)
:	Element(),
	m_drawTextureCommandElement(drawTextureCommandElement)
{
}

// ----------------------------------------------------------------------

std::string TextureWriteModeElement::getLabel() const
{
	std::string  result("Texture Write Mode: ");
	
	DrawTextureCommandElement::WriteTextureMode wtm = m_drawTextureCommandElement.getWriteTextureMode();
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(wtm), ms_modeNameCount);

	result += ms_modeName[static_cast<size_t>(wtm)];

	return result;
}

// ----------------------------------------------------------------------

unsigned int TextureWriteModeElement::getTypeIndex() const
{
	return ETI_textureWriteMode;
}

// ======================================================================
