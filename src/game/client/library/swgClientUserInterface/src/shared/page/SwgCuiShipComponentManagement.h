//======================================================================
//
// SwgCuiShipComponentManagement.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiShipComponentManagement_H
#define INCLUDED_SwgCuiShipComponentManagement_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ShipObject.h"

//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer;
class ShipChassisSlot;
class ShipObject;
class UIButton;
class UIDataSourceContainer;
class UIPage;
class UIText;
class UITreeView;

namespace MessageDispatch
{
	class Callback;
}

namespace ShipChassisSlotType
{
	enum Type;
}

//----------------------------------------------------------------------

class SwgCuiShipComponentManagement :
public CuiMediator,
public UIEventCallback
{
public:
	enum SetShipResult
	{
		SSR_noShip,
		SSR_notCertifiedForShip,
		SSR_newbieShip,
		SSR_nonCombatShip,
		SSR_ok
	};

public:
	explicit SwgCuiShipComponentManagement(UIPage & page);
	SetShipResult setShip(ShipObject * ship);
	void setTerminal(NetworkId const & nid);

	virtual void update (float deltaTimeSecs);
	virtual bool OnMessage(UIWidget* context, UIMessage const & msg);
	virtual void OnGenericSelectionChanged (UIWidget * context);
	virtual void OnButtonPressed(UIWidget * context);
	virtual void OnTreeRowExpansionToggled(UIWidget * context, int row);
	void onComponentsChanged (ShipObject::Messages::ComponentsChanged::Payload & ship);
	void onDroidPcdChanged(ShipObject::Messages::DroidPcdChanged::Payload & ship);
	void onObjectAddedToContainer(const ClientObject::Messages::ContainerMsg & payload);
	void onObjectRemovedFromContainer(const ClientObject::Messages::ContainerMsg & payload);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	void clearUI();
	void populateUI();
	void rebuildTree();
	void addInventoryComponentsForSlotToTreeView(ShipChassisSlot const & slot, UIDataSourceContainer & container);
	void addInventoryComponentsForSlotToTreeViewRecursive(ShipChassisSlot const & slot, UIDataSourceContainer & container, ClientObject const & inventoryObject);
	bool shouldCheckContainerForComponents(ClientObject const & inventoryObject);
	void installSelectedComponent();
	void uninstallSlot(ShipChassisSlotType::Type slot) const;
	void updateSelectedComponent(NetworkId const & object);
	void updateInstalledComponent(ShipChassisSlotType::Type slot);
	void updateInstalledComponent(NetworkId const & objectId);
	void rebuildOpenTreeBranches();
	void rebuildTreeShipSlot(UIDataSourceContainer & slotBaseContainer);
	void updateCurrentSelectedAndInstalledSlots();
	void updateBaseShipInformation();
	void repairSlot(ShipChassisSlotType::Type slotType) const;
	void addMissileAmmunitionToSlot(UIDataSourceContainer & container, ShipChassisSlotType::Type slotType);
	void addMissileAmmunitionToSlotRecursive(UIDataSourceContainer & container, ClientObject const & inventoryObject, ShipChassisSlotType::Type slotType);
	void addCountermeasurePacksToSlot(UIDataSourceContainer & container, ShipChassisSlotType::Type slotType);
	void addCountermeasurePacksToSlotRecursive(UIDataSourceContainer & container, ClientObject const & inventoryObject, ShipChassisSlotType::Type slotType);
	bool isAMissilePack(NetworkId const & object) const;
	bool isACountermeasurePack(NetworkId const & objectId) const;
	void loadConsumablePack(NetworkId const & object, ShipChassisSlotType::Type slotType) const;
	void addDroidSlot(UIDataSourceContainer & container) const;
	UIDataSourceContainer & addTreeItem(UIDataSourceContainer & parent, std::string const & internalName, Unicode::String const & displayName, std::string const & slotName) const;
	void addInstalledShipComponentToTree(UIDataSourceContainer & parent, Unicode::String const & displayName, std::string const & slotName) const;
	bool isADroidControlDevice(NetworkId const & objNid) const;
	void associateDroidWithShip(NetworkId const & droidControlDeviceId) const;
	void unloadCurrentDroidFromShip();
	bool canModifyShip() const;
	NetworkId const & getCommandTableTarget() const;
	bool isAnObjectWeCareAbout(ClientObject const & co) const;
	float getEnergyUsageOfShip() const;

private:
	//disabled
	SwgCuiShipComponentManagement(SwgCuiShipComponentManagement const & rhs);
	SwgCuiShipComponentManagement & operator= (SwgCuiShipComponentManagement const & rhs);

private:
	virtual ~SwgCuiShipComponentManagement();

private:
	MessageDispatch::Callback * m_callback;
	Watcher<ShipObject> m_ship;
	CuiWidget3dObjectListViewer * m_shipViewer;
	UITreeView * m_tree;
	float m_timer;
	UIText * m_selectedComponentText;
	UIText * m_installedComponentText;
	UIText * m_selectedComponentName;
	UIText * m_installedComponentName;
	UIButton * m_loadButton;
	UIButton * m_unloadButton;
	UIButton * m_repairButton;
	UIButton * m_repairChassisButton;
	UIText * m_massText;
	UIText * m_powerText;
	UIText * m_energyText;
	UIText * m_chassisText;
	UIText * m_notCertifiedForShipText;
	UIButton * m_closeButton;
	UIPage * m_pageNotNearTerminal;
};

//======================================================================

#endif
