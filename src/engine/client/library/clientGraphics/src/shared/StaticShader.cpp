// ======================================================================
//
// StaticShader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/StaticShader.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <map>
#include <vector>
#include <algorithm>

// ======================================================================

Tag TAG_DOT3 = TAG(D,O,T,3);

MemoryBlockManager  *StaticShader::ms_memoryBlockManager;

// ======================================================================

StaticShaderGraphicsData::~StaticShaderGraphicsData()
{
}

// ======================================================================

void StaticShader::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("StaticShader", true, sizeof(StaticShader), 0, 0, 0);
	ExitChain::add(remove, "StaticShader::remove()");
}

// ----------------------------------------------------------------------

void StaticShader::remove()
{
	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *StaticShader::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(StaticShader), ("Looks like a decendent class is trying to use our new routine"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void StaticShader::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);

	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

StaticShader::StaticShader(const StaticShaderTemplate &staticShaderTemplate)
: Shader(staticShaderTemplate),
	m_graphicsData(NULL),
	m_shared(true),
	m_addedDot3Placeholder(staticShaderTemplate.m_addedDot3Placeholder),
	m_obeysLightScale(ConfigClientGraphics::getEnableLightScaling()),
	m_usesVertexShader(false),
	m_phase(-1),
	m_numberOfPasses(0),
	m_textureSortKey(0),
	m_shaderImplementationSortKey(staticShaderTemplate.getShaderImplementationSortKey()),
	m_materialMap(const_cast<StaticShaderTemplate::MaterialMap *>(staticShaderTemplate.m_materialMap)),
	m_textureDataMap(const_cast<StaticShaderTemplate::TextureDataMap *>(staticShaderTemplate.m_textureDataMap)),
	m_textureCoordinateSetMap(const_cast<StaticShaderTemplate::TextureCoordinateSetMap *>(staticShaderTemplate.m_textureCoordinateSetMap)),
	m_textureFactorMap(const_cast<StaticShaderTemplate::TextureFactorMap *>(staticShaderTemplate.m_textureFactorMap)),
	m_textureScrollMap(const_cast<StaticShaderTemplate::TextureScrollMap *>(staticShaderTemplate.m_textureScrollMap)),
	m_alphaTestReferenceValueMap(const_cast<StaticShaderTemplate::AlphaTestReferenceValueMap *>(staticShaderTemplate.m_alphaTestReferenceValueMap)),
	m_stencilReferenceValueMap(const_cast<StaticShaderTemplate::StencilReferenceValueMap *>(staticShaderTemplate.m_stencilReferenceValueMap)),
	m_isHeatPasses(),
	m_optionTags()
{
	const ShaderImplementation *shaderImplementation = staticShaderTemplate.getShaderEffect().getActiveShaderImplementation();
	if (shaderImplementation)
	{
		m_phase = shaderImplementation->getPhase();
		m_numberOfPasses = shaderImplementation->getNumberOfPasses();
		m_isHeatPasses.reserve(m_numberOfPasses);
		for (int i = 0; i < m_numberOfPasses; ++i)
		{
			m_isHeatPasses.push_back(shaderImplementation->isHeatPass(i));
		}
		m_optionTags = shaderImplementation->getOptionTags();
		createGlData();
	}
}

// ----------------------------------------------------------------------

StaticShader::StaticShader(const StaticShaderTemplate &staticShaderTemplate, Modifiable)
: Shader(staticShaderTemplate),
	m_graphicsData(NULL),
	m_shared(false),
	m_addedDot3Placeholder(staticShaderTemplate.m_addedDot3Placeholder),
	m_obeysLightScale(ConfigClientGraphics::getEnableLightScaling()),
	m_usesVertexShader(false),
	m_phase(-1),
	m_numberOfPasses(0),
	m_textureSortKey(0),
	m_shaderImplementationSortKey(staticShaderTemplate.getShaderImplementationSortKey()),
	m_materialMap(const_cast<StaticShaderTemplate::MaterialMap *>(staticShaderTemplate.m_materialMap)),
	m_textureDataMap(const_cast<StaticShaderTemplate::TextureDataMap *>(staticShaderTemplate.m_textureDataMap)),
	m_textureCoordinateSetMap(const_cast<StaticShaderTemplate::TextureCoordinateSetMap *>(staticShaderTemplate.m_textureCoordinateSetMap)),
	m_textureFactorMap(const_cast<StaticShaderTemplate::TextureFactorMap *>(staticShaderTemplate.m_textureFactorMap)),
	m_textureScrollMap(const_cast<StaticShaderTemplate::TextureScrollMap *>(staticShaderTemplate.m_textureScrollMap)),
	m_alphaTestReferenceValueMap(const_cast<StaticShaderTemplate::AlphaTestReferenceValueMap *>(staticShaderTemplate.m_alphaTestReferenceValueMap)),
	m_stencilReferenceValueMap(const_cast<StaticShaderTemplate::StencilReferenceValueMap *>(staticShaderTemplate.m_stencilReferenceValueMap)),
	m_isHeatPasses(),
	m_optionTags()
{
	const ShaderImplementation *shaderImplementation = staticShaderTemplate.getShaderEffect().getActiveShaderImplementation();
	if (shaderImplementation)
	{
		m_phase = shaderImplementation->getPhase();
		m_numberOfPasses = shaderImplementation->getNumberOfPasses();
		m_isHeatPasses.reserve(m_numberOfPasses);
		for (int i = 0; i < m_numberOfPasses; ++i)
		{
			m_isHeatPasses.push_back(shaderImplementation->isHeatPass(i));
		}
		m_optionTags = shaderImplementation->getOptionTags();
		createGlData();
	}
}

// ----------------------------------------------------------------------

StaticShader::StaticShader(const StaticShader &staticShader)
: Shader(staticShader),
	m_graphicsData(NULL),
	m_shared(false),
	m_addedDot3Placeholder(staticShader.m_addedDot3Placeholder),
	m_obeysLightScale(staticShader.m_obeysLightScale),
	m_usesVertexShader(staticShader.m_usesVertexShader),
	m_phase(staticShader.m_phase),
	m_numberOfPasses(staticShader.m_numberOfPasses),
	m_textureSortKey(staticShader.m_textureSortKey),
	m_shaderImplementationSortKey(staticShader.m_shaderImplementationSortKey),
	m_materialMap(staticShader.m_materialMap),
	m_textureDataMap(staticShader.m_textureDataMap),
	m_textureCoordinateSetMap(staticShader.m_textureCoordinateSetMap),
	m_textureFactorMap(staticShader.m_textureFactorMap),
	m_textureScrollMap(staticShader.m_textureScrollMap),
	m_alphaTestReferenceValueMap(staticShader.m_alphaTestReferenceValueMap),
	m_stencilReferenceValueMap(staticShader.m_stencilReferenceValueMap),
	m_isHeatPasses(staticShader.m_isHeatPasses),
	m_optionTags(staticShader.m_optionTags)
{
	// copy all data that has been changed immediately because the data might change underneath us without us knowing

	if (m_materialMap != getStaticShaderTemplate().m_materialMap)
		m_materialMap = new StaticShaderTemplate::MaterialMap(*staticShader.m_materialMap);

	if (m_textureDataMap != getStaticShaderTemplate().m_textureDataMap)
	{
		m_textureDataMap = new StaticShaderTemplate::TextureDataMap(*staticShader.m_textureDataMap);
		StaticShaderTemplate::TextureDataMap::iterator end = m_textureDataMap->end();
		for (StaticShaderTemplate::TextureDataMap::iterator i = m_textureDataMap->begin(); i != end; ++i)
			if (i->second.texture)
				i->second.texture->fetch();
	}

	if (m_textureCoordinateSetMap != getStaticShaderTemplate().m_textureCoordinateSetMap)
		m_textureCoordinateSetMap = new StaticShaderTemplate::TextureCoordinateSetMap(*staticShader.m_textureCoordinateSetMap);

	if (m_textureFactorMap != getStaticShaderTemplate().m_textureFactorMap)
		m_textureFactorMap = new StaticShaderTemplate::TextureFactorMap(*staticShader.m_textureFactorMap);

	if (m_textureScrollMap != getStaticShaderTemplate().m_textureScrollMap)
		m_textureScrollMap = new StaticShaderTemplate::TextureScrollMap(*staticShader.m_textureScrollMap);

	if (m_alphaTestReferenceValueMap != getStaticShaderTemplate().m_alphaTestReferenceValueMap)
		m_alphaTestReferenceValueMap = new StaticShaderTemplate::AlphaTestReferenceValueMap(*staticShader.m_alphaTestReferenceValueMap);

	if (m_stencilReferenceValueMap != getStaticShaderTemplate().m_stencilReferenceValueMap)
		m_stencilReferenceValueMap = new StaticShaderTemplate::StencilReferenceValueMap(*staticShader.m_stencilReferenceValueMap);

	createGlData();
}

// ----------------------------------------------------------------------

StaticShader::~StaticShader()
{
	if (m_shared)
		getStaticShaderTemplate().destroyStaticShader(*this);

	delete m_graphicsData;

	if (m_materialMap != getStaticShaderTemplate().m_materialMap)
		delete m_materialMap;
	m_materialMap = NULL;

	if (m_textureDataMap != getStaticShaderTemplate().m_textureDataMap)
	{
		StaticShaderTemplate::TextureDataMap::iterator end = m_textureDataMap->end();
		for (StaticShaderTemplate::TextureDataMap::iterator i = m_textureDataMap->begin(); i != end; ++i)
			if (i->second.texture)
				i->second.texture->release();

		delete m_textureDataMap;
	}
	m_textureDataMap = NULL;

	if (m_textureCoordinateSetMap != getStaticShaderTemplate().m_textureCoordinateSetMap)
		delete m_textureCoordinateSetMap;
	m_textureCoordinateSetMap = NULL;

	if (m_textureFactorMap != getStaticShaderTemplate().m_textureFactorMap)
		delete m_textureFactorMap;
	m_textureFactorMap = NULL;

	if (m_textureScrollMap != getStaticShaderTemplate().m_textureScrollMap)
		delete m_textureScrollMap;
	m_textureScrollMap = NULL;

	if (m_alphaTestReferenceValueMap != getStaticShaderTemplate().m_alphaTestReferenceValueMap)
		delete m_alphaTestReferenceValueMap;
	m_alphaTestReferenceValueMap = NULL;

	if (m_stencilReferenceValueMap != getStaticShaderTemplate().m_stencilReferenceValueMap)
		delete m_stencilReferenceValueMap;
	m_stencilReferenceValueMap = NULL;
}

// ----------------------------------------------------------------------

Shader *StaticShader::convertToModifiableShader() const
{
	Shader *result = fetchModifiable();
	release();
	return result;
}

// ----------------------------------------------------------------------

bool StaticShader::obeysCustomizationData() const
{
	return false;
}

// ----------------------------------------------------------------------

void StaticShader::createGlData() const
{
	m_graphicsData = Graphics::createStaticShaderGraphicsData(*this);
	m_textureSortKey = m_graphicsData->getTextureSortKey();
}

// ----------------------------------------------------------------------

int StaticShader::getTextureSortKey() const
{
	return m_textureSortKey;
}

// ----------------------------------------------------------------------

int StaticShader::getShaderTemplateSortKey() const
{
	return reinterpret_cast<int>(&getStaticShaderTemplate());
}

// ----------------------------------------------------------------------

int StaticShader::getShaderImplementationSortKey() const
{
	return m_shaderImplementationSortKey;
}


// ----------------------------------------------------------------------

const StaticShader &StaticShader::prepareToView() const
{
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Set the phase in which to render this shader.
 */

void StaticShader::setPhaseTag(Tag tag)
{
	m_phase = ShaderPrimitiveSorter::getPhase(tag);
}

// ----------------------------------------------------------------------

bool StaticShader::containsPrecalculatedVertexLighting() const
{
	return getStaticShaderTemplate().containsPrecalculatedVertexLighting();
}

// ----------------------------------------------------------------------

void StaticShader::setObeysLightScale(const bool obeysLightScale)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	m_obeysLightScale = obeysLightScale;
}

// ----------------------------------------------------------------------

void StaticShader::setMaterial(Tag tag, const Material &material)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	DEBUG_FATAL(!m_materialMap, ("Shader has no materials to set"));

	if (m_materialMap == getStaticShaderTemplate().m_materialMap)
		m_materialMap = new StaticShaderTemplate::MaterialMap(*getStaticShaderTemplate().m_materialMap);
	
	StaticShaderTemplate::MaterialMap::iterator i = m_materialMap->find(tag);
	if (i == m_materialMap->end())
	{
		char buf[16];
		ConvertTagToString(tag, buf);
		WARNING(true, ("Material tag [%s] not found in [%s]", buf, getName()));
		return;
	}

	i->second = material;

	m_graphicsData->update(*this);
}

