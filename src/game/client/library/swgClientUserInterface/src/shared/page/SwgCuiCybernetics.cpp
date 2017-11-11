//===================================================================
//
// SwgCuiCybernetics.cpp
// copyright 2005, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCybernetics.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "clientGame/Game.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWidget3dPaperdoll.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsChangeRequest.h"
#include "sharedObject/Controller.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"

//===================================================================

namespace SwgCuiCyberneticsNamespace
{
	std::vector<std::string> cms_cyberneticSlots;
	std::vector<std::string> cms_wearableSlots;
	std::vector<std::string> cms_cyberneticsCodeDatas;
	std::map<std::string, std::string> cms_wearablesSlotToCyberneticSlotNameMap;
	std::map<std::string, std::string> cms_cyberneticSlotNameToCodeDataMap;
	std::map<std::string, UIPage *> m_cyberneticSlotNameToPageMap;

	std::string const cms_overlayPath("/Styles.New.active.rs_default");

	//--------------

	class IconCallback : public CuiIconManagerCallback
	{
	public:
		IconCallback();
		virtual ~IconCallback();
		virtual void overrideTooltip(const ClientObject & obj, Unicode::String & str) const;
		virtual bool overrideDoublems_droidCommandProgramSizeFile(const UIWidget & viewer) const;
	private:
		IconCallback(IconCallback const &);
		IconCallback & operator=(IconCallback const &);
	};

	//--------------

	//filter used to only show cybernetic pieces in the left window
	class ProviderFilter : public SwgCuiContainerProviderFilter
	{
	public:
		virtual bool showObject (const ClientObject & obj) const;
	};

	//--------------

	//is a given object a type of cybernetic?
	bool isCybernetic(ClientObject const * const object)
	{
		if(!object)
			return false;
		return GameObjectTypes::isTypeOf (object->getGameObjectType(), SharedObjectTemplate::GOT_cybernetic);
	}

	//--------------

	//is the given object a cybernetic piece that the player is wearing in a cybernetic slot?
	bool isEquippedCybernetic(ClientObject const * const object)
	{
		if(!isCybernetic(object))
			return false;

		CreatureObject const * const player = Game::getPlayerCreature();
		if(!player)
			return false;

		for(std::map<std::string, std::string>::const_iterator i = cms_wearablesSlotToCyberneticSlotNameMap.begin(); i != cms_wearablesSlotToCyberneticSlotNameMap.end(); ++i)
		{
			ClientObject const * const equippedObject = player->getEquippedObject(i->first.c_str());
			if(equippedObject == object)
				return true;

			ClientObject const * const appearanceEquippedObject = player->getAppearanceEquippedObject(i->first.c_str());
			if(appearanceEquippedObject == object)
				return true;
		}

		return false;
	}

	//--------------

	ClientObject const * getObjectEquippedInCyberneticSlot(std::string const & cyberneticSlot, bool appearanceItem)
	{
		ClientObject const * result = NULL;
		for(std::map<std::string, std::string>::const_iterator i = cms_wearablesSlotToCyberneticSlotNameMap.begin(); i != cms_wearablesSlotToCyberneticSlotNameMap.end(); ++i)
		{
			if(cyberneticSlot == i->second)
			{
				CreatureObject const * const playerCreature = Game::getPlayerCreature();
				if(playerCreature)
				{
					if(appearanceItem)
						result = playerCreature->getAppearanceEquippedObject(i->first.c_str());
					else
						result = playerCreature->getEquippedObject(i->first.c_str());
					break;
				}
			}
		}
		return result;
	}

	//--------------

	//given a base cybernetic page, grab the viewer from it
	CuiWidget3dObjectListViewer * getViewerFromCyberneticsPage(UIPage const & page)
	{
		return dynamic_cast<CuiWidget3dObjectListViewer *>(page.GetChild("icon"));
	}
}

//======================================================================

