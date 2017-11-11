// ======================================================================
//
// ContainerInterface.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ContainerInterface.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientController.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/Game.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlotIdArchive.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include <cstdio>

// -----------------------------------------------------------------------

//@todo consider hiding all container functions and friend this class to containers.

bool ContainerInterface::canTransferTo(ClientObject & destination, ClientObject& item)
{
	//check all scripts
	Container * container = getContainer(destination);
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	if (container)
		return container->mayAdd(item, tmp);

	return false;
}


// -----------------------------------------------------------------------

bool ContainerInterface::sharedTransferBegin(ClientObject& item, Container** sourceContainer)
{
	NOT_NULL(sourceContainer);
	ContainedByProperty * containedBy = getContainedByProperty(item);
	if (!containedBy || !containedBy->getContainedBy())
	{
		//This item is not contained by anything!
		//This means it is in the world!
		//Return null for the source container, but succeed.
		*sourceContainer = NULL;
		return true;
	}

	ClientObject* source = dynamic_cast<ClientObject*>(containedBy->getContainedBy());
	if (!source)
	{
		WARNING_STRICT_FATAL(true, ("This item's source was not found!"));
		return false;
	}

	*sourceContainer = getContainer(*source);
	if (!*sourceContainer)
	{
		WARNING_STRICT_FATAL(true, ("This source contains stuff, but has no property!!"));
		return false;
	}

	return true;
}

