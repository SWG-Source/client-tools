//======================================================================
//
// CuiDragManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDragManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientTangibleObjectTemplate.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/Game.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/TemplateCommandMappingManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSecureTradeManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"

namespace CuiDragManagerNamespace
{
	//----------------------------------------------------------------------

	CuiMessageBox *              s_messageBoxConfirmGiveItem;
	NetworkId s_source;
	NetworkId s_dest;

	class MyCallback : public MessageDispatch::Callback
	{
	public:

		MyCallback () :
		  MessageDispatch::Callback ()
		  {
		  }

		void onMessageBoxClosed (const CuiMessageBox & box)
		{
			if (&box == s_messageBoxConfirmGiveItem)
			{
				disconnect(s_messageBoxConfirmGiveItem->getTransceiverClosed(), *this, &MyCallback::onMessageBoxClosed);
				s_messageBoxConfirmGiveItem = 0;

				if (box.completedAffirmative ())
				{
					const Unicode::String & params = Unicode::narrowToWide (s_dest.getValueString ());
					static const uint32 hash_giveItem = Crc::normalizeAndCalculate ("giveItem");
					ClientCommandQueue::enqueueCommand(hash_giveItem, s_source, params);
				}
			}
		}
	};

	//@TODO add in install/remove to delete the callback class
	MyCallback *	 s_callback;
};

using namespace CuiDragManagerNamespace;

//======================================================================

bool CuiDragManager::handleDrop (const CuiDragInfo & info, ClientObject & dest, const std::string & slotname, bool askOnGiveItem)
{
	ClientObject * const object = info.getClientObject ();

	if (object)
		return handleDrop (*object, dest, slotname, askOnGiveItem);

	return false;
}

//----------------------------------------------------------------------

bool CuiDragManager::handleDrop (ClientObject & source, ClientObject & dest, const std::string & slotname, bool askOnGiveItem)
{
	TargetInfo tinfo;
	tinfo.slotName = slotname;

	Object * const currentContainer = ContainerInterface::getContainedByObject(source);

	// Check to see if someone is trying to drag us out of our appearance inventory.
	if(currentContainer)
	{
		SlottedContainer * const currentSlottedContainer = ContainerInterface::getSlottedContainer(*currentContainer);
		if(currentSlottedContainer)
		{
			CreatureObject * const creatureParentContainer = dynamic_cast<CreatureObject * const>(ContainerInterface::getContainedByObject(*currentContainer));
			if(creatureParentContainer)
			{
				if(creatureParentContainer->getAppearanceInventoryObject() && creatureParentContainer->getAppearanceInventoryObject() == currentContainer)
				{
					// No dragging and dropping cybernetics!
					if(&dest == creatureParentContainer->getInventoryObject() && source.getGameObjectType() != SharedObjectTemplate::GOT_cybernetic)
					{
						CuiInventoryManager::unequipAppearanceItem(source.getNetworkId());

						return true;
					}

					return false;
				}
			}
		}
	}


	if (!findTarget (source, dest, tinfo))
		return false;

	if (tinfo.creature)
	{
		if (tinfo.creature->isPlayer ())
		{
			CuiSecureTradeManager::initiateTrade (tinfo.creature->getNetworkId (), source.getNetworkId ());
			return true;
		}
	}
	else if (tinfo.volumeContainerObject)
	{	
		if(tinfo.volumeContainerObject)
		{
			if(dynamic_cast<const ClientMissionObject*>(tinfo.volumeContainerObject) != 0)
				return false;
		}
		if (ContainerInterface::transferItemToVolumeContainer (*tinfo.volumeContainerObject, source))
		{
			return true;
		}

		return false;
	}
	else if ((source.getObjectType() == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag) &&
	         (dest.getObjectType() == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag))
	{
		// Resource container combining

		ResourceContainerObject * const sourceResourceContainerObject      = safe_cast<ResourceContainerObject *>(&source);
		ResourceContainerObject * const destinationResourceContainerObject = safe_cast<ResourceContainerObject *>(&dest);

		if ((sourceResourceContainerObject != NULL) &&
		    (destinationResourceContainerObject != NULL) &&
		    (sourceResourceContainerObject->getResourceType() == destinationResourceContainerObject->getResourceType()) &&
			(destinationResourceContainerObject->getQuantity() < destinationResourceContainerObject->getMaxQuantity()))
		{
			static uint32 const hash = Crc::normalizeAndCalculate("resourceContainerTransfer");

			char text[128];
			int const amount = std::min(sourceResourceContainerObject->getQuantity(), destinationResourceContainerObject->getMaxQuantity() - destinationResourceContainerObject->getQuantity());
			snprintf(text, sizeof(text), "%s %d", dest.getNetworkId().getValueString().c_str(), amount);

			ClientCommandQueue::enqueueCommand(hash, sourceResourceContainerObject->getNetworkId(), Unicode::narrowToWide(text));

			return true;
		}
	}

	if (tinfo.powerup)
	{
		const Unicode::String & params = Unicode::narrowToWide (dest.getNetworkId ().getValueString ());
		static const uint32 hash_giveItem = Crc::normalizeAndCalculate ("applyPowerup");
		ClientCommandQueue::enqueueCommand(hash_giveItem, source.getNetworkId (), params);
		return true;
	}

	//-- ask before doing a giveItem because of our new 
	if(askOnGiveItem)
	{
		if(!s_callback)
			s_callback = new MyCallback;
		s_source = source.getNetworkId();
		s_dest = dest.getNetworkId();
		s_messageBoxConfirmGiveItem = CuiMessageBox::createYesNoBox(CuiStringIds::confirm_give_item_drag_drop.localize());
		s_callback->connect(s_messageBoxConfirmGiveItem->getTransceiverClosed(), *s_callback, &MyCallback::onMessageBoxClosed);
	}
	else
	{	
		//-- if all else fails, try a /giveItem
		const Unicode::String & params = Unicode::narrowToWide (dest.getNetworkId ().getValueString ());
		static const uint32 hash_giveItem = Crc::normalizeAndCalculate ("giveItem");
		ClientCommandQueue::enqueueCommand(hash_giveItem, source.getNetworkId (), params);
	}

	return true;
}

