//===================================================================
//
// NonTrackingProjectile.cpp
//
// Copyright 2000, 2001 Verant Interactive, Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/NonTrackingProjectile.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientParticle/LightningAppearance.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/SimpleDynamics.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace NonTrackingProjectileNamespace
{
	void remove();

	MemoryBlockManager* ms_memoryBlockManager;

	const float  ms_range          = 128.f;
	const uint16 ms_collisionFlags = ClientWorld::CF_terrain | ClientWorld::CF_tangible | ClientWorld::CF_tangibleNotTargetable | ClientWorld::CF_tangibleFlora | ClientWorld::CF_interiorObjects | ClientWorld::CF_interiorGeometry | ClientWorld::CF_childObjects | ClientWorld::CF_skeletal;
	const float ms_beamTimeToReachTargetMultiplier = 1.f / 0.75f; // 1 / seconds for beam to reach target - used to simulate the beam traveling toward the target
	
	float ms_flyByRange = 0.f;
	const float ms_beamLifetime = 1.5f;
}

using namespace NonTrackingProjectileNamespace;

//===================================================================

void NonTrackingProjectile::install()
{
	InstallTimer const installTimer("NonTrackingProjectile::install");

	ms_memoryBlockManager = new MemoryBlockManager("NonTrackingProjectile::ms_memoryBlockManager", true, sizeof(NonTrackingProjectile), 0, 0, 0);

	ms_flyByRange = ConfigClientGame::getProjectileFlyByRange();

	ExitChain::add(NonTrackingProjectileNamespace::remove, "NonTrackingProjectileNamespace::remove");
}

//-------------------------------------------------------------------

void NonTrackingProjectileNamespace::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("NonTrackingProjectile is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void *NonTrackingProjectile::operator new(size_t size)
{
	UNREF(size);

	DEBUG_FATAL(!ms_memoryBlockManager,("NonTrackingProjectile is not installed"));

	// do not try to alloc a descendant class with this allocator
	DEBUG_FATAL(size != sizeof(NonTrackingProjectile),("bad size"));

	return ms_memoryBlockManager->allocate();
}

//-------------------------------------------------------------------

void NonTrackingProjectile::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("NonTrackingProjectile is not installed"));
	ms_memoryBlockManager->free(pointer);
}

//===================================================================

NonTrackingProjectile::NonTrackingProjectile(const AppearanceTemplate* appearanceTemplate, WeaponObject &weaponObject, CrcLowerString const &emitterHardpoint) :
	Projectile(appearanceTemplate, NULL, NULL),
	m_isFirstAlter(true),
	m_wantFlyByEffect(true),
	m_hitType(HT_other),
	m_hitCellProperty(0),
	m_hitCellObject(0),
	m_hitPoint_w(),
	m_hitNormal_w(),
	m_travelDistance(ms_range),
	m_normalizedBeamLength(0.f),
	m_weaponWatcher(&weaponObject),
	m_weaponHardpoint(emitterHardpoint),
	m_attackerWatcher()
{
	m_isPaused = true;

	// store the info needed to create the hit effect in case the weapon is destroyed (unequiped, etc.)
	m_weaponObjectTemplateName = weaponObject.getObjectTemplateName();
	m_weaponType = weaponObject.getWeaponType();
	m_damageType = weaponObject.getDamageType();
	m_elementalDamageType = weaponObject.getElementalType();
}

//-------------------------------------------------------------------

NonTrackingProjectile::~NonTrackingProjectile()
{
	m_hitCellProperty = 0;
}

//-------------------------------------------------------------------

