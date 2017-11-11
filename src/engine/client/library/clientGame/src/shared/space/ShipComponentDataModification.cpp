//======================================================================
//
// ShipComponentDataModification.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataModification.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataModificationNamespace
{
}

using namespace ShipComponentDataModificationNamespace;

//----------------------------------------------------------------------

ShipComponentDataModification::ShipComponentDataModification (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData      (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataModification::~ShipComponentDataModification ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataModification::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataModification::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	result += padding + Unicode::narrowToWide ("Modifications have no special properties.\n");
}


//----------------------------------------------------------------------

void ShipComponentDataModification::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);
}

//======================================================================
