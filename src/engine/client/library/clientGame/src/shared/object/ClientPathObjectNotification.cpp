//===================================================================
//
// ClientPathObjectNotification.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientPathObjectNotification.h"

#include "clientGame/ClientPathObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedObject/ObjectList.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace ClientPathObjectNotificationNamespace
{
	bool       ms_debugReport;
	int        ms_numberOfTerrainChanged;
	int        ms_numberOfBoundaryResets;
	ObjectList ms_objectList;

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("-- ClientPathObjectNotification\n"));
		DEBUG_REPORT_PRINT (true, ("       numberOfObjects = %i\n", ms_objectList.getNumberOfObjects()));
		DEBUG_REPORT_PRINT (true, ("numberOfTerrainChanged = %i\n", ms_numberOfTerrainChanged));
		DEBUG_REPORT_PRINT (true, ("numberOfBoundaryResets = %i\n", ms_numberOfBoundaryResets));
	}
}

using namespace ClientPathObjectNotificationNamespace;

//===================================================================
// STATIC PUBLIC ClientPathObjectNotification
//===================================================================

ClientPathObjectNotification ClientPathObjectNotification::ms_instance;

//-------------------------------------------------------------------

void ClientPathObjectNotification::install ()
{
	InstallTimer const installTimer("ClientPathObjectNotification::install");

	DebugFlags::registerFlag (ms_debugReport, "ClientGame", "reportClientPathObjectNotification", debugReport);
}

//-------------------------------------------------------------------

ClientPathObjectNotification& ClientPathObjectNotification::getInstance ()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void ClientPathObjectNotification::terrainChanged (Rectangle2d const & extent)
{
	++ms_numberOfTerrainChanged;

	for (int i = 0; i < ms_objectList.getNumberOfObjects (); ++i)
	{
		Object * const object = ms_objectList.getObject (i);

		Vector const & position_w = object->getPosition_w ();
		Rectangle2d const rectangle2d (position_w.x - 128.f, position_w.z - 128.f, position_w.x + 128.f, position_w.z + 128.f);

		if (extent.intersects (rectangle2d))
		{
			++ms_numberOfBoundaryResets;

			ClientPathObject * const clientPathObject = safe_cast <ClientPathObject *> (object);
			clientPathObject->resetBoundary ();
		}
	}
}

//===================================================================
// PUBLIC ClientPathObjectNotification
//===================================================================

ClientPathObjectNotification::ClientPathObjectNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------

ClientPathObjectNotification::~ClientPathObjectNotification ()
{
}

//-------------------------------------------------------------------

void ClientPathObjectNotification::addToWorld (Object& object) const
{
	ms_objectList.addObject (&object);

	resetBoundary (object);
}

//-------------------------------------------------------------------

void ClientPathObjectNotification::removeFromWorld (Object& object) const
{
	ms_objectList.removeObject (&object);
}

//-------------------------------------------------------------------

bool ClientPathObjectNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//-------------------------------------------------------------------

bool ClientPathObjectNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//===================================================================
// PRIVATE ClientPathObjectNotification
//===================================================================

void ClientPathObjectNotification::resetBoundary (Object & object) const
{
	ClientPathObject * const clientPathObject = safe_cast <ClientPathObject *> (&object);
	clientPathObject->resetBoundary ();
}

//===================================================================

