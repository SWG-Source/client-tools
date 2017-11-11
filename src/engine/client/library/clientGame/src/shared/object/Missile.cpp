//===================================================================
//
// Missile.cpp
//
// Copyright 2004 Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Missile.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/Countermeasure.h"
#include "clientGame/Game.h"
#include "clientGame/MissileManager.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/MultiShape.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"

//===================================================================

const float Missile::ms_initialTurnTime = 2.0f; // how long the missile goes straight after launch before turning towards the target
const float Missile::ms_countermeasureDistance = 15.0f; // how far away countermeasured missiles should be from the target when they explode
const float Missile::ms_hitDistance= 5.0f; // how far away an on-target missile should be when it explodes
const float Missile::ms_countermeasureDistanceSquared = ms_countermeasureDistance * ms_countermeasureDistance ;
const float Missile::ms_hitDistanceSquared = ms_hitDistance * ms_hitDistance;

//===================================================================

Missile::Missile(const int missileId, const int missileTypeId, Object * const target, NetworkId const & source, const Transform & sourceLocation, const Vector & targetLocation, const int timeToTarget, const float minSpeed, const AppearanceTemplate * const appearance, const AppearanceTemplate * const trailAppearanceTemplate, const ClientEffectTemplate * const fireEffect, const ClientEffectTemplate * const hitEffect, const ClientEffectTemplate * const countermeasureEffect, const bool foundLauncher, const int targetComponent) :
	Object(),
	m_missileId(missileId),
	m_missileTypeId(missileTypeId),
	m_state(foundLauncher ? MS_launch : MS_seek),
	m_target(target),
	m_targetLocation(targetLocation),
	m_remainingTime(static_cast<float>(timeToTarget)),
	m_timeToTarget(timeToTarget),
	m_initialTurnTimer(std::min(ms_initialTurnTime, m_remainingTime / 2.0f)),
	m_minSpeed(minSpeed),
	m_velocity(0.0f, 0.0f, 0.0f),
	m_initialVelocity(),
	m_maxAcceleration(25.0f),
	m_trail(new Object),
	m_hitEffect(hitEffect),
	m_countermeasureEffect(countermeasureEffect),
	m_deflectVector(Vector::randomUnit()),
	m_targetComponent(targetComponent),
	m_source(source)
{
	// increase template refcounts to make sure they don't go away while we're saving them
	if (m_hitEffect)
		m_hitEffect->fetch();
	if (m_countermeasureEffect)
		m_countermeasureEffect->fetch();

	if (appearance)
		setAppearance(appearance->createAppearance());
	
	setTransform_o2p(sourceLocation);
	m_initialVelocity = rotate_o2p(Vector::unitZ * m_minSpeed);
	m_velocity = m_initialVelocity;
	
	// Create a smoke trail for the missile
	m_trail->setTransform_o2p(sourceLocation);
	Appearance * const trailAppearance = trailAppearanceTemplate->createAppearance();
	m_trail->setAppearance(trailAppearance);
	ParticleEffectAppearance * const trailAppearanceAsParticle = ParticleEffectAppearance::asParticleEffectAppearance(trailAppearance);
	if (trailAppearanceAsParticle)
	{
		trailAppearanceAsParticle->setRestartOnRemoveFromWorld(false);
		DEBUG_FATAL(!trailAppearanceAsParticle->isInfiniteLooping(),("Data bug:  Missile had appearance %s for its trail.  This appearance should be infinitely looping, but it is not.", trailAppearanceTemplate->getName()));
	}
	addChildObject_p(m_trail);
	RenderWorld::addObjectNotifications(*m_trail);
	m_trail->addNotification(ClientWorld::getIntangibleNotification());

	// Play fire effect
	if (fireEffect)
	{
#if 0 // waiting on art
		CrcLowerString const hardpointName("exhaust");
		ClientEffect * const clientEffect = fireEffect->createClientEffect(this, hardpointName);
#endif
		ClientEffect * const clientEffect = fireEffect->createClientEffect(getParentCell(), getPosition_p(), Vector::unitY);
		clientEffect->execute();
		delete clientEffect;
	}
}

// ----------------------------------------------------------------------

Missile::~Missile()
{
	MissileManager::removeMissile(m_missileId);	
	if (m_hitEffect)
		m_hitEffect->release();
	if (m_countermeasureEffect)
		m_countermeasureEffect->release();

	if (m_trail)
	{
		detachTrail();
		m_trail = NULL; //lint !e423:  not a memory leak because after detachTrail(), we don't own the trail anymore
	}
} //lint !e1740:  effects are released, not deleted

// ----------------------------------------------------------------------

