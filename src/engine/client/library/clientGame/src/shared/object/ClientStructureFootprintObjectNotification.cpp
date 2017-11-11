//===================================================================
//
// ClientStructureFootprintObjectNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientStructureFootprintObjectNotification.h"

#include "clientGame/ClientStructureFootprintObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedObject/ObjectList.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace ClientStructureFootprintObjectNotificationNamespace
{
	bool       ms_debugReport;
	int        ms_numberOfTerrainChanged;
	int        ms_numberOfBoundaryResets;
	ObjectList ms_objectList;

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("-- ClientStructureFootprintObjectNotification\n"));
		DEBUG_REPORT_PRINT (true, ("       numberOfObjects = %i\n", ms_objectList.getNumberOfObjects()));
		DEBUG_REPORT_PRINT (true, ("numberOfTerrainChanged = %i\n", ms_numberOfTerrainChanged));
		DEBUG_REPORT_PRINT (true, ("numberOfBoundaryResets = %i\n", ms_numberOfBoundaryResets));
	}
}

using namespace ClientStructureFootprintObjectNotificationNamespace;

//===================================================================
// STATIC PUBLIC ClientStructureFootprintObjectNotification
//===================================================================

ClientStructureFootprintObjectNotification ClientStructureFootprintObjectNotification::ms_instance;

//-------------------------------------------------------------------

void ClientStructureFootprintObjectNotification::install ()
{
	InstallTimer const installTimer("ClientStructureFootprintObjectNotification::install");

	DebugFlags::registerFlag (ms_debugReport, "ClientGame", "reportClientStructureFootprintObjectNotification", debugReport);
}

//-------------------------------------------------------------------

ClientStructureFootprintObjectNotification& ClientStructureFootprintObjectNotification::getInstance ()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void ClientStructureFootprintObjectNotification::terrainChanged (Rectangle2d const & extent)
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

			ClientStructureFootprintObject * const clientStructureFootprintObject = safe_cast <ClientStructureFootprintObject *> (object);
			clientStructureFootprintObject->resetBoundary ();
		}
	}
}

//===================================================================
// PUBLIC ClientStructureFootprintObjectNotification
//===================================================================

ClientStructureFootprintObjectNotification::ClientStructureFootprintObjectNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------

ClientStructureFootprintObjectNotification::~ClientStructureFootprintObjectNotification ()
{
}

//-------------------------------------------------------------------

void ClientStructureFootprintObjectNotification::addToWorld (Object& object) const
{
	ms_objectList.addObject (&object);

	resetBoundary (object);
}

//-------------------------------------------------------------------

void ClientStructureFootprintObjectNotification::removeFromWorld (Object& object) const
{
	ms_objectList.removeObject (&object);
}

//-------------------------------------------------------------------

bool ClientStructureFootprintObjectNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//-------------------------------------------------------------------

bool ClientStructureFootprintObjectNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	resetBoundary (object);

	return true;
}

//===================================================================
// PRIVATE ClientStructureFootprintObjectNotification
//===================================================================

void ClientStructureFootprintObjectNotification::resetBoundary (Object & object) const
{
	ClientStructureFootprintObject* const clientStructureFootprintObject = safe_cast <ClientStructureFootprintObject*> (&object);
	clientStructureFootprintObject->resetBoundary ();
}

//===================================================================

