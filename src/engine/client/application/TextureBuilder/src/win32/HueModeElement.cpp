// ======================================================================
//
// HueModeElement.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstTextureBuilder.h"
#include "HueModeElement.h"

#include "DrawTextureCommandElement.h"
#include "ElementTypeIndex.h"

#include <string>

// ======================================================================

namespace
{
	const char *const ms_modeName[] =
		{
			"none",   // HM_none
			"1 color" // HM_oneColor
		};
	const int         ms_modeNameCount = sizeof(ms_modeName) / sizeof(ms_modeName[0]);
}

// ======================================================================

HueModeElement::HueModeElement(DrawTextureCommandElement &drawTextureCommandElement) :
	Element(),
	m_drawTextureCommandElement(drawTextureCommandElement)
{
}

// ----------------------------------------------------------------------

std::string HueModeElement::getLabel() const
{
	std::string  result("Hue Mode: ");
	
	const DrawTextureCommandElement::HueMode hm = m_drawTextureCommandElement.getHueMode();
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(hm), ms_modeNameCount);

	result += ms_modeName[static_cast<size_t>(hm)];

	return result;
}

// ----------------------------------------------------------------------

unsigned int HueModeElement::getTypeIndex() const
{
	return ETI_hueMode;
}

// ======================================================================