bool ContainerInterface::handleTransferSource(Container* source, ClientObject& item)
{
	if (!source)
	{
		//A null source means we are in the world and not contained.
		//@todo, I assume we remove it from the world when we add it to a container??
//		World::removeClientObject(&item);
		return true;
	}
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return source->remove(item, tmp);
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToSlottedContainer (ClientObject & destination, ClientObject& item, int arrangementIndex)
{
	// NOTE: following code assumes that if Game::getPlayer() returns NULL, but you are
	//       multiplayer, then you must be in the UI where we want to run the client-controlled
	//       code.

	if (!Game::getSinglePlayer() && Game::getPlayer())
	{
		return transferItemToUnknownContainer(destination, item, arrangementIndex, Transform::identity);
	}
	else
	{
		if (arrangementIndex < 0)
		{
			WARNING_STRICT_FATAL(true, ("Invaid arrangement index in transferItemToSlottedContainer"));
			return false;
		}

		Container* sourceContainer = NULL;
		if (!sharedTransferBegin(item, &sourceContainer))
			return false;

		SlottedContainer* const destContainer = getSlottedContainer(destination);
		if (!destContainer)
		{
			WARNING_STRICT_FATAL(true, ("This destination is not a slot container!!"));
			return false;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		if (!destContainer->mayAdd(item, static_cast<size_t>(arrangementIndex), tmp))
			return false;

		//check all scripts

		//check source
		//then check item
		//then check destiation

		if (!handleTransferSource(sourceContainer, item))
			return false;

		item.setTransferInProgress(sourceContainer != NULL);
		const bool retval = destContainer->add(item, static_cast<size_t>(arrangementIndex), tmp);
		item.setTransferInProgress(false);

		if (!retval)
		{
			//@todo re-examine use of fatal here.
			FATAL(true, ("Checks to add an item succeeded, but the add failed! Panic"));
			return false;  //lint !e527 Unreachable code
		}
	}
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToVolumeContainer (ClientObject & destination, ClientObject& item)
{
	if (!Game::getSinglePlayer() && Game::getPlayer())
	{
		return transferItemToUnknownContainer(destination, item, -1, Transform::identity);
	}
	else
	{
		Container* sourceContainer = NULL;
		if (!sharedTransferBegin(item, &sourceContainer))
			return false;

		VolumeContainer* destContainer = getVolumeContainer(destination);
		if (!destContainer)
		{
			WARNING_STRICT_FATAL(true, ("This destination is not a volume container!!"));
			return false;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		if (!destContainer->mayAdd(item, tmp))
			return false;

		if (!handleTransferSource(sourceContainer, item))
			return false;

		item.setTransferInProgress(sourceContainer != NULL);
		const bool retval = destContainer->add(item, tmp);
		item.setTransferInProgress(false);

		if (!retval)
		{
			//@todo re-examine use of fatal here.
			FATAL(true, ("Checks to add an item succeeded, but the add failed! Panic"));
			return false; //lint !e527 Unreachable code
		}
	}
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToCell (ClientObject & destination, ClientObject& item, const Transform & pos)
{
	if (!Game::getSinglePlayer() && Game::getPlayer())
	{
		return transferItemToUnknownContainer(destination, item, -1, pos);
	}
	else
	{
		Container* sourceContainer = NULL;
		if (!sharedTransferBegin(item, &sourceContainer))
			return false;

		CellProperty * destContainer = getCell (destination);
		if (!destContainer)
		{
			WARNING_STRICT_FATAL(true, ("This destination is not a volume container!!"));
			return false;
		}

		if (!handleTransferSource(sourceContainer, item))
			return false;

		item.setTransferInProgress(sourceContainer != NULL);
		destContainer->addObject_w(item);
		item.setTransferInProgress(false);
	}
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToWorld (ClientObject& item, const Transform & pos)
{
	if (!Game::getSinglePlayer() && Game::getPlayer())
	{
		static const uint32 hash_open = Crc::normalizeAndCalculate("transferItemMisc");

		char buf [256];
		Vector v = pos.getPosition_p();
		_snprintf (buf, sizeof (buf), " %s %d %f %f %f", NetworkId::cms_invalid.getValueString().c_str(), -1, v.x, v.y, v.z);
		
		const Unicode::String & params = Unicode::narrowToWide (buf);
		
		ClientCommandQueue::enqueueCommand (hash_open, item.getNetworkId (), params);
		return true;
	}
	else
	{

		Container* sourceContainer = NULL;
		if (!sharedTransferBegin(item, &sourceContainer))
			return false;

		if (!handleTransferSource(sourceContainer, item))
			return false;

		item.setTransferInProgress(sourceContainer != NULL);
		item.addToWorld();
		item.setTransferInProgress(false);

		item.setTransform_o2p(pos);
	}
	return true;
}

// -----------------------------------------------------------------------

bool ContainerInterface::transferItemToUnknownContainer (ClientObject & destination, ClientObject& item, int arrangementIndex, const Transform & transform)
{
	if (!Game::getSinglePlayer() && Game::getPlayer())
	{

		static const uint32 hash_weapon = Crc::normalizeAndCalculate("transferItemWeapon");
		static const uint32 hash_armor = Crc::normalizeAndCalculate("transferItemArmor");
		static const uint32 hash_misc = Crc::normalizeAndCalculate("transferItemMisc");

		uint32 hash_command = hash_misc;

		const Object * source = getContainedByObject(item);
		
		if (&destination == Game::getPlayer() || source == Game::getPlayer())
		{
			SlottedContainmentProperty * slotted = ContainerInterface::getSlottedContainmentProperty(item);
			if (slotted && arrangementIndex >= 0)
			{
				if (GameObjectTypes::isTypeOf (item.getGameObjectType (), SharedObjectTemplate::GOT_armor))
					hash_command = hash_armor;
				else
				{
					//In this case, the player is transferring something from their equipment, which is not armor so it might be a weapon.  Check to see if its being equipped into hands or not.
					static const SlotId right_hand = SlotIdManager::findSlotId(CrcLowerString("hold_r"));
					static const SlotId left_hand = SlotIdManager::findSlotId(CrcLowerString("hold_l"));
					
					int numSlots = slotted->getNumberOfSlots(arrangementIndex);
					for (int i = 0; i < numSlots; ++i)
					{
						SlotId currentSlot = slotted->getSlotId(arrangementIndex, i);
						if (currentSlot == right_hand || currentSlot == left_hand)
						{
							hash_command = hash_weapon;
							break;
						}
					}
				}				
			}
		}


		char buf [256];
		Vector pos = transform.getPosition_p();
		_snprintf (buf, sizeof (buf), " %s %d %f %f %f", destination.getNetworkId().getValueString().c_str(), arrangementIndex, pos.x, pos.y, pos.z);
		
		const Unicode::String & params = Unicode::narrowToWide (buf);
		
		ClientCommandQueue::enqueueCommand (hash_command, item.getNetworkId (), params);
		return true;
	}
	else
	{

		VolumeContainer* destContainerV = getVolumeContainer(destination);
		if (destContainerV)
		{
			return transferItemToVolumeContainer(destination, item);
		}

		SlottedContainer* destContainerL = getSlottedContainer(destination);
		if (destContainerL)
		{
			return transferItemToSlottedContainer(destination, item, arrangementIndex);
		}
	}
	return false;

}


// -----------------------------------------------------------------------

bool ContainerInterface::canPlayerManipulateSlot (const SlotId& slot)
{
	return SlotIdManager::isSlotPlayerModifiable(slot);
}

// -----------------------------------------------------------------------

ClientObject* ContainerInterface::getSource(ClientObject &item)
{
	ContainedByProperty * prop = getContainedByProperty(item);
	if (!prop)
	{
		return NULL;
	}
	return dynamic_cast<ClientObject*>(&prop->getOwner());
}

// -----------------------------------------------------------------------

Container* ContainerInterface::getContainer(Object& obj)
{
	return obj.getContainerProperty();
}

// -----------------------------------------------------------------------

SlottedContainer* ContainerInterface::getSlottedContainer(Object& obj)
{
	return obj.getSlottedContainerProperty();
}

// -----------------------------------------------------------------------

VolumeContainer*  ContainerInterface::getVolumeContainer(Object& obj)
{
	return obj.getVolumeContainerProperty();
}

// -----------------------------------------------------------------------

CellProperty*  ContainerInterface::getCell(Object& obj)
{
	return obj.getCellProperty();
}

// -----------------------------------------------------------------------

PortalProperty*  ContainerInterface::getPortalProperty(Object& obj)
{
	return obj.getPortalProperty();
}

//-----------------------------------------------------------------

const Container* ContainerInterface::getContainer(const Object& obj)
{
	const Container * container = getSlottedContainer(obj);
	if (!container)
	{
		container = getVolumeContainer(obj);
		if (!container)
		{
			container = getCell(obj);
			if (!container)
			{
				container = getPortalProperty(obj);
				
			}
		}
	}
	return container;
}

//-----------------------------------------------------------------

const SlottedContainer* ContainerInterface::getSlottedContainer(const Object& obj)
{
	return obj.getSlottedContainerProperty();
}

//-----------------------------------------------------------------

const VolumeContainer* ContainerInterface::getVolumeContainer(const Object& obj)
{
	return obj.getVolumeContainerProperty();
}

//-----------------------------------------------------------------

const CellProperty* ContainerInterface::getCell(const Object& obj)
{
	return obj.getCellProperty();
}

//-----------------------------------------------------------------

const PortalProperty* ContainerInterface::getPortalProperty(const Object& obj)
{
	return obj.getPortalProperty();
}

// -----------------------------------------------------------------------

ContainedByProperty* ContainerInterface::getContainedByProperty(Object& obj)
{
	return obj.getContainedByProperty();
}

// -----------------------------------------------------------------------

SlottedContainmentProperty* ContainerInterface::getSlottedContainmentProperty(Object& obj)
{
	return safe_cast<SlottedContainmentProperty*>(obj.getProperty(SlottedContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

VolumeContainmentProperty* ContainerInterface::getVolumeContainmentProperty(Object& obj)
{
	return safe_cast<VolumeContainmentProperty*>(obj.getProperty(VolumeContainmentProperty::getClassPropertyId()));
}

// -----------------------------------------------------------------------

const ContainedByProperty* ContainerInterface::getContainedByProperty(const Object& obj)
{
	return obj.getContainedByProperty();
}

//-----------------------------------------------------------------

const SlottedContainmentProperty* ContainerInterface::getSlottedContainmentProperty(const Object& obj)
{
	return safe_cast<const SlottedContainmentProperty*>(obj.getProperty(SlottedContainmentProperty::getClassPropertyId()));
}

//-----------------------------------------------------------------

const VolumeContainmentProperty*  ContainerInterface::getVolumeContainmentProperty(const Object& obj)
{
	return safe_cast<const VolumeContainmentProperty*>(obj.getProperty(VolumeContainmentProperty::getClassPropertyId()));
}

//----------------------------------------------------------------------

Object * ContainerInterface::getContainedByObject(Object& obj)
{
	ContainedByProperty * const containedBy = getContainedByProperty(obj);
	return containedBy ? containedBy->getContainedBy() : 0;
}

//----------------------------------------------------------------------

const Object * ContainerInterface::getContainedByObject(const Object& obj)
{
	const ContainedByProperty * const containedBy = getContainedByProperty(obj);
	return containedBy ? containedBy->getContainedBy() : 0;
}

//----------------------------------------------------------------------

Object * ContainerInterface::getParentCellObject(Object& obj)
{
	CellProperty * const cellProperty = obj.getParentCell ();
	return cellProperty ? &cellProperty->getOwner () : 0;
}

//----------------------------------------------------------------------

const Object * ContainerInterface::getParentCellObject(const Object& obj)
{
	const CellProperty * const cellProperty = obj.getParentCell ();
	return cellProperty ? &cellProperty->getOwner () : 0;
}

//----------------------------------------------------------------------

ClientObject *  ContainerInterface::getObjectInSlot (Object & container, const char * const slotName)
{
	NOT_NULL (slotName);
	if (slotName)
	{
		const SlotId slot = SlotIdManager::findSlotId (CrcLowerString (slotName));
		if (slot != SlotId::invalid)
		{			
			SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (container);
			if (slotted)
			{						
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				return safe_cast<ClientObject *>(slotted->getObjectInSlot (slot, tmp).getObject ());
			}
		}
		else	
			WARNING (true, ("invalid slot %s", slotName));
	}

	return 0;
}

// -----------------------------------------------------------------------

bool ContainerInterface::isClientCachedOnly (const Object& object)
{
	//-- search for any ClientObjects that are not client cached
	
	//-- am I a ClientObject
	const ClientObject* const clientObject = dynamic_cast<const ClientObject*> (&object);
	if (clientObject)
	{
		//-- am I not cached?
		if (!clientObject->isClientCached ())
			return false;

		//-- are any of my contents not client cached?
		const Container* const container = ContainerInterface::getContainer (object);
		if (container)
		{
			ContainerConstIterator iter = container->begin ();
			ContainerConstIterator end  = container->end ();

			while (iter != end)
			{
				const Object* const containedObject = NetworkIdManager::getObjectById (*iter);
				if (!containedObject)
					WARNING (true, ("ContainerInterface::isClientCachedOnly: container %s contains null object %s.  cmayer says this can't happen.  asommers says it just did.\n", object.getNetworkId ().getValueString ().c_str (), (*iter).getValueString ().c_str ()));
				else
					if (!ContainerInterface::isClientCachedOnly (*containedObject))
						return false;

				++iter;
			}
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

Object * ContainerInterface::getTopmostContainer (Object& obj, bool ignoreCells)
{
	if (ignoreCells && getCell (obj))
		return 0;

	Object * currentObject = &obj;

	for ( ;currentObject; )
	{
		Object* const nextContainer = ContainerInterface::getContainedByObject (*currentObject);

		if (!nextContainer)
			break;

		//-- if the parent is a cell, we are topmost (if we are ignoring cells)
		if (ignoreCells && getCell (*nextContainer))
			break;

		currentObject = nextContainer;
	}

	return currentObject;
}

//----------------------------------------------------------------------

const Object * ContainerInterface::getTopmostContainer (const Object& obj, bool ignoreCells)
{
	return ContainerInterface::getTopmostContainer (const_cast<Object &>(obj), ignoreCells);
}


//----------------------------------------------------------------------

Object* ContainerInterface::getFirstParentInWorld(Object& child)
{
	Object * obj = &child;

	ContainedByProperty * cbp = obj->getContainedByProperty ();

	for ( ; !obj->isInWorld() && cbp; )
	{
		Object * const parent = cbp->getContainedBy ();

		if (parent)
		{
			obj = parent;
			cbp = obj->getContainedByProperty ();
		}
		else
			break;
	}

	return obj;
}

//----------------------------------------------------------------------

const Object* ContainerInterface::getFirstParentInWorld(const Object& child)
{
	return ContainerInterface::getFirstParentInWorld (const_cast<Object &>(child));
}

//----------------------------------------------------------------------

bool ContainerInterface::isNestedWithin(const Object& obj, const NetworkId& containerId)
{
	const Object* parent = getContainedByObject(obj);
	while (parent)
	{
		if (parent->getNetworkId() == containerId)
			return true;

		parent = getContainedByObject(*parent);
	}

	return false;
}

//----------------------------------------------------------------------
