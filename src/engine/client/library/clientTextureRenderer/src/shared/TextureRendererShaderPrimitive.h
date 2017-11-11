// ======================================================================
//
// TextureRendererShaderPrimitive.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TextureRendererShaderPrimitive_H
#define INCLUDED_TextureRendererShaderPrimitive_H

// ======================================================================

#include "clientGraphics/ShaderPrimitive.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class MemoryBlockManager;
class TextureRenderer;

// ======================================================================

/**
 * Simple pass-through ShaderPrimitive that piggy-backs TextureRenderer
 * data on top of another ShaderPrimitive.
 *
 * This is intended to be a temporary class until we have texture baking
 * supported either through a generic Texture interface or through
 * shader support.
 *
 * This is used to keep track of texture renderers that affect
 * the shader associated with a ShaderPrimitive.  It makes it possible
 * to bake textures associated with ShaderPrimtive objects without having
 * to separately store texture renderers.  It also makes it simpler for
 * code generating ShaderPrimitive objects to hide texture renderer management
 * from the API used to construct the ShaderPrimitive objects.
 */

class TextureRendererShaderPrimitive: public ShaderPrimitive
{
public:

	typedef stdvector<TextureRenderer*>::fwd  TextureRendererVector;
	typedef stdvector<Tag>::fwd               TagVector;

public:

	static void install();

	static void *operator new(size_t size);
	static void  operator delete(void *data, size_t size);

public:

	explicit TextureRendererShaderPrimitive(ShaderPrimitive *shaderPrimitive);
	TextureRendererShaderPrimitive(ShaderPrimitive *shaderPrimitive, const TextureRendererVector &textureRenderers, const TagVector &textureTags);
	~TextureRendererShaderPrimitive();

	void                         addTextureRenderer(TextureRenderer &textureRenderer, Tag textureTag);

	ShaderPrimitive             &getRealShaderPrimitive();
	const ShaderPrimitive       &getRealShaderPrimitive() const;

	virtual void                 getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const;
	virtual const Vector         getPosition_w() const;
	virtual float                alter(float deltaTime);
	virtual float                getDepthSquaredSortKey() const;
	virtual int                  getVertexBufferSortKey() const;
	virtual const StaticShader  &prepareToView() const;
	virtual void                 prepareToDraw() const;
	virtual void                 draw() const;
	virtual void                 setCustomizationData(CustomizationData *customizationData);
	virtual void                 addCustomizationVariables(CustomizationData &customizationData) const;
	virtual void                 calculateSkinnedGeometryNow();
	virtual void                 setSkinningMode(SkinningMode skinningMode);
	virtual bool                 isReady() const;

	virtual bool                 collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const;

	virtual SoftwareBlendSkeletalShaderPrimitive       *asSoftwareBlendSkeletalShaderPrimitive();
	virtual SoftwareBlendSkeletalShaderPrimitive const *asSoftwareBlendSkeletalShaderPrimitive() const;

	virtual TextureRendererShaderPrimitive             *asTextureRendererShaderPrimitive();
	virtual TextureRendererShaderPrimitive const       *asTextureRendererShaderPrimitive() const;

private:

	struct TextureRendererInfo;

	typedef stdvector<TextureRendererInfo>::fwd  TextureRendererInfoVector;

private:

	static void remove();

private:

	TextureRendererShaderPrimitive();
	TextureRendererShaderPrimitive(const TextureRendererShaderPrimitive&);
	TextureRendererShaderPrimitive &operator =(const TextureRendererShaderPrimitive&);

private:

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;

private:

	ShaderPrimitive           *m_shaderPrimitive;
	TextureRendererInfoVector *m_textureRendererInfoVector;
};

// ======================================================================

inline ShaderPrimitive &TextureRendererShaderPrimitive::getRealShaderPrimitive()
{
	return *m_shaderPrimitive;
}

// ----------------------------------------------------------------------

inline const ShaderPrimitive &TextureRendererShaderPrimitive::getRealShaderPrimitive() const
{
	return *m_shaderPrimitive;
}

// ======================================================================

#endif
