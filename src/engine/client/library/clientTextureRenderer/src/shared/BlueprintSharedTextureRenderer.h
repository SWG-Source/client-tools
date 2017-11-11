// ======================================================================
//
// BlueprintSharedTextureRenderer.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BlueprintSharedTextureRenderer_H
#define INCLUDED_BlueprintSharedTextureRenderer_H

// ======================================================================

#include "clientTextureRenderer/TextureRenderer.h"

class BlueprintTextureRendererTemplate ;
class CustomizationData;
class Shader;
class Texture;
class TextureRendererTemplate;

// ======================================================================
/**
 * Stores one configuration of a BlueprintTextureRenderer.
 *
 * TextureRenderer instances can be modified.  Several TextureRenderer
 * instances can have the same customizations applied.  If this is the
 * case, those TextureRenderer instances should share the same underlying
 * Texture since baking a Texture instance takes some much time.
 *
 * This sharing feature is used heavily during on-the-fly customization.
 * This is because many of the customizations will create and destroy 
 * TextureRenderer instances, even if they have the same value.  Since
 * we destroy the old TextureRenderer instances after we create the new
 * ones, the new ones created with the same customizations will cause
 * the shared Texture instance (held by this class) to be reused rather
 * than rebaked.
 *
 * @see BlueprintTextureRenderer
 * @see BlueprintTextureRendererTemplate
 */

class BlueprintSharedTextureRenderer: public TextureRenderer
{
public:

	typedef stdvector<int>::fwd  IntVector;

public:

	BlueprintSharedTextureRenderer(const BlueprintTextureRendererTemplate &textureRendererTemplate, const IntVector &intValues);

	virtual bool           render();
	virtual void           setCustomizationData(CustomizationData *customizationData);
	virtual void           addCustomizationVariables(CustomizationData &customizationData) const;
	virtual const Texture *fetchTexture() const;
	virtual bool           isTextureReady() const;

	const IntVector       &getIntValues() const;
	bool                   isBakingComplete() const;

private:

	typedef stdvector<Shader*>::fwd  ShaderVector;

private:

	virtual ~BlueprintSharedTextureRenderer();

	// disabled
	BlueprintSharedTextureRenderer();
	BlueprintSharedTextureRenderer(const BlueprintSharedTextureRenderer&);
	BlueprintSharedTextureRenderer &operator =(const BlueprintSharedTextureRenderer&);

private:

	IntVector    *const m_intValues;
	Texture      *const m_texture;
	ShaderVector *const m_shaders;
	bool                m_isBakingComplete;

};

// ======================================================================

inline const BlueprintSharedTextureRenderer::IntVector &BlueprintSharedTextureRenderer::getIntValues() const
{
	return *m_intValues;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether the Texture instance modified by this
 * TextureRenderer instance has been baked.
 *
 * @return  true if the Texture instance modified by this
 *          TextureRenderer instance has been baked; false otherwise.
 */

inline bool BlueprintSharedTextureRenderer::isBakingComplete() const
{
	return m_isBakingComplete;
}

// ======================================================================

#endif
