//======================================================================
//
// SwgCuiChatWindow.cpp
// copyright(c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"
#include "swgClientUserInterface/SwgCuiChatWindow_TabEditor.h"
#include "swgClientUserInterface/SwgCuiChatWindow_TabChanged.h"

#include "clientGame/ChatLogManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/CommunityManager_FriendData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/WhoManager.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatParserStrategy.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiChatRoomManagerStatusMessage.h"
#include "clientUserInterface/CuiChatRoomMessage.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsoleHelperOutputGenerated.h"
#include "clientUserInterface/CuiCSManagerListener.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiFontSizer.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiPrelocalizedChatMessage.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsChat.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "clientUserInterface/CuiStringIdsInstantMessage.h"
#include "clientUserInterface/CuiStringIdsPersistentMessage.h"
#include "clientUserInterface/CuiStringIdsWho.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiSystemMessageManagerData.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedCommandParser/CommandParserHistory.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "swgClientUserInterface/SwgCuiActions.h"
#include "swgClientUserInterface/SwgCuiCommandParserChatRoom.h"
#include "swgClientUserInterface/SwgCuiCommandParserDefault.h"
#include "swgClientUserInterface/SwgCuiCommandParserMount.h"
#include "swgClientUserInterface/SwgCuiCommandParserRemote.h"
#include "swgClientUserInterface/SwgCuiCommandParserScene.h"
#include "swgClientUserInterface/SwgCuiCommandParserUi.h"
#include "swgClientUserInterface/SwgCuiCommandParserVideo.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIIMEManager.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPopupMenu.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <set>
#include <vector>

//#include "DejaLib.h"

//======================================================================

namespace SwgCuiChatWindowNamespace
{
	const UILowerString s_lcdTabName("lcd");

	namespace MenuItems
	{
		const std::string tab_add              = "tab_add";
		const std::string tab_clone            = "tab_clone";
		const std::string tab_delete           = "tab_delete";
		const std::string tab_channels         = "tab_channels";
		const std::string tab_channel_join     = "tab_channel_join";
		const std::string font                 = "font";
		const std::string tab_reset_defaults   = "tab_reset_defaults";
	}

	namespace UnnamedMessages
	{
		const char * const ConGenericMessage = "ConGenericMessage";
	}

	namespace Icons
	{
		UIImageStyle * blink   = 0;
		UIImageStyle * noblink = 0;
	}

	namespace IconColors
	{
		UIColor blink  (0xff,0xff,0xff);
		UIColor noblink(0xff,0xff,0xff);
	}

	namespace DefaultTabs
	{
		typedef SwgCuiChatWindow::ChannelType ChannelType;

		const int maxChannels = 6;
		const ChannelType tabs [][maxChannels] =
		{
			{ SwgCuiChatWindow::CT_spatial, SwgCuiChatWindow::CT_quest, SwgCuiChatWindow::CT_instantMessage, SwgCuiChatWindow::CT_systemMessage, SwgCuiChatWindow::CT_group, SwgCuiChatWindow::CT_matchMaking },
			{ SwgCuiChatWindow::CT_combat, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none },
		};

		const int count = sizeof(tabs) / sizeof(tabs [0]);
	}

	namespace DefaultTabsSpace
	{
		typedef SwgCuiChatWindow::ChannelType ChannelType;
		
		const int maxChannels = 6;

		const ChannelType tabs [][maxChannels] =
		{
			{ SwgCuiChatWindow::CT_spatial, SwgCuiChatWindow::CT_quest, SwgCuiChatWindow::CT_instantMessage, SwgCuiChatWindow::CT_systemMessage, SwgCuiChatWindow::CT_group, SwgCuiChatWindow::CT_matchMaking },
			{ SwgCuiChatWindow::CT_combat, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none, SwgCuiChatWindow::CT_none },
		};
		
		const int count = sizeof(tabs) / sizeof(tabs [0]);
	}

	const std::string ChatTabDragCommand  = "/ui action chatTab ";
	const size_t ChatTabDragCommandLength = ChatTabDragCommand.size();

	void getChatRoomPrefix(const CuiChatRoomDataNode & roomNode, Unicode::String & str)
	{
		std::string shortPath;
		CuiChatRoomManager::getRoomShortPath(roomNode, shortPath);

		static const Unicode::String tabStr(Unicode::narrowToWide("] "));

		str.push_back('[');
		IGNORE_RETURN(str.append(Unicode::narrowToWide(shortPath)));
		IGNORE_RETURN(str.append(tabStr));
	}

	class ChatFontSizeCallbackReceiver : public CallbackReceiver
	{
	public:
		void performCallback()
		{
			SwgCuiChatWindow::onChatFontSizeChanged();
		}
	};

	class ChatBoxKeyClickCallbackReceiver : public CallbackReceiver
	{
	public:
		void performCallback()
		{
			SwgCuiChatWindow::onChatBoxKeyClickChanged();
		}
	};

	namespace Properties
	{
		const UILowerString LastSize        = UILowerString("LastSize");
		const UILowerString LastLocation    = UILowerString("LastLocation");
		const UILowerString LastMinimumSize = UILowerString("LastMinimumSize");
	}

	ChatFontSizeCallbackReceiver s_chatFontSizeCallbackReceiver[Game::ST_numTypes];
	ChatBoxKeyClickCallbackReceiver s_chatBoxKeyClickCallbackReceiver[Game::ST_numTypes];

	static SwgCuiChatWindow::ChatWindowSet ms_activeChatWindows[Game::ST_numTypes];
	
	//----------------------------------------------------------------------
	// Cached spatial chat.

	// Spatial chat cache from ground/space.
	Game::SceneType s_cachedSpatialChatSceneType = Game::ST_reset;
	int const s_spatialChatCacheSizeMax = 16 * 1024;
	int s_spatialChatCacheSize = 0;
	typedef std::deque<Unicode::String> SpatialChatCache;
	SpatialChatCache s_spatialChatCache(s_spatialChatCacheSizeMax);

	// Add spatial chat.
	void addSpatialChat(Unicode::String const & chat)
	{
		if (chat.empty()) 
			return;

		while (s_spatialChatCacheSize > s_spatialChatCacheSizeMax && !s_spatialChatCache.empty())
		{
			s_spatialChatCacheSize -= s_spatialChatCache.front().length();
			s_spatialChatCache.pop_front();
		}

		s_spatialChatCacheSize = std::max(0, s_spatialChatCacheSize);
		s_spatialChatCacheSize += chat.length();
		s_spatialChatCache.push_back(chat);
	}

	// Purge all spatial chat.
	void purgeSpatialChatCache()
	{
		s_spatialChatCacheSize = 0;
		s_spatialChatCache.clear();
	}

	// Add the spatial chat to the chat window.
	void updateCachedSpatialChat(SwgCuiChatWindow * const chatWindow)
	{
		if (chatWindow && s_cachedSpatialChatSceneType != chatWindow->getSceneType()) 
		{
			// Dump the chat to the window.
			for (SpatialChatCache::const_iterator itChat = s_spatialChatCache.begin(); itChat != s_spatialChatCache.end(); ++itChat) 
			{
				Unicode::String const & chatStr = *itChat;
				
				if (chatStr.empty()) 
					continue;
				
				chatWindow->onSpatialChatReceived(chatStr);
			}

			// Save the spatial chat scene.
			s_cachedSpatialChatSceneType = chatWindow->getSceneType();

			// Purge the current data.
			purgeSpatialChatCache();
		}
	}

	//----------------------------------------------------------------------

	namespace Settings
	{
		const std::string tabCount = "tabCount";
		const std::string subWindows = "subWindows";
		const std::string subWindowIndex = "subWindowIndex";
		const std::string activeTab = "activeTab";
	}

	//----------------------------------------------------------------------

	char const * const s_mediatorName = "SwgCuiChatWindowMediator";
	std::string const s_pageName = "SwgCuiChatWindow";

	const float FADE_CHAT_TEXT_START_TIME = 2.0f;
	const float FADE_CHAT_TEXT_TIME = 15.0f;
	const float FADE_CHAT_WAIT_AFTER_FORCE = 5.0f;
	const float FADE_CHAT_MIN_OPACITY = 0.4f;
}

using namespace SwgCuiChatWindowNamespace;

//----------------------------------------------------------------------

class SwgCuiChatWindow::SwgCuiChatWindowAction : public CuiAction
{
public:

	explicit SwgCuiChatWindowAction(SwgCuiChatWindow * chatWindow) : CuiAction(), m_chatWindow(NON_NULL(chatWindow)) {}

	bool performAction(const std::string & id, const Unicode::String & params) const
	{
		NOT_NULL(m_chatWindow);
		if (id == "chatTab")
		{
			//@todo: pass tab info through params string
			UNREF(params);
			int tabId = atoi(Unicode::wideToNarrow(Unicode::getTrim(params)).c_str());
			IGNORE_RETURN(SwgCuiChatWindow::activateChatTabByTabId(tabId));
		}

		else if (id == SwgCuiActions::chatCollapse)
			m_chatWindow->toggleCollapsed();

		else if (id == SwgCuiActions::chatTabNext)
			m_chatWindow->nextTab();

		else if (id == SwgCuiActions::chatTabPrev)
			m_chatWindow->prevTab();

		else if (id == CuiActions::chatCursorHome)
			m_chatWindow->cursorGotoEnd(-1);

		else if (id == CuiActions::chatCursorEnd)
			m_chatWindow->cursorGotoEnd(1);

		else if (id == CuiActions::chatCursorLeft)
			m_chatWindow->cursorMove(-1, false);

		else if (id == CuiActions::chatCursorRight)
			m_chatWindow->cursorMove(1, false);

		else if (id == CuiActions::chatCursorLeftOneWord)
			m_chatWindow->cursorMove(-1, true);

		else if (id == CuiActions::chatCursorRightOneWord)
			m_chatWindow->cursorMove(1, true);

		else if (id == CuiActions::chatHistoryUp)
			m_chatWindow->historyTraverse(-1);

		else if (id == CuiActions::chatHistoryDown)
			m_chatWindow->historyTraverse(1);

		else if (id == CuiActions::chatCommandCompletion)
			m_chatWindow->commandComplete();

		else if (id == CuiActions::chatBackspace)
			m_chatWindow->performBackspaceKey();

		else if (id == CuiActions::chatDelete)
			m_chatWindow->performDeleteKey();

		else if (id == CuiActions::chatEnter)
			m_chatWindow->performEnterKey();

		else if (id == CuiActions::chatFontBigger)
			SwgCuiChatWindow::fontSizeIncrementAllWindows(1);

		else if (id == CuiActions::chatFontSmaller)
			SwgCuiChatWindow::fontSizeIncrementAllWindows(-1);

		else
			return false;

		return true;
	}

	private:
		SwgCuiChatWindow * m_chatWindow;
		SwgCuiChatWindowAction();
};

//----------------------------------------------------------------------

