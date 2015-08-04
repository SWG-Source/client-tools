// ======================================================================
//
// MarkerAppearanceTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/MarkerAppearanceTemplate.h"

#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/MarkerAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/AppearanceTemplateList.h"

// ======================================================================

namespace MarkerAppearanceTemplateNamespace
{
	const Tag TAG_MKAT = TAG(M,K,A,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_installed;
}

using namespace MarkerAppearanceTemplateNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(MarkerAppearanceTemplate, true, 0, 0, 0);

// ======================================================================

void MarkerAppearanceTemplate::install()
{
	DEBUG_FATAL(s_installed, ("MarkerAppearanceTemplate already installed."));

	installMemoryBlockManager();
	s_installed = true;

	AppearanceTemplateList::assignBinding(TAG_MKAT, create);

	ExitChain::add(remove, "MarkerAppearanceTemplate");
}

// ======================================================================

MarkerAppearanceTemplate::~MarkerAppearanceTemplate()
{
	if (m_shaderTemplate)
	{
		m_shaderTemplate->release();
		m_shaderTemplate = 0;
	}
}

// ----------------------------------------------------------------------

Appearance *MarkerAppearanceTemplate::createAppearance() const
{
	// @todo implement this.
	return new MarkerAppearance(*this);
}

// ----------------------------------------------------------------------

float MarkerAppearanceTemplate::getParentRadiusMultiplier() const
{
	return m_parentRadiusMultiplier;
}

// ----------------------------------------------------------------------

float MarkerAppearanceTemplate::getParentRadiusAddition() const
{
	return m_parentRadiusAddition;
}

// ----------------------------------------------------------------------

const Shader *MarkerAppearanceTemplate::fetchShader() const
{
	if (m_shaderTemplate)
		return m_shaderTemplate->fetchShader();
	else
		return 0;
}

// ----------------------------------------------------------------------

int MarkerAppearanceTemplate::getTextureStartIndex() const
{
	return m_textureStartIndex;
}

// ----------------------------------------------------------------------

int MarkerAppearanceTemplate::getTextureStopIndex() const
{
	return m_textureStopIndex;
}

// ----------------------------------------------------------------------

MarkerAppearanceTemplate::AnimationMode MarkerAppearanceTemplate::getTextureAnimationMode() const
{
	return m_textureAnimationMode;
}

// ----------------------------------------------------------------------

float MarkerAppearanceTemplate::getTexturesPerSecond() const
{
	return m_texturesPerSecond;
}

// ----------------------------------------------------------------------

void MarkerAppearanceTemplate::getUvCoordinates(int textureIndex, float &left, float &top, float &right, float &bottom) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, textureIndex, m_xTextureCount * m_yTextureCount);

	const int yCoordinate = textureIndex / m_xTextureCount;
	const int xCoordinate = textureIndex % m_xTextureCount;

	left   = static_cast<float>(xCoordinate) * m_uTextureInterval;
	top    = static_cast<float>(yCoordinate) * m_vTextureInterval;

	right  = left + m_uTextureInterval;
	bottom = top + m_vTextureInterval;
}

// ======================================================================

void MarkerAppearanceTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("MarkerAppearanceTemplate not installed."));
	s_installed = false;

	AppearanceTemplateList::removeBinding(TAG_MKAT);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

AppearanceTemplate *MarkerAppearanceTemplate::create(const char *name, Iff *iff)
{
	NOT_NULL(iff);
	return new MarkerAppearanceTemplate(name, *iff);
}

// ======================================================================

MarkerAppearanceTemplate::MarkerAppearanceTemplate(const char *name, Iff &iff) :
	AppearanceTemplate(name),
	m_parentRadiusMultiplier(1.1f),
	m_parentRadiusAddition(0.0f),
	m_shaderTemplate(0),
	m_xTextureCount(1),
	m_yTextureCount(1),
	m_uTextureInterval(1.0f),
	m_vTextureInterval(1.0f),
	m_textureStartIndex(0),
	m_textureStopIndex(0),
	m_textureAnimationMode(AM_none),
	m_texturesPerSecond(1.0f)
{
	iff.enterForm(TAG_MKAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;
			
			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("MarkerAppearanceTemplate: attempting to load unsupported version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_MKAT);

	//-- Validate some post conditions.
	DEBUG_FATAL(m_textureStopIndex < m_textureStartIndex, ("MarkerAppearanceTemplate: invalid: m_textureStopIndex [%d] < m_textureStartIndex [%d].", m_textureStopIndex, m_textureStartIndex));
	DEBUG_FATAL(m_textureStartIndex < 0, ("MarkerAppearanceTemplate: invalid: m_textureStartIndex [%d] < 0.", m_textureStartIndex));
}

// ----------------------------------------------------------------------

void MarkerAppearanceTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			//-- Load parent radius-related data.
			m_parentRadiusMultiplier = iff.read_float();
			m_parentRadiusAddition   = iff.read_float();

			//-- Fetch the shader template.
			char name[2 * MAX_PATH];
			iff.read_string(name, sizeof(name) - 1);
			m_shaderTemplate = ShaderTemplateList::fetch(TemporaryCrcString(name, true));

			//-- Retrieve subtexture definition.
			m_xTextureCount = static_cast<int>(iff.read_uint8());
			m_yTextureCount = static_cast<int>(iff.read_uint8());

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(1, m_xTextureCount, 255);
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(1, m_yTextureCount, 255);
			
			m_uTextureInterval = 1.0f / static_cast<float>(m_xTextureCount);
			m_vTextureInterval = 1.0f / static_cast<float>(m_yTextureCount);

			//-- Retrieve subtexture range data.
			m_textureStartIndex = static_cast<int>(iff.read_uint8());
			m_textureStopIndex  = static_cast<int>(iff.read_uint8());

			// Adjust subtexture range, artist specifies in MIF file as 1-based, but we handle them internally as 0-based.
			--m_textureStartIndex;
			--m_textureStopIndex;

			//-- Retrieve texture animation mode.
			const int animationModeValue = static_cast<int>(iff.read_uint8());
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, animationModeValue, static_cast<int>(AM_COUNT));
			m_textureAnimationMode = static_cast<AnimationMode>(animationModeValue);

			//-- Load animation rate.
			m_texturesPerSecond = iff.read_float();

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
