//======================================================================
//
// SwgCuiHudWindowManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudWindowManager_H
#define INCLUDED_SwgCuiHudWindowManager_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"

class Camera;
class ClientObject;
class ClientProceduralTerrainAppearance;
class CreatureObject;
class CuiMediator;
class CuiWorkspace;
class MessageQueueCommandTimer;
class PermissionListCreateMessage;
class StartingLocationData;
class StringId;
class SwgCuiButtonBar;
class SwgCuiCharacterSheet;
class SwgCuiChatWindow;
class SwgCuiCombatQueue;
class SwgCuiDataStorage;
class SwgCuiHighlight;
class SwgCuiHud;
class SwgCuiInventory;
class SwgCuiLocationDisplay;
class SwgCuiMfdStatus;
class SwgCuiNotifications;
class SwgCuiPermissionList;
class SwgCuiQuestHelper;
class SwgCuiSurvey;
class SwgCuiTargets;
class SwgCuiTicketPurchase;
class SwgCuiToolbar;
class TangibleObject;
class UIPage;
class UIText;
struct ChatPersistentMessageToClientData;
struct UIPoint;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiHudWindowManager :
public MessageDispatch::Receiver
{
public:

	                    SwgCuiHudWindowManager                (const SwgCuiHud & hud, CuiWorkspace & workspace);
	virtual            ~SwgCuiHudWindowManager                () = 0;

	void                updateWindowManager                   (float deltaTimeSecs);

	virtual void        handlePerformActivate                 ();
	virtual void        handlePerformDeactivate               ();
	virtual void        receiveMessage                        (const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message);
	SwgCuiChatWindow *  getChatWindow                         ();
	void                receiveDroppedChatTab                 (const UIPoint & pt, float value);


	void                toggleInventory                       (ClientObject * container);
	void                acceptTextInput                       (bool setKeyboardInput);
	virtual void        update                                ();
	void                updateServerInfo                      ();

	void                toggleCommunity                       ();
	void                toggleService                         ();
	void                spawnTicketSubmission                 ();
	void                spawnHarassmentMessage                ();
	void                spawnHarassmentMessageFromKBSearch    ();
	void                spawnShipComponentManagement          (NetworkId const & shipId, NetworkId const & terminalId) const;
	void                spawnCustomizationWindow              (NetworkId const & objectId, std::string const &customVarName1, int minVar1, int maxVar1, std::string const &customVarName2, int minVar2, int maxVar2, std::string const &customVarName3, int minVar3, int maxVar3, std::string const &customVarName4, int minVar4, int maxVar4) const;
	void				spawnRatingWindow					  (std::string const & windowTitle, std::string const & windowText);


	void                colorTest                             (TangibleObject & id);
	void                hueObjectTest                         (const NetworkId & id, int maxIndex1, int maxIndex2);

	void                spawnCharacterSheet                   (CreatureObject *creatureObjectToExamine);
	void                spawnMissionDetails                   ();
	void                spawnPermissionList                   (const PermissionListCreateMessage& msg);
	void                spawnNewMacro                         (const Unicode::String& params);
	void                spawnConsent                          (const Unicode::String& question, int id) const;

	void                toggleButtonBar                       ();
	bool                isButtonBarVisible                    ();

	void				setPetToolbarVisible				  (const bool b);

	void                onBeginTrade                          (const bool & b);
	void                onTradeRequested                      (const NetworkId & other);
	void                onTradeRequestDenied                  (const NetworkId & recipient);
	void                onTradeRequestDeniedPlayerBusy        (const NetworkId & recipient);
	void                onTradeRequestDeniedPlayerUnreachable (const NetworkId & recipient);

	void                onItemOpenRequest                     (const std::pair<ClientObject *, std::string> & msg);
	void                onItemOpenRequestNewWindow            (const std::pair<ClientObject *, std::string> & msg);
	void                onItemCloseAllInstances               (const std::pair<ClientObject *, std::string> & msg);
	void                onCreatureDamageTaken                 (const CreatureObject & creature);
	void                onStartingLocationsReceived           (const stdvector<std::pair <StartingLocationData, bool> >::fwd & locations);
	
	void                displayPlayerDamageText               (int h, int a, int m);

	void                spawnChatRoomWho                      (uint32 roomId);

	void                onStartComposingNewMessage            (const ChatPersistentMessageToClientData & data);

	void spawnSpaceZoneMap(Unicode::String const & params) const;

	void activateChatWindow(bool val);
	static bool isSpaceChatVisible();
	static void setSpaceChatVisible(bool val);

protected:

	CuiWorkspace &      getWorkspace                          ();
	void                toggleMediator                        (CuiMediator & mediator);
	void                setBlinkingMediator                   (CuiMediator &mediator, float blinkTime);
	void setStatusMediator(SwgCuiMfdStatus * status);

	MessageDispatch::Callback *  m_callback;
	
private:

	                    SwgCuiHudWindowManager                (const SwgCuiHudWindowManager &);
	SwgCuiHudWindowManager &     operator=                    (const SwgCuiHudWindowManager &);

	SwgCuiInventory * createInventory (ClientObject * container, const std::string & slotName, bool usePaperDoll);
	void              updateHudOpacity ();
	UIPage * getToolbarPage();
	void cacheToolbar();
	SwgCuiToolbar *getCachedToolbar();

	SwgCuiChatWindow *         m_chatWindowMediator;
	SwgCuiMfdStatus *          m_mfdStatusMediator;
	SwgCuiToolbar *            m_toolbarMediator;
	SwgCuiNotifications *      m_notificationsMediator;
	SwgCuiHighlight *          m_highlightMediator;
	SwgCuiInventory *          m_inventory;
	SwgCuiButtonBar *          m_buttonBar;
	SwgCuiQuestHelper *        m_questHelper;

	CuiWorkspace *             m_workspace;

	int                        m_lastFrameUpdateMFD;

	bool                       m_WindowManagerActive;
	bool                       m_pendingBeginTrade;
	bool                       m_pendingRequestTrade;
	NetworkId                  m_pendingRequestTradeId;

	struct ItemRequestInfo
	{
		CachedNetworkId container;
		std::string     slotName;
		bool            newWindow;

		ItemRequestInfo (const ClientObject * _containerObject, const std::string & _slotName, bool _newWindow);
	};

	typedef stdvector<ItemRequestInfo>::fwd ItemRequestInfoVector;
	ItemRequestInfoVector *  m_pendingItemOpenInfo;
	ItemRequestInfoVector *  m_pendingItemCloseInfo;

	float                    m_lastHudOpacity;

	bool m_sendOpenCharacterSheet;
	bool m_sendCloseCharacterSheet;
	bool m_sendOpenStatMigration;
	bool m_sendCloseStatMigration;
	bool m_sendOpenInventory;
	bool m_sendCloseInventory;
	bool m_sendOpenHolocron;
	bool m_sendCloseHolocron;

	CuiMediator* m_blinkingMediator;
	float        m_blinkTime;

	UIPage * m_singleToolbarPage;
	UIPage * m_doubleToolbarPage;

	SwgCuiToolbar *m_singleToolbar;
	SwgCuiToolbar *m_doubleToolbar;
};

//----------------------------------------------------------------------

inline SwgCuiChatWindow *  SwgCuiHudWindowManager::getChatWindow()
{
	return m_chatWindowMediator;
}

//----------------------------------------------------------------------

inline CuiWorkspace & SwgCuiHudWindowManager::getWorkspace()
{
	return *NON_NULL(m_workspace);
}

//======================================================================

#endif