// ----------------------------------------------------------------------

void StaticShader::setTexture(Tag tag, const Texture &texture)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	if(!m_textureDataMap)
	{
		WARNING(true , ("StaticShader::setTexture -- Shader has no textures to set"));
		return;
	}

	if (m_textureDataMap == getStaticShaderTemplate().m_textureDataMap)
	{
		m_textureDataMap = new StaticShaderTemplate::TextureDataMap(*getStaticShaderTemplate().m_textureDataMap);

		// add a reference count to all of these textures
		StaticShaderTemplate::TextureDataMap::iterator end = m_textureDataMap->end();
		for (StaticShaderTemplate::TextureDataMap::iterator i = m_textureDataMap->begin(); i != end; ++i)
			if (i->second.texture)
				i->second.texture->fetch();
	}

	StaticShaderTemplate::TextureDataMap::iterator i = m_textureDataMap->find(tag);
	if (i == m_textureDataMap->end())
	{
		char tagString[5];
		ConvertTagToString(tag, tagString);
		DEBUG_WARNING(true, ("StaticShader::setTexture: texture tag [%s] not found in shader template %s", tagString, getStaticShaderTemplate().getName().getString()));
	}
	else
	{
		// replace the old texture
		if (i->second.texture)
			i->second.texture->release();
		i->second.placeholder = false;
		i->second.texture = &texture;
		i->second.texture->fetch();

		m_graphicsData->update(*this);
		m_textureSortKey = m_graphicsData->getTextureSortKey();
	}
}

