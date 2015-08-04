//======================================================================
//
// ClientShipComponentData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ClientShipComponentData.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ClientShipComponentDataNamespace
{
}

using namespace ClientShipComponentDataNamespace;

//----------------------------------------------------------------------

ClientShipComponentData::ClientShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor) :
ShipComponentData      (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ClientShipComponentData::~ClientShipComponentData ()
{
}

//----------------------------------------------------------------------

bool ClientShipComponentData::readDataFromShip      (int const chassisSlot, ShipObject const & ship)
{
	m_armorHitpointsMaximum        = ship.getComponentArmorHitpointsMaximum        (chassisSlot);
	m_armorHitpointsCurrent        = ship.getComponentArmorHitpointsCurrent        (chassisSlot);
	m_efficiencyGeneral            = ship.getComponentEfficiencyGeneral            (chassisSlot);
	m_efficiencyEnergy             = ship.getComponentEfficiencyEnergy             (chassisSlot);
	m_energyMaintenanceRequirement = ship.getComponentEnergyMaintenanceRequirement (chassisSlot);
	m_mass                         = ship.getComponentMass                         (chassisSlot);
	m_hitpointsCurrent             = ship.getComponentHitpointsCurrent             (chassisSlot);
	m_hitpointsMaximum             = ship.getComponentHitpointsMaximum             (chassisSlot);
	m_flags                        = ship.getComponentFlags                        (chassisSlot);

	m_name = ship.getComponentName(chassisSlot);

	return true;
}

//----------------------------------------------------------------------

void ClientShipComponentData::writeDataToShip       (int const /*chassisSlot*/, ShipObject & /*ship*/) const
{
}

//----------------------------------------------------------------------

bool ClientShipComponentData::readDataFromComponent (TangibleObject const & /*component*/)
{
	return true;
}

//----------------------------------------------------------------------

void ClientShipComponentData::writeDataToComponent  (TangibleObject & /*component*/) const
{
}


//----------------------------------------------------------------------

void ClientShipComponentData::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	result += padding + Unicode::narrowToWide ("Armor has no special properties.\n");
}

//----------------------------------------------------------------------

void ClientShipComponentData::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ShipComponentData::getAttributes(data);
}

//======================================================================
