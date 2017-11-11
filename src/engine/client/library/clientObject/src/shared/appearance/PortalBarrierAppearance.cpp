// ======================================================================
//
// PortalBarrierAppearance.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/PortalBarrierAppearance.h"

#include "clientGraphics/SimplePolyPrimitive.h"
#include "clientGraphics/RenderWorld.h"

#include "sharedCollision/BarrierObject.h"
#include "sharedMath/AxialBox.h"

#include "dpvsObject.hpp"

// ======================================================================

PortalBarrierAppearance::PortalBarrierAppearance(VertexList const & verts, const VectorArgb &newColor)
{
	attachPrimitive( new SimplePolyPrimitive(*this, verts, newColor) );

	AxialBox box;
	box.add(verts);
	m_dpvsObject = RenderWorld::createObject(this, box);
}

// ----------------------------------------------------------------------

PortalBarrierAppearance::~PortalBarrierAppearance()
{
	m_dpvsObject->release();
}

// ----------------------------------------------------------------------

DPVS::Object * PortalBarrierAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void PortalBarrierAppearance::render() const
{
	BarrierObject const * barrier = safe_cast<BarrierObject const *>(getOwner());

	if(barrier && barrier->isActive())
	{
		SimpleAppearance::render();
	}
}

// ======================================================================