float NonTrackingProjectile::alter(float deltaTime)
{
	//-- update the projectile and see if it died on its own
	if (Projectile::alter(deltaTime) == AlterResult::cms_kill) //lint !e777 // Testing floats for equality // It's okay, we're using constants.
		return AlterResult::cms_kill;

	LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(getAppearance());

	//-- setup travel distance (thereby only doing collision once)
	if (m_isFirstAlter)
	{
		m_isFirstAlter = false;

		if (m_lastCellObject)
		{
			CollisionInfo result;
			if (ClientWorld::collide(m_lastCellProperty, m_lastPosition_w, m_lastPosition_w + getObjectFrameK_w() * ms_range, CollideParameters::cms_default, result, ms_collisionFlags, m_excludeObject))
			{
				if (dynamic_cast<const TerrainObject*>(result.getObject()))
					m_hitType = HT_terrain;
				else
				{
					const TangibleObject* const tangibleObject = dynamic_cast<const TangibleObject*> (result.getObject ());
					if (tangibleObject)
					{
						const SharedTangibleObjectTemplate* const tangibleObjectTemplate = dynamic_cast<const SharedTangibleObjectTemplate*> (tangibleObject->getObjectTemplate ());
						if (tangibleObjectTemplate)
						{
							switch (tangibleObjectTemplate->getSurfaceType ())
							{
							case SharedTangibleObjectTemplate::ST_metal:  m_hitType = HT_metal;  break;
							case SharedTangibleObjectTemplate::ST_stone:  m_hitType = HT_stone;  break;
							case SharedTangibleObjectTemplate::ST_wood:   m_hitType = HT_wood;   break;
							default:
							case SharedTangibleObjectTemplate::ST_other:  m_hitType = HT_other;  break;
							}
						}
						else
							DEBUG_WARNING (true, ("NonTrackingProjectile::alter: TangibleObject [%s] does not have a SharedTangibleObjectTemplate", tangibleObject->getObjectTemplate ()->getName ()));
					}
					else
						m_hitType = HT_other;
				}

				m_hitCellProperty = result.getObject()->getParentCell();
				m_hitCellObject   = &m_hitCellProperty->getOwner();
				m_hitPoint_w      = result.getPoint();
				m_hitNormal_w     = result.getNormal();
				m_travelDistance  = m_hitPoint_w.magnitudeBetween(m_lastPosition_w);

				if(lightningAppearance)
				{
					// change the velocity so that the beams all last at least the specified amount of time (ms_beamLifetime)
					const float distanceToTravel = m_travelDistance;
					const float newSpeed = distanceToTravel / ms_beamLifetime;
					if (newSpeed < getSpeed())
					{
						setSpeed(newSpeed);
					}
				}
			}
			else
			{
				// isn't going to collide w/anything, so just create a hit position based on the direction and range
				m_hitPoint_w = m_startPosition_w + (getObjectFrameK_w() * ms_range);

				if (lightningAppearance)
				{
					// this is a beam into space, so make it disappear faster (otherwise it's visible for way too long)
					setSpeed(getSpeed() * 2.f);
				}
			}
		}
	}
	else
	{
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


			//-- see if we've travelled our travel distance
			m_travelDistance -= getPosition_w().magnitudeBetween(m_lastPosition_w);
			if (m_travelDistance <= 0.f)
			{
				//-- is the cell that we want to create the client effect in still valid?
				if (m_hitCellObject && (m_hitType != HT_other))
				{
					Transform const & transform_o2w = m_hitCellObject->getTransform_o2w();
					Vector const point_o = (m_hitCellProperty == CellProperty::getWorldCellProperty()) ? m_hitPoint_w : transform_o2w.rotateTranslate_p2l(m_hitPoint_w);
					Vector const normal_o = (m_hitCellProperty == CellProperty::getWorldCellProperty()) ? m_hitNormal_w : transform_o2w.rotate_p2l(m_hitNormal_w);

					CombatEffectsManager::createMissEffect(*m_hitCellProperty, point_o, normal_o, m_weaponObjectTemplateName, m_weaponType, m_damageType, m_elementalDamageType);

					// display any waiting combat text
					if (m_attackerWatcher.getPointer() && m_target.getPointer())
					{
						CuiCombatManager::removeCompletedCombatAction(m_attackerWatcher.getPointer()->getNetworkId(), m_target.getPointer()->getNetworkId());
					}
				}

				setPosition_w(m_hitPoint_w);

				return AlterResult::cms_kill;
			}

		}
		// unpause this projectile
		setPaused(false);
	}

	if (lightningAppearance)
	{
		// beam/lightning projectile
			
		//-- see if the target still exists
		if (!m_target)
		{
			return AlterResult::cms_kill;
		}

		Vector startPosition(getPosition_w());
		Vector targetPosition = m_target->getAppearanceSphereCenter_w();

		m_normalizedBeamLength = m_normalizedBeamLength > 1.f ? 1.f : m_normalizedBeamLength + (deltaTime * ms_beamTimeToReachTargetMultiplier);
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

void NonTrackingProjectile::setExcludeObject(Object const * const excludeObject)
{
	m_excludeObject = excludeObject;
}

//-------------------------------------------------------------------

void NonTrackingProjectile::setTravelDistance(float travelDistance)
{
	m_travelDistance = travelDistance;
}

//-------------------------------------------------------------------

void NonTrackingProjectile::setAttacker(Object* const attacker)
{
	m_attackerWatcher = attacker;
}

//===================================================================
