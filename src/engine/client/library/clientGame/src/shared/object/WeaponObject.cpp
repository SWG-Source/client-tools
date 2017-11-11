//===================================================================
//
// WeaponObject.cpp
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WeaponObject.h"

#include "clientGame/ClientTangibleObjectTemplate.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"

//===================================================================

namespace WeaponObjectNamespace
{
	uint32 s_weaponTypeToBitValueMap[static_cast<int>(WeaponObject::WT_count)];
	uint32 s_installed = false;

	const uint32 B_RIFLE=				0x00000001;
	const uint32 B_CARBINE=				0x00000002;
	const uint32 B_PISTOL=				0x00000004;
	const uint32 B_HEAVY=				0x00000008;
	const uint32 B_1HAND_MELEE=			0x00000010;
	const uint32 B_2HAND_MELEE=			0x00000020;
	const uint32 B_UNARMED=				0x00000040;
	const uint32 B_POLEARM=				0x00000080;
	const uint32 B_THROWN=				0x00000100;
	const uint32 B_1HAND_LIGHTSABER=	0x00000200;
	const uint32 B_2HAND_LIGHTSABER=	0x00000400;
	const uint32 B_POLEARM_LIGHTSABER=	0x00000800;
	const uint32 B_GROUND_TARGETTING=	0x00001000;
	const uint32 B_DIRECTIONAL=	        0x00002000;
	const uint32 B_RANGED=				0x08000000;
	const uint32 B_MELEE=				0x10000000;
	const uint32 B_ALL=					0x20000000;
	const uint32 B_ALL_LIGHTSABERS=		0x40000000;

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const WeaponObject::Messages::MaxRangeChanged::Payload &,    WeaponObject::Messages::MaxRangeChanged>    maxRangeChanged;
		MessageDispatch::Transceiver<const WeaponObject::Messages::MinRangeChanged::Payload &,    WeaponObject::Messages::MinRangeChanged>    minRangeChanged;
	}
};

using namespace WeaponObjectNamespace;

void WeaponObject::install()
{
	
	//Bit values from command_table.tab:
	//
	// RIFLE=1,CARBINE=2,	PISTOL=3,HEAVY=4,	1HAND_MELEE=5,			2HAND_MELEE=6,	UNARMED=7,	POLEARM=8,	THROWN=9,
	// 1HAND_LIGHTSABER=10,	2HAND_LIGHTSABER=11,POLEARM_LIGHTSABER=12,	RANGED=28,		MELEE=29,	ALL=30,
	// ALL_LIGHTSABERS=31,	ALL_BUT_HEAVY=32
	s_installed = true;

	s_weaponTypeToBitValueMap[WT_rifle] =					B_ALL | B_RANGED			                       | B_RIFLE;
	s_weaponTypeToBitValueMap[WT_lightRifle] =				B_ALL | B_RANGED			                       | B_CARBINE;
	s_weaponTypeToBitValueMap[WT_pistol] =					B_ALL | B_RANGED			                       | B_PISTOL;
	s_weaponTypeToBitValueMap[WT_heavyWeapon] =				B_ALL | B_RANGED			                       | B_HEAVY;
	s_weaponTypeToBitValueMap[WT_1handMelee] =				B_ALL |            B_MELEE                         | B_1HAND_MELEE;
	s_weaponTypeToBitValueMap[WT_2handMelee] =				B_ALL |            B_MELEE                         | B_2HAND_MELEE;
	s_weaponTypeToBitValueMap[WT_unarmed] =					B_ALL |            B_MELEE                         | B_UNARMED;
	s_weaponTypeToBitValueMap[WT_polearm] =					B_ALL |            B_MELEE                         | B_POLEARM;
	s_weaponTypeToBitValueMap[WT_thrown] =					B_ALL | B_RANGED                                   | B_THROWN;
	s_weaponTypeToBitValueMap[WT_1handLightsaber] =			B_ALL |                          B_ALL_LIGHTSABERS | B_1HAND_LIGHTSABER;
	s_weaponTypeToBitValueMap[WT_2handLightsaber] =			B_ALL |                          B_ALL_LIGHTSABERS | B_2HAND_LIGHTSABER;
	s_weaponTypeToBitValueMap[WT_polearmLightsaber] =		B_ALL |                          B_ALL_LIGHTSABERS | B_POLEARM_LIGHTSABER;
	s_weaponTypeToBitValueMap[WT_groundTargetting] =		B_ALL | B_RANGED                                   | B_HEAVY | B_GROUND_TARGETTING;
	s_weaponTypeToBitValueMap[WT_directionalTargetting] =	B_ALL | B_RANGED                                   | B_HEAVY | B_DIRECTIONAL;
}

WeaponObject::WeaponObject(const SharedWeaponObjectTemplate* newTemplate) :
	TangibleObject(newTemplate),
	m_attackSpeed(0.0f),
	m_accuracy(0),
	m_minRange(0.0f),
	m_maxRange(0.0f),
	m_damageType(0),
	m_elementalType(0),
	m_elementalValue(0),
	m_weaponType(0)
{

	m_minRange.setSourceObject(this);
	m_maxRange.setSourceObject(this);

	addSharedVariable    (m_attackSpeed);
	addSharedVariable    (m_accuracy);
	addSharedVariable    (m_minRange);
	addSharedVariable    (m_maxRange);
	addSharedVariable    (m_damageType);
	addSharedVariable    (m_elementalType);
	addSharedVariable    (m_elementalValue);
	addSharedVariable_np (m_weaponType);
}

//-------------------------------------------------------------------
	
WeaponObject::~WeaponObject()
{
}

//-------------------------------------------------------------------

WeaponObject * WeaponObject::asWeaponObject()
{
	return this;
}

