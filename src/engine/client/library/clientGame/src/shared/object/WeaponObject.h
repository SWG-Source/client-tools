//===================================================================
//
// WeaponObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_WeaponObject_H
#define INCLUDED_WeaponObject_H

//===================================================================

class SharedWeaponObjectTemplate;

#include "clientGame/TangibleObject.h"

//===================================================================

class WeaponObject : public TangibleObject
{
public:

	// Duplicated from ServerObjectTemplate.h
	enum DamageType
	{
		DT_none = 0x00000000,
		DT_kinetic = 0x00000001,
		DT_energy = 0x00000002,
		DT_blast = 0x00000004,
		DT_stun = 0x00000008,
		DT_restraint = 0x00000010,
		DT_elemental_heat = 0x00000020,
		DT_elemental_cold = 0x00000040,
		DT_elemental_acid = 0x00000080,
		DT_elemental_electrical = 0x00000100,
		DT_environmental_heat = 0x00000200,
		DT_environmental_cold = 0x00000400,
		DT_environmental_acid = 0x00000800,
		DT_environmental_electrical = 0x00001000
	};

	// Duplicated from ServerWeaponObjectTemplate.h
	// @NOTE: Verify and update isRangedWeapon() and isMeleeWeapon() whenever this enum changes
	enum WeaponType
	{
		WT_rifle,
		WT_lightRifle,
		WT_pistol,
		WT_heavyWeapon,
		WT_1handMelee,
		WT_2handMelee,
		WT_unarmed,
		WT_polearm,
		WT_thrown,
		WT_1handLightsaber,
		WT_2handLightsaber,
		WT_polearmLightsaber,
		WT_groundTargetting,
		WT_directionalTargetting,
		WT_count
	};

	struct Messages
	{
		struct MinRangeChanged
		{
			typedef WeaponObject Payload;
		};
		struct MaxRangeChanged
		{
			typedef WeaponObject Payload;
		};
	};


	static char const * getDamageTypeString(DamageType const damageType);
	static char const * getWeaponTypeString(WeaponType const weaponType);

	static void install();

public:

	explicit WeaponObject (const SharedWeaponObjectTemplate* newTemplate);
	virtual ~WeaponObject ();

	virtual WeaponObject *         asWeaponObject();
	virtual WeaponObject const *   asWeaponObject() const;

	virtual void endBaselines ();

	float getAttackSpeed() const;
	float getMinRange() const;
	float getMaxRange() const;
	int   getAccuracy() const;
	DamageType getDamageType() const;
	WeaponType getWeaponType() const;
	DamageType getElementalType() const;
	int   getElementalValue() const;

	bool  isRangedWeapon () const;
	bool  isMeleeWeapon  () const;
	bool  isLightsaberWeapon() const;
	bool  isHeavyWeapon() const;
	bool  isGroundTargetting() const;
	bool  isDirectionalTargetting() const;

	static bool weaponTypeSatisfies(WeaponType weaponType, uint32 weaponTypesValid, uint32 weaponTypesInvalid);

private:

 	WeaponObject ();
	WeaponObject (const WeaponObject& rhs);
	WeaponObject&	operator= (const WeaponObject& rhs);

	struct Callbacks
	{
		template <typename S, typename U> struct DefaultCallback
		{
			void modified (WeaponObject & target, const U & old, const U & value, bool local) const;
		};

		typedef DefaultCallback<Messages::MinRangeChanged, float> MinRangeChanged;
		typedef DefaultCallback<Messages::MaxRangeChanged, float> MaxRangeChanged;
	};


private:
	Archive::AutoDeltaVariable<float> m_attackSpeed;
	Archive::AutoDeltaVariable<int>   m_accuracy;
	Archive::AutoDeltaVariableCallback<float,Callbacks::MinRangeChanged, WeaponObject> m_minRange;
	Archive::AutoDeltaVariableCallback<float,Callbacks::MaxRangeChanged, WeaponObject> m_maxRange;
	Archive::AutoDeltaVariable<int>   m_damageType;
	Archive::AutoDeltaVariable<int>   m_elementalType;
	Archive::AutoDeltaVariable<int>   m_elementalValue;
	Archive::AutoDeltaVariable<int>   m_weaponType;
};


//----------------------------------------------------------------------

inline float WeaponObject::getAttackSpeed() const
{
	return m_attackSpeed.get();
}

inline float WeaponObject::getMinRange() const
{
	return m_minRange.get();
}

inline float WeaponObject::getMaxRange() const
{
	return m_maxRange.get();
}

inline int WeaponObject::getAccuracy() const
{
	return m_accuracy.get();
}

inline int WeaponObject::getElementalValue() const
{
	return m_elementalValue.get();
}

//===================================================================

#endif
