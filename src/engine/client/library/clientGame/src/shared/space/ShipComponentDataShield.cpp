//======================================================================
//
// ShipComponentDataShield.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataShield.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataShieldNamespace
{
}

using namespace ShipComponentDataShieldNamespace;

//----------------------------------------------------------------------

ShipComponentDataShield::ShipComponentDataShield (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData        (shipComponentDescriptor),
m_shieldHitpointsFrontCurrent  (0.0f),
m_shieldHitpointsFrontMaximum  (0.0f),
m_shieldHitpointsBackCurrent   (0.0f),
m_shieldHitpointsBackMaximum   (0.0f),
m_shieldRechargeRate           (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataShield::~ShipComponentDataShield ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataShield::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_shieldHitpointsFrontCurrent  = ship.getShieldHitpointsFrontCurrent ();
	m_shieldHitpointsFrontMaximum  = ship.getShieldHitpointsFrontMaximum ();
	m_shieldHitpointsBackCurrent   = ship.getShieldHitpointsBackCurrent  ();
	m_shieldHitpointsBackMaximum   = ship.getShieldHitpointsBackMaximum  ();
	m_shieldRechargeRate           = ship.getShieldRechargeRate          ();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataShield::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sShieldHitpointsFront:  %f / %f\n"
		"%sShieldHitpointsBack:   %f / %f\n"
		"%sShieldRechargeRate:    %f (%f)\n",
		nPad.c_str (), m_shieldHitpointsFrontCurrent, m_shieldHitpointsFrontMaximum,
		nPad.c_str (), m_shieldHitpointsBackCurrent, m_shieldHitpointsBackMaximum,
		nPad.c_str (), m_shieldRechargeRate, m_shieldRechargeRate * m_efficiencyGeneral);
	
	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataShield::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsFrontCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsFrontMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_hitpoints_front, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsBackCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsBackMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_hitpoints_back, attrib));

	snprintf(buffer, buffer_size, "%.2f", m_shieldRechargeRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_recharge_rate, attrib));
}

//======================================================================
