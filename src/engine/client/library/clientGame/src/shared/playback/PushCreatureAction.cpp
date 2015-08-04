// ======================================================================
//
// PushCreatureAction.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PushCreatureAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PushCreatureActionTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/MessageQueuePushCreature.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"

#include <algorithm>

// ======================================================================
// inlines
// ======================================================================

inline const PushCreatureActionTemplate &PushCreatureAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const PushCreatureActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class PushCreatureAction: public member functions
// ======================================================================

PushCreatureAction::PushCreatureAction(const PushCreatureActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initializedPlaybackData(false),
	m_offset(),
	m_totalTime(0.5f),
	m_time(0)
{
}

// ----------------------------------------------------------------------

bool PushCreatureAction::update(float deltaTime, PlaybackScript &script)
{
	if (m_time < 0)
		return false;

	const PushCreatureActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the actor to push
	Object *const defender = script.getActor(actionTemplate.getDefenderActorIndex());
	if (!defender)
		return false;

	//-- Initialize the action first time through.
	if (!m_initializedPlaybackData)
	{
		//-- Retrieve values for script variables.
		Object *const attacker = script.getActor(actionTemplate.getAttackerActorIndex());
		if (!attacker)
			return false;

		float pushDistance = 0;
		bool success = script.getFloatVariable(actionTemplate.getPushDistanceVariable(), pushDistance);
		DEBUG_WARNING(!success, ("on target push distance variable missing."));

		success = script.getFloatVariable(actionTemplate.getPushTimeVariable(), m_totalTime);
		DEBUG_WARNING(!success, ("on target push time variable missing."));
		m_time = m_totalTime;

		// compute the offet vector
		const Vector defenderPos(defender->getPosition_w());
		const Vector attackerPos(attacker->getPosition_w());
		m_offset = defenderPos;
		m_offset -= attackerPos;
		m_offset.normalize();
		m_offset *= pushDistance;

		//-- Tell actor to ignore dead reckoning for a specified duration.  If this wasn't done,
		//   the movement supplied by this action would be overridden by dead reckoning.
		CreatureController * const controller = dynamic_cast<CreatureController *>(defender->getController());
		if (controller != NULL)
		{
			//-- Disable dead reckoning for the specified amount of time.
			controller->pauseDeadReckoning(m_totalTime + 0.5f);

			CreatureController * sendTo = NULL;
			if (Game::getConstPlayer()->getController() == controller)
				sendTo = controller;
			else if (defender->asClientObject()->asCreatureObject() != NULL && defender->asClientObject()->asCreatureObject()->getPlayerObject() == NULL &&
				Game::getConstPlayer()->getController() == attacker->getController())
			{
				sendTo = dynamic_cast<CreatureController *>(attacker->getController());
			}

			// the server is going to need to update the defender's position
			if (sendTo != NULL)
			{
				MessageQueuePushCreature * const msg = new MessageQueuePushCreature(attacker->getNetworkId(), defender->getNetworkId(), attackerPos, defenderPos, pushDistance);
				sendTo->appendMessage (CM_pushCreature, 0.0f, msg, 
					GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
			}
		}

		m_initializedPlaybackData = true;
	}

	m_time -= deltaTime;
	if (m_time < 0)
		deltaTime += m_time;
	if (deltaTime > 0)
	{
		Vector delta(m_offset);
		delta *= (deltaTime / m_totalTime);
		defender->move_p(defender->rotate_w2p(delta));
	}
	return true;
}

// ======================================================================
