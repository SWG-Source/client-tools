//======================================================================
//
// AttachmentData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AttachmentData.h"

#include "clientGame/ClientWaypointObject.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/Waypoint.h"
#include "sharedGame/WaypointData.h"

//======================================================================

AttachmentData::AttachmentData () :
m_type       (T_none),
m_waypoint   (0),
m_valid      (false)
{
}

//----------------------------------------------------------------------

AttachmentData::AttachmentData (const Unicode::String & oob) :
m_type       (T_none),
m_waypoint   (0),
m_valid      (false)
{
	set (oob);
}

//----------------------------------------------------------------------

AttachmentData::AttachmentData (const ClientObject & obj) :
m_type       (T_none),
m_waypoint   (0),
m_valid      (false)
{
	set (obj);
}

//----------------------------------------------------------------------

AttachmentData::AttachmentData (const AttachmentData & rhs) :
m_type       (rhs.m_type),
m_waypoint   (0),
m_valid      (rhs.m_valid)
{
	if (rhs.m_waypoint)
		m_waypoint = new WaypointDataBase(*rhs.m_waypoint);
}

//----------------------------------------------------------------------

AttachmentData & AttachmentData::operator=      (const AttachmentData & rhs)
{
	m_type       = rhs.m_type;

	if (m_waypoint)
		delete m_waypoint;

	m_waypoint   = 0;
	m_valid      = rhs.m_valid;
	
	if (rhs.m_waypoint)
		m_waypoint = new WaypointDataBase(*rhs.m_waypoint);

	return *this;
}

//----------------------------------------------------------------------

AttachmentData::~AttachmentData ()
{
	clear ();
}

//----------------------------------------------------------------------

bool AttachmentData::set (const OutOfBandBase & base)
{
	const int type = base.getTypeId ();
	switch (type)
	{
	case OutOfBandPackager::OT_waypointData:
		{
			const OutOfBand<WaypointDataBase> * const oobw = dynamic_cast<const OutOfBand<WaypointDataBase> *> (&base);
			m_waypoint = oobw->getObject ();
			m_valid = true;
			m_type  = T_waypoint;
		}
		break;
	default:
		WARNING (true, ("AttachmentData::set Unhandled oob type %d, possible memleak", type));
		break;
	}

	return m_valid;
}

//----------------------------------------------------------------------

bool AttachmentData::set (const Unicode::String & oob)
{
	clear ();	

	const OutOfBandBase * const base = OutOfBandPackager::unpackOne (oob);

	if (base)
	{
		m_valid = set (*base);
		delete base;
	}

	return m_valid;
}

//----------------------------------------------------------------------

void AttachmentData::clear           ()
{
	m_valid = false;

	if (m_waypoint)
	{
		delete m_waypoint;
		m_waypoint = 0;
	}

	m_type = T_none;
}

//----------------------------------------------------------------------

bool  AttachmentData::set (const ClientObject & obj)
{
	const ClientWaypointObject * const waypoint = dynamic_cast<const ClientWaypointObject *>(&obj);

	if (waypoint)
	{
		WaypointDataBase wd;
		wd.m_location = Location(waypoint->getLocation(), NetworkId::cms_invalid, Location::getCrcBySceneName(waypoint->getPlanetName()));
		wd.setName(waypoint->getObjectName());

		uint8 const color = Waypoint::getColorIdByName(waypoint->getColor());
		if (color != static_cast<uint8>(Waypoint::Invisible))
			wd.m_color = color;

		Unicode::String oob;
		OutOfBandPackager::pack(wd, 0, oob);
		return set (oob);
	}
	else
		clear ();

	return m_valid;
}

//----------------------------------------------------------------------

void AttachmentData::packToOob (Unicode::String & oob, int position) const
{
	switch (m_type)
	{
	case AttachmentData::T_waypoint:
		NOT_NULL (m_waypoint);			
		OutOfBandPackager::pack (*m_waypoint, position, oob);
		break;
	}
}

//======================================================================
