//======================================================================
//
// NebulaVisualQuad.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/NebulaVisualQuad.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"

//======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(NebulaVisualQuad, true, 0, 0, 0);

//======================================================================

float NebulaVisualQuad::ms_cornerUvsForStyle[][8] =
{
	{/*NW*/ 0.0f, 0.0f, /*SW*/ 0.0f, 0.5f, /*SE*/ 0.5f, 0.5f, /*NE*/ 0.5f, 0.0f}, //NW

	{/*NW*/ 0.5f, 0.0f, /*SW*/ 0.5f, 0.5f, /*SE*/ 1.0f, 0.5f, /*NE*/ 1.0f, 0.0f}, //NE
	{/*NW*/ 0.5f, 0.5f, /*SW*/ 0.5f, 1.0f, /*SE*/ 1.0f, 1.0f, /*NE*/ 1.0f, 0.5f}, //SE
	{/*NW*/ 0.0f, 0.5f, /*SW*/ 0.0f, 1.0f, /*SE*/ 0.5f, 1.0f, /*NE*/ 0.5f, 0.5f}, //SW
};

int NebulaVisualQuad::ms_numStyles = 4;

//----------------------------------------------------------------------

NebulaVisualQuad::NebulaVisualQuad(Vector const & center, float radius, VectorArgb const * colors, int style, Vector const & rotation, bool const isOriented) :
m_sphere(center, radius),
m_rotation(rotation),
m_flags(isOriented ? static_cast<uint8>(F_oriented) : static_cast<uint8>(0)),
m_style(style),
m_sort(0.0f),
m_renderThisFrame(false),
m_renderTransform(),
m_renderAlpha(0.f),
m_renderShowFront(false)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_style, ms_numStyles);

	m_vertexColors[0] = colors[0];
	m_vertexColors[1] = colors[1];
	m_vertexColors[2] = colors[2];
	m_vertexColors[3] = colors[3];
}

//----------------------------------------------------------------------

NebulaVisualQuad::~NebulaVisualQuad()
{
}

//----------------------------------------------------------------------

Vector const & NebulaVisualQuad::getRotation() const
{
	return m_rotation;
}

//----------------------------------------------------------------------

bool NebulaVisualQuad::isOriented() const
{
	return (m_flags & F_oriented) != 0;
}

//----------------------------------------------------------------------

void NebulaVisualQuad::setSort(float const sort) const
{
	m_sort = sort;
}

//----------------------------------------------------------------------

bool NebulaVisualQuad::getRenderThisFrame() const
{
	return m_renderThisFrame;
}

//----------------------------------------------------------------------

void NebulaVisualQuad::setRenderThisFrame(bool const renderThisFrame) const
{
	m_renderThisFrame = renderThisFrame;
}

//----------------------------------------------------------------------

Transform const & NebulaVisualQuad::getRenderTransform() const
{
	return m_renderTransform;
}

//----------------------------------------------------------------------

void NebulaVisualQuad::setRenderTransform(Transform const & renderTransform) const
{
	m_renderTransform = renderTransform;
}

//----------------------------------------------------------------------

float NebulaVisualQuad::getRenderAlpha() const
{
	return m_renderAlpha;
}

//----------------------------------------------------------------------

void NebulaVisualQuad::setRenderAlpha(float const renderAlpha) const
{
	m_renderAlpha = renderAlpha;
}

//----------------------------------------------------------------------

bool NebulaVisualQuad::getRenderShowFront() const
{
	return m_renderShowFront;
}

//----------------------------------------------------------------------

void NebulaVisualQuad::setRenderShowFront(bool const renderShowFront) const
{
	m_renderShowFront = renderShowFront;
}

//======================================================================
