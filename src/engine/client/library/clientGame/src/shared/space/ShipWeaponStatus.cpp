// ======================================================================
//
// ShipWeaponStatus.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipWeaponStatus.h"

// ======================================================================

ShipWeaponStatus::ShipWeaponStatus() :
	m_playerControlled(false),
	m_turretTarget(),
	m_refireTimer(),
	m_isTurretOrientationSet(false),
	m_turretYaw(0.f),
	m_turretPitch(0.f),
	m_isFiring(false),
	m_lookAtTarget( Vector::unitZ ),
	m_previousYaw(0.0f),
	m_previousPitch(0.0f)
{
}

// ----------------------------------------------------------------------

ShipWeaponStatus::ShipWeaponStatus(ShipWeaponStatus const &rhs) :
	m_playerControlled(rhs.m_playerControlled),
	m_turretTarget(rhs.m_turretTarget),
	m_refireTimer(rhs.m_refireTimer),
	m_isTurretOrientationSet(rhs.m_isTurretOrientationSet),
	m_turretYaw(rhs.m_turretYaw),
	m_turretPitch(rhs.m_turretPitch),
	m_isFiring(rhs.m_isFiring),
	m_lookAtTarget(rhs.m_lookAtTarget),
	m_previousPitch(rhs.m_previousPitch),
	m_previousYaw(rhs.m_previousYaw)
{
}

// ======================================================================

