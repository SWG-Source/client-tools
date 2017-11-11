//======================================================================
//
// SwgCuiShipComponentManagement.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiShipComponentManagement.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsShipComponent.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/Container.h"
#include "UIButton.h"
#include "UIDataSourceContainer.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITreeView.h"
#include "UITreeView_DataNode.h"
#include "UIWidget.h"

//======================================================================

namespace SwgCuiShipComponentManagementNamespace
{
	ShipChassisSlotType::Type ms_currentSlot = ShipChassisSlotType::SCST_invalid;
	bool ms_currentSlotIsDroid = false;
	NetworkId ms_currentComponent = NetworkId::cms_invalid;
	NetworkId ms_terminal;
	bool ms_energyUsageDirty = false;
	float ms_currentEnergyUsage = 0.0f;

	int const cms_numNewbieShips = 3;
	std::string cms_newbieShips[cms_numNewbieShips] = {"object/ship/player/shared_player_basic_tiefighter.iff", "object/ship/player/shared_player_basic_hutt_light.iff", "object/ship/player/shared_player_basic_z95.iff"};
	int const cms_numNonCombatShips = 1;
	std::string cms_noncombatShips[cms_numNonCombatShips] = {"object/ship/player/shared_player_sorosuub_space_yacht.iff"};

	bool ms_badShipSet = false;
	bool ms_rebuildingTree = false;

	float const cms_updateTime = 0.25f;
	int const cms_treeViewBadRow = -1;
	std::string const cms_installed("installed");
	std::string const cms_energyRequiredAttribName("ship_component.ship_component_energy_required");
	std::string const cms_massAttribName("ship_component.ship_component_mass");
	std::string const cms_missileLauncherCompatability("wpn_1");
	std::string const cms_countermeasureLauncherCompatability("cms_0");
	std::string const cms_ammuntionSlotName("ammunition");
	std::string const cms_countermeasurePacksSlotName("countermeasurepacks");
	std::string const cms_droidSlotName("droid");

	std::string const cms_installCommandName("installShipComponent");
	std::string const cms_uninstallCommandName("uninstallShipComponent");
	std::string const cms_repairCommandName("repairShipComponentInSlot");
	std::string const cms_loadItemOntoSlotCommandName("insertItemIntoShipComponentSlot");
	std::string const cms_associateDroidControlDeviceWithShipCommandName("associateDroidControlDeviceWithShip");

	std::string const cms_shipComponentAttributePre("ship_component.");

	float const cms_maxRangeFromTerminal = 16.0f;

	namespace DataProperties
	{
		UILowerString const SlotName("SlotName");
	};//lint !e19 useless declaration (no, it's used)


	std::vector<UIDataSourceContainer const *> ms_nodesToIgnore;
}

using namespace SwgCuiShipComponentManagementNamespace;

//======================================================================

