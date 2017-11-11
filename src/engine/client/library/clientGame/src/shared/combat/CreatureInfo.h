// ======================================================================
//
// CreatureInfo.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CreatureInfo_H
#define INCLUDED_CreatureInfo_H

// ======================================================================

class NetworkId;

// ======================================================================

class CreatureInfo
{
public:

	enum PostureState
	{
		PS_incapacitatedAtEndBaselines,
		PS_deadAtEndBaselines,
		PS_incapacitatedBySetPostureMessage,
		PS_deadBySetPostureMessage,
		PS_incapacitatedByPlayAnimationAction,
		PS_deadByPlayAnimationAction,
		PS_incapacitatedByCombatManagerAsAttacker,
		PS_deadByCombatManagerAsAttacker,
		PS_incapacitatedByCombatManagerAsDefender,
		PS_deadByCombatManagerAsDefender,
	};

public:

	static void        install();

	static bool        setCreaturePostureState(NetworkId const &creatureId, PostureState newState);
	static void        getCreaturePostureState(NetworkId const &creatureId, bool &hasState, PostureState &state);

	static void        removeCreature(NetworkId const &creatureId);

	static char const *getPostureStateAsCharConst(PostureState state);

};

// ======================================================================

#endif