// ----------------------------------------------------------------------

void StaticShader::setTextureCoordinateSet(Tag tag, uint8 textureCoordinateSet)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	DEBUG_FATAL(!m_textureCoordinateSetMap, ("Shader has no texture coordinates to set"));

	// hack to support old art
	if (tag == TAG_DOT3)
		m_addedDot3Placeholder = false;

	if (m_textureCoordinateSetMap == getStaticShaderTemplate().m_textureCoordinateSetMap)
		m_textureCoordinateSetMap = new StaticShaderTemplate::TextureCoordinateSetMap(*getStaticShaderTemplate().m_textureCoordinateSetMap);

	StaticShaderTemplate::TextureCoordinateSetMap::iterator i = m_textureCoordinateSetMap->find(tag);
	DEBUG_FATAL(i == m_textureCoordinateSetMap->end(), ("Texture coordinate set not found"));
	i->second = textureCoordinateSet;

	m_graphicsData->update(*this);
}

// ----------------------------------------------------------------------

void StaticShader::setAlphaTestReferenceValue(Tag tag, uint8 alphaTestReferenceValue)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	DEBUG_FATAL(!m_alphaTestReferenceValueMap , ("Shader has no alpha reference values to set"));

	if (m_alphaTestReferenceValueMap == getStaticShaderTemplate().m_alphaTestReferenceValueMap)
		m_alphaTestReferenceValueMap = new StaticShaderTemplate::AlphaTestReferenceValueMap(*getStaticShaderTemplate().m_alphaTestReferenceValueMap);

	StaticShaderTemplate::AlphaTestReferenceValueMap::iterator i = m_alphaTestReferenceValueMap->find(tag);
	DEBUG_FATAL(i == m_alphaTestReferenceValueMap->end(), ("Alpha test reference value not found"));
	i->second = alphaTestReferenceValue;

	m_graphicsData->update(*this);
}

