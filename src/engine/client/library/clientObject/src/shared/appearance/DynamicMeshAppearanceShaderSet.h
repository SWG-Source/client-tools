//======================================================================
//
// DynamicMeshAppearanceShaderSet.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DynamicMeshAppearanceShaderSet_H
#define INCLUDED_DynamicMeshAppearanceShaderSet_H

#include "sharedMath/Vector.h"

class AxialBox;
class Plane;
class Shader;
class Sphere;
class StaticIndexBuffer;
class StaticVertexBuffer;
class Vector;

//======================================================================

class DynamicMeshAppearanceShaderSetEdge
{
public:
	Vector v[2];
};

//----------------------------------------------------------------------

class DynamicMeshAppearanceShaderSetIndexedEdge
{
public:
	uint16 v[2];
};

//----------------------------------------------------------------------

class DynamicMeshAppearanceShaderSet
{
public:
	
	typedef DynamicMeshAppearanceShaderSetEdge Edge;
	typedef DynamicMeshAppearanceShaderSetIndexedEdge IndexedEdge;
	
	typedef stdvector<Edge>::fwd EdgeVector;
	typedef stdvector<uint16>::fwd IndexVector;

public:

	static void split(Vector const & randomization, Plane const & plane, Shader const & shader, StaticVertexBuffer const & inputVertexBuffer, StaticIndexBuffer const & inputIndexBuffer, DynamicMeshAppearanceShaderSet *& outputFront, DynamicMeshAppearanceShaderSet *& outputBack, EdgeVector & edges);

public:

	DynamicMeshAppearanceShaderSet();
	~DynamicMeshAppearanceShaderSet();

	DynamicMeshAppearanceShaderSet & set(DynamicMeshAppearanceShaderSet const & rhs);

	Shader const * getShader() const;
	StaticVertexBuffer * getVertexBuffer() const;
	StaticIndexBuffer * getIndexBuffer() const;

	AxialBox calculateAxialBox() const;
	
private:

	DynamicMeshAppearanceShaderSet(DynamicMeshAppearanceShaderSet const & shaderSet);
	DynamicMeshAppearanceShaderSet & operator=(DynamicMeshAppearanceShaderSet const & rhs);

	DynamicMeshAppearanceShaderSet(Shader const & shader, StaticVertexBuffer const & vertexBuffer, StaticIndexBuffer const & indexBuffer);
	DynamicMeshAppearanceShaderSet(Shader const & shader, StaticVertexBuffer const & vertexBuffer, IndexVector const & indexVector);

	void set(Shader const & shader, StaticVertexBuffer const & vertexBuffer, IndexVector const & indexVector);

private:

	Shader const * m_shader;
	StaticVertexBuffer * m_vertexBuffer;
	StaticIndexBuffer * m_indexBuffer;

};

//----------------------------------------------------------------------

inline Shader const * DynamicMeshAppearanceShaderSet::getShader() const
{
	return m_shader;
}

//----------------------------------------------------------------------

inline StaticVertexBuffer * DynamicMeshAppearanceShaderSet::getVertexBuffer() const
{
	return m_vertexBuffer;
}

//----------------------------------------------------------------------

inline StaticIndexBuffer * DynamicMeshAppearanceShaderSet::getIndexBuffer() const
{
	return m_indexBuffer;
}

#endif

//----------------------------------------------------------------------
