//======================================================================
//
// SwgCuiContainerProviderDefault.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiInventoryState.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Container.h"
#include "sharedObject/VolumeContainer.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include <algorithm>

//======================================================================

namespace
{
	namespace Slots
	{
		const char * const default_weapon = "default_weapon";
		const char * const mission_bag    = "mission_bag";
	}
}

//----------------------------------------------------------------------

SwgCuiContainerProviderDefault::SwgCuiContainerProviderDefault () :
SwgCuiContainerProvider (),
m_containerId           (new CachedNetworkId),
m_containerSecondaryId  (new CachedNetworkId),
m_containerTertiaryId   (new CachedNetworkId),
m_slotName              (),
m_objects               (new ObjectWatcherVector),
m_callback              (new MessageDispatch::Callback),
m_soundType             (ST_none)
{
	m_callback->connect (*this, &SwgCuiContainerProviderDefault::onAddedToContainer,          static_cast<ClientObject::Messages::AddedToContainer*> (0));
	m_callback->connect (*this, &SwgCuiContainerProviderDefault::onRemovedFromContainer,          static_cast<ClientObject::Messages::RemovedFromContainer*> (0));
}

//----------------------------------------------------------------------

SwgCuiContainerProviderDefault::~SwgCuiContainerProviderDefault ()
{
	delete m_callback;
	m_callback = 0;

	setContainerObject (0, std::string ());

	delete m_containerId;
	delete m_containerSecondaryId;
	delete m_containerTertiaryId;
	delete m_objects;

	m_containerId = 0;
	m_containerSecondaryId = 0;
	m_containerTertiaryId = 0;
	m_objects = 0;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::onAddedToContainer (const ClientObject::Messages::ContainerMsg & msg)
{
	const ClientObject * const obj = msg.first;
	
	if (obj)
	{
		const NetworkId & id = obj->getNetworkId ();
		
		if (id == *m_containerId || id == *m_containerSecondaryId || id == *m_containerTertiaryId)
			setContentDirty (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::onRemovedFromContainer (const ClientObject::Messages::ContainerMsg & msg)
{
	const ClientObject * const obj = msg.first;
	
	if (obj)
	{
		const NetworkId & id = obj->getNetworkId ();
		
		if (id == *m_containerId || id == *m_containerSecondaryId || id == *m_containerTertiaryId)
			setContentDirty (true);
	}
}

//----------------------------------------------------------------------

const ClientObject * SwgCuiContainerProviderDefault::getContainerObject () const
{
	return dynamic_cast<const ClientObject *>(m_containerId->getObject ());
}

//----------------------------------------------------------------------

ClientObject * SwgCuiContainerProviderDefault::getContainerObject ()
{
	return dynamic_cast<ClientObject *>(m_containerId->getObject ());
}

//----------------------------------------------------------------------

const ClientObject * SwgCuiContainerProviderDefault::getContainerSecondaryObject () const
{
	return dynamic_cast<const ClientObject *>(m_containerSecondaryId->getObject ());
}

//----------------------------------------------------------------------

ClientObject * SwgCuiContainerProviderDefault::getContainerSecondaryObject ()
{
	return dynamic_cast<ClientObject *>(m_containerSecondaryId->getObject ());
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::getObjectVectorFromSlot (ObjectWatcherVector & owv)
{
	owv.clear ();

	Object * const containerObj = getContainerObject ();

	if (!containerObj)
		return;

	ClientObject * const content   = ContainerInterface::getObjectInSlot (*containerObj, m_slotName.c_str ());
	if (content)
	{
		owv.push_back (ObjectWatcher (content));
	}
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::getObjectVector        (ObjectWatcherVector & owv)
{
	if (isContentDirty ())
		updateObjectVector ();

	owv = *m_objects;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderDefault::populateFromContainer (const ClientObject & containerObject, const ObjectSet & currentObjSet, ObjectSet & actualObjSet)
{
	const Container * const container = ContainerInterface::getContainer (containerObject);

	if (!container)
		return false;

	const ClientObject * hair           = 0;
	const ClientObject * inventory      = 0;
	const ClientObject * datapad        = 0;
	const ClientObject * playerObject   = 0;
	const ClientObject * bankObject     = 0;
	const ClientObject * defaultWeapon  = 0;
	const ClientObject * missionBag     = 0;
	const ClientObject * appearanceInv  = 0;

	const CreatureObject * const creature = dynamic_cast<const CreatureObject *>(&containerObject);

	if (creature)
	{
		hair          = creature->getHairObject      ();
		inventory     = creature->getInventoryObject ();
		datapad       = creature->getDatapadObject   ();
		playerObject  = creature->getPlayerObject    ();
		bankObject    = creature->getBankObject      ();
		defaultWeapon = creature->getEquippedObject  (Slots::default_weapon);
		missionBag    = creature->getEquippedObject  (Slots::mission_bag);
		appearanceInv = creature->getAppearanceInventoryObject();
	}

	bool changed = false;

	//----------------------------------------------------------------------
	//-- add object from new container

	const Filter * const filter = getFilter ();

	ContainerConstIterator containerIterator = container->begin();
	for (; containerIterator != container->end(); ++containerIterator)
	{
		const CachedNetworkId & id = *containerIterator;
		ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
		
		if (obj)
		{
			//-- filter the object
			if (filter)
			{
				if (!filter->showObject (*obj))
					continue;
			}
			
			if (obj == hair || obj == inventory || obj == datapad || obj == playerObject || obj == bankObject || obj == defaultWeapon || obj == missionBag || obj == appearanceInv )
			{
				continue;
			}

			else if (!obj->getAppearance () && obj->getGameObjectType () != SharedObjectTemplate::GOT_data_token)
			{
				WARNING (true, ("SwgCuiContainerProviderDefault object [%s] (%s) has no appearance", Unicode::wideToNarrow (obj->getLocalizedName ()).c_str (), obj->getObjectTemplateName ()));
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

	if(&containerObject == creature && appearanceInv && CuiPreferences::getShowAppearanceInventory())
	{
		if(m_containerTertiaryId)
			*m_containerTertiaryId = *appearanceInv;

		const Container * const appContainer = ContainerInterface::getContainer (*appearanceInv);
		ContainerConstIterator containerIterator = appContainer->begin();
		for (; containerIterator != appContainer->end(); ++containerIterator)
		{
			const CachedNetworkId & id = *containerIterator;
			ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());

			if (obj)
			{
				//-- filter the object
				if (filter)
				{
					if (!filter->showObject (*obj))
						continue;
				}

				if (obj == hair || obj == inventory || obj == datapad || obj == playerObject || obj == bankObject || obj == defaultWeapon || obj == missionBag || obj == appearanceInv )
				{
					continue;
				}

				else if (!obj->getAppearance () && obj->getGameObjectType () != SharedObjectTemplate::GOT_data_token)
				{
					WARNING (true, ("SwgCuiContainerProviderDefault object [%s] (%s) has no appearance", Unicode::wideToNarrow (obj->getLocalizedName ()).c_str (), obj->getObjectTemplateName ()));
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

void SwgCuiContainerProviderDefault::updateObjectVector ()
{
	if (!m_slotName.empty ())
	{
		getObjectVectorFromSlot (*m_objects);
		return;
	}

	ClientObject * const containerObject = getContainerObject ();

	if (!containerObject)
		return;

	const ClientObject * const secondary  = getContainerSecondaryObject ();

	bool changed = false;

	if(m_containerTertiaryId)
		*m_containerTertiaryId = CachedNetworkId(NetworkId::cms_invalid);
	
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

	if (secondary)
	{
		if (populateFromContainer (*secondary, currentObjSet, actualObjSet))
			changed = true;
	}

	if (populateFromContainer (*containerObject, currentObjSet, actualObjSet))
		changed = true;
	
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

void SwgCuiContainerProviderDefault::setContainerObject          (ClientObject * container, const std::string & slotName)
{
	ClientObject * const oldContainer = getContainerObject ();

	if (container == oldContainer && slotName == m_slotName)
		return;

	ClientObject * const player = Game::getClientPlayer ();

	applyStates ();

	checkReleaseContainerOpenReference ();

	m_slotName = slotName;

	if (container)
	{
		const bool isInventory  = CuiInventoryManager::isPlayerInventory (*container);
		*m_containerId          = *container;
		if (isInventory && player)
		{
			*m_containerSecondaryId = *player;
			CuiInventoryState::getInventoryState (*m_objects);
		}
		else
			*m_containerSecondaryId = NetworkId::cms_invalid;

		const bool isDatapad   = !isInventory && CuiInventoryManager::getPlayerDatapad () == container;
		const bool isOnPlayer  = CuiInventoryManager::isNestedInventory (*container) || CuiInventoryManager::isNestedEquipped (*container);
		
		if (isDatapad)
			m_soundType = ST_data;
		else if (isInventory)
			m_soundType = ST_default;
		else if (isOnPlayer)
			m_soundType = ST_default;
		else
			m_soundType = ST_default;

		playAppropriateSound (true);
	}
	else
	{
		playAppropriateSound (false);

		m_soundType             = ST_none;
		*m_containerId          = NetworkId::cms_invalid;
		*m_containerSecondaryId = NetworkId::cms_invalid;
	}

	setContentDirty (true);
	setProviderDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::playAppropriateSound (bool opening)
{
	switch (m_soundType)
	{
	case ST_none:
		break;
	case ST_default:
		if (opening)
			CuiSoundManager::play (CuiSounds::backpack_open);
		else
			CuiSoundManager::play (CuiSounds::backpack_close);
		break;
	case ST_data:
		break;
	case ST_hopper:
		break;
	}

}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::checkReleaseContainerOpenReference ()
{
	ClientObject * const oldContainer = getContainerObject ();

	//-- release global ref on object
	if (oldContainer)
		CuiInventoryManager::notifyItemClosed (*oldContainer, m_slotName);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::setObjectSorting (const IntVector & iv)
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

void SwgCuiContainerProviderDefault::onOpeningAndClosing    (bool opening)
{
	playAppropriateSound (opening);
}

//----------------------------------------------------------------------

int SwgCuiContainerProviderDefault::getTotalVolume          () const
{
	int volume = 0;

	if (m_slotName.empty ())
	{
		const ClientObject * const obj          = getContainerObject ();
		const VolumeContainer * const container = obj ? ContainerInterface::getVolumeContainer (*obj) : 0;
	
		if (container)
			volume = container->getTotalVolume ();
	}
	
	return volume;
}

//----------------------------------------------------------------------

int SwgCuiContainerProviderDefault::getCurrentVolume        () const
{
	int volume = 0;

	if (m_slotName.empty ())
	{
		const ClientObject * const obj          = getContainerObject ();
		const VolumeContainer * const container = obj ? ContainerInterface::getVolumeContainer (*obj) : 0;
		
		if (container)
			volume = container->getCurrentVolume ();
	}

	return volume;
}

//----------------------------------------------------------------------

int SwgCuiContainerProviderDefault::getTotalVolumeLimitedByParents () const
{
	int volume = 0;

	if (m_slotName.empty ())
	{
		const ClientObject * const obj          = getContainerObject ();
		const VolumeContainer * const container = obj ? ContainerInterface::getVolumeContainer (*obj) : 0;
		
		if (container)
			volume = container->getTotalVolumeLimitedByParents ();
	}

	return volume;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::applyStates                 () const
{
	const Object * const containerObj = getContainerObject ();
	
	if (containerObj)
	{
		if (CuiInventoryManager::isPlayerInventory (*containerObj))
		{
			CuiInventoryState::setInventoryState (*m_objects);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderDefault::tryToApplyOrdering(std::vector<NetworkId> const &ordering)
{
	//DEBUG_WARNING(true, ("SwgCuiContainerProviderDefault::tryToApplyOrdering"));
	updateObjectVector();
	//DEBUG_WARNING(true, ("----------------------"));
	/*unsigned int c;
	for(c = 0; c < ordering.size(); ++c)
	{
		DEBUG_WARNING(true, ("ordering[%d] = %s", c, ordering[c].getValueString()));
	}*/
	ObjectWatcherVector newVector;
	for(std::vector<NetworkId>::const_iterator oi = ordering.begin(); oi != ordering.end(); oi++)
	{
		const NetworkId &nid = *oi;
		bool found = false;
		int index = -1;
		ObjectWatcherVector::iterator owvi = m_objects->begin();
		for(unsigned int i = 0; (i < m_objects->size()) && !found; )
		{
			ClientObject *obj = (*m_objects) [i].getPointer ();
			if(obj && obj->getNetworkId() == nid)
			{
				found = true;
				index = i;
			}
			else
			{
				++i;
				++owvi;
			}
		}
		if(found)
		{
			newVector.push_back((*m_objects)[index]);
			m_objects->erase(owvi);
		}
	}
	/*DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < newVector.size(); ++c)
	{
		if(newVector[c].getPointer ())
			DEBUG_WARNING(true, ("newVector[%d] = %s", c, newVector[c].getPointer ()->getNetworkId().getValueString()));
	}*/
	//put all the remaining items in
	for(unsigned int j = 0; (j < m_objects->size()); ++j)
	{
		newVector.push_back((*m_objects)[j]);
	}
	/*DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < newVector.size(); ++c)
	{
		if(newVector[c].getPointer ())
			DEBUG_WARNING(true, ("newVector[%d] = %s", c, newVector[c].getPointer ()->getNetworkId().getValueString()));
	}*/
	m_objects->clear();
	for(unsigned int k = 0; k < newVector.size(); k++)
	{
		m_objects->push_back(newVector[k]);
	}
	/*DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < m_objects->size(); ++c)
	{
		if((*m_objects) [c].getPointer ())
			DEBUG_WARNING(true, ("m_objects[%d] = %s", c, (*m_objects) [c].getPointer ()->getNetworkId().getValueString()));
	}*/
	setContentDirty(true);
}

void SwgCuiContainerProviderDefault::tryToPlaceItemBefore(const NetworkId & object, const NetworkId & before)
{
	/*unsigned int c;
	DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < m_objects->size(); ++c)
	{
		if((*m_objects) [c].getPointer ())
			DEBUG_WARNING(true, ("m_objects[%d] = %s", c, (*m_objects) [c].getPointer ()->getNetworkId().getValueString()));
	}*/
	//DEBUG_WARNING(true, ("SwgCuiContainerProviderDefault::tryToPlaceItemBefore %s %s", object.getValueString().c_str(), before.getValueString().c_str()));
	bool found = false;
	int index = -1;
	ObjectWatcherVector::iterator owvi = m_objects->begin();
	for(unsigned int i = 0; (i < m_objects->size()) && !found; )
	{
		ClientObject *obj = (*m_objects) [i].getPointer ();
		if(obj && obj->getNetworkId() == object)
		{
			found = true;
			index = i;
		}
		else
		{
			++i;
			++owvi;
		}
	}
	if(!found)
	{
		//DEBUG_WARNING(true, ("SwgCuiContainerProvider::tryToPlaceItemBefore couldn't find object %s", object.getValueString().c_str()));
		return;
	}

	ObjectWatcher ow = (*m_objects)[index];
	m_objects->erase(owvi);

	/*DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < m_objects->size(); ++c)
	{
		if((*m_objects) [c].getPointer ())
			DEBUG_WARNING(true, ("m_objects[%d] = %s", c, (*m_objects) [c].getPointer ()->getNetworkId().getValueString()));
	}*/

	bool foundB = false;
	int indexB = -1;
	ObjectWatcherVector::iterator owviB = m_objects->begin();
	for(unsigned int iB = 0; (iB < m_objects->size()) && !foundB; )
	{
		ClientObject *obj = (*m_objects) [iB].getPointer ();
		if(obj && obj->getNetworkId() == before)
		{
			foundB = true;
			indexB = iB;
		}
		else
		{
			++iB;
			++owviB;
		}
	}

	m_objects->insert(owviB, ow);

	/*DEBUG_WARNING(true, ("----------------------"));
	for(c = 0; c < m_objects->size(); ++c)
	{
		if((*m_objects) [c].getPointer ())
			DEBUG_WARNING(true, ("m_objects[%d] = %s", c, (*m_objects) [c].getPointer ()->getNetworkId().getValueString()));
	}*/

	setContentDirty(true);
}

//======================================================================
