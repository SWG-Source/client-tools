//======================================================================
//
// SwgCuiHudWindowManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudWindowManager.h"

#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroupObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/TangibleObject.h"

#include "clientGraphics/Graphics.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiHueObject.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiIMEManager.h"
#include "clientUserInterface/CuiInputMessage.def"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSecureTradeManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIdsShipComponent.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/cuiMediatorFactory.h"
#include "sharedFoundation/Production.h"
#include "sharedInputMap/InputMap.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"
#include "sharedNetworkMessages/CloseHolocronMessage.h"
#include "sharedNetworkMessages/EditAppearanceMessage.h"
#include "sharedNetworkMessages/EditStatsMessage.h"
#include "sharedNetworkMessages/EnterStructurePlacementModeMessage.h"
#include "sharedNetworkMessages/EnterTicketPurchaseModeMessage.h"
#include "sharedNetworkMessages/NewbieTutorialEnableHudElement.h"
#include "sharedNetworkMessages/NewbieTutorialHighlightUIElement.h"
#include "sharedNetworkMessages/NewbieTutorialRequest.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"
#include "sharedNetworkMessages/OpenHolocronToPageMessage.h"
#include "sharedNetworkMessages/ResourceHarvesterActivatePageMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/StartingLocationData.h"
#include "swgClientUserInterface/SwgCuiAllTargets.h"
#include "swgClientUserInterface/SwgCuiAvatarLocation2.h"
#include "swgClientUserInterface/SwgCuiButtonBar.h"
#include "swgClientUserInterface/SwgCuiCharacterSheet.h"
#include "swgClientUserInterface/SwgCuiChatRoomsWho.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiColorTest.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"
#include "swgClientUserInterface/SwgCuiCustomize.h"
#include "swgClientUserInterface/SwgCuiDpsMeter.h"
#include "swgClientUserInterface/SwgCuiHarassmentMessage.h"
#include "swgClientUserInterface/SwgCuiHighlight.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiIncap.h"
#include "swgClientUserInterface/SwgCuiInventory.h"
#include "swgClientUserInterface/SwgCuiKillMeter.h"
#include "swgClientUserInterface/SwgCuiLocationDisplay.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiMfdStatus.h"
#include "swgClientUserInterface/SwgCuiMissionDetails.h"
#include "swgClientUserInterface/SwgCuiNetStatus.h"
#include "swgClientUserInterface/SwgCuiNewMacro.h"
#include "swgClientUserInterface/SwgCuiNotifications.h"
#include "swgClientUserInterface/SwgCuiOpt.h"
#include "swgClientUserInterface/SwgCuiPermissionList.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageComposer.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"
#include "swgClientUserInterface/SwgCuiRating.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "swgClientUserInterface/SwgCuiShipComponentManagement.h"
#include "swgClientUserInterface/SwgCuiSpaceZoneMap.h"
#include "swgClientUserInterface/SwgCuiStructurePlacement.h"
#include "swgClientUserInterface/SwgCuiSystemMessage.h"
#include "swgClientUserInterface/SwgCuiTargets.h"
#include "swgClientUserInterface/SwgCuiTicketPurchase.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission.h"
#include "swgClientUserInterface/SwgCuiToolbar.h"
#include "swgClientUserInterface/SwgCuiTrade.h"
#include "swgSharedNetworkMessages/ConsentRequestMessage.h"
#include "swgSharedNetworkMessages/PermissionListCreateMessage.h"

#include "UIData.h"
#include "UIIMEManager.h"
#include "UIManager.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UnicodeUtils.h"

//======================================================================

namespace SwgCuiHudWindowManagerNamespace
{
	namespace UnnamedMessages
	{
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}

	const std::string cms_newbieTutorialRequestOpenCharacterSheet  ("openCharacterSheet");
	const std::string cms_newbieTutorialRequestCloseCharacterSheet ("closeCharacterSheet");
	const std::string cms_newbieTutorialRequestOpenInventory       ("openInventory");
	const std::string cms_newbieTutorialRequestCloseInventory      ("closeInventory");
	const std::string cms_newbieTutorialEnableHudElementAll        ("all");
	const std::string cms_newbieTutorialEnableHudElementChatbox    ("chatbox");
	const std::string cms_newbieTutorialEnableHudElementHambar     ("hambar");
	const std::string cms_newbieTutorialEnableHudElementToolbar    ("toolbar");
	const std::string cms_newbieTutorialEnableHudElementButtonbar  ("buttonbar");
	const std::string cms_newbieTutorialRequestOpenStatMigration   ("openStatMigration");
	const std::string cms_newbieTutorialRequestCloseStatMigration  ("closeStatMigration");
	const std::string cms_newbieTutorialRequestOpenHolocron        ("openHolocron");
	const std::string cms_newbieTutorialRequestCloseHolocron       ("closeHolocron");

	bool ms_spaceChatVisible = true;	
}

using namespace SwgCuiHudWindowManagerNamespace;

//----------------------------------------------------------------------

SwgCuiHudWindowManager::ItemRequestInfo::ItemRequestInfo (const ClientObject * _containerObject, const std::string & _slotName, bool _newWindow) :
container (), 
slotName  (_slotName), 
newWindow (_newWindow) 
{
	if (_containerObject)
		container = CachedNetworkId (*_containerObject);
}

//----------------------------------------------------------------------

