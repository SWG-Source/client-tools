//======================================================================
//
// SwgCuiHudAction.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudAction.h"

#include "shellapi.h"

#include "UIButton.h"
#include "UIClipboard.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIUtils.h"

#include "clientGame/AuctionManagerClient.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlanetMapManagerClient.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientGame/ShipObject.h"
#include "clientGame/Species.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiBugSubmissionPage.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiChatParser.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiNotepad.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiSetName.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSurveyManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/TextIterator.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/HttpGetEncoder.h"
#include "swgClientUserInterface/SwgCuiActions.h"
#include "swgClientUserInterface/SwgCuiAppearanceTab.h"
#include "swgClientUserInterface/SwgCuiBiographyEdit.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffee.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffer.h"
#include "swgClientUserInterface/SwgCuiChatRooms.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"
#include "swgClientUserInterface/SwgCuiCollections.h"
#include "swgClientUserInterface/SwgCuiCommandBrowser.h"
#include "swgClientUserInterface/SwgCuiExamine.h"
#include "swgClientUserInterface/SwgCuiGalacticCivilWar.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiHudWindowManager.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerGround.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerSpace.h"
#include "swgClientUserInterface/SwgCuiImageDesignerDesigner.h"
#include "swgClientUserInterface/SwgCuiImageDesignerRecipient.h"
#include "swgClientUserInterface/SwgCuiIncubator.h"
#include "swgClientUserInterface/SwgCuiLfg.h"
#include "swgClientUserInterface/SwgCuiLootBox.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiMissionBrowser.h"
#include "swgClientUserInterface/SwgCuiNpeContinuation.h"
#include "swgClientUserInterface/SwgCuiPlayerQuestConversation.h"
#include "swgClientUserInterface/SwgCuiQuestAcceptance.h"
#include "swgClientUserInterface/SwgCuiQuestBuilder.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction.h"
#include "swgClientUserInterface/SwgCuiSpaceConversation.h"
#include "swgClientUserInterface/SwgCuiTcgManager.h"
#include "swgClientUserInterface/SwgCuiTcgWindow.h"
#include "swgClientUserInterface/SwgCuiToolbar.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"
#include "swgClientUserInterface/SwgCuiVoiceFlyBar.h"


#if PRODUCTION != 0
#include "sharedDebug/DebugFlags.h" //lint !e766 //only used in release build
#endif

//======================================================================

namespace
{
	float s_toggleTime = 1.0f / 20.f;
	int s_toggleDebounceCounter = 250;
	bool s_allowServiceWindow = false;

	CuiMessageBox * s_confirmCsBrowserSpawn = 0;
	CuiMessageBox * s_confirmOpenQuestBuilder = 0;

	class MyCallback : public MessageDispatch::Callback
	{
	public:

		MyCallback () :
		  MessageDispatch::Callback ()
		  {
		  }

		  void onMessageBoxClosed (const CuiMessageBox & box)
		  {
			  if (&box == s_confirmCsBrowserSpawn)
			  {
				  // if confirmed, execute the service action with a
				  // "yes" parameter.
				  if (box.completedAffirmative())
				  {
					  IGNORE_RETURN(CuiActionManager::performAction (CuiActions::service, Unicode::narrowToWide("yes")));
				  }
			  }
			  else if (&box == s_confirmOpenQuestBuilder)
			  {
				  if (box.completedAffirmative())
				  {
					  IGNORE_RETURN(CuiActionManager::performAction (CuiActions::questBuilder, Unicode::narrowToWide("yes")));
				  }
			  }
		  }
	};
	MyCallback * s_callback = 0;
}

//----------------------------------------------------------------------

