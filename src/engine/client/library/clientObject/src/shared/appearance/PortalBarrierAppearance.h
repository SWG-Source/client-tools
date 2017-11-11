// ======================================================================
//
// PortalBarrierAppearance.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_PortalBarrierAppearance_H
#define INCLUDED_PortalBarrierAppearance_H

#include "clientGraphics/SimpleAppearance.h"

class Vector;
class VectorArgb;

typedef stdvector<Vector>::fwd VertexList;

// ======================================================================

class PortalBarrierAppearance : public SimpleAppearance
{
public:

	PortalBarrierAppearance(VertexList const & verts, VectorArgb const & color);
	virtual ~PortalBarrierAppearance();

	void render() const;

protected:

	DPVS::Object * getDpvsObject() const;

private:

	DPVS::Object * m_dpvsObject;

private:
	// ----------

	PortalBarrierAppearance();
	PortalBarrierAppearance(const PortalBarrierAppearance&);
	PortalBarrierAppearance &operator=(const PortalBarrierAppearance&);
};

// ======================================================================

#endif
