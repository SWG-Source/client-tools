//===================================================================
//
// TrackingProjectile.cpp
//
// Copyright 2000, 2001 Verant Interactive, Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TrackingProjectile.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ClientWorld.h"	
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientParticle/LightningAppearance.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/SimpleDynamics.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"



//===================================================================

namespace TrackingProjectileNamespace
{
	void remove();

	MemoryBlockManager* ms_memoryBlockManager;

	const float ms_beamLifetime = 1.5f;          // min duration for a "beam" projectile in seconds
	const float ms_beamTimeToReachTargetMultiplier = 1.f / 0.75f; // 1 / seconds for beam to reach target - used to simulate the beam traveling toward the target
	
	float ms_flyByRange = 0.f;
}

using namespace TrackingProjectileNamespace;

//===================================================================

void TrackingProjectile::install()
{
	InstallTimer const installTimer("TrackingProjectile::install");

	ms_memoryBlockManager = new MemoryBlockManager("TrackingProjectile::ms_memoryBlockManager", true, sizeof(TrackingProjectile), 0, 0, 0);

	ms_flyByRange = ConfigClientGame::getProjectileFlyByRange();

	ExitChain::add(TrackingProjectileNamespace::remove, "TrackingProjectile::remove");
}

//-------------------------------------------------------------------

void TrackingProjectileNamespace::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("TrackingProjectile is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void *TrackingProjectile::operator new(size_t size)
{
	UNREF(size);

	DEBUG_FATAL(!ms_memoryBlockManager,("TrackingProjectile is not installed"));

	// do not try to alloc a descendant class with this allocator
	DEBUG_FATAL(size != sizeof(TrackingProjectile),("bad size"));

	return ms_memoryBlockManager->allocate();
}

//-------------------------------------------------------------------

void TrackingProjectile::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("TrackingProjectile is not installed"));
	ms_memoryBlockManager->free(pointer);
}

//===================================================================

TrackingProjectile::TrackingProjectile(const AppearanceTemplate* appearanceTemplate, WeaponObject &weaponObject, CrcLowerString const &emitterHardpoint, bool useRicochet) :
	Projectile(appearanceTemplate, NULL, NULL),
	m_isFirstAlter(true),
	m_wantFlyByEffect(true),
	m_useRicochet(useRicochet),
	m_targetHardpoint(),
	m_normalizedBeamLength(0.f),
	m_weaponWatcher(&weaponObject),
	m_weaponHardpoint(emitterHardpoint),
	m_actionNameCrc(0),
	m_attackerWatcher()
{
	setPaused(true);

	// store the info needed to create the hit effect in case the weapon is destroyed (unequiped, etc.)
	m_weaponObjectTemplateName = weaponObject.getObjectTemplateName();
	m_weaponType = weaponObject.getWeaponType();
	m_damageType = weaponObject.getDamageType();
	m_elementalDamageType = weaponObject.getElementalType();
}

//-------------------------------------------------------------------

TrackingProjectile::~TrackingProjectile()
{
}

//-------------------------------------------------------------------

