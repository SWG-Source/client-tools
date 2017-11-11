// ======================================================================
//
// StaticShaderTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/StaticShaderTemplate.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFile/Iff.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/ShaderEffectList.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <map>

// ======================================================================

const Tag TAG_TAG  = TAG3(T,A,G);
const Tag TAG_TXM  = TAG3(T,X,M);

const Tag TAG_ARVS = TAG(A,R,V,S);
const Tag TAG_DOT3 = TAG(D,O,T,3);
const Tag TAG_ENVM = TAG(E,N,V,M);
const Tag TAG_MATS = TAG(M,A,T,S);
const Tag TAG_NRML = TAG(N,R,M,L);
const Tag TAG_SRVS = TAG(S,R,V,S);
const Tag TAG_SSHT = TAG(S,S,H,T);
const Tag TAG_TCSS = TAG(T,C,S,S);
const Tag TAG_TFNS = TAG(T,F,N,S);
const Tag TAG_TSNS = TAG(T,S,N,S);
const Tag TAG_TXMS = TAG(T,X,M,S);

// ======================================================================

MemoryBlockManager  *StaticShaderTemplate::ms_memoryBlockManager;

// ======================================================================

void StaticShaderTemplate::install(bool preloadStaticVertexShaders)
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("StaticShaderTemplate", true, sizeof(StaticShaderTemplate), 0, 0, 0);
	ShaderTemplateList::registerShaderTemplateType(TAG_SSHT, create);

	if (preloadStaticVertexShaders)
		ShaderTemplateList::preloadVertexColorShaderTemplates ();

	ExitChain::add(remove, "StaticShaderTemplate::remove()");
}

// ----------------------------------------------------------------------

void StaticShaderTemplate::remove()
{
	NOT_NULL(ms_memoryBlockManager);

	ShaderTemplateList::releaseVertexColorShaderTemplates();

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ShaderTemplateList::deregisterShaderTemplateType(TAG_SSHT);
}

// ----------------------------------------------------------------------

void *StaticShaderTemplate::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(StaticShaderTemplate), ("Looks like a decendent class is trying to use our new routine"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  StaticShaderTemplate::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);

	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

ShaderTemplate *StaticShaderTemplate::create(const CrcString &name, Iff &iff)
{
	return new StaticShaderTemplate(name, iff);
}

// ======================================================================
// @todo do not allocate the arrays that are never used

StaticShaderTemplate::StaticShaderTemplate(const CrcString &name, Iff &iff)
: ShaderTemplate(name),
	m_effect(NULL),
	m_materialMap(NULL),
	m_textureDataMap(NULL),
	m_textureCoordinateSetMap(NULL),
	m_textureFactorMap(NULL),
	m_textureScrollMap(NULL),
	m_alphaTestReferenceValueMap(NULL),
	m_stencilReferenceValueMap(NULL),
	m_staticShader(NULL),
	m_addedDot3Placeholder(false)
{
	load(iff);

	// hack to add dot3 texture coordinate set for old art
	if (m_textureCoordinateSetMap && m_textureCoordinateSetMap->find(TAG_NRML) != m_textureCoordinateSetMap->end() && m_textureCoordinateSetMap->find(TAG_DOT3) == m_textureCoordinateSetMap->end())
	{
		m_addedDot3Placeholder = true;
		TextureCoordinateSetMap::value_type entry(TAG_DOT3, 1);
		std::pair<TextureCoordinateSetMap::iterator, bool> result = m_textureCoordinateSetMap->insert(entry);
		DEBUG_FATAL(!result.second, ("insert failed"));
	}
 }

// ----------------------------------------------------------------------

StaticShaderTemplate::~StaticShaderTemplate()
{
	DEBUG_FATAL(m_staticShader, ("StaticShader still exists for this template"));

	if (m_textureDataMap)
	{
		TextureDataMap::iterator end = m_textureDataMap->end(); 
		for (TextureDataMap::iterator i = m_textureDataMap->begin(); i != end; ++i)
			if (i->second.texture)
				i->second.texture->release();
	}

	delete m_materialMap;
	delete m_textureDataMap;
	delete m_textureCoordinateSetMap;
	delete m_textureFactorMap;
	delete m_textureScrollMap;
	delete m_alphaTestReferenceValueMap;
	delete m_stencilReferenceValueMap;

	m_effect->release();
	m_effect = 0;
}

// ----------------------------------------------------------------------

void StaticShaderTemplate::destroyStaticShader(const StaticShader &staticShader) const
{
	DEBUG_FATAL(&staticShader != m_staticShader, ("Incorrect shader being destroyed"));
	UNREF (staticShader);
	m_staticShader = NULL;
}
	
