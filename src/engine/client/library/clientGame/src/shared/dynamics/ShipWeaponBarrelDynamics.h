// ======================================================================
//
// ShipWeaponBarrelDynamics.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipWeaponBarrelDynamics_H
#define INCLUDED_ShipWeaponBarrelDynamics_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedObject/Dynamics.h"

// ======================================================================

class ShipWeaponBarrelDynamics: public Dynamics
{
public:
	ShipWeaponBarrelDynamics(Object *owner, Transform const &defaultTransform, float maxRecoilDistance, float recoilRate, float recoilReturnRate);

	virtual float alter(float time);

	void onFired();

private:
	ShipWeaponBarrelDynamics();
	ShipWeaponBarrelDynamics(ShipWeaponBarrelDynamics const &);
	ShipWeaponBarrelDynamics& operator=(ShipWeaponBarrelDynamics const &);

private:
	Transform m_defaultTransform;
	float m_maxRecoilDistance;
	float m_recoilRate;
	float m_recoilReturnRate;
	bool m_active;
	float m_recoilDistance;
};

// ======================================================================

#endif // INCLUDED_ShipWeaponBarrelDynamics_H
