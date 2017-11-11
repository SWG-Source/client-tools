// ======================================================================
//
// ShipObject_MissileUpdate.cpp
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObject_MissileUpdate.h"

ShipObject::MissileUpdate::MissileUpdate() :
	m_missileTargetAcquisitionTimer(),
	m_acquired(false),
	m_acquiring(false),
	m_hasmissiles(false)
{
}
