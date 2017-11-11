// ======================================================================
//
// GrenadeLobAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GrenadeLobAction.h"
#pragma optimize ("", off)

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/GrenadeLobActionTemplate.h"
#include "clientGame/PlaySkeletalAnimationAction.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>

// ======================================================================

const ConstCharCrcLowerString cs_explodeEventName("explode");
const ConstCharCrcLowerString cs_flyEventName("fly");

const float          PI_UNDER_180     = 180.0f / PI;

const float          cs_minPitchSpeed = 180.0f * PI_OVER_180;
const float          cs_maxPitchSpeed = 720.0f * PI_OVER_180;

const float          cs_minYawSpeed   = cs_minPitchSpeed / 3.0f;
const float          cs_maxYawSpeed   = cs_maxPitchSpeed / 3.0f;

// ======================================================================
// inlines
// ======================================================================

inline const GrenadeLobActionTemplate &GrenadeLobAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const GrenadeLobActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class GrenadeLobAction: public member functions
// ======================================================================

GrenadeLobAction::GrenadeLobAction(const GrenadeLobActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_state(S_setupNotification),
	m_xzVelocity(),
	m_yAcceleration(-9.8f),
	m_yStartVelocity(0.0f),
	m_yStartPosition(0.0f),
	m_accumulatedFlyTime(0.0f),
	m_totalFlyTime(0.0f),
	m_accumulatedSitTime(0.0f),
	m_totalSitTime(0.0f),
	m_trackTarget(false),
	m_showGrenadeDuringFlight(false),
	m_grenadeWatcher(0),
	m_grenadeDestination(),
	m_grenadeDestinationCell(0),
	m_grenadeExplodeEffectName(0),
	m_grenadePitchSpeed(0.0f),
	m_grenadeYawSpeed(0.0f),
	m_grenadeDeleted(false),
	m_animationNotification(0),
	m_throwAnimationMessageName(),
	m_receivedThrowMessage(false)
{
}

// ----------------------------------------------------------------------

GrenadeLobAction::~GrenadeLobAction()
{
	// Make sure we clean up our thrown grenade.
	Object *grenadeObject = 0;

	if (m_grenadeWatcher)
		grenadeObject = *m_grenadeWatcher;

	if(grenadeObject)
	{
		if(grenadeObject->isInWorld())
			grenadeObject->removeFromWorld();

		delete grenadeObject;
	}

	delete m_grenadeExplodeEffectName;
	delete m_grenadeWatcher;

	if (m_animationNotification)
	{
		m_animationNotification->removeCallback(staticNotificationCallback, this);

		m_animationNotification->release();
		m_animationNotification = NULL;
	}

	m_grenadeDestinationCell = NULL;
}

// ----------------------------------------------------------------------

void GrenadeLobAction::cleanup(PlaybackScript &playbackScript)
{
	// Cleanup notification.
	if (m_animationNotification)
	{
		m_animationNotification->removeCallback(staticNotificationCallback, this);

		m_animationNotification->release();
		m_animationNotification = 0;
	}

	// Ensure the grenade is destroyed.
	if (m_grenadeDeleted)
	{
		// Nothing to do.
		return;
	}
	
	Object *grenadeObject = 0;
	
	//-- Try to get the grenade object from the watcher.  If this is
	//   non-NULL, the grenade has already been removed from the attacker's hand.
	if (m_grenadeWatcher)
		grenadeObject = *m_grenadeWatcher;

	if (!grenadeObject)
	{
		//-- Try to get the grenade object from the attacker's hand.

		// Get our template.
		const GrenadeLobActionTemplate &actionTemplate = getOurTemplate();

		// Get the attacker object & appearance.
		Object *const attackerObject = playbackScript.getActor(actionTemplate.getAttackerActorIndex());
		if (!attackerObject)
		{
			DEBUG_WARNING(true, ("GrenadeLobAction: attacker is NULL."));
			return;
		}

		SkeletalAppearance2 *const attackerAppearance = dynamic_cast<SkeletalAppearance2*>(attackerObject->getAppearance());
		if (!attackerAppearance)
		{
			DEBUG_WARNING(true, ("GrenadeLobAction: attacker does not have a skeletal appearance."));
			return;
		}

		// Get the attachment hardpoint name.
		CrcLowerString  hardpointName;

		const bool gotHardpointName = actionTemplate.getHardpointName(playbackScript, hardpointName);
		if (!gotHardpointName)
		{
			DEBUG_WARNING(true, ("GrenadeLobAction: failed to get hardpoint name."));
			return;
		}

		// Get the grenade object from the hardpoint.
		grenadeObject = attackerAppearance->findAttachedObject(hardpointName);
		if (grenadeObject)
		{
			// Remove from the hardpoint.
			attackerAppearance->detach(grenadeObject);
		}
	}

	delete grenadeObject;
	m_grenadeDeleted = true;
}

// ----------------------------------------------------------------------

bool GrenadeLobAction::update(float deltaTime, PlaybackScript &script)
{
	bool keepGoing = true;

	while (keepGoing)
	{
		switch (m_state)
		{
			case S_setupNotification:
				{
					//-- Setup the animation notification.
					if (!m_animationNotification)
					{
						//-- Get our template.
						const GrenadeLobActionTemplate &actionTemplate = getOurTemplate();

						//-- Setup animation notification callback.
						PlaySkeletalAnimationAction *const action = dynamic_cast<PlaySkeletalAnimationAction*>(script.getAction(actionTemplate.getThrowAnimationThreadIndex(), actionTemplate.getThrowAnimationActionIndex()));
						if (!action)
						{
							DEBUG_WARNING(true, ("GrenadeLobAction: specified skeletal animation action for throw animation is not a PlaySkeletalAnimationAction, skipping: thread=[%d],action=[%d].", actionTemplate.getThrowAnimationThreadIndex(), actionTemplate.getThrowAnimationActionIndex()));
							return false;
						}

						m_animationNotification = action->fetchNotification();
						NOT_NULL(m_animationNotification);

						//-- Get throw animation message name.
						std::string  messageName("throw");
						bool const gotVariable = script.getStringVariable(actionTemplate.getThrowAnimationMessageNameVariable(), messageName);
						if (!gotVariable)
						{
							DEBUG_WARNING(true, ("GrenadeLobAction: specified throw animation message name doesn't exist on playback script, skipping."));
							return false;
						}
						m_throwAnimationMessageName.set(messageName.c_str(), true);

						m_animationNotification->addCallback(staticNotificationCallback, this);
					}

					//-- Wait for an animation to be associated with the notification.
					if (m_animationNotification->getChannelCount() < 1)
						return true;
					else
						m_state = S_waitingForAnimation;
				}
				break;

			case S_waitingForAnimation:
				{
					NOT_NULL(m_animationNotification);

					//-- Wait for reception of throw message or for animation to end.
					if (m_receivedThrowMessage || m_animationNotification->isAnimationDone(0))
					{
						//-- Move on to lobbing state, we either received the throw message or the animation has completed for whatever reason.
#ifdef _DEBUG
						if (m_receivedThrowMessage)
							DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: throwing grenade because animation trigger [%s] was received (as expected).\n", m_throwAnimationMessageName.getString()));
						else
							DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: throwing grenade because animation never triggered throw message,reason=[%s] (animation issue).\n", AnimationNotification::getStateName(m_animationNotification->getState(0))));
#endif
						m_state = S_initialize;
					}
					else
					{
						//-- Wait for throw message to be received or for animation to get trumped.
						return true;
					}
				}
				break;

			case S_initialize:
				return updateUninitialized(script);

			case S_lobbing:
				return updateLobbing(deltaTime, script);

			case S_landed:
				return updateLanded(deltaTime);

			default:
				DEBUG_FATAL(true, ("unknown state [%d].", static_cast<int>(m_state)));
				return false; //lint !e527 // Unreachable // Reachable in release.
		}
	}

	return false;
}

// ======================================================================
// private static member functions
// ======================================================================

float GrenadeLobAction::getObjectRadius(const Object &object)
{
	//-- Attempt to get the radius from the SharedCreatureObjectTemplate.
	const CreatureObject *const creatureObject = dynamic_cast<const CreatureObject*>(&object);
	if (creatureObject)
	{
		const SharedCreatureObjectTemplate *const objectTemplate = dynamic_cast<const SharedCreatureObjectTemplate*>(creatureObject->getObjectTemplate());
		if (objectTemplate)
		{
			//-- Return the scaled collision radius.
			const float radius = objectTemplate->getCollisionRadius() * creatureObject->getScaleFactor();
			DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: retrieved defender radius from object template and scale [%.2f].\n", radius));

			return radius;
		}
	}

	//-- Attempt to get the object's radius from the appearance.
	// @todo resolve this issue: SkeletalAppearance2 returns an appearance
	//       that is scaled appropriately.  All other appearances do not include
	//       scale.
	const Appearance *const appearance = object.getAppearance();
	if (appearance)
	{
		const Sphere &sphere = appearance->getSphere();
		const float   radius = sphere.getRadius();
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: retrieved defender radius from appearance radius [%.2f].\n", radius));

		return radius;
	}
	else
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: defender's appearance is NULL, assuming radius is [1.0].\n"));
		return 1.0f;
	}
}

