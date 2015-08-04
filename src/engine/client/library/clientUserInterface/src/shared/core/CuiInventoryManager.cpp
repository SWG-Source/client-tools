// ======================================================================
//
// CuiInventoryManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiInventoryManager.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/VehicleObject.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include <cstdio>

// ======================================================================

namespace
{
	bool s_installed = false;

	namespace Tranceivers
	{
		MessageDispatch::Transceiver<const CuiInventoryManager::Messages::ItemOpenRequest::Payload &, CuiInventoryManager::Messages::ItemOpenRequest> 
			itemOpenRequest;
		MessageDispatch::Transceiver<const CuiInventoryManager::Messages::ItemOpenRequestNewWindow::Payload &, CuiInventoryManager::Messages::ItemOpenRequestNewWindow> 
			itemOpenRequestNewWindow;
		MessageDispatch::Transceiver<const CuiInventoryManager::Messages::ItemCloseAllInstances::Payload &, CuiInventoryManager::Messages::ItemCloseAllInstances> 
			itemCloseAllInstances;
		MessageDispatch::Transceiver<const CuiInventoryManager::Messages::SplitContainer::Payload &, CuiInventoryManager::Messages::SplitContainer> 
			splitContainer;
	}

	namespace MessageNames
	{
		const char * const OpenedContainerMessage   = "OpenedContainerMessage";
		const char * const ClosedContainerMessage   = "ClosedContainerMessage";
	};

	//----------------------------------------------------------------------

	class Listener : 
	public MessageDispatch::Receiver
	{
	public:
		
		//----------------------------------------------------------------------
		
		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::OpenedContainerMessage);
			connectToMessage (MessageNames::ClosedContainerMessage);
		}
		
		//----------------------------------------------------------------------
		
		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------
			
			if(message.isType(MessageNames::OpenedContainerMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();

				typedef std::pair<NetworkId, std::string> ContainerIdentifier;
				typedef std::pair<int, ContainerIdentifier> OpenedContainerMessagePayload;

				const GenericValueTypeMessage<OpenedContainerMessagePayload> gvt (ri);

				const OpenedContainerMessagePayload & payload = gvt.getValue ();
				const ContainerIdentifier & cid = payload.second;
				const int sequence              = payload.first;

				ClientObject * const obj = dynamic_cast<ClientObject *>(CachedNetworkId (cid.first).getObject ());

				if (!obj)
					WARNING (true, ("Received ContainerOpenedMessage for item %s which does not exist.", cid.first.getValueString ().c_str ()));
				else
					CuiInventoryManager::handleItemOpen (*obj, cid.second, 0, 0, true, sequence);
			}
			else if (message.isType (MessageNames::ClosedContainerMessage))
			{
				//-- client doesn't really care about this message
			}
		}
	};

	Listener * s_listener = 0;

	int s_sequence = 0;

	typedef stdset<int>::fwd IntSet;
	IntSet  s_sequenceIgnoreSet;

	typedef stdmap<NetworkId, int>::fwd ContainerRefMap;
	ContainerRefMap s_containerRefMap;

	NetworkId m_idConfirmDelete;

	void onMessageBoxConfirmDelete (const CuiMessageBox & box)
	{
		if (box.completedAffirmative ())
		{
			CuiInventoryManager::destroyObject (m_idConfirmDelete, true);
		}
	}
}

//-----------------------------------------------------------------

void CuiInventoryManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));
	s_listener  = new Listener;
	s_sequenceIgnoreSet.clear ();
	s_containerRefMap.clear ();
	s_installed = true;
}

//-----------------------------------------------------------------

void CuiInventoryManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	delete s_listener;
	s_listener = 0;
	s_sequenceIgnoreSet.clear ();
	s_containerRefMap.clear ();
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiInventoryManager::reset   ()
{
	s_containerRefMap.clear ();
}

//----------------------------------------------------------------------

void CuiInventoryManager::update  (float)
{
	for (ContainerRefMap::iterator it = s_containerRefMap.begin (); it != s_containerRefMap.end ();)
	{
		NetworkId const & id = (*it).first;
		Object * const obj = NetworkIdManager::getObjectById (id);
		if (obj)
		{
			obj->scheduleForAlter ();

			++it;
		}
		else
		{
			s_containerRefMap.erase(it++);
		}
	}
}

//----------------------------------------------------------------------