SwgCuiCyberneticsNamespace::IconCallback::IconCallback()
: CuiIconManagerCallback()
{
}

//----------------------------------------------------------------------

SwgCuiCyberneticsNamespace::IconCallback::~IconCallback()
{
}

//----------------------------------------------------------------------

void SwgCuiCyberneticsNamespace::IconCallback::overrideTooltip(const ClientObject &, Unicode::String &) const
{
}

//----------------------------------------------------------------------

bool SwgCuiCyberneticsNamespace::IconCallback::overrideDoublems_droidCommandProgramSizeFile(const UIWidget &) const
{
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiCyberneticsNamespace::ProviderFilter::showObject (const ClientObject & obj) const
{
	//only show unequipped cybernetic pieces
	return isCybernetic(&obj) && !isEquippedCybernetic(&obj) && obj.getGameObjectType() != SharedObjectTemplate::GOT_cybernetic_component;
}

//===================================================================

using namespace SwgCuiCyberneticsNamespace;

//===================================================================

SwgCuiCybernetics::SwgCuiCybernetics (UIPage& page) :
UIEventCallback(),
CuiMediator("SwgCuiCybernetics",page),
m_callback(new MessageDispatch::Callback),
m_openType(MessageQueueCyberneticsOpen::OT_VIEW),
m_containerMediator(NULL),
m_containerProvider(new SwgCuiContainerProviderDefault),
m_providerFilter(new ProviderFilter),
m_installButton(NULL),
m_repairButton(NULL),
m_uninstallButton(NULL),
m_cancelButton(NULL),
m_showAtabItems(NULL),
m_characterName(NULL),
m_slotParentPage(NULL),
m_containerPage(NULL),
m_composite(NULL),
m_selectionEnabled(true),
m_selectedSlotName(),
m_selectedInventoryObjectId(),
m_npc(),
m_selectedCyberneicSlotObjectId(),
m_characterViewer(NULL)
{
	{
		//TODO EAS move this into a data table

		//to add a cybernetic slot, the following is needed

		//1: append a programmer-definable cybernetic "slot" name here
		cms_cyberneticSlots.clear();
		cms_cyberneticSlots.push_back("leftarm");
		cms_cyberneticSlots.push_back("rightarm"); 
		cms_cyberneticSlots.push_back("legs");
		cms_cyberneticSlots.push_back("torso");
		cms_cyberneticSlots.push_back("lefthand");
		cms_cyberneticSlots.push_back("righthand");

		//2: append the name of the wearable slot
		cms_wearableSlots.clear();
		cms_wearableSlots.push_back("bracer_lower_l");
		cms_wearableSlots.push_back("bracer_lower_r");
		cms_wearableSlots.push_back("pants1");
		cms_wearableSlots.push_back("chest2");
		cms_wearableSlots.push_back("cybernetic_hand_l");
		cms_wearableSlots.push_back("cybernetic_hand_r");

		//3: append the name of the codedata for the page with the icon and viewer
		cms_cyberneticsCodeDatas.clear();
		cms_cyberneticsCodeDatas.push_back("leftarmpage");
		cms_cyberneticsCodeDatas.push_back("rightarmpage");
		cms_cyberneticsCodeDatas.push_back("legspage");
		cms_cyberneticsCodeDatas.push_back("torsopage");
		cms_cyberneticsCodeDatas.push_back("lefthandpage");
		cms_cyberneticsCodeDatas.push_back("righthandpage");

		//4: done
	}

	FATAL((cms_cyberneticSlots.size() != cms_wearableSlots.size()) || 
		    (cms_wearableSlots.size() != cms_cyberneticsCodeDatas.size()),
				("Mismatched sizes of cybernetic vectors"));

	int const numberOfCyberneticSlots = cms_cyberneticSlots.size();

	for(int i = 0; i < numberOfCyberneticSlots; ++i)
	{
		cms_wearablesSlotToCyberneticSlotNameMap[cms_wearableSlots[i]] = cms_cyberneticSlots[i];
		cms_cyberneticSlotNameToCodeDataMap[cms_cyberneticSlots[i]] = cms_cyberneticsCodeDatas[i];
	}

	//get the buttons
	getCodeDataObject (TUIButton, m_installButton, "installbutton");
	getCodeDataObject (TUIButton, m_repairButton, "repairbutton");
	getCodeDataObject (TUIButton, m_uninstallButton, "uninstallbutton");
	getCodeDataObject (TUIButton, m_cancelButton, "cancelButton");
	registerMediatorObject(*m_installButton, true);
	registerMediatorObject(*m_repairButton, true);
	registerMediatorObject(*m_uninstallButton, true);
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUICheckbox, m_showAtabItems, "checkShowATab");
	registerMediatorObject(*m_showAtabItems, true);
	m_showAtabItems->SetChecked(false, false);

	getCodeDataObject (TUIPage, m_slotParentPage, "cyberneticslotparentpage");
	getCodeDataObject (TUIPage, m_composite, "pagescomp");

	//set the player's name
	getCodeDataObject (TUIText, m_characterName, "characternametext");
	CreatureObject * const player = NON_NULL(Game::getPlayerCreature());
	m_characterName->SetPreLocalized(true);
	m_characterName->SetLocalText(player->getLocalizedName());

	//get the pages for the cybernetic slots
	UIWidget * cyberneticsSlotWidget = NULL;
	UIPage * cyberneticSlotPage = NULL;
	m_cyberneticSlotNameToPageMap.clear();
	for(std::map<std::string, std::string>::const_iterator j = cms_cyberneticSlotNameToCodeDataMap.begin(); j != cms_cyberneticSlotNameToCodeDataMap.end(); ++j)
	{
		getCodeDataObject (TUIWidget, cyberneticsSlotWidget, j->second.c_str());
		cyberneticSlotPage = NON_NULL(dynamic_cast<UIPage *>(cyberneticsSlotWidget));
		registerMediatorObject(*cyberneticSlotPage, true);
		m_cyberneticSlotNameToPageMap[j->first] = cyberneticSlotPage;
	}

	updateViewers();

	//set up the container provider
	getCodeDataObject(TUIPage, m_containerPage, "ContainerPage");
	m_containerMediator = new SwgCuiInventoryContainer(*m_containerPage);
	m_containerMediator->setInventoryType(SwgCuiInventory::IT_CRAFTING);	//use crafting because it does not save
	m_containerMediator->fetch();
	m_containerMediator->setContainerProvider(m_containerProvider);
	m_containerProvider->setFilter(m_providerFilter);
	m_containerMediator->setContainerObject(player->getInventoryObject(), "");

	UIWidget * widget = NULL;
	getCodeDataObject (TUIWidget, widget, "paperdoll");
	m_characterViewer = NON_NULL (dynamic_cast<CuiWidget3dPaperdoll *>(widget));
	registerMediatorObject (*m_characterViewer, true);
	setupCharacterViewer (Game::getClientPlayer ());

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	updatePage();
}