SwgCuiHudWindowManager::SwgCuiHudWindowManager (const SwgCuiHud & hud, CuiWorkspace & workspace) :
MessageDispatch::Receiver   (),
m_callback                  (new MessageDispatch::Callback),
m_chatWindowMediator        (0),
m_mfdStatusMediator         (0),
m_toolbarMediator           (0),
m_inventory                 (0),
m_buttonBar                 (0),
m_workspace                 (&workspace),
m_lastFrameUpdateMFD        (0),
m_WindowManagerActive       (false),
m_pendingBeginTrade         (false),
m_pendingRequestTrade       (false),
m_pendingRequestTradeId     (),
m_pendingItemOpenInfo       (new ItemRequestInfoVector),
m_pendingItemCloseInfo      (new ItemRequestInfoVector),
m_lastHudOpacity            (-1.0f),
m_sendOpenCharacterSheet    (false),
m_sendCloseCharacterSheet   (false),
m_sendOpenStatMigration     (false),
m_sendCloseStatMigration    (false),
m_sendOpenInventory         (false),
m_sendCloseInventory        (false),
m_sendOpenHolocron          (false),
m_sendCloseHolocron         (false),
m_blinkingMediator (0),
m_blinkTime (0.f),
m_singleToolbarPage(0),
m_doubleToolbarPage(0),
m_singleToolbar(0),
m_doubleToolbar(0)
{
	
	{
		UIPage * mediatorPage = 0;
		
		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "SystemMessage");
			mediatorPage->SetEnabled (false);
			SwgCuiSystemMessage * const sysMessage = new SwgCuiSystemMessage (*mediatorPage);
			sysMessage->setStickyVisible (true);
			sysMessage->activate         ();
			m_workspace->addMediator     (*sysMessage);
		} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.
		
		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "SystemMessageNoob");
			mediatorPage->SetEnabled (false);
			SwgCuiSystemMessage * const sysMessageNoob = new SwgCuiSystemMessage (*mediatorPage, SwgCuiSystemMessage::T_noob);
			sysMessageNoob->setStickyVisible (true);
			sysMessageNoob->activate         ();
			m_workspace->addMediator     (*sysMessageNoob);
		} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.
		
		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     m_singleToolbarPage,    "Toolbar");
			hud.getCodeDataObject (TUIPage,     m_doubleToolbarPage,    "DoubleToolbar");
			m_singleToolbarPage->SetVisible(false);
			m_doubleToolbarPage->SetVisible(false);
			m_toolbarMediator = new SwgCuiToolbar (*getToolbarPage(), Game::getHudSceneType());
			m_toolbarMediator->setSettingsAutoSizeLocation (false, true);
			m_toolbarMediator->setStickyVisible(!Game::isHudSceneTypeSpace());
			m_toolbarMediator->setShowFocusedGlowRect(false);
			m_toolbarMediator->fetch ();
			m_toolbarMediator->activate ();
			m_toolbarMediator->startProcessingActions();
			m_workspace->addMediator (*m_toolbarMediator);
			cacheToolbar();
		}

		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "Notifications");
			m_notificationsMediator = new SwgCuiNotifications (*mediatorPage, Game::getHudSceneType());
			m_notificationsMediator->setSettingsAutoSizeLocation (true, true);
			m_notificationsMediator->setStickyVisible(!Game::isHudSceneTypeSpace());
			m_notificationsMediator->setShowFocusedGlowRect(false);
			m_notificationsMediator->fetch ();
			m_notificationsMediator->activate ();
			m_workspace->addMediator (*m_notificationsMediator);
		}

		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "Highlight");
			m_highlightMediator = new SwgCuiHighlight (*mediatorPage, Game::getHudSceneType());
			m_highlightMediator->setSettingsAutoSizeLocation (true, true);
			m_highlightMediator->setStickyVisible(!Game::isHudSceneTypeSpace());
			m_highlightMediator->setShowFocusedGlowRect(false);
			m_highlightMediator->fetch ();
			m_highlightMediator->activate ();
			m_workspace->addMediator (*m_highlightMediator);
		}
		
		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "ButtonBar");
			m_buttonBar = new SwgCuiButtonBar (*mediatorPage);
			m_buttonBar->setSettingsAutoSizeLocation (true, true);
			m_buttonBar->setStickyVisible(!Game::isHudSceneTypeSpace());
			m_buttonBar->setShowFocusedGlowRect(false);
			m_buttonBar->fetch ();
			m_buttonBar->activate ();
			m_workspace->addMediator (*m_buttonBar);
		}
		
		//----------------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "incap");
			mediatorPage->SetEnabled (false);
			SwgCuiIncap * const incap = new SwgCuiIncap (*mediatorPage);
			incap->setSettingsAutoSizeLocation (true, true);
			incap->setStickyVisible (true);
			m_workspace->addMediator (*incap);
		} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.
		
		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject (TUIPage,     mediatorPage,           "ChatWindow");
			mediatorPage->SetVisible(false);
			mediatorPage->SetEnabled (false);
			m_chatWindowMediator = SwgCuiChatWindow::createInto(m_workspace, Game::getHudSceneType());
			m_chatWindowMediator->setSettingsAutoSizeLocation (true, true);
			m_chatWindowMediator->setStickyVisible (true);
			m_chatWindowMediator->fetch();
			m_workspace->addMediator (*m_chatWindowMediator);
			m_chatWindowMediator->activate();
		}

		//-----------------------------------------------------------------
		{
			hud.getCodeDataObject(TUIPage, mediatorPage, "questHelper");
			m_questHelper = new SwgCuiQuestHelper(*mediatorPage);
			m_questHelper->setSettingsAutoSizeLocation(true, true);
			m_questHelper->setStickyVisible(true);
			m_questHelper->setShowFocusedGlowRect(false);
			m_questHelper->fetch();
			m_questHelper->activate();
			m_workspace->addMediator(*m_questHelper);
		}
	}  //lint !e429 //custodial pointer

	if (CuiPreferences::getNetStatusEnabled ())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_NetStatus);

	if (CuiPreferences::getUseExpMonitor ())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ExpMonitorBox);

	if (CuiPreferences::getUseWaypointMonitor ())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_WaypointMonitorBox);

	if (CuiPreferences::getLocationDisplayEnabled() && !Game::isHudSceneTypeSpace())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_LocationDisplay);

	if (!Game::isHudSceneTypeSpace())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_KillMeter);

	if (CuiPreferences::getDpsMeterEnabled() && !Game::isHudSceneTypeSpace())
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_DpsMeter);

	CuiIMEManager::GetCuiIMEManager()->SetIMEIndicator(Game::isHudSceneTypeSpace() ? CuiMediatorTypes::WS_IMEIndicatorSpace : CuiMediatorTypes::WS_IMEIndicator);	

	if (CuiPreferences::getVoiceShowFlybar())
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_VoiceFlyBar);

}

//----------------------------------------------------------------------