bool CuiInventoryManager::findLeastOccupiedArrangementForEquipping (const TangibleObject & obj, TangibleObject & target, size_t & arrangementId, int & volumeRequired)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	SlottedContainer * const slotted       = ContainerInterface::getSlottedContainer (target);

	if (!slotted)
		return false;

	const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(obj);

	if (!slottedProperty)
		return false;

	typedef std::vector<int> SizeTVector;
	SizeTVector returnList;
	Container::ContainerErrorCode tmp = Container::CEC_Success;

	slotted->getValidArrangements (obj, returnList, tmp);

	bool   found             = false;
	size_t min_arrangementId = 0;
	int    min_volume        = 16384;
	size_t min_slots         = 16384;

	//-- iterate through the valid arrangements for the object in the target

	for (SizeTVector::const_iterator it = returnList.begin (); it != returnList.end (); ++it)
	{
		const size_t loopArrangementId = *it;

		typedef std::set<Object *> ObjectSet;
		ObjectSet blockingObjects;

		int volume_required = 0;
		//-- iterate through the slots for this arrangement, computing total volume required to displace

		const SlottedContainmentProperty::SlotArrangement & slotIds = slottedProperty->getSlotArrangement (loopArrangementId);
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		for (SlottedContainmentProperty::SlotArrangement::const_iterator ait = slotIds.begin (); ait != slotIds.end (); ++ait)
		{
			const SlotId & slot = *ait;
			Container::ContainedItem contained = slotted->getObjectInSlot (slot, tmp);
			ClientObject * const containedObject     = dynamic_cast<ClientObject*>(contained.getObject ());

			if (containedObject)
			{
				const std::pair<ObjectSet::iterator, bool> insert_retval = blockingObjects.insert (containedObject);

				if (insert_retval.second)
				{
					// @todo this hack should be removed when objects actually have volume
					volume_required += std::max (1, containedObject->getVolume ());
				}
			}
		}

		//----------------------------------------------------------------------

		if (volume_required < min_volume || (volume_required == min_volume && slotIds.size () < min_slots))
		{
			min_volume        = volume_required;
			min_arrangementId = loopArrangementId;
			min_slots         = slotIds.size ();
			found             = true;

			//- go ahead and break if we found a single completely unoccupied slot
			if (min_volume == 0 && min_slots == 1)
				break;
		}
	}

	if (found)
	{
		arrangementId  = min_arrangementId;
		volumeRequired = min_volume;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------
/**
*
* @param obj the object to remove from the target
* @param target the object (usually the player) from which to unequip the item
*/

bool CuiInventoryManager::unequipItem (TangibleObject & obj, TangibleObject & target)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	//-----------------------------------------------------------------
	//-- @todo check if the volume can be held by the inventory, if not, abort

	SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (target);

	//-----------------------------------------------------------------
	//-- find the inventory container if it exists

	const SlotId & inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("inventory"));
	DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));
	Container::ContainerErrorCode tmp = Container::CEC_Success;

	const CachedNetworkId containedInventory = slotted->getObjectInSlot (inventorySlot, tmp);

	ClientObject * const inventory = static_cast<ClientObject *>(containedInventory.getObject ()); //lint !e1744
	if (!inventory)
	{
		DEBUG_REPORT_LOG(true, ("No inventory for player character."));
		return false;
	}
	//----------------------------------------------------------------------

	const bool retval = ContainerInterface::transferItemToVolumeContainer (*inventory, obj);

	WARNING (!retval, ("could not remove item."));

	if (!retval) //lint !e744
		return false;

	return true;
}

//----------------------------------------------------------------------

