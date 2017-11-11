//======================================================================
//
// NebulaVisualQuadShaderGroup.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/NebulaVisualQuadShaderGroup.h"

#include "clientGraphics/Shader.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Vector.h"

#include <algorithm>

//======================================================================

NebulaVisualQuadShaderGroup::NebulaVisualQuadShaderGroup(Shader const & shader, int const reserveNumQuads, bool const ownsQuads) :
m_shader(&shader),
m_nebulaVisualQuadVector(),
m_ownsQuads(ownsQuads),
m_sphere(),
m_sphereDirty(true)
{
	m_shader->fetch();
	m_nebulaVisualQuadVector.reserve(reserveNumQuads);
}


//----------------------------------------------------------------------

NebulaVisualQuadShaderGroup::~NebulaVisualQuadShaderGroup()
{
	m_shader->release();
	m_shader = NULL;

	if (m_ownsQuads)
		std::for_each(m_nebulaVisualQuadVector.begin(), m_nebulaVisualQuadVector.end(), PointerDeleter());

	m_nebulaVisualQuadVector.clear();
}

//----------------------------------------------------------------------

void NebulaVisualQuadShaderGroup::addDataFromShaderGroup(NebulaVisualQuadShaderGroup const & rhs)
{
	m_nebulaVisualQuadVector.reserve(m_nebulaVisualQuadVector.size() + rhs.m_nebulaVisualQuadVector.size());
	m_nebulaVisualQuadVector.insert(m_nebulaVisualQuadVector.end(), rhs.m_nebulaVisualQuadVector.begin(), rhs.m_nebulaVisualQuadVector.end());
}

//----------------------------------------------------------------------

void NebulaVisualQuadShaderGroup::computeSphere() const
{
	AxialBox box;

	for (NebulaVisualQuadVector::const_iterator it = m_nebulaVisualQuadVector.begin(); it != m_nebulaVisualQuadVector.end(); ++it)
	{
		NebulaVisualQuad const & quad = **it;

		Sphere const & sphere = quad.getSphere();

		Vector const & offset = Vector::xyz111 * sphere.getRadius();

		box.add(sphere.getCenter() - offset);
		box.add(sphere.getCenter() + offset);
	}

	m_sphere.set(box.getCenter(), box.getRadius());

	m_sphereDirty = false;
}

//----------------------------------------------------------------------

void NebulaVisualQuadShaderGroup::addQuad(NebulaVisualQuad const * const quad)
{
	m_nebulaVisualQuadVector.push_back(quad);
}

//----------------------------------------------------------------------

void NebulaVisualQuadShaderGroup::sortQuads(Vector const & cameraPosition_w)
{
	if (m_nebulaVisualQuadVector.empty())
		return;

	for (NebulaVisualQuadVector::iterator it = m_nebulaVisualQuadVector.begin(); it != m_nebulaVisualQuadVector.end(); ++it)
	{
		NebulaVisualQuad const & quad = **it;
		quad.setSort(-cameraPosition_w.magnitudeBetweenSquared(quad.getSphere().getCenter()));
	}

	std::sort(m_nebulaVisualQuadVector.begin(), m_nebulaVisualQuadVector.end(), LessPointerComparator());
}

//----------------------------------------------------------------------

bool NebulaVisualQuadShaderGroup::hasQuadWithinRangeSquared(Vector const & point, float const rangeSquared) const
{
	for (NebulaVisualQuadVector::const_iterator f_it = m_nebulaVisualQuadVector.begin(); f_it != m_nebulaVisualQuadVector.end(); ++f_it)
	{
		NebulaVisualQuad const & quad = **f_it;
		if (quad.getSphere().getCenter().magnitudeBetweenSquared(point) < rangeSquared)
			return true;
	}

	return false;
}

//======================================================================
