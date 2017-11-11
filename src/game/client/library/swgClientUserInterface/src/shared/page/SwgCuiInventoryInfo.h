// ======================================================================
//
// SwgCuiInventoryInfo.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventoryInfo_H
#define INCLUDED_SwgCuiInventoryInfo_H

// ======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"
#include "swgClientUserInterface/SwgCuiInventory.h"

class CachedNetworkId;
class ClientObject;
class CreatureObject;
class CuiIconManagerObjectProperties;
class CuiWidget3dObjectListViewer;
class DraftSchematicInfo;
class Object;
class PlayerObject;
class SwgCuiInventoryContainer;
class UIButton;
class UICheckbox;
class UIComposite;
class UIScrollbar;
class UIText;

template <typename T> class Watcher;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiInventoryInfo:
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:
	typedef Watcher<Object> ObjectWatcher;
	typedef std::pair<NetworkId, PlayerObject const * const> BiographyOwner;

	explicit                SwgCuiInventoryInfo                 (UIPage & page, bool alterObjects = false, bool isExamine = false);

	void                    receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message);

	void                    setInfoObject                       (Object * obj, bool requestAttributeUpdate = true);
	Object *                getInfoObject                       ();

	void                    connectToSelectionTransceiver       (CuiContainerSelectionChanged::TransceiverType & transceiver);
	void                    disconnectFromSelectionTransceiver  (CuiContainerSelectionChanged::TransceiverType & transceiver);

	void                    onSelectionChanged                  (const std::pair<int, ClientObject *> & payload);

	void                    onAttributesChanged                 (const NetworkId & id);
	void                    onDraftSchematicInfoChanged         (const DraftSchematicInfo & info);
	void                    onBiographyRetrieved                (BiographyOwner const &);

	virtual void            update                              (float elapsedTime);

	void                    setInventoryType                    (SwgCuiInventory::InventoryType type);

	void updateAttributeFlags();

	void                    setDropThroughTarget                (SwgCuiInventoryContainer *container);

	bool                    OnMessage                           (UIWidget * context, const UIMessage & msg);
	virtual void            OnButtonPressed                     (UIWidget *context);
	CuiWidget3dObjectListViewer *      getViewer				(void);

	void					OnCheckboxSet(UIWidget *context);
	void					OnCheckboxUnset(UIWidget *context);

	UICheckbox *            getHideAppearanceItemsCheckbox      ();

	static Object *         getLastInfoObject();
	static void             refreshNonAppearanceCreature();

protected:
	void                    performActivate ();
	void                    performDeactivate ();

private:
	void                    updatePlayerData();

private:
	                       ~SwgCuiInventoryInfo ();
	SwgCuiInventoryInfo ();
	SwgCuiInventoryInfo (const SwgCuiInventoryInfo & rhs);
	SwgCuiInventoryInfo & operator= (const SwgCuiInventoryInfo & rhs);

	void                    setPlayerInfo                       (CreatureObject * creatureObj);

	CuiWidget3dObjectListViewer *      m_viewer;
	UIText *                           m_label;
	UIComposite *                      m_content;
	UIText *                           m_textAttribs;
	UIText *                           m_textDesc;
	UIText *                           m_noTrade;
	UIText *                           m_tier;
	UIText *                           m_unique;
	UIButton *                         m_buttonCollections;
	UICheckbox *                       m_hideAppearanceItems;

	MessageDispatch::Callback *        m_callback;

	bool                               m_autoEnableContent;

	float                              m_defaultViewerPitch;

	ObjectWatcher *                    m_watcher;

	Unicode::String                    m_currentGuild;
	Unicode::String                    m_currentTitle;
	Unicode::String                    m_currentGuildTitle;
	Unicode::String                    m_currentBio;
	Unicode::String                    m_currentBadges;

	CuiIconManagerObjectProperties *   m_objectProperties;

	bool m_isPlayer;

	SwgCuiInventoryContainer *         m_container;
};

// ======================================================================

#endif
