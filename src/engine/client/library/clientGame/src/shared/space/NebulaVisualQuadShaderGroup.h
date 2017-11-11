//======================================================================
//
// NebulaVisualQuadShaderGroup.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaVisualQuadShaderGroup_H
#define INCLUDED_NebulaVisualQuadShaderGroup_H

//======================================================================

#include "clientGame/NebulaVisualQuad.h"
#include "sharedMath/Sphere.h"
#include <vector>

class Shader;

//----------------------------------------------------------------------

class NebulaVisualQuadShaderGroup
{
public:

	typedef stdvector<NebulaVisualQuad const *>::fwd NebulaVisualQuadVector;

	NebulaVisualQuadShaderGroup(Shader const & shader, int reserveNumQuads, bool ownsQuads);
	~NebulaVisualQuadShaderGroup();

	Shader const & getShader() const;

	Sphere const & getSphere() const;

	void addDataFromShaderGroup(NebulaVisualQuadShaderGroup const & rhs);

	NebulaVisualQuadVector const & getNebulaVisualQuads() const;

	void addQuad(NebulaVisualQuad const * quad);

	void sortQuads(Vector const & cameraPosition_w);

	bool hasQuadWithinRangeSquared(Vector const & point, float rangeSquared) const;

private:

	void computeSphere() const;

	Shader const * m_shader;
	NebulaVisualQuadVector m_nebulaVisualQuadVector;
	bool const m_ownsQuads;
	mutable Sphere m_sphere;
	mutable bool m_sphereDirty;

private:

	NebulaVisualQuadShaderGroup(NebulaVisualQuadShaderGroup const &);
	NebulaVisualQuadShaderGroup & operator=(NebulaVisualQuadShaderGroup const &);
};

//----------------------------------------------------------------------

inline Shader const & NebulaVisualQuadShaderGroup::getShader() const
{
	return *NON_NULL(m_shader);
}

//----------------------------------------------------------------------

inline Sphere const & NebulaVisualQuadShaderGroup::getSphere() const
{
	if (m_sphereDirty)
		computeSphere();

	return m_sphere;
}

//----------------------------------------------------------------------

inline NebulaVisualQuadShaderGroup::NebulaVisualQuadVector const & NebulaVisualQuadShaderGroup::getNebulaVisualQuads() const
{
	return m_nebulaVisualQuadVector;
}

//======================================================================

#endif