//-------------------------------------------------------------------

WeaponObject const * WeaponObject::asWeaponObject() const
{
	return this;
}

//-------------------------------------------------------------------

void WeaponObject::endBaselines ()
{
	//-- chain up
	TangibleObject::endBaselines ();

	//-- apply any child objects
#if 0
	std::vector<int> fakeComponents;
	fakeComponents.push_back (Crc::calculate ("stock_1"));
	fakeComponents.push_back (Crc::calculate ("scope_sm_2"));
	fakeComponents.push_back (Crc::calculate ("muzzle_sm_2"));
	ClientTangibleObjectTemplate::applyComponents (this, fakeComponents);
#else
	ClientTangibleObjectTemplate::applyComponents (this, getComponents ());
#endif
}

//-------------------------------------------------------------------
WeaponObject::DamageType WeaponObject::getDamageType() const
{
	return static_cast<DamageType>(m_damageType.get());
}

//-------------------------------------------------------------------
WeaponObject::WeaponType WeaponObject::getWeaponType() const
{
	return static_cast<WeaponType>(m_weaponType.get());
}

//-------------------------------------------------------------------
WeaponObject::DamageType WeaponObject::getElementalType() const
{
	return static_cast<DamageType>(m_elementalType.get());
}

//-------------------------------------------------------------------
char const * WeaponObject::getDamageTypeString(DamageType const damageType)
{
	switch (damageType)
	{
	case DT_none: { return "none"; }
		case DT_kinetic: { return "kinetic"; }
		case DT_energy: { return "energy"; }
		case DT_blast: { return "blast"; }
		case DT_stun: { return "stun"; }
		case DT_restraint: { return "restraint"; }
		case DT_elemental_heat: { return "elemental_heat"; }
		case DT_elemental_cold: { return "elemental_cold"; }
		case DT_elemental_acid: { return "elemental_acid"; }
		case DT_elemental_electrical: { return "elemental_electrical"; }
		case DT_environmental_heat: { return "environmental_heat"; }
		case DT_environmental_cold: { return "environmental_cold"; }
		case DT_environmental_acid: { return "environmental_acid"; }
		case DT_environmental_electrical: { return "environmental_electrical"; }
		default: { break; }
	}

	return "error";
}

//-------------------------------------------------------------------

char const * WeaponObject::getWeaponTypeString(WeaponType const weaponType)
{
	switch (weaponType)
	{
		case WT_rifle: { return "rifle"; }
		case WT_lightRifle: { return "lightRifle"; }
		case WT_pistol: { return "pistol"; }
		case WT_heavyWeapon: { return "heavyWeapon"; }
		case WT_1handMelee: { return "onehandMelee"; }
		case WT_2handMelee: { return "twohandMelee"; }
		case WT_unarmed: { return "unarmed"; }
		case WT_polearm: { return "polearm"; }
		case WT_thrown: { return "thrown"; }
		case WT_1handLightsaber: { return "onehandLightsaber"; }
		case WT_2handLightsaber: { return "twohandLightsaber"; }
		case WT_polearmLightsaber: { return "polearmLightsaber"; }
		case WT_groundTargetting: { return "groundTargetting"; }
		case WT_directionalTargetting: { return "directionalTargetting"; }
		default: { break; }
	}

	return "error";
}

//-------------------------------------------------------------------

bool WeaponObject::isRangedWeapon() const
{
	return ((s_weaponTypeToBitValueMap[m_weaponType.get()] & B_RANGED) != 0);
}

//-------------------------------------------------------------------

bool WeaponObject::isMeleeWeapon() const
{
	return ((s_weaponTypeToBitValueMap[m_weaponType.get()] & B_MELEE) != 0);
}

//-------------------------------------------------------------------

bool WeaponObject::isLightsaberWeapon() const
{
	return ((s_weaponTypeToBitValueMap[m_weaponType.get()] & B_ALL_LIGHTSABERS) != 0);
}

//-------------------------------------------------------------------

//The assumption is made here that direction and ground-targetting weapons are
//also, in most respects, heavy weapons, e.g. they use special commands as their
//default actions (see CreatureObject::getCurrentPrimaryActionName)
bool WeaponObject::isHeavyWeapon() const
{
	return ((s_weaponTypeToBitValueMap[m_weaponType.get()] & B_HEAVY) != 0);
}

//-------------------------------------------------------------------

bool WeaponObject::isGroundTargetting() const
{
	return (WT_groundTargetting == m_weaponType.get());
}

//-------------------------------------------------------------------

bool WeaponObject::isDirectionalTargetting() const
{
	return (WT_directionalTargetting == m_weaponType.get());
}

//-------------------------------------------------------------------

bool WeaponObject::weaponTypeSatisfies(WeaponType weaponType, uint32 weaponTypesValid, uint32 weaponTypesInvalid)
{	
	bool result = (!(weaponTypesInvalid & s_weaponTypeToBitValueMap[static_cast<int>(weaponType)]) 
		&& (weaponTypesValid & s_weaponTypeToBitValueMap[static_cast<int>(weaponType)]));
	return result;
}

// - CALLBACKS -------------------------------------------------------
// --------------------------------------------------------------------

void WeaponObject::Callbacks::MinRangeChanged::modified(WeaponObject &target, const float &old, const float &value, bool local) const 
{
	UNREF(old);
	UNREF(value);
	UNREF(local);
	Transceivers::minRangeChanged.emitMessage(target);
}

// --------------------------------------------------------------------

void WeaponObject::Callbacks::MaxRangeChanged::modified(WeaponObject &target, const float &old, const float &value, bool local) const 
{
	UNREF(old);
	UNREF(value);
	UNREF(local);
	Transceivers::maxRangeChanged.emitMessage(target);
}

//===================================================================