// ----------------------------------------------------------------------
/**
 * Create a Vector pointing in a random direction with a length
 * randomized between the specified min and max distances.
 *
 * @param minDistance  the minimum length of the returned Vector.
 * @param maxDistance  the maximum length of the returned Vector.
 *
 * @return  a Vector pointing in a random direction with a randomized
 *          length in between the specified min and max length.
 */

Vector GrenadeLobAction::createRandomVector(float minDistance, float maxDistance)
{
	DEBUG_FATAL(minDistance > maxDistance, ("minDistance [%g] > maxDistance [%g].", minDistance, maxDistance));

	//-- Randomize a yaw for the offset.
	const float yawAngle = Random::randomReal(PI * 2.0f);
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: target offset angle [%.2f].\n", yawAngle * PI_UNDER_180));

	//-- Compute the offset direction.
	Transform t;
	t.yaw_l(yawAngle);

	const Vector offsetDirection = t.rotate_l2p(Vector::unitZ);

	//-- Randomize distance used.
	const float distanceFromTarget = minDistance + (maxDistance - minDistance) * Random::randomReal(1.0f);
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: target offset distance [%.2f].\n", distanceFromTarget));

	return distanceFromTarget * offsetDirection;
}

// ----------------------------------------------------------------------
/**
 * Return the world-space target position for the grenade.
 *
 * @param targetObject  the target object for the grenade throw operation.
 * @param isHit         if true, the target was hit by the grenade; if false, the target is not hit.
 *
 * @return the world-space position where the grenade should land.
 */

