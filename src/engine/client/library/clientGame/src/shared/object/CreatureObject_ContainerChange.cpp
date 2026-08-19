// ======================================================================
//
// CreatureObject_ContainerChange.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CreatureObject.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/ClientGameAppearanceEvents.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureController.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/CrcString.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"

// ======================================================================
//lint suppressions
// ======================================================================

namespace CreatureObjectNamespace
{
	void  attachObject(SkeletalAppearance2 &containerAppearance, ClientObject &containedObject, int arrangementIndex);

	void  attachWearableToOwnerCustomizationData(SkeletalAppearance2 &containerAppearance, SkeletalAppearance2 &wearableAppearance);
	void  detachWearableFromOwnerCustomizationData(SkeletalAppearance2 &containerAppearance, SkeletalAppearance2 &wearableAppearance);

	extern bool ms_logAppearanceTabMessages;
}

// ======================================================================
// Namespace CreatureObjectNamespace
// ======================================================================

void CreatureObjectNamespace::attachObject(SkeletalAppearance2 &containerAppearance, ClientObject &containedObject, int arrangementIndex)
{
	// goal: find the first SlotId in the specified arrangement that has an appearance
	//       hardpoint associated with it.  attach the attachable appearance to that
	//       hardpoint.

	//-- get slotted containment property
	const Property *const baseProperty = containedObject.getProperty(SlottedContainmentProperty::getClassPropertyId());
	if (!baseProperty)
	{
		DEBUG_WARNING(true, ("Attachable object can't be attached because it doesn't have a slotted containment property, aborting."));
		return;
	}

	const SlottedContainmentProperty *const property = safe_cast<const SlottedContainmentProperty*>(baseProperty);
	NOT_NULL(property);

	//-- Make sure the specified arrangement index is valid.
	if ((arrangementIndex < 0) || (arrangementIndex >= property->getNumberOfArrangements()))
	{
		DEBUG_WARNING(true, ("attachObject(): tried to attach object [id=%s] to object [id=%s] with invalid arrangement number %d, ignoring.", containedObject.getNetworkId().getValueString().c_str(), containerAppearance.getOwner() ? containerAppearance.getOwner()->getNetworkId().getValueString().c_str() : "null appearance owner", arrangementIndex));
		return;
	}

	//-- get container's skeleton
	//   note: we're using the lowest LOD skeleton to check if the attachment point transform is
	//         present.  Every skeleton LOD should support all points of attachment that an appearance provides.
	const Skeleton &containerSkeleton = containerAppearance.getSkeleton(containerAppearance.getSkeletonLodCount() - 1);
	
	//-- attach the attachable to the first slot that has a hardpoint that is present on the container.
	const int slotCount = property->getNumberOfSlots(arrangementIndex);
	for (int slotIndex = 0; slotIndex < slotCount; ++slotIndex)
	{
		//-- get hardpoint name associated with slot
		const SlotId slotId = property->getSlotId(arrangementIndex, slotIndex);

		if (!SlotIdManager::isSlotAppearanceRelated(slotId))
			continue;

		CrcString const &hardpointName = SlotIdManager::getSlotHardpointName(slotId);
		if (hardpointName.isEmpty())
			continue;

		//-- check if container appearance has the hardpoint
		bool hasHardpoint   = false;
		int  transformIndex = -1;

		containerSkeleton.findTransformIndex(hardpointName, &transformIndex, &hasHardpoint);
		if (!hasHardpoint)
		{
			// this hardpoint isn't present on appearance
#ifdef _DEBUG
			Object const *const owner = containerAppearance.getOwner();

			DEBUG_WARNING(true, ("data error: can't equip appearance-related item: slot [%s] requires hardpoint [%s], missing on holder object id [%s], template [%s], appearance [%s].", 
				SlotIdManager::getSlotName(slotId).getString(), 
				hardpointName.getString(), 
				owner ? owner->getNetworkId().getValueString().c_str() : "<null owner object>",
				owner ? owner->getObjectTemplateName() : "<null owner object>",
				containerAppearance.getAppearanceTemplateName()));
#endif
			continue;
		}

		//-- we've got the hardpoint, attach to it.
		containerAppearance.attach(&containedObject, hardpointName);
		return;
	}

	//-- The arrangement had no associated hardpoints or none that were on the container slot.
	DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
		("CO_container: appearance template=[%s] could not visually attach object id=[%s] template=[%s] using arrangement (%d); container doesn't have the hardpoint.\n", 
		containerAppearance.getAppearanceTemplateName(),
		containedObject.getNetworkId().getValueString().c_str(), 
		containedObject.getObjectTemplateName(),
		arrangementIndex));
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::attachWearableToOwnerCustomizationData(SkeletalAppearance2 &containerAppearance, SkeletalAppearance2 &wearableAppearance)
{
	//-- get owner's customization data
	CustomizationData *const ownerCd = containerAppearance.fetchCustomizationData();
	if (!ownerCd)
	{
		// Owner doesn't have customization data.  Skip.
		return;
	}

	//-- get wearable's customization data
	CustomizationData *const wearableCd = wearableAppearance.fetchCustomizationData();
	if (!wearableCd)
	{
		// Wearable doesn't have customization data.  Skip.
		ownerCd->release();
		return;
	}

	//-- make a link from wearable's /shared_owner directory to owner's /shared_owner/ customization variable directory
	IGNORE_RETURN(wearableCd->mountRemoteCustomizationData(*ownerCd, "/shared_owner/", "/shared_owner"));

	ownerCd->release();
	wearableCd->release();
}

