// ======================================================================
//
// ArcTargetAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ArcTargetAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ArcTargetActionTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureController.h"
#include "clientObject/TimerObject.h"
#include "sharedCollision/Extent.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Watcher.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/SimpleDynamics.h"
#include "sharedRandom/Random.h"
//#include "sharedObject/Object.h"

// ======================================================================
// class ArcTargetAction: inlines
// ======================================================================

inline const ArcTargetActionTemplate *ArcTargetAction::getOurTemplate() const
{
	return safe_cast<const ArcTargetActionTemplate*>(getPlaybackActionTemplate());
}

// ======================================================================
// class ArcTargetAction: public member functions
// ======================================================================

bool ArcTargetAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(script);
	UNREF(deltaTime);

	if (!m_arcTargetCreated)
	{
		DEBUG_FATAL(m_arcTargetWatcher, ("arc target watcher is set but it shouldn't be.  Logic error."));

		// Create the arc target, send it on its way.

		//-- Construct arc target.
		Object *const arcTarget = createArcTarget(script);

		//-- Build watcher for arc target.
		if (arcTarget)
			m_arcTargetWatcher = new Watcher<Object>(arcTarget);

		//-- Remember that the arc target has been created.
		m_arcTargetCreated = true;

		//-- Check if the attacker object has a creature controller.  This allows me to
		//   skip some dynamic casts later on.
		Object *const attacker = getAttacker(script);
		if (attacker)
		{
			Controller *const controller = dynamic_cast<CreatureController*>(attacker->getController());
			m_hasCreatureController = (controller != 0);
		}
	}

	//-- Override the attacker actor's target.
	if (m_arcTargetWatcher)
	{
		// Get the arc target Object.
		Object *const overrideTarget = m_arcTargetWatcher->getPointer();
		if (overrideTarget)
		{
			// Get the attacker controller.
			CreatureController *const controller = getAttackerController(script);
			if (controller)
			{
				// Set the override Object.
				controller->overrideAnimationTarget(overrideTarget, true, CrcLowerString::empty);
			}
		}
	}

	//-- This action is complete when either:
	//   (1) the arc object wasn't created for some reason, in which case it is null, or
	//   (2) the arc target object dies (should be due to a timer expiration).  This is the normal expected exit condition.
	return (m_arcTargetWatcher != 0) && (m_arcTargetWatcher->getPointer() != 0);
}

// ======================================================================
// class ArcTargetAction: private member functions
// ======================================================================

ArcTargetAction::ArcTargetAction(const ArcTargetActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_arcTargetCreated(false),
	m_arcTargetWatcher(0),
	m_hasCreatureController(false)
{
}

// ----------------------------------------------------------------------

ArcTargetAction::~ArcTargetAction()
{
	delete m_arcTargetWatcher;
}

// ----------------------------------------------------------------------

Object *ArcTargetAction::getAttacker(PlaybackScript &script) const
{
	//-- Get the action template.
	const ArcTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getAttackerActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

Object *ArcTargetAction::getDefender(PlaybackScript &script) const
{
	//-- Get the action template.
	const ArcTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getDefenderActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

CreatureController *ArcTargetAction::getAttackerController(PlaybackScript &script) const
{
	//-- Punt if the Object controller is not a creature controller.
	if (!m_hasCreatureController)
		return 0;

	//-- Get the attacker Object.
	Object *const attacker = getAttacker(script);
	if (!attacker)
		return 0;

	//-- Return the attacker's controller.
	return safe_cast<CreatureController*>(attacker->getController());
}

// ----------------------------------------------------------------------

float ArcTargetAction::getArcTargetTimeToLive(const PlaybackScript &script) const
{
	//-- Get the action template.
	const ArcTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get time to live from script.
	float  ttl;
	if (script.getFloatVariable(actionTemplate->getArcTimeVariable(), ttl))
		return ttl;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

float ArcTargetAction::getArcDistanceMultiplier(const PlaybackScript &script) const
{
	//-- Get the action template.
	const ArcTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get time to live from script.
	float  multiplier;
	if (script.getFloatVariable(actionTemplate->getArcDistanceMultipleVariable(), multiplier))
		return multiplier;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

float ArcTargetAction::getArcHeightMultiplier(const PlaybackScript &script) const
{
	//-- Get the action template.
	const ArcTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get time to live from script.
	float  multiplier;
	if (script.getFloatVariable(actionTemplate->getArcHeightMultipleVariable(), multiplier))
		return multiplier;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

Object *ArcTargetAction::createArcTarget(PlaybackScript &script) const
{
	//-- Get attacker and defender.
	Object *attacker = getAttacker(script);
	if (!attacker)
		return 0;

	Object *defender = getDefender(script);
	if (!defender)
		return 0;

	//-- Create world-space delta vector between attacker and defender.
	const Vector defenderPosition_w = defender->getAppearanceSphereCenter_w();
	const Vector attackerPosition_w = attacker->getAppearanceSphereCenter_w();

	Vector separationVector_w = defenderPosition_w - attackerPosition_w;
	IGNORE_RETURN(separationVector_w.normalize());

	//-- Create direction vector for arc fire direction.

	// Get defender up vector.
	const Vector defenderFrameJ_w = defender->getObjectFrameJ_w();

	// World-space direction of motion for arc target is perpendicular to the separation and defender up vector.
	Vector arcTargetDirection_w = defenderFrameJ_w.cross(separationVector_w);

	// Reverse the direction occassionally.
	if (Random::random(100) > 50)
		arcTargetDirection_w = -arcTargetDirection_w;

	//-- Get the # seconds for which the arc target should live.
	const float arcTargetTtl = getArcTargetTimeToLive(script);
	if (arcTargetTtl <= 0.0f)
		return 0;

	//-- Get the radius of the defender appearance.
	Appearance *const appearance = defender->getAppearance();
	if (!appearance)
		return 0;

	const Extent *const extent = appearance->getExtent();
	if (!extent)
		return 0;

	const float defenderRadius = extent->getSphere().getRadius();

	//-- Calculate the velocity at which the arc target should move to cover the desired distance in the specified amount of time.
	const float  arcTravelDistance = getArcDistanceMultiplier(script) * defenderRadius;
	const Vector arcVelocity_w     = (arcTravelDistance  / arcTargetTtl) * arcTargetDirection_w;

	//-- Calculate the starting position of the arc target.
	Vector arcStartPosition_w  = defenderPosition_w + ((getArcHeightMultiplier(script) * defenderRadius) * defenderFrameJ_w);
	arcStartPosition_w        -= ((0.5f * arcTravelDistance) * arcTargetDirection_w);

	//-- Create the arc target Object.
	Object *const arcTarget = new TimerObject(arcTargetTtl);
	NOT_NULL(arcTarget);

	// Ensure sequence object goes on the intangible list.
	arcTarget->addNotification(ClientWorld::getIntangibleNotification());

	// Set arc target start position.
	arcTarget->setPosition_w(arcStartPosition_w);

	//-- Set the dynamics for the arc target.
	//lint -esym(429, dynamics) // custodial pointer not freed or returned // it's okay, owned by Object
	SimpleDynamics *const dynamics = new SimpleDynamics(arcTarget);
	dynamics->setCurrentVelocity_w(arcVelocity_w);

	arcTarget->setDynamics(dynamics);

	//-- Add arc target to world.
	ClientWorld::queueObject(arcTarget); //lint !e1060 // protected object cannot be accessed // huh?

	// @todo finish this
	return arcTarget;
}

// ======================================================================