SwgCuiHudAction::SwgCuiHudAction (SwgCuiHud & hud) :
CuiAction (),
m_mediator (hud),
m_nextToggleUpDeactivesPointer (false),
m_toggleDebounceCounter       (0),
m_nextToggleUpDeactivesNames   (false),
m_toggleDownTimeNames          (0.0f)
{
	CuiActionManager::addAction (CuiActions::bugReport,                  this, false);
	CuiActionManager::addAction (CuiActions::characterSheet,             this, false);
	CuiActionManager::addAction (CuiActions::chatBubblesClear,           this, false);
	CuiActionManager::addAction (CuiActions::chatRoomBrowser,            this, false);
	CuiActionManager::addAction (CuiActions::chatRoomWho,                this, false);
	CuiActionManager::addAction (CuiActions::commandBrowser,             this, false);
	CuiActionManager::addAction (CuiActions::commandBrowserString,       this, false);
	CuiActionManager::addAction (CuiActions::commandBrowserMacro,        this, false);
	CuiActionManager::addAction (CuiActions::community,                  this, false);
	CuiActionManager::addAction (CuiActions::dataPad,                    this, false);
	CuiActionManager::addAction (CuiActions::debugPrint,                 this, false);
	CuiActionManager::addAction (CuiActions::examine,                    this, false);
	CuiActionManager::addAction (CuiActions::examineCharacterSheet,      this, false);
	CuiActionManager::addAction (CuiActions::inventoryClose,             this, false);
	CuiActionManager::addAction (CuiActions::missionBrowser,             this, false);
	CuiActionManager::addAction (CuiActions::missionCreation,            this, false);
	CuiActionManager::addAction (CuiActions::missionDetails,             this, false);
	CuiActionManager::addAction (CuiActions::permissionList,             this, false);
	CuiActionManager::addAction (CuiActions::persistentMessageBrowser,   this, false);
	CuiActionManager::addAction (CuiActions::persistentMessageComposer,  this, false);
	CuiActionManager::addAction (CuiActions::pointerToggleDown,          this, false);
	CuiActionManager::addAction (CuiActions::pointerToggleUp,            this, false);
	CuiActionManager::addAction (CuiActions::service,                    this, false);
	CuiActionManager::addAction (CuiActions::skills,                     this, false);
	CuiActionManager::addAction (CuiActions::startChat,                  this, false);
	CuiActionManager::addAction (CuiActions::startChatCommand,           this, false);
	CuiActionManager::addAction (CuiActions::startChatGroup,             this, false);
	CuiActionManager::addAction (CuiActions::startChatReply,             this, false);
	CuiActionManager::addAction (CuiActions::startChatTell,              this, false);
	CuiActionManager::addAction (CuiActions::targetGroup0,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup1,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup2,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup3,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup4,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup5,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup6,               this, false);
	CuiActionManager::addAction (CuiActions::targetGroup7,               this, false);
	CuiActionManager::addAction (CuiActions::targetSelf,                 this, false);
	CuiActionManager::addAction (CuiActions::terminalAuctionUse,         this, false);
	CuiActionManager::addAction (CuiActions::toggleChatBubblesMinimized, this, false);
	CuiActionManager::addAction (CuiActions::toggleHud,                  this, false);
	CuiActionManager::addAction (CuiActions::toggleObjectNamesDown,      this, false);
	CuiActionManager::addAction (CuiActions::toggleObjectNamesUp,        this, false);
	CuiActionManager::addAction (CuiActions::toggleObjectNamesMyName,    this, false);
	CuiActionManager::addAction (CuiActions::vendorUse,                  this, false);
	CuiActionManager::addAction (CuiActions::imageDesignerDesigner,      this, false);
	CuiActionManager::addAction (CuiActions::imageDesignerRecipient,     this, false);
	CuiActionManager::addAction (CuiActions::planetMap,                  this, false);
	CuiActionManager::addAction (CuiActions::newMacro,                   this, false);
	CuiActionManager::addAction (CuiActions::netStatus,                  this, false);
	CuiActionManager::addAction (CuiActions::gameMenuActivate,           this, false);
	CuiActionManager::addAction (CuiActions::untarget,                   this, false);
	CuiActionManager::addAction (CuiActions::clientExamine,              this, false);
	CuiActionManager::addAction (CuiActions::defaultAction,              this, false);
	CuiActionManager::addAction (CuiActions::radialMenu,                 this, false);
	CuiActionManager::addAction (CuiActions::options,                    this, false);
	CuiActionManager::addAction (CuiActions::targetAtCursor,             this, false);
	CuiActionManager::addAction (CuiActions::targetAtCursorStop,         this, false);
	CuiActionManager::addAction (CuiActions::setIntendedTarget,          this, false);
	CuiActionManager::addAction (SwgCuiActions::toggleButtonBar,         this, false);

	CuiActionManager::addAction (CuiActions::chatCopy,                   this, false);
	CuiActionManager::addAction (CuiActions::chatCut,                    this, false);
	CuiActionManager::addAction (CuiActions::chatPaste,                  this, false);
	CuiActionManager::addAction (CuiActions::console,                    this, false);
	CuiActionManager::addAction (CuiActions::askconsent,                 this, false);
	CuiActionManager::addAction (CuiActions::getSample,                  this, false);

	CuiActionManager::addAction (CuiActions::expMonitor,                 this, false);
	CuiActionManager::addAction (CuiActions::waypointMonitor,            this, false);
	CuiActionManager::addAction (CuiActions::locationDisplay,            this, false);
	CuiActionManager::addAction (CuiActions::dpsMeter,                   this, false);
	CuiActionManager::addAction (CuiActions::questJournal,               this, false);
	CuiActionManager::addAction (CuiActions::questAcceptance,            this, false);
	CuiActionManager::addAction (CuiActions::questHelper,                this, false);
	CuiActionManager::addAction (CuiActions::roadmap,                    this, false);
	CuiActionManager::addAction (CuiActions::expertise,                  this, false);
	CuiActionManager::addAction (CuiActions::ticketPurchase,             this, false);

	CuiActionManager::addAction (CuiActions::harvester,                  this, false);

	CuiActionManager::addAction (CuiActions::defaultButton,              this, false);
	CuiActionManager::addAction (CuiActions::notepad,                    this, false);
	CuiActionManager::addAction (CuiActions::setName,                    this, false);

	CuiActionManager::addAction (CuiActions::ticketSubmission,           this, false);
	CuiActionManager::addAction (CuiActions::harassmentMessage,          this, false);
	CuiActionManager::addAction (CuiActions::harassmentMessageFromKBSearch,this, false);
	CuiActionManager::addAction (CuiActions::manageShipComponents,       this, false);
	CuiActionManager::addAction (CuiActions::spaceZoneMap,               this, false);
	CuiActionManager::addAction (CuiActions::editBiography,              this, false);

	CuiActionManager::addAction (CuiActions::commPlayer,                 this, false);
	CuiActionManager::addAction (CuiActions::lootBox,                    this, false);
	CuiActionManager::addAction (CuiActions::npeContinuation,            this, false);
	CuiActionManager::addAction (CuiActions::blueGlowieOn,               this, false);
	CuiActionManager::addAction (CuiActions::blueGlowieOff,              this, false);
	CuiActionManager::addAction (CuiActions::holonetOn,                  this, false);
	CuiActionManager::addAction (CuiActions::holonetOff,                 this, false);

	CuiActionManager::addAction (CuiActions::spaceConversation,          this, false);
	CuiActionManager::addAction (CuiActions::setIntendedAndSummonRadialMenu, this, false);
	CuiActionManager::addAction (CuiActions::openCustomizationWindow,       this, false);

	CuiActionManager::addAction (CuiActions::openBuffBuilderBufferWindow, this, false);
	CuiActionManager::addAction (CuiActions::openBuffBuilderBuffeeWindow, this, false);

	CuiActionManager::addAction (CuiActions::untargetOnly, this, false);
	
	CuiActionManager::addAction (CuiActions::openIncubatorWindow, this, false);

	CuiActionManager::addAction (CuiActions::collections, this, false);
	CuiActionManager::addAction (CuiActions::myCollections, this, false);

	CuiActionManager::addAction(CuiActions::sendSavedPlayerInterestsToServer, this, false);
	CuiActionManager::addAction (SwgCuiActions::toggleVoiceFlyBar, this, false);

	CuiActionManager::addAction (CuiActions::tcg, this, false);

	CuiActionManager::addAction (CuiActions::appearanceTab, this, false);

	CuiActionManager::addAction (CuiActions::questBuilder, this, false);

	CuiActionManager::addAction (CuiActions::rating, this, false);
	CuiActionManager::addAction (CuiActions::commPlayerQuest, this, false);

	CuiActionManager::addAction (CuiActions::gcwInfo, this, false);

	static bool s_installed = false;
	if (!s_installed)
	{
		s_installed = true;
#if PRODUCTION == 0
		s_allowServiceWindow = true;
#else
		s_allowServiceWindow = false;
		DebugFlags::registerFlag(s_allowServiceWindow,       "ClientUserInterface", "allowServiceWindow");
#endif
	}
	if (s_callback == 0)
	{
		s_callback = new MyCallback;
	}
}

//----------------------------------------------------------------------

SwgCuiHudAction::~SwgCuiHudAction ()
{
	CuiActionManager::removeAction (this);
	if (s_callback)
	{
		delete s_callback;
		s_callback = 0;
	}
}

//----------------------------------------------------------------------

bool  SwgCuiHudAction::performAction (const std::string & id, const Unicode::String & params) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	
	static const Unicode::String chat_cmdStr  (1, CuiChatParser::getCmdChar ());
	static const Unicode::String space (1, ' ');
	static const Unicode::String dot (1, '.');
	static const Unicode::String chat_tellStr   = chat_cmdStr + CuiInstantMessageManager::ms_cmdTell + space;
	static const Unicode::String chat_replyStr  = chat_cmdStr + CuiInstantMessageManager::ms_cmdReply + space;
