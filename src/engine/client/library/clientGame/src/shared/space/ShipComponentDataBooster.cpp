//======================================================================
//
// ShipComponentDataBooster.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipComponentDataBooster.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataBoosterNamespace
{
}

using namespace ShipComponentDataBoosterNamespace;

//----------------------------------------------------------------------

ShipComponentDataBooster::ShipComponentDataBooster (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData         (shipComponentDescriptor),
m_boosterEnergyCurrent          (100.0f),
m_boosterEnergyMaximum          (100.0f),
m_boosterEnergyRechargeRate     (20.0f),
m_boosterEnergyConsumptionRate  (20.0f),
m_boosterAcceleration           (5.0f),
m_boosterSpeedMaximum           (10.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataBooster::~ShipComponentDataBooster ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataBooster::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_boosterEnergyCurrent          = ship.getBoosterEnergyCurrent ();
	m_boosterEnergyMaximum          = ship.getBoosterEnergyMaximum ();
	m_boosterEnergyRechargeRate     = ship.getBoosterEnergyRechargeRate ();
	m_boosterEnergyConsumptionRate  = ship.getBoosterEnergyConsumptionRate ();
	m_boosterAcceleration           = ship.getBoosterAcceleration ();
	m_boosterSpeedMaximum           = ship.getBoosterSpeedMaximum ();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataBooster::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sboosterEnergyCurrent: %f\n"
		"%sboosterEnergyMaximum: %f\n"
		"%sboosterEnergyRechargeRate: %f\n"
		"%sboosterEnergyConsumptionRate: %f\n"
		"%sboosterAcceleration: %f\n"
		"%sboosterSpeedMaximum: %f\n",
		nPad.c_str (), m_boosterEnergyCurrent,
		nPad.c_str (), m_boosterEnergyMaximum,
		nPad.c_str (), m_boosterEnergyRechargeRate,
		nPad.c_str (), m_boosterEnergyConsumptionRate,
		nPad.c_str (), m_boosterAcceleration,
		nPad.c_str (), m_boosterSpeedMaximum);
	
	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataBooster::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_boosterEnergyCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_boosterEnergyMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_booster_energy, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_boosterEnergyRechargeRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_booster_energy_recharge_rate, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_boosterEnergyConsumptionRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_booster_energy_consumption_rate, attrib));
	
	snprintf(buffer, buffer_size, "%.1f", m_boosterAcceleration);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_booster_acceleration, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_boosterSpeedMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_booster_speed_maximum, attrib));
}

//======================================================================