//----------------------------------------------------------------------

SwgCuiCybernetics::~SwgCuiCybernetics ()
{
	m_cyberneticSlotNameToPageMap.clear();

	m_installButton = 0;
	m_repairButton = 0;
	m_uninstallButton = 0;
	m_cancelButton = 0;

	m_containerMediator->setContainerProvider(0);
	m_containerMediator->setFilter(0);
	m_containerMediator->release();
	m_containerMediator = 0;

	m_characterViewer = 0;

	delete m_containerProvider;
	m_containerProvider = 0;

	delete m_providerFilter;
	m_providerFilter = 0;

	delete m_callback;
	m_callback = NULL;
}

//-------------------------------------------------------------------

void SwgCuiCybernetics::performActivate ()
{
	CuiMediator::performActivate();
	CuiManager::requestPointer (true);
	setIsUpdating(true);

	updateViewers();

	m_callback->connect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiCybernetics::onInventorySelectionChanged);

	if (m_containerMediator != 0)
	{
		m_containerMediator->activate();

		UIPage * const containerPage = safe_cast<UIPage *>(m_containerMediator->getPage().GetParentWidget());
		if (containerPage != 0)
		{
			containerPage->Pack();
		}
	}

	m_characterViewer->setPaused (false);

	m_showAtabItems->SetChecked(false, false);

	updateInstalledCybernetics();
}