// ----------------------------------------------------------------------

void StaticShader::setStencilReferenceValue(Tag tag, uint32 stencilReferenceValue)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	DEBUG_FATAL(!m_stencilReferenceValueMap , ("Shader has no stencil coordinates to set"));

	if (m_stencilReferenceValueMap == getStaticShaderTemplate().m_stencilReferenceValueMap)
		m_stencilReferenceValueMap = new StaticShaderTemplate::StencilReferenceValueMap(*getStaticShaderTemplate().m_stencilReferenceValueMap);

	StaticShaderTemplate::StencilReferenceValueMap::iterator i = m_stencilReferenceValueMap->find(tag);
	DEBUG_FATAL(i == m_stencilReferenceValueMap->end(), ("Stencil reference value not found"));
	i->second = stencilReferenceValue;

	m_graphicsData->update(*this);
}

// ----------------------------------------------------------------------

void StaticShader::setTextureFactor(Tag tag, uint32 textureFactor)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	if (!m_textureFactorMap)
	{
		WARNING(true, ("Shader has no texture factors to set"));
		return;
	}

	if (m_textureFactorMap == getStaticShaderTemplate().m_textureFactorMap)
		m_textureFactorMap = new StaticShaderTemplate::TextureFactorMap(*getStaticShaderTemplate().m_textureFactorMap);

	StaticShaderTemplate::TextureFactorMap::iterator i = m_textureFactorMap->find(tag);

	if (i == m_textureFactorMap->end())
	{
		WARNING(true, ("Setting non-existant texture factor"));
	}
	else
	{
		i->second = textureFactor;
		m_graphicsData->update(*this);
	}
}