float TrackingProjectile::alter(float time)
{
	//-- see if the target still exists
	if (!m_target)
		return AlterResult::cms_kill;

	//-- see if the projectile has died on its own
	if (Projectile::alter(time) == AlterResult::cms_kill) //lint !e777 // (Info -- Testing floats for equality) // Okay, we're using constants everywhere.
		return AlterResult::cms_kill;

	//-- see how far I have to go
	const Vector projectilePosition(getPosition_w());

	Vector targetPosition = m_target->getAppearanceSphereCenter_w();
	const float targetSphereRadius = m_target->getAppearanceSphereRadius();

	// track a hit position just on the outside of the sphere
	Vector hitPosition;
	
	if (m_targetHardpoint != CrcLowerString::empty)
	{
		Appearance const * const targetAppearance = m_target->getAppearance();

		if (targetAppearance != NULL)
		{
			Transform hardPointTransform;
			if (targetAppearance->findHardpoint(m_targetHardpoint, hardPointTransform))
			{
				targetPosition = m_target->rotateTranslate_o2w(hardPointTransform.getPosition_p());
				hitPosition = targetPosition;
			}
		}
	}
	else
	{
		hitPosition = targetPosition - (m_initialDirection * targetSphereRadius);
	}

	Vector direction_w(hitPosition - projectilePosition);

	LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(getAppearance());


	if (m_isFirstAlter)
	{
		m_initialDirection = direction_w;
		m_initialDirection.normalize(); // also used to scale the radius when calculating hitPosition, so it needs to be normalized

		if (!lightningAppearance)
		{
			// normal projectile - orient the projectile towards the target
			setFacing(getParentCell(), projectilePosition, hitPosition);
		}
		else
		{
			// change the velocity so that the beams all last at least the specified amount of time (ms_beamLifetime)
			const float distanceToTravel = (hitPosition - projectilePosition).magnitude();
			const float newSpeed = distanceToTravel / ms_beamLifetime;
			if (newSpeed < getSpeed())
			{
				setSpeed(newSpeed);
			}
		}

		m_isFirstAlter = false;
	}
	else
	{
		IGNORE_RETURN(direction_w.normalize());

		// see if the projectile has reached or passed the target
		const bool hitTarget = (direction_w.dot(m_initialDirection) <= 0.f);

		if (!getPaused())
		{
			//-- check whether to trigger the flyby effect
			if (m_wantFlyByEffect)
			{
				GroundScene* gs = safe_cast<GroundScene*>(Game::getScene());

				if (gs && (ms_flyByRange >= getPosition_w().magnitudeBetween(gs->getSoundObject()->getPosition_w())))
				{
					CombatEffectsManager::createFlyByEffect(*this, m_weaponObjectTemplateName, m_weaponType, m_damageType, m_elementalDamageType);
					m_wantFlyByEffect = false;	// disable subsequent checks
				}
			}

			if (hitTarget)
			{
				Vector hitNormal_w     = direction_w;
				Vector hitPoint_w      = projectilePosition;

				Vector rayBegin        = targetPosition + (direction_w * targetSphereRadius * 2.f);
				Vector rayEnd          = targetPosition ;

				CollisionInfo result;
				const uint16 collisionFlags = ClientWorld::CF_tangible | ClientWorld::CF_childObjects | ClientWorld::CF_skeletal | ClientWorld::CF_terrain;
				
				if (ClientWorld::collideObject(m_target, rayBegin, rayEnd, CollideParameters::cms_default, result, collisionFlags))
				{
					hitPoint_w = result.getPoint();
					hitNormal_w = result.getNormal();
					hitNormal_w.normalize();
					hitPoint_w += hitNormal_w * 0.05f; // move the hit slightly outside the mesh
				}

				CombatEffectsManager::createHitEffect(*m_target->getParentCell(), hitPoint_w, hitNormal_w, m_weaponObjectTemplateName, m_weaponType, m_damageType, m_elementalDamageType, m_actionNameCrc, m_useRicochet);

				// display any waiting combat text
				if (m_attackerWatcher.getPointer() && m_target.getPointer())
				{
					CuiCombatManager::removeCompletedCombatAction(m_attackerWatcher.getPointer()->getNetworkId(), m_target.getPointer()->getNetworkId());
				}

				setPosition_w(targetPosition);

				return AlterResult::cms_kill;
			}
			else
			{
				if (!lightningAppearance)
				{
					// normal projectile
					//-- slide towards target
					SimpleDynamics* const simpleDynamics = safe_cast<SimpleDynamics*>(getDynamics());
					if (simpleDynamics)
					{
						simpleDynamics->setCurrentVelocity_w(m_speed * direction_w);
					}
				}
			}
		}

		setPaused(false);
	}

	if (lightningAppearance)
	{
		// beam/lightning projectile
		Vector startPosition(projectilePosition);

		m_normalizedBeamLength = m_normalizedBeamLength > 1.f ? 1.f : m_normalizedBeamLength + time * ms_beamTimeToReachTargetMultiplier;
		const Vector endPosition = m_normalizedBeamLength < 1.f ?  (m_startPosition_w + m_normalizedBeamLength * (targetPosition - m_startPosition_w)) : targetPosition;

		WeaponObject * weaponObject = m_weaponWatcher.getPointer();
		if (weaponObject)
		{
			if (m_weaponHardpoint != CrcLowerString::empty)
			{
				Appearance const * const weaponAppearance = weaponObject->getAppearance();

				if (weaponAppearance != NULL)
				{
					Transform hardPointTransform;
					if (weaponAppearance->findHardpoint(m_weaponHardpoint, hardPointTransform))
					{
						startPosition = weaponObject->rotateTranslate_o2w(hardPointTransform.getPosition_p());
					}
				}
			}
		}

		
		lightningAppearance->setPosition_w(0, startPosition, endPosition);
	}

	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void TrackingProjectile::setTargetHardpoint(const CrcLowerString &hardpoint)
{
	m_targetHardpoint = hardpoint;
}

//-------------------------------------------------------------------

void TrackingProjectile::setAttacker(Object* const attacker)
{
	m_attackerWatcher = attacker;
}

//-------------------------------------------------------------------
