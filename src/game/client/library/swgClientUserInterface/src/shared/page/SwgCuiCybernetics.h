//===================================================================
//
// SwgCuiCybernetics.h
// copyright 2005, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiCybernetics_H
#define INCLUDED_SwgCuiCybernetics_H

//===================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"
#include "clientUserInterface/CuiMediator.h"

#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"

//===================================================================

class ClientObject;
class CuiWidget3dObjectListViewer;
class CuiWidget3dPaperdoll;
class SwgCuiInventoryContainer;
class SwgCuiContainerProviderDefault;
class SwgCuiContainerProviderFilter;
class UIButton;
class UICheckbox;
class UIComposite;
class UIList;
class UIPage;
class UIText;

//===================================================================

namespace MessageDispatch
{
	class Callback;
};

//===================================================================

class SwgCuiCybernetics : public UIEventCallback, public CuiMediator
{
public:
	explicit SwgCuiCybernetics(UIPage& page);

	bool OnMessage (UIWidget * context, UIMessage const & msg);
	void OnButtonPressed (UIWidget * context);
	void onInventorySelectionChanged (const CuiContainerSelectionChanged::Payload & payload);
	void OnCheckboxSet(UIWidget * context);
	void OnCheckboxUnset(UIWidget * context);
	virtual void performActivate ();
	virtual void performDeactivate ();
	void setPageOpenType(MessageQueueCyberneticsOpen::OpenType openType);
	void setNPC(NetworkId const & npc);

private:
	virtual ~SwgCuiCybernetics();

private:
	//disabled
	SwgCuiCybernetics();
	SwgCuiCybernetics(const SwgCuiCybernetics&);
	SwgCuiCybernetics& operator=(const SwgCuiCybernetics&);

private:
	void updateInstalledCybernetics();
	void setCyberneticsViewer(std::string const & cyberneticsSlot, ClientObject const & cyberneticsPiece);
	void clearAllCyberneticViewers();
	void clearCyberneticsViewer(std::string const & cyberneticsSlot);
	void updatePage();
	void selectCyberneticSlot(std::string const & cyberneticSlotName);
	void setSlotSelectionEnabled(bool enabled);
	bool isSlotSelectionEnabled() const;
	void clearCyberneticSlotOverlays();
	void updateViewers();
	void setupCharacterViewer (ClientObject * object);
	
private:
	MessageDispatch::Callback * m_callback;

	MessageQueueCyberneticsOpen::OpenType m_openType;

	SwgCuiInventoryContainer * m_containerMediator;
	SwgCuiContainerProviderDefault * m_containerProvider;
	SwgCuiContainerProviderFilter * m_providerFilter;

	UIButton * m_installButton;
	UIButton * m_repairButton;
	UIButton * m_uninstallButton;
	UIButton * m_cancelButton;
	UICheckbox * m_showAtabItems;
	UIText * m_characterName;
	UIPage * m_slotParentPage;
	UIPage * m_containerPage;
	UIComposite * m_composite;

	bool m_selectionEnabled;
	std::string m_selectedSlotName;
	NetworkId m_selectedInventoryObjectId;
	NetworkId m_npc;
	NetworkId m_selectedCyberneicSlotObjectId;
	CuiWidget3dPaperdoll * m_characterViewer;
};

//===================================================================

#endif
