//======================================================================
//
// ShipComponentDataReactor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataReactor.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataReactorNamespace
{
}

using namespace ShipComponentDataReactorNamespace;

//----------------------------------------------------------------------

ShipComponentDataReactor::ShipComponentDataReactor (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData      (shipComponentDescriptor),
m_energyGenerationRate (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataReactor::~ShipComponentDataReactor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataReactor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_energyGenerationRate = ship.getReactorEnergyGenerationRate ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataReactor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sEnergyGenerationRate: %f (%f)\n",
		nPad.c_str (), m_energyGenerationRate, m_energyGenerationRate * m_efficiencyGeneral);

	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataReactor::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_energyGenerationRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(SharedObjectAttributes::ship_component_reactor_generation_rate, attrib));
}

//======================================================================