// ----------------------------------------------------------------------

const Shader *StaticShaderTemplate::fetchShader() const
{
	if (!m_staticShader)
		m_staticShader = new StaticShader(*this);

	m_staticShader->fetch();
	return m_staticShader;
}

// ----------------------------------------------------------------------
	
Shader *StaticShaderTemplate::fetchModifiableShader() const
{
	StaticShader *shader = new StaticShader(*this, StaticShader::modifiable);
	shader->fetch();
	return shader;
}

// ----------------------------------------------------------------------

const ShaderEffect &StaticShaderTemplate::getShaderEffect() const
{
	NOT_NULL(m_effect);
	return *m_effect;
}

// ----------------------------------------------------------------------

int StaticShaderTemplate::getShaderImplementationSortKey() const
{
	NOT_NULL(m_effect);
	return m_effect->getShaderImplementationSortKey();
}

// ----------------------------------------------------------------------

bool StaticShaderTemplate::isOpaqueSolid() const
{
	NOT_NULL(m_effect);
	return m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->isOpaqueSolid();
}

// ----------------------------------------------------------------------

bool StaticShaderTemplate::isCollidable() const
{
	NOT_NULL(m_effect);
	return m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->isCollidable();
}

// ----------------------------------------------------------------------

bool StaticShaderTemplate::castsShadows() const
{
	NOT_NULL(m_effect);
	return m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->castsShadows();
}

// ----------------------------------------------------------------------

bool StaticShaderTemplate::containsPrecalculatedVertexLighting() const
{
	NOT_NULL(m_effect);
	return m_effect->containsPrecalculatedVertexLighting();
}

// ----------------------------------------------------------------------

