// ======================================================================
//
// SwitchTextureShader.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SwitchTextureShader.h"

#include "sharedObject/AlterResult.h"
#include "sharedSwitcher/Switcher.h"
#include "sharedSwitcher/SwitcherTemplate.h"

#include <vector>

// ======================================================================

SwitchTextureShader::SwitchTextureShader(const SwitchTextureShaderTemplate &shaderTemplate)
: Shader(shaderTemplate),
	m_switcher(shaderTemplate.m_switcherTemplate->create()),
	m_shaders(shaderTemplate.m_shaders)
{
}

// ----------------------------------------------------------------------

SwitchTextureShader::~SwitchTextureShader()
{
	delete m_switcher;
}

// ----------------------------------------------------------------------

Shader *SwitchTextureShader::convertToModifiableShader() const
{
	return const_cast<SwitchTextureShader*>(this);
}

// ----------------------------------------------------------------------

bool SwitchTextureShader::obeysCustomizationData() const
{
	return false;
}

// ----------------------------------------------------------------------

float SwitchTextureShader::alter(const float time) const
{
	// all the child shaders should be passed the alter call
	m_switcher->alter(time);

	// @todo: figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const StaticShader  &SwitchTextureShader::prepareToView() const
{
	const int state = m_switcher->getState();
	DEBUG_FATAL(state < 0 || state >= static_cast<int>(m_shaders->size()), ("switcher state out of range"));
	return (*m_shaders)[state]->prepareToView();	
}

// ----------------------------------------------------------------------

bool SwitchTextureShader::usesVertexShader() const
{
	const int state = m_switcher->getState();
	DEBUG_FATAL(state < 0 || state >= static_cast<int>(m_shaders->size()), ("switcher state out of range"));
	return (*m_shaders)[state]->usesVertexShader();	
}

// ----------------------------------------------------------------------

const StaticShader *SwitchTextureShader::getStaticShader() const
{
	//-- it makes no sense to return a shader here -- there's more than one.
	//   return null to prevent animating animating shaders
	return 0;
}

// ----------------------------------------------------------------------

StaticShader *SwitchTextureShader::getStaticShader()
{
	//-- it makes no sense to return a shader here -- there's more than one.
	//   return null to prevent animating animating shaders
	return 0;
}

// ======================================================================
