// ======================================================================
//
// ContainerInterface.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// This will be moved to a game dependant place so that we can add game restrictions on these function.
//
// ======================================================================

#ifndef	INCLUDED_ContainerInterface_H
#define	INCLUDED_ContainerInterface_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedObject/SlotId.h"

class Object;
class CellProperty;
class ClientObject;
class Container;
class NetworkId;
class PortalProperty;
class SlottedContainer;
class VolumeContainer;
class ContainedByProperty;
class SlottedContainmentProperty;
class VolumeContainmentProperty;

// ======================================================================

/**
* The ContainerInterface is intended to provide an interface to the game (network, scripts, UI, etc) for manipulating
* containers.  The game shouldn't interface with the containers directly, since this will take care of scripts (on the server side)
* and will clean up old containers, and keep everything synch'd.
*/

class ContainerInterface
{
public:
	static bool      canTransferTo(ClientObject & destination, ClientObject& item);  //May need id of the player transferring the item.
	static bool      transferItemToSlottedContainer (ClientObject & destination, ClientObject& item, int arrangementIndex);
	static bool      transferItemToUnknownContainer (ClientObject & destination, ClientObject& item, int arrangementIndex, const Transform & transform);
	static bool      transferItemToVolumeContainer (ClientObject & destination, ClientObject& item);	
	static bool      transferItemToCell (ClientObject & destination, ClientObject& item, const Transform& pos);	
	static bool      transferItemToWorld (ClientObject& item, const Transform& position);
	static bool      isClientCachedOnly (const Object& object);
	
	static bool      canPlayerManipulateSlot (const SlotId& slot);

	//Property manipulator helper functions
	static Container*           getContainer(Object& obj);
	static SlottedContainer*    getSlottedContainer(Object& obj);
	static VolumeContainer*     getVolumeContainer(Object& obj);
	static CellProperty*        getCell(Object& obj);
	static PortalProperty*      getPortalProperty(Object& obj);

	static const Container*           getContainer(const Object& obj);
	static const SlottedContainer*    getSlottedContainer(const Object& obj);
	static const VolumeContainer*     getVolumeContainer(const Object& obj);
	static const CellProperty*        getCell(const Object& obj);
	static const PortalProperty*      getPortalProperty(const Object& obj);

	static ContainedByProperty*              getContainedByProperty(Object& obj);
	static SlottedContainmentProperty*       getSlottedContainmentProperty(Object& obj);
	static VolumeContainmentProperty*        getVolumeContainmentProperty(Object& obj);

	static const ContainedByProperty*        getContainedByProperty(const Object& obj);
	static const SlottedContainmentProperty* getSlottedContainmentProperty(const Object& obj);
	static const VolumeContainmentProperty*  getVolumeContainmentProperty(const Object& obj);

	static Object *                          getContainedByObject(Object& obj);
	static const Object *                    getContainedByObject(const Object& obj);

	static Object *                          getParentCellObject(Object& obj);
	static const Object *                    getParentCellObject(const Object& obj);

	static ClientObject *                    getObjectInSlot (Object & container, const char * const slotName);

	static Object*                           getTopmostContainer (Object& obj, bool ignoreCells);
	static const Object*                     getTopmostContainer (const Object& obj, bool ignoreCells);

	static Object*                           getFirstParentInWorld(Object& obj);
	static const Object*                     getFirstParentInWorld(const Object& obj);

	static bool                              isNestedWithin(const Object& obj, const NetworkId& containerId);

	//At the moment I can think of no good reason to have this interface.
	//But it is conceivably desirable.
	//	static bool      transferItem (ClientObject & destination, ClientObject& item);


private:
	ContainerInterface();
	ContainerInterface(const ContainerInterface&);
	ContainerInterface& operator= (const ContainerInterface&);
	~ContainerInterface();

	static bool      sharedTransferBegin(ClientObject& item, Container** source);
	static bool      handleTransferSource(Container* source, ClientObject& item);
	static ClientObject*   getSource(ClientObject& item);
};

// ======================================================================

#endif