//-------------------------------------------------------------------

void SwgCuiCybernetics::performDeactivate ()
{
	CuiMediator::performDeactivate();
	if (m_containerMediator != 0)
	{
		m_containerMediator->deactivate();
	}

	setIsUpdating(false);

	m_characterViewer->setPaused (true);

	m_callback->disconnect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiCybernetics::onInventorySelectionChanged);
	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::onInventorySelectionChanged (const CuiContainerSelectionChanged::Payload & )
{
	if (m_containerMediator->isActive())
	{
		ClientObject * const selection = m_containerMediator->getLastSelection ();
		if(selection)
		{
			m_selectedInventoryObjectId = selection->getNetworkId();
			selectCyberneticSlot("");
		}
		else
		{
			m_selectedInventoryObjectId = NetworkId::cms_invalid;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::OnButtonPressed (UIWidget * const context)
{
	MessageQueueCyberneticsChangeRequest * msg = NULL;

	NetworkId selectedObjectId = m_selectedCyberneicSlotObjectId != NetworkId::cms_invalid ? m_selectedCyberneicSlotObjectId : m_selectedInventoryObjectId;

	if (context == m_installButton)
	{
		msg = new MessageQueueCyberneticsChangeRequest(MessageQueueCyberneticsChangeRequest::CT_INSTALL, selectedObjectId, m_npc);
	}
	else if (context == m_repairButton)
	{
		msg = new MessageQueueCyberneticsChangeRequest(MessageQueueCyberneticsChangeRequest::CT_REPAIR, selectedObjectId, m_npc);
	}
	else if (context == m_uninstallButton)
	{
		msg = new MessageQueueCyberneticsChangeRequest(MessageQueueCyberneticsChangeRequest::CT_UNINSTALL, selectedObjectId, m_npc);
	}
	else if (context == m_cancelButton)
	{
		closeThroughWorkspace ();
	}

	if(msg)
	{
		CreatureObject * const player = NON_NULL(Game::getPlayerCreature());
		Controller * const playerController  = NON_NULL(player->getController());
		playerController->appendMessage(CM_cyberneticsChangeRequest, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiCybernetics::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDown)
	{
		for(std::map<std::string, UIPage *>::iterator i = m_cyberneticSlotNameToPageMap.begin(); i != m_cyberneticSlotNameToPageMap.end(); ++i)
		{
			UIPage * const page = i->second;
			if(page == context)
			{
				if (page && page->HitTest (msg.MouseCoords))
				{

					ClientObject const * const objectInSlot = getObjectEquippedInCyberneticSlot(i->first, m_showAtabItems->IsChecked());
					if((m_openType == MessageQueueCyberneticsOpen::OT_REPAIR || m_openType == MessageQueueCyberneticsOpen::OT_UNINSTALL) && (!objectInSlot || !isCybernetic(objectInSlot)))
					{
						return true;
					}
					
					selectCyberneticSlot(i->first);
				}
			}
		}
		return true;
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::setPageOpenType(MessageQueueCyberneticsOpen::OpenType const openType)
{
	m_openType = openType;
	//since we're setting up the page, remove any old slot selection
	selectCyberneticSlot("");

	//show inventory for install and repair modes
	m_containerPage->SetVisible(openType == MessageQueueCyberneticsOpen::OT_INSTALL || openType == MessageQueueCyberneticsOpen::OT_REPAIR);

	//allow selection of installed cybernetic slots for uninstall and repair modes
	setSlotSelectionEnabled(openType == MessageQueueCyberneticsOpen::OT_UNINSTALL || openType == MessageQueueCyberneticsOpen::OT_REPAIR);

	m_composite->Pack();
	updatePage();
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::setNPC(NetworkId const & npc)
{
	m_npc = npc;
}

//----------------------------------------------------------------------

//update the viewers
void SwgCuiCybernetics::updateInstalledCybernetics()
{
	CreatureObject * player = NON_NULL(Game::getPlayerCreature());
	if(!player)
		return;

	for(std::map<std::string, std::string>::const_iterator i = cms_wearablesSlotToCyberneticSlotNameMap.begin(); i != cms_wearablesSlotToCyberneticSlotNameMap.end(); ++i)
	{
		ClientObject * equippedObject = NULL;
		
		if(m_showAtabItems->IsChecked())
			equippedObject = player->getAppearanceEquippedObject(i->first.c_str());	
		else
			equippedObject = player->getEquippedObject(i->first.c_str());

		if(!equippedObject)
			continue;

		if(isCybernetic(equippedObject))
		{
			setCyberneticsViewer(i->second, *equippedObject);
		}
		else
		{
			clearCyberneticsViewer(i->second);
		}
	}
}

//----------------------------------------------------------------------

//set an object into a cybernetic slot viewer
void SwgCuiCybernetics::setCyberneticsViewer(std::string const & cyberneticSlotName, ClientObject const & cyberneticsPiece)
{
	UIPage * const p = m_cyberneticSlotNameToPageMap[cyberneticSlotName];
	if(!p)
		return;

	CuiWidget3dObjectListViewer * const viewer = getViewerFromCyberneticsPage(*p);
	if(!viewer)
		return;

	viewer->setObject (const_cast<ClientObject *>(&cyberneticsPiece));
	std::vector<std::pair<std::string, Unicode::String> > attribs;
	bool const result = ObjectAttributeManager::getAttributes(cyberneticsPiece.getNetworkId(), attribs, false, true);
	if(result)
	{
		Unicode::String resultStr;
		ObjectAttributeManager::formatAttributes(attribs, resultStr, NULL, NULL, false, true);
		viewer->SetLocalTooltip(resultStr);
	}
}

//----------------------------------------------------------------------

//clear a cybernetic slot viewer
void SwgCuiCybernetics::clearCyberneticsViewer(std::string const & cyberneticSlotName)
{
	UIPage * const p = m_cyberneticSlotNameToPageMap[cyberneticSlotName];
	if(!p)
		return;

	CuiWidget3dObjectListViewer * const viewer = getViewerFromCyberneticsPage(*p);
	if(!viewer)
		return;

	viewer->clearObjects();
	viewer->SetLocalTooltip(Unicode::emptyString);
}

//----------------------------------------------------------------------

//set the appropriate buttons visible
void SwgCuiCybernetics::updatePage()
{
	m_installButton->SetVisible(m_openType == MessageQueueCyberneticsOpen::OT_INSTALL);
	m_uninstallButton->SetVisible(m_openType == MessageQueueCyberneticsOpen::OT_UNINSTALL);
	m_repairButton->SetVisible(m_openType == MessageQueueCyberneticsOpen::OT_REPAIR);
}

//----------------------------------------------------------------------

//put an overlay over the given slot, and deselect all other slots
void SwgCuiCybernetics::selectCyberneticSlot(std::string const & cyberneticSlotName)
{
	m_selectedCyberneicSlotObjectId = NetworkId::cms_invalid;
	clearCyberneticSlotOverlays();

	if(isSlotSelectionEnabled())
	{
		std::map<std::string, UIPage *>::iterator i = m_cyberneticSlotNameToPageMap.find(cyberneticSlotName);
		if(i != m_cyberneticSlotNameToPageMap.end())
		{
			CuiWidget3dObjectListViewer * const viewer = getViewerFromCyberneticsPage(*(i->second));
			if(viewer)
			{
				//TODO EAS get real art
				UIRectangleStyle * const rs = safe_cast<UIRectangleStyle *>(UIManager::gUIManager ().GetObjectFromPath (cms_overlayPath.c_str (), TUIRectangleStyle));
				viewer->setOverlay (rs);
				m_selectedSlotName = i->first;

				ClientObject const * const equippedObject = getObjectEquippedInCyberneticSlot(i->first, m_showAtabItems->IsChecked());
				if(equippedObject)
				{
					m_selectedCyberneicSlotObjectId = equippedObject->getNetworkId();
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::clearCyberneticSlotOverlays()
{
	for(std::map<std::string, UIPage *>::iterator i = m_cyberneticSlotNameToPageMap.begin(); i != m_cyberneticSlotNameToPageMap.end(); ++i)
	{
		CuiWidget3dObjectListViewer * viewer = getViewerFromCyberneticsPage(*(i->second));
		if(viewer)
		{
			viewer->setOverlay (NULL);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCybernetics::setSlotSelectionEnabled(bool const enabled)
{
	m_selectionEnabled = enabled;
}

//----------------------------------------------------------------------

bool SwgCuiCybernetics::isSlotSelectionEnabled() const
{
	return m_selectionEnabled;
}

//----------------------------------------------------------------------

//set up the viewers for the cybernetic slots
void SwgCuiCybernetics::updateViewers()
{
	for(std::map<std::string, UIPage *>::iterator k = m_cyberneticSlotNameToPageMap.begin(); k != m_cyberneticSlotNameToPageMap.end(); ++k)
	{
		UIPage * const p = k->second;
		if(p)
		{
			CuiWidget3dObjectListViewer * const v = getViewerFromCyberneticsPage(*p);
			if(v)
			{
				v->clearObjects();
				v->setViewDirty(true);
				v->setAutoZoomOutOnly(false);
				v->setCameraZoomInWhileTurn(false);
				v->setAlterObjects(false);
				v->setCameraLookAtCenter(true);
				v->setDragYawOk(false);	
				v->SetDragable(false);	
				v->SetContextCapable(true, false);
				v->setRotateSpeed(0.0f);
				v->setCameraTransformToObj(true);
				v->setCameraLodBias(3.0f);
				v->setCameraLodBiasOverride(true);
				v->setCameraForceTarget(true);
				v->recomputeZoom();
				v->setCameraForceTarget(false);
				v->setViewDirty(true);
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCybernetics::setupCharacterViewer (ClientObject * object)
{
	m_characterViewer->setAutoComputeMinimumVectorFromExtent (true);
	m_characterViewer->setAlterObject (false);
	const Vector camMax (CONST_REAL (0.0), CONST_REAL (1.65), CONST_REAL (1));
	IGNORE_RETURN (m_characterViewer->setObject (object, Vector (), camMax));
}

//-----------------------------------------------------------------

void SwgCuiCybernetics::OnCheckboxSet(UIWidget * context)
{
	if(context == m_showAtabItems)
	{
		clearAllCyberneticViewers();
		updateInstalledCybernetics();
	}
}

//-----------------------------------------------------------------

void SwgCuiCybernetics::OnCheckboxUnset(UIWidget * context)
{
	if(context == m_showAtabItems)
	{
		clearAllCyberneticViewers();
		updateInstalledCybernetics();
	}
}

//-----------------------------------------------------------------

void SwgCuiCybernetics::clearAllCyberneticViewers()
{
	clearCyberneticSlotOverlays();

	for(std::map<std::string, std::string>::const_iterator i = cms_wearablesSlotToCyberneticSlotNameMap.begin(); i != cms_wearablesSlotToCyberneticSlotNameMap.end(); ++i)
	{
		clearCyberneticsViewer((*i).second);
	}
}

//===================================================================