SwgCuiHudWindowManager::~SwgCuiHudWindowManager ()
{
	delete m_callback;
	m_callback = 0;

	delete m_pendingItemOpenInfo;
	m_pendingItemOpenInfo = 0;

	delete m_pendingItemCloseInfo;
	m_pendingItemCloseInfo = 0;

	if (m_workspace != NULL)
	{
		m_workspace->setEffectors(0, 0);
		
		if (m_chatWindowMediator != NULL)
		{
			m_workspace->removeMediator (*m_chatWindowMediator);
		}

		if (m_toolbarMediator != NULL)
		{
			m_workspace->removeMediator (*m_toolbarMediator);
		}

		if (m_buttonBar != NULL)
		{
			m_workspace->removeMediator (*m_buttonBar);
		}

		if (m_notificationsMediator != NULL)
		{
			m_workspace->removeMediator (*m_notificationsMediator);
		}

		if (m_highlightMediator != NULL)
		{
			m_workspace->removeMediator (*m_highlightMediator);
		}

		if (m_questHelper != NULL)
		{
			m_workspace->removeMediator (*m_questHelper);
		}
	}

	if (m_inventory != NULL)
	{
		if (m_workspace != NULL)
		{
			if (m_inventory->getContainingWorkspace () == m_workspace)
				m_workspace->removeMediator (*m_inventory);
		}

		m_inventory->release ();
		m_inventory = 0;
	}


	if (m_chatWindowMediator)
	{
		m_chatWindowMediator->release ();
		m_chatWindowMediator = 0;
	}

	if (m_mfdStatusMediator)
	{
		m_mfdStatusMediator->release ();
		m_mfdStatusMediator = 0;
	}

	if (m_toolbarMediator)
	{
		m_toolbarMediator->release ();
		m_toolbarMediator = 0;
	}

	if (m_notificationsMediator)
	{
		m_notificationsMediator->release ();
		m_notificationsMediator = 0;
	}

	if (m_buttonBar)
	{
		m_buttonBar->release ();
		m_buttonBar      = 0;
	}

	if (m_highlightMediator)
	{
		m_highlightMediator->release ();
		m_highlightMediator      = 0;
	}

	if (m_questHelper)
	{
		m_questHelper->release();
		m_questHelper = 0;
	}

	if(m_singleToolbar)
	{
		m_singleToolbar->release();
		m_singleToolbar = 0;
	}
	if(m_doubleToolbar)
	{
		m_doubleToolbar->release();
		m_doubleToolbar = 0;
	}
	
	m_workspace      = 0;
	m_blinkingMediator = 0;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::handlePerformActivate ()
{
	if (m_WindowManagerActive)
		return;

	connectToMessage (Game::Messages::SCENE_CHANGED);
	connectToMessage (ResourceHarvesterActivatePageMessage::MESSAGE_TYPE);
	connectToMessage (UnnamedMessages::ConnectionServerConnectionClosed);
	connectToMessage (PermissionListCreateMessage::MessageType);
	connectToMessage (EnterStructurePlacementModeMessage::cms_name);
	connectToMessage (EnterTicketPurchaseModeMessage::cms_name);
	connectToMessage (NewbieTutorialRequest::cms_name);
	connectToMessage (NewbieTutorialEnableHudElement::cms_name);
	connectToMessage (ConsentRequestMessage::cms_name);
	connectToMessage (OpenHolocronToPageMessage::MessageType);
	connectToMessage (CloseHolocronMessage::MessageType);
	connectToMessage (EditAppearanceMessage::MessageType);
	connectToMessage (EditStatsMessage::MessageType);
	connectToMessage (NewbieTutorialHighlightUIElement::cms_name);


	m_callback->connect (*this, &SwgCuiHudWindowManager::onBeginTrade,                static_cast<ClientSecureTradeManager::Messages::BeginTrade *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onTradeRequested,            static_cast<ClientSecureTradeManager::Messages::Requested *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onItemOpenRequest,           static_cast<CuiInventoryManager::Messages::ItemOpenRequest *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onItemOpenRequestNewWindow,  static_cast<CuiInventoryManager::Messages::ItemOpenRequestNewWindow *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onItemCloseAllInstances,     static_cast<CuiInventoryManager::Messages::ItemCloseAllInstances *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onCreatureDamageTaken,       static_cast<CreatureObject::Messages::DamageTaken *>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onStartingLocationsReceived, static_cast<PlayerCreatureController::Messages::StartingLocationsReceived*>(0));
	m_callback->connect (*this, &SwgCuiHudWindowManager::onStartComposingNewMessage,  static_cast<CuiPersistentMessageManager::Messages::StartComposing *>(0));

	m_WindowManagerActive = true;

	m_workspace->setEnabled (true);

	if (ConfigClientGame::getDebugPrint ())
	{
		CuiMediatorFactory::activate (CuiMediatorTypes::DebugInfoPage);
	}

	updateHudOpacity ();

	if (NULL != m_mfdStatusMediator)
		m_mfdStatusMediator->setTarget(Game::getPlayerCreature());
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::handlePerformDeactivate ()
{
	if (!m_WindowManagerActive)
		return;
	
	disconnectFromMessage (Game::Messages::SCENE_CHANGED);
	disconnectFromMessage (ResourceHarvesterActivatePageMessage::MESSAGE_TYPE);
	disconnectFromMessage (UnnamedMessages::ConnectionServerConnectionClosed);
	disconnectFromMessage (PermissionListCreateMessage::MessageType);
	disconnectFromMessage (EnterStructurePlacementModeMessage::cms_name);
	disconnectFromMessage (EnterTicketPurchaseModeMessage::cms_name);
	disconnectFromMessage (NewbieTutorialRequest::cms_name);
	disconnectFromMessage (NewbieTutorialEnableHudElement::cms_name);
	disconnectFromMessage (ConsentRequestMessage::cms_name);
	disconnectFromMessage (OpenHolocronToPageMessage::MessageType);
	disconnectFromMessage (CloseHolocronMessage::MessageType);
	disconnectFromMessage (EditAppearanceMessage::MessageType);
	disconnectFromMessage (EditStatsMessage::MessageType);
	disconnectFromMessage (NewbieTutorialHighlightUIElement::cms_name);

	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onBeginTrade,                static_cast<ClientSecureTradeManager::Messages::BeginTrade *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onTradeRequested,            static_cast<ClientSecureTradeManager::Messages::Requested *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onItemOpenRequest,           static_cast<CuiInventoryManager::Messages::ItemOpenRequest *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onItemOpenRequestNewWindow,  static_cast<CuiInventoryManager::Messages::ItemOpenRequestNewWindow *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onItemCloseAllInstances,     static_cast<CuiInventoryManager::Messages::ItemCloseAllInstances *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onCreatureDamageTaken,       static_cast<CreatureObject::Messages::DamageTaken *>(0));
	m_callback->disconnect (*this, &SwgCuiHudWindowManager::onStartingLocationsReceived, static_cast<PlayerCreatureController::Messages::StartingLocationsReceived*>(0));

	m_WindowManagerActive = false;

	m_workspace->setEnabled (false);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{	

	//----------------------------------------------------------------------
 
	if (message.isType (Game::Messages::SCENE_CHANGED))
	{
		if (m_mfdStatusMediator != NULL)
		{
			m_mfdStatusMediator->setTarget (dynamic_cast<TangibleObject *>(Game::getPlayer ()));
		}
		CuiIMEManager::GetCuiIMEManager()->SetIMEIndicator(Game::isHudSceneTypeSpace() ? CuiMediatorTypes::WS_IMEIndicatorSpace : CuiMediatorTypes::WS_IMEIndicator);	
	}

	//----------------------------------------------------------------------

	else if (message.isType (ResourceHarvesterActivatePageMessage::MESSAGE_TYPE))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		ResourceHarvesterActivatePageMessage	act(ri);
		IGNORE_RETURN(CuiActionManager::performAction  (CuiActions::harvester, Unicode::narrowToWide (act.getHarvesterId().getValueString())));
	}

	//----------------------------------------------------------------------

	else if  (message.isType (PermissionListCreateMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const PermissionListCreateMessage plcm(ri);
		spawnPermissionList (plcm);
	}

	//----------------------------------------------------------------------

	else if (message.isType (EnterStructurePlacementModeMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const EnterStructurePlacementModeMessage msg (ri);

		const NetworkId&   deedNetworkId                     = msg.getDeedNetworkId ();
		const std::string& structureSharedObjectTemplateName = msg.getStructureSharedObjectTemplateName ();

		CuiMediatorFactory::activate (CuiMediatorTypes::StructurePlacement);
		if (!safe_cast<SwgCuiStructurePlacement*> (NON_NULL (CuiMediatorFactory::get (CuiMediatorTypes::StructurePlacement, false)))->setData (deedNetworkId, structureSharedObjectTemplateName.c_str ()))
		{
			DEBUG_WARNING (true, ("GroundScene::receiveMessage (EnterStructurePlacementModeMessage): setting structure placement data failed!"));
			CuiMediatorFactory::deactivate (CuiMediatorTypes::StructurePlacement);
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType (EnterTicketPurchaseModeMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const EnterTicketPurchaseModeMessage msg (ri);

		const std::string& planetName      = msg.getPlanetName ();
		const std::string& travelPointName = msg.getTravelPointName ();
		const bool instantTravel = msg.getInstantTravel();

		SwgCuiTicketPurchase * const mediator = safe_cast<SwgCuiTicketPurchase *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_TicketPurchase));

		if(mediator)
		{
			mediator->setTravelType(instantTravel ?
				SwgCuiTicketPurchase::TT_instantTravel : SwgCuiTicketPurchase::TT_buyTicket);
			if (!mediator->setData (planetName, travelPointName))
			{
				DEBUG_WARNING (true, ("GroundScene::receiveMessage (EnterTicketPurchaseModeMessage): setting ticket purchase data failed!"));
			}
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType (NewbieTutorialRequest::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialRequest newbieTutorialRequest (ri);

		if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestOpenCharacterSheet)
			m_sendOpenCharacterSheet = true;
		else if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestCloseCharacterSheet)
			m_sendCloseCharacterSheet = true;
		else if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestOpenInventory)
			m_sendOpenInventory = true;
		else if (newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestCloseInventory)
		{
			m_sendCloseInventory = true;
			if(m_inventory)
				m_inventory->setSendClose(true);
		}
		else if(newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestOpenStatMigration)
			m_sendOpenStatMigration = true;
		else if(newbieTutorialRequest.getRequest () == cms_newbieTutorialRequestCloseStatMigration)
			m_sendCloseStatMigration = true;
	}

	//----------------------------------------------------------------------

	else if (message.isType (NewbieTutorialEnableHudElement::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialEnableHudElement newbieTutorialEnableHudElement (ri);

		if (newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementChatbox || newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementAll)
		{
			setBlinkingMediator (*m_chatWindowMediator, newbieTutorialEnableHudElement.getBlinkTime ());

			if (newbieTutorialEnableHudElement.getEnable ())
			{
				activateChatWindow(true);
				ms_spaceChatVisible = true;
			}
			else
			{
				activateChatWindow(false);
				ms_spaceChatVisible = false;
			}
		}

		if (newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementHambar || newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementAll)
		{
			if (m_mfdStatusMediator != NULL)
			{
				setBlinkingMediator (*m_mfdStatusMediator, newbieTutorialEnableHudElement.getBlinkTime ());

				if (newbieTutorialEnableHudElement.getEnable ())
				{
					m_mfdStatusMediator->activate ();
				}
				else
				{
					m_mfdStatusMediator->deactivate ();
				}
			}
		}

		if (newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementToolbar || newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementAll)
		{
			setBlinkingMediator (*m_toolbarMediator, newbieTutorialEnableHudElement.getBlinkTime ());

			if (newbieTutorialEnableHudElement.getEnable ())
			{
				m_toolbarMediator->activate ();
			}
			else
			{
				m_toolbarMediator->deactivate ();
			}
		}

		if (newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementButtonbar || newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementAll)
		{
			setBlinkingMediator (*m_buttonBar, newbieTutorialEnableHudElement.getBlinkTime ());

			if (newbieTutorialEnableHudElement.getEnable ())
			{
				m_buttonBar->activate ();
			}
			else
			{
				m_buttonBar->deactivate ();
			}
		}
	}

	//----------------------------------------------------------------------

	if (message.isType (NewbieTutorialHighlightUIElement::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialHighlightUIElement newbieTutorialHighlightUIElement (ri);

		const std::string & widgetPath = newbieTutorialHighlightUIElement.getWidgetPath();	
		if(_strnicmp(widgetPath.c_str(), "/GroundHUD.ButtonBar.buttonsComposite", 37) == 0)
		{
			if(m_buttonBar)
				m_buttonBar->ensureMenuIsVisible();
		}
		//HACK these really shouldn't be here.  We're misusing "show arrows around a widget" to instead just show the widget in these cases.
		else if(_stricmp(widgetPath.c_str(), "/GroundHUD.Quest") == 0)
		{
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_QuestJournal);
		}
		else if(_stricmp(widgetPath.c_str(), "/GroundHUD.RoadMap") == 0)
		{
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Roadmap);
		}
	}

	else if (message.isType (ConsentRequestMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		ConsentRequestMessage consentRequestMessage (ri);

		const ProsePackage& p = consentRequestMessage.getQuestion();
		Unicode::String s;
		IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(p, s));
		const int id     = consentRequestMessage.getId();
		spawnConsent(s, id);
	}

	else if (message.isType (OpenHolocronToPageMessage::MessageType))
	{
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::service, Unicode::emptyString));
	}

	else if (message.isType (EditAppearanceMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		EditAppearanceMessage eam(ri);
		UNREF(eam);
	}

	else if (message.isType (EditStatsMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		EditStatsMessage esm(ri);
		//@TODO: This message should probably just go away
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::acceptTextInput (bool setKeyboardInput)
{
	m_workspace->focusMediator (*m_chatWindowMediator, true);
	m_chatWindowMediator->acceptTextInput (true, setKeyboardInput);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::update ()
{
	if (m_lastHudOpacity != CuiPreferences::getHudOpacity ()) //lint !e777 //floats equality
		updateHudOpacity ();
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::toggleMediator (CuiMediator & mediator)
{
	if (!mediator.isActive ())
	{
		mediator.activate ();
		m_workspace->focusMediator (mediator, true);
		mediator.setEnabled (true);
	}
	else
	{
		if (mediator.isIconified ())
			m_workspace->restoreFromIcon (0, mediator);
		else
			m_workspace->close (mediator);
	}
}

//----------------------------------------------------------------------

SwgCuiInventory * SwgCuiHudWindowManager::createInventory (ClientObject * container, const std::string & slotName, bool usePaperDoll)
{
	SwgCuiInventory * const inv = SwgCuiInventory::createInto (m_workspace->getPage (), container, slotName, usePaperDoll, usePaperDoll);

	if (inv)
	{
		inv->getPage ().Center ();
		m_workspace->addMediator (*inv);
	}

	return inv;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::toggleInventory (ClientObject * container)
{
	if (!m_inventory)
	{
		m_inventory = NON_NULL (createInventory (container, std::string (), true));
		m_inventory->fetch ();
	}
	
	if (m_inventory->isActive ())
	{
		m_inventory->deactivate ();
		//newbie tutorial response is sent from the inventory's deactivate function
	}
	else
	{
		m_inventory->activate ();
		m_inventory->setEnabled (true);

		if (m_sendCloseInventory)
		{
			m_sendCloseInventory = false;
			m_inventory->setSendClose(true);
		}

		if (container)
			m_inventory->setContainerObject (container, std::string ());

		m_workspace->focusMediator(*m_inventory, true);

		if (m_sendOpenInventory)
		{
			m_sendOpenInventory = false;

			const NewbieTutorialResponse response (cms_newbieTutorialRequestOpenInventory);
			GameNetwork::send (response, true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::toggleCommunity ()
{
	CuiMediator * mediator = m_workspace->findMediatorByType (typeid (SwgCuiCommunity));

	if (mediator)
	{
		toggleMediator (*mediator);
	}
	else
	{
		SwgCuiCommunity * const community = SwgCuiCommunity::createInto (&m_workspace->getPage ());
		community->setSettingsAutoSizeLocation (true, true);
		community->getPage ().Center ();
		m_workspace->addMediator (*community);
		community->activate ();
		m_workspace->focusMediator (*community, true);
		community->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::toggleService ()
{
	CuiMediator * mediator = m_workspace->findMediatorByType (typeid (SwgCuiService));

	if (mediator)
	{
		toggleMediator (*mediator);
	}
	else
	{
		SwgCuiService * const service = SwgCuiService::createInto (&m_workspace->getPage ());
		service->setSettingsAutoSizeLocation (true, true);
		service->getPage ().Center ();
		m_workspace->addMediator (*service);
		service->activate ();
		m_workspace->focusMediator (*service, true);
		service->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnTicketSubmission ()
{
	CuiMediator * mediator = m_workspace->findMediatorByType (typeid (SwgCuiTicketSubmission));

	if (mediator)
	{
		if (!mediator->isActive ())
		{
			mediator->activate ();
			m_workspace->focusMediator (*mediator, true);
			mediator->setEnabled (true);
		}
	}
	else
	{
		SwgCuiTicketSubmission * const ticketSubmission = SwgCuiTicketSubmission::createInto (&m_workspace->getPage ());
		ticketSubmission->setSettingsAutoSizeLocation (true, true);
		ticketSubmission->getPage ().Center ();
		m_workspace->addMediator (*ticketSubmission);
		ticketSubmission->activate ();
		m_workspace->focusMediator (*ticketSubmission, true);
		ticketSubmission->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnHarassmentMessage ()
{
	SwgCuiHarassmentMessage * mediator = safe_cast<SwgCuiHarassmentMessage *>(m_workspace->findMediatorByType (typeid (SwgCuiHarassmentMessage)));

	if (mediator)
	{
		toggleMediator (*mediator);
		mediator->setFromKBSearch(false);
	}
	else
	{
		SwgCuiHarassmentMessage * const harassmentMessage = SwgCuiHarassmentMessage::createInto (&m_workspace->getPage ());
		harassmentMessage->setSettingsAutoSizeLocation (true, true);
		harassmentMessage->getPage ().Center ();
		harassmentMessage->setFromKBSearch(false);
		m_workspace->addMediator (*harassmentMessage);
		harassmentMessage->activate ();
		m_workspace->focusMediator (*harassmentMessage, true);
		harassmentMessage->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnHarassmentMessageFromKBSearch ()
{
	SwgCuiHarassmentMessage * mediator = safe_cast<SwgCuiHarassmentMessage *>(m_workspace->findMediatorByType (typeid (SwgCuiHarassmentMessage)));

	if (mediator)
	{
		toggleMediator (*mediator);
		mediator->setFromKBSearch(true);
	}
	else
	{
		SwgCuiHarassmentMessage * const harassmentMessage = SwgCuiHarassmentMessage::createInto (&m_workspace->getPage ());
		harassmentMessage->setSettingsAutoSizeLocation (true, true);
		harassmentMessage->getPage ().Center ();
		harassmentMessage->setFromKBSearch(true);
		m_workspace->addMediator (*harassmentMessage);
		harassmentMessage->activate ();
		m_workspace->focusMediator (*harassmentMessage, true);
		harassmentMessage->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnCharacterSheet (CreatureObject *creatureToExamine)
{
	CuiMediator * mediator = m_workspace->findMediatorByType (typeid (SwgCuiCharacterSheet));
		
	if (mediator)
	{
		SwgCuiCharacterSheet *cs = dynamic_cast<SwgCuiCharacterSheet*>(mediator);

		if (cs)
		{
			if (creatureToExamine)
			{
				if (mediator->isActive())
				{
					toggleMediator(*mediator);
					cs->setExamineMode(creatureToExamine);
				}
				else
					cs->setExamineMode(creatureToExamine);
			}
			else
			{
				cs->setExamineMode(0);

				if (mediator->isActive() && !cs->isExaminingSelf())
					toggleMediator(*mediator);
			}

			toggleMediator(*mediator);
		}

	}
	else
	{
		SwgCuiCharacterSheet * const characterSheet = SwgCuiCharacterSheet::createInto (m_workspace->getPage ());
		characterSheet->setExamineMode(creatureToExamine);
		characterSheet->setSettingsAutoSizeLocation (true, true);
		m_workspace->addMediator (*characterSheet);
		characterSheet->activate ();
		m_workspace->focusMediator (*characterSheet, true);
		characterSheet->setEnabled (true);

		mediator = characterSheet;
	}

	if (mediator)
	{
		if (mediator->isActive ())
		{
			if (m_sendOpenCharacterSheet)
			{
				m_sendOpenCharacterSheet = false;

				const NewbieTutorialResponse response (cms_newbieTutorialRequestOpenCharacterSheet);
				GameNetwork::send (response, true);
			}
		}
		else
		{
			if (m_sendCloseCharacterSheet)
			{
				m_sendCloseCharacterSheet = false;

				const NewbieTutorialResponse response (cms_newbieTutorialRequestCloseCharacterSheet);
				GameNetwork::send (response, true);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnPermissionList (const PermissionListCreateMessage& msg)
{
	SwgCuiPermissionList * mediator = safe_cast<SwgCuiPermissionList *>(m_workspace->findMediatorByType (typeid (SwgCuiPermissionList)));

	if (!mediator)
	{
		mediator = SwgCuiPermissionList::createInto (m_workspace->getPage ());
		mediator->setSettingsAutoSizeLocation (true, true);
		m_workspace->addMediator   (*mediator);
		mediator->activate         ();
		m_workspace->focusMediator (*mediator, true);
		mediator->setEnabled       (true);
	}

	mediator->activate ();
	mediator->setData(msg);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnNewMacro (const Unicode::String& params)
{
	UNREF(params);
	SwgCuiNewMacro * newMacro = safe_cast<SwgCuiNewMacro *>(m_workspace->findMediatorByType (typeid (SwgCuiNewMacro)));

	if (!newMacro)
	{
		newMacro = SwgCuiNewMacro::createInto (m_workspace->getPage ());
		NOT_NULL (newMacro);
		newMacro->setSettingsAutoSizeLocation (true, true);
		m_workspace->addMediator (*newMacro);
	}

	m_workspace->focusMediator (*newMacro, true);
	newMacro->setEnabled (true);
	newMacro->activate ();
	newMacro->setParams(params);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnMissionDetails ()
{
	CuiMediator * const mediator = m_workspace->findMediatorByType (typeid (SwgCuiMissionDetails));
		
	if (mediator)
	{
		toggleMediator (*mediator);
	}
	else
	{
		SwgCuiMissionDetails * const missionDetails = SwgCuiMissionDetails::createInto (m_workspace->getPage ());
		missionDetails->setSettingsAutoSizeLocation (true, true);
		m_workspace->addMediator (*missionDetails);
		missionDetails->activate ();
		m_workspace->focusMediator (*missionDetails, true);
		missionDetails->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnChatRoomWho (uint32 roomId)
{
	SwgCuiChatRoomsWho * mediator = SwgCuiChatRoomsWho::getMediatorForRoom (roomId);

	if (!mediator)
	{
		mediator = SwgCuiChatRoomsWho::createInto (m_workspace->getPage ());
		NOT_NULL (mediator);
		mediator->setSettingsAutoSizeLocation (true, true);
		m_workspace->addMediator (*mediator);
		mediator->setRoomId (roomId);
	}

	mediator->activate ();
	if (mediator->isActive ())
	{
		m_workspace->focusMediator (*mediator, true);
		mediator->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnConsent(const Unicode::String& question, int id) const
{
	CuiConsentManager::showConsentWindow(question, id, false);
}


//----------------------------------------------------------------------
void SwgCuiHudWindowManager::spawnCustomizationWindow(NetworkId const & objectId, std::string const &customVarName1, int minVar1, int maxVar1, std::string const &customVarName2, int minVar2, int maxVar2, std::string const &customVarName3, int minVar3, int maxVar3, std::string const & customVarName4, int minVar4, int maxVar4) const
{
	SwgCuiCustomize * const mediator = safe_cast<SwgCuiCustomize * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Customize));
	mediator->setCustomizationObject(objectId, customVarName1, minVar1, maxVar1, customVarName2, minVar2, maxVar2, customVarName3, minVar3, maxVar3, customVarName4, minVar4, maxVar4);
}

void SwgCuiHudWindowManager::spawnRatingWindow(std::string const & windowTitle, std::string const & windowText)
{
	SwgCuiRating * const mediator = safe_cast<SwgCuiRating * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Rating));
	
	mediator->setWindowTitle(windowTitle);
	mediator->setWindowDescription(windowText);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnShipComponentManagement(NetworkId const & shipId, NetworkId const & terminalId) const
{
	Object * const o = NetworkIdManager::getObjectById(shipId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	ShipObject * const ship = co ? co->asShipObject() : NULL;
	if(ship)
	{
		SwgCuiShipComponentManagement * const mediator = safe_cast<SwgCuiShipComponentManagement * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ShipComponentManagement));
		if(mediator)
		{
			SwgCuiShipComponentManagement::SetShipResult const result = mediator->setShip(ship);
			if(result == SwgCuiShipComponentManagement::SSR_newbieShip)
			{
				CuiMessageBox::createInfoBox(CuiStringIdsShipComponent::err_cannot_open_newbie_ship.localize());
			}
			if(result == SwgCuiShipComponentManagement::SSR_nonCombatShip)
			{
				CuiMessageBox::createInfoBox(CuiStringIdsShipComponent::err_cannot_open_noncombat_ship.localize());
			}
			else if(!PlayerObject::isAdmin() && (result == SwgCuiShipComponentManagement::SSR_notCertifiedForShip))
			{
				CuiMessageBox::createInfoBox(CuiStringIdsShipComponent::err_cannot_open_ship_not_certified.localize());
			}
			mediator->setTerminal(terminalId);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::toggleButtonBar()
{
	m_buttonBar->toggleMenu();
}

//----------------------------------------------------------------------

bool SwgCuiHudWindowManager::isButtonBarVisible()
{
	return m_buttonBar->isCompositeVisible();
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::colorTest (TangibleObject & object)
{
	UNREF(object); //for release build
#if PRODUCTION == 0
	SwgCuiColorTest * const colorTestMediator = SwgCuiColorTest::createInto (m_workspace->getPage ());
	if (colorTestMediator)
	{
		m_workspace->addMediator (*colorTestMediator);
		colorTestMediator->activate ();
		m_workspace->focusMediator (*colorTestMediator, true);
		colorTestMediator->setEnabled (true);
		colorTestMediator->setObject (object);
	}
#endif
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::hueObjectTest (const NetworkId & id, int maxIndex1, int maxIndex2)
{
	UNREF (id);
	UNREF (maxIndex1);
	UNREF (maxIndex2);
#if PRODUCTION == 0
	CuiHueObject * const hueTest = CuiHueObject::createInto (m_workspace->getPage ());
	if (hueTest)
	{
		m_workspace->addMediator (*hueTest);
		hueTest->activate ();
		m_workspace->focusMediator (*hueTest, true);
		hueTest->setEnabled (true);
		ClientObject * const object = safe_cast<ClientObject *>(CachedNetworkId (id).getObject ());
		if (object)
			hueTest->setTarget (object->getNetworkId(), maxIndex1, maxIndex2);
	}
#endif
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onBeginTrade     (const bool & )
{
	m_pendingBeginTrade = true;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onTradeRequested (const NetworkId & other)
{
	m_pendingRequestTrade    = true;
	m_pendingRequestTradeId = other;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onTradeRequestDenied                  (const NetworkId & recipient)
{
	const CachedNetworkId id (recipient);
	
	const ClientObject * const obj = safe_cast<ClientObject *>(id.getObject ());

	Unicode::String result;

	if (obj)
		CuiStringVariablesManager::process (CuiStringIdsTrade::request_player_denied_prose, obj->getLocalizedName (), Unicode::String (), Unicode::String (), result);
	else
		result = CuiStringIdsTrade::request_player_denied_no_obj.localize ();

	CuiSystemMessageManager::sendFakeSystemMessage (result);
} //lint !e1762 //stfu

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onTradeRequestDeniedPlayerBusy        (const NetworkId & recipient)
{
	const CachedNetworkId id (recipient);
	
	const ClientObject * const obj = safe_cast<ClientObject *>(id.getObject ());

	Unicode::String result;

	if (obj)
		CuiStringVariablesManager::process (CuiStringIdsTrade::request_player_busy_prose, obj->getLocalizedName (), Unicode::String (), Unicode::String (), result);
	else
		result = CuiStringIdsTrade::request_player_busy_no_obj.localize ();

	CuiSystemMessageManager::sendFakeSystemMessage (result);
} //lint !e1762 //stfu

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onTradeRequestDeniedPlayerUnreachable (const NetworkId & recipient)
{
	const CachedNetworkId id (recipient);
	
	const ClientObject * const obj = safe_cast<ClientObject *>(id.getObject ());

	Unicode::String result;

	//-- obj is the target in this string
	if (obj)
		CuiStringVariablesManager::process (CuiStringIdsTrade::request_player_unreachable_prose, Unicode::String (), obj->getLocalizedName (), Unicode::String (), result);
	else
		result = CuiStringIdsTrade::request_player_unreachable_no_obj.localize ();

	CuiSystemMessageManager::sendFakeSystemMessage (result);
} //lint !e1762 //stfu

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::updateWindowManager (const float elapsedTime)
{
	if (m_blinkingMediator)
	{
		m_blinkTime -= elapsedTime;

		if (m_blinkTime > 0.f)
		{
			if (m_blinkTime - static_cast<int> (m_blinkTime) > 0.5f)
			{
				if (m_blinkingMediator->isActive ())
					m_blinkingMediator->deactivate ();
			}
			else
			{
				if (!m_blinkingMediator->isActive ())
					m_blinkingMediator->activate ();
			}
		}
		else
		{
			if (!m_blinkingMediator->isActive ())
				m_blinkingMediator->activate ();

			m_blinkingMediator = 0;
		}
	}
	else
	{
		if (m_pendingBeginTrade)
		{
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Trade);
			m_pendingBeginTrade = false;
		}

		else if (m_pendingRequestTrade)
		{
			const CachedNetworkId & id = ClientSecureTradeManager::getRequestInitiator ();

			const ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
			if (obj)
			{
				CuiSecureTradeManager::receiveRequest (*obj);
			}

			m_pendingRequestTrade = false;
			m_pendingRequestTradeId = NetworkId::cms_invalid;
		}
		
		if (!m_pendingItemOpenInfo->empty ())
		{
			for (ItemRequestInfoVector::iterator it = m_pendingItemOpenInfo->begin (); it != m_pendingItemOpenInfo->end (); ++it)
			{
				const ItemRequestInfo & info = *it;
				ClientObject * const container = safe_cast<ClientObject *>(info.container.getObject ());

				if (!container)
					continue;
				
				if (info.newWindow)
				{

					// start handle shift click looting
					InputMap* inputMap = Game::getGameInputMap();
					if(inputMap)
					{
						uint32 val = inputMap->getShiftState();
						if(val & CuiM_BITS_SHIFT) 
						{
							bool bLootRuleOK = true;
							CreatureObject const * const player = Game::getPlayerCreature();
							if (player)
							{
								GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());
								if (group 
									&& (group->getLootRule() == GroupObject::LR_lottery 
										|| group->getLootRule() == GroupObject::LR_random
									)
								)
								{
									bLootRuleOK = false;	
								}
							}
							
							if(bLootRuleOK)
							{
								ClientObject * const containedBy = const_cast<ClientObject *>(safe_cast<const ClientObject *>(ContainerInterface::getContainedByObject (*container)));	
								if(containedBy)
								{
									CreatureObject * const creature = containedBy->asCreatureObject ();
									if (creature && creature->isDead())
									{
										ClientCommandQueue::enqueueCommand ("loot", creature->getNetworkId(), Unicode::emptyString);
										CuiSoundManager::play (CuiSounds::radial_complete);
										continue;
									}
								}
							}	
						}
					}
					// end handle shift click looting
				}

				SwgCuiInventory * inventory = SwgCuiInventory::findInventoryPageByContainer (container->getNetworkId (), info.slotName);

				// If the inventory ui does not already exist, create it, if it does exist
				// we need to decrement the opened counter for the object because it was
				// just incremented by the CuiInventoryManager prior to asking us to open a window
				if (!inventory)
					inventory = NON_NULL (createInventory (container, info.slotName, false));
				else
					CuiInventoryManager::notifyItemClosed(*container, info.slotName);
				
				inventory->activate ();
				inventory->setEnabled (true);
				m_workspace->focusMediator (*inventory, true);

			}
			m_pendingItemOpenInfo->clear ();
		}

		if (!m_pendingItemCloseInfo->empty ())
		{
			for (ItemRequestInfoVector::iterator it = m_pendingItemCloseInfo->begin (); it != m_pendingItemCloseInfo->end (); ++it)
			{
				const ItemRequestInfo & info = *it;
				
				ClientObject * const container = safe_cast<ClientObject *>(info.container.getObject ());
				if (container)
				{
					//-- there can be more than one open container UI on a given container 
					SwgCuiInventory::InventoryVector iv;
					SwgCuiInventory::findInventoryPagesByContainer (container->getNetworkId (), info.slotName, iv);

					for (SwgCuiInventory::InventoryVector::const_iterator iit = iv.begin (); iit != iv.end (); ++iit)
					{
						SwgCuiInventory * const inventory = NON_NULL (*iit);
						inventory->deactivate ();
						m_workspace->removeMediator (*inventory);
					}
				}
			}
			m_pendingItemCloseInfo->clear ();
		}
	}
	if(m_toolbarMediator && (m_toolbarMediator->isDoubleToolbar() != CuiPreferences::getUseDoubleToolbar()))
	{
		UIPoint oldLocation = m_toolbarMediator->getPage().GetLocation();
		int oldPane = m_toolbarMediator->getCurrentPane();
		m_toolbarMediator->deactivate();
		m_toolbarMediator->stopProcessingActions();
		m_workspace->removeMediator(*m_toolbarMediator);
		m_toolbarMediator->release();

		SwgCuiToolbar *cachedToolbar = getCachedToolbar();
		if(cachedToolbar)
		{
			m_toolbarMediator = cachedToolbar;
			m_toolbarMediator->loadSettings();
		}
		else
		{
			m_toolbarMediator = new SwgCuiToolbar (*getToolbarPage(), Game::getHudSceneType());
			m_toolbarMediator->setSettingsAutoSizeLocation (true, true);
			m_toolbarMediator->setStickyVisible(!Game::isHudSceneTypeSpace());
			m_toolbarMediator->setShowFocusedGlowRect(false);
			cacheToolbar();
		}
		m_toolbarMediator->fetch ();
		m_toolbarMediator->activate ();
		m_toolbarMediator->startProcessingActions();
		m_toolbarMediator->getPage().SetLocation(oldLocation);
		m_toolbarMediator->switchToPane(oldPane);
		m_workspace->addMediator (*m_toolbarMediator);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::receiveDroppedChatTab                 (const UIPoint & pt, float value)
{
	SwgCuiChatWindow::cloneFromTabDragged (pt, static_cast<int>(value));

} //lint !e1762 //stfu

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onItemOpenRequest (const CuiInventoryManager::Messages::ItemOpenRequest::Payload & msg)
{		
	ClientObject * const container = msg.first;
	const std::string & slotname   = msg.second;
	
	if (!container)
		return;
	
	m_pendingItemOpenInfo->push_back (ItemRequestInfo (container, slotname, false));
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onItemOpenRequestNewWindow (const CuiInventoryManager::Messages::ItemOpenRequestNewWindow::Payload & msg)
{
	ClientObject * const container = msg.first;
	const std::string & slotname   = msg.second;

	if (!container)
		return;

	m_pendingItemOpenInfo->push_back (ItemRequestInfo (container, slotname, true));
}
		
//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onItemCloseAllInstances (const CuiInventoryManager::Messages::ItemCloseAllInstances::Payload & msg)
{
	ClientObject * const container = msg.first;
	const std::string & slotname   = msg.second;
		
	if (!container)
		return;

	m_pendingItemCloseInfo->push_back (ItemRequestInfo (container, slotname, false));
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::updateHudOpacity ()
{
	m_lastHudOpacity = CuiPreferences::getHudOpacity ();

	m_workspace->updateOpacities (m_lastHudOpacity, m_lastHudOpacity);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onCreatureDamageTaken (const CreatureObject & creature)
{
	if (&creature == static_cast<const Object* >(Game::getPlayer ()))
	{
		if (Game::isViewFirstPerson ())
		{
			int h = 0, a = 0, m = 0;		
			creature.getAccumulatedHamDamage (h, a, m);

			displayPlayerDamageText (h, a, m);
		}
		
		if (m_mfdStatusMediator != NULL)
		{
			m_mfdStatusMediator->getPage ().SetEnabled (false);
			m_mfdStatusMediator->getPage ().SetOpacity (0.0f);
			m_mfdStatusMediator->getPage ().SetEnabled (true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::displayPlayerDamageText (int h, int a, int m)
{
	if (m_mfdStatusMediator != NULL)
	{
		const UIPoint & mfdLocation = m_mfdStatusMediator->getPage ().GetWorldLocation ();
		const UIPoint & centerPoint = mfdLocation + (m_mfdStatusMediator->getPage ().GetSize () / 2L);
		
		const UIPoint & screenCenter = m_workspace->getPage ().GetSize () / 2L;
		
		UIPoint pt = mfdLocation;
		
		bool leftRight  = false;
		
		if (centerPoint.x < screenCenter.x)
		{
			pt.x += m_mfdStatusMediator->getPage ().GetWidth ();
			leftRight = true;
		}
		
		SwgCuiAllTargets * const allTargets = safe_cast<SwgCuiAllTargets *>(m_workspace->findMediatorByType (typeid (SwgCuiAllTargets)));
		
		NOT_NULL (allTargets);
		
		allTargets->displayDamageText (pt, leftRight, h, a, m);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onStartingLocationsReceived   (const PlayerCreatureController::Messages::StartingLocationsReceived::Payload & locations)
{
	SwgCuiAvatarLocation2 * const avloc2 = safe_cast<SwgCuiAvatarLocation2 *>(CuiMediatorFactory::activate   (CuiMediatorTypes::AvatarLocation2));
	SwgCuiHudFactory::setHudActive(false);

	avloc2->setLocations (locations);
} //lint !e1762 function could be const (no it couldn't it must have this signature for the transceiver)

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::onStartComposingNewMessage            (const ChatPersistentMessageToClientData & data)
{
	//-- fromCharacterName contains the embedded recipients list
	SwgCuiPersistentMessageComposer::setNextParameters (data.fromCharacterName, data.subject, data.message, data.outOfBand);
	IGNORE_RETURN(CuiActionManager::performAction (CuiActions::persistentMessageComposer, Unicode::emptyString));
} //lint !e1762 function could be const (no it couldn't it must have this signature for the transceiver)

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::spawnSpaceZoneMap(Unicode::String const & params) const
{
	CuiMediator * const mediator = CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_SpaceZoneMap);
	SwgCuiSpaceZoneMap * const zoneMap = safe_cast<SwgCuiSpaceZoneMap *>(mediator);
	if (zoneMap != 0)
	{
		std::string sceneId(Unicode::wideToNarrow(params));
		if (sceneId.empty())
		{
			sceneId = Game::getSceneId();
		}

		zoneMap->setSceneId(sceneId);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::setBlinkingMediator(CuiMediator &mediator, float blinkTime)
{
	if (blinkTime > 0.f)
	{
		m_blinkingMediator = &mediator;
		m_blinkTime = blinkTime;
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::setStatusMediator(SwgCuiMfdStatus * status)
{
	if (status)
		status->fetch();

	if (m_mfdStatusMediator)
		m_mfdStatusMediator->release();

	m_mfdStatusMediator = status;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::activateChatWindow(bool val)
{
	if(val)
	{
		if (!m_chatWindowMediator->isActive ())
		{
			m_chatWindowMediator->activate ();
		}
	}
	else
	{
		if (m_chatWindowMediator->isActive ())
		{
			m_chatWindowMediator->deactivate ();
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiHudWindowManager::isSpaceChatVisible()
{
	return ms_spaceChatVisible;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::setSpaceChatVisible(bool val)
{
	ms_spaceChatVisible = val;
}

//----------------------------------------------------------------------

UIPage *SwgCuiHudWindowManager::getToolbarPage()
{
	if(CuiPreferences::getUseDoubleToolbar())
		return m_doubleToolbarPage;
	else
		return m_singleToolbarPage;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManager::cacheToolbar()
{
	if(CuiPreferences::getUseDoubleToolbar())
	{
		if(!m_doubleToolbar)
		{
			m_doubleToolbar = m_toolbarMediator;
			m_doubleToolbar->fetch();
		}
	}
	else
	{
		if(!m_singleToolbar)
		{
			m_singleToolbar = m_toolbarMediator;
			m_singleToolbar->fetch();
		}
	}
}

void SwgCuiHudWindowManager::setPetToolbarVisible(const bool b)
{
	m_toolbarMediator->setPetBarVisible(b);
}

//----------------------------------------------------------------------

SwgCuiToolbar *SwgCuiHudWindowManager::getCachedToolbar()
{
	SwgCuiToolbar *result = NULL;
	if(CuiPreferences::getUseDoubleToolbar())
		result = m_doubleToolbar;
	else
		result = m_singleToolbar;
	return result;
}

//======================================================================
