//======================================================================
//
// ShipComponentDataEngine.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstclientGame.h"
#include "clientGame/ShipComponentDataEngine.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataEngineNamespace
{
}

using namespace ShipComponentDataEngineNamespace;

//----------------------------------------------------------------------

ShipComponentDataEngine::ShipComponentDataEngine (ShipComponentDescriptor const & shipComponentDescriptor) :
ClientShipComponentData         (shipComponentDescriptor),
m_engineAccelerationRate        (30.0f),
m_engineDecelerationRate        (30.0f),
m_enginePitchAccelerationRate   (PI),
m_engineYawAccelerationRate     (PI),
m_engineRollAccelerationRate    (PI),
m_enginePitchRateMaximum        (PI),
m_engineYawRateMaximum          (PI),
m_engineRollRateMaximum         (PI),
m_engineSpeedMaximum            (50.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataEngine::~ShipComponentDataEngine ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataEngine::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ClientShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_engineAccelerationRate        = ship.getEngineAccelerationRate      ();
	m_engineDecelerationRate        = ship.getEngineDecelerationRate      ();
	m_enginePitchAccelerationRate   = ship.getEnginePitchAccelerationRate ();
	m_engineYawAccelerationRate     = ship.getEngineYawAccelerationRate   ();
	m_engineRollAccelerationRate    = ship.getEngineRollAccelerationRate  ();
	m_enginePitchRateMaximum        = ship.getEnginePitchRateMaximum      ();
	m_engineYawRateMaximum          = ship.getEngineYawRateMaximum        ();
	m_engineRollRateMaximum         = ship.getEngineRollRateMaximum       ();
	m_engineSpeedMaximum            = ship.getEngineSpeedMaximum          ();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataEngine::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sEngineAccelerationRate:  %f\n"
		"%sEngineDecelerationRate:  %f\n"
		"%sEnginePitchAccelerationRate:  %f\n"
		"%sEngineYawAccelerationRate:  %f\n"
		"%sEngineRollAccelerationRate:  %f\n"
		"%sEnginePitchRateMaximum:  %f\n"
		"%sEngineYawRateMaximum:  %f\n"
		"%sEngineRollRateMaximum:  %f\n"
		"%sEngineSpeedMaximum:  %f\n",
		nPad.c_str (), m_engineAccelerationRate,
		nPad.c_str (), m_engineDecelerationRate,
		nPad.c_str (), convertRadiansToDegrees(m_enginePitchAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_engineYawAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_engineRollAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_enginePitchRateMaximum),
		nPad.c_str (), convertRadiansToDegrees(m_engineYawRateMaximum),
		nPad.c_str (), convertRadiansToDegrees(m_engineRollRateMaximum),
		nPad.c_str (), m_engineSpeedMaximum);
	
	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataEngine::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const
{
	ClientShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_enginePitchRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_pitch_rate_maximum, attrib));
	
	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_engineYawRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_yaw_rate_maximum, attrib));

	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_engineRollRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_roll_rate_maximum, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_engineSpeedMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_speed_maximum, attrib));
}

//======================================================================
