//===================================================================
//
// ClearNonCollidableFloraNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClearNonCollidableFloraNotification.h"

#include "clientGame/ClientDataFile.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================
// STATIC PUBLIC ClearNonCollidableFloraNotification
//===================================================================

ClearNonCollidableFloraNotification ClearNonCollidableFloraNotification::ms_instance;

//-------------------------------------------------------------------

ClearNonCollidableFloraNotification& ClearNonCollidableFloraNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ClearNonCollidableFloraNotification
//===================================================================

ClearNonCollidableFloraNotification::ClearNonCollidableFloraNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
ClearNonCollidableFloraNotification::~ClearNonCollidableFloraNotification ()
{
}

//-------------------------------------------------------------------

void ClearNonCollidableFloraNotification::addToWorld (Object& object) const
{
	const SharedObjectTemplate* const objectTemplate = dynamic_cast<const SharedObjectTemplate*> (object.getObjectTemplate ());
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("ClearNonCollidableFloraNotification::addToWorld - [%s] does not have a shared object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const ClientDataFile* const clientData = dynamic_cast<const ClientDataFile*> (objectTemplate->getClientData ());
	if (!clientData)
	{
		DEBUG_WARNING (true, ("ClearNonCollidableFloraNotification::addToWorld - [%s] does not have an appropriate client data file", object.getObjectTemplate ()->getName ()));
		return;
	}

	ClientDataFile::ClearFloraEntryList const * const clearFloraEntryList = clientData->getClearFloraEntryList();
	if (!clearFloraEntryList)
	{
		DEBUG_WARNING(true, ("ClearNonCollidableFloraNotification::addToWorld - [%s] with client data file %s does not have a clear flora entry list", object.getObjectTemplate()->getName(), clientData->getName()));
		return;
	}

	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ClearNonCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->addClearNonCollidableFloraObject(&object, *clearFloraEntryList);
}

//-------------------------------------------------------------------

void ClearNonCollidableFloraNotification::removeFromWorld (Object& object) const
{
	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ClearNonCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->removeClearNonCollidableFloraObject (&object);
}

//-------------------------------------------------------------------

bool ClearNonCollidableFloraNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//-------------------------------------------------------------------

bool ClearNonCollidableFloraNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//===================================================================