SwgCuiChatWindow::SwgCuiChatWindow(UIPage & page, Game::SceneType sceneType, std::string const & windowName) :
SwgCuiLockableMediator              (s_mediatorName, page),
UINotification           (),
MessageDispatch::Receiver(),
m_inputBox               (0),
m_outputBox              (0),
m_groundHudSkin          (0),
m_groundHudScrollBar     (0),
m_groundHudClose         (0),
m_groundHudChatBar       (0),
m_groundHudChatChannel   (0),
m_tabs                   (0),
m_buttonCollapse         (0),
m_buttonExpand           (0),
m_collapsingPage         (0),
m_persistentPage         (0),
m_history                (new CommandParserHistory),
m_parserStrategy         (0),
m_consoleHelper          (0),
m_printServerOutput      (true),
m_action                 (0),
m_callback               (new MessageDispatch::Callback),
m_textChannel            (0),
m_tabVector              (new TabVector),
m_popupTabIndex          (-1),
m_messageBoxResetDefaults(0),
m_sceneType(static_cast<Game::SceneType>(sceneType)),
m_rootWindow(false),
m_fadeTimer(0.0f),
m_needToRunHoverOutScript(false)
{
	getCodeDataObject(TUITextbox,    m_inputBox,           "Input");
	getCodeDataObject(TUIText,       m_outputBox,          "Output");

	getCodeDataObject(TUIPage,       m_groundHudSkin,           "groundHudSkin",        true);
	getCodeDataObject(TUIPage,       m_groundHudScrollBar,      "groundHudScrollBar",   true);
	getCodeDataObject(TUIButton,     m_groundHudClose,          "groundHudClose",       true);
	getCodeDataObject(TUITextbox,    m_groundHudChatBar,        "groundHudChatBar",     true);
	getCodeDataObject(TUIText,       m_groundHudChatChannel,    "groundHudChatChannel", true);

	getCodeDataObject(TUITabbedPane, m_tabs,               "Tabs");
	getCodeDataObject(TUIButton,     m_buttonCollapse,     "buttonCollapse");
	getCodeDataObject(TUIButton,     m_buttonExpand,       "buttonExpand");
	getCodeDataObject(TUIText,       m_textChannel,        "textChannel");

	m_buttonExpand->RemoveProperty(UIButton::PropertyName::OnPress);
	m_buttonCollapse->RemoveProperty(UIButton::PropertyName::OnPress);

	m_buttonExpand->SetVisible(false);
	m_buttonCollapse->SetVisible(true);

	//----------------------------------------------------------------------
	//-- note: collapsing page and persistent page edges should be flush with
	//-- the edges of the mediator's main page. Any margins included in the UI
	//-- should be included in the persistent and collapsing pages themselves

	getCodeDataObject(TUIPage, m_collapsingPage,  "collapsingPage");
	getCodeDataObject(TUIPage, m_persistentPage,  "persistentPage");

	SwgCuiCommandParserDefault * parser = new SwgCuiCommandParserDefault(m_history);
	IGNORE_RETURN(parser->addSubCommand(new SwgCuiCommandParserUI()));
#if PRODUCTION == 0
	IGNORE_RETURN(parser->addSubCommand(new SwgCuiCommandParserRemote()));
	IGNORE_RETURN(parser->addSubCommand(new SwgCuiCommandParserScene()));
	IGNORE_RETURN(parser->addSubCommand(new SwgCuiCommandParserMount()));
#endif
	IGNORE_RETURN(parser->addSubCommand(new SwgCuiCommandParserVideo()));

	m_rootWindow = ms_activeChatWindows[m_sceneType].empty();
	ms_activeChatWindows[m_sceneType].insert(this);

	m_parserStrategy = new CuiChatParserStrategy(parser, m_rootWindow);
	m_consoleHelper = new CuiConsoleHelper(getPage(), *m_outputBox, *m_inputBox, m_parserStrategy, m_history);
	m_consoleHelper->setEcho                  (false);
//	m_consoleHelper->setEscapeErasesInput     (false);
	m_consoleHelper->setEatEnterMessageOnInput(false);
	parser->setAliasHandler(&m_consoleHelper->getAliasHandler());

	connectToMessage(CuiIoWin::Messages::POINTER_INPUT_TOGGLED);

	if (m_rootWindow)
	{
		connectToMessage(ExecuteConsoleCommand::MessageType);

		CuiChatManager::getChatFontSizeCallback().attachReceiver(s_chatFontSizeCallbackReceiver[m_sceneType]);
		CuiChatManager::getChatBoxKeyClickCallback().attachReceiver(s_chatBoxKeyClickCallbackReceiver[m_sceneType]);

		if (Icons::blink)
			Icons::blink->Attach(0);

		if (Icons::noblink)
			Icons::noblink->Attach(0);

		m_action = new SwgCuiChatWindowAction(this);

		CuiActionManager::addAction(SwgCuiActions::chatTabNext,      m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatTabPrev,      m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatTabEdit,      m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatTabNew,       m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatTabKill,      m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatCollapse,     m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatIconify,      m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatMaximize,     m_action, false);
		CuiActionManager::addAction(SwgCuiActions::chatTab,          m_action, false);

		CuiActionManager::addAction(CuiActions::chatCursorLeftOneWord,  m_action, false);
		CuiActionManager::addAction(CuiActions::chatCursorRightOneWord, m_action, false);
		CuiActionManager::addAction(CuiActions::chatCursorLeft,         m_action, false);
		CuiActionManager::addAction(CuiActions::chatCursorRight,        m_action, false);
		CuiActionManager::addAction(CuiActions::chatCursorHome,         m_action, false);
		CuiActionManager::addAction(CuiActions::chatCursorEnd,          m_action, false);
		CuiActionManager::addAction(CuiActions::chatHistoryUp,          m_action, false);
		CuiActionManager::addAction(CuiActions::chatHistoryDown,        m_action, false);
		CuiActionManager::addAction(CuiActions::chatCommandCompletion,  m_action, false);
		CuiActionManager::addAction(CuiActions::chatFontBigger,         m_action, false);
		CuiActionManager::addAction(CuiActions::chatFontSmaller,        m_action, false);
		CuiActionManager::addAction(CuiActions::chatBackspace,          m_action, false);
		CuiActionManager::addAction(CuiActions::chatDelete,             m_action, false);
		CuiActionManager::addAction(CuiActions::chatEnter,              m_action, false);

		UIButton * const buttonClose = getButtonClose();
		if (buttonClose)
		{
			buttonClose->SetEnabled(false);
			buttonClose->SetVisible(false);
		}

		m_callback->connect(*this, &SwgCuiChatWindow::onCommandParserRequest, static_cast<CuiMessageQueueManager::Messages::CommandParserRequest *>(0));

		//-- force keyclick state to initialize
		onChatBoxKeyClickChanged();
	}
	else
	{
		m_inputBox->SetEnabled(true);
		
		
		m_persistentPage->SetEnabled(true);

		setState(MS_closeable);

		UIButton * const buttonClose = getButtonClose();
		if (buttonClose)
		{
			buttonClose->SetEnabled(true);
			buttonClose->SetVisible(true);
		}

		// update the mediator name.
		if (windowName.empty()) 
		{
			updateDebugMediatorName();
		}
		else
		{
			setMediatorDebugName(windowName);
		}
	}

	
	m_consoleHelper->setOutputText(Unicode::emptyString);
	m_inputBox->SetText(Unicode::emptyString);

	// remove all tabs.
	deleteAllTabs();

	m_outputBox->SetPropertyInteger(CuiFontSizer::Properties::FontIndex, 0);
	const int fontIndex = CuiChatManager::getChatWindowFontSizeDefaultIndex();

	fontSizeIncrement(fontIndex);

	
	// attach to the parent window.
	UIDataSource * const dataSource = NON_NULL(NON_NULL(m_tabs)->GetDataSource());
	dataSource->Clear();

	m_outputBox->SetVisible(true);

	if(m_groundHudSkin)
	{
		m_groundHudSkin->SetVisible(true);
	}

	setIsUpdating(true);

	connectToMessage(UnnamedMessages::ConGenericMessage);
	connectToMessage(CuiSpatialChatManager::Messages::CHAT_RECEIVED);

	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomCreateFailed,           static_cast<CuiChatRoomManager::Messages::CreateFailed*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomCreated,                static_cast<CuiChatRoomManager::Messages::Created*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomDestroyed,              static_cast<CuiChatRoomManager::Messages::Destroyed*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomDestroyFailed,          static_cast<CuiChatRoomManager::Messages::DestroyFailed*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomJoinFailed,             static_cast<CuiChatRoomManager::Messages::JoinFailed*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomMessageReceived,        static_cast<CuiChatRoomManager::Messages::MessageReceived*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomOtherEntered,           static_cast<CuiChatRoomManager::Messages::OtherEntered*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomOtherLeft,              static_cast<CuiChatRoomManager::Messages::OtherLeft*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomSelfEntered,            static_cast<CuiChatRoomManager::Messages::SelfEntered*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomSelfLeft,               static_cast<CuiChatRoomManager::Messages::SelfLeft*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomStatusMessage,          static_cast<CuiChatRoomManager::Messages::StatusMessage*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomModeratorAdded,         static_cast<CuiChatRoomManager::Messages::ModeratorAdded*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onChatRoomModeratorRemoved,       static_cast<CuiChatRoomManager::Messages::ModeratorRemoved*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onPrelocalizedChat,               static_cast<CuiChatRoomManager::Messages::Prelocalized*>(0));
	
	m_callback->connect(*this, &SwgCuiChatWindow::onChatStartInput,                 static_cast<Game::Messages::ChatStartInput*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onCombatSpamReceived,				static_cast<CuiCombatManager::Messages::CombatSpamReceived*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onGCWScoreUpdatedThisGalaxy,      static_cast<GuildObject::Messages::GCWScoreUpdatedThisGalaxy *> (0));

	m_callback->connect(*this, &SwgCuiChatWindow::onConsoleHelperOutput,            static_cast<CuiConsoleHelper::Messages::OutputGenerated*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onDebugPrintUi,                   static_cast<Game::Messages::DebugPrintUi*>(0));
	
	m_callback->connect(*this, &SwgCuiChatWindow::onInstantMessageFailed,           static_cast<CuiInstantMessageManager::Messages::MessageFailed *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onInstantMessageReceived,         static_cast<CuiInstantMessageManager::Messages::MessageReceived *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onPlanetRoomIdChanged,            static_cast<CuiChatRoomManager::Messages::PlanetRoomIdChanged*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onGroupRoomIdChanged,             static_cast<CuiChatRoomManager::Messages::GroupRoomIdChanged*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onGuildRoomIdChanged,             static_cast<CuiChatRoomManager::Messages::GuildRoomIdChanged*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onCityRoomIdChanged,              static_cast<CuiChatRoomManager::Messages::CityRoomIdChanged*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onNamedRoomIdChanged,             static_cast<CuiChatRoomManager::Messages::NamedRoomIdChanged*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onPmReceived,                     static_cast<CuiPersistentMessageManager::Messages::HeaderReceived*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onSocialReceived,                 static_cast<CuiSocialsManager::Messages::SocialReceived*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onSystemMessageReceived,          static_cast<CuiSystemMessageManager::Messages::Received*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onTabEditingCompleted,            static_cast<SwgCuiChatWindow::TabEditor::Messages::ChangeCompleted*>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onMatchMakingMessageReceived,     static_cast<MatchMakingManager::Messages::MatchFoundResultString *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onQuickMatchMessageReceived,      static_cast<MatchMakingManager::Messages::QuickMatchFoundResultString *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onWhoSearchResultMessageReceived, static_cast<PlayerCreatureController::Messages::CharacterMatchRetrieved *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onWhoStatusMessageReceived,       static_cast<PlayerObject::Messages::WhoStatusMessage *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onShowFriendListMessageReceived,  static_cast<CommunityManager::Messages::ShowFriendListMessage *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onShowIgnoreListMessageReceived,  static_cast<CommunityManager::Messages::ShowIgnoreListMessage *>(0));
	m_callback->connect(*this, &SwgCuiChatWindow::onFriendOnlineStatusChanged,      static_cast<CommunityManager::Messages::FriendOnlineStatusChanged *>(0));

	registerMediatorObject(*m_outputBox, true);
	registerMediatorObject(*m_tabs,      true);
	registerMediatorObject(getPage(),    true);

	setSettingsAutoSizeLocation(true, true);

	// Add spatial chat from the other scene.
	updateCachedSpatialChat(this);
} //lint !e429 //parser

//----------------------------------------------------------------------

SwgCuiChatWindow::~SwgCuiChatWindow()
{
	std::for_each(m_tabVector->begin(), m_tabVector->end(), PointerDeleter());
	delete m_tabVector;
	m_tabVector = 0;

	delete m_callback;
	m_callback = 0;

	if (m_action)
	{
		CuiActionManager::removeAction(m_action);
		delete m_action;
		m_action = 0;
	}

	m_inputBox       = 0;
	m_outputBox      = 0;
	m_tabs           = 0;
	m_persistentPage = 0;
	m_buttonCollapse = 0;
	m_buttonExpand   = 0;
	m_collapsingPage = 0;
	m_textChannel    = 0;
	
	m_messageBoxResetDefaults = 0;

	delete m_consoleHelper;
	m_consoleHelper = 0;

	delete m_parserStrategy;
	m_parserStrategy = 0;

	delete m_history;
	m_history = 0;

	ms_activeChatWindows[m_sceneType].erase(this);

	if (ms_activeChatWindows[m_sceneType].empty())
	{
		if (Icons::blink)
			Icons::blink->Detach(0);

		Icons::blink = 0;

		if (Icons::noblink)
			Icons::noblink->Detach(0);

		Icons::noblink = 0;

		CuiChatManager::getChatFontSizeCallback().detachReceiver(s_chatFontSizeCallbackReceiver[m_sceneType]);
		CuiChatManager::getChatBoxKeyClickCallback().detachReceiver(s_chatBoxKeyClickCallbackReceiver[m_sceneType]);
	}

	m_groundHudSkin = NULL;
	m_groundHudScrollBar = NULL;
	m_groundHudClose = NULL;
	m_groundHudChatBar = NULL;
	m_groundHudChatChannel = NULL;
}

//-----------------------------------------------------------------

