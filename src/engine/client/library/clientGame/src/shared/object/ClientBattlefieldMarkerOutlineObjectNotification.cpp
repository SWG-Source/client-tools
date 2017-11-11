//===================================================================
//
// ClientBattlefieldMarkerOutlineObjectNotification.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientBattlefieldMarkerOutlineObjectNotification.h"

#include "clientGame/ClientBattlefieldMarkerOutlineObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedObject/ObjectList.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace ClientBattlefieldMarkerOutlineObjectNotificationNamespace
{
	bool       ms_debugReport;
	int        ms_numberOfTerrainChanged;
	int        ms_numberOfBoundaryResets;
	ObjectList ms_objectList;

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("-- ClientBattlefieldMarkerOutlineObjectNotification\n"));
		DEBUG_REPORT_PRINT (true, ("       numberOfObjects = %i\n", ms_objectList.getNumberOfObjects()));
		DEBUG_REPORT_PRINT (true, ("numberOfTerrainChanged = %i\n", ms_numberOfTerrainChanged));
		DEBUG_REPORT_PRINT (true, ("numberOfBoundaryResets = %i\n", ms_numberOfBoundaryResets));
	}
}

using namespace ClientBattlefieldMarkerOutlineObjectNotificationNamespace;

//===================================================================
// STATIC PUBLIC ClientBattlefieldMarkerOutlineObjectNotification
//===================================================================

ClientBattlefieldMarkerOutlineObjectNotification ClientBattlefieldMarkerOutlineObjectNotification::ms_instance;

//-------------------------------------------------------------------

void ClientBattlefieldMarkerOutlineObjectNotification::install ()
{
	InstallTimer const installTimer("ClientBattlefieldMarkerOutlineObjectNotification::install");

	DebugFlags::registerFlag (ms_debugReport, "ClientGame", "reportClientBattlefieldMarkerOutlineObjectNotification", debugReport);
}

//-------------------------------------------------------------------

ClientBattlefieldMarkerOutlineObjectNotification& ClientBattlefieldMarkerOutlineObjectNotification::getInstance ()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void ClientBattlefieldMarkerOutlineObjectNotification::terrainChanged (Rectangle2d const & extent)
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

			ClientBattlefieldMarkerOutlineObject * const clientBattlefieldMarkerOutlineObject = safe_cast <ClientBattlefieldMarkerOutlineObject *> (object);
			clientBattlefieldMarkerOutlineObject->resetBoundary ();
		}
	}
}

//===================================================================
// PUBLIC ClientBattlefieldMarkerOutlineObjectNotification
//===================================================================

ClientBattlefieldMarkerOutlineObjectNotification::ClientBattlefieldMarkerOutlineObjectNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------

ClientBattlefieldMarkerOutlineObjectNotification::~ClientBattlefieldMarkerOutlineObjectNotification ()
{
}

//-------------------------------------------------------------------

void ClientBattlefieldMarkerOutlineObjectNotification::addToWorld (Object& object) const
{
	ms_objectList.addObject (&object);

	resetBoundary (object);
}

//-------------------------------------------------------------------

void ClientBattlefieldMarkerOutlineObjectNotification::removeFromWorld (Object& object) const
{
	ms_objectList.removeObject (&object);
}

//-------------------------------------------------------------------

bool ClientBattlefieldMarkerOutlineObjectNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//-------------------------------------------------------------------

bool ClientBattlefieldMarkerOutlineObjectNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//===================================================================
// PRIVATE ClientBattlefieldMarkerOutlineObjectNotification
//===================================================================

void ClientBattlefieldMarkerOutlineObjectNotification::resetBoundary (Object & object) const
{
	ClientBattlefieldMarkerOutlineObject * const clientBattlefieldMarkerOutlineObject = safe_cast <ClientBattlefieldMarkerOutlineObject *> (&object);
	clientBattlefieldMarkerOutlineObject->resetBoundary ();
}

//===================================================================