// ----------------------------------------------------------------------

void StaticShader::setTextureScroll(const Tag tag, const StaticShaderTemplate::TextureScroll &textureScroll)
{
	DEBUG_FATAL(m_shared, ("Trying to change a shared static shader"));
	if (!m_textureScrollMap)
	{
		WARNING(true, ("Shader has no texture scrolls to set"));
		return;
	}

	if (m_textureScrollMap == getStaticShaderTemplate().m_textureScrollMap)
		m_textureScrollMap = new StaticShaderTemplate::TextureScrollMap(*getStaticShaderTemplate().m_textureScrollMap);

	StaticShaderTemplate::TextureScrollMap::iterator i = m_textureScrollMap->find(tag);

	if (i == m_textureScrollMap->end())
	{
		WARNING(true, ("Setting non-existant texture scroll"));
	}
	else
	{
		i->second = textureScroll;
		m_graphicsData->update(*this);
	}
}

// ----------------------------------------------------------------------

bool StaticShader::getMaterial(Tag tag, Material &material) const
{
	if (!m_materialMap)
		return false;

	const StaticShaderTemplate::MaterialMap::const_iterator i = m_materialMap->find(tag);
	if (i == m_materialMap->end())
		return false;

	material = i->second;
	return true;
}

// ----------------------------------------------------------------------

void StaticShader::getTextureTags(stdvector<Tag>::fwd & textureTags) const
{
	if (m_textureDataMap)
	{
		StaticShaderTemplate::TextureDataMap::const_iterator end = m_textureDataMap->end();
		for (StaticShaderTemplate::TextureDataMap::const_iterator iter = m_textureDataMap->begin(); iter != end; ++iter)
			textureTags.push_back(iter->first);
	}
}

// ----------------------------------------------------------------------

