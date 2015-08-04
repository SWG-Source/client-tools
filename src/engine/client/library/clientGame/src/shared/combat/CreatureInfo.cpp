// ======================================================================
//
// CreatureInfo.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CreatureInfo.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"

#include <map>

// ======================================================================

namespace CreatureInfoNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<NetworkId, CreatureInfo::PostureState>  IdPostureStateMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const *s_postureStateNames[] =
		{
			"PS_incapacitatedAtEndBaselines",
			"PS_deadAtEndBaselines",
			"PS_incapacitatedBySetPostureMessage",
			"PS_deadBySetPostureMessage",
			"PS_incapacitatedByPlayAnimationAction",
			"PS_deadByPlayAnimationAction",
			"PS_incapacitatedByCombatManagerAsAttacker",
			"PS_deadByCombatManagerAsAttacker",
			"PS_incapacitatedByCombatManagerAsDefender",
			"PS_deadByCombatManagerAsDefender"
		};

	int const   s_postureStateNameCount = sizeof(s_postureStateNames)/sizeof(s_postureStateNames[0]);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool               s_installed;
	IdPostureStateMap *s_idPostureStateMap;

	bool               s_log;

}

using namespace CreatureInfoNamespace;

// ======================================================================
// namespace CreatureInfoNamespace;
// ======================================================================

void CreatureInfoNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("CreatureInfo not installed."));
	s_installed = false;

	WARNING(!s_idPostureStateMap->empty(), ("CreatureInfo: entries for [%d] creatures were not removed.", static_cast<int>(s_idPostureStateMap->size())));

	delete s_idPostureStateMap;
	s_idPostureStateMap = 0;
}

// ======================================================================
// class CreatureInfo: PUBLIC STATIC
// ======================================================================

void CreatureInfo::install()
{
	InstallTimer const installTimer("CreatureInfo::install");

	DEBUG_FATAL(s_installed, ("CreatureInfo already installed."));

	s_idPostureStateMap = new IdPostureStateMap;
	DebugFlags::registerFlag(s_log, "ClientGame/Combat", "logCreatureInfo");

	s_installed = true;
	ExitChain::add(remove, "CreatureInfo");
}

// ----------------------------------------------------------------------

bool CreatureInfo::setCreaturePostureState(NetworkId const &creatureId, PostureState newState)
{
	DEBUG_FATAL(!s_installed, ("CreatureInfo not installed."));

	//-- Check if first dead/incap posture state is already set.
	bool          hasState;
	PostureState  oldState;

	getCreaturePostureState(creatureId, hasState, oldState);
	if (hasState)
	{
		//-- Creature's first death/incap posture state has been set already.
		DEBUG_REPORT_LOG(s_log, ("CreatureInfo::setCreaturePostureState(): tried to reset id [%s]'s first dead/incap state to [%s] when already set to [%s].\n", creatureId.getValueString().c_str(), getPostureStateAsCharConst(newState), getPostureStateAsCharConst(oldState)));
		return false;
	}

	//-- Set the state.
	IGNORE_RETURN(s_idPostureStateMap->insert(IdPostureStateMap::value_type(creatureId, newState)));

#ifdef _DEBUG
	if (s_log)
	{
		Object const *const object             = NetworkIdManager::getObjectById(creatureId);
		char const *const   objectTemplateName = (object ? object->getObjectTemplateName() : "<NULL object template name>");
		DEBUG_REPORT_LOG(true, ("CreatureInfo::setCreaturePostureState(): setting id [%s], type [%s] to [%s].\n", creatureId.getValueString().c_str(), objectTemplateName, getPostureStateAsCharConst(newState)));
	}
#endif

	return true;
}

// ----------------------------------------------------------------------

void CreatureInfo::getCreaturePostureState(NetworkId const &creatureId, bool &hasState, PostureState &state)
{
	DEBUG_FATAL(!s_installed, ("CreatureInfo not installed."));

	IdPostureStateMap::iterator findIt = s_idPostureStateMap->find(creatureId);
	if (findIt == s_idPostureStateMap->end())
	{
		// Not found.
		hasState = false;
	}
	else
	{
		hasState = true;
		state    = findIt->second;
	}
}

// ----------------------------------------------------------------------

void CreatureInfo::removeCreature(NetworkId const &creatureId)
{
	DEBUG_FATAL(!s_installed, ("CreatureInfo not installed."));

	//-- Remove any info on the specified network id.  It's okay if it's not present.
	IdPostureStateMap::iterator findIt = s_idPostureStateMap->find(creatureId);
	if (findIt != s_idPostureStateMap->end())
		s_idPostureStateMap->erase(findIt);
}

// ----------------------------------------------------------------------

char const *CreatureInfo::getPostureStateAsCharConst(PostureState state)
{
	DEBUG_FATAL(!s_installed, ("CreatureInfo not installed."));

	unsigned index = static_cast<unsigned>(state);
	if (index >= s_postureStateNameCount)
	{
		DEBUG_WARNING(s_log, ("CreatureInfo::getPostureStateAsCharConst(): passed out-of-range index [%u].", index));
		return "<invalid posture state enumeration>";
	}

	return s_postureStateNames[index];
}

// ======================================================================
