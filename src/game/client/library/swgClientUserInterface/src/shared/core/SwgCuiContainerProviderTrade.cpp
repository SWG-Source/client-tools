//======================================================================
//
// SwgCuiContainerProviderTrade.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiContainerProviderTrade.h"

#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include <algorithm>

//======================================================================

SwgCuiContainerProviderTrade::SwgCuiContainerProviderTrade () :
SwgCuiContainerProvider (),
m_objects               (new ObjectWatcherVector),
m_callback              (new MessageDispatch::Callback),
m_isSelf                (false)
{
	m_callback->connect (*this, &SwgCuiContainerProviderTrade::onAddItem,          static_cast<ClientSecureTradeManager::Messages::AddItem*>    (0));
	m_callback->connect (*this, &SwgCuiContainerProviderTrade::onRemoveItem,       static_cast<ClientSecureTradeManager::Messages::RemoveItem*> (0));
}

//----------------------------------------------------------------------

SwgCuiContainerProviderTrade::~SwgCuiContainerProviderTrade ()
{
	delete m_callback;
	m_callback = 0;

	delete m_objects;
	m_objects = 0;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::getObjectVector        (ObjectWatcherVector & owv)
{
	if (isContentDirty ())
		updateObjectVector ();

	owv = *m_objects;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderTrade::populateFromTrade (const ObjectSet & currentObjSet, ObjectSet & actualObjSet)
{
	bool changed = false;

	//----------------------------------------------------------------------
	//-- add object from new container

	const Filter * const filter = getFilter ();

	CachedNetworkIdVector selfItems;
	CachedNetworkIdVector otherItems;

	ClientSecureTradeManager::getItems    (selfItems, otherItems);

	const CachedNetworkIdVector * const theItems = m_isSelf ? &selfItems : &otherItems;
	
	{
		for (CachedNetworkIdVector::const_iterator it = theItems->begin (); it != theItems->end (); ++it)
		{
			const CachedNetworkId & id = *it;
			ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
			if (obj)
			{
				//-- filter the object
				if (filter)
				{
					if (!filter->showObject (*obj))
						continue;
				}
				
				if (!obj->getAppearance ())
				{
					continue;
				}
				
				actualObjSet.insert (obj);
				if (currentObjSet.find (obj) == currentObjSet.end ())
				{
					m_objects->push_back (ObjectWatcher (obj));
					changed = true;
				}
			}
		}
	}

	return changed;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::updateObjectVector ()
{
	bool changed = false;
	
	ObjectSet currentObjSet;
	
	{
		const Filter * const filter = getFilter ();
		
		for (ObjectWatcherVector::iterator it = m_objects->begin (); it != m_objects->end ();)
		{
			const ObjectWatcher & watcher = *it;
			const ClientObject * const obj = watcher.getPointer ();
			
			//-- re-filter the object in case filter or object has changed
			if (obj && (!filter || filter->showObject (*obj)))
			{
				currentObjSet.insert (obj);
				++it;
			}
			else
			{
				it = m_objects->erase (it);
				changed = true;
			}
		}
	}
	
	ObjectSet actualObjSet;

	populateFromTrade (currentObjSet, actualObjSet);
	
	//----------------------------------------------------------------------
	//-- remove objects that are no longer in the container (s)

	{
		for (ObjectSet::iterator it = currentObjSet.begin (); it != currentObjSet.end (); ++it)
		{
			const ClientObject * obj = *it;
			const ObjectSet::iterator fit = actualObjSet.find (obj);
			if (fit == actualObjSet.end ())
			{
				m_objects->erase (std::remove (m_objects->begin (), m_objects->end (), obj), m_objects->end ());
				changed = true;

			}
			//-- erase the element from the set to speed up the find in future iterations of this loop
			else
				actualObjSet.erase (fit);
		}
	}

	setContentDirty (false);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::setObjectSorting (const IntVector & iv)
{
	if (iv.size () != m_objects->size ())
	{
		WARNING (true, ("bad vector sizes"));
		return;
	}

	ObjectWatcherVector newObjects;
	const int size = m_objects->size ();
	newObjects.reserve (size);

	int which = 0;

	for (IntVector::const_iterator it = iv.begin (); it != iv.end (); ++it, ++which)
	{
		const int index = *it;

		if (index < 0 || index >= size)
		{
			DEBUG_FATAL (true, ("bad index %d for sort at position %d", index, which));
			return;
		}
		ClientObject * const obj = (*m_objects) [index];
		DEBUG_WARNING (obj == NULL, ("setObjectSorting(): NULL object found, unexpected"));
		newObjects.push_back (ObjectWatcher (obj));
	}

	*m_objects = newObjects;

	setContentDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::onAddItem                   (const ClientSecureTradeManager::Messages::AddItem::Payload & payload)
{
	//-- self
	if (payload.first)
	{
		if (m_isSelf)
			setContentDirty (true);
	}
	else if (!m_isSelf)
		setContentDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::onRemoveItem                (const ClientSecureTradeManager::Messages::RemoveItem::Payload & payload)
{
	//-- self
	if (payload.first)
	{
		if (m_isSelf)
			setContentDirty (true);
	}
	else if (!m_isSelf)
		setContentDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderTrade::setIsSelf                   (bool b)
{
	m_isSelf = b;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderTrade::isDropOk (const ClientObject & obj) const
{
	//-- only accept items that are on the player and aren't appearance inventory objects.
	const Object * container = ContainerInterface::getContainedByObject(obj);
	if(CuiInventoryManager::getPlayerAppearanceInventory() == container)
		return false;

	return CuiInventoryManager::isOnCreature (obj);

}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderTrade::handleDragOver          (const CuiDragInfo & cdinfo, bool & ok)
{
	ok = false;
	if (m_isSelf && cdinfo.type == CuiDragInfoTypes::CDIT_object)
	{
		const ClientObject * const clientObject = cdinfo.getClientObject ();
		if (clientObject)
		{
			//-- only accept items that are on the player.
			ok = isDropOk (*clientObject);
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderTrade::handleDrop              (const CuiDragInfo & cdinfo)
{ 
	if (m_isSelf && cdinfo.type == CuiDragInfoTypes::CDIT_object)
	{
		const ClientObject * const clientObject = cdinfo.getClientObject ();
		if (clientObject)
		{
			if (isDropOk (*clientObject))
			{
				ClientSecureTradeManager::addItem (clientObject->getNetworkId ());
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderTrade::handleDragStart         (const CuiDragInfo & cdinfo)
{
	if (m_isSelf && cdinfo.type == CuiDragInfoTypes::CDIT_object)
	{
		const ClientObject * const clientObject = cdinfo.getClientObject ();
		if (clientObject)
		{
//			ClientSecureTradeManager::removeItem (clientObject->getNetworkId ());
		}
	}
	
	return false;
}

//======================================================================
