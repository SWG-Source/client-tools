// ======================================================================
//
// WaitForBoltAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WaitForBoltAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/CreatureController.h"
#include "clientGame/WaitForBoltActionTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <algorithm>

Tag const TAG_PVEL = TAG(P,V,E,L);

// ======================================================================
// inlines
// ======================================================================

inline const WaitForBoltActionTemplate &WaitForBoltAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const WaitForBoltActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// class WaitForBoltAction: public member functions
// ======================================================================

WaitForBoltAction::WaitForBoltAction(const WaitForBoltActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initializedPlaybackData(false),
	m_timeToLive(0.0f)
{
}

// ----------------------------------------------------------------------

bool WaitForBoltAction::update(float deltaTime, PlaybackScript &script)
{
	const WaitForBoltActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the actor to rotate.
	Object *const attackerObject = script.getActor(actionTemplate.getAttackerActorIndex());
	if (!attackerObject)
		return false;

	//-- Get target actor.  If target isn't available, abort the action.
	Object *const defenderObject = script.getActor(actionTemplate.getDefenderActorIndex());
	if (!defenderObject)
		return false;

	//-- Initialize the action first time through.
	if (!m_initializedPlaybackData)
	{
		Vector attackerPosition = attackerObject->getPosition_w();
		Vector defenderPosition = defenderObject->getPosition_w();
		float distance = attackerPosition.magnitudeBetween(defenderPosition);
		if(actionTemplate.getSubtractInitialWait())
		{
			distance -= 10.0f;
			if(distance < 0.0f) distance = 0.0f;
		}
		// PVEL float variable

		float projectileVelocity;
		if(!script.getFloatVariable(TAG_PVEL, projectileVelocity))
			projectileVelocity = 55.0f;
		m_timeToLive = distance / projectileVelocity;	
		
		m_initializedPlaybackData = true;
	}

	//-- Check if time to live has expired.
	m_timeToLive -= deltaTime;
	
	//-- Continue running if time to live has not expired.
	return (m_timeToLive > 0.0f);
}

// ======================================================================