bool StaticShader::getTextureData(Tag tag, StaticShaderTemplate::TextureData &textureData) const
{
	if (!m_textureDataMap)
		return false;

	const StaticShaderTemplate::TextureDataMap::const_iterator i = m_textureDataMap->find(tag);
	if (i == m_textureDataMap->end())
		return false;

	textureData = i->second;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getTexture(Tag tag, const Texture *&texture) const
{
	if (!m_textureDataMap)
		return false;

	const StaticShaderTemplate::TextureDataMap::const_iterator i = m_textureDataMap->find(tag);
	if (i == m_textureDataMap->end())
		return false;

	texture = i->second.texture;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getTextureCoordinateSet(Tag tag, uint8 &textureCoordinateSet) const
{
	if (!m_textureCoordinateSetMap)
		return false;

	const StaticShaderTemplate::TextureCoordinateSetMap::const_iterator i = m_textureCoordinateSetMap->find(tag);
	if (i == m_textureCoordinateSetMap->end())
		return false;

	textureCoordinateSet = i->second;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getTextureFactor(Tag tag, uint32 &textureFactor) const
{
	if (!m_textureFactorMap)
		return false;

	const StaticShaderTemplate::TextureFactorMap::const_iterator i = m_textureFactorMap->find(tag);
	if (i == m_textureFactorMap->end())
		return false;

	textureFactor = i->second;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getTextureScroll(Tag tag, StaticShaderTemplate::TextureScroll &textureScroll) const
{
	if (!m_textureScrollMap)
		return false;

	const StaticShaderTemplate::TextureScrollMap::const_iterator i = m_textureScrollMap->find(tag);
	if (i == m_textureScrollMap->end())
		return false;

	textureScroll = i->second;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getAlphaTestReferenceValue(Tag tag, uint8 &alphaTestReferenceValue) const
{
	if (!m_alphaTestReferenceValueMap)
		return false;

	const StaticShaderTemplate::AlphaTestReferenceValueMap::const_iterator i = m_alphaTestReferenceValueMap->find(tag);
	if (i == m_alphaTestReferenceValueMap->end())
		return false;

	alphaTestReferenceValue = i->second;
	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::getStencilReferenceValue(Tag tag, uint32 &stencilReferenceValue) const
{
	if (!m_stencilReferenceValueMap)
		return false;

	const StaticShaderTemplate::StencilReferenceValueMap::const_iterator i = m_stencilReferenceValueMap->find(tag);
	if (i == m_stencilReferenceValueMap->end())
		return false;

	stencilReferenceValue = i->second;
	return true;
}

// ----------------------------------------------------------------------
/**
 * Check if this shader makes use of a specified material.
 *
 * Note that different video cards support different implementations of
 * the same shader.  It may be the case that a shader attribute
 * is supported on some hardware but not on others.
 *
 * @param tag  the material tag indicating the material to be checked for.
 *
 * @return  true if the material is supported; false otherwise.
 */

bool StaticShader::hasMaterial(Tag tag) const
{
	if (!m_materialMap)
		return false;

	const StaticShaderTemplate::MaterialMap::const_iterator i = m_materialMap->find(tag);
	if (i == m_materialMap->end())
		return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Check if this shader makes use of a specified texture.
 *
 * Note that different video cards support different implementations of
 * the same shader.  It may be the case that a texture or other shader attribute
 * is supported on some hardware but not on others.
 *
 * @param tag  the texture tag indicating the texture to be checked for.
 *
 * @return  true if the texture is supported; false otherwise.
 */

bool StaticShader::hasTexture(Tag tag) const
{
	if (!m_textureDataMap)
		return false;

	const StaticShaderTemplate::TextureDataMap::const_iterator i = m_textureDataMap->find(tag);
	if (i == m_textureDataMap->end())
		return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Check if this shader makes use of a specified texture factor.
 *
 * Note that different video cards support different implementations of
 * the same shader.  It may be the case that a shader attribute
 * is supported on some hardware but not on others.
 *
 * @param tag  the texture factor tag indicating the texture factor to be checked for.
 *
 * @return  true if the texture factor is supported; false otherwise.
 */

bool StaticShader::hasTextureFactor(Tag tag) const
{
	if (!m_textureFactorMap)
		return false;

	const StaticShaderTemplate::TextureFactorMap::const_iterator i = m_textureFactorMap->find(tag);
	if (i == m_textureFactorMap->end())
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool StaticShader::hasTextureScroll(Tag tag) const
{
	if (!m_textureScrollMap)
		return false;

	const StaticShaderTemplate::TextureScrollMap::const_iterator i = m_textureScrollMap->find(tag);
	if (i == m_textureScrollMap->end())
		return false;

	return true;
}

// ----------------------------------------------------------------------

const StaticShader *StaticShader::getStaticShader() const
{
	return this;
}

// ----------------------------------------------------------------------

StaticShader *StaticShader::getStaticShader()
{
	return this;
}

//----------------------------------------------------------------------

bool StaticShader::hasOptionTag(Tag tag) const
{
	return std::binary_search(m_optionTags.begin(), m_optionTags.end(), tag);
}


// ----------------------------------------------------------------------

bool StaticShader::usesVertexShader() const
{
	return m_usesVertexShader;
}

// ======================================================================

