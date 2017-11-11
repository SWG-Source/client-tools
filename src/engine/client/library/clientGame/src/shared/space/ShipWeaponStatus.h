// ======================================================================
//
// ShipWeaponStatus.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipWeaponStatus_H
#define INCLUDED_ShipWeaponStatus_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedMath/Vector.h"
// ======================================================================

class ShipWeaponStatus
{
public:
	ShipWeaponStatus();
	ShipWeaponStatus(ShipWeaponStatus const &);

	bool m_playerControlled;
	CachedNetworkId m_turretTarget;
	Timer m_refireTimer;
	bool m_isTurretOrientationSet;
	float m_turretYaw;
	float m_turretPitch;
	bool m_isFiring;
	Vector m_lookAtTarget;
	float m_previousYaw;
	float m_previousPitch;
};

// ======================================================================

#endif // INCLUDED_ShipWeaponStatus_H

