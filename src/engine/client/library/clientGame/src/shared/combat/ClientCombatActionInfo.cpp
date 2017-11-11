// ======================================================================
//
// ClientCombatActionInfo.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCombatActionInfo.h"

// @todo: youch, this is exactly what I've been trying to avoid!  Get this out of here.
#include "swgSharedUtility/Postures.def"

#include <algorithm>

// ======================================================================

// NOTE: this number must match the number of postures defined by the game system (i.e. server).
// @todo fix, this is broken.  This can't be shared code if this value needs to change per
//       server combat system.  Perhaps configure this.
const int ClientCombatActionInfo::cms_postureCount = Postures::NumberOfPostures;

// ======================================================================

int ClientCombatActionInfo::getPostureCount()
{
	return cms_postureCount;
}

// ======================================================================

ClientCombatActionInfo::ClientCombatActionInfo(
	Object          *attacker, 
	Object          *attackerWeapon,
	int              attackerPostureEnd, 
	int              attackerTrailBits,
	int              attackerClientEffectId,
	int              attackerActionNameCrc,
	bool             attackerUseLocation,
	const Vector    &attackerTargetLocation,
	const NetworkId &attackerTargetCell,
	uint32           attackerActionId, 
	Object          *defender, 
	int              defenderPostureEnd, 
	DefenderDefense  defenderDefense,
	int              defenderClientEffectId,
	int              defenderHitLocation,
	int              damageAmount
	) :
	m_attacker(attacker),
	m_attackerWeapon(attackerWeapon),
	m_attackerPostureEnd(std::max(attackerPostureEnd, 0)), // Note: There's still cases where a -1 will come in for non-creatures.  I'd like to track down the initial cause of the -1 and remove this max call.
	m_attackerTrailBits(attackerTrailBits),
	m_attackerClientEffectId(attackerClientEffectId),
	m_attackerActionNameCrc(attackerActionNameCrc),
	m_attackerUseLocation(attackerUseLocation),
	m_attackerTargetLocation(attackerTargetLocation),
	m_attackerTargetCell(attackerTargetCell),
	m_attackerActionId(attackerActionId),
	m_defender(defender),
	m_defenderPostureEnd(std::max(defenderPostureEnd, 0)), // Note: There's still cases where a -1 will come in for non-creatures.  I'd like to track down the initial cause of the -1 and remove this max call.
	m_defenderDefense(defenderDefense),
	m_defenderClientEffectId(defenderClientEffectId),
	m_defenderHitLocation(defenderHitLocation),
	m_damageAmount(damageAmount)
{
}

// ======================================================================
