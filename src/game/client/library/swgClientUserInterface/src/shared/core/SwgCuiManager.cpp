// ======================================================================
//
// SwgCuiManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiManager.h"

#include "clientAudio/Audio.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/ChatLogManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ShipStation.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiInventoryState.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsCommand.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Production.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/ClientMfdStatusUpdateMessage.h"
#include "sharedObject/Object.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiCharacterSheet.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiHudWindowManager.h"
#include "swgClientUserInterface/SwgCuiLoadingGround.h"
#include "swgClientUserInterface/SwgCuiLoadingSpace.h"
#include "swgClientUserInterface/SwgCuiMediatorFactorySetup.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"
#include "swgClientUserInterface/SwgCuiServerData.h"
#include "swgClientUserInterface/SwgCuiStatusGround.h"
#include "swgClientUserInterface/SwgCuiTcgManager.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"

// ======================================================================

namespace SwgCuiManagerNamespace
{
	namespace UnnamedMessages
	{
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}

	bool s_checkHud = false;
	bool s_playerLoginNotificationSent = false;
	bool s_installed;
	///are we currently handling a hud type change, see SwgCuiManager::update
	bool s_handlingHudTypeChange = false;
	///are we currently showing the message of the day?
	bool s_showingMotd = false;
	///are we showing an "empty" page, i.e. a loading page before we've received the new planet message (i.e. "Connecting to Server")?
	bool s_showingEmptyPage = false;
	///are the client tools configured to load the hud
	bool s_hudLoadedInClientTools = false;


	void showLoadingScreen()
	{
		if(CuiLoadingManager::isLoadingScreenVisible())
			return;

		StringId textId;
		StringId titleId;
		std::string picture;

		if (!Game::getSceneId().empty())
			CuiLoadingManager::setPlanetName(Game::getSceneId());

		std::string const & planetName = CuiLoadingManager::getPlanetName();

		std::string const & motdTable = ConfigClientUserInterface::getMessageOfTheDayTable();

		//show motd for the first loading screen
		if (s_showingMotd || (CuiLoadingManager::isInitialLoadingScreen() && !motdTable.empty()))
		{
			textId  = StringId(motdTable, "text");
			titleId = StringId(motdTable, "title");
			picture = Unicode::wideToNarrow(StringId(motdTable, "image").localize());

			if (!picture.empty())
			{
				if (!TreeFile::exists(picture.c_str()))
				{
					WARNING(true, ("MOTD picture [%s] does not exist (fix in %s)", picture.c_str(), motdTable.c_str()));
				}
			}

			s_showingMotd = true;
			SwgCuiLoadingSpace * const loadingSpace = dynamic_cast<SwgCuiLoadingSpace*>(CuiMediatorFactory::get (CuiMediatorTypes::LoadingSpace));
			if(loadingSpace)
			{
				if(!CuiLoadingManager::firstLoadingScreenHasBeenShown() || s_showingEmptyPage)
				{
					loadingSpace->setupPage(planetName, textId, titleId, picture);
				}
			}
			IGNORE_RETURN(CuiMediatorFactory::activate (CuiMediatorTypes::LoadingSpace));
			CuiLoadingManager::setInitialLoadingScreenShown();
		}
		else
		{
			IGNORE_RETURN (CuiLoadingManager::getLoadingData(textId, titleId, picture));

			char const * const loadingMediatorType = CuiLoadingManager::getLoadingTypeMediator();

			CuiMediator * const loading = CuiMediatorFactory::get (loadingMediatorType);

			SwgCuiLoadingSpace * const loadingSpace = dynamic_cast<SwgCuiLoadingSpace*>(loading);
			if(loadingSpace)
			{
				if(!CuiLoadingManager::firstLoadingScreenHasBeenShown() || s_showingEmptyPage)
				{
					loadingSpace->setupPage(planetName, textId, titleId, picture);
				}
			}
			else
			{
				SwgCuiLoadingGround * const loadingGround = dynamic_cast<SwgCuiLoadingGround*>(loading);
				if(loadingGround)
				{
					if(!CuiLoadingManager::firstLoadingScreenHasBeenShown() || s_showingEmptyPage)
					{
						loadingGround->setupPage(planetName, textId, titleId, picture);
					}
				}
			}

			IGNORE_RETURN(CuiMediatorFactory::activate (loadingMediatorType));		
		}

		s_showingEmptyPage = planetName.empty();
	}


