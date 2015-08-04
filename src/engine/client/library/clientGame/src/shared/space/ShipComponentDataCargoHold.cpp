//======================================================================
//
// ShipComponentDataCargoHold.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataCargoHold.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataCargoHoldNamespace
{
}

using namespace ShipComponentDataCargoHoldNamespace;

//----------------------------------------------------------------------

ShipComponentDataCargoHold::ShipComponentDataCargoHold (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData      (shipComponentDescriptor),
m_cargoHoldContentsMaximum(0),
m_cargoHoldContentsCurrent(0)
{
}

//----------------------------------------------------------------------

ShipComponentDataCargoHold::~ShipComponentDataCargoHold ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataCargoHold::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_cargoHoldContentsMaximum = ship.getCargoHoldContentsMaximum();
	m_cargoHoldContentsCurrent = ship.getCargoHoldContentsCurrent();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCargoHold::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%scontents: %d/%d\n",
		nPad.c_str (), m_cargoHoldContentsCurrent, m_cargoHoldContentsMaximum);

	result += Unicode::narrowToWide (buf);
}


//----------------------------------------------------------------------

void ShipComponentDataCargoHold::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%d", m_cargoHoldContentsMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_cargo_hold_contents_max, attrib));

}

//======================================================================
