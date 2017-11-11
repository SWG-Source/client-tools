//===================================================================
//
// ClientObjectTerrainModificationNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientObjectTerrainModificationNotification.h"

#include "clientGame/ClientObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/Vector2d.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainModificationHelper.h"

//===================================================================

namespace
{
	bool ms_debugReport;
	int  ms_numberOfInstances;

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("numberOfInstances = %i\n", ms_numberOfInstances));
	}
}

//===================================================================

ProceduralTerrainAppearance* ClientObjectTerrainModificationNotification::ms_terrainAppearance;
ClientObjectTerrainModificationNotification ClientObjectTerrainModificationNotification::ms_instance;

//===================================================================

void ClientObjectTerrainModificationNotification::install ()
{
	InstallTimer const installTimer("ClientObjectTerrainModificationNotification::install");

	DebugFlags::registerFlag (ms_debugReport, "ClientGame", "reportBuildingObjectTerrainModificationNotification", debugReport);
}

//-------------------------------------------------------------------

ClientObjectTerrainModificationNotification &ClientObjectTerrainModificationNotification::getInstance()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void ClientObjectTerrainModificationNotification::setTerrainAppearance (ProceduralTerrainAppearance* const terrainAppearance)
{
	ms_terrainAppearance = terrainAppearance;
}

//===================================================================

ClientObjectTerrainModificationNotification::ClientObjectTerrainModificationNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------

ClientObjectTerrainModificationNotification::~ClientObjectTerrainModificationNotification ()
{
}

//-------------------------------------------------------------------

void ClientObjectTerrainModificationNotification::addToWorld (Object& object) const
{
	++ms_numberOfInstances;

	//-- insert the layer into the terrain system
	ClientObject* const clientObject = object.asClientObject();
	TerrainGenerator::Layer* const layer = clientObject->getLayer ();
	ms_terrainAppearance->addLayer (layer);

	updateTerrain (object, false, true);
}

//-------------------------------------------------------------------

void ClientObjectTerrainModificationNotification::removeFromWorld (Object& object) const
{
	--ms_numberOfInstances;

	//-- remove the layer from the terrain system
	ClientObject* const clientObject = object.asClientObject();
	TerrainGenerator::Layer* const layer = clientObject->getLayer ();
	ms_terrainAppearance->removeLayer (layer);

	updateTerrain (object, true, false);
}

//-------------------------------------------------------------------

bool ClientObjectTerrainModificationNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& oldPosition) const
{
	UNREF(oldPosition);
	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, true, true);

	return true;
}

//-------------------------------------------------------------------

void ClientObjectTerrainModificationNotification::rotationChanged (Object& object, bool /*dueToParentChange*/) const
{
	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, false, false);
}

//-------------------------------------------------------------------

bool ClientObjectTerrainModificationNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& oldPosition) const
{
	UNREF(oldPosition);

	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, true, true);

	return true;
}

//-------------------------------------------------------------------

void ClientObjectTerrainModificationNotification::updateTerrain (Object& object, bool const updateOldExtent, bool const updateNewExtent) const
{
	DEBUG_REPORT_LOG (true, ("ClientObjectTerrainModificationNotification::updateTerrain: object %s [%s] caused a terrain rebuild\n", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplate ()->getName ()));

	ClientObject* const clientObject = object.asClientObject();
	TerrainGenerator::Layer* const layer = clientObject->getLayer ();
	ProceduralTerrainAppearance* const proceduralTerrainAppearance = safe_cast<ProceduralTerrainAppearance*> (ms_terrainAppearance);
	NOT_NULL (proceduralTerrainAppearance);

	//-- get the new position of the object
	const Vector position = object.getPosition_w ();

	//-- save the current extent of the layer
	layer->calculateExtent ();
	Rectangle2d oldExtent;
	bool hasOldExtent = updateOldExtent && layer->getUseExtent ();
	if (hasOldExtent)
		oldExtent = layer->getExtent ();

	//-- calculate the updated layer information (position and heights)
	const Vector2d& position2d = Vector2d (position.x, position.z);
	TerrainModificationHelper::setPosition (layer, position2d);
	TerrainModificationHelper::setRotation (layer, object.getObjectFrameK_w ().theta ());
	TerrainModificationHelper::setHeight (layer, ms_terrainAppearance->generateHeight_expensive (position2d));

	//-- prepare the terrain
	ms_terrainAppearance->prepareGenerator ();

	//-- get the new extent of the layer
	layer->calculateExtent ();
	Rectangle2d newExtent;
	bool hasNewExtent = updateNewExtent && layer->getUseExtent ();
	if (hasNewExtent)
		newExtent = layer->getExtent ();

	//-- tell the terrain system the old and new extent regions have changed
	if (hasOldExtent)
		ms_terrainAppearance->invalidateRegion (oldExtent);

	if (hasNewExtent)
		ms_terrainAppearance->invalidateRegion (newExtent);
}

//===================================================================

