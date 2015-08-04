//======================================================================
//
// ShipComponentDataArmor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataArmor.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataArmorNamespace
{
}

using namespace ShipComponentDataArmorNamespace;

//----------------------------------------------------------------------

ShipComponentDataArmor::ShipComponentDataArmor (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData      (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataArmor::~ShipComponentDataArmor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataArmor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataArmor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	result += padding + Unicode::narrowToWide ("Armor has no special properties.\n");
}


//----------------------------------------------------------------------

void ShipComponentDataArmor::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);
}

//======================================================================