Vector GrenadeLobAction::getTargetPosition(const Object &targetObject, bool /* isHit */, bool throwAtCenter, bool randomOffset)
{
#if 0
	//-- Create the min and max randomization vector based on appearance radius.
	float  minDistanceRadiusFactor;
	float  maxDistanceRadiusFactor;

	// @todo load these factors from script.
	if (isHit)
	{
		minDistanceRadiusFactor = 0.0f;
		maxDistanceRadiusFactor = 0.75f;
	}
	else
	{
		minDistanceRadiusFactor = 4.0f;
		maxDistanceRadiusFactor = 8.0f;
	}
#endif

#if 1
	//-- All grenade throws land within a very tight area around the target.
	//   This is the quick-fix solution to prevent an attacker from being close
	//   to a defender but then throwing the grenade backwards or far from where
	//   it would make sense.
	float const minRadius = 0.0f;
	float const maxRadius = 0.5f;
#else
	//-- Get the target object's appearance radius.
	const float objectRadius = getObjectRadius(targetObject);

	const float minRadius    = minDistanceRadiusFactor * objectRadius;
	const float maxRadius    = maxDistanceRadiusFactor * objectRadius;

	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: random offset radius min/max factor [%g/%g].\n", minDistanceRadiusFactor, maxDistanceRadiusFactor));
#endif

	//-- Create the random offset.
	const Vector randomOffsetVector = createRandomVector(minRadius, maxRadius);

	//-- Get the target object position in world space.
	const Vector objectPosition_w = throwAtCenter ? targetObject.getCollisionSphereExtent_w().getCenter() : targetObject.getPosition_w();

	//-- Create the destination estimate position based on target object plus a random offset.
	const Vector destinationEstimate_w = randomOffset ? objectPosition_w + randomOffsetVector : objectPosition_w;

	//-- Check for a collision between the target object and the destination estimate.
	CollisionInfo       collisionInfo;
	const CellProperty *parentCell = targetObject.getParentCell();

	bool collided = ClientWorld::collide(parentCell, objectPosition_w, destinationEstimate_w, CollideParameters::cms_default, collisionInfo);
	if (collided)
	{
		// We are trying to go some distance away from the target but we have collided with something on the
		// way to that point.  Use the collision point as the target location for the grenade.
		return collisionInfo.getPoint();
	}
	else
	{
		// Shoot a ray straight down the -y axis starting one meter above the destination estimate position.
		Vector startPosition_w  = destinationEstimate_w;
		startPosition_w.y      += 1.0f;

		Vector endPosition_w   = startPosition_w;
		endPosition_w.y       -= 1000.0f;

		collided = ClientWorld::collide(parentCell, startPosition_w, endPosition_w, CollideParameters::cms_default, collisionInfo);
		if (collided)
			return collisionInfo.getPoint();
		else
			return endPosition_w;
	}
}