void SwgCuiChatWindow::performActivate()
{
	setEnabled(true);
	acceptTextInput(false);

	bool const useJapanese = UIManager::gUIManager().isLocaleJapanese();
	bool inputBoxHasIME;
	m_inputBox->GetPropertyBoolean(UITextbox::PropertyName::IME, inputBoxHasIME);
	if (useJapanese && inputBoxHasIME)
		UIManager::gUIManager().getUIIMEManager()->SetEnabled(true);

	fetchTextForActiveTab();

	if(!CuiConsoleHelper::getActiveConsoleHelper())
		CuiConsoleHelper::setActiveConsoleHelper(m_consoleHelper);
}

//-----------------------------------------------------------------

void SwgCuiChatWindow::performDeactivate()
{
	if (m_messageBoxResetDefaults)
		m_callback->disconnect(m_messageBoxResetDefaults->getTransceiverClosed(), *this, &SwgCuiChatWindow::onMessageBoxClosed);

	setEnabled(false);
	acceptTextInput(false);

	if (m_messageBoxResetDefaults)
		m_messageBoxResetDefaults->closeMessageBox();

	// Make sure all chat logging is saved
	ChatLogManager::flush();
}

//-----------------------------------------------------------------

void SwgCuiChatWindow::acceptTextInput(bool b, bool setKeyboardInput, bool unfocusMediator)
{
	if (!isActive() || !getPage().IsVisible())
		b = false;
	else if (isMaximized())
		b = true;

	setKeyboardInput = !CuiPreferences::getModalChat() || setKeyboardInput;
	
	const bool pointerActive = CuiManager::getPointerInputActive() && !CuiManager::getIoWin().getRadialMenuActiveHack();
	CuiManager::getIoWin().setProcessChatInput(!CuiPreferences::getModalChat());
	
	bool const enablePage = pointerActive || b;
	setEnabled(enablePage);
	
	if (b)
	{
		if (m_inputBox->IsEnabled())
		{
			if (setKeyboardInput)
			{
				m_inputBox->SetGetsInput(true);
				m_inputBox->SetFocus();
				setKeyboardInputActive(CuiPreferences::getModalChat());
			}
			else
			{
				m_inputBox->SetGetsInput(false);
				m_outputBox->SetFocus();
				setKeyboardInputActive(false);
			}
		}
	}
	else
	{
		setKeyboardInputActive(false);
		
		UIPage * const parent = dynamic_cast<UIPage *>(m_inputBox->GetParent());
		if (parent)
			parent->SelectChild(0);
		
		if (unfocusMediator)
		{
			DEBUG_FATAL(true,("SwgCuiChatWindow::acceptTextInput unfocusMediator set to true. This code was unused and may not work properly."));
			
			CuiWorkspace * const ws = getContainingWorkspace();
			if (ws)
				IGNORE_RETURN(ws->focusTopMediator(this));
		}
	}
	
	// If the page is enabled & it has has a console helper and the input box is selected,
	// then we set the focus to the page & use it's console helper.
	if (m_consoleHelper && b)
	{
		CuiWorkspace * const ws = getContainingWorkspace();
		if (ws)
			IGNORE_RETURN(ws->focusMediator(*this, true));
		
		CuiConsoleHelper::setActiveConsoleHelper(m_consoleHelper);
		
		m_inputBox->SetSelected(true);
	}
	else
	{
		CuiWorkspace * const ws = getContainingWorkspace();
		if (ws)
			IGNORE_RETURN(ws->focusTopMediator(this));
		
		m_inputBox->SetSelected(false);
	}
	
	
	forceFadeIn();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomStatusMessage(const CuiChatRoomManager::Messages::StatusMessage::Payload & msg)
{
	appendToAllTabs(msg.str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onConsoleHelperOutput(const CuiConsoleHelperOutputGenerated & msg)
{
	if (&msg.helper == m_consoleHelper)
	{
		appendTextToCurrentTab(msg.str);
		msg.processed = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onDebugPrintUi(const Unicode::String & str)
{
	appendTextToCurrentTab(str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatStartInput(const Game::Messages::ChatStartInput::Payload & str)
{
	if (m_inputBox) 
	{
		CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());
		SwgCuiChatWindow * const chatMediator = dynamic_cast<SwgCuiChatWindow*>(workspace->getFocusMediator());
		
		if (chatMediator) 
		{
			if (chatMediator == this) 
			{
				acceptTextInput(true);
				
				if (!str.empty())
				{
					m_inputBox->SetLocalText(str);
					m_inputBox->MoveCaratToEndOfLine();
				}
			}
		}
		else
		{
			if (isRootChatWindow()) 
			{
				acceptTextInput(true);
				
				if (!str.empty())
				{
					m_inputBox->SetLocalText(str);
					m_inputBox->MoveCaratToEndOfLine();
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onSystemMessageReceived(const CuiSystemMessageManagerData & msg)
{
	if ((msg.flags & CuiSystemMessageManagerData::F_quest) != 0)
		appendTextToChannel(ChannelId(CT_quest), ClientTextManager::colorAndFilterText(msg.translated, ClientTextManager::TT_systemMessage, false));
	else
		appendTextToChannel(ChannelId(CT_systemMessage), ClientTextManager::colorAndFilterText(msg.translated, ClientTextManager::TT_systemMessage, false));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onCombatSpamReceived(std::pair<Unicode::String, int> const &payload)
{
	Unicode::String const &text = payload.first;
	ClientTextManager::TextType const spamType = static_cast<ClientTextManager::TextType>(payload.second);

	appendTextToChannel(ChannelId(CT_combat), ClientTextManager::colorAndFilterText(text, spamType, false));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onGCWScoreUpdatedThisGalaxy(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload const & obj)
{
	if (obj.second.first)
	{
		appendTextToChannel(ChannelId(CT_gcw), ClientTextManager::colorAndFilterText(Unicode::narrowToWide(FormattedString<1024>().sprintf("GCW score updated for %s - R:%d%% I:%d%% -> R:%d%% I:%d%%", Unicode::wideToNarrow(StringId("gcw_regions", obj.first).localize()).c_str(), (100 - obj.second.second.first), obj.second.second.first, (100 - obj.second.second.second), obj.second.second.second)), ClientTextManager::TT_systemMessage, false));
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onSocialReceived(const Unicode::String & str)
{
	appendTextToChannel(ChannelId(CT_spatial), ClientTextManager::colorAndFilterText(str, ClientTextManager::TT_social, false));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onSpatialChatReceived(const Unicode::String & str)
{
	appendTextToChannel(ChannelId(CT_spatial), str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onPmReceived(const ChatPersistentMessageToClientData & data)
{
	if (data.status == 'N')
		appendToAllTabs(CuiStringIdsPersistentMessage::new_message_received.localize());
}

//-----------------------------------------------------------------

void SwgCuiChatWindow::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType(UnnamedMessages::ConGenericMessage))
	{
		if (m_printServerOutput)
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin(); //lint !e1774
			ConGenericMessage const cg(ri);

			if (cg.getMsg().empty())
			{
				WARNING(true,("SwgCuiChatWindow received ConGenericMessage with empty msg."));
			}
			else 
				appendToAllTabs(Unicode::narrowToWide(cg.getMsg()));
		}
	}
	else if (message.isType(CuiIoWin::Messages::POINTER_INPUT_TOGGLED))
	{
		acceptTextInput(false);
	}
	else if (message.isType(CuiSpatialChatManager::Messages::CHAT_RECEIVED))
	{
		CuiSpatialChatManager::ChatReceivedMsg const * const cmsg = NON_NULL(dynamic_cast<const CuiSpatialChatManager::ChatReceivedMsg *>(&message));
		onSpatialChatReceived(cmsg->getValue());
	}	
	else if (message.isType(ExecuteConsoleCommand::MessageType))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExecuteConsoleCommand const msg(ri);
		std::set<Unicode::String> s;
		m_consoleHelper->processInput(Unicode::narrowToWide(msg.getCommand()), s, false);		
	}
}

//-----------------------------------------------------------------

void SwgCuiChatWindow::fetchTextForActiveTab()
{
	Tab * const tab = getActiveTab();

	if (tab)
	{
		m_consoleHelper->setOutputText(tab->getText());

		Unicode::String name = tab->getName();

		const size_t dotpos = name.rfind('.');

		if (dotpos != static_cast<size_t>(Unicode::String::npos))
			name = name.substr(dotpos + 1);

		m_textChannel->SetText(name);
		tab->setUnmodified();
		tab->clearCharactersCut();

		for (TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it)
		{
			Tab * const otherTab = NON_NULL(*it);
			if (otherTab == tab)
				continue;

			IGNORE_RETURN(otherTab->updateModifiedDifference(*tab));
		}
	}
	else
	{
		m_consoleHelper->setOutputText(Unicode::emptyString);
		m_textChannel->SetText(Unicode::emptyString);
	}

	m_consoleHelper->scrollToBottom();

	updateActiveDefaultChannel();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::updateActiveDefaultChannel()
{
	SwgCuiCommandParserChatRoom::setCurrentChatRoom(0);
	m_parserStrategy->setChatRoomId(0);
	m_parserStrategy->setUseChatRoom(false);

	Tab const * const tab = getActiveTab();
	if (tab)
	{
		const ChannelId & defaultChannel = tab->getDefaultChannel();

		if (defaultChannel.type == CT_chatRoom || defaultChannel.type == CT_named)
		{
			const std::string & name = defaultChannel.getName();
			const CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNode(name);

			if (roomNode)
			{
				m_parserStrategy->setUseChatRoom(true);
				m_parserStrategy->setChatRoomId(roomNode->data.id);
				SwgCuiCommandParserChatRoom::setCurrentChatRoom(roomNode->data.id);
			}
		}
		else if (defaultChannel.type == CT_planet)
		{
			const uint32 planetRoomId = CuiChatRoomManager::getPlanetRoomId();

			m_parserStrategy->setUseChatRoom(true);
			m_parserStrategy->setChatRoomId(planetRoomId);
			SwgCuiCommandParserChatRoom::setCurrentChatRoom(0);
		}
		else if (defaultChannel.type == CT_group)
		{
			const uint32 groupRoomId = CuiChatRoomManager::getGroupRoomId();

			m_parserStrategy->setUseChatRoom(true);
			m_parserStrategy->setChatRoomId(groupRoomId);
			SwgCuiCommandParserChatRoom::setCurrentChatRoom(0);
		}
		else if (defaultChannel.type == CT_guild)
		{
			const uint32 guildRoomId = CuiChatRoomManager::getGuildRoomId();

			m_parserStrategy->setUseChatRoom(true);
			m_parserStrategy->setChatRoomId(guildRoomId);
			SwgCuiCommandParserChatRoom::setCurrentChatRoom(0);
		}
		else if (defaultChannel.type == CT_city)
		{
			const uint32 cityRoomId = CuiChatRoomManager::getCityRoomId();

			m_parserStrategy->setUseChatRoom(true);
			m_parserStrategy->setChatRoomId(cityRoomId);
			SwgCuiCommandParserChatRoom::setCurrentChatRoom(0);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Notify(UINotificationServer * server, UIBaseObject *, UINotification::Code)
{
	////DEJA_CONTEXT("SwgCuiChatWindow::Notify");

	Tab * const tab = dynamic_cast<Tab *>(server);
	if (!tab)
	{
		WARNING(true,("Bad notification"));
		return;
	}

	////DEJA_TRACE("SwgCuiChatWindow::Notify","Getting Index");

	int const index = std::distance(m_tabVector->begin(), std::find(m_tabVector->begin(), m_tabVector->end(), tab));

	Tab * const activeTab = getActiveTab();

	if (!activeTab)
	{
		WARNING(true,("Bad state"));
		return;
	}

	const bool modified = tab->updateModifiedDifference(*activeTab, false);

	////DEJA_TRACE("SwgCuiChatWindow::Notify","Updating Icons");

	if (modified)
	{
		m_tabs->SetButtonIcon(index, Icons::blink);
		m_tabs->SetButtonIconColor(index, IconColors::blink);
	}
	else
	{
		m_tabs->SetButtonIcon(index, Icons::noblink);
		m_tabs->SetButtonIconColor(index, IconColors::noblink);
	}

	m_tabs->SetButtonText(index, tab->getName());
	
	if(tab == activeTab)
	{
		tab->setDoAppend(true);
		setIsUpdating(true);
	}

	updateActiveDefaultChannel();
}

//-----------------------------------------------------------------

bool SwgCuiChatWindow::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::MouseWheel && context == m_outputBox)
	{
		if (msg.Modifiers.isControlDown())
		{
			fontSizeIncrement(-msg.Data);
			return false;
		}
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::DragStart)
	{
		//-- dragging a chat tab
		if (context->GetParent() == m_tabs && context->IsA(TUIButton))
		{
			if (!isRootChatWindow() && m_tabVector->size() == 1)
			{
				CuiWorkspace * const ws = getContainingWorkspace();
				if (ws)
					getPage().SetOpacity(ws->getOpacityDisabled() * 0.5f);

				// if a tab is torn off, you cant drag n' drop.
				return true;
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragOver)
	{
		if (msg.DragObject)
		{
			CuiDragInfo const info(*msg.DragObject);
			switch(info.type)
			{
			case CuiDragInfoTypes::CDIT_command:
				{
					context->SetDropFlagOk(false);

					if (info.commandValueValid && info.str.compare(0, ChatTabDragCommandLength, ChatTabDragCommand) == 0)
					{
						int tabId = static_cast<int>(info.commandValue);
						const int index = findChatTabByTabId(tabId);
						context->SetDropFlagOk(index < 0);
						return false;
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragCancel)
	{
		if (msg.DragSource == m_tabs || msg.DragSource->GetParent() == m_tabs)
		{
			CuiWorkspace * const ws = getContainingWorkspace();
			if (ws)
				ws->updateOpacities(ws->getOpacityEnabled(), ws->getOpacityDisabled());
		}

		return true;
	}
	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragEnd)
	{
		if (!msg.DragSource)
			return false;

		if (msg.DragSource == m_tabs || msg.DragSource->GetParent() == m_tabs)
		{
			CuiWorkspace * const ws = getContainingWorkspace();
			if (ws)
				ws->updateOpacities(ws->getOpacityEnabled(), ws->getOpacityDisabled());
			return true;
		}

		if (msg.DragObject)
		{
			const CuiDragInfo info(*msg.DragObject);
			switch(info.type)
			{
			case CuiDragInfoTypes::CDIT_command:
				{
					if (info.commandValueValid && info.str.compare(0, ChatTabDragCommandLength, ChatTabDragCommand) == 0)
					{
						int tabId = static_cast<int>(info.commandValue);

						SwgCuiChatWindow * oldChatWindow = 0;
						int oldIndex = 0;

						const Tab * const oldTab = staticFindChatTabByTabId(tabId, oldChatWindow, oldIndex);

						if (!oldTab)
						{
							WARNING(true,("Tab went away"));
							return false;
						}

						NOT_NULL(oldChatWindow);
						DEBUG_FATAL(oldIndex < 0,("bad index"));

						Tab * const newTab = new Tab(*oldTab);
						newTab->setTabId(oldTab->getTabId());
						addTab(*newTab);

						delete oldChatWindow->removeTab(oldIndex);
						oldChatWindow->deactivateIfEmpty();

						return false; //lint !e429 // custodial pointer
					}
				}
			}
		}
	}


	//-----------------------------------------------------------------

	else if (msg.Type == UIMessage::ContextRequest)
	{

		if (context != m_tabs && !context->isAncestor(m_tabs))
			return true;


		UIPopupMenu * const pop = new UIPopupMenu(&getPage());
		pop->SetStyle(getPage().FindPopupStyle());

		IGNORE_RETURN(pop->AddItem(MenuItems::tab_add,            CuiStringIdsChat::chat_tab_menu_tab_add.localize()));
		IGNORE_RETURN(pop->AddItem(MenuItems::tab_channel_join,   CuiStringIdsChat::chat_tab_menu_join_channel.localize()));
		IGNORE_RETURN(pop->AddItem(MenuItems::font,               CuiStringIdsChat::chat_tab_menu_font_size.localize()));
		IGNORE_RETURN(pop->AddItem(MenuItems::tab_reset_defaults, CuiStringIdsChat::chat_tab_menu_reset_defaults.localize()));

		if (context->isAncestor(m_tabs))
		{
			IGNORE_RETURN(pop->AddItem(MenuItems::tab_clone,     CuiStringIdsChat::chat_tab_menu_tab_clone.localize()));
			IGNORE_RETURN(pop->AddItem(MenuItems::tab_delete,    CuiStringIdsChat::chat_tab_menu_tab_delete.localize()));
			IGNORE_RETURN(pop->AddItem(MenuItems::tab_channels,  CuiStringIdsChat::chat_tab_menu_tab_channels.localize()));
		}

		appendPopupOptions(pop);

		pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords + UIPoint::one);
		pop->SetPopupLocation(pop->GetLocation());
		pop->AddCallback(this);
		UIManager::gUIManager().PushContextWidget(*pop);

		m_popupTabIndex = -1;

		if (context->isAncestor(m_tabs))
		{
			const UIButton * const button = safe_cast<const UIButton*>(context);
			NOT_NULL(button);
			m_popupTabIndex = m_tabs->FindTabIndex(*button);
		}

		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (context == m_tabs)
		{
			const int index = m_tabs->GetTabFromPoint(msg.MouseCoords);
			if (index >= 0)
				editChannels(index);
			return false;
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDown)
	{
		if (context == m_tabs)
		{
			int const index = m_tabs->GetTabFromPoint(msg.MouseCoords);
			if (index >= 0)
			{
				setActiveTab(index);
			}
			return false;
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		bool const inputIsSelected = m_inputBox->IsSelected();
		if (inputIsSelected)
		{
			acceptTextInput(true);
		}
	}

	//-----------------------------------------------------------------

	else if (context == &getPage())
	{
		if (msg.Type == UIMessage::KeyDown)
		{
			if (msg.Keystroke == UIMessage::Escape)
			{
				bool const inputIsSelected = m_inputBox && m_inputBox->IsSelected();

				if (inputIsSelected)
				{
					acceptTextInput(false);
					return true; //return true, since this message still needs to be processed later in the message pipeline
				}
			}

			else if (msg.Keystroke == UIMessage::Enter)
			{
				bool const inputIsSelected = m_inputBox && m_inputBox->IsSelected();
				if (inputIsSelected)
				{
					performEnterKey();
					return false;
				}
			}
		}
		else if (msg.Type == UIMessage::RightMouseUp)
		{
			UISize size = m_tabs->GetSize();
			UIPoint Min = m_tabs->GetLocation() + getPage().GetWorldLocation();
			UIPoint Max = Min + size;
			UIPoint mouse = getPage().GetWorldLocation() + msg.MouseCoords;

			bool notOverTabsPage = !(mouse.x >= Min.x && mouse.x <= Max.x && mouse.y >= Min.y && mouse.y <= Max.y);
			
			if (notOverTabsPage)
			{
				SwgCuiLockableMediator::generateLockablePopup(context, msg);
				return false;
			}
		}
	}

	return true;
} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void SwgCuiChatWindow::OnTabbedPaneChanged(UIWidget * context)
{
	if (context != m_tabs)
		return;

	fetchTextForActiveTab();
} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void SwgCuiChatWindow::OnPopupMenuSelection(UIWidget * context)
{
	if (!context->IsA(TUIPopupMenu))
		return;

	UIPopupMenu * const pop = UI_ASOBJECT(UIPopupMenu, context);

	UINarrowString const & sel = pop->GetSelectedName();

	int const index = m_popupTabIndex;
	m_popupTabIndex = -1;

	Unicode::NarrowString str;
	
	if (sel == MenuItems::tab_clone)
	{
		if (index >= 0)
		{
			SwgCuiChatWindow * const newWindow = NON_NULL(cloneFromTab(index, UIPoint::zero, false));
			UNREF(newWindow);
		}
		return;
	}
	else if (sel == MenuItems::tab_delete)
	{
		if (index >= 0)
		{
			delete removeTab(index);
			deactivateIfEmpty();
		}
		return;
	}
	else if (sel == MenuItems::tab_channels)
	{
		if (index >= 0)
			editChannels(index);
	}
	else if (sel == MenuItems::tab_channel_join)
	{
		CuiActionManager::performAction(CuiActions::chatRoomBrowser, Unicode::emptyString);
	}
	else if (sel == MenuItems::tab_add)
	{
		addTab(ChannelId(CT_none), Unicode::emptyString);
	}
	else if (sel == MenuItems::font)
	{
		CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());
		CuiFontSizer * const mediator = CuiFontSizer::createInto(workspace->getPage());

		const CuiFontSizer::SizeVector & sv = ConfigClientUserInterface::getChatWindowFontSizes();

		mediator->setTargetText(*m_outputBox, sv);
		workspace->addMediator(*mediator);
		workspace->positionMediator(*mediator);
		mediator->activate();
		workspace->focusMediator(*mediator, true);
	}
	else if (sel == MenuItems::tab_reset_defaults)
	{
		if (!m_messageBoxResetDefaults)
		{
			m_messageBoxResetDefaults = CuiMessageBox::createYesNoBox(CuiStringIdsChat::chat_tab_confirm_reset_defaults.localize());
			m_callback->connect(m_messageBoxResetDefaults->getTransceiverClosed(), *this, &SwgCuiChatWindow::onMessageBoxClosed);
		}
	}
	else
		SwgCuiLockableMediator::OnPopupMenuSelection(context);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::toggleCollapsed()
{
	if (!getContainingWorkspace())
		return;

	getPage().ReleaseMouseLock(UIPoint::zero);

	//----------------------------------------------------------------------
	//-- collapse the collapsing page

	if (m_sceneType == Game::ST_space) 
	{
		if (m_collapsingPage->IsVisible())
		{
			const UIPoint & persistentLocation = m_persistentPage->GetLocation();

			m_collapsingPage->SetVisible(false);
			m_buttonExpand->SetVisible  (true);
			m_buttonCollapse->SetVisible(false);

			m_persistentPage->SetPropertyPoint(Properties::LastLocation, persistentLocation);
			getPage().SetPropertyPoint(Properties::LastMinimumSize, getPage().GetMinimumSize());
			m_collapsingPage->SetPropertyPoint(Properties::LastSize, m_collapsingPage->GetSize());

			const long newPageHeight  = getPage().GetHeight() - persistentLocation.y;
			UIPoint pageLocation     (getPage().GetLocation());
			pageLocation.y            += persistentLocation.y;

			getPage().SetMinimumSize    (UISize(getPage().GetMinimumSize().x, newPageHeight));
			getPage().SetMaximumSize    (UISize(16384L, newPageHeight));
			getPage().SetHeight         (newPageHeight);
			getPage().Pack();
			getPage().SetLocation       (pageLocation);
			m_persistentPage->SetLocation(m_persistentPage->GetLocation().x, newPageHeight - m_persistentPage->GetHeight());

		}
		//----------------------------------------------------------------------
		//-- expand the collapsing page

		else
		{
			m_collapsingPage->SetVisible(true);
			m_buttonCollapse->SetVisible(true);
			m_buttonExpand->SetVisible  (false);

			UIPoint lastPersistentLocation;
			UIPoint lastMinimumSize;
			UIPoint lastCollapsingSize;

			m_persistentPage->GetPropertyPoint(Properties::LastLocation, lastPersistentLocation);
			getPage().GetPropertyPoint(Properties::LastMinimumSize, lastMinimumSize);
			m_collapsingPage->GetPropertyPoint(Properties::LastSize, lastCollapsingSize);

			const long newPageHeight = lastPersistentLocation.y + m_persistentPage->GetHeight();

			UIPoint pageLocation         (getPage().GetLocation());
			pageLocation.y              -=(newPageHeight - getPage().GetHeight());
			pageLocation.y               = std::max(0L, pageLocation.y);

			getPage().SetMaximumSize   (UISize(16384L, 16384L));
			getPage().SetMinimumSize   (lastMinimumSize);
			getPage().SetHeight        (newPageHeight);
			getPage().Pack             ();


			m_persistentPage->SetLocation(m_persistentPage->GetLocation().x, lastPersistentLocation.y);
			m_collapsingPage->SetLocation(m_persistentPage->GetLocation().x, 0L);
			m_collapsingPage->SetHeight  (lastCollapsingSize.y);
			getPage().SetLocation       (pageLocation);

		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::cloneFromTabDragged(const UIPoint & pt, int tabId)
{
	Game::SceneType const sceneType = Game::getHudSceneType();
	for (ChatWindowSet::iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
	{
		SwgCuiChatWindow * const chatWindow = NON_NULL(*it);
		int const index = chatWindow->findChatTabByTabId(tabId);

		if (index >= 0)
		{
			if (chatWindow->getTabCount() > 1) 
			{
				chatWindow->cloneFromTab(index, pt, true);
			}
			return;
		}
	}
}

//----------------------------------------------------------------------

SwgCuiChatWindow * SwgCuiChatWindow::createNewWindow(UIPage & parentPage, Game::SceneType sceneType, std::string const & windowName)
{
	UIPage * const page = static_cast<UIPage *>(parentPage.GetObjectFromPath(sceneType == Game::ST_ground ? "/GroundHud.ChatWindow" : "/HudSpace.ChatWindow", TUIPage));
	SwgCuiChatWindow * newWindow = NULL;
	if (page)
	{
		UIPage * const newPage = static_cast<UIPage *>(page->DuplicateObject());
		if (newPage) 
		{
			UIPage * const pageGroundHud = static_cast<UIPage *>(parentPage.GetObjectFromPath(sceneType == Game::ST_ground ? "/GroundHud" : "/HudSpace", TUIPage));

			if (pageGroundHud)
				pageGroundHud->AddChild(newPage);
			
			newPage->SetVisible(true);

			newPage->Link();

			newWindow = new SwgCuiChatWindow(*newPage, sceneType, windowName);
			
			newWindow->openNextFrame();
			newWindow->setSettingsAutoSizeLocation(true, true);
		}
	}

	return newWindow;
}

//----------------------------------------------------------------------

SwgCuiChatWindow * SwgCuiChatWindow::cloneFromTab(int index, const UIPoint & pos, bool tearOff)
{
	CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());
	SwgCuiChatWindow * newWindow = 0;

	if (tearOff)
	{
		newWindow = createInto(workspace, m_sceneType);

		//-- invisify this page for the duration of the mediator window placement,
		//-- so the newly placed window doesn't collide with it if it is going away
		bool const invisify = (m_tabVector->size() == 1);
		if (invisify)
			getPage().SetVisible(false);

		newWindow->getPage().SetLocation(pos);
		
		if (invisify)
			getPage().SetVisible(true);
	}
	else
		newWindow = this;

	Tab * const oldTab = getTab(index);
	if (oldTab)
	{
		Tab * const newTab = new Tab(*oldTab);
		newTab->setTabId(oldTab->getTabId());
		newWindow->addTab(*newTab);

		if (newWindow != this)
		{
			delete removeTab(index);
		}
	} //lint !e429 // custodial pointer 
	else
		WARNING(true,("No tab in clone"));

	if (tearOff)
	{
		newWindow->setState(CuiMediator::MS_settingsLoaded);
	}

	return newWindow;
}

//-----------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::addTab(const ChannelId & id, const Unicode::String & str)
{
	Tab * const tab = new Tab;
	tab->setDefaultChannel(id);

	if (!str.empty())
		tab->appendText(id, str);

	tab->setUnmodified();
	addTab(*tab);

	fetchTextForActiveTab();

	return tab;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::addTab(Tab & tab)
{
	m_tabVector->push_back(&tab);
	tab.Listen(this);

	UIDataSource * const ds = NON_NULL(m_tabs->GetDataSource());

	UIData * const data = new UIData;
	data->SetName(Unicode::wideToNarrow(tab.getName()));
	data->SetProperty(UITabbedPane::DataProperties::Text, tab.getName());

	data->SetPropertyNarrow(UITabbedPane::DataProperties::DATA_DRAGTYPE, CuiDragInfo::DragTypes::Command);

	Unicode::String istr;
	UIUtils::FormatInteger(istr, tab.getTabId());

	Unicode::String extraProps;
	extraProps += Unicode::narrowToWide(CuiDragInfo::Properties::CommandString.c_str());
	extraProps.push_back('=');
	extraProps += Unicode::narrowToWide(ChatTabDragCommand) + istr;
	extraProps.push_back(';');
	extraProps += Unicode::narrowToWide(CuiDragInfo::Properties::CommandValue.c_str());
	extraProps.push_back('=');
	extraProps += istr;

	data->SetProperty       (UITabbedPane::DataProperties::DATA_EXTRA_PROPS,       extraProps);

	data->SetPropertyNarrow (UITabbedPane::DataProperties::DATA_TARGET,            "output.text");
	data->SetPropertyBoolean(UITabbedPane::DataProperties::DATA_CONTEXT_TO_PARENT, true);
	data->SetPropertyBoolean(UITabbedPane::DataProperties::DATA_CONTEXT_CAPABLE, true);

	data->SetPropertyBoolean(UITabbedPane::DataProperties::DATA_DROP_TO_PARENT,    true);
	if (Icons::noblink)
		data->SetPropertyNarrow(UITabbedPane::DataProperties::DATA_ICON_PATH,       Icons::noblink->GetFullPath());
	data->SetPropertyColor  (UITabbedPane::DataProperties::DATA_ICON_COLOR,        IconColors::noblink);

	ds->AddChild(data);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::removeTab(int index)
{
	const long activeTab = m_tabs->GetActiveTab();

	Tab * const tab = getTab(index);

	if (tab)
	{
		tab->StopListening(this);

		m_tabVector->erase(std::remove(m_tabVector->begin(), m_tabVector->end(), tab), m_tabVector->end());

		UIDataSource * const ds = NON_NULL(m_tabs->GetDataSource());

		UIData * const data = ds->GetChildByPosition(index);

		if (data)
		{
			ds->RemoveChild(data);
		}
		else
			WARNING(true,("No data object"));

		if (index <= activeTab)
			m_tabs->SetActiveTab(std::max(0L, activeTab - 1L));

		fetchTextForActiveTab();
	}
	else
		WARNING(true,("No tab to remove there."));

	DEBUG_FATAL(ms_activeChatWindows[m_sceneType].empty(),("no chat windows"));

	return tab;
}

//----------------------------------------------------------------------

int SwgCuiChatWindow::getTabCount() const
{
	return static_cast<int>(m_tabVector->size());
}


//----------------------------------------------------------------------

void SwgCuiChatWindow::onInstantMessageReceived(const CuiInstantMessageManagerElement & elem)
{
	if (CuiChatManager::getChatStyle() == CuiChatManager::CS_Brief)
	{
		const Unicode::String & str = CuiChatManager::prosify(elem.avatarId, ClientTextManager::colorAndFilterText(elem.message, ClientTextManager::TT_tell, Game::isProfanityFiltered()), CuiStringIdsInstantMessage::im_received_brief_prose);
		appendTextToChannel(ChannelId(CT_instantMessage), str);
	}
	else
	{
		const Unicode::String & str = CuiChatManager::prosify(elem.avatarId, ClientTextManager::colorAndFilterText(elem.message, ClientTextManager::TT_tell, Game::isProfanityFiltered()), CuiStringIdsInstantMessage::im_received_prose);
		appendTextToChannel(ChannelId(CT_instantMessage), str);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onInstantMessageFailed(const Unicode::String & str)
{
	appendToAllTabs(str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomCreated(const CuiChatRoomManager::Messages::Created::Payload & roomNode)
{
	ChannelId const id(CT_chatRoom, roomNode.data.path);
	
	for (SwgCuiChatWindow::TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it)
	{
		SwgCuiChatWindow::Tab * const tab = NON_NULL(*it);
		
		if (tab->hasChannel(id))
		{
			Unicode::String str;
			getChatRoomPrefix(roomNode, str);
			str += CuiStringIdsChatRoom::created.localize();
			appendToAllTabs(str);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomDestroyed(const CuiChatRoomManager::Messages::Destroyed::Payload & payload)
{
	const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId();

	const CuiChatRoomDataNode & roomNode = *NON_NULL(payload.first);
	const CuiChatAvatarId & avatar       = *NON_NULL(payload.second);

	Unicode::String str;
	getChatRoomPrefix(roomNode, str);

	bool found = false;

	if (selfId == avatar.chatId)
	{
		str += CuiStringIdsChatRoom::destroyed_self.localize();
		found = true;
	}
	else
	{
		str += CuiChatManager::prosify(avatar.chatId, CuiStringIdsChatRoom::destroyed_other_prose);
	}

	const ChannelId id(CT_chatRoom, roomNode.data.path);

	for (SwgCuiChatWindow::TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it)
	{
		SwgCuiChatWindow::Tab * const tab = NON_NULL(*it);

		if (tab->hasChannel(id))
		{
			found = true;
			break;
		}
	}

	onChatRoomSelfLeftInternal(roomNode, false);

	if (found)
		appendToAllTabs(str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomDestroyFailed(const CuiChatRoomManager::Messages::DestroyFailed::Payload & payload)
{
	const Unicode::String & str = payload.second;
	appendToAllTabs(str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onPlanetRoomIdChanged(const CuiChatRoomManager::Messages::PlanetRoomIdChanged::Payload &)
{
	updateActiveDefaultChannel();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onGroupRoomIdChanged(const CuiChatRoomManager::Messages::GroupRoomIdChanged::Payload &)
{
	bool found = false;
	
	ChannelId const cid(CT_group);
	for (ChatWindowSet::const_iterator itChatWin = ms_activeChatWindows[m_sceneType].begin(); itChatWin != ms_activeChatWindows[m_sceneType].end(); ++itChatWin)
	{
		SwgCuiChatWindow const * const cw = NON_NULL(*itChatWin);
		
		for (TabVector::const_iterator it = cw->m_tabVector->begin(); it != cw->m_tabVector->end(); ++it)
		{
			Tab const * const tab = *it;
			if (tab->hasChannel(cid))
			{
				found = true;
				break;
			}
		}
	}
	
	if (!found && isRootChatWindow())
	{
		IGNORE_RETURN(addTab(cid, Unicode::emptyString));
	}

	updateActiveDefaultChannel();
}

//-----------------------------------------------------------------------

void SwgCuiChatWindow::onGuildRoomIdChanged(const CuiChatRoomManager::Messages::GuildRoomIdChanged::Payload &)
{
	bool found = false;
	
	ChannelId const cid(CT_guild);
	for (ChatWindowSet::const_iterator itChatWin = ms_activeChatWindows[m_sceneType].begin(); itChatWin != ms_activeChatWindows[m_sceneType].end(); ++itChatWin)
	{
		SwgCuiChatWindow const * const cw = NON_NULL(*itChatWin);
		
		for (TabVector::const_iterator it = cw->m_tabVector->begin(); it != cw->m_tabVector->end(); ++it)
		{
			const Tab * const tab = *it;
			if (tab->hasChannel(cid))
			{
				found = true;
				break;
			}
		}
	}
	
	if (!found && isRootChatWindow())
	{
		addTab(cid, Unicode::emptyString);
	}

	updateActiveDefaultChannel();
}

//-----------------------------------------------------------------------

void SwgCuiChatWindow::onCityRoomIdChanged(const CuiChatRoomManager::Messages::CityRoomIdChanged::Payload &)
{
	bool found = false;

	ChannelId const cid(CT_city);
	for (ChatWindowSet::const_iterator itChatWin = ms_activeChatWindows[m_sceneType].begin(); itChatWin != ms_activeChatWindows[m_sceneType].end(); ++itChatWin)
	{
		SwgCuiChatWindow const * const cw = NON_NULL(*itChatWin);

		for (TabVector::const_iterator it = cw->m_tabVector->begin(); it != cw->m_tabVector->end(); ++it)
		{
			const Tab * const tab = *it;
			if (tab->hasChannel(cid))
			{
				found = true;
				break;
			}
		}
	}

	if (!found && isRootChatWindow())
	{
		addTab(cid, Unicode::emptyString);
	}

	updateActiveDefaultChannel();
}

//-----------------------------------------------------------------------

void SwgCuiChatWindow::onNamedRoomIdChanged(const CuiChatRoomManager::Messages::NamedRoomIdChanged::Payload & payload)
{
	Game::SceneType const sceneType = Game::getHudSceneType();
	
	bool found = false;
	
	CuiChatRoomDataNode const * const roomNode = CuiChatRoomManager::findRoomNode(payload);
	if (roomNode)
	{
		ChannelId const cid(CT_named, roomNode->getFullPath());
		for (ChatWindowSet::const_iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
		{
			SwgCuiChatWindow const * const cw = NON_NULL(*it);
			
			for (TabVector::iterator it = cw->m_tabVector->begin(); it != cw->m_tabVector->end(); ++it)
			{
				Tab * tab = *it;
				if (tab->getDefaultChannel().type == cid.type && tab->getDefaultChannel().getDisplayName() == cid.getDisplayName())
				{
					tab->setDefaultChannel(cid);
					found = true;
					break;
				}
			}
		}
		
		if (!found && isRootChatWindow())
		{
			addTab(cid, Unicode::emptyString);
		}
		
		updateActiveDefaultChannel();
	}
}

//-----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomMessageReceived(const CuiChatRoomManager::Messages::MessageReceived::Payload & message)
{
	const CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNode(message.roomId);
	if (roomNode)
	{
		Unicode::String str;
		if ( ! CuiChatRoomManager::hasNamedRoom(message.roomId))
		{
			Unicode::String widePrefix;
			getChatRoomPrefix(*roomNode, widePrefix);
			std::string narrowPrefix = Unicode::wideToNarrow(widePrefix);
			if (narrowPrefix.find("named.") == std::string::npos)
				getChatRoomPrefix(*roomNode, str);
		}

		static const Unicode::String nameTabStr(Unicode::narrowToWide("\\>032: "));
		str.append(CuiChatManager::getShortName(message.sender.chatId));
		str.append(nameTabStr);

		Unicode::String text = message.message;
		ProsePackageManagerClient::appendAllProsePackages(message.oob, text);

		if (message.roomId == CuiChatRoomManager::getGroupRoomId())
		{
			str += ClientTextManager::colorAndFilterText(text, ClientTextManager::TT_chatChannelGroup, Game::isProfanityFiltered());
		}
		else if (message.roomId == CuiChatRoomManager::getGuildRoomId())
		{
			str += ClientTextManager::colorAndFilterText(text, ClientTextManager::TT_guild, Game::isProfanityFiltered());
		}
		else if (message.roomId == CuiChatRoomManager::getCityRoomId())
		{
			str += ClientTextManager::colorAndFilterText(text, ClientTextManager::TT_city, Game::isProfanityFiltered());
		}
		else
		{
			str += ClientTextManager::colorAndFilterText(text, ClientTextManager::TT_chatChannel, Game::isProfanityFiltered());
		}

		str.append(Unicode::narrowToWide("\\>000"));

		if (message.roomId == CuiChatRoomManager::getPlanetRoomId())
			appendTextToChannel(ChannelId(CT_planet), str);
		else if (message.roomId == CuiChatRoomManager::getGroupRoomId())
			appendTextToChannel(ChannelId(CT_group), str);
		else if (message.roomId == CuiChatRoomManager::getGuildRoomId())
			appendTextToChannel(ChannelId(CT_guild), str);
		else if (message.roomId == CuiChatRoomManager::getCityRoomId())
			appendTextToChannel(ChannelId(CT_city), str);
		else if (CuiChatRoomManager::hasNamedRoom(message.roomId))
			appendTextToChannel(ChannelId(CT_named, roomNode->getFullPath()), str);
		else
			appendTextToChannel(ChannelId(CT_chatRoom, roomNode->getFullPath()), str);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::handleChatRoomJoinDefault(const CuiChatRoomDataNode & roomNode)
{
	const std::string & fullPath = roomNode.getFullPath();

	bool const isPublic = roomNode.data.roomType == CHAT_ROOM_PUBLIC;
	ChannelId id(CT_chatRoom, fullPath, isPublic);
	appendTextToChannel(id, Unicode::emptyString, true);

	Unicode::String str;
	getChatRoomPrefix(roomNode, str);

	Unicode::String str2;
	CuiStringIdsChatRoom::self_join.localize(str2);

	str += str2;

	for (TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it)
	{
		SwgCuiChatWindow::Tab * tab = *it;
		if (tab->getDefaultChannel() == id)
		{
			if (tab->getDefaultChannel().getDisplayName().empty())
			{
				tab->setDefaultChannel(id);
			}
		}
	}
	
	appendToAllTabs(str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomSelfEntered(const CuiChatRoomManager::Messages::SelfEntered::Payload & roomNode)
{
	CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());

	TabEditor * const mediator = dynamic_cast<TabEditor *>(workspace->findMediatorByType(typeid(TabEditor)));
	if (mediator)
	{
		if (mediator->handleChatRoomJoin(roomNode))
			return;
	}

	handleChatRoomJoinDefault(roomNode);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomOtherEntered(const CuiChatRoomManager::Messages::OtherEntered::Payload & payload)
{
	const CuiChatRoomDataNode & roomNode = *NON_NULL(payload.first);

	//-- zero roomType is private
	if (roomNode.data.roomType != 0)
	{
		const CuiChatAvatarId & avatar = *NON_NULL(payload.second);

		Unicode::String str;
		getChatRoomPrefix(roomNode, str);

		str += CuiChatManager::prosify(avatar.chatId, CuiStringIdsChatRoom::other_entered_prose);

		appendTextToChannel(ChannelId(CT_chatRoom, roomNode.getFullPath()), str);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomCreateFailed(const CuiChatRoomManager::Messages::CreateFailed::Payload & payload)
{
	appendToAllTabs(payload);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomJoinFailed(const CuiChatRoomManager::Messages::JoinFailed::Payload & payload)
{
	appendToAllTabs(payload);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomModeratorAdded(const CuiChatRoomManager::Messages::ModeratorAdded::Payload & payload)
{
	const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId();

	const CuiChatRoomDataNode & roomNode = *NON_NULL(payload.first);
	const CuiChatAvatarId & avatar = *NON_NULL(payload.second);

	Unicode::String str;
	getChatRoomPrefix(roomNode, str);

	if (selfId == avatar.chatId)
		str += CuiStringIdsChatRoom::self_moderator_added.localize();
	else
		str += CuiChatManager::prosify(avatar.chatId, CuiStringIdsChatRoom::other_moderator_added_prose);

	appendTextToChannel(ChannelId(CT_chatRoom, roomNode.getFullPath()), str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomModeratorRemoved(const CuiChatRoomManager::Messages::ModeratorRemoved::Payload & payload)
{
	const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId();

	const CuiChatRoomDataNode & roomNode = *NON_NULL(payload.first);
	const CuiChatAvatarId & avatar       = *NON_NULL(payload.second);

	Unicode::String str;
	getChatRoomPrefix(roomNode, str);

	if (selfId == avatar.chatId)
		str += CuiStringIdsChatRoom::self_moderator_removed.localize();
	else
		str += CuiChatManager::prosify(avatar.chatId, CuiStringIdsChatRoom::other_moderator_removed_prose);

	appendTextToChannel(ChannelId(CT_chatRoom, roomNode.getFullPath()), str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onPrelocalizedChat(const CuiPrelocalizedChatMessage & payload)
{
	appendTextToCurrentTab(payload.str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::handleChatRoomLeave(const CuiChatRoomDataNode & roomNode, bool printMsg)
{
	const std::string & fullPath = roomNode.getFullPath();
	const ChannelId channelId(CT_chatRoom, fullPath);

	if (printMsg)
	{
		Unicode::String str;
		getChatRoomPrefix(roomNode, str);

		Unicode::String str2;
		CuiStringIdsChatRoom::self_left.localize(str2);

		str += str2;
		appendTextToChannel(channelId, str);
	}


	{
		for (TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it)
		{
			Tab * const tab = *it;
			tab->removeChannel(channelId);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomSelfLeft(const CuiChatRoomManager::Messages::SelfLeft::Payload & room)
{
	onChatRoomSelfLeftInternal(room, true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomSelfLeftInternal(const CuiChatRoomDataNode & roomNode , bool printMsg)
{
	CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());

	TabEditor * const mediator = dynamic_cast<TabEditor *>(workspace->findMediatorByType(typeid(TabEditor)));
	if (mediator)
		mediator->handleChatRoomLeave(roomNode);

	handleChatRoomLeave(roomNode, printMsg);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatRoomOtherLeft(const CuiChatRoomManager::Messages::OtherLeft::Payload & payload)
{
	const CuiChatRoomDataNode & roomNode = *NON_NULL(payload.first);
	//-- zero roomType is private
	if (roomNode.data.roomType != 0)
	{
		const CuiChatAvatarId & avatar = *NON_NULL(payload.second);

		Unicode::String str;
		getChatRoomPrefix(roomNode, str);

		str += CuiChatManager::prosify(avatar.chatId, CuiStringIdsChatRoom::other_left_prose);

		appendTextToChannel(ChannelId(CT_chatRoom, roomNode.getFullPath()), str);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::appendToAllTabs(const Unicode::String & str)
{
	appendTextToChannel(ChannelId(), str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::appendTextToChannel(const ChannelId & id, const Unicode::String & str, bool /*create*/)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::appendTextToChannel");

	const int activeTab = m_tabs->GetActiveTab();

	const Tab * result = 0;
	int count = 0;

	for (SwgCuiChatWindow::TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it, ++count)
	{
		SwgCuiChatWindow::Tab * const tab = NON_NULL(*it);

		if (id.type == CT_none || tab->hasChannel(id))
		{
#ifdef _DEBUG
			const unsigned int strSize = str.size();
			UNREF(strSize);
#endif // _DEBUG

			tab->appendText(id, str);
			
			
			if (count == activeTab)
				result = tab;
		}
	}

	// Cache spatial chat.
	if (id == ChannelId(CT_spatial) && m_sceneType == s_cachedSpatialChatSceneType)
	{
		addSpatialChat(str);
	}
	
	// ju - commented out so that external input into the window won't cause it to fade in
	//forceFadeIn();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::appendTextToCurrentTab(const Unicode::String & str)
{
	int const activeTab = m_tabs->GetActiveTab();

	if (activeTab < 0 || activeTab >= static_cast<int>(m_tabVector->size()))
		return;

	SwgCuiChatWindow::Tab * const tab =(*m_tabVector) [activeTab];
	if (tab)
		tab->appendText(ChannelId(CT_none), str);
}

//----------------------------------------------------------------------

const SwgCuiChatWindow::Tab * SwgCuiChatWindow::getTab(int index) const
{
	return const_cast<SwgCuiChatWindow *>(this)->getTab(index);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::getTab(int index)
{
	if (!m_tabVector->empty()) 
	{
		index = clamp(0, index, static_cast<int>(m_tabVector->size()));
		return(*m_tabVector) [static_cast<size_t>(index)];
	}

	return NULL;
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::getActiveTab()
{
	const int index = static_cast<int>(m_tabs->GetActiveTab());
	return getTab(index);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::getLcdTab()
{
	for(int i = 0; i < m_tabs->GetTabCount(); ++i)
	{
		SwgCuiChatWindow::Tab *currentTab = getTab(i);
		if(UILowerString(Unicode::wideToNarrow(currentTab->getName())) == s_lcdTabName)
			return currentTab;
	}
	const int index = static_cast<int>(m_tabs->GetActiveTab());
	return getTab(index);
}

//----------------------------------------------------------------------

const SwgCuiChatWindow::Tab * SwgCuiChatWindow::getActiveTab() const
{
	return const_cast<SwgCuiChatWindow *>(this)->getActiveTab();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::editChannels(int index)
{
	Tab * const tab = getTab(index);
	if (tab)
	{
		CuiWorkspace * const workspace = NON_NULL(getContainingWorkspace());

		TabEditor * mediator = dynamic_cast<TabEditor *>(workspace->findMediatorByType(typeid(TabEditor)));

		if (!mediator)
		{
			mediator = TabEditor::createInto(workspace->getPage());
			mediator->setSettingsAutoSizeLocation(true, true);
			workspace->addMediator(*mediator);
		}

		mediator->activate();

		workspace->focusMediator(*mediator, true);
		mediator->setTab(*tab);
	}
	else
		WARNING(true,("No tab"));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onTabEditingCompleted(const Tab & tab)
{
	SwgCuiChatWindow * chatWindow = 0;
	int index = 0;

	Tab * const oldTab = staticFindChatTabByTabId(tab.getTabId(), chatWindow, index);

	if (oldTab && chatWindow == this)
	{
		oldTab->copy(tab, false);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::setupDefaultTabs(bool printMessage)
{
	DEBUG_FATAL(ms_activeChatWindows[m_sceneType].empty(),("no chat windows"));

	if (m_sceneType == Game::ST_space)
	{
		for (int i = 0; i < DefaultTabsSpace::count; ++i)
		{
			ChannelType const def = DefaultTabsSpace::tabs[i][0];
			ChannelId const defChannelId(def);
			
			Unicode::String msg;
			
			if (printMessage)
			{
				msg = CuiStringIdsChat::chat_tab_default_recreated.localize();
				msg.append(1, ' ');
				msg += defChannelId.getDisplayName();
			}
			
			Tab * const tab = addTab(defChannelId, msg);
			
			for (int j = 1; j < DefaultTabsSpace::maxChannels; ++j)
			{
				const ChannelType type = DefaultTabsSpace::tabs[i][j];
				
				if (type == CT_none)
					break;
				
				tab->addChannel(ChannelId(type));
			}
		}
	}
	else
	{
		for (int i = 0; i < DefaultTabs::count; ++i)
		{
			const ChannelType def = DefaultTabs::tabs [i][0];
			const ChannelId defChannelId(def);
			
			Unicode::String msg;
			
			if (printMessage)
			{
				msg = CuiStringIdsChat::chat_tab_default_recreated.localize();
				msg.append(1, ' ');
				msg += defChannelId.getDisplayName();
			}
			
			Tab * const tab = addTab(defChannelId, msg);
			
			for (int j = 1; j < DefaultTabs::maxChannels; ++j)
			{
				const ChannelType type = DefaultTabs::tabs [i][j];
				
				if (type == CT_none)
					break;
				
				tab->addChannel(ChannelId(type));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::nextTab()
{
	int const activeTab = m_tabs->GetActiveTab();
	if (activeTab < 0 || activeTab >=(m_tabs->GetTabCount() - 1))
		setActiveTab(0);
	else
		setActiveTab(activeTab + 1);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::prevTab()
{
	int const activeTab = m_tabs->GetActiveTab();
	if (activeTab <= 0)
		setActiveTab(m_tabs->GetTabCount() - 1);
	else
		setActiveTab(activeTab - 1);
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::activateChatTabByTabId(int id)
{
	SwgCuiChatWindow * chatWindow = 0;
	int index = 0;
	Tab const * const tab = staticFindChatTabByTabId(id, chatWindow, index);
	if (tab && chatWindow)
	{
		chatWindow->setActiveTab(index);

		if (chatWindow->getContainingWorkspace())
			chatWindow->getContainingWorkspace()->focusMediator(*chatWindow, true);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

int SwgCuiChatWindow::findChatTabByTabId(int id) const
{
	int index = 0;
	for (TabVector::iterator it = m_tabVector->begin(); it != m_tabVector->end(); ++it, ++index)
	{
		Tab const * const tab = NON_NULL(*it);
		if (tab->getTabId() == id)
			return index;
	}

	return -1;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::setActiveTab(int index)
{
	if (m_tabs->GetTabCount() <= 0)
		return;

	index = std::min((int)m_tabs->GetTabCount() - 1, std::max(0, index));
	m_tabs->SetActiveTab(index);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab * SwgCuiChatWindow::staticFindChatTabByTabId(int tabId, SwgCuiChatWindow * & chatWindow, int & index)
{
	Game::SceneType sceneType = Game::getHudSceneType();
	
	for (ChatWindowSet::iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
	{
		SwgCuiChatWindow * const cw = NON_NULL(*it);

		int const localIndex = cw->findChatTabByTabId(tabId);

		if (localIndex >= 0)
		{
			index = localIndex;
			chatWindow = cw;
			return chatWindow->getTab(index);
		}
	}

	return false;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onMessageBoxClosed(const CuiMessageBox & box)
{
	if (&box == m_messageBoxResetDefaults)
	{
		m_callback->disconnect(m_messageBoxResetDefaults->getTransceiverClosed(), *this, &SwgCuiChatWindow::onMessageBoxClosed);
		m_messageBoxResetDefaults = 0;

		if (box.completedAffirmative())
		{
			int size = static_cast<int>(m_tabVector->size());

			while (size--)
				delete removeTab(0);

			setupDefaultTabs(true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onMatchMakingMessageReceived(const Unicode::String &matchString)
{
	appendTextToChannel(ChannelId(CT_matchMaking), ClientTextManager::colorAndFilterText(matchString, ClientTextManager::TT_matchMaking, Game::isProfanityFiltered()));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onQuickMatchMessageReceived(const Unicode::String &matchString)
{
	appendTextToChannel(ChannelId(CT_matchMaking), ClientTextManager::colorAndFilterText(matchString, ClientTextManager::TT_matchMaking, Game::isProfanityFiltered()));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onWhoSearchResultMessageReceived(const PlayerCreatureController::Messages::CharacterMatchRetrieved::MatchResults &whoList)
{
	Unicode::String whiteColorCode(ClientTextManager::getColorCode(PackedRgb::solidWhite));

	if (whoList.m_matchingCharacterData.empty())
	{
		// No matching results

		appendTextToCurrentTab(whiteColorCode + CuiStringIdsWho::title.localize());
		appendTextToCurrentTab(whiteColorCode + CuiStringIdsWho::empty.localize());
	}
	else
	{
		appendTextToCurrentTab(whiteColorCode + CuiStringIdsWho::title.localize());

		// Dump the sorted result list

		WhoManager::UnicodeStringList unicodeStringList;
		WhoManager::buildResultStringList(whoList, unicodeStringList);

		WhoManager::UnicodeStringList::const_iterator iterUnicodeStringList = unicodeStringList.begin();

		for (; iterUnicodeStringList != unicodeStringList.end(); ++iterUnicodeStringList)
		{
			Unicode::String const &unicodeString =(*iterUnicodeStringList);

			appendTextToCurrentTab(unicodeString);
		}

		// Display the number of matches

		if (unicodeStringList.size() <= 1)
		{
			// Only one match

			appendTextToCurrentTab(whiteColorCode + CuiStringIdsWho::found_one.localize());
		}
		else
		{
			// More than one match

			CuiStringVariablesData data;
			data.digit_i = unicodeStringList.size();

			Unicode::String whoCountResultString;
			CuiStringVariablesManager::process(CuiStringIdsWho::found_many, data, whoCountResultString);

			appendTextToCurrentTab(whiteColorCode + whoCountResultString);
		}

		// Let the user know if the list was truncated

		if (whoList.m_hasMoreMatches)
		{
			appendTextToCurrentTab(whiteColorCode + CuiStringIdsWho::found_truncated.localize());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onWhoStatusMessageReceived(const PlayerObject::Messages::WhoStatusMessage::Message &message)
{
	appendTextToCurrentTab(ClientTextManager::colorText(message, ClientTextManager::TT_systemMessage));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onShowFriendListMessageReceived(const CommunityManager::Messages::ShowFriendListMessage::Status &)
{
	CommunityManager::FriendList const &friendList = CommunityManager::getFriendList();

	if (friendList.size() == 0)
	{
		// No friends

		appendTextToCurrentTab(CuiStringIdsWho::friend_title.localize());
		appendTextToCurrentTab(CuiStringIdsWho::friend_none.localize());
	}
	else
	{
		appendTextToCurrentTab(CuiStringIdsWho::friend_title.localize());

		typedef std::map<Unicode::String, CommunityManager::FriendData> FriendDataMap;
		typedef std::map<Unicode::String, FriendDataMap>                GroupSortedFriends;
		GroupSortedFriends groupSortedFriends;
		int friendHiddenCount = 0;

		// Sort all the friends into groups

		CommunityManager::FriendList::const_iterator iterFriends = friendList.begin();

		for (; iterFriends != friendList.end(); ++iterFriends)
		{
			CommunityManager::FriendData const &friendData = iterFriends->second;

			if (   !friendData.isNotifyOnlineStatus()
			    ||(CommunityManager::isHideOfflineFriends()
				&& !friendData.isOnline()))
			{
				++friendHiddenCount;
				continue;
			}

			Unicode::String group;

			if (friendData.getGroup().empty())
			{
				group = CuiStringIdsWho::friend_list_group_other.localize();
			}
			else
			{
				group = friendData.getGroup();
			}

			GroupSortedFriends::iterator iterGroupSortedFriends = groupSortedFriends.find(group);

			if (iterGroupSortedFriends != groupSortedFriends.end())
			{
				// Found this group, add the name to it

				iterGroupSortedFriends->second.insert(std::make_pair(Unicode::toLower(friendData.getName()), friendData));
			}
			else
			{
				// New group, create the new group and add the person to it

				FriendDataMap newFriendDataMap;
				newFriendDataMap.insert(std::make_pair(Unicode::toLower(friendData.getName()), friendData));
				groupSortedFriends.insert(std::make_pair(group, newFriendDataMap));
			}
		}

		// Display the friends list sorted by groups

		GroupSortedFriends::const_iterator iterGroupSortedFriends = groupSortedFriends.begin();

		for (; iterGroupSortedFriends != groupSortedFriends.end(); ++iterGroupSortedFriends)
		{
			// Show the current group name

			Unicode::String group;
			group += ClientTextManager::getColorCode(PackedRgb::solidGreen);
			group += iterGroupSortedFriends->first;
			group += ClientTextManager::getResetTagCode();
			appendTextToCurrentTab(group);

			FriendDataMap::const_iterator iterFriendDataMap = iterGroupSortedFriends->second.begin();

			for (; iterFriendDataMap != iterGroupSortedFriends->second.end(); ++iterFriendDataMap)
			{
				CommunityManager::FriendData const &friendData = iterFriendDataMap->second;
				Unicode::String result;
				result.append(1, ' ');
				result.append(1, ' ');
				result.append(1, ' ');
				result.append(1, ' ');
				result.append(1, ' ');

				if (friendData.isOnline())
				{
					result += ClientTextManager::getColorCode(ClientTextManager::TT_onlineStatus);
					result += friendData.getName();
					result += ClientTextManager::getResetTagCode();
				}
				else
				{
					result += friendData.getName();
				}

				appendTextToCurrentTab(result);
			}
		}

		Unicode::String friendCountString;

		if (friendList.size() == 1)
		{
			friendCountString += CuiStringIdsWho::friend_one.localize();
		}
		else
		{
			// More than one match

			CuiStringVariablesData data;
			data.digit_i = static_cast<int>(friendList.size());

			Unicode::String friendCountResultString;
			CuiStringVariablesManager::process(CuiStringIdsWho::friend_many, data, friendCountResultString);

			friendCountString += friendCountResultString;
		}

		// Show the hidden count

		CuiStringVariablesData data;
		data.digit_i = friendHiddenCount;

		Unicode::String friendHiddenOfflineString;
		CuiStringVariablesManager::process(CuiStringIdsWho::friend_hidden, data, friendHiddenOfflineString);

		friendCountString.append(1, ' ');
		friendCountString.append(1, ' ');
		friendCountString += friendHiddenOfflineString;

		appendTextToCurrentTab(friendCountString);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onFriendOnlineStatusChanged(const CommunityManager::Messages::FriendOnlineStatusChanged::Name &name)
{
	CuiStringVariablesData data;

	Unicode::String group;

	if (CommunityManager::getFriendGroup(name, group))
	{
		data.sourceName = name;
		data.sourceName.append(1, ' ');
		data.sourceName += group;
	}
	else
	{
		data.sourceName = name;
	}

	CuiUtils::FormatDate(data.otherName, CuiUtils::GetSystemSeconds());

	Unicode::String onlineStatusResultString;

	if (CommunityManager::isFriendOnline(name))
	{
		if (name.find('.') == std::string::npos)
		{
			// friend is from the same cluster
			CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_online, data, onlineStatusResultString);
		}
		else
		{
			// friend is from a different cluster or different game
			CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_online_remote, data, onlineStatusResultString);
		}
	}
	else
	{
		CuiStringVariablesManager::process(CuiStringIdsCommunity::friend_offline, data, onlineStatusResultString);
	}

	appendTextToCurrentTab(ClientTextManager::colorAndFilterText(onlineStatusResultString, ClientTextManager::TT_onlineStatus, false));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onShowIgnoreListMessageReceived(const CommunityManager::Messages::ShowIgnoreListMessage::Status &)
{
	CommunityManager::IgnoreList const &ignoreList = CommunityManager::getIgnoreList();

	if (ignoreList.size() == 0)
	{
		// No friends

		appendTextToCurrentTab(CuiStringIdsWho::ignore_title.localize());
		appendTextToCurrentTab(CuiStringIdsWho::ignore_none.localize());
	}
	else
	{
		// Display the list of people to ignore

		appendTextToCurrentTab(CuiStringIdsWho::ignore_title.localize());

		CommunityManager::IgnoreList::const_iterator iterIgnoreList = ignoreList.begin();

		for (; iterIgnoreList != ignoreList.end(); ++iterIgnoreList)
		{
			Unicode::String const &name = iterIgnoreList->second;
			appendTextToCurrentTab(name);
		}

		if (ignoreList.size() == 1)
		{
			appendTextToCurrentTab(CuiStringIdsWho::ignore_one.localize());
		}
		else
		{
			// More than one match

			CuiStringVariablesData data;
			data.digit_i = static_cast<int>(ignoreList.size());

			Unicode::String ignoreCountResultString;
			CuiStringVariablesManager::process(CuiStringIdsWho::ignore_many, data, ignoreCountResultString);

			appendTextToCurrentTab(ignoreCountResultString);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onCommandParserRequest(CuiMessageQueueManager::Messages::CommandParserRequest::Payload & payload)
{
	//-- ChatWindow needs to process commands for the active HUD at all times.
	if (Game::getHudSceneType() == m_sceneType)
	{
		if (!payload.first.empty())
		{
			std::set<Unicode::String> s;
			m_consoleHelper->processInput(Unicode::utf8ToWide(payload.first), s, false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::cursorMove(int distance, bool words)
{
	m_consoleHelper->cursorMove(distance, words);
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::cursorGotoEnd(int direction)
{
	m_consoleHelper->cursorGotoEnd(direction);
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::historyTraverse(int distance)
{
	m_consoleHelper->historyTraverse(distance);
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::commandComplete()
{
	m_consoleHelper->commandComplete();
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::performEnterKey()
{
	m_consoleHelper->processCurrentInput();

	//-- turn off input if we hit enter & you are the root window.
	if (isRootChatWindow()) 
	{
		acceptTextInput(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::performDeleteKey()
{
	m_consoleHelper->performDeleteKey();
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::performBackspaceKey()
{
	m_consoleHelper->performBackspaceKey();
	acceptTextInput(true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::fontSizeIncrement(int increment)
{
	const CuiFontSizer::SizeVector & sv = ConfigClientUserInterface::getChatWindowFontSizes();

	int sizeIndex = 0;
	m_outputBox->GetPropertyInteger(CuiFontSizer::Properties::FontIndex, sizeIndex);

	sizeIndex += increment;

	sizeIndex = std::min(static_cast<int>(sv.size()) - 1, sizeIndex);
	sizeIndex = std::max(0, sizeIndex);
	int size = 0;
	UITextStyle * const textStyle = CuiFontSizer::getTextStyle(*m_outputBox, sv, std::string(), sizeIndex, size);

	if (textStyle)
	{
		bool wasAtEnd = false;
		int scrolledToC = m_consoleHelper->getScrolledToCharacter(wasAtEnd);

		m_outputBox->SetStyle(textStyle);
		m_outputBox->SetPropertyInteger(CuiFontSizer::Properties::FontIndex, sizeIndex);
		m_outputBox->SetPropertyInteger(CuiFontSizer::Properties::FontSize,  size);

		if (isRootChatWindow())
			CuiChatManager::setChatWindowFontSizeDefaultIndex(sizeIndex);

		onChatFontSizeChanged();

		if (!wasAtEnd)
			m_consoleHelper->scrollToCharacter(scrolledToC);
		else
			m_consoleHelper->scrollToBottom();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::fontSizeIncrementAllWindows(int increment)
{
	Game::SceneType const sceneType = Game::getHudSceneType();
	
	for (ChatWindowSet::iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
	{
		SwgCuiChatWindow * const chatWindow = NON_NULL(*it);
		chatWindow->fontSizeIncrement(increment);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatFontSizeChanged()
{
	Game::SceneType const sceneType = Game::getHudSceneType();

	for (ChatWindowSet::iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
	{
		SwgCuiChatWindow * const chatWindow = NON_NULL(*it);
		if (chatWindow && chatWindow->isRootChatWindow()) 
		{
			CuiFontSizer::SizeVector const & sv = ConfigClientUserInterface::getChatWindowFontSizes();
			
			const int sizeIndex = CuiChatManager::getChatWindowFontSizeDefaultIndex();
			int size = 0;
			UITextStyle * const textStyle = CuiFontSizer::getTextStyle(*(chatWindow->m_outputBox), sv, std::string(), sizeIndex, size);
			
			if (textStyle)
			{
				chatWindow->m_outputBox->SetStyle(textStyle);
				chatWindow->m_outputBox->SetPropertyInteger(CuiFontSizer::Properties::FontIndex, sizeIndex);
				chatWindow->m_outputBox->SetPropertyInteger(CuiFontSizer::Properties::FontSize, size);
			}
			else
				WARNING(true,("SwgCuiChatWindow tried to default text style to null"));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::onChatBoxKeyClickChanged()
{
	Game::SceneType const sceneType = Game::getHudSceneType();
	
	for (ChatWindowSet::iterator it = ms_activeChatWindows[sceneType].begin(); it != ms_activeChatWindows[sceneType].end(); ++it)
	{
		SwgCuiChatWindow * const chatWindow = NON_NULL(*it);
		if (chatWindow && chatWindow->isRootChatWindow()) 
		{
			NON_NULL(chatWindow->m_inputBox)->SetKeyClickOn(CuiChatManager::getChatBoxKeyClick());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::saveSettings() const
{
	SwgCuiLockableMediator::saveSettings();

	//-- setup for all windows
	int const numTabs = static_cast<int>(m_tabVector->size());
	CuiSettings::saveInteger(getMediatorDebugName(), Settings::tabCount, numTabs);

	for (int i = 0; i < numTabs; ++i)
	{
		Tab const * const tab =(*m_tabVector)[i];
		tab->saveTabSettings(getMediatorDebugName(), i);
		tab->saveChatHistory(getMediatorDebugName(), i);
	}

	int const activeTabIndex = static_cast<int>(m_tabs->GetActiveTab());
	CuiSettings::saveInteger(getMediatorDebugName(), Settings::activeTab, activeTabIndex);
	
	//-- setup for torn off windows? look for existing windows and remove them?
	if (isRootChatWindow()) 
	{
		SwgCuiChatWindow::ChatWindowSet const & chatWindodws = ms_activeChatWindows[m_sceneType];
		
		int const numWindowsTornOff = static_cast<int>(chatWindodws.size() - 1);
		CuiSettings::saveInteger(getMediatorDebugName(), Settings::subWindows, numWindowsTornOff);
		
		char key[128];
		const size_t key_size = sizeof(key);
		
		int chatIndex = 0;
		for (ChatWindowSet::const_iterator it = chatWindodws.begin(); it != chatWindodws.end(); ++it)
		{
			SwgCuiChatWindow const * const chatWindow = *it;
			
			if (!chatWindow->isRootChatWindow()) 
			{
				snprintf(key, key_size, "%s_%d", Settings::subWindowIndex.c_str(), chatIndex++);
				CuiSettings::saveData(getMediatorDebugName(), key, chatWindow->getMediatorDebugName());
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::loadSettings()
{
	SwgCuiLockableMediator::loadSettings();

	deleteAllTabs();

	//-- setup for all windows
	int numTabs = -1;
	if (CuiSettings::loadInteger(getMediatorDebugName(), Settings::tabCount, numTabs))
	{
		std::set<Tab *> deletedTabs;

		for (int i = 0; i < numTabs; ++i)
		{
			Tab * const tab = new Tab;
			tab->loadTabSettings(getMediatorDebugName(), i);
			tab->loadChatHistory(getMediatorDebugName(), i);

			if (tab->channelsEmpty()) 
			{
				deletedTabs.insert(tab);
			}

			addTab(*tab);	
		} //lint !e429 // custodial pointer

		// remove deleted tabs.
		for (std::set<Tab *>::const_iterator iter = deletedTabs.begin(); iter != deletedTabs.end(); ++iter)
		{
			int const tabCount = getTabCount();

			for (int tabIdx = 0; tabIdx < tabCount; ++tabIdx)
			{
				Tab * const tab = getTab(tabIdx);
				
				if (tab == *iter)
				{
					delete removeTab(tabIdx);
					break;
				}
			}
		}
	}
	
	int activeTabIndex = 0;
	IGNORE_RETURN(CuiSettings::loadInteger(getMediatorDebugName(), Settings::activeTab, activeTabIndex));
	setActiveTab(0);
	setActiveTab(activeTabIndex);

	if (isRootChatWindow()) 
	{
		// no tabs? add the defaults.
		if (numTabs == -1) 
		{
			setupDefaultTabs(false);
		}

		char key[128];
		const size_t key_size = sizeof(key);
		
		int numWindowsTornOff = 0;
		if (CuiSettings::loadInteger(getMediatorDebugName(), Settings::subWindows, numWindowsTornOff))
		{
			for (int chatIndex = 0; chatIndex < numWindowsTornOff; ++chatIndex)
			{
				snprintf(key, key_size, "%s_%d", Settings::subWindowIndex.c_str(), chatIndex);
				
				std::string mediatorName;
				if (CuiSettings::loadData(getMediatorDebugName(), key, mediatorName))
				{
					//-- after creating the window with the appropriate name, 
					// the window will load its own settings
					IGNORE_RETURN(createInto(getContainingWorkspace(), m_sceneType, mediatorName));
				}
			}
		}
	}

	NOT_NULL(m_consoleHelper);
	m_consoleHelper->scrollToBottom();

	fetchTextForActiveTab();

	deactivateIfEmpty();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::deleteAllTabs()
{
	while (!m_tabVector->empty())
	{
		delete removeTab(0);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::updateDebugMediatorName()
{
	FormattedString<128> mediatorName;
	setMediatorDebugName(mediatorName.sprintf("%s_%d_%d_%d", s_mediatorName, time(NULL), ms_activeChatWindows[m_sceneType].size(), rand()));
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::deactivateIfEmpty()
{
	if (!isRootChatWindow() && (getTabCount() < 1))
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::OnHoverIn( UIWidget *Context )
{
	if (Context == &getPage())
	{
		setIsUpdating(false);
		m_outputBox->SetOpacity(1.0f);

		setGroundHudChatWindowOpacities(1.0f);
	}

	if (CuiPreferences::getChatBarFadesOut())
	{
		UIString value;
		getPage().GetProperty( UILowerString("ChatOnHoverIn"), value );
		UIManager::gUIManager().ExecuteScript( value, &getPage() );	
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::OnHoverOut( UIWidget *Context )
{
	if (!CuiPreferences::getChatBarFadesOut())
	{
		forceFadeIn();
		return;
	}
	
	UIString value;
	getPage().GetProperty( UILowerString("ChatOnHoverOut"), value );
	UIManager::gUIManager().ExecuteScript( value, &getPage() );
	
	if (Context == &getPage())
	{
		setIsUpdating(true);
		m_fadeTimer = 0.0f;
	}			
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::update(float deltaTimeSecs)
{
	Tab* activeTab = getActiveTab ();

	if(activeTab)
	{
		if(activeTab->getDoAppend())
		{
			m_consoleHelper->appendOutputText(activeTab->getAppendText(),false);

			activeTab->resetAppendText();
			activeTab->setDoAppend(false);
		}
	}

	if (!CuiPreferences::getChatBarFadesOut())
	{
		m_outputBox->SetOpacity(1.0f);

		setGroundHudChatWindowOpacities(1.0f);
		setIsUpdating(false);
	}
	else
	{

		m_fadeTimer += deltaTimeSecs;
		if (m_fadeTimer >(FADE_CHAT_TEXT_TIME + FADE_CHAT_TEXT_START_TIME))
		{
			m_outputBox->SetOpacity(FADE_CHAT_MIN_OPACITY);

			setGroundHudChatWindowOpacities(0.0f);
			setIsUpdating(false);
		}
		float opacity;
		if (m_fadeTimer < FADE_CHAT_TEXT_START_TIME)
			opacity = 1.0f;
		else
		{
			if (m_needToRunHoverOutScript)
			{
				UIString value;
				getPage().GetProperty( UILowerString("ChatOnHoverOut"), value );
				UIManager::gUIManager().ExecuteScript( value, &getPage() );
				m_needToRunHoverOutScript = false;
			}
			opacity = 1.0f -((m_fadeTimer - FADE_CHAT_TEXT_START_TIME) /(FADE_CHAT_TEXT_TIME));
		}

		const float lowerClampedVal = std::max(opacity,FADE_CHAT_MIN_OPACITY);
		m_outputBox->SetOpacity(lowerClampedVal);

		setGroundHudChatWindowOpacities(opacity);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::forceFadeIn()
{	
	setIsUpdating(false);
	m_outputBox->SetOpacity(1.0f);

	setGroundHudChatWindowOpacities(1.0f);

	if (CuiPreferences::getChatBarFadesOut())
	{
		setIsUpdating(true);
		m_fadeTimer = -FADE_CHAT_WAIT_AFTER_FORCE;
		m_needToRunHoverOutScript = true;
	}
}

//----------------------------------------------------------------------

SwgCuiChatWindow * SwgCuiChatWindow::createInto(CuiWorkspace * workspace, Game::SceneType sceneType, std::string const & name)
{
	SwgCuiChatWindow * chat = NULL;

	if (workspace) 
	{
		chat = createNewWindow(workspace->getPage(), sceneType, name);
	}

	return chat;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::setGroundHudChatWindowOpacities(float val)
{
	const float lowerClampedVal = std::max(val,FADE_CHAT_MIN_OPACITY);
	
	if(m_groundHudSkin)
	{
		m_groundHudSkin->SetOpacity(val);
	}

	if(m_groundHudScrollBar)
	{
		m_groundHudScrollBar->SetOpacity(val);
	}
	
	if(m_groundHudClose)
	{
		m_groundHudClose->SetOpacity(val);
	}

	if(m_groundHudChatBar)
	{
		m_groundHudChatBar->SetOpacity(lowerClampedVal);
	}

	if(m_groundHudChatChannel)
	{
		m_groundHudChatChannel->SetOpacity(val);
	}

	const long tabCount = m_tabs->GetTabCount();
	for(long index = 0; index < tabCount; ++index)
	{
		UIButton* button = m_tabs->GetTabButton(index);
		if(button)
		{
			UIPage * parent = dynamic_cast<UIPage *>(button->GetParent());
			if (parent)
			{
				parent->SetOpacity(val);
			}
				button->SetOpacity(val);
		}
	}
}

//======================================================================