	bool maybeShowLoadingScreen()
	{
		//don't show another loading screen if we're currently handling a hudtype change (which generates a scene change), see SwgCuiManager::update
		if(s_handlingHudTypeChange)
		{
			return false;
		}

		//don't show a loading screen if we're in the process of destructing a scene (i.e. logging out)
		if(Game::getScene() == NULL)
		{
			return false;
		}

		//don't do anything if we're already showing a loading screen
		if(CuiLoadingManager::isLoadingScreenVisible() && !s_showingEmptyPage)
		{
			return false;
		}

		return true;
	}


	void hideLoadingScreen()
	{
		if(!CuiLoadingManager::isLoadingScreenVisible())
			return;

		//if showing the message of the day, always close the space loading screen (the one used for motd)
		if(s_showingMotd)
		{
			CuiMediatorFactory::deactivate (CuiMediatorTypes::LoadingSpace);
			s_showingMotd = false;
		}
		else
		{	
			//otherwise, close the loading screen that matches the scene we're loading into
			char const * const loadingMediatorType = CuiLoadingManager::getLoadingTypeMediator();
			CuiMediatorFactory::deactivate (loadingMediatorType);
		}

		//clear out the current planet, so the next load starts cleanly
		CuiLoadingManager::setPlanetName("");
	}


	bool shouldToggleMousePointer()
	{
		CreatureObject const * const player = Game::getPlayerCreature();

		bool sendToggle = true;

		if (!player 
			|| player->getShipStation() != ShipStation::ShipStation_None
			|| player->getPilotedShip() != 0)
		{
			sendToggle = false;
		}
	
		return sendToggle;
	}

	class Listener : public MessageDispatch::Receiver
	{
	public:
		virtual void receiveMessage (const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------

			if (message.isType (UnnamedMessages::ConnectionServerConnectionClosed))
			{
				s_playerLoginNotificationSent = false;

				//-- chat data is sometimes sent down from the connection server, not the game server
				//-- the clientside data cache for chat data must be cleared here
				CuiChatRoomManager::reset                 ();
				CuiPersistentMessageManager::clear        ();

				if (Game::getScene () && !Game::isSceneLoading ())
				{
					CuiMediatorFactory::activate (CuiMediatorTypes::ServerDisconnected);
					Game::startExitTimer ();
				}
				/*
				else
				{
					GameNetwork::setAcceptSceneCommand      (false);
					CuiMediatorFactory::deactivateAll ();
					CuiManager::raiseToTop (); 
					CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop);
					CuiMediatorFactory::activate (CuiMediatorTypes::LoginScreen);
				}
				*/
			}

			//----------------------------------------------------------------------
			
			else if (message.isType (Game::Messages::SCENE_CHANGED))
			{
				Scene * const scene = Game::getScene ();
				CuiMediator * const console = CuiMediatorFactory::get (CuiMediatorTypes::Console, false);
				const bool consoleWasActive = console && console->isActive ();

				CuiIconManager::setActiveStyleMap(static_cast<int>(Game::getHudSceneType()));

				// Close any current screens.  The first loading screen is deactivated here.
				CuiMediatorFactory::deactivateAll ();

				CuiManager::raiseToTop ();

				CuiInventoryState::saveInventoryState ();

				CuiMediatorFactory::deactivate (CuiMediatorTypes::ServerDisconnected);

				CuiObjectTextManager::reset ();

				if (!scene)
				{
					ResourceIconManager::reset ();
					CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop);

					if (Game::getSinglePlayer ())
						CuiMediatorFactory::activate (CuiMediatorTypes::SceneSelection);
					else
						CuiMediatorFactory::activate (CuiMediatorTypes::AvatarSelection);

					CuiManager::restartMusic ();
				}
				else
				{
					if (!Game::playerIsLastPlayer ())
					{
						DraftSchematicManager::reset              ();
						ClientRegionManager::clearLastRegion      ();
						CurrentUserOptionManager::save            ();

						// This option might not exist for the next character and it's crucial that we don't
						// re-use the value from the previous character, so reset to 0 before load.
                        CuiPreferences::setUiSettingsVersion(0);

						std::string     loginId;
						std::string     clusterName;
						Unicode::String playerName;
						NetworkId       playerId;
						if (Game::getPlayerPath      (loginId, clusterName, playerName, playerId))
						{
							const std::string filename = std::string ("profiles/") + loginId + "/" + clusterName + "/" + playerId.getValueString () + ".opt";
							CurrentUserOptionManager::load (filename.c_str ());
						}
						
						CustomerServiceManager::gameStart();
						CommunityManager::gameStart();
						ChatLogManager::gameStart();
						ShipWeaponGroupManager::reset();
						SwgCuiHudWindowManager::setSpaceChatVisible(true);
					}
					
					CuiActionManager::resetActionMapIndex();
					SwgCuiHudFactory::releaseHudIfNeeded();

					CuiInventoryManager::reset                     ();
					AwayFromKeyBoardManager::resetAwayFromKeyBoard ();
					ClientTextManager::loadUserSettings();
					MatchMakingManager::loadUserSettings();
					
					PerformanceTimer performanceTimer;
					performanceTimer.start();
					float waitTimer = 0.0f;
					
					CuiManager::stopMusic                 (1.0f);
					SwgCuiAvatarCreationHelper::stopMusic (1.0f, false);
					
					//-- only use this artificial loop when switching scenes, but not hud types

					if (!s_handlingHudTypeChange)
					{
						while (waitTimer < 1.0f)
						{
							performanceTimer.stop();
							float const deltaTime = performanceTimer.getElapsedTime();
							performanceTimer.start();
							
							waitTimer += deltaTime;
							Audio::alter(deltaTime, NULL);
							Sleep(5);
						}
					}
					else if(shouldToggleMousePointer())
					{
						CuiManager::setPointerToggledOn(CuiPreferences::getMouseModeDefault());
					}

					s_checkHud = true;
				}

				if (maybeShowLoadingScreen())
				{
					showLoadingScreen();
				}

				if (consoleWasActive)
				{
					CuiMediatorFactory::activate (CuiMediatorTypes::Console);
				}

				CuiManager::getIoWin ().resetInputMaps ();

				SwgCuiHudFactory::updateShaders();
			}

