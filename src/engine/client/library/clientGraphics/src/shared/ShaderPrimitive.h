// ======================================================================
//
// ShaderPrimitive.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderPrimitive_H
#define INCLUDED_ShaderPrimitive_H

// ======================================================================

#include "sharedMath/Vector.h"

class CollisionInfo;
class CustomizationData;
class SoftwareBlendSkeletalShaderPrimitive;
class StaticShader;
class TextureRendererShaderPrimitive;

// ======================================================================

class ShaderPrimitive
{
public:

	enum SkinningMode
	{
		SM_noSkinning,
		SM_hardSkinning,
		SM_softSkinning
	};

public:

	ShaderPrimitive();
	virtual ~ShaderPrimitive();

	virtual void                getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const;
	virtual float               alter(float time);
	virtual const StaticShader &prepareToView() const = 0;
	virtual const Vector        getPosition_w() const = 0;
	virtual float               getRadius() const;
	virtual float               getDepthSquaredSortKey() const = 0;
	virtual int                 getVertexBufferSortKey() const = 0;
	virtual void                prepareToDraw() const = 0;
	virtual void                draw() const = 0;
	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;
	virtual void                calculateSkinnedGeometryNow();
	virtual void                setSkinningMode(SkinningMode skinningMode);
	virtual bool                isReady() const;

	virtual bool                collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const;

	virtual SoftwareBlendSkeletalShaderPrimitive       *asSoftwareBlendSkeletalShaderPrimitive();
	virtual SoftwareBlendSkeletalShaderPrimitive const *asSoftwareBlendSkeletalShaderPrimitive() const;

	virtual TextureRendererShaderPrimitive             *asTextureRendererShaderPrimitive();
	virtual TextureRendererShaderPrimitive const       *asTextureRendererShaderPrimitive() const;

protected:

	float m_radius;
};

//----------------------------------------------------------------------

inline float ShaderPrimitive::getRadius() const
{
	return m_radius;
}

// ======================================================================

#endif
