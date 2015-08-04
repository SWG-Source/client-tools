//======================================================================
//
// ShipComponentDataWeapon.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataWeapon.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataWeaponNamespace
{
}

using namespace ShipComponentDataWeaponNamespace;

//----------------------------------------------------------------------

ShipComponentDataWeapon::ShipComponentDataWeapon (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData      (shipComponentDescriptor),
m_weaponDamageMaximum        (10.0f),
m_weaponDamageMinimum        (1.0f),
m_weaponEffectivenessShields (0.5f),
m_weaponEffectivenessArmor   (0.5f),
m_weaponEnergyPerShot        (10.0f),
m_weaponRefireRate           (0.5f),
m_weaponAmmoCurrent          (0),
m_weaponAmmoMaximum          (0),
m_weaponAmmoType             (0)
{
}

//----------------------------------------------------------------------

ShipComponentDataWeapon::~ShipComponentDataWeapon ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataWeapon::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_weaponDamageMaximum         = ship.getWeaponDamageMaximum        (chassisSlot);
	m_weaponDamageMinimum         = ship.getWeaponDamageMinimum        (chassisSlot);
	m_weaponEffectivenessShields  = ship.getWeaponEffectivenessShields (chassisSlot);
	m_weaponEffectivenessArmor    = ship.getWeaponEffectivenessArmor   (chassisSlot);
	m_weaponEnergyPerShot         = ship.getWeaponEnergyPerShot        (chassisSlot);
	m_weaponRefireRate            = ship.getWeaponRefireRate           (chassisSlot);
	m_weaponAmmoCurrent           = ship.getWeaponAmmoCurrent          (chassisSlot);
	m_weaponAmmoMaximum           = ship.getWeaponAmmoMaximum          (chassisSlot);
	m_weaponAmmoType              = ship.getWeaponAmmoType             (chassisSlot);
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataWeapon::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sDamage:                %f-%f\n"
		"%sEffectivenessShields:  %f\n"
		"%sEffectivenessArmor:    %f\n"
		"%sEnergyPerShot:         %f\n"
		"%sRefireRate:            %f\n"
		"%sAmmo:                  (%lu) %d/%d\n",
		nPad.c_str (), m_weaponDamageMaximum, m_weaponDamageMinimum,
		nPad.c_str (), m_weaponEffectivenessShields,
		nPad.c_str (), m_weaponEffectivenessArmor,
		nPad.c_str (), m_weaponEnergyPerShot,
		nPad.c_str (), m_weaponRefireRate,
		nPad.c_str (), m_weaponAmmoType, m_weaponAmmoCurrent, m_weaponAmmoMaximum);

	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataWeapon::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f-%.1f", m_weaponDamageMinimum, m_weaponDamageMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_damage, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponEffectivenessShields);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_effectiveness_shields, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponEffectivenessArmor);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_effectiveness_armor, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_weaponEnergyPerShot);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_energy_per_shot, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponRefireRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_refire_rate, attrib));

	if(m_weaponAmmoMaximum != 0)
	{
		_itoa(m_weaponAmmoCurrent, buffer, 10);
		attrib = Unicode::narrowToWide(buffer);
		attrib += cm_slash;
		_itoa(m_weaponAmmoMaximum, buffer, 10);
		attrib += Unicode::narrowToWide(buffer);
		data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_ammo, attrib));
	}
}

//======================================================================
