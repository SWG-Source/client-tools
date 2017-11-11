// ======================================================================
//
// SwitchShaderTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SwitchShaderTemplate.h"

#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/SwitchShader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedSwitcher/SwitcherTemplate.h"

#include <vector>

// ======================================================================

const Tag TAG_SWSH = TAG(S,W,S,H);

bool SwitchShaderTemplate::ms_installed;

// ======================================================================

void SwitchShaderTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("SwitchShaderTemplate already installed"));

	ShaderTemplateList::registerShaderTemplateType(TAG_SWSH, create);

	ms_installed = true;
	ExitChain::add(remove, "SwitchShaderTemplate");
}

// ======================================================================

const Shader *SwitchShaderTemplate::fetchShader() const
{
	Shader *shader = new SwitchShader(*this);
	shader->fetch();
	return shader;
}

// ----------------------------------------------------------------------

Shader *SwitchShaderTemplate::fetchModifiableShader() const
{
	Shader *shader = new SwitchShader(*this);
	shader->fetch();
	return shader;
}

// ======================================================================

void SwitchShaderTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("SwitchShaderTemplate not installed"));
	ms_installed = false;

	ShaderTemplateList::deregisterShaderTemplateType(TAG_SWSH);
}

// ----------------------------------------------------------------------

ShaderTemplate *SwitchShaderTemplate::create(const CrcString &name, Iff &iff)
{
	return new SwitchShaderTemplate(name, iff);
}

// ======================================================================

SwitchShaderTemplate::SwitchShaderTemplate(const CrcString &name, Iff &iff)
: ShaderTemplate(name),
	m_switcherTemplate(NULL),
	m_shaderTemplate(new ShaderTemplates)
{
	DEBUG_FATAL(!ms_installed, ("SwitchShaderTemplate not installed"));

	load(iff);
}

// ----------------------------------------------------------------------

SwitchShaderTemplate::~SwitchShaderTemplate()
{
	delete m_switcherTemplate;
	const ShaderTemplates::iterator end = m_shaderTemplate->end();
	for (ShaderTemplates::iterator i = m_shaderTemplate->begin(); i != end; ++i)
		(*i)->release();

	delete m_shaderTemplate;
}

// ----------------------------------------------------------------------

void SwitchShaderTemplate::load(Iff &iff)
{
	iff.enterForm(TAG_SWSH);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown file version %s", buffer));
				}
		}

	iff.exitForm(TAG_SWSH);
}

// ----------------------------------------------------------------------

void SwitchShaderTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		
		m_switcherTemplate = SwitcherTemplate::create(iff);

		const int numberOfChildren = iff.getNumberOfBlocksLeft();
		m_shaderTemplate->reserve(static_cast<size_t>(numberOfChildren));
		for (int i = 0; i < numberOfChildren; ++i)
		{
			m_shaderTemplate->push_back(ShaderTemplateList::fetch(iff));
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool SwitchShaderTemplate::isOpaqueSolid() const
{
	const ShaderTemplates::iterator end = m_shaderTemplate->end();
	for (ShaderTemplates::iterator i = m_shaderTemplate->begin(); i != end; ++i)
		if (!(*i)->isOpaqueSolid())
			return false;

	return true;
}

// ----------------------------------------------------------------------

bool SwitchShaderTemplate::isCollidable() const
{
	const ShaderTemplates::iterator end = m_shaderTemplate->end();
	for (ShaderTemplates::iterator i = m_shaderTemplate->begin(); i != end; ++i)
		if (!(*i)->isCollidable())
			return false;

	return true;
}

// ----------------------------------------------------------------------

bool SwitchShaderTemplate::castsShadows() const
{
	const ShaderTemplates::iterator end = m_shaderTemplate->end();
	for (ShaderTemplates::iterator i = m_shaderTemplate->begin(); i != end; ++i)
		if (!(*i)->castsShadows())
			return false;

	return true;
}

// ======================================================================