bool CuiInventoryManager::itemIsEquipped (const TangibleObject & obj, const TangibleObject & target)
{
	const ContainedByProperty * const cbp = ContainerInterface::getContainedByProperty (obj);

	if (cbp)
	{
		return (&target == static_cast<const Object *>(cbp->getContainedBy ()));
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiInventoryManager::equipItem (TangibleObject & obj, TangibleObject & target)
{
	size_t arrangementId = 0;
	int requiredVolume = 0;

	if (CuiInventoryManager::findLeastOccupiedArrangementForEquipping (obj, target, arrangementId, requiredVolume))
	{
		// @todo make sure that the required volume is satisfied by this container

		if (!CuiInventoryManager::equipItem (obj, target, arrangementId))
		{
			WARNING (true, ("somehow could not add the item in the specified arrangement."));
			return false; //lint !e527
		}
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

/**
*
* @param obj the object to equip onto the target
* @param target the object (usually the player) upon which to equip the item
*/

bool CuiInventoryManager::equipItem (TangibleObject & obj, TangibleObject & target, size_t arrangementId)
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	SlottedContainer * const slotted       = ContainerInterface::getSlottedContainer (target);

	if (!slotted)
		return false;

	bool inventoryChanged = false;
	if (Game::getSinglePlayer() || !Game::getPlayer())
	{
		//-----------------------------------------------------------------
		//-- find the inventory container if it exists, so we know where to dump displaced stuff

		const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("inventory"));
		DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		const CachedNetworkId containedInventory = slotted->getObjectInSlot (inventorySlot, tmp);

		ClientObject * const inventory = static_cast<ClientObject *>(containedInventory.getObject ()); //lint !e1774

		//-----------------------------------------------------------------
		//-- remove all objects blocking this one.

		const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(obj);

		if (!slottedProperty)
			return false;

		//-----------------------------------------------------------------
		//-- first count the total volume that must be removed to accomodate this item
		//-- and store the set of objects that must be removed

		typedef std::set<Object *> ObjectSet;
		ObjectSet blockingObjects;
//		size_t  totalSlotsRequired = 0;

		{
			const SlottedContainmentProperty::SlotArrangement & slotIds = slottedProperty->getSlotArrangement (arrangementId);
			for (SlottedContainmentProperty::SlotArrangement::const_iterator it = slotIds.begin (); it != slotIds.end (); ++it)
			{
				const SlotId & slot = *it;
				Container::ContainedItem contained = slotted->getObjectInSlot (slot, tmp);
				ClientObject * const containedObject     = dynamic_cast<ClientObject*>(contained.getObject ());

				if (containedObject)
				{
					IGNORE_RETURN(blockingObjects.insert(containedObject));
//					const std::pair<ObjectSet::iterator, bool> insert_retval = blockingObjects.insert (containedObject);

//					if (insert_retval.second)
//						totalSlotsRequired += containedObject->getVolume ();
				}
			}
		}

		//-----------------------------------------------------------------
		//-- @todo check if the displaced volume can be held by the inventory, if not, abort

		inventoryChanged = !blockingObjects.empty ();

		//-----------------------------------------------------------------
		//-- remove all the blocking items

		{
			for (ObjectSet::iterator it = blockingObjects.begin (); it != blockingObjects.end (); ++it)
			{
				//@todo, The object set should probably be a set of ClientObjects.
				ClientObject * const containedObject = dynamic_cast<ClientObject*>(*it);
				if (containedObject)
				{
					//-- if there is no inventory, just dump the object in the world.  This should not happen in gameplay
					
					if (!inventory)
					{
						if (!ContainerInterface::transferItemToWorld (*containedObject, target.getTransform_o2w ()))
							WARNING (true, ("could not transfer item from equipment to world."));
					}
					else if (!ContainerInterface::transferItemToVolumeContainer (*inventory, *containedObject))
						WARNING (true, ("could not transfer item from equipment to inventory."));
				}
			}
		}
	}

	//-----------------------------------------------------------------
	//-- now, FINALLY try to add the object to the equipment slots.
	if (!ContainerInterface::transferItemToSlottedContainer (target, obj, arrangementId))
		WARNING (true, ("somehow could not add the item in the specified arrangement."));
	else
		inventoryChanged = true;

	return inventoryChanged;
}

//----------------------------------------------------------------------

bool CuiInventoryManager::canPotentiallyEquip (const TangibleObject & obj, const TangibleObject & target)
{
	const SlottedContainer * const slotted = target.getSlottedContainerProperty ();
	if (!slotted)
		return false;

	static std::vector<int>  returnList;
	returnList.clear ();

	Container::ContainerErrorCode error = Container::CEC_Success;

	slotted->getValidArrangements (obj, returnList, error, true);

	return !returnList.empty ();
}

//----------------------------------------------------------------------

void  CuiInventoryManager::closeAllInstancesOfItem (ClientObject & obj, const std::string & slotname)
{
	Tranceivers::itemCloseAllInstances.emitMessage (Messages::ItemCloseAllInstances::Payload (&obj, slotname));
}

//----------------------------------------------------------------------

void CuiInventoryManager::requestItemOpen (ClientObject & obj, const std::string & slotname, int x, int y, bool newWindow, bool ignoreResult)
{
	static const uint32 hash_open = Crc::normalizeAndCalculate("openContainer");

	++s_sequence;
	if (s_sequence == 0)
		++s_sequence;
	
	if (ignoreResult)
		s_sequenceIgnoreSet.insert (s_sequence);
	
	if (Game::getSinglePlayer ())
	{
		handleItemOpen (obj, slotname, x, y, newWindow, s_sequence);
	}
	else
	{
		//-- only request an /openContainer if the client thinks it doesn't already has the container open
		const ContainerRefMap::iterator it = s_containerRefMap.find (obj.getNetworkId ());
		if (it == s_containerRefMap.end () || (*it).second <= 0)
		{
			char buf [128];
			_snprintf (buf, sizeof (buf), "%d %s", s_sequence, slotname.c_str ());
			
			const Unicode::String & params = Unicode::narrowToWide (buf);
			
			ClientCommandQueue::enqueueCommand (hash_open, obj.getNetworkId (), params);
			return;
		}
		
		//-- we are already observing the container, just open it normally
		handleItemOpen (obj, slotname, x, y, newWindow, s_sequence);
	}
}

//----------------------------------------------------------------------

void CuiInventoryManager::handleItemOpen      (ClientObject & obj, const std::string & slotname, int x, int y, bool newWindow, int sequence)
{
	{
		const ContainerRefMap::iterator it = s_containerRefMap.find (obj.getNetworkId ());
		if (it != s_containerRefMap.end ())
			++((*it).second);
		else
			s_containerRefMap.insert (std::make_pair (obj.getNetworkId (), 1));
	}

	{
		const IntSet::iterator it = s_sequenceIgnoreSet.find (sequence);
		
		if (it != s_sequenceIgnoreSet.end ())
		{
			s_sequenceIgnoreSet.erase (it);
			return;
		}
	}
	
	
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	typedef MessageDispatch::Message<std::pair<ClientObject *, std::string> > MsgType;
	
	if (!newWindow)
	{
		Tranceivers::itemOpenRequest.emitMessage (Messages::ItemOpenRequest::Payload (&obj, slotname));
	}
	else
	{
		Tranceivers::itemOpenRequestNewWindow.emitMessage (Messages::ItemOpenRequestNewWindow::Payload (&obj, slotname));
	}

	UNREF (x);
	UNREF (y);
}

//----------------------------------------------------------------------

void CuiInventoryManager::notifyItemClosed    (ClientObject & obj, const std::string & slotname)
{
	const NetworkId & id = obj.getNetworkId ();

	const ContainerRefMap::iterator it = s_containerRefMap.find (obj.getNetworkId ());
	if (it != s_containerRefMap.end ())
	{
		const int ref = --((*it).second);
		if (ref > 0)
			return;
	}
	else
	{
		WARNING (true, ("notifyItemClosed for item [%d] [%s] with no entry in s_containerRefMap", 
			obj.getNetworkId ().getValueString ().c_str (), Unicode::wideToNarrow (obj.getLocalizedName ()).c_str ()));
		return; 
	}

	//-- only send the close message when all references to this container are closed

	s_containerRefMap.erase (id);

	static const uint32 hash_close = Crc::normalizeAndCalculate ("closeContainer");
	char buf [128];
	_snprintf (buf, sizeof (buf), "%s", slotname.c_str ());

	const Unicode::String & params = Unicode::narrowToWide (buf);

	ClientCommandQueue::enqueueCommand (hash_close, obj.getNetworkId (), params);
}

//----------------------------------------------------------------------

bool  CuiInventoryManager::isNestedDatapad (const ClientObject & obj, const CreatureObject * owner)
{
	if (!owner && 0 == (owner = Game::getPlayerCreature ()))
		return false;

	const ClientObject  * const datapad = owner->getDatapadObject ();

	if (!datapad)
		return false;

	return isNestedInContainer (obj, *datapad);
}

//----------------------------------------------------------------------

bool  CuiInventoryManager::isNestedInventory (const ClientObject & obj, const CreatureObject * owner)
{
	if (!owner && 0 == (owner = Game::getPlayerCreature ()))
		return false;

	const ClientObject  * const inventory = owner->getInventoryObject ();

	if (!inventory)
		return false;

	return isNestedInContainer (obj, *inventory);
}

//----------------------------------------------------------------------

bool  CuiInventoryManager::isNestedAppearanceInventory(const ClientObject & obj, const CreatureObject * owner)
{
	if (!owner && 0 == (owner = Game::getPlayerCreature ()))
		return false;

	const ClientObject  * const inventory = owner->getAppearanceInventoryObject ();

	if (!inventory)
		return false;

	return isNestedInContainer (obj, *inventory);
}


//----------------------------------------------------------------------

bool  CuiInventoryManager::isNestedEquipped  (const ClientObject & obj, const CreatureObject * owner, bool specialsReturn)
{
	if (!owner && 0 == (owner = Game::getPlayerCreature ()))
		return false;

	if (owner->getInventoryObject () == &obj || owner->getDatapadObject () == &obj || owner->getBankObject () == &obj)
		return specialsReturn;

	const Object * immediateSubContainer = 0;
	const bool isNestedInOwner = isNestedInContainer (obj, *owner, &immediateSubContainer);
	
	if (isNestedInOwner)
	{
		if (immediateSubContainer)
		{
			if (owner->getInventoryObject () == immediateSubContainer || owner->getDatapadObject () == immediateSubContainer || owner->getBankObject () == immediateSubContainer)
				return false;
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiInventoryManager::isOnCreature        (const ClientObject & obj, const CreatureObject * owner)
{
	if (!owner && 0 == (owner = Game::getPlayerCreature ()))
		return false;

	return isNestedInventory (obj, owner) || isNestedDatapad (obj, owner) || isNestedEquipped (obj, owner, true);
}

//----------------------------------------------------------------------

bool  CuiInventoryManager::isNestedInContainer (const ClientObject & obj, const ClientObject & container, const Object ** immediateSubContainer)
{
	if (&obj == &container)
		return false;

	const Object * cur = &obj;
	
	if (immediateSubContainer)
		*immediateSubContainer = 0;

	while (cur && 0 != (cur = ContainerInterface::getContainedByObject (*cur)))
	{
		//-- recursive containment!?
		if (cur == &obj)
			return false;

		if (cur == &container)
			return true;

		if (immediateSubContainer)
			*immediateSubContainer = cur;
	}

	if (immediateSubContainer)
		*immediateSubContainer = 0;

	return false;
}

//-----------------------------------------------------------------

bool CuiInventoryManager::isPlayerInventory   (const Object & obj)
{
	const CreatureObject * owner = Game::getPlayerCreature ();
	return (owner && owner->getInventoryObject () == &obj);
}

//----------------------------------------------------------------------

bool CuiInventoryManager::isPlayerAppearanceInventory(const Object & obj)
{
	const CreatureObject * owner = Game::getPlayerCreature();
	return (owner && owner->getAppearanceInventoryObject() == &obj);
}

//----------------------------------------------------------------------

ClientObject * CuiInventoryManager::getPlayerInventory ()
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player != NULL)
		return player->getInventoryObject();

	return 0;
}

//----------------------------------------------------------------------

ClientObject * CuiInventoryManager::getPlayerDatapad    ()
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (player != NULL)
		return player->getDatapadObject();

	return 0;
}

//----------------------------------------------------------------------

ClientObject * CuiInventoryManager::getPlayerAppearanceInventory()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(player != NULL)
		return player->getAppearanceInventoryObject();
	return 0;
}

//----------------------------------------------------------------------

void CuiInventoryManager::pickupObject (const NetworkId & id)
{		
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player || !player->getInventoryObject())
		return;
	
	ClientObject * const obj = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (id));
	if (!obj)
		return;
	
	const ClientObject * const containedBy = safe_cast<const ClientObject *>(ContainerInterface::getContainedByObject (*obj));
	
	if (containedBy && containedBy == player->getInventoryObject () || containedBy == player)
		return;

	InstallationObject * const installation = dynamic_cast<InstallationObject *> (obj);
	BuildingObject *     const building     = dynamic_cast<BuildingObject *>     (obj);
	VehicleObject *      const vehicle      = dynamic_cast<VehicleObject *>      (obj);
		
	if (!vehicle && !building && !installation)
		ContainerInterface::transferItemToVolumeContainer(*player->getInventoryObject(), *obj);
}

	//----------------------------------------------------------------------

