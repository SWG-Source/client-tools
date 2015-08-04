// ======================================================================
//
// ShipObject_MissileUpdate.h
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipObject_MissileUpdate_H
#define INCLUDED_ShipObject_MissileUpdate_H

// ======================================================================

#include "clientGame/ShipObject.h"

// ======================================================================

struct ShipObject::MissileUpdate
{
	MissileUpdate();
	
	Timer m_missileTargetAcquisitionTimer;
	bool m_acquired;
	bool m_acquiring;
	bool m_hasmissiles;

};

// ======================================================================

#endif
