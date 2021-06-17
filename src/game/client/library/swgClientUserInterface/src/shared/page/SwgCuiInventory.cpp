// ======================================================================
//
// SwgCuiInventory.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventory.h"

#include "UIButton.h"
#include "UIData.h"
#include "UILoader.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIScriptEngine.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessagebox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryEquipment.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"

#include <list>

//-----------------------------------------------------------------

namespace SwgCuiInventoryNamespace
{
	typedef std::list <SwgCuiInventory *> InventoryList;
	InventoryList s_inventoryList;

	bool  s_rangeChecked = false;
	float s_containerRange = 0.0f;

	void checkRange ()
	{
		if (s_rangeChecked)
			return;

		s_rangeChecked = true;

		if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::ITEM_OPEN, s_containerRange))
			s_containerRange *= 1.2f;
		else
			WARNING (true, ("SwgCuiInventory RadialMenuManager::getRangeForMenuType failed"));
	}

	const std::string cms_newbieTutorialRequestCloseInventory      ("closeInventory");

	const std::string cms_examineSettingName ("ShowExamineWindow");
	const std::string cms_paperdollSettingName ("ShowPaperDoll");
	const std::string cms_mediatorBaseName ("SwgCuiInventory");

	std::vector<std::string> cms_cyberneticWearableSlots;

	void saveSortSettings(SwgCuiInventoryContainer const * container,
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	void loadSortSettings(SwgCuiInventoryContainer * container,
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	namespace Settings
	{
		const std::string inventoryContainerSortColumn    = "inventoryContainerSortColumn";
		const std::string inventoryContainerSortSelection = "inventoryContainerSortSelection";
	}

	//filter used to hide equipped cybernetic pieces
	class ProviderFilter : public SwgCuiContainerProviderFilter
	{
	public:
		virtual bool showObject (const ClientObject & obj) const;
	};

}

//----------------------------------------------------------------------

bool SwgCuiInventoryNamespace::ProviderFilter::showObject (const ClientObject & obj) const
{
	//show all non-cybernetics
	if(!GameObjectTypes::isTypeOf (obj.getGameObjectType(), SharedObjectTemplate::GOT_cybernetic))
		return true;

	//don't show equipped cybernetics
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return false;
	for(std::vector<std::string>::const_iterator i = cms_cyberneticWearableSlots.begin(); i != cms_cyberneticWearableSlots.end(); ++i)
	{
		ClientObject const * const equippedObject = player->getEquippedObject(i->c_str());
		if(equippedObject && (equippedObject == &obj))
			return false;
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiInventoryNamespace::saveSortSettings(SwgCuiInventoryContainer const * const container,
														std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	int column = 0;
	UITableModel::SortDirection direction;

	if (container != 0)
	{
		container->getCurrentSortOnColumnState(column, direction);
		CuiSettings::saveInteger(mediatorName, columnName, column);
		CuiSettings::saveInteger(mediatorName, sortDirectionName, direction);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryNamespace::loadSortSettings(SwgCuiInventoryContainer * const container,
														std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	// 08/14/06 ARH - this option is being temporarily removed until we decide what to do with it
	UNREF(container);
	UNREF(mediatorName);
	UNREF(columnName);
	UNREF(sortDirectionName);
	/*if ((container != 0) && (CuiPreferences::getAutoSortInventoryContents()))
	{
		int column = container->getDefaultSortColumn();
		int direction = 0;

		CuiSettings::loadInteger(mediatorName, columnName, column);
		CuiSettings::loadInteger(mediatorName, sortDirectionName, direction);

		container->setCurrentSortOnColumnState(column, static_cast<UITableModel::SortDirection>(direction));
	}*/
}


using namespace SwgCuiInventoryNamespace;

// ======================================================================

SwgCuiInventory::SwgCuiInventory (UIPage & page, ClientObject * container, const std::string & slotName, bool usePaperDoll, bool dontClose) :
CuiMediator                     (cms_mediatorBaseName.c_str(), page),
UIEventCallback                 (),
m_eqMediator                    (0),
m_containerMediator             (0),
m_togglePaperdollButton         (0),
m_toggleExamineButton           (0),
m_dontClose                     (dontClose),
m_containerProvider             (new SwgCuiContainerProviderDefault),
m_providerFilter                (new ProviderFilter),
m_paperdollVisible              (false),
m_examineVisible                (true),
m_timeSinceLastRangeCheck       (0.0f),
m_sendClose                     (false),
m_alreadyNotifiedUnsetContainer (false),
m_info                          (0),
m_callback                      (new MessageDispatch::Callback),
m_inventoryType                 (IT_NORMAL),
m_onlyStoreBaseMediatorSettings (false)
{
	//Get type

	if (!usePaperDoll)
	{
		removeState (MS_popupHelpOk);
	}

	checkRange ();

	const bool isPlayer = !container || CuiInventoryManager::isNestedInventory (*container);

	UIPage * containerPage = 0;
	getCodeDataObject (TUIPage, containerPage, "ContainerPage");
	m_containerMediator = new SwgCuiInventoryContainer (*containerPage);
	m_containerMediator->fetch ();
	m_containerMediator->setContainerProvider (m_containerProvider);
	m_containerProvider->setFilter(m_providerFilter);

	UIPage * eqPage = 0;
	getCodeDataObject (TUIPage,   eqPage,          "EqPage");
	getCodeDataObject (TUIButton, m_togglePaperdollButton, "TogglePaperdollButton");
	getCodeDataObject (TUIButton, m_toggleExamineButton,   "ToggleExamineButton");

	registerMediatorObject (*m_togglePaperdollButton, true);
	registerMediatorObject (*m_toggleExamineButton, true);

	m_togglePaperdollButton->RemoveProperty (UIButton::PropertyName::OnPress);
	m_toggleExamineButton->RemoveProperty   (UIButton::PropertyName::OnPress);

	containerPage->SetVisible (false);
	eqPage->SetVisible        (false);

	if (!container || (usePaperDoll && isPlayer))
	{
		m_eqMediator  = new SwgCuiInventoryEquipment (*eqPage);
		m_eqMediator->fetch ();
		ClientObject * const player = Game::getClientPlayer ();
		m_eqMediator->setupCharacterViewer (player);
		m_eqMediator->deactivate ();
		m_paperdollVisible = false;
		m_dontClose = true;
	}
	else
	{
		m_dontClose = false;
		//-- decrease the minimum size of the page
		UIPoint minSize;
		IGNORE_RETURN (getPage ().GetPropertyPoint (UILowerString ("MinimumSizeItem"), minSize));
		getPage ().SetMinimumSize (minSize);

		eqPage->SetVisible (false);
		m_togglePaperdollButton->SetVisible (false);
		setContainerObject (container, slotName);
	}

	UIPage * infoPage = 0;
	getCodeDataObject (TUIPage, infoPage, "info");
	m_info      = new SwgCuiInventoryInfo (*infoPage);
	m_info->fetch ();
	m_info->setInventoryType(m_inventoryType);
	m_info->setDropThroughTarget(m_containerMediator);
				
	setSettingsAutoSizeLocation (true, true);
	setState (MS_closeable);

	s_inventoryList.push_back (this);

	m_info->deactivate();
	setIsUpdating       (true);

	cms_cyberneticWearableSlots.clear();
	cms_cyberneticWearableSlots.push_back("bracer_lower_l");
	cms_cyberneticWearableSlots.push_back("bracer_lower_r");
	cms_cyberneticWearableSlots.push_back("pants1");
	cms_cyberneticWearableSlots.push_back("chest2");
	cms_cyberneticWearableSlots.push_back("cybernetic_hand_l");
	cms_cyberneticWearableSlots.push_back("cybernetic_hand_r");


}

//-----------------------------------------------------------------

SwgCuiInventory::~SwgCuiInventory ()
{
	if (m_eqMediator)
	{
		m_eqMediator->release ();
		m_eqMediator = 0;
	}

	m_containerMediator->setContainerProvider (0);

	m_containerMediator->release ();
	m_containerMediator = 0;
	m_info->setDropThroughTarget(0);

	delete m_containerProvider;
	m_containerProvider = 0;

	delete m_providerFilter;
	m_providerFilter = 0;

	m_info->release ();
	m_info = 0;

	s_inventoryList.remove (this);

	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void SwgCuiInventory::performActivate ()
{
	m_alreadyNotifiedUnsetContainer = false;

	m_containerMediator->activate ();
	if(m_examineVisible)
	{
		m_info->activate ();
		m_info->setInfoObject(m_containerMediator->getLastSelection());
	}
	CuiManager::requestPointer (true);

	if (m_eqMediator && m_togglePaperdollButton)
	{
		ClientObject * const player = Game::getClientPlayer ();

		if (m_paperdollVisible)
		{
			m_eqMediator->activate ();
			m_eqMediator->setupCharacterViewer        (player);
		}

		if (player)
		{
			//Only setup a fake inventory in single player mode
			if (Game::getSinglePlayer())
				setupPlayerInventory (player);

			//-----------------------------------------------------------------
			//-- find the inventory container if it exists and set it into the container ui
			ClientObject * const inventory = CuiInventoryManager::getPlayerInventory ();
			setContainerObject (inventory, std::string ());
		}
	}

	UIPage * const cp = safe_cast<UIPage *>(m_containerMediator->getPage ().GetParentWidget ());
	if (cp)
		cp->Pack ();

	m_callback->connect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventory::onSelectionChanged);

	removeState(MS_settingsLoaded);
}

//-----------------------------------------------------------------

void SwgCuiInventory::performDeactivate ()
{
	m_containerMediator->deactivate ();
	m_info->deactivate ();

	if (m_eqMediator && m_togglePaperdollButton)
	{
		m_paperdollVisible = m_eqMediator->isActive ();
		m_eqMediator->deactivate ();
	}

	CuiManager::requestPointer (false);

	if(m_sendClose)
	{
		const NewbieTutorialResponse response (cms_newbieTutorialRequestCloseInventory);
		GameNetwork::send (response, true);
	}

	applyStates ();

	m_callback->disconnect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventory::onSelectionChanged);
}

//----------------------------------------------------------------------

void SwgCuiInventory::applyStates () const
{
	m_containerProvider->applyStates ();
}

//----------------------------------------------------------------------

void SwgCuiInventory::setSendClose(bool sendClose)
{
	m_sendClose = sendClose;
}

//----------------------------------------------------------------------

void SwgCuiInventory::OnButtonPressed( UIWidget *context )
{
	if (context == m_togglePaperdollButton)
	{
		if (m_eqMediator)
		{
			if (m_eqMediator->isActive ())
				m_eqMediator->deactivate ();
			else
			{
				m_eqMediator->activate ();
				m_eqMediator->setupCharacterViewer (Game::getClientPlayer ());
			}

			m_paperdollVisible = m_eqMediator->isActive ();
		}
	}
	else if (context == m_toggleExamineButton)
	{
		if (m_info)
		{
			if (m_info->isActive ())
			{
				m_info->deactivate ();
				m_examineVisible = false;
			}
			else
			{
				m_info->activate ();
				m_info->setInfoObject(m_containerMediator->getLastSelection());
				m_examineVisible = true;
			}
		}
	}
}

//-----------------------------------------------------------------

bool SwgCuiInventory::close ()
{
	if (m_dontClose)
	{
		deactivate ();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void SwgCuiInventory::setContainerObject (ClientObject * containerObject, const std::string & slotname)
{
	//if this is the player's bank, find a nearby bank terminal to associate with this opening (so when they run away, we close it)
	CreatureObject* playerCreature = Game::getPlayerCreature();
	if(playerCreature)
	{
		const ClientObject* const bank = playerCreature->getBankObject();
		if(containerObject && (containerObject == bank))
		{
			bool foundBankObject = false;
			std::vector<Object*> closeByObjects;
			ClientWorld::findObjectsInRange(playerCreature->getPosition_w(), 16.0f, closeByObjects);
			for(std::vector<Object*>::iterator i = closeByObjects.begin(); i != closeByObjects.end(); ++i)
			{
				const Object* const o = *i;
				if(o)
				{
					const SharedObjectTemplate * const oTemplate = dynamic_cast<const SharedObjectTemplate* const>(o->getObjectTemplate());
					if(oTemplate)
					{
						SharedObjectTemplate::GameObjectType objectGOTType = oTemplate->getGameObjectType();
						if(objectGOTType == SharedObjectTemplate::GOT_terminal_bank)
						{
							setAssociatedObjectId(o->getNetworkId());
							setMaxRangeFromObject(32.0f);
							foundBankObject = true;
						}
					}
				}
			}
			//we couldn't find an object to associate with the bank container, so don't let them open it
			if(!foundBankObject)
			{
				DEBUG_WARNING(true, ("Could not find a bank terminal to associate with the bankOpen request, aborting..."));
				return;
			}
		}
	}

	if (containerObject &&
		!CuiInventoryManager::isNestedInventory (*containerObject) &&
		!CuiInventoryManager::isPlayerInventory (*containerObject) &&
		!CuiInventoryManager::isPlayerAppearanceInventory(*containerObject) &&
		m_eqMediator)
		return;


	InventoryType resultType = IT_NORMAL;
	if (containerObject)
	{
		ClientObject * const containedBy = const_cast<ClientObject *>(safe_cast<const ClientObject *>(ContainerInterface::getContainedByObject (*containerObject)));

		if (containerObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public)
		{
			resultType = IT_PUBLIC;
		}
		else if (containedBy)
		{
			CreatureObject * const creature = containedBy->asCreatureObject ();

			CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getPlayer());

			if (creature)
			{
				if(creature->isDead())
				{
					// make sure we don't try to loot ourself
					if(!player || creature != player)
					{
						resultType = IT_LOOT;
					}
				}
			}

			else if(GameObjectTypes::isTypeOf (containedBy->getGameObjectType(), SharedObjectTemplate::GOT_weapon))
			{
				resultType = IT_LIGHTSABER;
			}
			
		}
	}

	m_eqMediator->setInventoryType(resultType);
	m_containerMediator->setInventoryType(resultType);
	m_inventoryType = resultType;
	m_containerMediator->setContainerObject (containerObject, slotname);

	switch(resultType)	
	{
	case IT_PUBLIC:
		{		
			getPage().SetSize(UISize(688,519));
			Unicode::String value;
			getPage().GetProperty(UILowerString("OnSetPublic"), value);
			UIManager::gUIManager ().ExecuteScript(value, &getPage());

			if (containerObject)
			{
				std::string debugName = cms_mediatorBaseName + "_";
				debugName += containerObject->getNetworkId ().getValueString () + "_" + slotname;
				setMediatorDebugName (debugName);
			}
		}
		break;
	case IT_NORMAL:
		{		
			getPage().SetSize(UISize(688,519));
			Unicode::String value;
			getPage().GetProperty(UILowerString("OnSetNormal"), value);
			UIManager::gUIManager ().ExecuteScript(value, &getPage());

			if (containerObject)
			{
				std::string debugName = cms_mediatorBaseName + "_";
				debugName += containerObject->getNetworkId ().getValueString () + "_" + slotname;
				setMediatorDebugName (debugName);
			}
		}
		break;
	case IT_LOOT:
		{		
			getPage().SetLocation(Graphics::getFrameBufferMaxWidth() - 410, Graphics::getFrameBufferMaxHeight() / 2L - 130);			
			if(m_eqMediator)
				m_eqMediator->deactivate();
			getPage().SetSize(UISize(400, 245));
			Unicode::String value;
			getPage().GetProperty(UILowerString("OnSetLoot"), value);
			UIManager::gUIManager ().ExecuteScript(value, &getPage());

			setMediatorDebugName(cms_mediatorBaseName + "_loot");
			m_onlyStoreBaseMediatorSettings = true;
		}
		break;
	case IT_LIGHTSABER:	
		{		
			if(m_eqMediator)
				m_eqMediator->deactivate();
			getPage().SetSize(UISize(688,265));
			Unicode::String value;
			getPage().GetProperty(UILowerString("OnSetLightsaber"), value);
			UIManager::gUIManager ().ExecuteScript(value, &getPage());	

			setMediatorDebugName(cms_mediatorBaseName + "_lightsaber");
			m_onlyStoreBaseMediatorSettings = true;
		}
		break;
	}
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventory::getContainerObject            () const
{
	if (m_containerMediator)
		return m_containerMediator->getContainerObject ();

	return 0;
}

//-----------------------------------------------------------------

SwgCuiInventory * SwgCuiInventory::findSelectedInventoryPage ()
{
	SwgCuiInventory * selected = 0;

	for (InventoryList::iterator it = s_inventoryList.begin (); it != s_inventoryList.end (); ++it)
	{
		SwgCuiInventory * const inventory = *it;

		if (inventory->isActive () && inventory->getPage ().IsSelected ())
		{
			if (selected)
			{
				WARNING (true, ("d"));
			}
			selected = inventory;
		}
	}
	if(!selected)
	{
		if(!s_inventoryList.empty())
		{
			SwgCuiInventory *first = *(s_inventoryList.begin());
			UIWidget *parent = first->getPage().GetParentWidget();
			UIBaseObject::UIObjectList childList;
			if(parent)
				parent->GetChildren(childList);
			for(UIBaseObject::UIObjectList::iterator it2 = childList.begin(); !selected && (it2 != childList.end()); ++it2)
			{
				UIBaseObject *child = *it2;
				for (InventoryList::iterator it3 = s_inventoryList.begin (); !selected && (it3 != s_inventoryList.end()); ++it3)
				{
					if(&(*it3)->getPage() == child)
						selected = *it3;
				}
			}
		}
	}

	return selected;
}

//----------------------------------------------------------------------

SwgCuiInventory * SwgCuiInventory::findInventoryPageByContainer (const NetworkId & containerId, const std::string & slotName)
{
	for (InventoryList::iterator it = s_inventoryList.begin (); it != s_inventoryList.end (); ++it)
	{
		SwgCuiInventory * const inventory = *it;

		if (inventory->m_containerMediator)
		{
			ClientObject * const containerObject = inventory->m_containerMediator->getContainerObject ();

			if (containerObject)
			{
				if (containerObject->getNetworkId () == containerId && inventory->m_containerMediator->getContainerSlotName () == slotName)
					return inventory;
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiInventory::findInventoryPagesByContainer (const NetworkId & containerId, const std::string & slotName, InventoryVector & iv)
{
	for (InventoryList::iterator it = s_inventoryList.begin (); it != s_inventoryList.end (); ++it)
	{
		SwgCuiInventory * const inventory = *it;

		if (inventory->m_containerMediator)
		{
			ClientObject * const containerObject = inventory->m_containerMediator->getContainerObject ();

			if (containerObject)
			{
				if (containerObject->getNetworkId () == containerId && inventory->m_containerMediator->getContainerSlotName () == slotName)
					iv.push_back (inventory);
			}
		}
	}
}

//----------------------------------------------------------------------

SwgCuiInventory * SwgCuiInventory::createInto (UIPage & parent, ClientObject * container, const std::string & slotName, bool usePaperDoll, bool dontClose)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.inventory"));
	dupe->Center ();
	return new SwgCuiInventory (*dupe, container, slotName, usePaperDoll, dontClose);
}


//----------------------------------------------------------------------

void SwgCuiInventory::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (!m_containerMediator)
		return;

	if (isCloseNextFrame ())
		return;

	const ClientObject * const containerObject = m_containerMediator->getContainerObject ();

	if (containerObject == 0)
	{
		if (!m_alreadyNotifiedUnsetContainer)
		{
			const Unicode::String & containerLocalizedName = m_containerMediator->getContainerLocalizedName ();
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIds::inv_object_destroyed_prose, Unicode::emptyString, containerLocalizedName, Unicode::emptyString, result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
			m_alreadyNotifiedUnsetContainer = true;
			closeNextFrame ();
			setContainerObject (0, std::string ());
		}
		return;
	}

	m_timeSinceLastRangeCheck += deltaTimeSecs;

	if (m_timeSinceLastRangeCheck > 3.3f && !Game::getPlayerObject()->isAdmin())
	{
		m_timeSinceLastRangeCheck = 0.0f;

		const Object * const player          = Game::getPlayer ();
		const Object * const containerParent = ContainerInterface::getFirstParentInWorld (*containerObject);
		if (!containerParent)
			return;

		const Vector & objPos_w     = containerParent->findPosition_w ();
		const Vector & playerPos_w  = player->findPosition_w ();

		if ((objPos_w.magnitudeBetween (playerPos_w) - containerParent->getAppearanceSphereRadius ()) > s_containerRange)
		{
			if (!m_alreadyNotifiedUnsetContainer)
			{
				const Unicode::String & containerLocalizedName = m_containerMediator->getContainerLocalizedName ();
				Unicode::String result;
				CuiStringVariablesManager::process (CuiStringIds::inv_out_of_range_prose, Unicode::emptyString, containerLocalizedName, Unicode::emptyString, result);
				CuiSystemMessageManager::sendFakeSystemMessage (result);
			}

			m_alreadyNotifiedUnsetContainer = true;

			closeNextFrame ();
			setContainerObject (0, std::string ());
			return;
		}
	}

	//Check for container being empty - close then
	Container const * container = ContainerInterface::getContainer(*containerObject);
	if (!container)
		return;
	if((m_inventoryType == IT_LOOT) && (container->getNumberOfItems() == 0))
		closeNextFrame();					
}

// @todo this function is a hack to make inventory work in single player

//----------------------------------------------------------------------

void SwgCuiInventory::setupPlayerInventory (ClientObject * player)
{
	if (!player || !Game::getSinglePlayer())
		return;

	SlottedContainer *const equipmentContainer = player->getSlottedContainerProperty();

	const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("inventory"));

	DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));

	const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
	AsynchronousLoader::disable ();
	Container::ContainerErrorCode tmp = Container::CEC_Success;

	if (equipmentContainer->isSlotEmpty (inventorySlot, tmp))
	{
		ClientObject * inventoryObject = dynamic_cast<ClientObject*>(ObjectTemplate::createObject("object/tangible/inventory/shared_character_inventory.iff"));
		DEBUG_FATAL(!inventoryObject, ("Could not create an inventory for the creature"));

		// fake the endBaselines() call
		ClientObject *clientObject = safe_cast<ClientObject*> (inventoryObject);
		clientObject->endBaselines ();

		if (!equipmentContainer)
		{
			DEBUG_FATAL(true, ("player does not have an slotted container for equipment.\n"));
		}

		int validArrangementIndex = -1;
		if (!equipmentContainer->getFirstUnoccupiedArrangement(*inventoryObject, validArrangementIndex, tmp) || (validArrangementIndex < 0))
		{
			DEBUG_FATAL(true,("Must be able to create the inventory"));
		}

		if (!ContainerInterface::transferItemToSlottedContainer(*player, *inventoryObject, validArrangementIndex))
		{
			DEBUG_FATAL(true,("Must be able to install the inventory"));
		}

		//-----------------------------------------------------------------

		const char * const hackObjectNames [] =
		{
			"object/weapon/ranged/pistol/shared_pistol_scout_blaster.iff",
//				"object/weapon/ranged/rifle/shared_e3_rifle_e11.iff",

			"object/tangible/container/drum/shared_tatt_drum_1.iff",
			"object/tangible/wearables/backpack/shared_backpack_s01.iff",

			// Below wearables updated by Matt Bogue on 8-5-2002

//				"object/tangible/wearables/shirt/shared_shirt_s05.iff",
			//"object/tangible/wearables/shirt/shared_shirt_s07.iff",
			"object/tangible/wearables/vest/shared_vest_s01.iff",
			//"object/tangible/wearables/vest/shared_vest_s04.iff",
			//"object/tangible/wearables/vest/shared_vest_s09.iff",
			"object/tangible/wearables/jacket/shared_jacket_s10.iff",
			//"object/tangible/wearables/jacket/shared_jacket_s11.iff",
			//"object/tangible/wearables/jacket/shared_jacket_s15.iff",
			//"object/tangible/wearables/jacket/shared_jacket_s26.iff",
			"object/tangible/wearables/pants/shared_pants_s01.iff",
			//"object/tangible/wearables/pants/shared_pants_s04.iff",
			//"object/tangible/wearables/pants/shared_pants_s05.iff",
			//"object/tangible/wearables/pants/shared_pants_s09.iff",
			//"object/tangible/wearables/pants/shared_pants_s10.iff",
			//"object/tangible/wearables/pants/shared_pants_s13.iff",
			//"object/tangible/wearables/pants/shared_pants_s21.iff",
			//"object/tangible/wearables/pants/shared_pants_s28.iff",
//				"object/tangible/wearables/skirt/shared_skirt_s07.iff",
//				"object/tangible/wearables/dress/shared_dress_s07.iff",
			//"object/tangible/wearables/dress/shared_dress_s10.iff",
			//"object/tangible/wearables/dress/shared_dress_s11.iff",
			//"object/tangible/wearables/dress/shared_dress_s15.iff",
			//"object/tangible/wearables/dress/shared_dress_s18.iff",
			//"object/tangible/wearables/dress/shared_dress_s19.iff",
			//"object/tangible/wearables/robe/shared_robe_inquisitor.iff",
			"object/tangible/wearables/shoes/shared_shoes_s01.iff",
			//"object/tangible/wearables/shoes/shared_shoes_s02.iff",
			//"object/tangible/wearables/shoes/shared_shoes_s03.iff",
//				"object/tangible/wearables/boots/shared_boots_s04.iff",
			//"object/tangible/wearables/boots/shared_boots_s05.iff",
			//"object/tangible/wearables/boots/shared_boots_s12.iff",
			"object/tangible/wearables/armor/tantel/shared_armor_tantel_skreej_boots.iff",
			"object/tangible/wearables/armor/tantel/shared_armor_tantel_skreej_chest_plate.iff",
		};

		const int arraySize = sizeof (hackObjectNames) / sizeof (hackObjectNames [0]);

		{
			for (int i = 0; i < arraySize; ++i)
			{
				ClientObject * const cobj = dynamic_cast<ClientObject *>(ObjectTemplate::createObject (hackObjectNames [i]));

				if (cobj)
				{
					cobj->endBaselines ();
					if (i == 0)
						cobj->setObjectName (Unicode::narrowToWide ("object zero"));
					IGNORE_RETURN(ContainerInterface::transferItemToVolumeContainer (*inventoryObject, *cobj));
				}
				else
				{
					DEBUG_WARNING(true, ("Unable to create ui hack object: %s", hackObjectNames [i]));
				}
			}
		}
	}

	if (asynchronousLoaderEnabled)
		AsynchronousLoader::enable ();
}

//----------------------------------------------------------------------

void SwgCuiInventory::onSelectionChanged (const CuiContainerSelectionChanged::Payload & payload)
{
	if(m_info)
		m_info->setInfoObject(payload.second);
}

//----------------------------------------------------------------------

void SwgCuiInventory::saveSettings () const
{
	CuiMediator::saveSettings ();

	if (m_onlyStoreBaseMediatorSettings)
		return;

	saveSortSettings(m_containerMediator, getMediatorDebugName(), Settings::inventoryContainerSortColumn, Settings::inventoryContainerSortSelection);

	if(m_eqMediator)
		CuiSettings::saveBoolean(cms_mediatorBaseName.c_str(), cms_paperdollSettingName, m_paperdollVisible);
	if(m_info)
		CuiSettings::saveBoolean(cms_mediatorBaseName.c_str(), cms_examineSettingName, m_examineVisible);
}

//----------------------------------------------------------------------

void SwgCuiInventory::loadSettings ()
{
	CuiMediator::loadSettings ();

	if (m_onlyStoreBaseMediatorSettings)
		return;

	bool showExamine = false;
	bool result = CuiSettings::loadBoolean(cms_mediatorBaseName.c_str(), cms_examineSettingName, showExamine);

	if (result)
		m_examineVisible = showExamine;
	else
		m_examineVisible = true;

	if (m_info)
	{
		if (m_examineVisible)
		{
			m_info->activate();
			if (m_containerMediator)
			{
				m_containerMediator->activate();
				m_info->setInfoObject(m_containerMediator->getLastSelection());
			}
		}
		else
		{
			m_info->deactivate();
		}
	}

	loadSortSettings(m_containerMediator, getMediatorDebugName(), Settings::inventoryContainerSortColumn, Settings::inventoryContainerSortSelection);

	if(m_eqMediator)
	{
		m_eqMediator->deactivate();
		bool showPaperdoll = true;
		result = CuiSettings::loadBoolean(cms_mediatorBaseName.c_str(), cms_paperdollSettingName, showPaperdoll);
		if(showPaperdoll)
		{
			m_eqMediator->activate();
			ClientObject * const player = Game::getClientPlayer ();
			m_eqMediator->setupCharacterViewer (player);		
		}
	}
}

void SwgCuiInventory::openSelectedRadial()
{
	if(m_containerMediator)
		m_containerMediator->openSelectedRadial();
}

// ======================================================================