void CuiInventoryManager::equipObject (const NetworkId & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	TangibleObject * const tangible     = dynamic_cast<TangibleObject *>     (NetworkIdManager::getObjectById (id));
	
	if (tangible)
		CuiInventoryManager::equipItem (*tangible, *player);
}

//----------------------------------------------------------------------

void CuiInventoryManager::unequipObject (const NetworkId & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	TangibleObject * const tangible     = dynamic_cast<TangibleObject *>     (NetworkIdManager::getObjectById (id));
	
	if (tangible)
		CuiInventoryManager::unequipItem (*tangible, *player);
}

//----------------------------------------------------------------------

void CuiInventoryManager::dropObject (const NetworkId & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	TangibleObject * const tangible     = dynamic_cast<TangibleObject *>     (NetworkIdManager::getObjectById (id));

	if (!tangible)
		return;

	const bool isNestedInventory = CuiInventoryManager::isNestedInventory (*tangible, player);
	const bool isNestedEquipped  = CuiInventoryManager::isNestedEquipped  (*tangible, player);

	if (!isNestedInventory && !isNestedEquipped)
		return;
	
	const CellProperty * const parentCell = player->getParentCell ();
	
	if (!parentCell || parentCell == CellProperty::getWorldCellProperty ())
	{
		Transform t (player->getTransform_o2w ());
		t.move_p (player->getObjectFrameK_p () * 1.0f);
		
		const Vector position (t.getPosition_p ());
		
		const TerrainObject * const terrain = TerrainObject::getConstInstance ();
		if (terrain)
		{
			float y = 0.0f;
			if (terrain->getHeight (position, y))
				t.move_p (Vector (0.0f, y - position.y, 0.0f));
		}					
		
		ContainerInterface::transferItemToWorld (*tangible, t);
		
		//@todo object should be transfered to world _then_ moved, to handle portals properly
	}
	else
	{
		const Transform & t = player->getTransform_o2c();
		
		ContainerInterface::transferItemToCell(*safe_cast<ClientObject *>(const_cast<Object *>(&parentCell->getOwner ())), *tangible, t);
	}			
}