// ----------------------------------------------------------------------

void CreatureObjectNamespace::detachWearableFromOwnerCustomizationData(SkeletalAppearance2 &containerAppearance, SkeletalAppearance2 &wearableAppearance)
{
	//-- get owner's customization data
	CustomizationData *const ownerCd = containerAppearance.fetchCustomizationData();
	if (!ownerCd)
	{
		// Owner doesn't have customization data.  Skip.
		return;
	}

	//-- get wearable's customization data
	CustomizationData *const wearableCd = wearableAppearance.fetchCustomizationData();
	if (!wearableCd)
	{
		// Wearable doesn't have customization data.  Skip.
		ownerCd->release();
		return;
	}

	//-- make a link from wearable's /shared_owner directory to owner's /shared_owner/ customization variable directory
	IGNORE_RETURN(wearableCd->dismountRemoteCustomizationData("/shared_owner"));

	ownerCd->release();
	wearableCd->release();
}

// ======================================================================
// class CreatureObject: public member functions related to container change processing.
// ======================================================================

/**
 * Notify that specified object was added to this object's container.
 *
 * This implementation handles either wearing or attaching one object
 * on this container object.
 *
 * @see ClientObject::addedToContainer()
 */

void CreatureObject::handleAttachWearForContainerAdd(ClientObject &containedObject, int arrangement)
{
	// Change us back if we're currently in disguise.
	bool const usingAltAppearance = isUsingAlternateAppearance();

	if(usingAltAppearance)
		useDefaultAppearance();

	//-- Chain down to parent.
	ClientObject::addedToContainer(containedObject, arrangement);

	//-- Ignore CreatureObject-derived contained items.  We don't want to try to wear another creature!
	CreatureObject *const creatureContainedObject = containedObject.asCreatureObject();
	if (!creatureContainedObject)
	{
		//-- If this container has a skeletal appearance, handle container changes.
		Appearance *const containerBaseAppearance = getAppearance();
		SkeletalAppearance2 *const containerAppearance = (containerBaseAppearance ? containerBaseAppearance->asSkeletalAppearance2() : NULL);
		if (!containerAppearance)
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
				("CO_container: container id=[%s] template=[%s] ignoring addition of object id=[%s] template=[%s], container not skeletal appearance.\n", 
				getNetworkId().getValueString().c_str(),
				getObjectTemplateName(),
				containedObject.getNetworkId().getValueString().c_str(), 
				containedObject.getObjectTemplateName()));
		}
		else
		{
			//-- Get the appearance type of the item going into the container.
			Appearance *const containedBaseAppearance = containedObject.getAppearance();
			SkeletalAppearance2 *const containedAppearance = (containedBaseAppearance ? containedBaseAppearance->asSkeletalAppearance2() : NULL);
			if (containedAppearance)
			{
				// Container is skeletal, contained is skeletal, do a wear operation.
				DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
					("CO_container: container id=[%s] template=[%s] wearing object id=[%s] template=[%s].\n", 
					getNetworkId().getValueString().c_str(),
					getObjectTemplateName(),
					containedObject.getNetworkId().getValueString().c_str(), 
					containedObject.getObjectTemplateName()));
				
				CreatureObjectNamespace::attachWearableToOwnerCustomizationData(*containerAppearance, *containedAppearance);
				PlayerObject* player = NULL;
				if (containerAppearance->getOwner() && containerAppearance->getOwner()->asClientObject() && containerAppearance->getOwner()->asClientObject()->asCreatureObject())
					player = containerAppearance->getOwner()->asClientObject()->asCreatureObject()->getPlayerObject();
				if(player)
				{
					if(!player->getShowBackpack())
					{
						if(PlayerObject::isObjectABackpack(containedBaseAppearance))
								containedObject.setShouldBakeIntoMesh(false);
					}

					if(!player->getShowHelmet())
					{
						if(PlayerObject::isObjectAHelmet(containedBaseAppearance))
							containedObject.setShouldBakeIntoMesh(false);
					}
				}

				ClientObject* appearaceInv = getAppearanceInventoryObject();
				if(appearaceInv)
				{
					SlottedContainmentProperty * slottedContainment = ContainerInterface::getSlottedContainmentProperty(containedObject);
					bool objectInSlotAlready = false;
					if(slottedContainment)
					{
						SlottedContainmentProperty::SlotArrangement const slots = slottedContainment->getSlotArrangement(arrangement);
						for(unsigned int i = 0; i < slots.size(); ++i)
						{
							DEBUG_WARNING(CreatureObjectNamespace::ms_logAppearanceTabMessages, ("HandleContainerWear: Wearing item [%s], checking slot [%s] on appearance for already worn items.", containedObject.getObjectTemplateName(),SlotIdManager::getSlotName(slots[i]).getString()));
							ClientObject* appearInvObj = ContainerInterface::getObjectInSlot(*appearaceInv, SlotIdManager::getSlotName(slots[i]).getString());
							if(appearInvObj && containedAppearance->isWearing(appearInvObj))
								objectInSlotAlready = true;
						}
					}

					if(!objectInSlotAlready)
						containerAppearance->wear(&containedObject);
				}
				else
					containerAppearance->wear(&containedObject);

				// Update our duped creatures
				this->setDupedCreaturesDirty(true);
			}
			else
			{
				// Container is skeletal, contained is not skeletal.  Lookup the hardpoint 
				// for the specified arrangement and attach contained object to the hardpoint.
				DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
					("CO_container: container id=[%s] template=[%s] attaching object id=[%s] template=[%s].\n", 
					getNetworkId().getValueString().c_str(),
					getObjectTemplateName(),
					containedObject.getNetworkId().getValueString().c_str(), 
					containedObject.getObjectTemplateName()));

				CreatureObjectNamespace::attachObject(*containerAppearance, containedObject, arrangement);
			}

			//-- Send OnEquipped event to the appearance.
			if (containedBaseAppearance)
			{
				containedBaseAppearance->onEvent(ClientGameAppearanceEvents::getOnEquippedEventId());

				// If controller thinks we're in combat, send an OnEquippedEnteredCombat message as well.
				CreatureController const *const controller = dynamic_cast<CreatureController*>(getController());
				if (controller && controller->areVisualsInCombat())
					containedBaseAppearance->onEvent(ClientGameAppearanceEvents::getOnEquippedEnteredCombatEventId());
			}
		}
	}
	else
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
			("CO_container: container id=[%s] template=[%s] ignoring addition of object id=[%s] template=[%s], object to add is a creature.\n", 
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			containedObject.getNetworkId().getValueString().c_str(), 
			containedObject.getObjectTemplateName()));
	}

	//-- Handle post processing.  Derived classes may override 
	//   this method with class-specific handling.
	doPostContainerChangeProcessing();

	if(usingAltAppearance)
		useAlternateAppearance();
}

