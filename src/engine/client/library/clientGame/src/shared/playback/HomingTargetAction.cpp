// ======================================================================
//
// HomingTargetAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/HomingTargetAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/HomingTargetActionTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureController.h"
#include "clientObject/TimerObject.h"
#include "sharedFoundation/Watcher.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

// ======================================================================
// class HomingTargetAction: inlines
// ======================================================================

inline const HomingTargetActionTemplate *HomingTargetAction::getOurTemplate() const
{
	return safe_cast<const HomingTargetActionTemplate*>(getPlaybackActionTemplate());
}

// ======================================================================
// class HomingTargetAction: public member functions
// ======================================================================

bool HomingTargetAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(script);
	UNREF(deltaTime);

	if (m_targetCreated)
	{
		//-- Update fraction between attacker and defender at which to place the target.
		m_targetFraction += deltaTime * m_targetFraction;
	}
	else
	{
		DEBUG_FATAL(m_targetWatcher, ("target watcher is set but it shouldn't be.  Logic error."));

		// Create the target, send it on its way.

		//-- Construct arc target.
		Object *const target = createTarget(script);

		//-- Build watcher for target.
		if (target)
			m_targetWatcher = new Watcher<Object>(target);

		//-- Remember that the arc target has been created.
		m_targetCreated = true;

		//-- Initialize the start fraction.
		m_targetFraction = getStartDistanceFraction(script);

		//-- Calculate differential d(targetFraction)/d(time).
		m_deltaTargetFractionPerTime = calculateDeltaTargetFraction(script);

		//-- Check if the attacker object has a creature controller.  This allows me to
		//   skip some dynamic casts later on.
		Object *const attacker = getAttacker(script);
		if (attacker)
		{
			Controller *const controller = dynamic_cast<CreatureController*>(attacker->getController());
			m_hasCreatureController      = (controller != 0);
		}
	}

	//-- Perform target maintenance.
	if (m_targetWatcher)
	{
		//-- Update target object's position.
		updateTargetPosition(script);

		//-- Override the attacker actor's target.
		// Get the arc target Object.
		Object *const overrideTarget = m_targetWatcher->getPointer();
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
	//   (1) the object wasn't created for some reason, in which case it is null, or
	//   (2) the target object dies (should be due to a timer expiration).  This is the normal expected exit condition.
	return (m_targetWatcher != 0) && (m_targetWatcher->getPointer() != 0);
}

// ======================================================================
// class HomingTargetAction: private member functions
// ======================================================================

HomingTargetAction::HomingTargetAction(const HomingTargetActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_targetCreated(false),
	m_targetWatcher(0),
	m_hasCreatureController(false),
	m_targetFraction(0.0f),
	m_deltaTargetFractionPerTime(1.0f)
{
}

// ----------------------------------------------------------------------

HomingTargetAction::~HomingTargetAction()
{
	delete m_targetWatcher;
}

// ----------------------------------------------------------------------

Object *HomingTargetAction::getAttacker(PlaybackScript &script) const
{
	//-- Get the action template.
	const HomingTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getAttackerActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

Object *HomingTargetAction::getDefender(PlaybackScript &script) const
{
	//-- Get the action template.
	const HomingTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getDefenderActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

CreatureController *HomingTargetAction::getAttackerController(PlaybackScript &script) const
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

float HomingTargetAction::getTravelTime(const PlaybackScript &script) const
{
	//-- Get the action template.
	const HomingTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get time to live from script.
	float  ttl;
	if (script.getFloatVariable(actionTemplate->getTravelTimeVariable(), ttl))
		return ttl;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

float HomingTargetAction::getStartDistanceFraction(const PlaybackScript &script) const
{
	//-- Get the action template.
	const HomingTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get fraction from script.
	float  fraction;
	if (script.getFloatVariable(actionTemplate->getStartDistanceFractionVariable(), fraction))
		return fraction;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------

float HomingTargetAction::getEndDistanceFraction(const PlaybackScript &script) const
{
	//-- Get the action template.
	const HomingTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0.0f;
	
	//-- Get fraction from script.
	float  fraction;
	if (script.getFloatVariable(actionTemplate->getEndDistanceFractionVariable(), fraction))
		return fraction;
	else
		return 1.0f;
}

// ----------------------------------------------------------------------

float HomingTargetAction::calculateDeltaTargetFraction(const PlaybackScript &script) const
{
	const float scriptTravelTime      = getTravelTime(script);
	const float travelTime            = (scriptTravelTime > 0.0f) ? scriptTravelTime : 1.0f;

	const float startDistanceFraction = getStartDistanceFraction(script);
	const float endDistanceFraction   = getEndDistanceFraction(script);

	return (endDistanceFraction - startDistanceFraction) / travelTime;
}

// ----------------------------------------------------------------------

Object *HomingTargetAction::createTarget(const PlaybackScript &script) const
{
	//-- Get the # seconds for which the arc target should live.
	const float targetTtl = getTravelTime(script);

	//-- Create the arc target Object.
	Object *const target = new TimerObject(targetTtl);
	NOT_NULL(target);

	// Ensure target object goes on the intangible list.
	target->addNotification(ClientWorld::getIntangibleNotification());

	//-- Add target to world.
	ClientWorld::queueObject(target); //lint !e1060 // protected object cannot be accessed // huh?

	return target;
}

// ----------------------------------------------------------------------

void HomingTargetAction::updateTargetPosition(PlaybackScript &script) const
{
	//-- Ensure target watcher has been created.
	NOT_NULL(m_targetWatcher);

	//-- Get the target.
	Object *const target = m_targetWatcher->getPointer();
	if (!target)
	{
		//-- Nothing to do.
		return;
	}

	//-- Get attacker and defender.
	Object *attacker = getAttacker(script);
	if (!attacker)
	{
		// Nothing to do.
		return;
	}

	Object *defender = getDefender(script);
	if (!defender)
	{
		// Nothing to do.
		return;
	}

	//-- Get world-space positions for attacker and defender.
	//@todo convert to attacker cell coordinates if we ever want to think about
	//      rendering a debug shape for this.
	const Vector defenderPosition_w = defender->getPosition_w();
	const Vector attackerPosition_w = attacker->getPosition_w();

	//-- Calculate target's world-space coordinates.
	const float defenderFraction = m_targetFraction;
	const float attackerFraction = 1.0f - defenderFraction;

	const Vector targetPosition_w(
		defenderPosition_w.x * defenderFraction + attackerPosition_w.x * attackerFraction,
		defenderPosition_w.y * defenderFraction + attackerPosition_w.y * attackerFraction,
		defenderPosition_w.z * defenderFraction + attackerPosition_w.z * attackerFraction);

	//-- Set target's world-space coordinates.
	target->setPosition_w(targetPosition_w);
}

// ======================================================================
