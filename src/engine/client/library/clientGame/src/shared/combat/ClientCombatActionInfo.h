// ======================================================================
//
// ClientCombatActionInfo.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ClientCombatActionInfo_H
#define INCLUDED_ClientCombatActionInfo_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"

class Object;

// ======================================================================

class ClientCombatActionInfo
{
public:

	// NOTE: the values in this enum are significant and must match up with DD_* values in dsrc\include\ClientCombatPlaybackManagerDefines.h.
	//       Changes to these values will require rebuilding the combat_manager.mif file.
	enum DefenderDefense
	{
		DD_none = 0,
		DD_hit,
		DD_block,
		DD_evade,
		DD_redirect,
		DD_counterAttack,
		DD_fumble,
		DD_lightsaberBlock,   // block caused by a lightsaber (deflection)
		DD_lightsaberCounter, // counterattack caused by a lightsaber deflection.
		DD_lightsaberCounterTarget, // counterattack to jedi's target caused by a lightsaber deflection.
		// ---
		DD_count
	};

	
	enum TrailLocation
	{
		TL_lfoot  = BINARY2(0000,0001),
		TL_rfoot  = BINARY2(0000,0010),
		TL_lhand  = BINARY2(0000,0100),
		TL_rhand  = BINARY2(0000,1000),
		TL_weapon = BINARY2(0001,0000)
	};


public:

	static int getPostureCount();

public:

	ClientCombatActionInfo(
		Object          *attacker, 
		Object          *attackerWeapon,
		int              attackerPostureEnd, 
		int              attackerTrailBits,
		int              attackerClientEffectId,
		int              attackerActionNameCrc,     // crc of the combat action name (command name)
		bool             attackerUseLocation,
		const Vector    &attackerTargetLocation,
		const NetworkId &attackerTargetCell,
		uint32           attackerActionId,          // crc of the animation name
		Object          *defender, 
		int              defenderPostureEnd, 
		DefenderDefense  defenderDefense,
		int              defenderClientEffectId,
		int              defenderHitLocation,
		int              damageAmount
		);

	Object          *getAttacker() const;
	Object          *getAttackerWeapon() const;
	int              getAttackerPostureEnd() const;
	int              getAttackerTrailBits() const;
	int              getAttackerClientEffectId() const;
	int              getAttackerActionNameCrc() const;
	bool             getAttackerUseLocation() const;
	const Vector    &getAttackerTargetLocation() const;
	const NetworkId &getAttackerTargetCell() const;
	uint32           getAttackerActionId() const;

	Object          *getDefender() const;
	int              getDefenderPostureEnd() const;
	DefenderDefense  getDefenderDefense() const;
	int              getDefenderClientEffectId() const;
	int              getDefenderHitLocation() const;
	int              getDamageAmount() const;

private:

	// Disabled.
	ClientCombatActionInfo();
	ClientCombatActionInfo &operator =(const ClientCombatActionInfo&);

private:

	const static int cms_postureCount;

private:

	Object          *m_attacker;
	Object          *m_attackerWeapon;
	int              m_attackerPostureEnd;
	int              m_attackerTrailBits;
	int              m_attackerClientEffectId;
	int              m_attackerActionNameCrc;
	bool             m_attackerUseLocation;
	Vector           m_attackerTargetLocation;
	NetworkId        m_attackerTargetCell;
	uint32           m_attackerActionId;

	Object          *m_defender;
	int              m_defenderPostureEnd;
	DefenderDefense  m_defenderDefense;
	int              m_defenderClientEffectId;
	int              m_defenderHitLocation;
	int              m_damageAmount;
};

// ======================================================================
// class ClientCombatPlaybackManager::CombatActionInfo
// ======================================================================

inline Object *ClientCombatActionInfo::getAttacker() const
{
	return m_attacker;
} //lint !e1763 // const function indirectly modifies class // that's okay, this is a glorified structure.

// ----------------------------------------------------------------------

inline Object *ClientCombatActionInfo::getAttackerWeapon() const
{
	return m_attackerWeapon;
} //lint !e1763 // const function indirectly modifies class // that's okay, this is a glorified structure.

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getAttackerPostureEnd() const
{
	return m_attackerPostureEnd;
}

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getAttackerTrailBits() const
{
	return m_attackerTrailBits;
}

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getAttackerClientEffectId() const
{
	return m_attackerClientEffectId;
}

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getAttackerActionNameCrc() const
{
	return m_attackerActionNameCrc;
}

// ----------------------------------------------------------------------

inline bool ClientCombatActionInfo::getAttackerUseLocation() const
{
	return m_attackerUseLocation;
}

// ----------------------------------------------------------------------

inline const Vector &ClientCombatActionInfo::getAttackerTargetLocation() const
{
	return m_attackerTargetLocation;
}

// ----------------------------------------------------------------------

inline const NetworkId &ClientCombatActionInfo::getAttackerTargetCell() const
{
	return m_attackerTargetCell;
}

// ----------------------------------------------------------------------

inline uint32 ClientCombatActionInfo::getAttackerActionId() const
{
	return m_attackerActionId;
}

// ----------------------------------------------------------------------

inline Object *ClientCombatActionInfo::getDefender() const
{
	return m_defender;
} //lint !e1763 // const function indirectly modifies class // that's okay, this is a glorified structure.

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getDefenderPostureEnd() const
{
	return m_defenderPostureEnd;
}

// ----------------------------------------------------------------------

inline ClientCombatActionInfo::DefenderDefense ClientCombatActionInfo::getDefenderDefense() const
{
	return m_defenderDefense;
}

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getDefenderClientEffectId() const
{
	return m_defenderClientEffectId;
}

// ----------------------------------------------------------------------

inline int ClientCombatActionInfo::getDefenderHitLocation() const
{
	return m_defenderHitLocation;
}

inline int ClientCombatActionInfo::getDamageAmount() const
{
	return m_damageAmount;
}
// ======================================================================

#endif
