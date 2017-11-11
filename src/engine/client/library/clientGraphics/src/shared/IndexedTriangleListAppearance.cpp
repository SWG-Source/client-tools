// ======================================================================
//
// IndexedTriangleListAppearance.cpp
// asommers
//
// copyright 2001, sony online entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/IndexedTriangleListAppearance.h"

#include "clientGraphics/IndexedTriangleListShaderPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedMath/IndexedTriangleList.h"

#include "dpvsObject.hpp"

#include <vector>

// ======================================================================

IndexedTriangleListAppearance::IndexedTriangleListAppearance(const IndexedTriangleList *indexedTriangleList, const VectorArgb& color) 
: Appearance(0),
  m_indexedTriangleListShaderPrimitive (0),
	m_dpvsObject(0)
{
	NOT_NULL (indexedTriangleList);
	IndexedTriangleList * clone = indexedTriangleList->clone ();

	//-- build an extent
	BoxExtent* const extent = new BoxExtent ();

	uint i;
	for (i = 0; i < clone->getVertices ().size (); ++i)
		extent->updateMinAndMax (clone->getVertices () [i]);

	extent->calculateCenterAndRadius ();

	setExtent (ExtentList::fetch (extent));

	m_indexedTriangleListShaderPrimitive = new IndexedTriangleListShaderPrimitive (*this, clone, true, color);

	m_dpvsObject = RenderWorld::createObject(this, extent->getBox());
}

// ----------------------------------------------------------------------

IndexedTriangleListAppearance::~IndexedTriangleListAppearance()
{
	delete m_indexedTriangleListShaderPrimitive;
	m_indexedTriangleListShaderPrimitive = 0;
	m_dpvsObject->release();
	m_dpvsObject = 0;
}

// ----------------------------------------------------------------------

DPVS::Object *IndexedTriangleListAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void IndexedTriangleListAppearance::render() const
{
	ShaderPrimitiveSorter::add (*m_indexedTriangleListShaderPrimitive);
}

// ======================================================================