float Missile::alter(float elapsedTime)
{	
	// if we can still see the target, aim for it.  Otherwise, aim for the location the server gave us or the last known location
	const Object * const targetObject=m_target.getPointer();
	if (targetObject)
	{
		ClientObject const * const targetClientObject = targetObject->asClientObject();
		ShipObject const * const targetShipObject = (targetClientObject) ? targetClientObject->asShipObject() : NULL;
		if (targetShipObject && m_targetComponent != ShipChassisSlotType::SCST_invalid)
		{
			Vector newTargetLocation_w;
			if (targetShipObject->getShipObjectAttachments().getFirstComponentPosition_w(m_targetComponent, newTargetLocation_w))
				m_targetLocation = newTargetLocation_w;
			// otherwise the component has already been destroyed, so the missile continues on its present path
		}
		else
			m_targetLocation = targetObject->getPosition_w();
	}

	switch(m_state)
	{
	case MS_launch:
		{
			// smoothly turn after launch, so that it looks like the missile came out of the launcher

			Vector spiralVelocity;
			float speed = 0.0f;
			getVelocitySpiralModel(spiralVelocity, speed);
			
			m_velocity=Vector::linearInterpolate(m_initialVelocity, spiralVelocity, 1.0f-(m_initialTurnTimer / ms_initialTurnTime));

			m_initialTurnTimer -= elapsedTime;
			if (m_initialTurnTimer <= 0)
				m_state=MS_seek;
		}
		break;

	case MS_seek:
		{
			if (isWithinHitDistance() || elapsedTime >= m_remainingTime)
			{
				detachTrail();
				playHitEffect();
				kill();

				//-- If the player's ship is getting hit, notify the ClientShipTargeting system
				if (m_target.getPointer() == Game::getPlayerContainingShip())
					ClientShipTargeting::onPlayerShipHit(m_target.getPointer());

				return Object::alter(elapsedTime);
			}
			else
			{
				float speed;
				getVelocitySpiralModel(m_velocity, speed);
				IGNORE_RETURN(avoidObstacles(m_velocity, speed));
			}
		}
		break;

	case MS_miss:
		{
			if (checkForCollisions(elapsedTime))
			{
				// hit something, blow up
				detachTrail();
				playHitEffect();
				kill(); 
				return Object::alter(elapsedTime);
			}
		}
		break;

	case MS_countermeasured:
		{
			// follow the normal flight model, but blow up shortly before the target
			const float distanceSquared = (m_targetLocation - getPosition_w()).magnitudeSquared();
			if (distanceSquared <= ms_countermeasureDistanceSquared || elapsedTime >= m_remainingTime)
			{
				blowupCountermeasured(false);
				return Object::alter(elapsedTime);
			}
			else
			{
				float speed = 0.0f;
				getVelocitySpiralModel(m_velocity, speed);

				if (checkForCollisions(elapsedTime))
				{
					// would need to steer around something, so just blowup here instead
					blowupCountermeasured(true);
					return Object::alter(elapsedTime);
				}				
			}
		}
		break;
	}	

	// move towards the target
	const Vector newPosition = getPosition_w() + (m_velocity * elapsedTime);
	setPosition_w(newPosition);

	// face the way we're moving
	if (m_velocity!=Vector::zero)
		lookAt_p(getPosition_w()+m_velocity);

	// adjust remaining time
	m_remainingTime-=elapsedTime;
	
	IGNORE_RETURN(Object::alter(elapsedTime));
	return AlterResult::cms_alterNextFrame;

}

// ----------------------------------------------------------------------

/**
 * Get the velocity for the missile using a spiraling flight model.
 * The missile will fly towards the target at a speed that will get there at
 * the right time.  If that speed is slower than the allowed minimum, the
 * missile will spiral around the target.
 */
void Missile::getVelocitySpiralModel(Vector & resultVelocity, float & resultSpeed) const
{
	DEBUG_FATAL(m_remainingTime <= 0,("Programmer bug:  getVelocitySpiralModel() shouldn't be called if remainingTime <= 0"));
	if (m_remainingTime <= 0)
		return;

	// compute the velocity needed to get there at the right time
	resultVelocity = (m_targetLocation - getPosition_w()) / m_remainingTime;
	float const speedSquared = resultVelocity.magnitudeSquared();

	if (speedSquared < m_minSpeed * m_minSpeed)
	{
		// deflect to miss the target				
		Vector perpendicular=m_deflectVector.cross(resultVelocity);
		if (perpendicular.magnitudeSquared() < 0.01f)
		{
			// target was in that direction, the perpendicular isn't very accurate
			// so use a different direction.
			perpendicular=getObjectFrameJ_w().cross(resultVelocity);
			if (perpendicular.magnitudeSquared() < 0.01f)
				perpendicular=getObjectFrameK_w().cross(resultVelocity); // one of these has got to work
		}
		IGNORE_RETURN(perpendicular.normalize());
	
		// scale the perpendicular so that the sum of the vectors has the right length
		const float scale = sqrt((m_minSpeed * m_minSpeed) - speedSquared);				

		resultVelocity+=perpendicular*scale;
		resultSpeed = m_minSpeed;
	}
	else
		resultSpeed = sqrt(speedSquared);	
}