// ----------------------------------------------------------------------
/**
 * x64 catch-up fix: re-attach worn clothing whose appearance was not yet
 * loaded when it was first slotted.
 *
 * The normal attach path is handleAttachWearForContainerAdd() (called from
 * addedToContainer()). But the player's worn items arrive over the network
 * and can be slotted into chest1/pants1/shoes/etc. BEFORE their own
 * appearance has finished loading. At that point handleAttachWearForContainerAdd()
 * sees a null / non-skeletal contained appearance, falls through to the
 * hardpoint-attach branch, and the clothing is never worn - and there was no
 * safety net for *regular* worn clothing (only the appearance-inventory items
 * had verifyWornAppearanceItems(), which is itself gated behind
 * getAppearanceInventoryObject()). Result: the player renders naked even
 * though the clothing IS correctly equipped server-side.
 *
 * This pass is driven periodically from CreatureObject::alter (the
 * m_verifyAppearanceTimer / m_initAppearanceWearables block). It walks the
 * creature's own worn slots and wears any skeletal-appearance item that
 * should be worn but isn't yet. It is idempotent (isWearing() guard) and
 * naturally ignores non-skinned slot contents (weapons, the ghost/bank
 * objects, etc. have no SkeletalAppearance2).
 */

void CreatureObject::verifyWornItems()
{
	Appearance *const baseAppearance = getAppearance();
	SkeletalAppearance2 *const skeleAppearance = baseAppearance ? baseAppearance->asSkeletalAppearance2() : 0;
	if (!skeleAppearance)
		return;

	if (isUsingAlternateAppearance())
		return;

	SlottedContainer *const slotted = ContainerInterface::getSlottedContainer(*this);
	if (!slotted)
		return;

	std::vector<SlotId> slotList;
	slotted->getSlotIdList(slotList);

	for (std::vector<SlotId>::const_iterator it = slotList.begin(); it != slotList.end(); ++it)
	{
		Container::ContainerErrorCode err = Container::CEC_Success;
		Object *const obj = slotted->getObjectInSlot(*it, err).getObject();
		if (!obj)
			continue;

		ClientObject *const clientObj = obj->asClientObject();
		if (!clientObj)
			continue;

		//-- never try to "wear" another creature (mounts/riders/vehicles)
		if (clientObj->asCreatureObject())
			continue;

		//-- only skinned (MGN) wearables can be worn on the body; this also
		//   naturally skips weapons / the ghost+bank objects, and skips items
		//   whose appearance simply hasn't loaded yet (retried next tick)
		Appearance *const itemBaseAppearance = clientObj->getAppearance();
		SkeletalAppearance2 *const itemAppearance = itemBaseAppearance ? itemBaseAppearance->asSkeletalAppearance2() : 0;
		bool const alreadyWearing = skeleAppearance->isWearing(clientObj);
		if (!itemAppearance)
			continue; // appearance not (yet) a skinned mesh - retry next tick

		if (alreadyWearing)
			continue;

		//-- it's in a worn slot, it's skinned, and it isn't being worn yet:
		//   attach it now, mirroring the core wear path of
		//   handleAttachWearForContainerAdd()
		CreatureObjectNamespace::attachWearableToOwnerCustomizationData(*skeleAppearance, *itemAppearance);
		skeleAppearance->wear(clientObj);
		setDupedCreaturesDirty(true);
	}
}