//----------------------------------------------------------------------

bool CuiDragManager::isDropOk   (const CuiDragInfo & info, ClientObject & dest, const std::string & slotname)
{
	switch (info.type)
	{
	case CuiDragInfoTypes::CDIT_object:
		{
			ClientObject * const object = info.getClientObject ();
			
			if (object)
			{
				TargetInfo tinfo;
				tinfo.slotName = slotname;

				Object * const currentContainer = ContainerInterface::getContainedByObject(*object);

				// Check to see if someone is trying to drag us out of our appearance inventory.
				if(currentContainer)
				{
					SlottedContainer * const currentSlottedContainer = ContainerInterface::getSlottedContainer(*currentContainer);
					if(currentSlottedContainer)
					{
						CreatureObject * const creatureParentContainer = dynamic_cast<CreatureObject * const>(ContainerInterface::getContainedByObject(*currentContainer));
						if(creatureParentContainer)
						{
							if(creatureParentContainer->getAppearanceInventoryObject() && creatureParentContainer->getAppearanceInventoryObject() == currentContainer)
							{
								if(&dest == creatureParentContainer->getInventoryObject())
									return true;

								return false;
							}
						}
					}
				}

				if (findTarget (*object, dest, tinfo))
				{
					// Check for resource container combining

					if ((object->getObjectType() == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag) &&
					    (dest.getObjectType() == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag))
					{
						ResourceContainerObject *source = safe_cast<ResourceContainerObject *>(object);
						ResourceContainerObject *destination = safe_cast<ResourceContainerObject *>(&dest);

						if ((source != NULL) && 
						    (destination != NULL) &&
						    (source->getResourceType() == destination->getResourceType()))
						{
							return (destination->getQuantity() < destination->getMaxQuantity());
						}
						return false;
					}
					else
						return true;
				}
				else
					return false;
			}
		}
		break;
	
	case CuiDragInfoTypes::CDIT_macro:
	case CuiDragInfoTypes::CDIT_command:
	case CuiDragInfoTypes::CDIT_attachmentData:
	case CuiDragInfoTypes::CDIT_none:
	case CuiDragInfoTypes::CDIT_numTypes:
	default:
		WARNING (true, ("bad type"));
		break;
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiDragManager::findTarget (ClientObject & source, ClientObject & dest, TargetInfo & tinfo, bool warning)
{
	tinfo.volumeContainerObject  = 0;
	tinfo.creature               = 0;
	tinfo.slottedContainerObject = 0;
	tinfo.powerup                = false;

	if (&source == &dest)
		return false;

	if ((tinfo.creature = dynamic_cast<CreatureObject *>(&dest)) != 0)
		return true;
	
	const int got_src = source.getGameObjectType ();
	const int got_dst = dest.getGameObjectType ();
	
	if (ContainerInterface::getVolumeContainer (dest))
	{
		if (got_dst == SharedObjectTemplate::GOT_misc_container || 
			got_dst == SharedObjectTemplate::GOT_misc_container_wearable || 
			got_dst == SharedObjectTemplate::GOT_misc_container_ship_loot || 
			got_dst == SharedObjectTemplate::GOT_misc_crafting_station ||
			got_dst == SharedObjectTemplate::GOT_misc_furniture ||
			got_dst == SharedObjectTemplate::GOT_misc_blueprint ||
			got_dst == SharedObjectTemplate::GOT_chronicles_quest_holocron ||
			got_dst == SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe )
			tinfo.volumeContainerObject = &dest;
		
		if (got_dst == SharedObjectTemplate::GOT_installation_minefield &&
			got_src == SharedObjectTemplate::GOT_weapon_heavy_mine)
			tinfo.volumeContainerObject = &dest;

		if (got_dst == SharedObjectTemplate::GOT_misc_fishing_pole &&
			got_src == SharedObjectTemplate::GOT_misc_fishing_bait)
			tinfo.volumeContainerObject = &dest;
	}
	else
	{
		if (tinfo.slotName.empty ())
		{
			const int ot_src = source.getObjectType ();
			const int ot_dst = dest.getObjectType ();

			if (ot_dst == SharedObjectTemplate::GOT_misc_crafting_station)
			{
			}
			else if ((ot_src == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag) &&
			         (ot_dst == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag))
			{
				return true;
			}
		}
		else
		{
			SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (dest);

			const SlotId & slotId = SlotIdManager::findSlotId(CrcLowerString(tinfo.slotName.c_str ()));

			if (slotId != SlotId::invalid && slotted->hasSlot (slotId))
			{
				tinfo.slottedContainerObject = &dest;
			}
		}
	}
	
	ClientObject * const containerObject = tinfo.volumeContainerObject ? tinfo.volumeContainerObject : tinfo.slottedContainerObject;

	if (containerObject)
	{
		if (containerObject == &source)
		{
			WARNING (warning, ("Attempt to drop object into self."));
		}
		else if (ContainerInterface::getContainedByObject (source) == containerObject)
		{
			WARNING (warning, ("Attempt to drop object into current container."));
		}
		else if (CuiInventoryManager::isNestedInContainer(*containerObject, source))
		{
			WARNING (warning, ("Attempt to drop object into subcontainer of self."));
		}
		else
		{
			return true;
		}

		tinfo.volumeContainerObject  = 0;
		tinfo.slottedContainerObject = 0;
		return false;
	}

	if (GameObjectTypes::isTypeOf (got_src, SharedObjectTemplate::GOT_powerup_armor) && GameObjectTypes::isTypeOf (got_dst, SharedObjectTemplate::GOT_armor))
	{
		if (GameObjectTypes::doesPowerupApply (got_src, got_dst))
		{
			tinfo.powerup = true;
			return true;
		}
	}

	if (GameObjectTypes::isTypeOf (got_src, SharedObjectTemplate::GOT_powerup_weapon) && GameObjectTypes::isTypeOf (got_dst, SharedObjectTemplate::GOT_weapon))
	{
		if (GameObjectTypes::doesPowerupApply (got_src, got_dst))
		{
			tinfo.powerup = true;
			return true;
		}
	}

	// see if we are dropping a gem into a socket
	const TangibleObject * gem = dynamic_cast<const TangibleObject *>(&source);
	if (gem != NULL)
	{
		// -TRF- converted safe_cast to dynamic_cast --- I hit this in test branch after dragging a grenade object which was not dervied from ClientTangibleObjectTemplate.
		const ClientTangibleObjectTemplate * const gemTemplate = dynamic_cast<const ClientTangibleObjectTemplate *>(gem->getObjectTemplate());
		if (gemTemplate)
		{
			const int destinations = gemTemplate->getSocketDestinationsCount();
			if (destinations > 0)
			{
				// see if the destination got matches any of the gem's gots
				for (int i = 0; i < destinations; ++i)
				{
					if (got_dst == gemTemplate->getSocketDestinations(i))
					{
						// @todo: see if the destination has a free slot
						const TangibleObject * const tangible = dest.asTangibleObject ();
						if(tangible && tangible->hasCondition(TangibleObject::C_magicItem))
						{
							ObjectAttributeManager::AttributeVector attribs;
							if(ObjectAttributeManager::hasAttributesForId(tangible->getNetworkId()))
							{
								ObjectAttributeManager::getAttributes(tangible->getNetworkId(), attribs);
								for(unsigned attribNum = 0; attribNum < attribs.size(); ++attribNum)
								{
									if(attribs[attribNum].first == "sockets")
									{
										if(Unicode::toInt(attribs[attribNum].second) > 0)
										return true;
									}
								}
							}
							else
							{
								return true; // let you try it if we've gotten this far.
							}
						}
					}
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiDragManager::handleAction (const CuiDragInfo & info)
{
	switch (info.type)
	{
	case CuiDragInfoTypes::CDIT_object:
		{
			ClientObject * const obj = info.getClientObject ();
			
			if (!obj)
			{
				WARNING (true, ("no object found for the dragging object handle action whatever."));
				return false;
			}
			
			CuiRadialMenuManager::performDefaultAction (*obj);

			//send command for object to server, if the object has a command associated with it
			uint32 templateCrc = Crc::normalizeAndCalculate(obj->getTemplateName());
			bool executeFromToolbar = TemplateCommandMappingManager::getExecuteFromToolbarForTemplateCrc(templateCrc);
			if(executeFromToolbar)
			{
				uint32 commandCrc = TemplateCommandMappingManager::getCommandCrcForTemplateCrc(templateCrc);	
				if(commandCrc > 0)
				{
					const Command & cmd = CommandTable::getCommand (commandCrc);
					if(!cmd.isNull())
					{
						Unicode::String params;
						Unicode::narrowToWide(obj->getNetworkId().getValueString(), params);
						
						NetworkId targetId;
						CreatureObject * const player = Game::getPlayerCreature();
						if (player != NULL)
							targetId = NetworkId(player->getLookAtTarget().getValue());

						ClientCommandQueue::enqueueCommand(commandCrc, targetId, params);
						//CuiMessageQueueManager::executeCommandByName (cmd.m_commandName);
					}
				}
			}
		}
		break;
	case CuiDragInfoTypes::CDIT_command:
	case CuiDragInfoTypes::CDIT_macro:
		if (!info.cmd.empty ())
			return CuiMessageQueueManager::executeCommandByName (info.cmd);
		else if (!info.str.empty ())
			return CuiMessageQueueManager::executeCommandByString (info.str, true);
		else
			WARNING (true, ("Invalid"));
		return false;

	case CuiDragInfoTypes::CDIT_attachmentData:
	case CuiDragInfoTypes::CDIT_none:
	case CuiDragInfoTypes::CDIT_numTypes:
	default:
		WARNING (true, ("bad type"));
		return false;
	}

	return true;
}

//======================================================================