			//----------------------------------------------------------------------

			else if (message.isType (CuiLoadingManager::Messages::FullscreenLoadingEnabled))
			{
				showLoadingScreen();
			}

			//----------------------------------------------------------------------

			else if (message.isType (CuiLoadingManager::Messages::FullscreenLoadingDisabled))
			{
				hideLoadingScreen();

				if (maybeShowLoadingScreen())
				{
					if(shouldToggleMousePointer())
					{
						CuiManager::setPointerToggledOn(CuiPreferences::getMouseModeDefault());
					}
				}
			}

			//----------------------------------------------------------------------
			
			else if (message.isType(ClientMfdStatusUpdateMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				ClientMfdStatusUpdateMessage mfdUpdate(ri);

				SwgCuiStatusGround::deliverClientMfdStatusUpdateMessage(mfdUpdate);
			}
		}

		//----------------------------------------------------------------------

	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (UnnamedMessages::ConnectionServerConnectionClosed);
			connectToMessage (Game::Messages::SCENE_CHANGED);
			connectToMessage (CuiLoadingManager::Messages::FullscreenLoadingEnabled);
			connectToMessage (CuiLoadingManager::Messages::FullscreenLoadingDisabled);
			connectToMessage (ClientMfdStatusUpdateMessage::MESSAGE_TYPE);
		}

	};
	
#if PRODUCTION
	bool s_allowConsoleToggle = false;
#else
	bool s_allowConsoleToggle = true;
#endif

	class CallbackConsole : public CallbackReceiver
	{
	public:
		void performCallback ()
		{
			if (s_allowConsoleToggle)
			{
				CuiMediator * const console = CuiMediatorFactory::get (CuiMediatorTypes::Console);
				if (console)
				{
					if (console->isActive ())
						console->deactivate ();
					else
						console->activate ();
				}
			}
		}
	};

	CallbackConsole s_callbackConsole;
	Listener * s_receiver;
}

using namespace SwgCuiManagerNamespace;

//-----------------------------------------------------------------

void SwgCuiManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));

	SwgCuiMediatorFactorySetup::install();
	SwgCuiAvatarCreationHelper::install();
	SwgCuiQuestHelper::install();
	SwgCuiWebBrowserManager::install();
	SwgCuiTcgManager::install();

	s_receiver = new Listener;

	if(
		(!Game::isParticleEditor() && !Game::isAnimationEditor())
		|| ConfigFile::getKeyBool ("ClientTools", "loadHud", false)
	)
	{
		s_hudLoadedInClientTools = true;
		SwgCuiHudFactory::createHudTemplates();
	}

	s_installed = true;

	CuiManager::getIoWin ().getCallbackConsole ().attachReceiver (s_callbackConsole);
}
//-----------------------------------------------------------------

void SwgCuiManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	CuiManager::getIoWin ().getCallbackConsole ().detachReceiver (s_callbackConsole);

	delete s_receiver;
	s_receiver = 0;
	
	SwgCuiWebBrowserManager::remove();
	SwgCuiTcgManager::remove ();
	SwgCuiQuestHelper::remove ();
	SwgCuiAvatarCreationHelper::remove ();
	SwgCuiMediatorFactorySetup::remove ();

	s_installed = false;
}

//----------------------------------------------------------------------

/**
* Return false if test was successful, true otherwise
*/
bool SwgCuiManager::test (std::string & result)
{
	UNREF (result);

	bool error = false;

	/*
	std::vector<std::string> templateNames;
	templateNames.reserve (12);
	templateNames.push_back ("object/creature/player/shared_human_male.iff");
	templateNames.push_back ("object/creature/player/shared_human_female.iff");
	templateNames.push_back ("object/creature/player/shared_rodian_male.iff");
	templateNames.push_back ("object/creature/player/shared_rodian_female.iff");
	templateNames.push_back ("object/creature/player/shared_moncal_male.iff");
	templateNames.push_back ("object/creature/player/shared_moncal_female.iff");
	templateNames.push_back ("object/creature/player/shared_twilek_male.iff");
	templateNames.push_back ("object/creature/player/shared_twilek_female.iff");
	templateNames.push_back ("object/creature/player/shared_zabrak_male.iff");
	templateNames.push_back ("object/creature/player/shared_zabrak_female.iff");
	templateNames.push_back ("object/creature/player/shared_trandoshan_male.iff");
	templateNames.push_back ("object/creature/player/shared_trandoshan_female.iff");

	error = CuiCharacterLoadoutManager::test (result, templateNames) || error
	*/
	
	return error;
} //lint !e1764 //const

//----------------------------------------------------------------------

void SwgCuiManager::update (float deltaTimeSecs)
{
	UNREF (deltaTimeSecs);

	SwgCuiWebBrowserManager::update(deltaTimeSecs);
	SwgCuiTcgManager::update(deltaTimeSecs);
	
	if (s_checkHud)
	{
		s_checkHud = false;

		CuiActionManager::setCurrentActionMap(static_cast<int>(Game::getHudSceneType()));

		if(
			(!Game::isParticleEditor() && !Game::isAnimationEditor())
			|| s_hudLoadedInClientTools
		)
		{
			SwgCuiHudFactory::createHudIfNeeded();
		}

		if (!s_playerLoginNotificationSent)
		{
			s_playerLoginNotificationSent = true;

			if (!Game::getSinglePlayer())
			{
				//make sure to get the client's quest timer and counter data from the server
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand("handleClientLogin", NetworkId::cms_invalid, Unicode::emptyString));
			}

			Unicode::String date;
			IGNORE_RETURN (CuiUtils::FormatDate(date, CuiUtils::GetSystemSeconds()));

			CuiStringVariablesData data;
			data.targetName = date;
			Unicode::String loggingIn;
			CuiStringVariablesManager::process(CuiStringIdsCommand::logging_in, data, loggingIn);
			CuiChatRoomManager::sendPrelocalizedChat(loggingIn);
		}

		CuiManager::getIoWin().resetDeadZone();
	}
	else if (Game::getHudSceneType() != Game::getLastHudSceneType() && Game::getLastHudSceneType() != Game::ST_numTypes)
	{
		//mark that we're changing hud types, so the scene_changed message that is generated don't spawn additional load screen work
		s_handlingHudTypeChange = true;
		Game::emitSceneChange();
		s_handlingHudTypeChange = false;
	}
}

// ======================================================================
