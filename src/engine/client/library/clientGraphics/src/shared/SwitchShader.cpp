// ======================================================================
//
// SwitchShader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SwitchShader.h"

#include "sharedObject/AlterResult.h"
#include "sharedSwitcher/Switcher.h"
#include "sharedSwitcher/SwitcherTemplate.h"

#include <vector>

// ======================================================================

SwitchShader::SwitchShader(const SwitchShaderTemplate &shaderTemplate)
: Shader(shaderTemplate),
	m_switcher(shaderTemplate.m_switcherTemplate->create()),
	m_shader(new Shaders)
{
	m_shader->reserve(shaderTemplate.m_shaderTemplate->size());
	
	const SwitchShaderTemplate::ShaderTemplates::const_iterator end = shaderTemplate.m_shaderTemplate->end();
	for (SwitchShaderTemplate::ShaderTemplates::const_iterator i = shaderTemplate.m_shaderTemplate->begin(); i != end; ++i)
		m_shader->push_back((*i)->fetchShader());
}

// ----------------------------------------------------------------------

SwitchShader::~SwitchShader()
{
	delete m_switcher;

	const Shaders::iterator end = m_shader->end();
	for (Shaders::iterator i = m_shader->begin(); i != end; ++i)
		(*i)->release();

	delete m_shader;
}

// ----------------------------------------------------------------------

Shader *SwitchShader::convertToModifiableShader() const
{
	return const_cast<SwitchShader*>(this);
}

// ----------------------------------------------------------------------

bool SwitchShader::obeysCustomizationData() const
{
	//-- Return true if any one of the shaders obeys customization data.
	Shaders::iterator const endIt = m_shader->end();
	for (Shaders::iterator it = m_shader->begin(); it != endIt; ++it)
	{
		Shader const *const shader = *it;
		if (shader && shader->obeysCustomizationData())
			return true;
	}

	//-- Return false: none of shaders obey customization data.
	return false;
}

// ----------------------------------------------------------------------

void SwitchShader::setCustomizationData(CustomizationData *customizationData)
{
	//-- Return true if any one of the shaders obeys customization data.
	Shaders::iterator const endIt = m_shader->end();
	for (Shaders::iterator it = m_shader->begin(); it != endIt; ++it)
	{
		Shader *const shader = const_cast<Shader*>(*it);
		if (shader)
			shader->setCustomizationData(customizationData);
	}
}

// ----------------------------------------------------------------------

void SwitchShader::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- Return true if any one of the shaders obeys customization data.
	Shaders::iterator const endIt = m_shader->end();
	for (Shaders::iterator it = m_shader->begin(); it != endIt; ++it)
	{
		Shader const *const shader = *it;
		if (shader)
			shader->addCustomizationVariables(customizationData);
	}
}

// ----------------------------------------------------------------------

float SwitchShader::alter(const float time) const
{
	// all the child shaders should be passed the alter call
	m_switcher->alter(time);

	// @todo: figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const StaticShader  &SwitchShader::prepareToView() const
{
	const int state = m_switcher->getState();
	DEBUG_FATAL(state < 0 || state >= static_cast<int>(m_shader->size()), ("switcher state out of range"));
	return (*m_shader)[state]->prepareToView();	
}

// ----------------------------------------------------------------------

bool SwitchShader::usesVertexShader() const
{
	const int state = m_switcher->getState();
	DEBUG_FATAL(state < 0 || state >= static_cast<int>(m_shader->size()), ("switcher state out of range"));
	return (*m_shader)[state]->usesVertexShader();	
}

// ----------------------------------------------------------------------

const StaticShader *SwitchShader::getStaticShader() const
{
	//-- it makes no sense to return a shader here -- there's more than one.
	//   return null to prevent animating animating shaders
	return 0;
}

// ----------------------------------------------------------------------

StaticShader *SwitchShader::getStaticShader()
{
	//-- it makes no sense to return a shader here -- there's more than one.
	//   return null to prevent animating animating shaders
	return 0;
}

// ======================================================================