// ----------------------------------------------------------------------

void GrenadeLobAction::staticNotificationCallback(void *context, int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger)
{
	NOT_NULL(context);
	GrenadeLobAction *action = static_cast<GrenadeLobAction*>(context);
	action->notificationCallback(channel, animationMessage, elapsedTimeSinceTrigger);
}

// ======================================================================
// private member functions
// ======================================================================

bool GrenadeLobAction::initialize(PlaybackScript &script)
{
	//-- Get our template.
	const GrenadeLobActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the sit time for the grenade (how long to park it after it lands before exploding it)
	const bool gotSitTime = script.getFloatVariable(actionTemplate.getSitTimeVariable(), m_totalSitTime);
	if (!gotSitTime || (m_totalSitTime <= 0.0f))
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: invalid or unspecified sit time [%g], failing initialization.", m_totalSitTime));
		return false;
	}
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: sit time [%.2f] seconds.\n", m_totalSitTime));

	//-- Get the air-time for the lob.
	const bool gotAirTime = script.getFloatVariable(actionTemplate.getAirTimeVariable(), m_totalFlyTime);
	if (!gotAirTime || (m_totalFlyTime <= 0.0f))
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: invalid or unspecified air/fly time [%g], failing initialization.", m_totalFlyTime));
		return false;
	}
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: fly time [%.2f] seconds.\n", m_totalFlyTime));

	//-- Get the gravitational force applied in the y direction (should be negative to pull downward).
	const bool gotGraviationalForce = script.getFloatVariable(actionTemplate.getGravitationalForceVariable(), m_yAcceleration);
	if (!gotGraviationalForce  || (m_yAcceleration >= 0.0f))
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: invalid or unspecified graviational force [%g], must be negative to pull downward.", m_yAcceleration));
		return false;
	}
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: gravitational force [%.2f] meters / second^2.\n", m_yAcceleration));

	//-- Get the attacker object & appearance.
	Object *const attackerObject = script.getActor(actionTemplate.getAttackerActorIndex());
	if (!attackerObject)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: attacker is NULL, failing initialization."));
		return false;
	}

	SkeletalAppearance2 *const attackerAppearance = dynamic_cast<SkeletalAppearance2*>(attackerObject->getAppearance());
	if (!attackerAppearance)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: attacker does not have a skeletal appearance, failing initialization."));
		return false;
	}

	//-- Get the defender object.
	Object *defenderObject = script.getActor(actionTemplate.getDefenderActorIndex());

	if(!defenderObject)
	{
		// try and use the attacker as the defender
		defenderObject = script.getActor(actionTemplate.getAttackerActorIndex());
	}

	if (!defenderObject)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: defender is NULL, failing initialization."));
		return false;
	}

	//-- Determine whether grenade is visible during flight.  We show the grenade only when both
	//   the attacker and defender are in the world cell.
	m_showGrenadeDuringFlight = (attackerObject->getParentCell() == defenderObject->getParentCell());
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: show grenade flight: [%s].\n", m_showGrenadeDuringFlight ? "true" : "false"));

	//-- Detatch the grenade object from the hardpoint and create the watcher for it.
	// Get the attachment hardpoint.
	CrcLowerString  hardpointName;

	const bool gotHardpointName = actionTemplate.getHardpointName(script, hardpointName);
	if (!gotHardpointName)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: failed to get hardpoint name, failing intialization."));
		return false;
	}

	// Find the attached object in the specified hardpoint.
	Object *grenadeObject = attackerAppearance->findAttachedObject(hardpointName);

	if (m_showGrenadeDuringFlight)
		m_grenadeWatcher = new Watcher<Object>(grenadeObject);

	if (!grenadeObject)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: attacker does not have an object attached at hardpoint [%s], failing intialization.", hardpointName.getString()));
		return false;
	}

	//-- Look up the name of the explode effect on the grenade.
	m_grenadeExplodeEffectName = new CrcLowerString();
	ClientEventManager::getEffectName(cs_explodeEventName, *grenadeObject, *m_grenadeExplodeEffectName);

