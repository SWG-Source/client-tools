//===================================================================
//
// TurretObject.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TurretObject.h"

#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/Game.h"
#include "clientGame/Projectile.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/AlterResult.h"

//===================================================================
// TurretObjectNamespace
//===================================================================

namespace TurretObjectNamespace
{
	bool ms_debugFire;
}

using namespace TurretObjectNamespace;

//===================================================================
// STATIC PUBLIC TurretObject
//===================================================================

void TurretObject::install ()
{
	InstallTimer const installTimer("TurretObject::install");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_debugFire, "ClientGame/TurretObject", "debugFire");
#endif

	ExitChain::add (remove, "TurretObject::remove");
}

//-------------------------------------------------------------------

void TurretObject::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_debugFire);
#endif
}

//===================================================================
// PUBLIC TurretObject
//===================================================================

TurretObject::TurretObject (const float yawMaximumRadiansPerSecond) :
	Object (),
	m_yawMaximumRadiansPerSecond (yawMaximumRadiansPerSecond),
	m_barrel (0),
	m_pitchMinimumRadians (0.f),
	m_pitchMaximumRadians (0.f),
	m_pitchMaximumRadiansPerSecond (0.f),
	m_weaponObjectTemplate (0),
	m_muzzleTransform_o2p (),
	m_speed (0.f),
	m_expirationTime (0.f),
	m_projectile (0),
	m_target (0),
	m_debugFireTimer (2.f)
{
}

//-------------------------------------------------------------------

TurretObject::~TurretObject ()
{
	delete m_barrel;
	m_barrel = 0;

	if (m_weaponObjectTemplate)
	{
		m_weaponObjectTemplate->releaseReference ();
		m_weaponObjectTemplate = 0;
	}

	m_projectile = 0;
}

//-------------------------------------------------------------------

void TurretObject::setBarrel (Object* const object, const float pitchMinimumRadians, const float pitchMaximumRadians, const float pitchMaximumRadiansPerSecond)
{
	if (m_barrel)
		delete m_barrel;

	m_barrel = object;
	m_pitchMinimumRadians = pitchMinimumRadians;
	m_pitchMaximumRadians = pitchMaximumRadians;
	m_pitchMaximumRadiansPerSecond = pitchMaximumRadiansPerSecond;
}

//-------------------------------------------------------------------

void TurretObject::setWeapon (const ClientWeaponObjectTemplate* const weaponObjectTemplate, const Transform& muzzleTransform_o2p, const float speed, const float expirationTime)
{
	m_weaponObjectTemplate = weaponObjectTemplate;
	if (m_weaponObjectTemplate)
		m_weaponObjectTemplate->addReference ();

	m_muzzleTransform_o2p = muzzleTransform_o2p;
	m_speed = speed;
	m_expirationTime = expirationTime;
}

//-------------------------------------------------------------------

const Object* TurretObject::fire (const Object* const target, const bool hit)
{
	m_target = target;

	if (m_projectile)
	{
		delete m_projectile;
		m_projectile = 0;
	}

	if (m_weaponObjectTemplate)
		m_projectile = m_weaponObjectTemplate->createProjectile (!hit, false);

	return m_projectile;
}

//-------------------------------------------------------------------

float TurretObject::alter (float elapsedTime)
{
	const float result = Object::alter (elapsedTime);
	if (result == AlterResult::cms_kill) //lint !e777 // Testing floats for equality // It's okay, we're using constants.
		return AlterResult::cms_kill;

	if (m_projectile)
	{
		if (!m_target)
		{
			delete m_projectile;
			m_projectile = 0;
		}
		else
		{
			const Vector targetPosition_w = m_target->getAppearance () ? m_target->getPosition_w () + Vector::unitY * m_target->getAppearanceSphereRadius () : m_target->getPosition_w ();

			//-- turret face target
			{
				const Vector facing_o = rotateTranslate_w2o (targetPosition_w);
				if (facing_o != Vector::zero)
				{
					const float maximumYawThisFrame = m_yawMaximumRadiansPerSecond * elapsedTime;
					const float yaw = clamp (-maximumYawThisFrame, facing_o.theta (), maximumYawThisFrame);
					yaw_o (yaw);
				}
			}

			//-- barrel face target
			{
				if (m_barrel)
				{
					//-- barrel face target
					{
						const Vector facing_o = m_barrel->rotateTranslate_w2o (targetPosition_w);
						if (facing_o != Vector::zero)
						{
							const float maximumPitchThisFrame = m_pitchMaximumRadiansPerSecond * elapsedTime;
							const float pitch = clamp (-maximumPitchThisFrame, facing_o.phi (), maximumPitchThisFrame);
							m_barrel->pitch_o (pitch);
						}
					}

					//-- are we ready to fire?
					{
						bool shouldFire = false;

						Vector facing_o = m_barrel->rotateTranslate_w2o (targetPosition_w);
						if (facing_o == Vector::zero)
							shouldFire = true;
						else
						{
							if (!facing_o.normalize ())
								shouldFire = true;
							else
								if (facing_o.dot (Vector::unitZ) > 0.99f)
									shouldFire = true;
						}

						if (shouldFire)
						{
							Transform muzzleHardpoint;
							muzzleHardpoint.multiply (m_barrel->getTransform_o2w (), m_muzzleTransform_o2p);

							m_projectile->setExpirationTime (m_expirationTime);
							m_projectile->setFacing (getParentCell (), muzzleHardpoint.getPosition_p (), m_target->getPosition_w ());
							m_projectile->setSpeed (m_speed);
							m_projectile->setTarget (m_target);
							m_projectile->addToWorld ();
							m_projectile = 0;
						}
					}
				}
			}
		}
	}
	else
	{
#ifdef _DEBUG
		if (ms_debugFire)
		{
			if (m_debugFireTimer.updateZero (elapsedTime))
				fire (Game::getPlayer (), Random::random (1) == 0);
		}
#endif
	}

	return AlterResult::cms_alterNextFrame;
}

//===================================================================

