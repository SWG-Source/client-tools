// ======================================================================
//
// SwgCuiInventory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventory_H
#define INCLUDED_SwgCuiInventory_H

// ======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class ClientObject;
class ClientObject;
class CuiWidget3dObjectViewer;
class NetworkId;
class Object;
class SwgCuiInventoryContainer;
class SwgCuiInventoryEquipment;
class SwgCuiInventoryInfo;
class UIButton;
class SwgCuiContainerProviderDefault;
class SwgCuiContainerProviderFilter;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiInventory :
public CuiMediator,
public UIEventCallback
{
public:

	enum InventoryType
	{
		IT_NORMAL,
		IT_LOOT,
		IT_LIGHTSABER,
		IT_PUBLIC,
		IT_CRAFTING,
		IT_MAX_TYPES
	};

	typedef stdvector<SwgCuiInventory *>::fwd InventoryVector;

	explicit                 SwgCuiInventory               (UIPage & page, ClientObject * container = 0, const std::string & slotName = std::string (), bool usePaperDoll = true, bool dontClose = false);

	virtual void             OnButtonPressed               (UIWidget *context );

	//-- CuiMediator
	bool                     close                         ();

	virtual void             saveSettings                  () const;
	virtual void             loadSettings                  ();

	void                     setContainerObject            (ClientObject * containerObject, const std::string & slotname);
	ClientObject *           getContainerObject            () const;

	static SwgCuiInventory * findSelectedInventoryPage     ();

	static SwgCuiInventory * findInventoryPageByContainer  (const NetworkId & containerId, const std::string & slotName);
	static void              findInventoryPagesByContainer (const NetworkId & containerId, const std::string & slotName, InventoryVector & iv);

	static SwgCuiInventory * createInto                    (UIPage & parent, ClientObject * container, const std::string & slotName, bool usePaperDoll, bool dontClose);

	void                     onObjectDestroyed             (const NetworkId & id);
	void                     onSelectionChanged            (const std::pair<int, ClientObject *> & msg);

	void                     update                        (float deltaTimeSecs);
	void                     setSendClose                  (bool sendClose);

	// For use in single player
	static void              setupPlayerInventory          (ClientObject * player);

	void                     openSelectedRadial            ();

protected:
	virtual void             performActivate               ();
	virtual void             performDeactivate             ();

private:
	//disabled
	                         ~SwgCuiInventory              ();
	                         SwgCuiInventory               ();
	                         SwgCuiInventory               (const SwgCuiInventory & rhs);
	SwgCuiInventory &        operator=                     (const SwgCuiInventory & rhs);

private:
	void                     applyStates                   () const;

private:
	SwgCuiInventoryEquipment *         m_eqMediator;
	SwgCuiInventoryContainer *         m_containerMediator;
	UIButton *                         m_togglePaperdollButton;
	UIButton *                         m_toggleExamineButton;

	bool                               m_dontClose;
	SwgCuiContainerProviderDefault *   m_containerProvider;
	SwgCuiContainerProviderFilter *    m_providerFilter;
	bool                               m_paperdollVisible;
	bool                               m_examineVisible;
	float                              m_timeSinceLastRangeCheck;
	bool                               m_sendClose;
	bool                               m_alreadyNotifiedUnsetContainer;
	SwgCuiInventoryInfo *              m_info;
	MessageDispatch::Callback *        m_callback;
	InventoryType                      m_inventoryType;
	bool                               m_onlyStoreBaseMediatorSettings;
};

// ======================================================================

#endif
