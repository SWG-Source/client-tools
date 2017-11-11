//===================================================================
//
// NonTrackingProjectile.h
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#ifndef INCLUDED_NonTrackingProjectile_H
#define INCLUDED_NonTrackingProjectile_H

//===================================================================

class MemoryBlockManager;

//===================================================================

#include "clientGame/Projectile.h"
#include "clientGame/WeaponObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"

//===================================================================

/** This class represents a basic NonTrackingProjectile.  It has a begin and end position, a speed, and an expiration time.  When the timer expires,
 *  the object is deleted.  The speed and direction of the NonTrackingProjectile can be modified by adjusting the end position and speed
 */
class NonTrackingProjectile : public Projectile
{
public:

	static void  install();
	static void *operator new(size_t size);
	static void  operator delete(void* pointer);

public:

	NonTrackingProjectile(const AppearanceTemplate* appearanceTemplate, WeaponObject &weaponObject, CrcLowerString const &emitterHardpoint);
	virtual ~NonTrackingProjectile();

	virtual float alter(float time);

	void setExcludeObject(Object const * excludeObject);
	void setTravelDistance(float travelDistance);

	void setWantFlyByEffect(bool wantFlyByEffect);
	void setAttacker(Object* attacker);

private:

	NonTrackingProjectile();
	NonTrackingProjectile(const NonTrackingProjectile& rhs);
	NonTrackingProjectile& operator=(const NonTrackingProjectile& rhs);

private:

	bool                        m_isFirstAlter;
	bool                        m_wantFlyByEffect;
	HitType                     m_hitType;
	CellProperty*               m_hitCellProperty;
	ConstWatcher<Object>        m_hitCellObject;
	Vector                      m_hitPoint_w;
	Vector                      m_hitNormal_w;
	float                       m_travelDistance;
	ConstWatcher<Object>        m_excludeObject;

	Watcher<WeaponObject>       m_weaponWatcher;
	std::string                 m_weaponObjectTemplateName;
	WeaponObject::WeaponType    m_weaponType;
	WeaponObject::DamageType    m_damageType;
	WeaponObject::DamageType    m_elementalDamageType;
	CrcLowerString              m_weaponHardpoint;

	float                       m_normalizedBeamLength;
	Watcher<Object>             m_attackerWatcher;
};

//------------------------------------------------------------------


inline void NonTrackingProjectile::setWantFlyByEffect(bool wantFlyByEffect)
{
	m_wantFlyByEffect = wantFlyByEffect;
}


//===================================================================

#endif