#ifdef _DEBUG
	if (strlen(m_grenadeExplodeEffectName->getString()) == 0)
		DEBUG_WARNING(true, ("GLA: effect lookup for event [%s] on grenade object [%s] failed.", cs_explodeEventName.getString(), grenadeObject->getObjectTemplate()->getName()));
	else
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: will play event [%s] that maps to effect [%s].\n", cs_explodeEventName.getString(), m_grenadeExplodeEffectName->getString()));
#endif

	//-- Detatch the object from the specified hardpoint and add to world.
	// Detach.
	attackerAppearance->detach(grenadeObject);

	if (m_showGrenadeDuringFlight)
	{
		// Add the grenade to the world.
		grenadeObject->addToWorld();
		grenadeObject->setParentCell(defenderObject->getParentCell());

		// Play the "fly" event on the grenade as it travels through the world.
		const bool flySucceeded = ClientEventManager::playEvent(cs_flyEventName, grenadeObject, CrcLowerString::empty);
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: fly event played, return value [%s].\n", flySucceeded ? "true" : "false"));
		UNREF(flySucceeded);

		// Determine pitch and yaw speed for tumbling grenade.
		m_grenadePitchSpeed = Random::randomReal(cs_minPitchSpeed, cs_maxPitchSpeed);
		m_grenadeYawSpeed   = Random::randomReal(cs_minYawSpeed,   cs_maxYawSpeed);

		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: tumble rotation speeds (deg/sec) [pitch=%0.2f,yaw=%0.2f].\n", m_grenadePitchSpeed * PI_UNDER_180, m_grenadeYawSpeed * PI_UNDER_180));
	}
	else
	{
		// Delete the grenade object.
		delete grenadeObject;
		grenadeObject = 0;

		m_grenadeDeleted = true;
	}

	//-- Get the position of the hardpoint in the parent space of the attacker.
	Transform  hardpoint_o(Transform::IF_none);

	const bool gotHardpointTransform = attackerAppearance->findHardpoint(hardpointName, hardpoint_o);
	if (!gotHardpointTransform)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction: failed to get hardpoint transform [%s] from attacker appearance, failing intialization.", hardpointName.getString()));
		return false;
	}

	const Vector hardpointPosition_w = attackerObject->getTransform_o2w().rotateTranslate_l2p(hardpoint_o.getPosition_p());
	m_yStartPosition                 = hardpointPosition_w.y;

	if (m_showGrenadeDuringFlight)
	{
		NOT_NULL(grenadeObject);
		grenadeObject->setPosition_w(hardpointPosition_w);

#ifdef _DEBUG
		if (ConfigClientGame::getLogGrenadeLobAction())
		{
			const Vector grenadePosition_p = grenadeObject->getPosition_p();
			DEBUG_REPORT_LOG(true, ("GLA: grenade starting at defender cell coordinates [(x=%.2f,y=%.2f,z=%.2f)].\n", grenadePosition_p.x, grenadePosition_p.y, grenadePosition_p.z));
		}
#endif
	}

	{
		int track = 0;
		m_trackTarget = (script.getIntVariable(actionTemplate.getTrackTargetVariable(), track) && track > 0);
	}

	//-- Find the target position based on the defender position.
	int tmp;
	const bool       throwAtCenter    = (script.getIntVariable(actionTemplate.getThrowAtCenterVariable(), tmp) && tmp > 0);
	const bool       hitStatus        = isDefenderHit(script);
	const Vector     targetPosition_w = getTargetPosition(*defenderObject, hitStatus, throwAtCenter, !m_trackTarget);
	DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: target position in world space [(x=%.2f,y=%.2f,z=%.2f)].\n", targetPosition_w.x, targetPosition_w.y, targetPosition_w.z));

	//-- Save grenade destination info.
	m_grenadeDestinationCell = defenderObject->getParentCell();
	m_grenadeDestination     = targetPosition_w;

	//-- Calculate x-z velocity in attacker parent space.
	const Vector deltaPosition_w    = targetPosition_w - hardpointPosition_w;
	
	m_xzVelocity                      = deltaPosition_w;
	m_xzVelocity.y                    = 0.0f;
	m_xzVelocity                     /= m_totalFlyTime;

	//-- Calculate initial y velocity.
	/*
		Derivation of the equation for initial y velocity (ignoring x-z motion).

		Start with the equation y = f(v0, a, dt) where v0 = initial velocity, a = gravitational force/acceleration, dt = delta time.
		From these:
			vbar = dy / dt
			vbar = (v0 + v1) / 2
			a    = dv / dt = (v1 - v0) / dt

		We can substitute and come up with this:
			y = ((2 v0 + a dt) / 2) * dt    (i)

		This represents the height (+y = up) of a projectile given the initial velocity v0, gravity = a, and time since fire.

		We know at dt = 0 that y = 0.
		We know that at dt = t_air (fly time) the height of the projectile is the difference between the end height and 
		the initial height (call this dy_target).

		Plugging that into equation (i), we find v0
			v0 = dy_target / t_air - 1/2 a t_air
	*/
	const float dyTargetLaunch = targetPosition_w.y - hardpointPosition_w.y;
	m_yStartVelocity           = dyTargetLaunch / m_totalFlyTime - 0.5f * m_yAcceleration * m_totalFlyTime;

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool GrenadeLobAction::isDefenderHit(const PlaybackScript &script) const
{
	//-- Get our template.
	const GrenadeLobActionTemplate &actionTemplate = getOurTemplate();

	int damageAmount = 0;

	const bool gotDamageAmount = script.getIntVariable(actionTemplate.getIsHitVariable(), damageAmount);
	DEBUG_REPORT_LOG(!gotDamageAmount, ("GrenadeLobAction: failed to get the isHitVariable for defender, assuming not hit.\n"));
	UNREF(gotDamageAmount);

	return (damageAmount > 0);
}

