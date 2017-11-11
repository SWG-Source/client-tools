// ======================================================================
//
// TextureRendererShaderPrimitive.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererShaderPrimitive.h"

#include "clientGraphics/Material.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <vector>

// ======================================================================

bool                TextureRendererShaderPrimitive::ms_installed;
MemoryBlockManager *TextureRendererShaderPrimitive::ms_memoryBlockManager;

// ======================================================================

struct TextureRendererShaderPrimitive::TextureRendererInfo
{
public:

	TextureRendererInfo(TextureRenderer *textureRenderer, Tag textureTag);

public:

	TextureRenderer *m_textureRenderer;
	Tag              m_textureTag;

private:

	// disabled
	TextureRendererInfo();

};

// ======================================================================

TextureRendererShaderPrimitive::TextureRendererInfo::TextureRendererInfo(TextureRenderer *textureRenderer, Tag textureTag) :
	m_textureRenderer(textureRenderer),
	m_textureTag(textureTag)
{
}

// ======================================================================

void TextureRendererShaderPrimitive::install()
{
	DEBUG_FATAL(ms_installed, ("TextureRendererShaderPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("TextureRendererShaderPrimitive", true, sizeof(TextureRendererShaderPrimitive), 0, 0, 0);

	ms_installed = true;
	ExitChain::add(remove, "TextureRendererShaderPrimitive");
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::remove()
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererShaderPrimitive not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void *TextureRendererShaderPrimitive::operator new(size_t size)
{
	DEBUG_FATAL(size != sizeof(TextureRendererShaderPrimitive), ("descendants not supported"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::operator delete(void *data, size_t size)
{
	DEBUG_FATAL(size != sizeof(TextureRendererShaderPrimitive), ("descendants not supported"));
	UNREF(size);

	ms_memoryBlockManager->free(data);
}

// ======================================================================

TextureRendererShaderPrimitive::TextureRendererShaderPrimitive(ShaderPrimitive *shaderPrimitive) :
	ShaderPrimitive(),
	m_shaderPrimitive(shaderPrimitive),
	m_textureRendererInfoVector(new TextureRendererInfoVector())
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererShaderPrimitive not installed"));
	NOT_NULL(shaderPrimitive);
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive::TextureRendererShaderPrimitive(ShaderPrimitive *shaderPrimitive, const TextureRendererVector &textureRenderers, const TagVector &textureTags) :
	ShaderPrimitive(),
	m_shaderPrimitive(shaderPrimitive),
	m_textureRendererInfoVector(new TextureRendererInfoVector())
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererShaderPrimitive not installed"));
	DEBUG_FATAL(textureRenderers.size() != textureTags.size(), ("textureRenderers and tags args should have same # elements but don't %u/%u", textureRenderers.size(), textureTags.size()));
	NOT_NULL(shaderPrimitive);

	//-- Get the static shader so we can check which texture tags are present.  We'll drop the texture
	//   baking for any non-supported texture.
	StaticShader const &shader = m_shaderPrimitive->prepareToView();

	//-- fetch the texture renderers
	m_textureRendererInfoVector->reserve(textureRenderers.size());

	const size_t entryCount = textureRenderers.size();
	for (size_t i = 0; i < entryCount; ++i)
	{
		if (!shader.hasTexture(textureTags[i]))
		{
			//-- Skip this texture renderer -- the end result of it would not be used by this shader.
			continue;
		}

		//-- fetch reference to texture renderer
		TextureRenderer *const tr = textureRenderers[i];
		tr->fetch();

		//-- add texture renderer info to container
		m_textureRendererInfoVector->push_back(TextureRendererInfo(tr, textureTags[i]));
	}
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive::~TextureRendererShaderPrimitive()
{
	//-- release the texture renderers
	{
		const TextureRendererInfoVector::iterator endIt = m_textureRendererInfoVector->end();
		for (TextureRendererInfoVector::iterator it = m_textureRendererInfoVector->begin(); it != endIt; ++it)
		{
			it->m_textureRenderer->release();
		}
	}
	delete m_textureRendererInfoVector;

	//-- assume we own the shader primitive.  would be nice if this was ref counted.
	delete m_shaderPrimitive;
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::addTextureRenderer(TextureRenderer &textureRenderer, Tag textureTag)
{
	//-- fetch a reference
	textureRenderer.fetch();

	//-- add to list of references
	m_textureRendererInfoVector->push_back(TextureRendererInfo(&textureRenderer, textureTag));
} //lint !e1764 // (Info -- Reference parameter 'textureRenderer' (line 96) could be declared const ref) // hmm? &(const &object) == const object *, I need an object *.

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const
{
	m_shaderPrimitive->getCostEstimate(numberOfVertices, numberOfTriangles, complexity);
}

// ----------------------------------------------------------------------

float TextureRendererShaderPrimitive::alter(float deltaTime)
{
	return m_shaderPrimitive->alter(deltaTime);
}

// ----------------------------------------------------------------------

const Vector TextureRendererShaderPrimitive::getPosition_w() const
{
	return m_shaderPrimitive->getPosition_w();
}
	
// ----------------------------------------------------------------------

float TextureRendererShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_shaderPrimitive->getDepthSquaredSortKey();
}

// ----------------------------------------------------------------------

int TextureRendererShaderPrimitive::getVertexBufferSortKey() const
{
	return m_shaderPrimitive->getVertexBufferSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &TextureRendererShaderPrimitive::prepareToView() const
{
	//-- fetch TextureRenderer textures for the shader

	// @todo this const_cast is a no no.  This should go away once TextureRenderer
	// textures appear as a full-fledged Texture type.
	StaticShader *modifiableShader = const_cast<StaticShader*>(&m_shaderPrimitive->prepareToView());

	//-- assign each TextureRenderer texture to the appropriate Shader texture slot
	const TextureRendererInfoVector::const_iterator endIt = m_textureRendererInfoVector->end();
	for (TextureRendererInfoVector::const_iterator it = m_textureRendererInfoVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(it->m_textureRenderer);

		//-- fetch the texture
		const Texture *const texture = it->m_textureRenderer->fetchTexture();
		NOT_NULL(texture);

		//-- assign texture to the shader
		modifiableShader->setTexture(it->m_textureTag, *texture);

		//-- release the local texture reference
		texture->release();
	}
	
	return *modifiableShader;
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::prepareToDraw() const
{
	m_shaderPrimitive->prepareToDraw();
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::draw() const
{
	m_shaderPrimitive->draw();
}

// ----------------------------------------------------------------------
/**
 * Set the CustomizationData instance to be associated with this
 * ShaderPrimitive instance.
 *
 * This implementation will set the CustomizationData for all
 * TextureRenderer instances and the target ShaderPrimitive instance.
 *
 * @param customizationData  the new CustomizationData instance to be
 *                           associated with this ShaderPrimitive instance.
 *                           This value may be NULL and may be the same
 *                           customizationData value already associated with
 *                           this ShaderPrimitive instance.
 */

void TextureRendererShaderPrimitive::setCustomizationData(CustomizationData *customizationData)
{
	//-- set CustomizationData for each TextureRenderer instance
	const TextureRendererInfoVector::iterator endIt = m_textureRendererInfoVector->end();
	for (TextureRendererInfoVector::iterator it = m_textureRendererInfoVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(it->m_textureRenderer);
		it->m_textureRenderer->setCustomizationData(customizationData);
	}


	//-- set CustomizationData for the target ShaderPrimitive
	m_shaderPrimitive->setCustomizationData(customizationData);
}

// ----------------------------------------------------------------------
/**
 * @see ShaderPrimitive::addCustomizationVariables()
 */

void TextureRendererShaderPrimitive::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- add variables for each TextureRenderer
	const TextureRendererInfoVector::const_iterator endIt = m_textureRendererInfoVector->end();
	for (TextureRendererInfoVector::const_iterator it = m_textureRendererInfoVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(it->m_textureRenderer);
		it->m_textureRenderer->addCustomizationVariables(customizationData);
	}

	//-- add variables for target ShaderPrimitive
	m_shaderPrimitive->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::calculateSkinnedGeometryNow()
{
	m_shaderPrimitive->calculateSkinnedGeometryNow();
}

// ----------------------------------------------------------------------

void TextureRendererShaderPrimitive::setSkinningMode(SkinningMode skinningMode)
{
	m_shaderPrimitive->setSkinningMode(skinningMode);
}

// ----------------------------------------------------------------------

bool TextureRendererShaderPrimitive::isReady() const
{
	//-- Check if underlying shader primitive is ready.
	if (!m_shaderPrimitive->isReady())
		return false;

	//-- Check if any of the associated texture renderers are not ready.
		//-- assign each TextureRenderer texture to the appropriate Shader texture slot
	const TextureRendererInfoVector::const_iterator endIt = m_textureRendererInfoVector->end();
	for (TextureRendererInfoVector::const_iterator it = m_textureRendererInfoVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(it->m_textureRenderer);

		//-- fetch the texture
		bool const textureIsReady = it->m_textureRenderer->isTextureReady();
		if (!textureIsReady)
		{
			// At least one texture renderer is not ready.
			return false;
		}
	}

	//-- Everything is ready.
	return true;
}

// ----------------------------------------------------------------------

bool TextureRendererShaderPrimitive::collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const
{
	return m_shaderPrimitive->collide(start_o, end_o, result);
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive *TextureRendererShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive()
{
	return m_shaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive();
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive const *TextureRendererShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive() const
{
	return m_shaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive();
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive *TextureRendererShaderPrimitive::asTextureRendererShaderPrimitive()
{
	return this;
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive const *TextureRendererShaderPrimitive::asTextureRendererShaderPrimitive() const
{
	return this;
}

// ======================================================================