bool StaticShaderTemplate::getTextureCoordinateSetTag(Tag tag, uint8 & index) const
{
	if (m_textureCoordinateSetMap)
	{
		TextureCoordinateSetMap::const_iterator const i = m_textureCoordinateSetMap->find(tag);
		if (i != m_textureCoordinateSetMap->end())
		{
			index = i->second;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

uint8 StaticShaderTemplate::getTextureCoordinateSetUsageMask() const
{
	uint8 result = 0;

	if (m_textureCoordinateSetMap)
	{
		StaticShaderTemplate::TextureCoordinateSetMap::const_iterator const iEnd = m_textureCoordinateSetMap->end();
		for	(	StaticShaderTemplate::TextureCoordinateSetMap::const_iterator i = m_textureCoordinateSetMap->begin(); i != iEnd; ++i)
			result |= (1 << i->second);
	}

	return result;
}


// ----------------------------------------------------------------------

bool StaticShaderTemplate::hasTextureData(Tag tag) const
{
	return m_textureDataMap && m_textureDataMap->find(tag) != m_textureDataMap->end();
}

// ----------------------------------------------------------------------

void StaticShaderTemplate::load(Iff &iff)
{
	iff.enterForm(TAG_SSHT);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
				DEBUG_FATAL(true, ("unknown version number"));
				break;
		}

	iff.exitForm(TAG_SSHT);
}

// ----------------------------------------------------------------------

static void skipOptionalForm(Iff &iff, Tag tag)
{
	if (iff.enterForm(tag, true))
		iff.exitForm(tag, true);
}

// ----------------------------------------------------------------------

void StaticShaderTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		// we really want to load the effect first, but this is old data
		iff.allowNonlinearFunctions();
		skipOptionalForm(iff, TAG_MATS);
		skipOptionalForm(iff, TAG_TXMS);
		skipOptionalForm(iff, TAG_TCSS);
		skipOptionalForm(iff, TAG_TFNS);
		skipOptionalForm(iff, TAG_ARVS);
		skipOptionalForm(iff, TAG_SRVS);

		m_effect = ShaderEffectList::fetch(iff);
		const ShaderImplementation * implementation = m_effect->getActiveShaderImplementation();

		iff.goToTopOfForm();

		if (iff.enterForm(TAG_MATS, true))
		{
			iff.enterForm(TAG_0000);
				while (!iff.atEndOfForm())
				{
					iff.enterChunk(TAG_TAG);
						const Tag tag = iff.read_uint32();
					iff.exitChunk(TAG_TAG);

					Material material(iff);

					if (implementation && implementation->usesMaterial(tag))
					{
						if (!m_materialMap)
							m_materialMap = new MaterialMap;

						MaterialMap::value_type entry(tag, material);
						std::pair<MaterialMap::iterator, bool> result = m_materialMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitForm(TAG_0000);
			iff.exitForm(TAG_MATS);
		}	

		if (iff.enterForm(TAG_TXMS, true))
		{
			while (!iff.atEndOfForm())
				load_texture(iff);

			iff.exitForm(TAG_TXMS);
		}	

		if (iff.enterForm(TAG_TCSS, true))
		{
			int dot3TextureCoordinateSet = -1;
			uint8 lastNonDot3TextureCoordinateSet = 0;
			bool textureCoordinateSetUsed[8];
			Zero(textureCoordinateSetUsed);

			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag   tag = iff.read_uint32();
					const uint8 tcs = iff.read_uint8();
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcs), 8);

					if (implementation && implementation->usesTextureCoordinateSet(tag))
					{
						if (tag != TAG_DOT3)
						{
							textureCoordinateSetUsed[tcs] = true;
							lastNonDot3TextureCoordinateSet = std::max(lastNonDot3TextureCoordinateSet, tcs);
						}
						else
							dot3TextureCoordinateSet = tcs;

						if (!m_textureCoordinateSetMap)
							m_textureCoordinateSetMap = new TextureCoordinateSetMap;

						TextureCoordinateSetMap::value_type entry(tag, tcs);
						std::pair<TextureCoordinateSetMap::iterator, bool> result = m_textureCoordinateSetMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

				if (m_textureCoordinateSetMap && dot3TextureCoordinateSet > 0 && textureCoordinateSetUsed[dot3TextureCoordinateSet])
				{
					DEBUG_WARNING(true, ("Shader %s has fixed up the loaded DOT3 texture coordinate set from %d to %d", getName().getString(), (*m_textureCoordinateSetMap)[TAG_DOT3], lastNonDot3TextureCoordinateSet + 1));
					(*m_textureCoordinateSetMap)[TAG_DOT3] = static_cast<uint8>(lastNonDot3TextureCoordinateSet + 1);
					m_addedDot3Placeholder = true;
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TCSS);
		}	

		if (iff.enterForm(TAG_TFNS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag    tag           = iff.read_uint32();
					const uint32 textureFactor = iff.read_uint32();

					if (implementation && implementation->usesTextureFactor(tag))
					{
						if (!m_textureFactorMap)
							m_textureFactorMap = new TextureFactorMap;

						TextureFactorMap::value_type entry(tag, textureFactor);
						std::pair<TextureFactorMap::iterator, bool> result = m_textureFactorMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TFNS);
		}	

		if (iff.enterForm(TAG_ARVS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag   tag = iff.read_uint32();
					const uint8 arv = iff.read_uint8();

					if (implementation && implementation->usesAlphaReference(tag))
					{
						if (!m_alphaTestReferenceValueMap)
							m_alphaTestReferenceValueMap = new AlphaTestReferenceValueMap;

						AlphaTestReferenceValueMap::value_type entry(tag, arv);
						std::pair<AlphaTestReferenceValueMap::iterator, bool> result = m_alphaTestReferenceValueMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_ARVS);
		}	

		if (iff.enterForm(TAG_SRVS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag    tag = iff.read_uint32();
					const uint32 srv = iff.read_uint32();

					if (implementation && implementation->usesStencilReference(tag))
					{
						if (!m_stencilReferenceValueMap)
							m_stencilReferenceValueMap = new StencilReferenceValueMap;

						StencilReferenceValueMap::value_type entry(tag, srv);
						std::pair<StencilReferenceValueMap::iterator, bool> result = m_stencilReferenceValueMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_SRVS);
		}	

		// ignore the shader effect that is here in the data and just exit the form

	iff.exitForm(TAG_0000, true);
}

// ----------------------------------------------------------------------

void StaticShaderTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		m_effect = ShaderEffectList::fetch(iff);

		const ShaderImplementation * implementation = m_effect->getActiveShaderImplementation();

		if (iff.enterForm(TAG_MATS, true))
		{
			iff.enterForm(TAG_0000);
				while (!iff.atEndOfForm())
				{
					iff.enterChunk(TAG_TAG);
						const Tag tag = iff.read_uint32();
					iff.exitChunk(TAG_TAG);

					Material material(iff);

					if (implementation && implementation->usesMaterial(tag))
					{
						if (!m_materialMap)
							m_materialMap = new MaterialMap;

						MaterialMap::value_type entry(tag, material);
						std::pair<MaterialMap::iterator, bool> result = m_materialMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitForm(TAG_0000);
			iff.exitForm(TAG_MATS);
		}	

		if (iff.enterForm(TAG_TXMS, true))
		{
			while (!iff.atEndOfForm())
				load_texture(iff);

			iff.exitForm(TAG_TXMS);
		}	

		if (iff.enterForm(TAG_TCSS, true))
		{
			int dot3TextureCoordinateSet = -1;
			uint8 lastNonDot3TextureCoordinateSet = 0;
			bool textureCoordinateSetUsed[8];
			Zero(textureCoordinateSetUsed);

			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag   tag = iff.read_uint32();
					const uint8 tcs = iff.read_uint8();
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcs), 8);

					if (implementation && implementation->usesTextureCoordinateSet(tag))
					{
						if (tag != TAG_DOT3)
						{
							textureCoordinateSetUsed[tcs] = true;
							lastNonDot3TextureCoordinateSet = std::max(lastNonDot3TextureCoordinateSet, tcs);
						}
						else
							dot3TextureCoordinateSet = tcs;

						if (!m_textureCoordinateSetMap)
							m_textureCoordinateSetMap = new TextureCoordinateSetMap;

						TextureCoordinateSetMap::value_type entry(tag, tcs);
						std::pair<TextureCoordinateSetMap::iterator, bool> result = m_textureCoordinateSetMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

				if (m_textureCoordinateSetMap && dot3TextureCoordinateSet > 0 && textureCoordinateSetUsed[dot3TextureCoordinateSet])
				{
					DEBUG_WARNING(true, ("Shader %s has fixed up the loaded DOT3 texture coordinate set from %d to %d", getName().getString(), (*m_textureCoordinateSetMap)[TAG_DOT3], lastNonDot3TextureCoordinateSet + 1));
					(*m_textureCoordinateSetMap)[TAG_DOT3] = static_cast<uint8>(lastNonDot3TextureCoordinateSet + 1);
					m_addedDot3Placeholder = true;
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TCSS);
		}	

		if (iff.enterForm(TAG_TFNS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag    tag           = iff.read_uint32();
					const uint32 textureFactor = iff.read_uint32();

					if (implementation && implementation->usesTextureFactor(tag))
					{
						if (!m_textureFactorMap)
							m_textureFactorMap = new TextureFactorMap;

						TextureFactorMap::value_type entry(tag, textureFactor);
						std::pair<TextureFactorMap::iterator, bool> result = m_textureFactorMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TFNS);
		}	

		if (iff.enterForm(TAG_TSNS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag tag = iff.read_uint32();
					UNREF(tag);

					TextureScroll textureScroll;
					textureScroll.u1 = iff.read_float();
					textureScroll.v1 = iff.read_float();
					textureScroll.u2 = iff.read_float();
					textureScroll.v2 = iff.read_float();
					
					if (implementation && implementation->usesTextureScroll(tag))
					{
						if (!m_textureScrollMap)
							m_textureScrollMap = new TextureScrollMap;

						TextureScrollMap::value_type entry(tag, textureScroll);
						std::pair<TextureScrollMap::iterator, bool> result = m_textureScrollMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TSNS);
		}	

		if (iff.enterForm(TAG_ARVS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag   tag = iff.read_uint32();
					const uint8 arv = iff.read_uint8();

					if (implementation && implementation->usesAlphaReference(tag))
					{
						if (!m_alphaTestReferenceValueMap)
							m_alphaTestReferenceValueMap = new AlphaTestReferenceValueMap;

						AlphaTestReferenceValueMap::value_type entry(tag, arv);
						std::pair<AlphaTestReferenceValueMap::iterator, bool> result = m_alphaTestReferenceValueMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_ARVS);
		}	

		if (iff.enterForm(TAG_SRVS, true))
		{
			iff.enterChunk(TAG_0000);
				while (iff.getChunkLengthLeft())
				{
					const Tag    tag = iff.read_uint32();
					const uint32 srv = iff.read_uint32();

					if (implementation && implementation->usesStencilReference(tag))
					{
						if (!m_stencilReferenceValueMap)
							m_stencilReferenceValueMap = new StencilReferenceValueMap;

						StencilReferenceValueMap::value_type entry(tag, srv);
						std::pair<StencilReferenceValueMap::iterator, bool> result = m_stencilReferenceValueMap->insert(entry);
						DEBUG_FATAL(!result.second, ("insert failed"));
					}
				}

			iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_SRVS);
		}	

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void	StaticShaderTemplate::load_texture(Iff & iff)
{
	iff.enterForm(TAG_TXM);

		switch(iff.getCurrentName())
		{
			case TAG_0000:
				load_texture_0000(iff);
				break;

			case TAG_0001:
				load_texture_0001(iff);
				break;

			case TAG_0002:
				load_texture_0002(iff);
				break;

			default:
				DEBUG_FATAL(true,("Invalid shader texture info version"));
				break;
		}

	iff.exitForm(TAG_TXM);
}

// ----------------------------------------------------------------------

void	StaticShaderTemplate::load_texture_0000(Iff & iff)
{
	TextureData textureData;

	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			textureData.placeholder = iff.read_bool8();
			const Tag tag = iff.read_uint32();
			if (tag == TAG_ENVM)
				textureData.placeholder = true;
			textureData.addressU              = TA_invalid;
			textureData.addressV              = TA_invalid;
			textureData.addressW              = TA_invalid;
			textureData.mipFilter             = TF_invalid;
			textureData.minificationFilter    = TF_invalid;
			textureData.magnificationFilter   = TF_invalid;
			textureData.maxAnisotropy         = 1;
		iff.exitChunk(TAG_DATA);

		if ((m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->usesTexture(tag)) || ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			if (!m_textureDataMap)
				m_textureDataMap = new TextureDataMap;

			if (textureData.placeholder)
				textureData.texture = NULL;
			else
				textureData.texture = TextureList::fetch(&iff);

			TextureDataMap::value_type entry(tag, textureData);
			std::pair<TextureDataMap::iterator, bool> result = m_textureDataMap->insert(entry);
			DEBUG_FATAL(!result.second, ("insert failed"));
		}

	// if the texture wasn't used, we can bail out early
	// some placeholder shaders have texture names when they shouldn't
	iff.exitForm(TAG_0000, true);
}

// ----------------------------------------------------------------------

void	StaticShaderTemplate::load_texture_0001(Iff & iff)
{
	iff.enterForm(TAG_0001);

		TextureData textureData;

		iff.enterChunk(TAG_DATA);
			const Tag tag                     = iff.read_uint32();
			textureData.placeholder           = iff.read_bool8();
			if (tag == TAG_ENVM)
				textureData.placeholder = true;
			textureData.addressU              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.addressV              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.addressW              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.mipFilter             = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.minificationFilter    = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.magnificationFilter   = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.maxAnisotropy         = 1;
		iff.exitChunk(TAG_DATA);

		if ((m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->usesTexture(tag)) || ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			if (!m_textureDataMap)
				m_textureDataMap = new TextureDataMap;

			if (textureData.placeholder)
				textureData.texture = NULL;
			else
				textureData.texture = TextureList::fetch(&iff);

			TextureDataMap::value_type entry(tag, textureData);
			std::pair<TextureDataMap::iterator, bool> result = m_textureDataMap->insert(entry);
			DEBUG_FATAL(!result.second, ("insert failed"));
		}

	// if the texture wasn't used, we can bail out early
	// some placeholder shaders have texture names when they shouldn't
	iff.exitForm(TAG_0001, true);
}

// ----------------------------------------------------------------------

void	StaticShaderTemplate::load_texture_0002(Iff & iff)
{
	iff.enterForm(TAG_0002);

		TextureData textureData;

		iff.enterChunk(TAG_DATA);
			const Tag tag                     = iff.read_uint32();
			textureData.placeholder           = iff.read_bool8();
			if (tag == TAG_ENVM)
				textureData.placeholder = true;
			textureData.addressU              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.addressV              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.addressW              = static_cast<TextureAddress>(iff.read_uint8()); 
			textureData.mipFilter             = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.minificationFilter    = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.magnificationFilter   = static_cast<TextureFilter>(iff.read_uint8()); 
			textureData.maxAnisotropy         = iff.read_uint8();
		iff.exitChunk(TAG_DATA);

		if ((m_effect->getActiveShaderImplementation() && m_effect->getActiveShaderImplementation()->usesTexture(tag)) || ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			if (!m_textureDataMap)
				m_textureDataMap = new TextureDataMap;

			if (textureData.placeholder)
				textureData.texture = NULL;
			else
				textureData.texture = TextureList::fetch(&iff);

			TextureDataMap::value_type entry(tag, textureData);
			std::pair<TextureDataMap::iterator, bool> result = m_textureDataMap->insert(entry);
			DEBUG_FATAL(!result.second, ("insert failed"));
		}

	// if the texture wasn't used, we can bail out early
	// some placeholder shaders have texture names when they shouldn't
	iff.exitForm(TAG_0002, true);
}

// ======================================================================