// ----------------------------------------------------------------------
/**
 * Notify that specified object was removed from this object's container.
 *
 * @see ClientObject::addedToContainer()
 */

void CreatureObject::handleDetachUnwearForContainerRemove(ClientObject &containedObject)
{
	// Change us back if we're currently in disguise.
	bool const usingAltAppearance = isUsingAlternateAppearance();
	
	if(usingAltAppearance)
		useDefaultAppearance();
	
	//-- If this container has a skeletal appearance, handle container changes.
	Appearance *const containerBaseAppearance = getAppearance();

	//-- Ignore CreatureObject-derived contained items.  We don't want to try to unwear another creature!
	CreatureObject *const creatureContainedObject = containedObject.asCreatureObject();
	
	//Reset our Bake into Mesh property.
	containedObject.setShouldBakeIntoMesh(true);
	
	if (!creatureContainedObject)
	{
		SkeletalAppearance2 *const containerAppearance = (containerBaseAppearance ? containerBaseAppearance->asSkeletalAppearance2() : NULL);
		if (!containerAppearance)
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
				("CO_container: container id=[%s] template=[%s] ignoring removal of object id=[%s] template=[%s], container not skeletal appearance.\n", 
				getNetworkId().getValueString().c_str(),
				getObjectTemplateName(),
				containedObject.getNetworkId().getValueString().c_str(), 
				containedObject.getObjectTemplateName()));
		}
		else
		{
			//-- Get the appearance type of the item being removed from the container.
			Appearance *const containedBaseAppearance = containedObject.getAppearance();
			SkeletalAppearance2 *const containedAppearance = (containedBaseAppearance ? containedBaseAppearance->asSkeletalAppearance2() : NULL);
			if (containedAppearance)
			{
				// Container is skeletal, contained is skeletal, do an stop wearing operation.
				DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
					("CO_container: container id=[%s] template=[%s] will stop wearing object id=[%s] template=[%s].\n", 
					getNetworkId().getValueString().c_str(),
					getObjectTemplateName(),
					containedObject.getNetworkId().getValueString().c_str(), 
					containedObject.getObjectTemplateName()));
				
				containerAppearance->stopWearing(&containedObject);
				CreatureObjectNamespace::detachWearableFromOwnerCustomizationData(*containerAppearance, *containedAppearance);
			}
			else
			{
				// Container is skeletal, contained is not skeletal.  Detach the object
				// from the container.
				DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
					("CO_container: container id=[%s] template=[%s] detaching object id=[%s] template=[%s].\n", 
					getNetworkId().getValueString().c_str(),
					getObjectTemplateName(),
					containedObject.getNetworkId().getValueString().c_str(), 
					containedObject.getObjectTemplateName()));

				containerAppearance->detach(&containedObject);
			}

			if (containedBaseAppearance)
				containedBaseAppearance->onEvent(ClientGameAppearanceEvents::getOnUnequippedEventId());

			// Update our duped creatures.
			this->setDupedCreaturesDirty(true);
		}
	}

	ClientObject::removedFromContainer(containedObject);
	DEBUG_REPORT_LOG(ConfigClientGame::getLogContainerProcessing (), 
		("CO_container: container id=[%s] template=[%s] removed object id=[%s] template=[%s].\n", 
		getNetworkId().getValueString().c_str(),
		getObjectTemplateName(),
		containedObject.getNetworkId().getValueString().c_str(), 
		containedObject.getObjectTemplateName()));

	//-- Handle post processing.  Derived classes may override 
	//   this method with class-specific handling.
	doPostContainerChangeProcessing();

	if(usingAltAppearance)
		useAlternateAppearance();
}

// ======================================================================
