// ======================================================================
//
// BlueprintSharedTextureRenderer.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/BlueprintSharedTextureRenderer.h"

#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/Texture.h"
#include "clientTextureRenderer/BlueprintTextureRendererTemplate.h"
#include "clientTextureRenderer/TextureRendererManager.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <vector>

// ======================================================================

BlueprintSharedTextureRenderer::BlueprintSharedTextureRenderer(const BlueprintTextureRendererTemplate &textureRendererTemplate, const IntVector &intValues) :
	TextureRenderer(textureRendererTemplate),
	m_intValues(new IntVector(intValues)),
	m_texture(textureRendererTemplate.fetchCompatibleTexture()),
	m_shaders(new ShaderVector()),
	m_isBakingComplete(false)
{
	//-- create the shaders
	typedef BlueprintTextureRendererTemplate::ShaderTemplateContainer                  STContainer;
	typedef BlueprintTextureRendererTemplate::ShaderTemplateContainer::const_iterator  STCIterator;

	const STContainer &shaderTemplates = textureRendererTemplate.getShaderTemplates();
	m_shaders->reserve(shaderTemplates.size());

	const STCIterator itEnd = shaderTemplates.end();
	for (STCIterator it = shaderTemplates.begin(); it != itEnd; ++it)
	{
		const ShaderTemplate *const shaderTemplate = (*it);
		m_shaders->push_back(shaderTemplate->fetchModifiableShader());
	}

	//-- queue for initial bake
	TextureRendererManager::submitTextureRenderer(this);
}

// ----------------------------------------------------------------------

bool BlueprintSharedTextureRenderer::render()
{
	const BlueprintTextureRendererTemplate *const trTemplate = safe_cast<const BlueprintTextureRendererTemplate*>(&getTextureRendererTemplate());
	NOT_NULL(trTemplate);

	NOT_NULL(m_intValues); 
	trTemplate->prepareShaders(*m_intValues, *m_shaders);

	//-- render
	const bool result = trTemplate->render(m_texture, *m_intValues, *m_shaders);

	//-- baking is complete
	if (result)
		m_isBakingComplete = true;

	return result;
}

// ----------------------------------------------------------------------

void BlueprintSharedTextureRenderer::setCustomizationData(CustomizationData * /* customizationData */)
{
	DEBUG_FATAL(true, ("this should never be called for the BlueprintSharedTextureRenderer class"));
}

// ----------------------------------------------------------------------

void BlueprintSharedTextureRenderer::addCustomizationVariables(CustomizationData & /* customizationData */) const
{
	DEBUG_FATAL(true, ("this should never be called for the BlueprintSharedTextureRenderer class"));
}

// ----------------------------------------------------------------------

const Texture *BlueprintSharedTextureRenderer::fetchTexture() const
{
	m_texture->fetch();
	return m_texture;
}

// ----------------------------------------------------------------------

bool BlueprintSharedTextureRenderer::isTextureReady() const
{
	return isBakingComplete();
}

// ======================================================================

BlueprintSharedTextureRenderer::~BlueprintSharedTextureRenderer()
{
	//-- notify the template that this shared texture renderer is getting destroyed
	const BlueprintTextureRendererTemplate *const trTemplate = safe_cast<const BlueprintTextureRendererTemplate*>(&getTextureRendererTemplate());
	NOT_NULL(trTemplate);

	trTemplate->notifySharedTextureDestruction(this);

	//-- cleanup resources
	std::for_each(m_shaders->begin(), m_shaders->end(), VoidMemberFunction(&Shader::release));
	delete m_shaders;

	delete m_intValues;

	//lint -esym(1740, BlueprintSharedTextureRenderer::m_texture) // not zero'ed or freed // released, it's okay
	m_texture->release();
}

// ======================================================================