SwgCuiShipComponentManagement::SwgCuiShipComponentManagement (UIPage & page) :
CuiMediator ("SwgCuiShipComponentManagement", page),
UIEventCallback (),
m_callback (new MessageDispatch::Callback),
m_ship(NULL),
m_shipViewer(NULL),
m_tree(NULL),
m_timer(0.0f),
m_selectedComponentText(NULL),
m_installedComponentText(NULL),
m_selectedComponentName(NULL),
m_installedComponentName(NULL),
m_loadButton(NULL),
m_unloadButton(NULL),
m_repairButton(NULL),
m_repairChassisButton(NULL),
m_massText(NULL),
m_powerText(NULL),
m_energyText(NULL),
m_chassisText(NULL),
m_notCertifiedForShipText(NULL),
m_closeButton(NULL),
m_pageNotNearTerminal(NULL)
{
	UIWidget * wid = 0;
	//get the ship viewer (paperdoll)
	getCodeDataObject (TUIWidget, wid, "shipviewer");
	m_shipViewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	m_shipViewer->setAlterObjects(false);

	getCodeDataObject (TUITreeView, m_tree, "treeinventory");
	m_tree->ClearData();
	ms_nodesToIgnore.clear();

	getCodeDataObject (TUIText, m_installedComponentText, "installedobjecttext");
	m_installedComponentText->Clear();
	
	getCodeDataObject (TUIText, m_selectedComponentText, "selectedobjecttext");
	m_selectedComponentText->Clear();
	
	getCodeDataObject (TUIText, m_selectedComponentName, "selectedcomponentname");
	m_selectedComponentName->Clear();

	getCodeDataObject (TUIText, m_installedComponentName, "installedcomponentname");
	m_installedComponentName->Clear();

	getCodeDataObject (TUIButton, m_loadButton, "buttonload");
	registerMediatorObject(*m_loadButton, true);

	getCodeDataObject (TUIButton, m_unloadButton, "buttonunload");
	registerMediatorObject(*m_unloadButton, true);

	getCodeDataObject (TUIButton, m_repairButton, "buttonrepair");
	registerMediatorObject(*m_repairButton, true);

	getCodeDataObject (TUIButton, m_repairChassisButton, "buttonrepairchassis");
	registerMediatorObject(*m_repairChassisButton, true);
	m_repairChassisButton->SetVisible(false);

	getCodeDataObject (TUIText, m_massText, "masstext");
	m_massText->Clear();
	getCodeDataObject (TUIText, m_powerText, "powertext");
	m_powerText->Clear();
	getCodeDataObject (TUIText, m_energyText, "energytext");
	m_energyText->Clear();

	getCodeDataObject (TUIText, m_chassisText, "chassistext");
	m_chassisText->Clear();

	getCodeDataObject (TUIText, m_notCertifiedForShipText, "notcertifiedforshiptext");
	m_notCertifiedForShipText->SetVisible(false);
	
	getCodeDataObject (TUIButton, m_closeButton, "buttonDone");
	registerMediatorObject(*m_closeButton, true);

	getCodeDataObject (TUIPage, m_pageNotNearTerminal, "pageNotNearTerminal");
	m_pageNotNearTerminal->SetVisible(false);
	UIBaseObject * const parent = m_pageNotNearTerminal->GetParent();
	if(parent)
		parent->Link();

	clearUI();

	setState (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiShipComponentManagement::~SwgCuiShipComponentManagement ()
{
	m_ship = NULL;
	m_shipViewer = NULL;
	m_installedComponentText = NULL;
	m_selectedComponentText = NULL;
	m_selectedComponentName = NULL;
	m_installedComponentName = NULL;
	m_loadButton = NULL;
	m_unloadButton = NULL;
	m_repairButton = NULL;
	m_massText = NULL;
	m_powerText = NULL;
	m_energyText = NULL;
	m_chassisText = NULL;
	m_closeButton = NULL;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::performActivate ()
{
	//if a bad ship was set into us, we need to close the page
	if(ms_badShipSet)
	{
		deactivate();
	}

	CuiManager::requestPointer (true);
	m_shipViewer->setPaused (false);

	m_tree->AddCallback(this);

	populateUI();

	setIsUpdating(true);

	m_loadButton->SetEnabled(false);
	m_unloadButton->SetEnabled(false);

	m_callback->connect (*this, &SwgCuiShipComponentManagement::onComponentsChanged, static_cast<ShipObject::Messages::ComponentsChanged *>(0));
	m_callback->connect (*this, &SwgCuiShipComponentManagement::onDroidPcdChanged, static_cast<ShipObject::Messages::DroidPcdChanged *>(0));
	m_callback->connect (*this, &SwgCuiShipComponentManagement::onObjectAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>(0));
	m_callback->connect (*this, &SwgCuiShipComponentManagement::onObjectRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *>(0));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::performDeactivate ()
{
	clearUI();
	
	m_callback->disconnect (*this, &SwgCuiShipComponentManagement::onComponentsChanged, static_cast<ShipObject::Messages::ComponentsChanged *>(0));
	m_callback->disconnect (*this, &SwgCuiShipComponentManagement::onDroidPcdChanged, static_cast<ShipObject::Messages::DroidPcdChanged *>(0));
	m_callback->disconnect (*this, &SwgCuiShipComponentManagement::onObjectAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>(0));
	m_callback->disconnect (*this, &SwgCuiShipComponentManagement::onObjectRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *>(0));

	setIsUpdating(false);

	m_tree->RemoveCallback(this);

	m_shipViewer->setPaused (true);

	CuiManager::requestPointer (false);
}

//-----------------------------------------------------------------------

void SwgCuiShipComponentManagement::OnGenericSelectionChanged (UIWidget * const context)
{
	if(!m_ship)
		return;
	m_loadButton->SetEnabled(false);
	m_unloadButton->SetEnabled(false);

	if(context == m_tree)
	{
		bool currentSlotSet = false;
		long val = m_tree->GetLastSelectedRow();
		if(val != cms_treeViewBadRow)
		{
			UIDataSourceContainer const * const dsc = m_tree->GetDataSourceContainerAtRow(val);
			if(dsc)
			{
				std::string const name = dsc->GetName();
				//the container's name is either - a networkId, "installed", or a slot name
				NetworkId const nid(name.c_str());
				//if a networkid, it's an uninstalled component
				if(nid.isValid())
				{
					updateSelectedComponent(nid);
					m_loadButton->SetEnabled(true);
					UIDataSourceContainer const * const parentDsc = dynamic_cast<UIDataSourceContainer const *>(dsc->GetParent());
					if(parentDsc)
					{
						std::string const parentName = parentDsc->GetName();
						if(isADroidControlDevice(nid))
						{
							m_loadButton->SetEnabled(true);
							if(m_ship->getInstalledDroidControlDevice().isValid())
								m_unloadButton->SetEnabled(true);
							updateInstalledComponent(m_ship->getInstalledDroidControlDevice());
							ms_currentSlot = ShipChassisSlotType::SCST_invalid;
							ms_currentSlotIsDroid = true;
							currentSlotSet = true;
						}
						else
						{
							//this should be a slot name
							ShipChassisSlotType::Type const slot = ShipChassisSlotType::getTypeFromName (parentName);
							if(slot != ShipChassisSlotType::SCST_invalid)
							{
								updateInstalledComponent(slot);
								if(m_ship->isSlotInstalled(slot))
									m_unloadButton->SetEnabled(true);
								ms_currentSlot = slot;
								ms_currentSlotIsDroid = false;
								currentSlotSet = true;
							}
						}
					}
				}
				//if "installed", it's an installed component
				else if(name == cms_installed)
				{
					updateSelectedComponent(nid);
					UIDataSourceContainer const * const parentDsc = dynamic_cast<UIDataSourceContainer const *>(dsc->GetParent());
					if(parentDsc)
					{
						std::string const parentName = parentDsc->GetName();
						//this should be a slot name
						ShipChassisSlotType::Type const slot = ShipChassisSlotType::getTypeFromName (parentName);
						if(slot != ShipChassisSlotType::SCST_invalid)
						{
							updateInstalledComponent(slot);
							updateSelectedComponent(NetworkId::cms_invalid);
							ms_currentSlot = slot;
							ms_currentSlotIsDroid = false;
							currentSlotSet = true;
							m_unloadButton->SetEnabled(true);
						}
					}
				}
				//else it must be a base slot name node
				else
				{
					ShipChassisSlotType::Type const slot = ShipChassisSlotType::getTypeFromName (name);
					if(slot != ShipChassisSlotType::SCST_invalid)
					{
						updateInstalledComponent(slot);
						if(m_ship->isSlotInstalled(slot))
							m_unloadButton->SetEnabled(true);
						updateSelectedComponent(NetworkId::cms_invalid);
						ms_currentSlot = slot;
						ms_currentSlotIsDroid = false;
						currentSlotSet = true;
					}
					if(name == cms_droidSlotName)
					{
						if(m_ship->getInstalledDroidControlDevice().isValid())
							m_unloadButton->SetEnabled(true);
						ms_currentSlot = ShipChassisSlotType::SCST_invalid;
						ms_currentSlotIsDroid = true;
					}
				}
			}
		}
		if(!currentSlotSet)
		{
			ms_currentSlot = ShipChassisSlotType::SCST_invalid;
			ms_currentSlotIsDroid = false;
		}
	}

	if(!canModifyShip())
	{
		m_loadButton->SetEnabled(false);
		m_unloadButton->SetEnabled(false);
	}
}

//-----------------------------------------------------------------------

void SwgCuiShipComponentManagement::OnButtonPressed(UIWidget * const context)
{
	if(context == m_closeButton)
	{
		closeThroughWorkspace();
	}
	if(context == m_loadButton)
	{
		installSelectedComponent();
	}
	else if(context == m_unloadButton)
	{
		bool isADroid = false;
		long const val = m_tree->GetLastSelectedRow();
		if(val != cms_treeViewBadRow)
		{
			//the component node is the selected row
			UIDataSourceContainer const * const componentDsc = m_tree->GetDataSourceContainerAtRow(val);
			if(componentDsc)
			{
				//the slot node is its parent
				UIDataSourceContainer const * const slotDsc = dynamic_cast<UIDataSourceContainer const *>(componentDsc->GetParent());
				if(slotDsc)
				{
					Unicode::String slotNameWide;
					slotDsc->GetProperty(DataProperties::SlotName, slotNameWide);
					NetworkId objId(componentDsc->GetName());
					if(isADroidControlDevice(objId))
					{
						associateDroidWithShip(NetworkId::cms_invalid);
						isADroid = true;
					}
				}
			}
		}
		if(!isADroid)
			uninstallSlot(ms_currentSlot);
	}
	else if(context == m_repairButton)
	{
		if(ms_currentSlot != ShipChassisSlotType::SCST_invalid)
			repairSlot(ms_currentSlot);
	}
	else if(context == m_repairChassisButton)
	{
		repairSlot(ShipChassisSlotType::SCST_invalid);
	}
}

//-----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if(context == m_tree)
		{
			long const val = m_tree->GetLastSelectedRow();
			if(val != cms_treeViewBadRow)
			{
				if(canModifyShip())
					installSelectedComponent();
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::OnTreeRowExpansionToggled(UIWidget * context, int row)
{
	if(context == m_tree)
	{
		UIDataSourceContainer * const dsrc = m_tree->GetDataSourceContainerAtRow(row);
		if(dsrc)
			rebuildTreeShipSlot(*dsrc);
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::repairSlot(ShipChassisSlotType::Type const slotType) const
{
	if(!m_ship)
		return;
	char slotName[256];
	_itoa(slotType, slotName, 10);

	char buf[256];
	//params for repairShipComponentInSlot are "<shipId> <slotnumber>"
	_snprintf (buf, 255, "%s %s", m_ship->getNetworkId().getValueString().c_str(), slotName);

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cms_repairCommandName.c_str(), getCommandTableTarget(), Unicode::narrowToWide(buf)));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::uninstallSlot(ShipChassisSlotType::Type const slot) const
{
	if(!m_ship)
		return;
	char slotName[256];
	_itoa(slot, slotName, 10);

	char buf[256];
	//params for uninstallShipComponent are "<shipId> <slotnumber>"
	_snprintf (buf, 255, "%s %s", m_ship->getNetworkId().getValueString().c_str(), slotName);

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cms_uninstallCommandName.c_str(), getCommandTableTarget(), Unicode::narrowToWide(buf)));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::installSelectedComponent()
{
	if(!m_ship)
		return;

	long const val = m_tree->GetLastSelectedRow();
	if(val != cms_treeViewBadRow)
	{
		//the component node is the selected row
		UIDataSourceContainer const * const componentDsc = m_tree->GetDataSourceContainerAtRow(val);
		if(componentDsc)
		{
			std::vector<UIDataSourceContainer const *>::const_iterator const i = std::find(ms_nodesToIgnore.begin(), ms_nodesToIgnore.end(), componentDsc);
			if(i != ms_nodesToIgnore.end())
				return;

			//the slot node is its parent
			UIDataSourceContainer const * const slotDsc = dynamic_cast<UIDataSourceContainer const *>(componentDsc->GetParent());
			if(slotDsc)
			{
				Unicode::String slotNameWide;
				slotDsc->GetProperty(DataProperties::SlotName, slotNameWide);
				ShipChassisSlotType::Type const slotType = ShipChassisSlotType::getTypeFromName (Unicode::wideToNarrow(slotNameWide));
				NetworkId objNid(componentDsc->GetName());

				if(isADroidControlDevice(objNid))
				{
					associateDroidWithShip(objNid);
					return;
				}

				if(isAMissilePack(objNid) || isACountermeasurePack(objNid))
				{
					loadConsumablePack(objNid, slotType);
					return;
				}

				//check if the ship would be too heavy (EAS TODO check that server validates this also)
				std::vector<std::pair<std::string, Unicode::String> > attribs;
				bool const result = ObjectAttributeManager::getAttributes(objNid, attribs, false, true);
				if(result)
				{
					Unicode::String massText;
					for(std::vector<std::pair<std::string, Unicode::String> >::iterator i = attribs.begin(); i != attribs.end(); ++i)
					{
						if(i->first == cms_massAttribName)
						{
							massText = i->second;
							break;
						}
					}
					std::string const & massNarrow = Unicode::wideToNarrow(massText);
					float componentMass = 0.0f;
					int const count = sscanf(massNarrow.c_str(), "%f", &componentMass);
					if(count == 1)
					{
						float const currentMass = m_ship->getChassisComponentMassCurrent();
						float const existingMass = m_ship->getComponentMass(slotType);
						float const newMass = currentMass + componentMass - existingMass;
						if(newMass > m_ship->getChassisComponentMassMaximum())
						{
							CuiMessageBox::createInfoBox (CuiStringIdsShipComponent::err_too_heavy.localize ());
							return;
						}
					}
				}

				//check certification (EAS TODO check that server validates this also)
				Object const * const o = NetworkIdManager::getObjectById(objNid);
				ClientObject const * const co = o ? o->asClientObject() : NULL;
				TangibleObject const * const to = co ? co->asTangibleObject() : NULL;
				if(to && !Game::getPlayerCreature()->hasCertificationsForItem(*to))
				{
					CuiMessageBox::createInfoBox (CuiStringIdsShipComponent::err_component_not_certified.localize ());
					return;
				}

				//okay, uninstall current item, and install this one
				if(m_ship->isSlotInstalled(slotType))
				{
					uninstallSlot(slotType);
				}
				
				char slotName[256];
				_itoa(slotType, slotName, 10);

				char buf[256];
				//params for installShipComponent are "<shipId> <slotnumber> <objId>"
				_snprintf (buf, 255, "%s %s %s", m_ship->getNetworkId().getValueString().c_str(), slotName, componentDsc->GetName().c_str());
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cms_installCommandName.c_str(), getCommandTableTarget(), Unicode::narrowToWide(buf)));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::update (float const deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_timer += deltaTimeSecs;
	if(m_timer >= cms_updateTime)
	{
		m_timer = 0.0f;
		updateCurrentSelectedAndInstalledSlots();
		updateBaseShipInformation();

		m_tree->SetDataSourceContainer(m_tree->GetDataSourceContainer());
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::updateCurrentSelectedAndInstalledSlots()
{
	if(!m_ship)
		return;

	updateSelectedComponent(ms_currentComponent);
	if(!ms_currentSlotIsDroid)
		updateInstalledComponent(ms_currentSlot);
	else
		updateInstalledComponent(m_ship->getInstalledDroidControlDevice());
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::rebuildOpenTreeBranches()
{
	if(!m_ship)
		return;

	UIDataSourceContainer * const baseContainer = m_tree->GetDataSourceContainer();
	if(baseContainer)
	{
		UIBaseObject::UIObjectList children;
		int row = cms_treeViewBadRow;
		baseContainer->GetChildren(children);
		for(UIBaseObject::UIObjectList::iterator i = children.begin(); i != children.end(); ++i)
		{
			UIBaseObject * const c = *i;
			UIDataSourceContainer * const child = dynamic_cast<UIDataSourceContainer *>(c);
			if(child)
			{
				UITreeView::DataNode * const dataNode = m_tree->FindDataNodeByDataSource(*child, row);
				if(dataNode && dataNode->expanded)
				{
					rebuildTreeShipSlot(*child);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::rebuildTreeShipSlot(UIDataSourceContainer & slotBaseContainer)
{
	if(!m_ship)
		return;

	std::string const & name = slotBaseContainer.GetName();
	ShipChassisSlotType::Type const slotType = ShipChassisSlotType::getTypeFromName (name);
	if(slotType != ShipChassisSlotType::SCST_invalid)
	{
		slotBaseContainer.Clear();

		if(m_ship->isSlotInstalled(slotType))
		{
			addInstalledShipComponentToTree(slotBaseContainer, m_ship->getComponentName(slotType), name);
		}

		ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
		if (shipChassis)
		{
			int row = cms_treeViewBadRow;
			ShipChassisSlot const * const slot = shipChassis->getSlot(slotType);
			if(slot)
			{
				addInventoryComponentsForSlotToTreeView(*slot, slotBaseContainer);

				//add the ammunition branch if necessary (i.e. if it's a missile launcher)
				std::string const & slotName = slotBaseContainer.GetName();
				if(slot->canAcceptCompatibility(TemporaryCrcString(cms_missileLauncherCompatability.c_str(), true)))
				{
					UIDataSourceContainer & ammuntionContainer = addTreeItem(slotBaseContainer, cms_ammuntionSlotName, CuiStringIdsShipComponent::ammunition.localize(), slotName);
					ms_nodesToIgnore.push_back(&ammuntionContainer);
					addMissileAmmunitionToSlot(ammuntionContainer, slotType);
					UITreeView::DataNode * const ammoDataNode = m_tree->FindDataNodeByDataSource(ammuntionContainer, row);
					if(ammoDataNode)
						ammoDataNode->expanded = true;
				}
				//Add countermeasure packs if it is a counter measure slot
				if(slot->canAcceptCompatibility(TemporaryCrcString(cms_countermeasureLauncherCompatability.c_str(), true)))
				{
					UIDataSourceContainer & countermeasureContainer = addTreeItem(slotBaseContainer, cms_countermeasurePacksSlotName, CuiStringIdsShipComponent::countermeasurepacks.localize(), slotName);
					ms_nodesToIgnore.push_back(&countermeasureContainer);
					addCountermeasurePacksToSlot(countermeasureContainer, slotType);
					UITreeView::DataNode * const packsDataNode = m_tree->FindDataNodeByDataSource(countermeasureContainer, row);
					if(packsDataNode)
						packsDataNode->expanded = true;
				}
			}
		}
	}
	else if(name == cms_droidSlotName)
	{
		slotBaseContainer.Clear();
		addDroidSlot(slotBaseContainer);
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::rebuildTree()
{
	if(!m_ship)
		return;

	m_tree->ClearData();
	ms_nodesToIgnore.clear();
	
	UIDataSourceContainer * const container = m_tree->GetDataSourceContainer();
	if(!container)
	{
		DEBUG_WARNING(true, ("No container node in buildTree"));
		return;
	}

	ms_rebuildingTree = true;

	//detach from container for speed
	container->Attach(&getPage());
	m_tree->SetDataSourceContainer(NULL);

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
	if (shipChassis)
	{
		ShipChassis::SlotVector const & slots = shipChassis->getSlots ();
		for(ShipChassis::SlotVector::const_iterator i = slots.begin(); i != slots.end(); ++i)
		{
			ShipChassisSlot const & slot = *i;
			std::string const & name = ShipChassisSlotType::getNameFromType (slot.getSlotType());
			Unicode::String const & slotName = slot.getLocalizedSlotName();

			UIDataSourceContainer & newContainer = addTreeItem(*container, name, slotName, name);

			if(m_ship->isSlotInstalled(slot.getSlotType()))
			{
				addInstalledShipComponentToTree(newContainer, m_ship->getComponentName(slot.getSlotType()), name);
			}

			addInventoryComponentsForSlotToTreeView(slot, newContainer);

			//add the ammunition branch if necessary (i.e. if it's a missile launcher)
			if(slot.canAcceptCompatibility(TemporaryCrcString(cms_missileLauncherCompatability.c_str(), true)))
			{
				UIDataSourceContainer & ammuntionContainer = addTreeItem(newContainer, cms_ammuntionSlotName, CuiStringIdsShipComponent::ammunition.localize(), name);
				ms_nodesToIgnore.push_back(&ammuntionContainer);
				addMissileAmmunitionToSlot(ammuntionContainer, slot.getSlotType());
			}
			//add the countermeasure packs if necessary (i.e. if it's a countermeasure launcher)
			if(slot.canAcceptCompatibility(TemporaryCrcString(cms_countermeasureLauncherCompatability.c_str(), true)))
			{
				UIDataSourceContainer & counterMeasureContainer = addTreeItem(newContainer, cms_countermeasurePacksSlotName, CuiStringIdsShipComponent::countermeasurepacks.localize(), name);
				ms_nodesToIgnore.push_back(&counterMeasureContainer);
				addCountermeasurePacksToSlot(counterMeasureContainer, slot.getSlotType());
			}
		}

		UIDataSourceContainer & droidContainer = addTreeItem(*container, cms_droidSlotName, CuiStringIdsShipComponent::droid.localize(), cms_droidSlotName);
		addDroidSlot(droidContainer);
	}

	m_tree->SetDataSourceContainer(container);
	container->Detach(&getPage());
	ms_rebuildingTree = false;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addInstalledShipComponentToTree(UIDataSourceContainer & parent, Unicode::String const & displayName, std::string const & slotName) const
{
	Unicode::String result = displayName;
	result += CuiStringIdsShipComponent::loaded.localize();
	IGNORE_RETURN(addTreeItem(parent, cms_installed, result, slotName));
}

//----------------------------------------------------------------------

UIDataSourceContainer & SwgCuiShipComponentManagement::addTreeItem(UIDataSourceContainer & parent, std::string const & internalName, Unicode::String const & displayName, std::string const & slotName) const
{
	UIDataSourceContainer * container = NULL;
	if(!ms_rebuildingTree)
	{
		container = m_tree->GetDataSourceContainer();
		if(!container)
		{
			DEBUG_FATAL(true, ("No container node in addTreeItem"));
		}
		container->Attach(&getPage());
		m_tree->SetDataSourceContainer(NULL);
	}

	UIDataSourceContainer * const newChild = new UIDataSourceContainer();
	newChild->SetName(internalName);
	newChild->SetProperty(UITreeView::DataProperties::LocalText, displayName);
	newChild->SetProperty(DataProperties::SlotName, Unicode::narrowToWide(slotName));
	parent.AddChild(newChild);

	if(!ms_rebuildingTree)
	{
		m_tree->SetDataSourceContainer(container);
		if(container)
			container->Detach(&getPage());
	}

	return *newChild;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addMissileAmmunitionToSlot(UIDataSourceContainer & container, ShipChassisSlotType::Type const slotType)
{
	addMissileAmmunitionToSlotRecursive(container, *Game::getPlayerCreature(), slotType);
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addMissileAmmunitionToSlotRecursive(UIDataSourceContainer & container, ClientObject const & inventoryObject, ShipChassisSlotType::Type const slotType)
{
	if (!shouldCheckContainerForComponents (inventoryObject))
	{
		return;
	}

	Container const * const inventoryContainer = ContainerInterface::getContainer(inventoryObject);
	{
		if(inventoryContainer)
		{
			for(ContainerConstIterator i = inventoryContainer->begin(); i != inventoryContainer->end(); ++i)
			{
				Object * const object = (*i).getObject();
				ClientObject * const co = object ? object->asClientObject() : NULL;
				if(co)
				{
					//recursively check containers for missiles
					addMissileAmmunitionToSlotRecursive(container, *co, slotType);

					if(isAMissilePack(co->getNetworkId()))
					{
						IGNORE_RETURN(addTreeItem(container, co->getNetworkId().getValueString(), co->getLocalizedName(), ShipChassisSlotType::getNameFromType (slotType)));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addCountermeasurePacksToSlot(UIDataSourceContainer & container, ShipChassisSlotType::Type const slotType)
{
	addCountermeasurePacksToSlotRecursive(container, *Game::getPlayerCreature(), slotType);
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addCountermeasurePacksToSlotRecursive(UIDataSourceContainer & container, ClientObject const & inventoryObject, ShipChassisSlotType::Type const slotType)
{
	if (!shouldCheckContainerForComponents (inventoryObject))
	{
		return;
	}

	Container const * const inventoryContainer = ContainerInterface::getContainer(inventoryObject);
	{
		if(inventoryContainer)
		{
			for(ContainerConstIterator i = inventoryContainer->begin(); i != inventoryContainer->end(); ++i)
			{
				Object * const object = (*i).getObject();
				ClientObject * const co = object ? object->asClientObject() : NULL;
				if(co)
				{
					//recursively check containers for missiles
					addCountermeasurePacksToSlotRecursive(container, *co, slotType);

					if(isACountermeasurePack(co->getNetworkId()))
					{
						IGNORE_RETURN(addTreeItem(container, co->getNetworkId().getValueString(), co->getLocalizedName(), ShipChassisSlotType::getNameFromType (slotType)));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::isAMissilePack(NetworkId const & objectId) const
{
	Object const * const o = NetworkIdManager::getObjectById(objectId);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co && co->getGameObjectType() == SharedObjectTemplate::GOT_ship_component_missilepack)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::isACountermeasurePack(NetworkId const & objectId) const
{
	Object const * const o = NetworkIdManager::getObjectById(objectId);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co && co->getGameObjectType() == SharedObjectTemplate::GOT_ship_component_countermeasurepack)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::loadConsumablePack(NetworkId const & objectId, ShipChassisSlotType::Type const slotType) const
{
	if(!m_ship)
		return;

	char slotName[256];
	_itoa(slotType, slotName, 10);

	char buf[256];
	//params for insertItemIntoShipComponentSlot are "<shipId> <slotnumber> <objid>"
	_snprintf (buf, 255, "%s %s %s", m_ship->getNetworkId().getValueString().c_str(), slotName, objectId.getValueString().c_str());


	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cms_loadItemOntoSlotCommandName.c_str(), getCommandTableTarget(), Unicode::narrowToWide(buf)));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addInventoryComponentsForSlotToTreeView(ShipChassisSlot const & slot, UIDataSourceContainer & container)
{
	addInventoryComponentsForSlotToTreeViewRecursive(slot, container, *Game::getPlayerCreature());
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addInventoryComponentsForSlotToTreeViewRecursive(ShipChassisSlot const & slot, UIDataSourceContainer & container, ClientObject const & inventoryObject)
{
	if (!shouldCheckContainerForComponents(inventoryObject))
	{
		return;
	}

	Container const * const inventoryContainer = ContainerInterface::getContainer(inventoryObject);
	{
		if(inventoryContainer)
		{
			for(ContainerConstIterator i = inventoryContainer->begin(); i != inventoryContainer->end(); ++i)
			{
				Object * const object = (*i).getObject();
				ClientObject * const co = object ? object->asClientObject() : NULL;
				if(co)
				{
					//recursively check containers for ship components
					addInventoryComponentsForSlotToTreeViewRecursive(slot, container, *co);

					std::string const ot = co->getTemplateName();
					ConstCharCrcString const crcString = ObjectTemplateList::lookUp(ot.c_str());
					ShipComponentDescriptor const * const descriptor = ShipComponentDescriptor::findShipComponentDescriptorBySharedObjectTemplate(crcString.getCrc());
					if(descriptor)
					{
						ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
						if(shipChassis)
						{
							if(slot.canAcceptComponent(*descriptor))
							{
								Unicode::String finalName = co->getLocalizedName();
								TangibleObject const * const to = co->asTangibleObject();
								if(to && !Game::getPlayerCreature()->hasCertificationsForItem(*to))
								{
									finalName = Unicode::narrowToWide("\\#808080") + finalName + CuiStringIdsShipComponent::not_certified.localize();;
								}
								IGNORE_RETURN(addTreeItem(container, co->getNetworkId().getValueString(), finalName, ShipChassisSlotType::getNameFromType (slot.getSlotType())));
								ObjectAttributeManager::requestUpdate(co->getNetworkId(), true);
							}
						}
					}
				}
			}
		}
	}
}

bool SwgCuiShipComponentManagement::shouldCheckContainerForComponents(ClientObject const & inventoryObject)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return false;

	//don't look in the player's bank
	if(&inventoryObject == player->getBankObject())
		return false;

	//don't look in crafting tools
	if(inventoryObject.getGameObjectType() == SharedObjectTemplate::GOT_tool_crafting)
		return false;

	//don't look inside ships, we can't load them anyhow
	if(inventoryObject.asShipObject() != NULL)
		return false;

	//don't look in packed up houses
	if(inventoryObject.getGameObjectType() == SharedObjectTemplate::GOT_data_house_control_device)
		return false;

	return true;
}

//----------------------------------------------------------------------

SwgCuiShipComponentManagement::SetShipResult SwgCuiShipComponentManagement::setShip(ShipObject * const ship)
{
	ms_badShipSet = true;

	DEBUG_WARNING(!ship, ("SwgCuiShipComponentManagement::setShip called with a null ship"));
	if(!ship)
		return SSR_noShip;

	//if not a god, check for newbie ship or an uncertified pilot
	if(!PlayerObject::isAdmin())
	{
		//check to see if the ship is a newbie ship (these can't have their components managed)
		std::string const & templateName = ship->getObjectTemplateName();
		int i;
		for(i = 0; i < cms_numNewbieShips; ++i)
		{
			if(cms_newbieShips[i] == templateName)
				return SSR_newbieShip;
		}

		for(i = 0; i < cms_numNonCombatShips; ++i)
		{
			if(cms_noncombatShips[i] == templateName)
				return SSR_nonCombatShip;
		}
	}
	else
	{
		CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("(unlocalized) Bypassing ShipManagement checks due to GOD mode."));
	}

	m_ship = ship;
	if(isActive())
		populateUI();

	ms_badShipSet = false;
	return SSR_ok;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::setTerminal(NetworkId const & nid)
{
	ms_terminal = nid;

	Object const * const o = NetworkIdManager::getObjectById(nid);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co && (co->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space || co->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space_npe))
	{
		setAssociatedObjectId(ms_terminal);
		setMaxRangeFromObject(cms_maxRangeFromTerminal);
	}

	//if there's no terminal, show the text page warning that they can't change the ship
	m_pageNotNearTerminal->SetVisible(!canModifyShip());
	UIBaseObject * const parent = m_pageNotNearTerminal->GetParent();
	if(parent)
		parent->Link();
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::clearUI()
{
	m_tree->ClearData();
	ms_nodesToIgnore.clear();
	m_loadButton->SetEnabled(false);
	m_unloadButton->SetEnabled(false);
	m_repairButton->SetVisible(false);
	m_selectedComponentText->Clear();
	m_installedComponentText->Clear();
	m_selectedComponentName->Clear();
	m_installedComponentName->Clear();
	m_massText->Clear();
	m_powerText->Clear();
	m_energyText->Clear();
	m_chassisText->Clear();
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::populateUI()
{
	if(!m_ship)
		return;

	clearUI();
	m_shipViewer->clearObjects ();
	m_shipViewer->addObject (*m_ship);
	m_shipViewer->setViewDirty         (true);
	m_shipViewer->setCameraForceTarget (true);
	m_shipViewer->recomputeZoom ();
	m_shipViewer->setCameraForceTarget (false);
	m_shipViewer->setRotateSpeed(0.2f);

	ms_energyUsageDirty = true;

	//set up the tree
	rebuildTree();
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::updateSelectedComponent(NetworkId const & object)
{
	ms_currentComponent = object;

	m_selectedComponentText->Clear();
	m_selectedComponentName->Clear();
	if(object != NetworkId::cms_invalid)
	{
		std::vector<std::pair<std::string, Unicode::String> > attribs;
		std::vector<std::pair<std::string, Unicode::String> > finalAttribs;
		bool const result = ObjectAttributeManager::getAttributes(object, attribs, false, true);
		if(result)
		{
			bool inShipComponentAttributes = false;
			std::vector<std::pair<std::string, Unicode::String> >::iterator startShipComponentsIterator = attribs.begin();
			std::vector<std::pair<std::string, Unicode::String> >::iterator endShipComponentsIterator = attribs.end();

			//find the slice of attribs that refers to ship components, only use those
			for(std::vector<std::pair<std::string, Unicode::String> >::iterator i = attribs.begin(); i != attribs.end(); ++i)
			{
				if(!inShipComponentAttributes)
				{
					if(i->first.find(cms_shipComponentAttributePre) != i->first.npos) //lint !e737 npos signed/unsigned foolishness
					{
						startShipComponentsIterator = i;
						inShipComponentAttributes = true;
					}
				}
				else
				{
					if(i->first.find(cms_shipComponentAttributePre) == i->first.npos) //lint !e737 npos signed/unsigned foolishness
					{
						endShipComponentsIterator = i;
						break;
					}
				}
			}

			finalAttribs.insert(finalAttribs.end(), startShipComponentsIterator, endShipComponentsIterator);
			Unicode::String resultStr;
			ObjectAttributeManager::formatAttributes(finalAttribs, resultStr, NULL, NULL, false);
			m_selectedComponentText->SetLocalText(resultStr);
		}
	
		Object const * const o = NetworkIdManager::getObjectById(object);
		ClientObject const * const co = o ? o->asClientObject() : NULL;
		if(co)
			m_selectedComponentName->SetLocalText(co->getLocalizedName());
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::updateInstalledComponent(ShipChassisSlotType::Type const slot)
{
	m_installedComponentText->Clear();
	m_installedComponentName->Clear();
	m_repairButton->SetVisible(false);

	if(!m_ship)
		return;

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (m_ship->getChassisType());
	if (!shipChassis)
		return;
	ShipComponentData * const data = m_ship->createShipComponentData(slot);
	if(!data)
		return;

	if((data->m_hitpointsCurrent < data->m_hitpointsMaximum)
	 || (data->m_armorHitpointsCurrent < data->m_armorHitpointsMaximum))
		m_repairButton->SetVisible(true);

	std::vector<std::pair<std::string, Unicode::String> > attribs;
	data->getAttributes(attribs);
	delete data;
	Unicode::String result;
	ObjectAttributeManager::formatAttributes(attribs, result, NULL, NULL, false);
	m_installedComponentText->SetLocalText(result);

	m_installedComponentName->SetLocalText(m_ship->getComponentName(slot));

	if(!canModifyShip())
	{
		m_repairButton->SetVisible(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::updateInstalledComponent(NetworkId const & objectId)
{
	m_installedComponentText->Clear();
	m_installedComponentName->Clear();
	m_repairButton->SetVisible(false);

	if(objectId != NetworkId::cms_invalid)
	{
		std::vector<std::pair<std::string, Unicode::String> > attribs;
		bool const result = ObjectAttributeManager::getAttributes(objectId, attribs, false, true);
		if(result)
		{
			Unicode::String resultStr;
			ObjectAttributeManager::formatAttributes(attribs, resultStr, NULL, NULL, false);
			m_installedComponentText->SetLocalText(resultStr);
		}
	
		Object const * const o = NetworkIdManager::getObjectById(objectId);
		ClientObject const * const co = o ? o->asClientObject() : NULL;
		if(co)
			m_installedComponentName->SetLocalText(co->getLocalizedName());
	}

	if(!canModifyShip())
	{
		m_repairButton->SetVisible(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::updateBaseShipInformation()
{
	m_massText->Clear();
	m_powerText->Clear();
	m_energyText->Clear();
	m_chassisText->Clear();

	if(!m_ship)
		return;

	if(canModifyShip() && (m_ship->getCurrentChassisHitPoints() < m_ship->getMaximumChassisHitPoints()))
		m_repairChassisButton->SetVisible(true);
	else
		m_repairChassisButton->SetVisible(false);

	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);
	snprintf(buffer, buffer_size, "%.0f/%.0f", m_ship->getChassisComponentMassCurrent(), m_ship->getChassisComponentMassMaximum());
	m_massText->SetLocalText(Unicode::narrowToWide(buffer));

	float const energyUsageOfShip = getEnergyUsageOfShip();
	float const energyGenerationRate = m_ship->getReactorEnergyGenerationRate();

	if (energyUsageOfShip > energyGenerationRate)
		snprintf(buffer, buffer_size, "\\#ff0000%.0f/%.0f\\#.", getEnergyUsageOfShip(), m_ship->getReactorEnergyGenerationRate());
	else
		snprintf(buffer, buffer_size, "%.0f/%.0f", getEnergyUsageOfShip(), m_ship->getReactorEnergyGenerationRate());

	m_energyText->SetLocalText(Unicode::narrowToWide(buffer));

	snprintf(buffer, buffer_size, "%.0f/%.0f", m_ship->getCurrentChassisHitPoints(), m_ship->getMaximumChassisHitPoints());
	m_chassisText->SetLocalText(Unicode::narrowToWide(buffer));
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::addDroidSlot(UIDataSourceContainer & droidContainer) const
{
	if(!m_ship)
		return;
		
	ClientObject const * const datapad = Game::getPlayerCreature()->getDatapadObject();
	if(datapad)
	{
		Container const * const datapadContainer = ContainerInterface::getContainer(*datapad);
		{
			if(datapadContainer)
			{
				//look at each datapad object for possible droid control devices (they're data_droid_control_device)
				for(ContainerConstIterator i = datapadContainer->begin(); i != datapadContainer->end(); ++i)
				{
					Object const * const o = (*i).getObject();
					ClientObject const * const co = o ? o->asClientObject() : NULL;
					if(co)
					{
						if(isADroidControlDevice(co->getNetworkId()))
						{
							//if it's the loaded droid, show the loaded text
							NetworkId const & installedDroidId = m_ship->getInstalledDroidControlDevice();
							Unicode::String displayName = co->getLocalizedName();
							if(installedDroidId.isValid())
							{
								if(co->getNetworkId() == installedDroidId)
								{
									displayName = Unicode::narrowToWide("\\#ff0000");
									displayName += co->getLocalizedName();
									displayName += CuiStringIdsShipComponent::loaded.localize();
								}
							}

							IGNORE_RETURN(addTreeItem(droidContainer, co->getNetworkId().getValueString().c_str(), displayName, cms_droidSlotName));
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::onComponentsChanged (ShipObject::Messages::ComponentsChanged::Payload & ship)
{
	if(&ship == m_ship)
	{
		ms_energyUsageDirty = true;
		rebuildOpenTreeBranches();
	}
} //lint !e1764 ship could be const (no it can't, it HAS to match the transceiver EXACTLY)

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::onDroidPcdChanged(ShipObject::Messages::DroidPcdChanged::Payload & ship)
{
	if(&ship == m_ship)
	{
		rebuildOpenTreeBranches();
	}
} //lint !e1764 ship could be const (no it can't, it HAS to match the transceiver EXACTLY)

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::onObjectAddedToContainer(const ClientObject::Messages::ContainerMsg & payload)
{
	ClientObject const * const container = payload.first;
	UNREF(container);
	ClientObject const * const co = payload.second;
	if(co && isAnObjectWeCareAbout(*co))
	{
		rebuildOpenTreeBranches();
	}
}


//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::onObjectRemovedFromContainer(const ClientObject::Messages::ContainerMsg & payload)
{
	ClientObject const * const container = payload.first;
	UNREF(container);
	ClientObject const * const co = payload.second;
	if(co && isAnObjectWeCareAbout(*co))
	{
		rebuildOpenTreeBranches();
	}
}

//----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::isADroidControlDevice(NetworkId const & objId) const
{
	Object * const object = NetworkIdManager::getObjectById(objId);
	ClientObject * const co = object ? object->asClientObject() : NULL;
	if(co && co->getGameObjectType() == SharedObjectTemplate::GOT_data_droid_control_device)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiShipComponentManagement::associateDroidWithShip(NetworkId const & droidControlDeviceId) const
{
	//cms_invalid is a legal value, in fact it means "associate no droid with this ship"

	if(!m_ship)
		return;

	if(!isADroidControlDevice(droidControlDeviceId) && droidControlDeviceId.isValid())
		return;

	char buf[256];
	//params for associateDroidControlDeviceWithShip are "<shipId> <droidId>"
	_snprintf (buf, 255, "%s %s", m_ship->getNetworkId().getValueString().c_str(), droidControlDeviceId.getValueString().c_str());
	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(cms_associateDroidControlDeviceWithShipCommandName.c_str(), getCommandTableTarget(), Unicode::narrowToWide(buf)));
}

//----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::canModifyShip() const
{
	return (PlayerObject::isAdmin () ? true : ms_terminal.isValid());
}

//----------------------------------------------------------------------

NetworkId const & SwgCuiShipComponentManagement::getCommandTableTarget() const
{
	return (PlayerObject::isAdmin() ? Game::getPlayer()->getNetworkId() : ms_terminal);
}

//----------------------------------------------------------------------

bool SwgCuiShipComponentManagement::isAnObjectWeCareAbout(ClientObject const & co) const
{
	if(co.isGameObjectTypeOf(SharedObjectTemplate::GOT_ship_component))
		return true;

	if(isADroidControlDevice(co.getNetworkId()))
		return true;

	return false;
}

//----------------------------------------------------------------------

float SwgCuiShipComponentManagement::getEnergyUsageOfShip() const
{
	if(!m_ship)
		return 0.0f;

	//only recaculate the energy usage when necessary
	if(ms_energyUsageDirty)
	{
		ms_energyUsageDirty = false;
		ms_currentEnergyUsage = m_ship->getShipTotalComponentEnergyRequirement();
		//recalculate energy usage
	}
	return ms_currentEnergyUsage;
}

//======================================================================
