// ======================================================================
//
// ShaderPrimitiveSet.h
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShaderPrimitiveSet_H
#define INCLUDED_ShaderPrimitiveSet_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedMath/Vector.h"

class Appearance;
class CollideParameters;
class CollisionInfo;
class CustomizationData;
class Iff;
class Shader;
class ShaderPrimitiveSetTemplate;
class ShaderTemplate;
class StaticIndexBuffer;
class StaticVertexBuffer;
class Texture;

// ======================================================================

class ShaderPrimitiveSet
{
	friend class ShaderPrimitiveSetTemplate;

public:

	static void install();

public:

	~ShaderPrimitiveSet();

	float alter(float elapsedTime);
	void add();
	void addWithAlphaFadeOpacity(bool opaqueEnabled, float opaqueOpacity, bool alphaEnabled, float alphaOpacity);

	void setCustomizationData(CustomizationData *customizationData);
	void addCustomizationVariables(CustomizationData &customizationData);

	int                       getNumberOfShaders () const;
	const Shader*             getShader (int shaderIndex) const;
	const ShaderTemplate*     getShaderTemplate (int shaderIndex) const;
	const StaticVertexBuffer* getVertexBuffer (int shaderIndex) const;
	const StaticIndexBuffer*  getIndexBuffer (int shaderIndex) const;

	void                      getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const;

	void                      setTexture(Tag tag, const Texture &texture);

	bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;

private:

	ShaderPrimitiveSet(const Appearance &owner, const ShaderPrimitiveSetTemplate &shaderPrimitiveSetTemplate);

public:
	class LocalShaderPrimitive;

private:
	typedef stdvector<LocalShaderPrimitive *>::fwd Primitives;

private:

	Primitives 	     *m_primitives;
	Vector            m_localCameraPos;

private:

	// Disabled
	ShaderPrimitiveSet();
	ShaderPrimitiveSet(const ShaderPrimitiveSet &);
	ShaderPrimitiveSet &operator =(const ShaderPrimitiveSet &);
};

// ======================================================================

#endif
