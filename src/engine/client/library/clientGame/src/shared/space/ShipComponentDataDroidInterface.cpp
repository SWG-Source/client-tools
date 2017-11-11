//======================================================================
//
// ShipComponentDataDroidInterface.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataDroidInterface.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataDroidInterfaceNamespace
{
}

using namespace ShipComponentDataDroidInterfaceNamespace;

//----------------------------------------------------------------------

ShipComponentDataDroidInterface::ShipComponentDataDroidInterface (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData            (shipComponentDescriptor),
m_droidInterfaceCommandSpeed (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataDroidInterface::~ShipComponentDataDroidInterface ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataDroidInterface::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_droidInterfaceCommandSpeed = ship.getDroidInterfaceCommandSpeed ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sDroidInterfaceCommandSpeed: %f\n",
		nPad.c_str (), m_droidInterfaceCommandSpeed);

	result += Unicode::narrowToWide (buf);

}

//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_droidInterfaceCommandSpeed);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_droidinterface_speed, attrib));
}

//======================================================================