//----------------------------------------------------------------------

void CuiInventoryManager::splitContainer (const NetworkId & id)
{
	const Object * const object = NetworkIdManager::getObjectById (id);
	const ResourceContainerObject * const resourceContainerObject = dynamic_cast<const ResourceContainerObject * const>(object);
	const FactoryObject * const factoryObject = dynamic_cast<const FactoryObject * const>(object);
	if (resourceContainerObject == NULL && factoryObject == NULL)
		return;
	
	const Object * const parent = ContainerInterface::getContainedByObject(*object);
	if (!parent)
		return;
	
	IGNORE_RETURN(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::ResourceSplitter));
	
	// Send a message the split window to let it know what we are splitting

	Tranceivers::splitContainer.emitMessage(Messages::SplitContainer::Payload(parent->getNetworkId(), object->getNetworkId()));
}

//----------------------------------------------------------------------

void CuiInventoryManager::destroyObject (const NetworkId & id, bool confirmed)
{
	ClientObject * const obj = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (id));
	if (!obj)
		return;

	if (obj == Game::getPlayer ())
		return;
	
	if (!confirmed && CuiPreferences::getConfirmObjDelete ())
	{
		m_idConfirmDelete = id;
		
		Unicode::String str;
		CuiStringVariablesManager::process (CuiStringIds::confirm_obj_destroy_prose, obj->getLocalizedName (), Unicode::emptyString, Unicode::emptyString, str);
		CuiMessageBox::createYesNoBox (str, onMessageBoxConfirmDelete);
		return;
	}
	
	if(Game::getSinglePlayer())
	{			
		obj->kill();
	}
	else
	{
		static const uint32 destroy_object  = Crc::normalizeAndCalculate("serverDestroyObject");
		static const Unicode::String params = Unicode::narrowToWide ("1");
		ClientCommandQueue::enqueueCommand (destroy_object, id, params);
	}
}


void CuiInventoryManager::equipAppearanceItem(NetworkId const & id)
{
	TangibleObject * appearanceInventory = safe_cast<TangibleObject*>(getPlayerAppearanceInventory());
	TangibleObject * transferItem = safe_cast<TangibleObject*>(NetworkIdManager::getObjectById(id));
	if(transferItem && appearanceInventory)
	{	
			CuiInventoryManager::equipItem (*transferItem, *appearanceInventory);
	}
}

void CuiInventoryManager::unequipAppearanceItem(NetworkId const & id)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	TangibleObject * transferItem = safe_cast<TangibleObject*>(NetworkIdManager::getObjectById(id));
	if(transferItem)
	{	
		CuiInventoryManager::unequipItem (*transferItem, *player);
	}
}
// ======================================================================