// ----------------------------------------------------------------------

bool Missile::avoidObstacles(Vector & velocity, float const speed) const
{
	Vector avoidancePosition_w;
	Object const * const targetObject=m_target.getPointer();

	Vector targetPosition_w;
	if (targetObject)
	{
		targetPosition_w = targetObject->getPosition_w();
	}
	else
	{
		targetPosition_w = getPosition_w() + (m_remainingTime * velocity);
	}

	if (SpaceAvoidanceManager::getAvoidancePosition(getTransform_o2w(), getAppearanceSphereRadius(), velocity, targetPosition_w, avoidancePosition_w, targetObject))
	{
		Vector newVelocity(avoidancePosition_w - getPosition_w());
		IGNORE_RETURN(newVelocity.approximateNormalize());
		velocity = newVelocity * speed;
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

void Missile::hitTarget() const
{
}

// ----------------------------------------------------------------------

void Missile::countermeasure(Countermeasure * const targetCountermeasure)
{
	m_state = MS_countermeasured;
	m_target = targetCountermeasure;
}

// ----------------------------------------------------------------------

/**
 * The missile has been countermeasured, but the target ship is no longer
 * avaiable.  Therefore, we don't know where to launch the countermeasure 
 * from.
 */
void Missile::countermeasure()
{
	m_state = MS_miss;
}

// ----------------------------------------------------------------------

void Missile::missTarget()
{
	m_state = MS_miss;	
}

// ----------------------------------------------------------------------

void Missile::playHitEffect() const
{
	if (m_hitEffect)
	{
#if 0 // waiting on art
		CrcLowerString const hardpointName("nose");
		ClientEffect * const clientEffect = m_hitEffect->createClientEffect(this, hardpointName);
#endif
		ClientEffect * const clientEffect = m_hitEffect->createClientEffect(getParentCell(), getPosition_p(), Vector::unitY);
		clientEffect->execute();
		delete clientEffect;
	}
}

// ----------------------------------------------------------------------

void Missile::playCountermeasureEffect() const
{
	if (m_countermeasureEffect)
	{
#if 0 //waiting for art
		CrcLowerString const hardpointName("nose");
		ClientEffect * const clientEffect = m_countermeasureEffect->createClientEffect(this, hardpointName);
#endif
  		ClientEffect * const clientEffect = m_countermeasureEffect->createClientEffect(getParentCell(), getPosition_p(), Vector::unitY);
		clientEffect->execute();
		delete clientEffect;
	}
}

// ----------------------------------------------------------------------

bool Missile::isWithinHitDistance() const
{
	return (m_targetLocation - getPosition_w()).magnitudeSquared() < ms_hitDistanceSquared;
}

// ----------------------------------------------------------------------

void Missile::detachTrail()
{
	//-- Do not detach the trail if we're in World::remove().  We need to check that the World is valid so the trail doesn't re-add itself to the World.
	if (m_trail.getPointer() && World::isValid())
	{
		// leave the smoke trail in the world to dissipate
		removeChildObject(m_trail, Object::DF_parent);
		ParticleEffectAppearance * const appearance = ParticleEffectAppearance::asParticleEffectAppearance(m_trail->getAppearance());
		if (appearance)
			appearance->setEnabled(false);
		m_trail = NULL; //lint !e423:  not a memory leak because we don't own the trail anymore
	}
}

// ----------------------------------------------------------------------

/**
 * Destroy the missile and the countermeasure that was launched against it
 */
void Missile::blowupCountermeasured(bool useHitEffect)
{
	if (useHitEffect)
		playHitEffect();
	else
		playCountermeasureEffect();
	detachTrail();
	kill(); 
}

// ----------------------------------------------------------------------

Object const * const Missile::getTargetObject() const
{
	return m_target.getPointer();
}

// ----------------------------------------------------------------------

/**
 * Check whether the missile would collide with anything (other than the 
 * target and the ship that launched it)
 */
bool Missile::checkForCollisions(float const elapsedTime) const
{
	Capsule const missileCapsule_w(Sphere(getPosition_w(), 1.0f), (m_velocity * elapsedTime));
	ColliderList collidedWith;
	CollisionWorld::getDatabase()->queryFor(static_cast<int>(SpatialDatabase::Q_Physicals), CellProperty::getWorldCellProperty(), true, missileCapsule_w, collidedWith);
	for (ColliderList::const_iterator i = collidedWith.begin(); i != collidedWith.end(); ++i)
	{
		Object const * const collider = &(NON_NULL(*i)->getOwner());
		if (collider != m_source.getObject() && collider != m_target.getPointer())
			return true;
	}
	return false;
}

//===================================================================
