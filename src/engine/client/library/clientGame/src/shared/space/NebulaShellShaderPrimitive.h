//======================================================================
//
// NebulaShellShaderPrimitive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaShellShaderPrimitive_H
#define INCLUDED_NebulaShellShaderPrimitive_H

//======================================================================

#include "clientGraphics/ShaderPrimitive.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Sphere.h"

class NebulaVisualQuad;
class NebulaVisualQuadShaderGroup;
class Shader;
class StaticShader;
class VertexBufferFormat;

//----------------------------------------------------------------------

class NebulaShellShaderPrimitive : public ShaderPrimitive
{
public:

	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef stdvector<NebulaVisualQuad const *>::fwd NebulaVisualQuadVector;

	static VertexBufferFormat getVertexBufferFormat();

	static void install();
	static void remove();

public:
	
	NebulaShellShaderPrimitive(NebulaVisualQuadShaderGroup const & shaderGroup, int quadBegin, int quadEnd, float depthSquaredSortKey, bool isOutsideFarPlane);
	virtual ~NebulaShellShaderPrimitive();
	
	virtual const Vector getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

	bool isOutsideFarPlane() const;
		
private:
	
	Shader const * m_shader;
	Sphere m_sphere;
	float m_depthSquaredSortKey;
	NebulaVisualQuadVector const & m_nebulaVisualQuadVector;
	int const m_quadBegin;
	int const m_quadEnd;
	bool m_isOutsideFarPlane;
	mutable bool m_canDraw;
	
private:
	
	NebulaShellShaderPrimitive();
	NebulaShellShaderPrimitive(const NebulaShellShaderPrimitive &);             //lint -esym(754, NebulaShellShaderPrimitive::NebulaShellShaderPrimitive) // not referenced // defensive hiding
	NebulaShellShaderPrimitive &operator =(const NebulaShellShaderPrimitive &);
};

//----------------------------------------------------------------------

inline bool NebulaShellShaderPrimitive::isOutsideFarPlane() const
{
	return m_isOutsideFarPlane;
}

//======================================================================

#endif