//	static const Unicode::String chat_retellStr = chat_cmdStr + CuiInstantMessageManager::ms_cmdRetell + space;
	static const Unicode::String chat_groupStr  = chat_cmdStr + CuiChatRoomManager::ms_cmdGroup + space;


	if (id == CuiActions::spaceConversation)
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_SpaceConversation, false);
	else if (id == CuiActions::startChat)
		Game::startChatInput (Unicode::emptyString);
	else if (id == CuiActions::startChatCommand)
		Game::startChatInput (chat_cmdStr);
	else if (id == CuiActions::startChatTell)
		Game::startChatInput (chat_tellStr);
	else if (id == CuiActions::startChatReply)
	{
		// "Intelligent" reply.
		ChatAvatarId const & avatarInfo = CuiInstantMessageManager::getLastChatId();

		// Check for cross server chat.
		bool const crossServerChat = GameNetwork::getCentralServerName() != avatarInfo.cluster;
		
		// Build the appropriate name.
		if (avatarInfo.getName().empty()) 
		{
			// Unable to send a reply unless you receive one.
			Game::startChatInput(chat_tellStr + space);
			return false;
		}
		else
		{
			Unicode::String const & avatarName = crossServerChat ? Unicode::narrowToWide(avatarInfo.getFullName()) : avatarInfo.getName();
			Unicode::String const & tellString = chat_cmdStr + CuiInstantMessageManager::ms_cmdTell + space + avatarName + space;
			Game::startChatInput(tellString);
		}
	}
	else if (id == CuiActions::startChatGroup)
		Game::startChatInput (chat_groupStr);
	else if (id == CuiActions::inventoryClose)
		m_mediator.m_windowManager->toggleInventory (0);
	else if (id == CuiActions::notepad)
	{
		// if no file is specified, revert to default behavior (i.e. toggle notepad window)
		if (params.empty())
		{
			CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::Notepad);
		}
		else
		{
			CuiNotepad * notepad = dynamic_cast<CuiNotepad *>(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::Notepad, false, false));
			if (notepad && notepad->isActive())
			{
				// save the current notepad
				notepad->saveToFile();

				// set the user specified file
				CuiNotepad::setUserSpecifiedFileName(Unicode::wideToNarrow(params));

				// open the user specified file
				notepad->loadFromFile();
			}
			else
			{
				// set the user specified file
				CuiNotepad::setUserSpecifiedFileName(Unicode::wideToNarrow(params));

				// activate notepad
				CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::Notepad);
			}
		}
	}
	else if (id == CuiActions::setName)
	{
		CuiSetName::createNewInstance (params);
	}

	else if (id == CuiActions::persistentMessageBrowser)
		CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_PersistentMessageBrowser);

	else if (id == CuiActions::persistentMessageComposer)
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_PersistentMessageComposer, true);

	else if (id == CuiActions::chatRoomBrowser)
	{
		if (params.empty ())
			CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_ChatRooms);
		else
		{
			const uint32 roomId = static_cast<uint32>(atoi (Unicode::wideToNarrow (params).c_str ()));
			SwgCuiChatRooms * const chatRoomsMediator = safe_cast<SwgCuiChatRooms *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ChatRooms));
			chatRoomsMediator->selectChatRoom (roomId);
		}
	}

	else if (id == CuiActions::chatRoomWho)
	{
		const uint32 roomId = static_cast<uint32>(atoi (Unicode::wideToNarrow (params).c_str ()));
		m_mediator.m_windowManager->spawnChatRoomWho (roomId);
	}
	else if (id == CuiActions::toggleHud)
		m_mediator.setHudEnabled (!m_mediator.getHudEnabled ());
		
	else if (id == CuiActions::toggleChatBubblesMinimized)
		CuiChatBubbleManager::setBubblesMinimized (!CuiChatBubbleManager::getBubblesMinimized ());
		
	else if (id == CuiActions::chatBubblesClear)
		CuiChatBubbleManager::clearAll ();

	else if (id == CuiActions::conversationStart)
		m_mediator.startConversingWithSelectedCreature ();
		
	else if (id == CuiActions::conversationStop)
		IGNORE_RETURN (CuiConversationManager::stop ());
		
	else if (id == CuiActions::pointerToggleDown)
	{
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (CuiManager::getInputToggleActive () && timeMs > m_toggleDebounceCounter)
		{
			m_toggleDebounceCounter = timeMs + s_toggleDebounceCounter;
		}
	}			
	else if (id == CuiActions::pointerToggleUp)
	{	
		int const timeMs = static_cast<int>(Clock::timeMs());
		if (CuiManager::getInputToggleActive () && m_toggleDebounceCounter > timeMs)
		{
			m_toggleDebounceCounter = timeMs;

			const bool currentState = CuiManager::getPointerInputActive ();

			CuiManager::setPointerToggledOn(!currentState); 
			CuiSoundManager::play (CuiSounds::toggle_mouse_mode);
		}
	}
	else if (id == CuiActions::toggleObjectNamesDown)
	{
		const bool currentState = CuiPreferences::getDrawObjectNames ();

		m_toggleDownTimeNames = Clock::timeMs () * 0.001f;			
		CuiPreferences::setDrawObjectNames (!currentState);
	}
	else if (id == CuiActions::toggleObjectNamesUp)
	{
		const bool currentState = CuiPreferences::getDrawObjectNames ();
		
		float curTime = Clock::timeMs () * 0.001f;
		curTime -= Clock::frameTime ();
		
		if ((curTime - m_toggleDownTimeNames) > s_toggleTime)
			CuiPreferences::setDrawObjectNames (!currentState);
	}
	else if (id == CuiActions::toggleObjectNamesMyName)
	{
		CuiPreferences::setDrawObjectNamesMyName(!CuiPreferences::getDrawObjectNamesMyName());
	}
	else if (id == CuiActions::commandBrowser)
	{
		if (params.empty ())
			CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_CommandBrowser);
		else
		{
			SwgCuiCommandBrowser * const cb = safe_cast<SwgCuiCommandBrowser *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CommandBrowser));
			NOT_NULL (cb);
			cb->selectCommand (Unicode::wideToNarrow (params));
		}
	}
	else if (id == CuiActions::commandBrowserString)
	{
		SwgCuiCommandBrowser * const cb = safe_cast<SwgCuiCommandBrowser *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CommandBrowser));
		NOT_NULL (cb);
		cb->selectString (Unicode::wideToNarrow (params));
	}
	else if (id == CuiActions::commandBrowserMacro)
	{
		SwgCuiCommandBrowser * const cb = safe_cast<SwgCuiCommandBrowser *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CommandBrowser));
		NOT_NULL (cb);
		cb->selectMacro (Unicode::wideToNarrow (params));
	}
	else if (id == CuiActions::characterSheet)
		m_mediator.m_windowManager->spawnCharacterSheet (NULL);
	else if (id == CuiActions::dataPad)
		CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_DataStorage);

	//access the mission terminals
	else if (id == CuiActions::missionBrowser)
	{
		SwgCuiMissionBrowser* browser = safe_cast<SwgCuiMissionBrowser*>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::MissionBrowser));
		browser->refreshList();
	}
	//access the mission details ui
	else if (id == CuiActions::missionDetails)
		m_mediator.m_windowManager->spawnMissionDetails ();

	else if (id == CuiActions::targetSelf)
	{
		if (player)
		{
			if (player->getNetworkId() == player->getLookAtTarget())
			{
				ShipObject const * const playerContainingShip = ShipObject::getContainingShip(*player);
				if (NULL != playerContainingShip)
				{
					player->setLookAtTarget(playerContainingShip->getNetworkId());
					return true;
				}
			}

			player->setLookAtAndIntendedTarget (player->getNetworkId ());
		}
	}

	else if (id == CuiActions::targetGroup0)
	{
		if (player)
			player->targetGroupMember (0);
	}
	else if (id == CuiActions::targetGroup1)
	{
		if (player)
			player->targetGroupMember (1);
	}
	else if (id == CuiActions::targetGroup2)
	{
		if (player)
			player->targetGroupMember (2);
	}
	else if (id == CuiActions::targetGroup3)
	{
		if (player)
			player->targetGroupMember (3);
	}
	else if (id == CuiActions::targetGroup4)
	{
		if (player)
			player->targetGroupMember (4);
	}
	else if (id == CuiActions::targetGroup5)
	{
		if (player)
			player->targetGroupMember (5);
	}
	else if (id == CuiActions::targetGroup6)
	{
		if (player)
			player->targetGroupMember (6);
	}
	else if (id == CuiActions::targetGroup7)
	{
		if (player)
			player->targetGroupMember (7);
	}
	else if (id == CuiActions::examine)
	{
		Object * const obj = CuiAction::findObjectFromFirstParam (params, true, false, id);
		
		if (obj)
		{
			if (CuiPreferences::getDebugExamine ())
			{
				Unicode::String wstr;
				ObjectAttributeManager::formatDebugInfo (*obj, wstr);
				CuiChatRoomManager::sendPrelocalizedChat(wstr);

				if (CuiPreferences::getDebugClipboardExamine ())
				{
					TextIterator textIterator(wstr);
					UIClipboard::gUIClipboard ().SetText (textIterator.getPrintableText ());
				}
			}
			
			SwgCuiExamine * const examineMediator = safe_cast<SwgCuiExamine * >(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Examine));
			NOT_NULL (examineMediator);
			examineMediator->setTarget (obj);
		}
	}
	else if (id == CuiActions::examineCharacterSheet)
	{
		Object * const obj = CuiAction::findObjectFromFirstParam (params, true, false, id);

		if (obj)
		{
			CreatureObject *creatureTarget = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(obj->getNetworkId()));
			if (creatureTarget && creatureTarget->isPlayer())
			{
				m_mediator.m_windowManager->spawnCharacterSheet (creatureTarget);
			}
		}
	}
	else if (id == CuiActions::terminalAuctionUse)
	{
		Object * const obj = CuiAction::findObjectFromFirstParam (params, true, true, id);

		if (obj)
		{
			// make sure object is a bazaar terminal
			ClientObject * const clientObject = obj->asClientObject();
			if (clientObject)
			{
				if (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_terminal_bazaar)
				{
					CuiAuctionManager::handleStartAuction (obj->getNetworkId (), CuiMediatorTypes::WS_AuctionList);
				}
				else
				{
					Unicode::String errorString;
					CuiStringVariablesManager::process(CuiStringIdsAuction::err_not_bazaar, Unicode::emptyString, clientObject->getLocalizedName(), Unicode::emptyString, errorString);
					CuiSystemMessageManager::sendFakeSystemMessage(errorString);
				}
			}
		}
	}
	else if (id == CuiActions::vendorUse)
	{
		Object * const obj = CuiAction::findObjectFromFirstParam (params, true, true, id);

		if (obj)
		{
			// make sure object is a vendor
			ClientObject * const clientObject = obj->asClientObject();
			if (clientObject)
			{
				bool isVendor = false;
				if (clientObject->getGameObjectType() == SharedObjectTemplate::GOT_vendor)
				{
					isVendor = true;
				}
				else
				{
					TangibleObject * const tangible = clientObject->asTangibleObject();
					if (tangible && tangible->hasCondition(TangibleObject::C_vendor))
						isVendor = true;
				}

				if (isVendor)
				{
					CuiAuctionManager::handleStartVendor (obj->getNetworkId (), CuiMediatorTypes::WS_Vendor);
				}
				else
				{
					Unicode::String errorString;
					CuiStringVariablesManager::process(CuiStringIdsAuction::err_not_vendor, Unicode::emptyString, clientObject->getLocalizedName(), Unicode::emptyString, errorString);
					CuiSystemMessageManager::sendFakeSystemMessage(errorString);
				}
			}
		}
	}
	else if (id == CuiActions::debugPrint)
	{
		CuiMediatorFactory::toggle (CuiMediatorTypes::DebugInfoPage);
	}

	else if (id == CuiActions::community)
		m_mediator.m_windowManager->toggleCommunity();
	
	//access the skills ui
	else if (id == CuiActions::skills)
	{
		if(RoadmapManager::playerIsNewCharacter())
			;  //do nothing
		else if(RoadmapManager::playerIsOnRoadmap())
			CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_Roadmap);
	}
	
	else if (id == CuiActions::netStatus)
	{
		if (params.empty ())
		{
			CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_NetStatus);
		}
		else
		{
			if (params [0] != '0')
				CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_NetStatus);
			else
				CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_NetStatus);
		}
	}

	else if (id == CuiActions::planetMap)
	{
		if (!player)
			return true;
		
		if (Game::getPlayerContainingShip() == 0)
		{
			if (!PlanetMapManagerClient::isPlanetaryMapEnabled(Game::getSceneId(), player->getPosition_w()))
				CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_PlanetMap);
			else
				CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_PlanetMap);
		}
		else
		{
			getWindowManager().spawnSpaceZoneMap(params);
		}
	}
	
	else if (id == CuiActions::gameMenuActivate)
	{
		CuiMediator const * const gameMenu = CuiMediatorFactory::activate (CuiMediatorTypes::GameMenu);
		if (gameMenu && gameMenu->isActive ())
		{
			SwgCuiHudFactory::setHudActive (false);
			CuiMediatorFactory::activate (CuiMediatorTypes::GameMenu);
		}
		else
		{
			CuiMediatorFactory::deactivate (CuiMediatorTypes::GameMenu);
			SwgCuiHudFactory::setHudActive (true);
		}
	}

	else if (id == CuiActions::harvester)
	{
		const std::string & harvesterIdStr = Unicode::wideToNarrow(params);
		NetworkId harvesterId(harvesterIdStr);
		CuiResourceManager::setHarvester (harvesterId);
		SwgCuiResourceExtraction * const extraction = safe_cast<SwgCuiResourceExtraction*>(CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_ResourceExtraction));
		if(!Game::getSinglePlayer())
			extraction->toggleShowData(false);
	}

	else if (id == CuiActions::untargetOnly)
	{
		SwgCuiHudWindowManager& hudMan = getWindowManager();
		SwgCuiHudWindowManagerGround* groundHudMan = dynamic_cast<SwgCuiHudWindowManagerGround*>(&hudMan);
		SwgCuiHudWindowManagerSpace* spaceHudMan = dynamic_cast<SwgCuiHudWindowManagerSpace*>(&hudMan);
		if (groundHudMan)
		{
			CreatureObject * const playerCreature = NON_NULL(Game::getPlayerCreature());

			playerCreature->setIntendedTarget(NetworkId::cms_invalid);
			playerCreature->setLookAtTarget(NetworkId::cms_invalid);
		}
		else if (spaceHudMan)
		{
			CreatureObject * const playerCreature = NON_NULL(Game::getPlayerCreature());
			playerCreature->setLookAtTarget (NetworkId::cms_invalid);
		}

	}
	else if (id == CuiActions::untarget)
	{
		//Logic: if there is a pending action, cancel that (and don't do the untarget business), otherwise untarget
		if(SwgCuiToolbar::getActiveToolbar())
			SwgCuiToolbar::getActiveToolbar()->clearCommandDisplayStates();
		if(CuiCombatManager::hasAnyCommandsInQueue())
			ClientCommandQueue::clear();

		if (CuiRadialMenuManager::isStorytellerMode() || CuiRadialMenuManager::isGrenadeTargetMode())
		{
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIds::groundplace_abort.localize());
			CuiRadialMenuManager::setStorytellerMode(false);
			CuiRadialMenuManager::setGrenadeTargetMode(false);
			return false;
		}

		SwgCuiHudWindowManager& hudMan = getWindowManager();
		SwgCuiHudWindowManagerGround* groundHudMan = dynamic_cast<SwgCuiHudWindowManagerGround*>(&hudMan);
		SwgCuiHudWindowManagerSpace* spaceHudMan = dynamic_cast<SwgCuiHudWindowManagerSpace*>(&hudMan);

		if (groundHudMan)
		{
			if(hudMan.isButtonBarVisible())
			{
				hudMan.toggleButtonBar();
			}
			else
			{
				CreatureObject * const playerCreature = NON_NULL(Game::getPlayerCreature());

				// Even if there was a target set, we wanna toggle the button bar if the object didn't exist on the client and wasn't visible...
				ClientObject const * const clientTarget = safe_cast<ClientObject const *>(playerCreature->getIntendedTarget().getObject());
				if (!clientTarget)
					hudMan.toggleButtonBar();

				playerCreature->setIntendedTarget(NetworkId::cms_invalid);
				playerCreature->setLookAtTarget(NetworkId::cms_invalid);
			}
		}
		else if (spaceHudMan)
		{
			if(hudMan.isButtonBarVisible())
			{
				hudMan.toggleButtonBar();
			}
			else
			{
				CreatureObject * const playerCreature = NON_NULL(Game::getPlayerCreature());
				ClientObject const * const clientTarget = safe_cast<ClientObject const *>(playerCreature->getLookAtTarget().getObject());
				if( clientTarget )
					playerCreature->setLookAtTarget (NetworkId::cms_invalid);
				else
					hudMan.toggleButtonBar();	
			}
			
		}
		else
		{
			hudMan.toggleButtonBar();
		}
	}

	else if(id == CuiActions::clientExamine)
	{	
		if(PlayerObject::isAdmin())
		{		
			CreatureObject * const playerCreature = Game::getPlayerCreature();
			SwgCuiHud const * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
			Object *lookAt = hud != 0 ? hud->getLastSelectedObject() : 0;
			if(!lookAt)
			{
				lookAt = playerCreature->getLookAtTarget().getObject();
			}
			if(lookAt)
			{
				char buffer[512];
				snprintf(buffer, sizeof(buffer) - 1, "client object has appearance template [%s]", 			
					(lookAt->getAppearance() && lookAt->getAppearance()->getAppearanceTemplateName()) ? lookAt->getAppearance()->getAppearanceTemplateName() : "(null)");
				buffer[sizeof(buffer) - 1] = '\0';
				CuiChatRoomManager::sendPrelocalizedChat(Unicode::narrowToWide(buffer));
			}
		}
	}

	else if (id == SwgCuiActions::toggleButtonBar)
	{
		getWindowManager().toggleButtonBar();
	}
	
	else if (id == CuiActions::radialMenu)
	{
		m_mediator.summonRadialMenu(params);
	}
	
	else if (id == CuiActions::defaultAction)
	{
		SwgCuiHud const * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
		if (hud != 0)
		{
			Object * selected = hud->getLastSelectedObject();

			if (selected == 0)
			{
				selected = CuiAction::findObjectFromFirstParam (params, true, false, id);
			}

			if (selected != 0)
			{
				CuiRadialMenuManager::performDefaultAction(*selected);
			}
		}
	}

	else if (id == CuiActions::options)
	{
		CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_Opt);
	}

	else if (id == CuiActions::targetAtCursor)
	{
		m_mediator.targetAtCursor (true);
	}	

	else if (id == CuiActions::targetAtCursorStop)
	{
		m_mediator.targetAtCursor (false);
	}

	else if (id == CuiActions::setIntendedTarget)
	{
		if(player && (player->getLookAtTarget() != NetworkId::cms_invalid))
		{
			player->setIntendedTarget(player->getLookAtTarget());
		}
	}

	else if (id == CuiActions::chatCopy)
	{
		UIMessage msg;
		msg.Keystroke   = UIMessage::Copy;
		msg.MouseCoords = UIManager::gUIManager ().GetLastMouseCoord ();
		msg.Type        = UIMessage::KeyDown;
		IGNORE_RETURN(NON_NULL (UIManager::gUIManager ().GetRootPage ())->ProcessMessage (msg));
	}

	else if (id == CuiActions::chatCut)
	{
		UIMessage msg;
		msg.Keystroke   = UIMessage::Cut;
		msg.MouseCoords = UIManager::gUIManager ().GetLastMouseCoord ();
		msg.Type        = UIMessage::KeyDown;
		IGNORE_RETURN(NON_NULL (UIManager::gUIManager ().GetRootPage ())->ProcessMessage (msg));
	}

	else if (id == CuiActions::chatPaste)
	{
		UIMessage msg;
		msg.Keystroke   = UIMessage::Paste;
		msg.MouseCoords = UIManager::gUIManager ().GetLastMouseCoord ();
		msg.Type        = UIMessage::KeyDown;
		IGNORE_RETURN(NON_NULL (UIManager::gUIManager ().GetRootPage ())->ProcessMessage (msg));
	}
	
	else if (id == CuiActions::console)
	{
		CuiManager::getIoWin ().getCallbackConsole ().performCallback ();
	}

	else if (id == CuiActions::askconsent)
	{
		IGNORE_RETURN(CuiConsentManager::askLocalConsent(Unicode::emptyString, 0));
	}

	else if (id == CuiActions::getSample)
	{
		CuiSurveyManager::doGetSample();
	}

	else if (id == CuiActions::expMonitor)
	{
		if (CuiPreferences::getUseExpMonitor ())
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ExpMonitorBox);
		else
			CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_ExpMonitorBox);
	}

	else if (id == CuiActions::waypointMonitor)
	{
		if (CuiPreferences::getUseWaypointMonitor ())
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_WaypointMonitorBox);
		else
			CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_WaypointMonitorBox);
	}

	else if (id == CuiActions::locationDisplay)
	{
		if (CuiPreferences::getLocationDisplayEnabled() && !Game::isHudSceneTypeSpace())
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_LocationDisplay);
		else
			CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_LocationDisplay);
	}

	else if (id == CuiActions::dpsMeter)
	{
		if (CuiPreferences::getDpsMeterEnabled() && !Game::isHudSceneTypeSpace())
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_DpsMeter);
		else
			CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_DpsMeter);
	}

	else if (id == CuiActions::questJournal)
	{
		const bool up = !params.empty () && params [0] != 0;
		if (up)
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_QuestJournal);
		else
			CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_QuestJournal);
	}

	else if (id == CuiActions::questAcceptance)
	{
		SwgCuiQuestAcceptance * const questAcceptance = safe_cast<SwgCuiQuestAcceptance *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_QuestAcceptance));
		if(questAcceptance)
		{
			std::vector<Unicode::String> result;
			IGNORE_RETURN(Unicode::tokenize(params, result));
			if(result.size() == 2)
			{
				std::string const & acceptanceWindowTypeString = Unicode::wideToNarrow(result[0]);
				std::string const & questCrcString             = Unicode::wideToNarrow(result[1]);

				SwgCuiQuestAcceptance::WindowType wt = SwgCuiQuestAcceptance::WT_requestAccept;
				if(acceptanceWindowTypeString == "0")
				{
					wt = SwgCuiQuestAcceptance::WT_requestAccept;
				}
				else if(acceptanceWindowTypeString == "1")
				{
					wt = SwgCuiQuestAcceptance::WT_requestComplete;
				}
				else if(acceptanceWindowTypeString == "2")
				{
					wt = SwgCuiQuestAcceptance::WT_forceAccept;
				}

				questAcceptance->setData(wt, atoi(questCrcString.c_str()));
			}
		}
	}

	else if (id == CuiActions::questHelper)
	{
		SwgCuiQuestHelper * const questHelper = safe_cast<SwgCuiQuestHelper *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_QuestHelper));
		if (questHelper)
		{
			questHelper->setupData();
		}
	}

	else if (id == CuiActions::roadmap)
	{
		if(!RoadmapManager::playerIsNewCharacter())
		{		
			const bool up = !params.empty () && params [0] != 0;
			if (up)
				CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Roadmap);		
			else
				CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_Roadmap);		
		}
	}

	else if (id == CuiActions::expertise)
	{
		if(ClientExpertiseManager::hasExpertiseTrees())
		{		
			const bool up = !params.empty () && params [0] != 0;
			if (up)
				CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Expertise);		
			else
				CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_Expertise);		
		}
	}

	else if (id == CuiActions::ticketPurchase)
	{
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_TicketPurchase);
	}


	else if (id == CuiActions::service) // Customer Service
	{
		if (params.length() == 0)
		{
			s_confirmCsBrowserSpawn = CuiMessageBox::createYesNoBox (CuiStringIdsCustomerService::confirm_cs_browser_spawn.localize ());
			s_callback->connect (s_confirmCsBrowserSpawn->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);
			return true;
		}

		// build the parameters.
		HttpGetEncoder::GetParams httpParams;

		std::string loginId;
		std::string clusterName;
		Unicode::String playerName;
		NetworkId id;

		if (Game::getPlayerPath(loginId, clusterName, playerName, id))
		{
			httpParams["Station_Name"] = loginId;
			httpParams["Server"] = clusterName;
			httpParams["Character"] = Unicode::wideToNarrow(playerName);
			httpParams["Character_ID"] = id.getValueString();
		}

		char buf[64];

		const ClientObject* const player = Game::getClientPlayer();
		if (player)
		{
			const Vector & world_pos = player->getPosition_w();
			sprintf(buf, "%.02f %.02f %.02f", world_pos.x, world_pos.y, world_pos.z);
			httpParams["LOC"] = buf;

			const ObjectTemplate* const t = player->getObjectTemplate();
			const SharedCreatureObjectTemplate* const c_t = dynamic_cast<const SharedCreatureObjectTemplate*>(t);
			if (c_t)
			{
				const SharedCreatureObjectTemplate::Species species = c_t->getSpecies();
				const SharedCreatureObjectTemplate::Gender gender = c_t->getGender();
				const std::string speciesString = Unicode::wideToNarrow(Species::getLocalizedName(species));

				std::string genderString;

				switch (gender)
				{
				case SharedCreatureObjectTemplate::GE_male:   { genderString = "Male"; } break;
				case SharedCreatureObjectTemplate::GE_female: { genderString = "Female"; } break;
				case SharedCreatureObjectTemplate::GE_other:  { genderString = "Other"; } break;
				default:                                      { genderString = "Unknown gender"; } break;
				}

				httpParams["Race"] = speciesString;
				httpParams["Gender"] = genderString;
			}

			const CreatureObject * playerAsCreature = player->asCreatureObject();
			if (playerAsCreature)
			{
				sprintf(buf, "%u", playerAsCreature->getLevel());
				httpParams["Character_Level"] = buf;
			}		
		}

		sprintf(buf, "%lu", GameNetwork::getStationId());
		httpParams["Station_ID"] = buf;
		httpParams["Game"] = "Star Wars Galaxies";

		httpParams["Zone"] = Game::getSceneId();

		const char * session_Id = CuiLoginManager::getSessionIdKey();
		if (session_Id != 0)
			httpParams["sessionId"] =  session_Id;
// Code disabled for now until CS Web wants the chat log passed in via a URL
/*		const SwgCuiChatWindow * const chatWindow = getWindowManager().getChatWindow();
		if (chatWindow)
		{
			const SwgCuiChatWindow::Tab * const tab = chatWindow->getActiveTab();
			if (tab)
			{
				const Unicode::String text = tab->getText();
				std::string narrow = Unicode::wideToNarrow(text);
				if (narrow.length() > 1024)
					narrow = narrow.substr(narrow.length() - 1024, 1024);
				httpParams["Charchat"] = narrow;
			}
			else
			{
				httpParams["Charchat"] = "";
			}
		}
		else
		{*/
			httpParams["Charchat"] = "";
		//}

		httpParams["Character_Class"] = RoadmapManager::getPlayerSkillTemplate();
		httpParams["Character_Client"] = ApplicationVersion::getInternalVersion();
		// create the final URL

		std::string baseUrl = ConfigClientGame::getCsTrackingBaseUrl();
		std::string finalUrl = HttpGetEncoder::getUrl(baseUrl, httpParams);

		if (finalUrl.length() > 2048)
		{
			unsigned diff = finalUrl.length() - 2048;

			if (httpParams["Charchat"].length() > diff)
			{
				httpParams["Charchat"] = httpParams["Charchat"].substr(diff);
				finalUrl = HttpGetEncoder::getUrl(baseUrl, httpParams);
			}
			else
			{
				return false;
			}
		}
		//ShellExecute(NULL, "open", finalUrl.c_str(), NULL, "", SW_SHOW);
		SwgCuiWebBrowserManager::createWebBrowserPage(false);
		SwgCuiWebBrowserManager::setURL(finalUrl.c_str(), true);
	}

	else if (id == CuiActions::ticketSubmission) // Ticket Submission tabbed form
	{
		m_mediator.m_windowManager->spawnTicketSubmission();
	}

	else if (id == CuiActions::harassmentMessage) // Message displayed prior to a harrassment report
	{
		m_mediator.m_windowManager->spawnHarassmentMessage();
	}

	// Message after a 'community standards' search on kb - forwards to ticket submission form
	else if (id == CuiActions::harassmentMessageFromKBSearch)
	{
		m_mediator.m_windowManager->spawnHarassmentMessageFromKBSearch();
	}

	else if (id == CuiActions::defaultButton)
	{
		UIPage * const root = UIManager::gUIManager ().GetRootPage ();
		if (root)
		{
			UIButton * const button = root->FindDefaultButton (true);
			if (button)
				button->Press ();
		}
	}

	else if (id == CuiActions::cancelButton)
	{
		UIPage * const root = UIManager::gUIManager ().GetRootPage ();
		if (root)
		{
			UIButton * const button = root->FindCancelButton (true);
			if (button)
				button->Press ();
		}
	}

	else if (id == CuiActions::bugReport)
	{
		CuiMediator* const m = CuiMediatorFactory::get (CuiMediatorTypes::BugSubmission);
		CuiBugSubmissionPage* const p = dynamic_cast<CuiBugSubmissionPage*>(m);
		if(!p)
			return false;

		p->addLineToBody(CustomerServiceManager::getPlayerInformation());
		p->activate();
	}

	else if (id == CuiActions::newMacro)
		m_mediator.m_windowManager->spawnNewMacro (params);

	else if (id == CuiActions::imageDesignerDesigner)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() == 3)
		{
			std::string const & recipientIdStr = Unicode::wideToNarrow(result[0]);
			std::string const & terminalIdStr = Unicode::wideToNarrow(result[1]);
			std::string const & currentHoloEmote = Unicode::wideToNarrow(result[2]);
			NetworkId const recipientId(recipientIdStr);
			NetworkId const terminalId(terminalIdStr);
			SwgCuiImageDesignerDesigner * const imageDesignerDesigner = safe_cast<SwgCuiImageDesignerDesigner *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ImageDesignerDesigner));
			if(imageDesignerDesigner)
			{
				imageDesignerDesigner->setTerminal(terminalId);
				if(currentHoloEmote != "none")
					imageDesignerDesigner->setCurrentHoloemote(currentHoloEmote);
				imageDesignerDesigner->setRecipient(recipientId);
			}
		}
	}

	else if (id == CuiActions::imageDesignerRecipient)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() == 3)
		{
			std::string const & designerIdStr = Unicode::wideToNarrow(result[0]);
			std::string const & terminalIdStr = Unicode::wideToNarrow(result[1]);
			std::string const & currentHoloEmote = Unicode::wideToNarrow(result[2]);
			NetworkId const designerId(designerIdStr);
			NetworkId const terminalId(terminalIdStr);
			SwgCuiImageDesignerRecipient * const imageDesignerRecipient = safe_cast<SwgCuiImageDesignerRecipient *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ImageDesignerRecipient));
			if(imageDesignerRecipient)
			{
				imageDesignerRecipient->setTerminal(terminalId);
				if(currentHoloEmote != "none")
					imageDesignerRecipient->setCurrentHoloemote(currentHoloEmote);
				imageDesignerRecipient->setDesigner(designerId);
			}
		}
	}

	else if (id == CuiActions::openBuffBuilderBufferWindow)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() == 1)
		{
			std::string const & recipientIdStr = Unicode::wideToNarrow(result[0]);
			NetworkId const recipientId(recipientIdStr);
			
			SwgCuiBuffBuilderBuffer * const buffBuilderBuffer = safe_cast<SwgCuiBuffBuilderBuffer *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_BuffBuilderBuffer));
			if(buffBuilderBuffer)
			{
				buffBuilderBuffer->setRecipient(recipientId);
			}
		}
	}

	else if (id == CuiActions::openBuffBuilderBuffeeWindow)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() == 1)
		{
			std::string const & bufferIdStr = Unicode::wideToNarrow(result[0]);
			NetworkId const bufferId(bufferIdStr);
			SwgCuiBuffBuilderBuffee * const buffBuilderBuffee = safe_cast<SwgCuiBuffBuilderBuffee *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_BuffBuilderBuffee));
			if(buffBuilderBuffee)
			{
				buffBuilderBuffee->setBufferId(bufferId);
			}
		}
	}

	else if (id == CuiActions::openIncubatorWindow)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		
		if(result.size() == 14)
		{
			SwgCuiIncubator * const incubatorWindow = safe_cast<SwgCuiIncubator *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Incubator));
			if(incubatorWindow)
			{
				const int sessionNumber = atoi(Unicode::wideToNarrow(result[0]).c_str());
				const NetworkId playerId(Unicode::wideToNarrow(result[1]));
				const NetworkId terminalId(Unicode::wideToNarrow(result[2]));
				const int powerGauge = atoi(Unicode::wideToNarrow(result[3]).c_str());
				const int initialPointsSurvival = atoi(Unicode::wideToNarrow(result[4]).c_str());
				const int initialPointsBeastialResilience = atoi(Unicode::wideToNarrow(result[5]).c_str());
				const int initialPointsCunning = atoi(Unicode::wideToNarrow(result[6]).c_str());
				const int initialPointsIntelligence = atoi(Unicode::wideToNarrow(result[7]).c_str());
				const int initialPointsAggression = atoi(Unicode::wideToNarrow(result[8]).c_str());
				const int initialPointsHuntersInstinct = atoi(Unicode::wideToNarrow(result[9]).c_str());
				const int temperatureGauge = atoi(Unicode::wideToNarrow(result[10]).c_str());
				const int nutrientGauge = atoi(Unicode::wideToNarrow(result[11]).c_str());
				const int initialCreatureColorIndex = atoi(Unicode::wideToNarrow(result[12]).c_str());
				const std::string creatureTemplateName(Unicode::wideToNarrow(result[13]));

				incubatorWindow->SetSessionNumber(sessionNumber);
				UNREF(playerId); // not needed
				incubatorWindow->SetTerminalId(terminalId);
				incubatorWindow->SetPowerGaugeValue(powerGauge);
				incubatorWindow->SetInitialPointsSurvival(initialPointsSurvival);
				incubatorWindow->SetInitialPointsBeastialResilience(initialPointsBeastialResilience);
				incubatorWindow->SetInitialPointsCunning(initialPointsCunning);
				incubatorWindow->SetInitialPointsIntelligence(initialPointsIntelligence);
				incubatorWindow->SetInitialPointsAggression(initialPointsAggression);
				incubatorWindow->SetInitialPointsHuntersInstinct(initialPointsHuntersInstinct);
				incubatorWindow->SetTemperatureGaugeValue(temperatureGauge);
				incubatorWindow->SetNutrientGaugeValue(nutrientGauge);
				incubatorWindow->SetInitialCreatureColorIndex(initialCreatureColorIndex);
				incubatorWindow->SetCreatureTemplateName(creatureTemplateName);

				incubatorWindow->initializeControls();
			}
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Not opening incubator - SwgCuiActionHud ... id == CuiActions::openIncubatorWindow result.size() = %d\n",result.size()));
		}
	}

	else if (id == CuiActions::manageShipComponents)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() >= 1)
		{
			std::string const & shipIdStr = Unicode::wideToNarrow(result[0]);
			NetworkId const shipId(shipIdStr);
			std::string terminalIdStr;
			if(result.size() >= 2)
			{
				terminalIdStr = Unicode::wideToNarrow(result[1]);
			}
			NetworkId const terminalId(terminalIdStr);
			getWindowManager().spawnShipComponentManagement(shipId, terminalId);
		}
	}

	else if (id == CuiActions::openCustomizationWindow)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() >= 1)
		{
			std::string const & objectIdStr = Unicode::wideToNarrow(result[0]);
			NetworkId const objectId(objectIdStr);
			if(result.size() < 2)
				DEBUG_WARNING(true, ("got CuiAction openCustomizationWindow without any parameters; doing nothing"));
			else if(result.size () < 5)			
				getWindowManager().spawnCustomizationWindow(objectId, 
					Unicode::wideToNarrow(result[1]), Unicode::toInt(result[2]), Unicode::toInt(result[3]),
					std::string(""), -1, -1,
					std::string(""), -1, -1,
					std::string(""), -1, -1);
			else if(result.size() < 8)
				getWindowManager().spawnCustomizationWindow(objectId, 
					Unicode::wideToNarrow(result[1]), Unicode::toInt(result[2]), Unicode::toInt(result[3]),
					Unicode::wideToNarrow(result[4]), Unicode::toInt(result[5]), Unicode::toInt(result[6]),
					std::string(""), -1, -1,
					std::string(""), -1, -1); 
			else if(result.size() < 11)
				getWindowManager().spawnCustomizationWindow(objectId, 
					Unicode::wideToNarrow(result[1]), Unicode::toInt(result[2]), Unicode::toInt(result[3]),
					Unicode::wideToNarrow(result[4]), Unicode::toInt(result[5]), Unicode::toInt(result[6]),
					Unicode::wideToNarrow(result[7]), Unicode::toInt(result[8]), Unicode::toInt(result[9]),
					std::string(""), -1, -1); 
			else
				getWindowManager().spawnCustomizationWindow(objectId, 
				Unicode::wideToNarrow(result[1]), Unicode::toInt(result[2]), Unicode::toInt(result[3]),
				Unicode::wideToNarrow(result[4]), Unicode::toInt(result[5]), Unicode::toInt(result[6]),
				Unicode::wideToNarrow(result[7]), Unicode::toInt(result[8]), Unicode::toInt(result[9]),
				Unicode::wideToNarrow(result[10]), Unicode::toInt(result[11]), Unicode::toInt(result[12]));
		}
	}

	//used in ground for decimator map
	else if (id == CuiActions::spaceZoneMap)
	{
		if (Game::isSpace())
		{
			getWindowManager().spawnSpaceZoneMap(params);
		}
	}

	else if (id == CuiActions::editBiography)
	{
		SwgCuiBiographyEdit * const bioEdit = dynamic_cast<SwgCuiBiographyEdit *>(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_BiographyEdit));
		if (bioEdit) 
		{
			CachedNetworkId const playerId(Unicode::wideToNarrow(params));
			bioEdit->requestBiography(playerId);
		}
	}

	else if (id == CuiActions::commPlayer)
	{
		size_t pos = 0;

		//
		Unicode::String netIdString;
		IGNORE_RETURN(Unicode::getFirstToken(params, 0, pos, netIdString));
		NetworkId const taunterId(Unicode::wideToNarrow(netIdString));

		//
		Unicode::String appearanceOverrideSharedTemplateCrcWideString;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, appearanceOverrideSharedTemplateCrcWideString));
		uint32 const appearanceOverrideSharedTemplateCrc = static_cast<uint32>(atoi(Unicode::wideToNarrow(appearanceOverrideSharedTemplateCrcWideString).c_str()));

		//
		Unicode::String soundEffectParam;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, soundEffectParam));
		std::string soundEffect = Unicode::wideToNarrow(soundEffectParam);

		// 
		Unicode::String durationParam;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, durationParam));
		float const duration = static_cast<float>(atof(Unicode::wideToNarrow(durationParam).c_str()));

		//
		Unicode::String const & text = params.substr(pos+1);

		SwgCuiSpaceConversation * const convo = safe_cast<SwgCuiSpaceConversation *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_SpaceConversation, false));
		if(convo)
		{
			convo->setTauntInfo(taunterId, text, appearanceOverrideSharedTemplateCrc, soundEffect, duration);
		}
	}

	else if (id == CuiActions::lootBox)
	{
		SwgCuiLootBox * const lootBox = safe_cast<SwgCuiLootBox *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_LootBox, false));
		if(lootBox)
		{
			lootBox->clearRewards();
			std::vector<Unicode::String> result;
			Unicode::tokenize(params, result);
			for(std::vector<Unicode::String>::const_iterator i = result.begin(); i < result.end(); ++i)
			{
				std::string const & idStr = Unicode::wideToNarrow(*i);
				NetworkId const nid(idStr);
				lootBox->addReward(nid);
			}
		}
		else
		{
			DEBUG_FATAL(true, ("No loot box!"));
		}
	}

	else if (id == CuiActions::npeContinuation)
	{
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_NpeContinuation);
	}
	else if (id == CuiActions::blueGlowieOn)
	{
		if(PlayerObject::isAdmin() || Game::getSinglePlayer())
		{		
			Object *obj = Game::getPlayer();
			Appearance *app = obj->getAppearance();
			SkeletalAppearance2 *skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
			if(skelApp)
				skelApp->setIsBlueGlowie(true);
		}
	}
	else if (id == CuiActions::blueGlowieOff)
	{
		if(PlayerObject::isAdmin() || Game::getSinglePlayer())
		{
			Object *obj = Game::getPlayer();
			Appearance *app = obj->getAppearance();
			SkeletalAppearance2 *skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
			if(skelApp)
				skelApp->setIsBlueGlowie(false);
		}
	}
	else if (id == CuiActions::holonetOn)
	{
		if(PlayerObject::isAdmin() || Game::getSinglePlayer())
		{		
			Object *obj = Game::getPlayer();
			Appearance *app = obj->getAppearance();
			SkeletalAppearance2 *skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
			if(skelApp)
				skelApp->setIsHolonet(true);
		}
	}
	else if (id == CuiActions::holonetOff)
	{
		if(PlayerObject::isAdmin() || Game::getSinglePlayer())
		{
			Object *obj = Game::getPlayer();
			Appearance *app = obj->getAppearance();
			SkeletalAppearance2 *skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
			if(skelApp)
				skelApp->setIsHolonet(false);
		}
	}
	else if (id == CuiActions::setIntendedAndSummonRadialMenu)
	{
		if (params == Unicode::narrowToWide("down"))
			m_mediator.setIntendedAndSummonRadialMenu(false);
		if (params == Unicode::narrowToWide("up"))
			m_mediator.setIntendedAndSummonRadialMenu(true);
		if (params == Unicode::narrowToWide("cancel"))
			m_mediator.setIntendedAndSummonRadialMenu(true, true);
	}
	else if (id == CuiActions::collections)
	{
		Object * const obj = CuiAction::findObjectFromFirstParam(params, true, false, id);
		CreatureObject * const creatureObj = dynamic_cast<CreatureObject *>(obj);

		if (creatureObj && creatureObj->isPlayer())
		{
			SwgCuiCollections * const collectionsMediator = safe_cast<SwgCuiCollections * >(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_Collections, false, false));
			NOT_NULL(collectionsMediator);
			collectionsMediator->setTarget(creatureObj);
		}
	}

	else if (id == CuiActions::myCollections)
	{
		SwgCuiCollections * collectionsMediator = safe_cast<SwgCuiCollections * >(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_Collections, false, false, false));

		if (collectionsMediator && collectionsMediator->isActive())
		{
				CuiMediatorFactory::deactivateInWorkspace(CuiMediatorTypes::WS_Collections);
		}
		else
		{
			collectionsMediator = safe_cast<SwgCuiCollections * >(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_Collections, false, false));

			if (collectionsMediator)
				collectionsMediator->setTarget(player);
		}
	}
	else if (id == CuiActions::sendSavedPlayerInterestsToServer)
	{
		SwgCuiLfg::sendSavedInterests();
	}

	else if (id == SwgCuiActions::toggleVoiceFlyBar)
	{
		CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_VoiceFlyBar);
	}

	else if (id == SwgCuiActions::toggleVoiceActiveSpeakers)
	{
		CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_VoiceActiveSpeakers);
	}
	else if (id == CuiActions::tcg)
	{
		SwgCuiTcgWindow * tcgWindow = safe_cast<SwgCuiTcgWindow * >(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_TcgWindow, false, false, false));

		if (tcgWindow && tcgWindow->isActive())
		{
			CuiMediatorFactory::deactivateInWorkspace(CuiMediatorTypes::WS_TcgWindow);
		}
		else
		{
			SwgCuiTcgManager::setLoginInfo(GameNetwork::getUserName().c_str(), CuiLoginManager::getSessionIdKey(true));
			SwgCuiTcgManager::launch();

			tcgWindow = safe_cast<SwgCuiTcgWindow * >(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_TcgWindow, false, false));
		}
	}
	else if (id == CuiActions::appearanceTab)
	{
		CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_AppearanceTab);
	}
	else if (id == CuiActions::questBuilder)
	{
		SwgCuiQuestBuilder * questWindow = safe_cast<SwgCuiQuestBuilder * >(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_QuestBuilder, false, false, false));

		if (questWindow && questWindow->isActive())
		{
			if(params.length() == 0 && params.empty())
			{
				StringId confirmQuestBuilder("ui_quest", "confirm_open");
				s_confirmOpenQuestBuilder = CuiMessageBox::createYesNoBox (confirmQuestBuilder.localize());
				s_callback->connect (s_confirmOpenQuestBuilder->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);

				return true;
			}

			CuiMediatorFactory::deactivateInWorkspace(CuiMediatorTypes::WS_QuestBuilder);
		}

		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_QuestBuilder);
	}
	else if (id == CuiActions::rating)
	{
		std::string title, desc;
		std::string narrowParams = Unicode::wideToNarrow(params);
		std::string::size_type parsePoint = narrowParams.find('|');
		if(parsePoint != std::string::npos)
		{
			title = narrowParams.substr(0, parsePoint);
			desc = narrowParams.substr(parsePoint + 1, strlen(narrowParams.c_str()) - (parsePoint + 1));
		}

		getWindowManager().spawnRatingWindow(title, desc);
	}
	else if (id == CuiActions::commPlayerQuest)
	{
		size_t pos = 0;

		//
		Unicode::String netIdString;
		IGNORE_RETURN(Unicode::getFirstToken(params, 0, pos, netIdString));
		NetworkId const taunterId(Unicode::wideToNarrow(netIdString));

		//
		Unicode::String appearanceOverrideSharedTemplateCrcWideString;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, appearanceOverrideSharedTemplateCrcWideString));
		uint32 const appearanceOverrideSharedTemplateCrc = static_cast<uint32>(atoi(Unicode::wideToNarrow(appearanceOverrideSharedTemplateCrcWideString).c_str()));

		//
		Unicode::String soundEffectParam;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, soundEffectParam));
		std::string soundEffect = Unicode::wideToNarrow(soundEffectParam);

		// 
		Unicode::String durationParam;
		IGNORE_RETURN(Unicode::getFirstToken(params, pos, pos, durationParam));
		float const duration = static_cast<float>(atof(Unicode::wideToNarrow(durationParam).c_str()));

		//
		Unicode::String const & text = params.substr(pos+1);

		SwgCuiPlayerQuestConversation * const convo = safe_cast<SwgCuiPlayerQuestConversation *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_PlayerQuestConversation, false));
		if(convo)
		{
			convo->setTauntInfo(taunterId, text, appearanceOverrideSharedTemplateCrc, soundEffect, duration);
		}
	}
	else if (id == CuiActions::gcwInfo)
	{
		CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_GalacticCivilWar);
	}

	//unknown action
	else
		return false;
	
	return true;
}

//----------------------------------------------------------------------

SwgCuiHudWindowManager & SwgCuiHudAction::getWindowManager () const
{
	return *NON_NULL(m_mediator.m_windowManager);
}

//======================================================================
