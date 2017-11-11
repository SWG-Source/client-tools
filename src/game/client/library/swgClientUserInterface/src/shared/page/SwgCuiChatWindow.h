//======================================================================
//
// SwgCuiChatWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatWindow_H
#define INCLUDED_SwgCuiChatWindow_H

//======================================================================

#include "clientGame/CommunityManager.h"
#include "clientGame/GuildObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "clientGame/Game.h"

#include "UINotification.h"
#include "UITypes.h"

struct ChatPersistentMessageToClientData;

class ClientMapTerrainAppearanceTemplate;
class CommandParserHistory;
class CuiChatAvatarId;
class CuiChatParserStrategy;
class CuiChatRoomDataNode;
class CuiChatRoomManagerStatusMessage;
class CuiChatRoomMessage;
class CuiConsoleHelper;
class CuiConsoleHelperOutputGenerated;
class CuiConsoleHelperParserStrategy;
class CuiInstantMessageManagerElement;
class CuiMessageBox;
class CuiSystemMessageManagerData;
class CuiWidgetGroundRadar;
class MatchMakingResult;
class UIButton;
class UIImage;
class UIPage;
class UITabbedPane;
class UIText;
class UITextbox;
class Vector;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiChatWindow :
public SwgCuiLockableMediator,
public UINotification,
public MessageDispatch::Receiver
{
public:
 	//----------------------------------------------------------------------

	enum ChannelType
	{
		CT_none,
		CT_chatRoom,
		CT_spatial,
		CT_planet,
		CT_combat,
		CT_systemMessage,
		CT_instantMessage,
		CT_group,
		CT_matchMaking,
		CT_guild,
		CT_city,
		CT_quest,
		CT_gcw,

		// CT_named is the separator between the default/predefined channels
		// defined above and the named chat rooms that that the character has
		// entered; add new default/predefined channels immediately above CT_named
		CT_named,

		CT_numTypes
	};

	static bool isChannelTypeReadOnly (ChannelType type);

	//----------------------------------------------------------------------

	struct ChannelId;
	class Tab;
	class TabEditor;
	class TabChanged;

	typedef stdvector<Tab *>::fwd TabVector;
	typedef stdvector<std::string>::fwd StringVector;
	typedef stdvector<Unicode::String>::fwd UnicodeStringVector;
	typedef stdvector<SwgCuiChatWindow *>::fwd ChatWindowVector;
	typedef stdset<SwgCuiChatWindow *>::fwd ChatWindowSet;
	typedef stdset<int>::fwd ChatWindowIds;

	SwgCuiChatWindow(UIPage & page, Game::SceneType sceneType, std::string const & windowName = std::string());

	virtual bool OnMessage (UIWidget *context, const UIMessage & msg );
	virtual void Notify (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code );

	virtual void receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void acceptTextInput (bool b, bool setKeyboardInput = true, bool unfocusMediator = false);

	bool getPrintServerOutput () const;
	void setPrintServerOutput (bool b);

	virtual void OnTabbedPaneChanged (UIWidget * context);
	virtual void OnPopupMenuSelection (UIWidget * context);

	static void fontSizeIncrementAllWindows (int increment);
	static void onChatFontSizeChanged ();
	static void onChatBoxKeyClickChanged ();

	static void cloneFromTabDragged (const UIPoint & pt, int tabId);


	Tab * addTab (const ChannelId & id, const Unicode::String & value);
	void addTab (Tab & tab);

	const Tab * getTab (int index) const;
	const Tab * getActiveTab () const;
	void setActiveTab (int index);
	static const Tab * getTabDragged (SwgCuiChatWindow *& window, int & index);

	Tab * removeTab (int index);
	int getTabCount() const;
	void deactivateIfEmpty();

	void toggleCollapsed ();

	void onSystemMessageReceived (const CuiSystemMessageManagerData & msg);
	void onInstantMessageReceived (const CuiInstantMessageManagerElement & elem);
	void onInstantMessageFailed (const Unicode::String & msg);
	void onChatRoomMessageReceived (const CuiChatRoomMessage & message);
	void onCombatSpamReceived (std::pair<Unicode::String, int> const &payload);
	void onGCWScoreUpdatedThisGalaxy(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload const & obj);
	void onSocialReceived (const Unicode::String & str);
	void onSpatialChatReceived (const Unicode::String & str);
	void onPmReceived (const ChatPersistentMessageToClientData & data);
	void onChatRoomSelfEntered (const CuiChatRoomDataNode & id);
	void onChatRoomOtherEntered (const std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> & payload);
	void onChatRoomCreateFailed (const Unicode::String & payload);
	void onChatRoomJoinFailed (const Unicode::String & payload);

	void onChatRoomModeratorAdded (const std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> & payload);
	void onChatRoomModeratorRemoved (const std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> & payload);

	void onChatRoomSelfLeft (const CuiChatRoomDataNode & roomNode);
	void onChatRoomOtherLeft (const std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> & payload);
	void onChatRoomSelfLeftInternal (const CuiChatRoomDataNode & roomNode, bool printMsg);

	void onChatRoomCreated (const CuiChatRoomDataNode & chatRoom);
	void onChatRoomDestroyed (const std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> & payload);
	void onChatRoomDestroyFailed (const std::pair <const CuiChatRoomDataNode *, Unicode::String> & payload);
	void onPrelocalizedChat (const CuiPrelocalizedChatMessage & payload);
	void onPlanetRoomIdChanged (const uint32 & roomId);
	void onGroupRoomIdChanged (const uint32 & roomId);
	void onGuildRoomIdChanged (const uint32 & roomId);
	void onCityRoomIdChanged (const uint32 & roomId);
	void onNamedRoomIdChanged (const uint32 & roomId);

	void appendToAllTabs (const Unicode::String & str);
	void appendTextToChannel (const ChannelId & id, const Unicode::String & str, bool create = false);
	void appendTextToCurrentTab (const Unicode::String & str);

	void onTabEditingCompleted (const Tab & tab);

	void setupDefaultTabs (bool printMessage);

	void nextTab ();
	void prevTab ();

	static Tab * staticFindChatTabByTabId (int tabId, SwgCuiChatWindow * & chatWindow, int & index);

	static bool activateChatTabByTabId (int id);
	int findChatTabByTabId (int id) const;

	void handleChatRoomJoinDefault (const CuiChatRoomDataNode & roomNode);
	void handleChatRoomLeave (const CuiChatRoomDataNode & roomNode, bool printMsg);

	void onDebugPrintUi (const Unicode::String & str);
	void onChatStartInput (const Unicode::String & str);

	void onChatRoomStatusMessage (const CuiChatRoomManagerStatusMessage & msg);
	void onConsoleHelperOutput (const CuiConsoleHelperOutputGenerated & msg);

	void onMessageBoxClosed (const CuiMessageBox & box);

	void onMatchMakingMessageReceived (const Unicode::String &matchString);
	void onQuickMatchMessageReceived (const Unicode::String &matchString);
	void onWhoSearchResultMessageReceived (const PlayerCreatureController::Messages::CharacterMatchRetrieved::MatchResults &whoList);
	void onWhoStatusMessageReceived (const PlayerObject::Messages::WhoStatusMessage::Message &message);
	void onShowFriendListMessageReceived (const CommunityManager::Messages::ShowFriendListMessage::Status &status);
	void onFriendOnlineStatusChanged (const CommunityManager::Messages::FriendOnlineStatusChanged::Name &name);
	void onShowIgnoreListMessageReceived (const CommunityManager::Messages::ShowIgnoreListMessage::Status &status);

	void onCommandParserRequest (CuiMessageQueueManager::Messages::CommandParserRequest::Payload & payload);

	void cursorMove (int distance, bool words);
	void cursorMove (int distance);
	void cursorGotoEnd (int direction);
	void historyTraverse (int distance);
	void commandComplete ();
	void performEnterKey ();
	void performDeleteKey ();
	void performBackspaceKey ();
	void fontSizeIncrement (int increment);

	void saveSettings () const;
	void loadSettings ();

	Game::SceneType getSceneType() const;
	bool isRootChatWindow() const;

	virtual void OnHoverIn( UIWidget *Context );
	virtual void OnHoverOut( UIWidget *Context );
	void update(float deltaTimeSecs);

	static SwgCuiChatWindow * createInto(CuiWorkspace * workspace, Game::SceneType sceneType, std::string const & name = std::string());
	Tab * getActiveTab ();
	Tab * getLcdTab ();

protected:
	virtual void performActivate ();
	virtual void performDeactivate ();

	void updateDebugMediatorName();

	void deleteAllTabs();

private:
	virtual ~SwgCuiChatWindow ();
	 SwgCuiChatWindow ();
	 SwgCuiChatWindow (const SwgCuiChatWindow &);
	SwgCuiChatWindow & operator= (const SwgCuiChatWindow &);

	void fetchTextForActiveTab ();
	Tab * getTab (int index);
	void editChannels (int index);

	void updateActiveDefaultChannel ();

	void forceFadeIn();

	void setGroundHudChatWindowOpacities(float val);

	SwgCuiChatWindow * cloneFromTab (int index, const UIPoint & pt = UIPoint (16L, 16L), bool tearOff = false);
	static SwgCuiChatWindow * createNewWindow(UIPage & parentPage, Game::SceneType sceneType, std::string const & windowName = std::string());

	UITextbox * m_inputBox;
	UIText * m_outputBox;

	// controls to fade out in the groundhud chatwindow
	UIPage * m_groundHudSkin;
	UIPage * m_groundHudScrollBar;
	UIButton * m_groundHudClose;
	UITextbox * m_groundHudChatBar;
	UIText * m_groundHudChatChannel;

	UITabbedPane * m_tabs;

	UIButton * m_buttonCollapse;
	UIButton * m_buttonExpand;
	UIPage * m_collapsingPage;
	UIPage * m_persistentPage;

	CommandParserHistory * m_history;
	CuiChatParserStrategy * m_parserStrategy;
	CuiConsoleHelper * m_consoleHelper;

	bool m_printServerOutput;

	class SwgCuiChatWindowAction;
	SwgCuiChatWindowAction * m_action;

	MessageDispatch::Callback * m_callback;

	UIText * m_textChannel;
	TabVector * m_tabVector;
	int m_popupTabIndex;
	CuiMessageBox * m_messageBoxResetDefaults;

	Game::SceneType m_sceneType;

	bool m_rootWindow;

	float m_fadeTimer;
	bool m_needToRunHoverOutScript;
};

//-----------------------------------------------------------------

inline bool SwgCuiChatWindow::getPrintServerOutput () const
{
	return m_printServerOutput;
}

//-----------------------------------------------------------------

inline void SwgCuiChatWindow::setPrintServerOutput (bool b)
{
	m_printServerOutput = b;
}

//----------------------------------------------------------------------

inline bool SwgCuiChatWindow::isChannelTypeReadOnly (ChannelType type)
{
	return type >= CT_combat;
}

//----------------------------------------------------------------------

inline Game::SceneType SwgCuiChatWindow::getSceneType() const
{
	return m_sceneType;
}

//----------------------------------------------------------------------

inline bool SwgCuiChatWindow::isRootChatWindow() const
{
	return m_rootWindow;
}

//======================================================================

#endif
