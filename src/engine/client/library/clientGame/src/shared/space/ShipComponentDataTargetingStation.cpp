//======================================================================
//
// ShipComponentDataTargetingStation.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataTargetingStation.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataTargetingStationNamespace
{
}

using namespace ShipComponentDataTargetingStationNamespace;

//----------------------------------------------------------------------

ShipComponentDataTargetingStation::ShipComponentDataTargetingStation (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData            (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataTargetingStation::~ShipComponentDataTargetingStation ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataTargetingStation::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);
	UNREF(buf_size);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	UNREF(nPad);

	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);
}

//======================================================================
