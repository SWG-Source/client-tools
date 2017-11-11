//======================================================================
//
// ShipComponentDataCapacitor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataCapacitor.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataCapacitorNamespace
{
}

using namespace ShipComponentDataCapacitorNamespace;

//----------------------------------------------------------------------

ShipComponentDataCapacitor::ShipComponentDataCapacitor (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData        (shipComponentDescriptor),
m_capacitorEnergyCurrent       (0.0f),
m_capacitorEnergyMaximum       (0.0f),
m_capacitorEnergyRechargeRate  (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataCapacitor::~ShipComponentDataCapacitor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataCapacitor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_capacitorEnergyCurrent        = ship.getCapacitorEnergyCurrent ();
	m_capacitorEnergyMaximum        = ship.getCapacitorEnergyMaximum ();
	m_capacitorEnergyRechargeRate   = ship.getCapacitorEnergyRechargeRate ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCapacitor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sCapacitorEnergyCurrent:  %f\n"
		"%sCapacitorEnergyMaximum:  %f\n"
		"%sCapacitorEnergyRechargeRate:  %f\n",
		nPad.c_str (), m_capacitorEnergyCurrent,
		nPad.c_str (), m_capacitorEnergyMaximum,
		nPad.c_str (), m_capacitorEnergyRechargeRate);
	
	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataCapacitor::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_capacitor_energy, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyRechargeRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_capacitor_energy_recharge_rate, attrib));
}

//======================================================================
