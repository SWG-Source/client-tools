// ======================================================================
//
// HardpointTargetAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/HardpointTargetAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/HardpointTargetActionTemplate.h"
#include "clientGame/CreatureController.h"
#include "clientObject/HardpointObject.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/CrcLowerString.h"


// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(HardpointTargetAction, true, 0, 0, 0);

// ======================================================================
// class HardpointTargetAction: inlines
// ======================================================================

inline const HardpointTargetActionTemplate *HardpointTargetAction::getOurTemplate() const
{
	return safe_cast<const HardpointTargetActionTemplate*>(getPlaybackActionTemplate());
}

// ======================================================================
// class HardpointTargetAction: public member functions
// ======================================================================

bool HardpointTargetAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(script);
	UNREF(deltaTime);

	if (!m_targetCreated)
	{
		DEBUG_FATAL(m_targetWatcher, ("Target watcher is set but it shouldn't be.  Logic error."));

		// Create the target, attach it to the defender.

		//-- Construct target.
		m_targetWatcher = createTarget(script);
		m_targetCreated = true;

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
	if (m_targetWatcher)
	{
		// Get the arc target Object.
		Object *const overrideTarget = m_targetWatcher.getPointer();
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

	//-- This action completes when the target is destroyed.
	return (m_targetWatcher.getPointer() != 0);
}

// ----------------------------------------------------------------------

void HardpointTargetAction::stop(PlaybackScript &script)
{
	UNREF(script);
	killTarget();
}

// ----------------------------------------------------------------------

void HardpointTargetAction::cleanup(PlaybackScript &playbackScript)
{
	UNREF(playbackScript);
	killTarget();
}

// ======================================================================
// class HardpointTargetAction: private member functions
// ======================================================================

HardpointTargetAction::HardpointTargetAction(const HardpointTargetActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_targetCreated(false),
	m_targetWatcher(0),
	m_hasCreatureController(false)
{
}

// ----------------------------------------------------------------------

HardpointTargetAction::~HardpointTargetAction()
{
	killTarget();
}

// ----------------------------------------------------------------------

Object *HardpointTargetAction::getAttacker(PlaybackScript &script) const
{
	//-- Get the action template.
	const HardpointTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getAttackerActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

Object *HardpointTargetAction::getDefender(PlaybackScript &script) const
{
	//-- Get the action template.
	const HardpointTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
		return 0;

	//-- Get the attacker index.
	const int actorIndex = actionTemplate->getDefenderActorIndex();
	return script.getActor(actorIndex);
}

// ----------------------------------------------------------------------

CreatureController *HardpointTargetAction::getAttackerController(PlaybackScript &script) const
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

void HardpointTargetAction::getHardpointName(const PlaybackScript &script, CrcString &hardpointName) const
{
	//-- Get the action template.
	const HardpointTargetActionTemplate *const actionTemplate = getOurTemplate();
	if (!actionTemplate)
	{
		DEBUG_WARNING(true, ("getHardpointName(): PlaybackActionTemplate is NULL."));
		hardpointName.set("", false);
		return;
	}
	
	//-- Get hardpoint name from playback script.
	// @todo make this more efficient.
	std::string  hardpointNameStdString;

	if (script.getStringVariable(actionTemplate->getHardpointNameVariable(), hardpointNameStdString))
		hardpointName.set(hardpointNameStdString.c_str(), false);
	else
	{
		DEBUG_WARNING(true, ("getHardpointName(): failed to find hardpoint variable in PlaybackScript."));
		hardpointName.set("", false);
	}
}

// ----------------------------------------------------------------------

Object *HardpointTargetAction::createTarget(PlaybackScript &script) const
{
	//-- Get attacker and defender.
	Object *defender = getDefender(script);
	if (!defender)
		return 0;

	//-- Get attachment hardpoint.
	PersistentCrcString  hardpointName;
	getHardpointName(script, hardpointName);

	//-- Create target object.
	Object *const targetObject = new HardpointObject(hardpointName);
	defender->addChildObject_o(targetObject);

	return targetObject;
}

// ----------------------------------------------------------------------

void HardpointTargetAction::killTarget()
{
	if (m_targetCreated)
	{
		Object *const targetObject = m_targetWatcher.getPointer();
		if (targetObject != NULL)
		{
			targetObject->kill();
			targetObject->scheduleForAlter();

			m_targetWatcher = NULL;
		}
	}
}

// ======================================================================
