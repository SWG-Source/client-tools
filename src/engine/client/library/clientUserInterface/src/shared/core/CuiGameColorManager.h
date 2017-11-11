//======================================================================
//
// CuiGameColorManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiGameColorManager_H
#define INCLUDED_CuiGameColorManager_H

class NetworkId;
class TangibleObject;

struct UIColor;

//======================================================================

class CuiGameColorManager
{
public:

	enum Type
	{
		T_default,
		T_player,
		T_group,
		T_groupLeader,
		T_npc,
		T_attackable,
		T_canAttackYou,
		T_canAttackYouDark,
		T_willBeDeclared,
		T_wasDeclared,
		T_faction,
		T_factionNpc,
		T_target,
		T_combatTarget,
		T_intendedTarget,
		T_corpse,
		T_yourCorpse,
		T_turretTarget,
		T_miningAsteroidStatic,
		T_miningAsteroidDynamic,
		T_assist,
		T_spaceWillBeDeclared,
		T_numTypes
	};

	static Type            findTypeForObject  (const TangibleObject & obj, bool ignoreTarget = false);
	static UIColor const   findColorForObject (const TangibleObject & obj, bool ignoreTarget, bool allowOverride = false);
	static const UIColor & getColorForType    (Type type);
	static float           getRangeModForType (Type type);
	static bool            isTargetType       (Type type);
	static void update(float const deltaTimeSeconds);
	static UIColor const getCombatColor();
	static bool            objectHasOverrideColor(const TangibleObject & obj);
};

//----------------------------------------------------------------------

inline bool CuiGameColorManager::isTargetType       (Type type)
{
	return type == T_target || type == T_combatTarget || type == T_intendedTarget;
}

//======================================================================

#endif
