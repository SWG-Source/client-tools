//===================================================================
//
// ClientClearCollidableFloraNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientClearCollidableFloraNotification.h"

#include "sharedGame/SharedObjectTemplate.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================
// STATIC PUBLIC ClientClearCollidableFloraNotification
//===================================================================

ClientClearCollidableFloraNotification ClientClearCollidableFloraNotification::ms_instance;

//-------------------------------------------------------------------

ClientClearCollidableFloraNotification& ClientClearCollidableFloraNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ClientClearCollidableFloraNotification
//===================================================================

ClientClearCollidableFloraNotification::ClientClearCollidableFloraNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
ClientClearCollidableFloraNotification::~ClientClearCollidableFloraNotification ()
{
}

//-------------------------------------------------------------------

void ClientClearCollidableFloraNotification::addToWorld (Object& object) const
{
	const SharedObjectTemplate* const objectTemplate = dynamic_cast<const SharedObjectTemplate*> (object.getObjectTemplate ());
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("ClientClearCollidableFloraNotification::addToWorld - [%s] does not have a shared object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const float clearFloraRadius = objectTemplate->getClearFloraRadius ();
	if (clearFloraRadius <= 0.f)
	{
		DEBUG_WARNING (true, ("ClientClearCollidableFloraNotification::addToWorld - [%s] has a clearFloraRadius (%1.2f) <= 0", object.getObjectTemplate ()->getName (), clearFloraRadius));
		return;
	}

	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ClientClearCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->addClearCollidableFloraObject (&object, object.getPosition_w (), clearFloraRadius);
}

//-------------------------------------------------------------------

void ClientClearCollidableFloraNotification::removeFromWorld (Object& object) const
{
	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ClientClearCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->removeClearCollidableFloraObject (&object);
}

//-------------------------------------------------------------------

bool ClientClearCollidableFloraNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//-------------------------------------------------------------------

bool ClientClearCollidableFloraNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//===================================================================

