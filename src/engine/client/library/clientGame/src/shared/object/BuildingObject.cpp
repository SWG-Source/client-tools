// ======================================================================
//
// BuildingObject.cpp
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/BuildingObject.h"

#include "clientGame/ClientBuildingObjectTemplate.h"
#include "clientGame/ClientController.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/RemoteCreatureController.h"
#include "sharedCollision/BarrierObject.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/DoorObject.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/TerrainModificationHelper.h"

// ======================================================================
// PUBLIC BuildingObject
// ======================================================================

BuildingObject::BuildingObject(const SharedBuildingObjectTemplate* newTemplate) :
	TangibleObject(newTemplate)
{
	//-- create test layer (this should load from the template's layer filename)
	if (newTemplate->getTerrainModificationFileName ().length () != 0)
	{
		TerrainGenerator::Layer* layer = TerrainModificationHelper::importLayer (newTemplate->getTerrainModificationFileName ().c_str ());
		WARNING (!layer, ("Layer %s not found", newTemplate->getTerrainModificationFileName ().c_str ()));

		if (layer)
			setLayer(layer);
	}

}

// ----------------------------------------------------------------------
	
BuildingObject::~BuildingObject()
{
	//-- Handle destruction in same order as server's BuildingObject.
	if (isInWorld ())
		removeFromWorld ();
}

// ----------------------------------------------------------------------

void BuildingObject::addToWorld()
{
	TangibleObject::addToWorld();

	//-- create our interior data
#ifdef _DEBUG
	if (ConfigClientGame::getUseInteriorLayoutFiles())
#endif
	{
		safe_cast<ClientBuildingObjectTemplate const *>(getObjectTemplate())->applyInteriorLayout(this);
	}
}

// ----------------------------------------------------------------------

void BuildingObject::removeFromWorld ()
{
	//-- if the player's cell is not the world cell
	PortalProperty * const portalProperty = getPortalProperty ();
	if (portalProperty)
	{
		DEBUG_FATAL (!isInWorldCell (), ("We are making an assumption that the building is in the world cell.  This code will need to be revisited."));

		typedef std::vector<CreatureObject *> CreatureObjectList;
		CreatureObjectList creatureObjectList;

		{
			//-- walk the cell list and if a player is in the cell, eject the player
			int const numberOfCells = portalProperty->getNumberOfCells ();
			for (int i = 0; i < numberOfCells; ++i)
			{
				CellProperty * const cellProperty = portalProperty->getCell (i);
				if (cellProperty)
				{
					Object * const cellObject = &cellProperty->getOwner ();

					int const numberOfAttachedObjects = cellObject->getNumberOfAttachedObjects ();
					for (int j = 0; j < numberOfAttachedObjects; ++j)
					{
						Object * const attachedObject = cellObject->getAttachedObject (j);
						if (attachedObject && attachedObject->isInWorld () && !attachedObject->isChildObject () && attachedObject->asClientObject () && !attachedObject->asClientObject ()->isClientCached () && attachedObject->asClientObject ()->asCreatureObject ())
						{
							CreatureObject * const creatureObject = attachedObject->asClientObject ()->asCreatureObject ();
							creatureObjectList.push_back (creatureObject);
						}
					}
				}
			}
		}

		{
			while (!creatureObjectList.empty ())
			{
				CreatureObject * const creatureObject = creatureObjectList.back ();
				creatureObjectList.pop_back ();

				//-- get the player's world transform
				Transform const transform_w = creatureObject->getTransform_o2w ();

				//-- move the player to the world cell
				creatureObject->setParentCell (CellProperty::getWorldCellProperty ());
				CellProperty::setPortalTransitionsEnabled (false);
					creatureObject->setTransform_o2p (transform_w);
				CellProperty::setPortalTransitionsEnabled (true);

				//-- tell the collision system that the player warped
				CollisionWorld::objectWarped (creatureObject);

				//-- tell dead reckoning that the player has warped
				RemoteCreatureController * const remoteCreatureController = dynamic_cast<RemoteCreatureController *> (creatureObject->getController ());
				if (remoteCreatureController)
					remoteCreatureController->updateDeadReckoningModel (transform_w);
			}
		}
	}

	typedef std::vector<Object*> Objects;
	Objects objectsToDelete;

	int const numberOfChildObjects = getNumberOfChildObjects();
	for (int i = 0; i < numberOfChildObjects; ++i)
	{
		Object * const object = getChildObject(i);

		DoorObject * const door = dynamic_cast<DoorObject *>(object);
		
		if (door != 0)
		{
			objectsToDelete.push_back(door);
		}

		BarrierObject * const barrier = dynamic_cast<BarrierObject *>(object);

		if (barrier != 0)
		{
			objectsToDelete.push_back(barrier);
		}
	}

	Objects::const_iterator ii = objectsToDelete.begin();
	Objects::const_iterator iiEnd = objectsToDelete.end();

	for (; ii != iiEnd; ++ii)
	{
		Object * const object = *ii;
		delete object;
	}

	TangibleObject::removeFromWorld ();
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter const * BuildingObject::getInteriorLayout() const
{
	return safe_cast<ClientBuildingObjectTemplate const *>(getObjectTemplate())->getInteriorLayout();
}

// ======================================================================