// ----------------------------------------------------------------------

bool GrenadeLobAction::updateUninitialized(PlaybackScript &script)
{
	// Initialize the action.
	const bool initSuccess = initialize(script);
	if (!initSuccess)
	{
		DEBUG_WARNING(true, ("GrenadeLobAction failed to initialize, skipping action."));
		return false;
	}

	// Transition to lobbing state.
	m_state = S_lobbing;

	return true;
}

// ----------------------------------------------------------------------

bool GrenadeLobAction::updateLobbing(float deltaTime, PlaybackScript &script)
{
	//-- Skip this pass if no time has elapsed.
	if (deltaTime <= 0.0f)
	{
		// continue.
		return true;
	}

	// update destination and velocity for tracking grenades
	if(m_trackTarget)
	{
		GrenadeLobActionTemplate const & actionTemplate = getOurTemplate();
		Object const * const defenderObject = script.getActor(actionTemplate.getDefenderActorIndex());
		if(defenderObject)
		{
			//-- Find the target position based on the defender position.
			int tmp;
			const bool       throwAtCenter    = (script.getIntVariable(actionTemplate.getThrowAtCenterVariable(), tmp) && tmp > 0);
			const bool       hitStatus        = isDefenderHit(script);
			const Vector     targetPosition_w = getTargetPosition(*defenderObject, hitStatus, throwAtCenter, false);
			DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: target position in world space [(x=%.2f,y=%.2f,z=%.2f)].\n", targetPosition_w.x, targetPosition_w.y, targetPosition_w.z));

			//-- Save grenade destination info.
			m_grenadeDestinationCell = defenderObject->getParentCell();
			m_grenadeDestination     = targetPosition_w;

			if(m_showGrenadeDuringFlight && m_grenadeWatcher)
			{
				Object const * const grenadeObject = m_grenadeWatcher->getPointer();
				if (grenadeObject)
				{
					float const flyTimeLeft = m_totalFlyTime - m_accumulatedFlyTime;

					Vector const grenadePosition_w = grenadeObject->getPosition_w();

					m_xzVelocity = targetPosition_w - grenadePosition_w;
					m_xzVelocity.y = 0.0f;
					if(flyTimeLeft > FLT_EPSILON)
					{
						m_xzVelocity /= flyTimeLeft;
					}
				}
			}
		}
	}

	//-- Determine time to apply, capping at the max lob time.
	const float timeToApply = std::min(deltaTime, m_totalFlyTime - m_accumulatedFlyTime);
	m_accumulatedFlyTime += timeToApply;

	// Keep track of whether we should transition to the landed state next round.
	if (timeToApply < deltaTime)
	{
		m_state              = S_landed;

		//-- Place the grenade at its resting spot.
		if (m_showGrenadeDuringFlight)
		{
			Object *const grenadeObject = m_grenadeWatcher->getPointer();
			if (grenadeObject)
				grenadeObject->setPosition_w(m_grenadeDestination);
		}
	}

	//-- Update grenade visuals.
	if (m_showGrenadeDuringFlight)
	{
		//-- Compute the value of y according to equation (i) documented in the intialization function.
		const float deltaY = 0.5f * (2.0f * m_yStartVelocity + m_yAcceleration * m_accumulatedFlyTime) * m_accumulatedFlyTime;

		//-- Calculate change to x and z position.
		const Vector deltaXZ = m_xzVelocity * timeToApply;

		//-- Apply position changes to the object.
		NOT_NULL(m_grenadeWatcher);

		Object *const grenadeObject = m_grenadeWatcher->getPointer();
		if (!grenadeObject)
		{
			DEBUG_WARNING(true, ("Grenade object is NULL, must have been deleted, terminating action."));
			return false;
		}

		Vector position  = grenadeObject->getPosition_w();
		position        += deltaXZ;
		position.y       = m_yStartPosition + deltaY;

		grenadeObject->setPosition_w(position);

#ifdef _DEBUG
		if (ConfigClientGame::getLogGrenadeLobAction())
		{
			const Vector grenadePosition_w = grenadeObject->getPosition_w();
			DEBUG_REPORT_PRINT(true, ("GLA: grenade at x=%8.2f y=%8.2f z=%8.2f", grenadePosition_w.x, grenadePosition_w.y, grenadePosition_w.z));
		}
#endif

		//-- Adjust object pitch and yaw to create tumble effect.
		const float pitchAngle = m_grenadePitchSpeed * deltaTime;
		grenadeObject->pitch_o(pitchAngle);

		const float yawAngle = m_grenadeYawSpeed * deltaTime;
		grenadeObject->yaw_o(yawAngle);
	}

	return true;
}

