//===================================================================
//
// TrackingProjectile.h
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#ifndef INCLUDED_TrackingProjectile_H
#define INCLUDED_TrackingProjectile_H

//===================================================================

class AppearanceTemplate;
class ClientEffectTemplate;
class MemoryBlockManager;
class WeaponObject;

//===================================================================

#include "clientGame/Projectile.h"
#include "clientGame/WeaponObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"

//===================================================================

/** This class represents a basic TrackingProjectile.  It has a begin and end position, a speed, and an expiration time.  When the timer expires,
 *  the object is deleted.  The speed and direction of the TrackingProjectile can be modified by adjusting the end position and speed
 */
class TrackingProjectile : public Projectile
{
public:

	static void install();
	static void *operator new(size_t size);
	static void operator delete(void* pointer);

public:

	TrackingProjectile(const AppearanceTemplate* appearanceTemplate, WeaponObject &weaponObject, CrcLowerString const &emitterHardpoint, bool useRicochet);
	virtual ~TrackingProjectile();
	
	virtual float alter(float time);

	virtual void setTargetHardpoint(const CrcLowerString &hardpoint);
	void         setAttacker(Object* attacker);

	void setWantFlyByEffect(bool wantFlyByEffect);
	void setActionNameCrc(int actionNameCrc);

private:

	TrackingProjectile();
	TrackingProjectile(const TrackingProjectile& rhs);
	TrackingProjectile& operator=(const TrackingProjectile& rhs);

private:

	bool                        m_isFirstAlter;
	bool                        m_wantFlyByEffect;

	Watcher<WeaponObject>       m_weaponWatcher;
	std::string                 m_weaponObjectTemplateName;
	WeaponObject::WeaponType    m_weaponType;
	WeaponObject::DamageType    m_damageType;
	WeaponObject::DamageType    m_elementalDamageType;
	CrcLowerString              m_weaponHardpoint;

	bool                        m_useRicochet;
	Vector                      m_initialDirection;
	CrcLowerString              m_targetHardpoint;
	int                         m_actionNameCrc;
	float                       m_normalizedBeamLength;
	Watcher<Object>             m_attackerWatcher;
};

//------------------------------------------------------------------

inline void TrackingProjectile::setWantFlyByEffect(bool wantFlyByEffect)
{
	m_wantFlyByEffect = wantFlyByEffect;
}

//------------------------------------------------------------------

inline void TrackingProjectile::setActionNameCrc(int actionNameCrc)
{
	m_actionNameCrc = actionNameCrc;
}

//===================================================================

#endif
