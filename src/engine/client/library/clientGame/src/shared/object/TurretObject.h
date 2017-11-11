//===================================================================
//
// TurretObject.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TurretObject_H
#define INCLUDED_TurretObject_H

//===================================================================

#include "clientGame/Projectile.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

class ClientWeaponObjectTemplate;
class Projectile;

//===================================================================

class TurretObject : public Object
{
	friend class TurretController;

public:

	static void install ();

public:

	explicit TurretObject (float yawMaximumRadiansPerSecond);
	virtual ~TurretObject ();

	virtual float alter (float elapsedTime);

	void          setBarrel (Object* object, float pitchMinimumRadians, float pitchMaximumRadians, float pitchMaximumRadiansPerSecond);
	void          setWeapon (const ClientWeaponObjectTemplate* weaponObjectTemplate, const Transform& muzzleTransform_o2p, const float speed, const float expirationTime);
	const Object* fire (const Object* const target, const bool hit);

public:

	static void remove ();

private:

	TurretObject ();
	TurretObject (const TurretObject&);
	TurretObject& operator= (const TurretObject&);

private:

	float                             m_yawMaximumRadiansPerSecond;
	Object*                           m_barrel;
	float                             m_pitchMinimumRadians;
	float                             m_pitchMaximumRadians;
	float                             m_pitchMaximumRadiansPerSecond;
	const ClientWeaponObjectTemplate* m_weaponObjectTemplate;
	Transform                         m_muzzleTransform_o2p;
	float                             m_speed;
	float                             m_expirationTime;
	Watcher<Projectile>               m_projectile;
	ConstWatcher<Object>              m_target;
	Timer                             m_debugFireTimer;
};

//===================================================================

#endif
