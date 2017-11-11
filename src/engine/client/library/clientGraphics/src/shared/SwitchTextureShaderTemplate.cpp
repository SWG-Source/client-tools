// ======================================================================
//
// SwitchTextureShaderTemplate.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SwitchTextureShaderTemplate.h"

#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SwitchTextureShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedSwitcher/SwitcherTemplate.h"

#include <vector>

// ======================================================================

const Tag TAG_SWTS = TAG(S,W,T,S);

bool SwitchTextureShaderTemplate::ms_installed;

// ======================================================================

void SwitchTextureShaderTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("SwitchTextureShaderTemplate already installed"));

	ShaderTemplateList::registerShaderTemplateType(TAG_SWTS, create);

	ms_installed = true;
	ExitChain::add(remove, "SwitchTextureShaderTemplate");
}

// ======================================================================

const Shader *SwitchTextureShaderTemplate::fetchShader() const
{
	Shader *shader = new SwitchTextureShader(*this);
	shader->fetch();
	return shader;
}

// ----------------------------------------------------------------------

Shader *SwitchTextureShaderTemplate::fetchModifiableShader() const
{
	Shader *shader = new SwitchTextureShader(*this);
	shader->fetch();
	return shader;
}

// ======================================================================

void SwitchTextureShaderTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("SwitchTextureShaderTemplate not installed"));
	ms_installed = false;

	ShaderTemplateList::deregisterShaderTemplateType(TAG_SWTS);
}

// ----------------------------------------------------------------------

ShaderTemplate *SwitchTextureShaderTemplate::create(const CrcString &name, Iff &iff)
{
	return new SwitchTextureShaderTemplate(name, iff);
}

// ======================================================================

SwitchTextureShaderTemplate::SwitchTextureShaderTemplate(const CrcString &name, Iff &iff)
: ShaderTemplate(name),
	m_switcherTemplate(NULL),
	m_shaders(new Shaders)
{
	DEBUG_FATAL(!ms_installed, ("SwitchTextureShaderTemplate not installed"));

	load(iff);
}

// ----------------------------------------------------------------------

SwitchTextureShaderTemplate::~SwitchTextureShaderTemplate()
{
	delete m_switcherTemplate;

	const Shaders::iterator end = m_shaders->end();
	for (Shaders::iterator i = m_shaders->begin(); i != end; ++i)
		(*i)->release();

	delete m_shaders;

	m_shaderTemplate->release();
	m_shaderTemplate = 0;
}

// ----------------------------------------------------------------------

void SwitchTextureShaderTemplate::load(Iff &iff)
{
	iff.enterForm(TAG_SWTS);

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

	iff.exitForm(TAG_SWTS);
}

// ----------------------------------------------------------------------

void SwitchTextureShaderTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		{
			bool error = false;
			m_shaderTemplate = ShaderTemplateList::fetch(iff, error);

			WARNING(error, ("Problem loading shader template for SwitchTextureShaderTemplate %s", getName().getString()));
		}

		m_switcherTemplate = SwitcherTemplate::create(iff);

		const int numberOfChildren = iff.getNumberOfBlocksLeft();
		m_shaders->reserve(static_cast<size_t>(numberOfChildren));
		for (int i = 0; i < numberOfChildren; ++i)
		{
			iff.enterChunk();

				Shader* const shader = m_shaderTemplate->fetchModifiableShader();
				if (shader->getStaticShader())
				{
					const Tag textureTag = iff.read_int32();

					char textureFilename[512];
					iff.read_string(textureFilename, 512);
					const Texture *const texture = TextureList::fetch(textureFilename);

					if (shader->getStaticShader()->hasTexture (textureTag))
						shader->getStaticShader()->setTexture (textureTag, *texture);
					else
					{
						char tagString [5];
						ConvertTagToString (textureTag, tagString);
						DEBUG_WARNING (true, ("SwitchTextureShaderTemplate::load: texture tag %s does not exist in shader template %s", tagString, m_shaderTemplate->getName().getString()));
					}

					texture->release();

					m_shaders->push_back(shader);
				}
				else
					DEBUG_WARNING (true, ("SwitchTextureShaderTemplate::load: [%s] trying to animate animating shader %s", getName().getString(), m_shaderTemplate->getName().getString()));

			iff.exitChunk(true);
		}
	}
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool SwitchTextureShaderTemplate::isOpaqueSolid() const
{
	const Shaders::iterator end = m_shaders->end();
	for (Shaders::iterator i = m_shaders->begin(); i != end; ++i)
		if (!(*i)->getShaderTemplate().isOpaqueSolid())
			return false;

	return true;
}

// ----------------------------------------------------------------------

bool SwitchTextureShaderTemplate::isCollidable() const
{
	const Shaders::iterator end = m_shaders->end();
	for (Shaders::iterator i = m_shaders->begin(); i != end; ++i)
		if (!(*i)->getShaderTemplate().isCollidable())
			return false;

	return true;
}

// ----------------------------------------------------------------------

bool SwitchTextureShaderTemplate::castsShadows() const
{
	const Shaders::iterator end = m_shaders->end();
	for (Shaders::iterator i = m_shaders->begin(); i != end; ++i)
		if (!(*i)->getShaderTemplate().castsShadows())
			return false;

	return true;
}

// ======================================================================