// ----------------------------------------------------------------------

bool GrenadeLobAction::updateLanded(float deltaTime)
{
	//-- Skip this pass if no time has elapsed.
	if (deltaTime <= 0.0f)
	{
		// continue.
		return true;
	}

	//-- Determine time to apply, capping at the max sit time.
	const float timeToApply = std::min(deltaTime, m_totalSitTime - m_accumulatedSitTime);
	m_accumulatedSitTime += timeToApply;

	// Keep track of whether we sit time is done
	if (timeToApply >= deltaTime)
	{
		return true;//not done sitting yet
	}

	//-- Blow up the grenade.
	// Look up the name of the explode effect on the grenade.
	if (!m_grenadeExplodeEffectName || (strlen(m_grenadeExplodeEffectName->getString()) == 0))
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: skipping grenade explosion because no explode effect is specified.\n"));
		return false;
	}

	//-- Get the effect position.  The grenade destination is in world space, but we need effect position in the cell space.
	NOT_NULL(m_grenadeDestinationCell);
	DEBUG_WARNING(&m_grenadeDestinationCell->getOwner() == NULL, ("GLA: UpdateLanded: Grenade Destination Cell had an invalid owner. Cell Name [%s] Cell Index [%d]", m_grenadeDestinationCell->getCellName(), m_grenadeDestinationCell->getCellIndex()));
	
	
	if (m_showGrenadeDuringFlight)
	{
		//-- Get the grenade object.
		NOT_NULL(m_grenadeWatcher);
		Object *const grenadeObject = m_grenadeWatcher->getPointer();

		if (!grenadeObject)
		{
			DEBUG_WARNING(true, ("Grenade object is NULL, must have been deleted, terminating action."));
			return false;
		}

		// Delete the grenade.
		grenadeObject->kill();
		m_grenadeDeleted = true;
	}

	if(&m_grenadeDestinationCell->getOwner() != NULL)
	{
		const Vector effectPosition = m_grenadeDestinationCell->getOwner().rotateTranslate_w2o(m_grenadeDestination);
		//-- Log the world position of the grenade explosion.
		DEBUG_REPORT_LOG(ConfigClientGame::getLogGrenadeLobAction(), ("GLA: blowing up grenade now at cell coordinates (x=%0.2f, y=%0.2f, z=%0.2f).\n", effectPosition.x, effectPosition.y, effectPosition.z));
		IGNORE_RETURN(ClientEffectManager::playClientEffect(*m_grenadeExplodeEffectName, m_grenadeDestinationCell, effectPosition, Vector::unitY));
	}
	
	// We're done.
	return false;
}

// ----------------------------------------------------------------------

void GrenadeLobAction::notificationCallback(int channel, CrcString const &animationMessage, float /* elapsedTimeSinceTrigger */)
{
	if ((channel == 0) && (animationMessage == m_throwAnimationMessageName))
		m_receivedThrowMessage = true;
}

// ======================================================================
