// ======================================================================
//
// ShipWeaponBarrelDynamics.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipWeaponBarrelDynamics.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

// ======================================================================

ShipWeaponBarrelDynamics::ShipWeaponBarrelDynamics(Object *owner, Transform const &defaultTransform, float maxRecoilDistance, float recoilRate, float recoilReturnRate) :
	Dynamics(owner),
	m_defaultTransform(defaultTransform),
	m_maxRecoilDistance(maxRecoilDistance),
	m_recoilRate(recoilRate),
	m_recoilReturnRate(recoilReturnRate),
	m_active(false),
	m_recoilDistance(0.f)
{
}

// ----------------------------------------------------------------------

float ShipWeaponBarrelDynamics::alter(float time)
{
	float result = Dynamics::alter(time);

	bool const changed = m_active || m_recoilDistance > 0.f;

	// recoil
	if (m_active)
	{
		float recoilEndTime = (m_maxRecoilDistance-m_recoilDistance)/m_recoilRate;
		if (time < recoilEndTime)
			m_recoilDistance += time * m_recoilRate;
		else
		{
			// remaining time applied to recoil return
			m_active = false;
			time -= recoilEndTime;
			m_recoilDistance = m_maxRecoilDistance;
		}
	}

	// return from recoil
	if (!m_active && m_recoilDistance > 0.f)
		m_recoilDistance = std::max(0.f, m_recoilDistance - time * m_recoilReturnRate);

	if (changed)
	{
		Transform tr(m_defaultTransform);
		tr.move_l(Vector::unitZ * (-m_recoilDistance));
		getOwner()->setTransform_o2p(tr);
	}

	AlterResult::incorporateAlterResult(result, AlterResult::cms_alterNextFrame);
	return result;
}

// ----------------------------------------------------------------------

void ShipWeaponBarrelDynamics::onFired()
{
	m_active = true;
}

// ======================================================================
