//======================================================================
//
// NebulaVisualQuad.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaVisualQuad_H
#define INCLUDED_NebulaVisualQuad_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"

//----------------------------------------------------------------------

/**
* All grouped NebulaVisualQuads are assumed to use the same shader and style
*/

class NebulaVisualQuad
{
public:

	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	NebulaVisualQuad(Vector const & center, float radius, VectorArgb const * colors, int style, Vector const & rotation, bool isOriented);
	~NebulaVisualQuad();

	Sphere const & getSphere() const;
	Vector const & getRotation() const;
	bool isOriented() const;
	VectorArgb const * getVertexColors() const;
	void setSort(float sort) const;
	float getSort() const;

	enum Flag
	{
		F_oriented = 0x0001,
	};
 
	float const * getCornerUvsForStyle() const;

	bool operator<(NebulaVisualQuad const & rhs) const;

	//-- Used during render
	bool getRenderThisFrame() const;
	void setRenderThisFrame(bool renderThisFrame) const;
	Transform const & getRenderTransform() const;
	void setRenderTransform(Transform const & renderTransform) const;
	float getRenderAlpha() const;
	void setRenderAlpha(float alpha) const;
	bool getRenderShowFront() const;
	void setRenderShowFront(bool renderShowFront) const;

private:

	NebulaVisualQuad();
	NebulaVisualQuad(NebulaVisualQuad const &);

private:

	Sphere m_sphere;
	Vector m_rotation;
	uint8 m_flags;	
	VectorArgb m_vertexColors[4];
	int m_style;
	mutable float m_sort;
	mutable bool m_renderThisFrame;
	mutable Transform m_renderTransform;
	mutable float m_renderAlpha;
	mutable bool m_renderShowFront;

	static float ms_cornerUvsForStyle[][8];
	static int ms_numStyles;
};

//----------------------------------------------------------------------

inline Sphere const & NebulaVisualQuad::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

inline float const * NebulaVisualQuad::getCornerUvsForStyle() const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_style, ms_numStyles);
	return (ms_cornerUvsForStyle[m_style]);
	static float localUvs[8];	
}

//----------------------------------------------------------------------

inline VectorArgb const * NebulaVisualQuad::getVertexColors() const
{
	return m_vertexColors;
}

//----------------------------------------------------------------------

inline bool NebulaVisualQuad::operator<(NebulaVisualQuad const & rhs) const
{
	return m_sort < rhs.m_sort;
}

//----------------------------------------------------------------------

inline float NebulaVisualQuad::getSort() const
{
	return m_sort;
}

//======================================================================

#endif
