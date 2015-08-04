// ======================================================================
//
// CombatEffectsManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CombatEffectsManager_H
#define INCLUDED_CombatEffectsManager_H

#include "clientGame/WeaponObject.h"

class CellProperty;
class CrcLowerString;
class Object;
class Projectile;
class Vector;
class Appearance;
class VectorArgb;

// ----------------------------------------------------------------------
class CombatEffectsManager
{
public:

	static void install();

	static const VectorArgb & getWeaponDamageTypeColor(WeaponObject const & weaponObject);
	static const VectorArgb & getWeaponDamageTypeColor(WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType);
	static const CrcLowerString & getHitLocationHardPointName(const Object & targetObject, const int hitLocation);
	static void getSwooshEffect(WeaponObject & weaponObject, std::string &outSwoosh1Filename, VectorArgb& outColor);

	static void createMeleeSwingEffect(WeaponObject & weaponObject);
	static void createMeleeHitEffect(WeaponObject & weaponObject, CrcLowerString const & weaponHardPoint, Object const * const targetObject, int actionNameCrc);

	static void createSpecialAttackEffect(Object& targetObject, int actionNameCrc);
	
	static void createMuzzleFlash(WeaponObject & weaponObject, CrcLowerString const & hardPoint);
	static void attachProjectileAppearance(WeaponObject const & weaponObject, Object & parentProjectile);
	static void addProjectileSpecialAttackEffect(Object &projectile, uint32 attackCrc, WeaponObject const &weaponObject);
	static void createHitEffect(CellProperty & cell, Vector const & position_l, Vector const & normal_l, std::string & weaponObjectTemplateName, WeaponObject::WeaponType weaponType, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType, int actionNameCrc, bool ricochet);
	static void createMissEffect(CellProperty & cell, Vector const & position_l, Vector const & normal_l, std::string & weaponObjectTemplateName, WeaponObject::WeaponType weaponType, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType);

	static void createFlyByEffect(Object& projectile, std::string const & weaponObjectTemplateName, WeaponObject::WeaponType weaponType, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType);

	static bool actionHasSpecialAttackEffect(int actionNameCrc);


private:

	// Disabled

	CombatEffectsManager();
	CombatEffectsManager(CombatEffectsManager const &);
	CombatEffectsManager & operator =(CombatEffectsManager const &);
	~CombatEffectsManager();
};

// ======================================================================

#endif	// INCLUDED_CombatEffectsManager_H
