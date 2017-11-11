//======================================================================
//
// SwgCuiCraftAssembly.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftAssembly.h"

#include "LocalizationManager.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AttributeListMessage.h"
#include "sharedObject/Container.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "swgClientUserInterface/SwgCuiCraftAssemblyDialogOption.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include <list>


//======================================================================

namespace SwgCuiCraftAssemblyNamespace
{
	namespace Properties
	{
		const UILowerString overrideShader        = UILowerString ("overrideShader"); 
		const UILowerString IconColorValidBlocked = UILowerString ("IconColorValidBlocked"); 
		const UILowerString IconColorValid        = UILowerString ("IconColorValid"); 
		const UILowerString IconColorInvalid      = UILowerString ("IconColorInvalid"); 
		const UILowerString IconSlotValid         = UILowerString ("IconSlotValid"); 
		const UILowerString IconSlotValidBlocked  = UILowerString ("IconSlotValidblocked"); 
		const UILowerString IconSlotInvalid       = UILowerString ("IconSlotInvalid"); 
	}
}

using namespace SwgCuiCraftAssemblyNamespace;

//----------------------------------------------------------------------

class SwgCuiCraftAssembly::ContainerFilter : 
public SwgCuiInventoryContainerFilter
{
public:

	bool overrideMessage (const UIWidget *, const UIMessage & msg) const
	{
		if (msg.Type == UIMessage::DragEnd || msg.Type == UIMessage::DragOver)
		{
//			return true;
		}

		return false;
	}

	class IconCallback : public CuiIconManagerCallback
	{
	public:
		void overrideTooltip (const ClientObject & obj, Unicode::String & str) const
		{
			Unicode::String suffix;
			SwgCuiCraftAssembly::createObjectTooltipString (obj, suffix);
			
			if (!suffix.empty ())
			{
				str.push_back ('\n');
				str.append (suffix);
			}
		}

		bool overrideDoubleClick (const UIWidget & viewer) const
		{
			return NON_NULL (m_assembly)->overrideDoubleClick (viewer);
		}

		SwgCuiCraftAssembly * m_assembly;

		IconCallback () : m_assembly (0) {}
	};

	ContainerFilter () :
	SwgCuiInventoryContainerFilter (new IconCallback)
	{
	}
	
	~ContainerFilter ()
	{
		if (iconCallback)
			delete iconCallback;
	}
};

//----------------------------------------------------------------------

class SwgCuiCraftAssembly::ProviderFilter : public SwgCuiContainerProviderFilter
{
public:

	bool showObject      (const ClientObject & obj) const
	{
		if (m_assembly->m_checkFilter->IsChecked ())
		{
			if (obj.getGameObjectType () == SharedObjectTemplate::GOT_misc_container || 
				obj.getGameObjectType () == SharedObjectTemplate::GOT_misc_container_wearable)
			{
				//Show a container if it contains anything we can use
				const Container* c = ContainerInterface::getContainer(obj);
				if (c)
				{
					for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
					{
						Container::ContainedItem item = *i;
						ClientObject const * const o = safe_cast<ClientObject const *>(item.getObject());
						if (o && showObject(*o))
							return true;
					}
					return false;
				}
			}

			static CuiCraftManager::IntVector validOptions;
			const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
			const int numSlots = sv.size ();
			for (int i = 0; i < numSlots; ++i)
			{				
				validOptions.clear ();
				
				const bool hasValidSlotOptions = CuiCraftManager::getValidSlotOptions (obj, i, validOptions) && !validOptions.empty ();

				if (hasValidSlotOptions)
					return true;
			}
			return false;
		}
		else
			return true;
	}

	SwgCuiCraftAssembly * m_assembly;
};

//----------------------------------------------------------------------


SwgCuiCraftAssembly::SwgCuiCraftAssembly (UIPage & page) :
CuiMediator                     ("SwgCuiCraftAssembly", page),
UIEventCallback                 (),
m_textSlots                     (new TextVector),
m_textLoadingSlots              (new TextVector),
m_viewerSlots                   (new ViewerVector),
m_iconSlots                     (new IconVector),
m_qualityBarSlots               (new QualityBarVector),
m_maxNumSlots                   (0),
m_buttonBack                    (0),
m_buttonNext                    (0),
m_textDesc                      (0),
m_textComplexity                (0),
m_textSchematic                 (0),
m_hopperInfo                    (0),
m_containerHopper               (0),
m_containerInventory            (0),
m_viewer                        (0),
m_selectedSlot                  (-1),
m_hoverSlot                     (-1),
m_selectedTarget                (),
m_hoverTarget                   (),
m_callback                      (new MessageDispatch::Callback),
m_colorRequiredFilled           (),
m_colorRequiredEmpty            (),
m_colorOptionalFilled           (),
m_colorOptionalEmpty            (),
m_dialogOption                  (0),
m_containerFilter               (new ContainerFilter),
m_providerFilter                (new ProviderFilter),
m_dummyObject                   (0),
m_overrideShaderName            (),
m_pageUnloadHopper              (0),
m_pageUnloadInventory           (0),
m_iconSlotInvalid               (0),
m_iconSlotValid                 (0),
m_iconSlotValidBlocked          (0),
m_iconColorInvalid              (UIColor::red),
m_iconColorValid                (UIColor::green),
m_iconColorValidBlocked         (UIColor::yellow),
m_containerProviderInventory    (new SwgCuiContainerProviderDefault),
m_containerProviderHopper       (new SwgCuiContainerProviderDefault),
m_tabs                          (0),
m_checkFilter                   (0),
m_volumeSlots                   (0),
m_currentDraftSchematic         ()
{
	safe_cast<ContainerFilter::IconCallback *>(m_containerFilter->iconCallback)->m_assembly = this;
	m_providerFilter->m_assembly = this;
	m_containerProviderInventory->setFilter (m_providerFilter);
	m_containerProviderHopper->setFilter    (m_providerFilter);

	getCodeDataObject (TUIButton,     m_buttonBack,     "buttonBack");
	getCodeDataObject (TUIButton,     m_buttonNext,     "buttonNext");

	getCodeDataObject (TUIText,       m_textDesc,       "textDesc");

	getCodeDataObject (TUITabbedPane, m_tabs,           "tabs");
	getCodeDataObject (TUICheckbox,   m_checkFilter,    "checkFilter");
	
	m_textDesc->SetPreLocalized (true);

	getCodeDataObject (TUIText,       m_textSchematic,  "textSchematic");
	getCodeDataObject (TUIText,       m_textComplexity, "textComplexity");

	m_textComplexity->SetPreLocalized (true);
	m_textSchematic->SetPreLocalized  (true);
	m_checkFilter->SetChecked         (true);

	getCodeDataObject (TUIPage,       m_pageUnloadHopper,      "pageUnloadHopper");
	getCodeDataObject (TUIPage,       m_pageUnloadInventory,   "pageUnloadInventory");
	
	const UIData * const codeData = getCodeData ();

	codeData->GetPropertyColor (UILowerString ("colorReqEmpty"),          m_colorRequiredEmpty);
	codeData->GetPropertyColor (UILowerString ("colorOptEmpty"),          m_colorOptionalEmpty);
	codeData->GetPropertyColor (UILowerString ("colorReqEmptyInvalid"),   m_colorRequiredEmptyInvalid);
	codeData->GetPropertyColor (UILowerString ("colorOptEmptyInvalid"),   m_colorOptionalEmptyInvalid);

	codeData->GetPropertyNarrow (Properties::overrideShader, m_overrideShaderName);

	{				
		UIPage * infoPage = 0;
		getCodeDataObject (TUIPage, infoPage, "hopperInfo");
		m_hopperInfo      = new SwgCuiInventoryInfo (*infoPage);
		m_hopperInfo->fetch ();
		
		{
			UIPage * containerPage = 0;
			getCodeDataObject (TUIPage, containerPage, "containerInventory");
			m_containerInventory = new SwgCuiInventoryContainer (*containerPage, SwgCuiInventoryContainer::T_craftIngredients);
			m_containerInventory->setInventoryType(SwgCuiInventory::IT_CRAFTING);
			m_containerInventory->fetch ();			
			m_containerInventory->setContainerProvider (m_containerProviderInventory);
		}
		
		{
			UIPage * containerPage = 0;
			getCodeDataObject (TUIPage, containerPage, "containerHopper");
			m_containerHopper = new SwgCuiInventoryContainer (*containerPage, SwgCuiInventoryContainer::T_craftIngredients);
			m_containerInventory->setInventoryType(SwgCuiInventory::IT_CRAFTING);
			m_containerHopper->fetch ();
			m_containerHopper->setContainerProvider (m_containerProviderHopper);
		}

		//-- connect the info mediator to selection changes in the container mediator
		m_hopperInfo->connectToSelectionTransceiver (m_containerHopper->getTransceiverSelection ());
		m_hopperInfo->connectToSelectionTransceiver (m_containerInventory->getTransceiverSelection ());

	}

	{
		UIPage * dialog = 0;
		getCodeDataObject (TUIPage,       dialog, "dialogOption");
		m_dialogOption = new SwgCuiCraftAssemblyDialogOption (*dialog);
		m_dialogOption->fetch ();
	}

	{
		UIWidget * viewerWidget = 0;
		getCodeDataObject (TUIWidget,       viewerWidget, "viewer");
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(viewerWidget));
		m_viewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
		m_viewer->setAutoZoomOutOnly       (false);
		m_viewer->setCameraZoomInWhileTurn (false);
		m_viewer->setCameraLodBias         (3.0f);
		m_viewer->setCameraLodBiasOverride (true);

		if (!m_overrideShaderName.empty ())
			m_viewer->setUseOverrideShader (m_overrideShaderName.c_str (), true);
	}

	getCodeDataObject (TUIVolumePage, m_volumeSlots, "volumeSlots");

	{
		const UIBaseObject::UIObjectList & olist = m_volumeSlots->GetChildrenRef ();
		m_viewerSlots->reserve        (20);
		m_textSlots->reserve          (20);
		m_textLoadingSlots->reserve   (20);
		m_iconSlots->reserve          (20);
		m_qualityBarSlots->reserve    (20);

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIPage * const p = safe_cast<UIPage *>(*it);
			NOT_NULL (p);

			const UIData * const slotCodeData = safe_cast<UIData *>(p->GetChild ("CodeData"));
			NOT_NULL (slotCodeData);

			static const UILowerString prop_text        ("text");
			static const UILowerString prop_viewer      ("viewer");
			static const UILowerString prop_textLoading ("textLoading");
			static const UILowerString prop_qualityBar  ("qualityBar");

			std::string path_text;
			std::string path_viewer;
			std::string path_textLoading;
			std::string path_qualityBar;

			if (!slotCodeData->GetPropertyNarrow (prop_text,   path_text))
				DEBUG_FATAL (true, ("SwgCuiCraftAssembly viewer page prop [%s] not found", prop_text.c_str ()));
			if (!slotCodeData->GetPropertyNarrow (prop_viewer, path_viewer))
				DEBUG_FATAL (true, ("SwgCuiCraftAssembly viewer page prop [%s] not found", prop_viewer.c_str ()));
			if (!slotCodeData->GetPropertyNarrow (prop_textLoading, path_textLoading))
				DEBUG_FATAL (true, ("SwgCuiCraftAssembly viewer page prop [%s] not found", prop_textLoading.c_str ()));
			if (!slotCodeData->GetPropertyNarrow (prop_qualityBar, path_qualityBar))
				DEBUG_FATAL (true, ("SwgCuiCraftAssembly viewer page prop [%s] not found", prop_qualityBar.c_str ()));

			CuiWidget3dObjectListViewer * const viewer = safe_cast<CuiWidget3dObjectListViewer *>(p->GetObjectFromPath (path_viewer.c_str (), TUIWidget));
			UIText * const text                        = safe_cast<UIText *>(p->GetObjectFromPath (path_text.c_str (), TUIText));
			UIText * const textLoading                 = safe_cast<UIText *>(p->GetObjectFromPath (path_textLoading.c_str (), TUIText));
			UIPage * const qualityBar                  = safe_cast<UIPage *>(p->GetObjectFromPath (path_qualityBar.c_str (), TUIPage));

			DEBUG_FATAL (!viewer,      ("SwgCuiCraftAssembly viewer not found [%s]",      path_viewer.c_str      ()));
			DEBUG_FATAL (!text,        ("SwgCuiCraftAssembly text not found [%s]",        path_text.c_str        ()));
			DEBUG_FATAL (!textLoading, ("SwgCuiCraftAssembly textLoading not found [%s]", path_textLoading.c_str ()));

			m_colorOptionalFilled = m_colorRequiredFilled = viewer->GetBackgroundTint ();

			viewer->setCameraLookAtCenter (true);
			viewer->setPaused             (false);
			viewer->setCameraForceTarget  (true);

			text->SetPreLocalized        (true);
			textLoading->SetPreLocalized (true);

			m_viewerSlots->push_back      (viewer);
			m_textSlots->push_back        (text);
			m_textLoadingSlots->push_back (textLoading);
			m_iconSlots->push_back        (0);
			m_qualityBarSlots->push_back  (qualityBar);
		}
	}

	DEBUG_FATAL (m_viewerSlots->size () != m_textSlots->size (), ("Viewer slots != icon slots"));

	m_maxNumSlots = static_cast<int>(m_viewerSlots->size ());

	m_containerHopper->setFilter    (m_containerFilter);
	m_containerInventory->setFilter (m_containerFilter);

	m_dummyObject = safe_cast<ClientObject *>(ObjectTemplate::createObject ("object/resource_container/shared_simple.iff"));
	WARNING (!m_dummyObject, ("no dummy"));

	registerMediatorObject (*m_tabs,                true);
	registerMediatorObject (*m_buttonBack,          true);
	registerMediatorObject (*m_buttonNext,          true);
	registerMediatorObject (*m_pageUnloadInventory, true);
	registerMediatorObject (*m_pageUnloadHopper,    true);
	registerMediatorObject (*m_checkFilter,         true);

	setState    (MS_closeable);

	m_callback->connect (*this, &SwgCuiCraftAssembly::onAttributesChanged,          static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));
}

//----------------------------------------------------------------------

SwgCuiCraftAssembly::~SwgCuiCraftAssembly ()
{
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onAttributesChanged,         static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));

	delete m_dummyObject;
	m_dummyObject = 0;

	if (m_containerHopper)
		m_containerHopper->setFilter (0);
	if (m_containerInventory)
		m_containerInventory->setFilter (0);

	delete m_containerFilter;
	m_containerFilter = 0;

	delete m_providerFilter;
	m_providerFilter = 0;

	m_dialogOption->release ();
	m_dialogOption = 0;

	delete m_callback;
	m_callback = 0;

	m_buttonBack          = 0;
	m_buttonNext          = 0;
	m_textDesc            = 0;
	m_textComplexity      = 0;
	m_textSchematic       = 0;
	m_pageUnloadHopper    = 0;
	m_pageUnloadInventory = 0;

	ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (manf_schem )
		m_viewer->removeObject (*manf_schem);

	m_viewer = 0;

	delete m_viewerSlots;
	m_viewerSlots = 0;
	delete m_textSlots;
	m_textSlots = 0;
	delete m_textLoadingSlots;
	m_textLoadingSlots = 0;
	delete m_iconSlots;
	m_iconSlots = 0;
	delete m_qualityBarSlots;
	m_qualityBarSlots = 0;

	if (m_hopperInfo)
	{
		if (m_containerInventory)
			m_hopperInfo->disconnectFromSelectionTransceiver (m_containerInventory->getTransceiverSelection ());
		if (m_containerHopper)
			m_hopperInfo->disconnectFromSelectionTransceiver (m_containerHopper->getTransceiverSelection ());
	}

	if(m_hopperInfo)
	{
		m_hopperInfo->release ();
		m_hopperInfo = 0;
	}

	if(m_containerHopper)
	{
		m_containerHopper->release ();
		m_containerHopper = 0;
	}
	
	if(m_containerInventory)
	{
		m_containerInventory->release ();
		m_containerInventory = 0;
	}

	delete m_containerProviderInventory;
	m_containerProviderInventory = 0;
	delete m_containerProviderHopper;
	m_containerProviderHopper    = 0;

}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::performActivate   ()
{
	if (!CuiCraftManager::isCrafting ())
	{
		closeNextFrame ();
		return;
	}

	CuiManager::requestPointer (true);

	m_selectedSlot = -1;
	m_hoverSlot    = -1;
	
	m_buttonNext->SetEnabled(true);

	ClientObject * const proto = CuiCraftManager::getSchematicPrototype ();

	{
		const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
		const int numSlots = static_cast<int>(sv.size ());
		
		if (numSlots > m_maxNumSlots)
			WARNING (true, ("Draft Schematic for [%s] specifies %d slots. %d is the current max", proto ? Unicode::wideToNarrow (proto->getLocalizedName ()).c_str () : "<none>", numSlots, m_maxNumSlots));
				
		{
			for (int i = 0; i < m_maxNumSlots; ++i)
			{
				CuiWidget3dObjectListViewer * const viewer      = (*m_viewerSlots)      [i];
				UIText                      * const text        = (*m_textSlots)        [i];
				UIText                      * const textLoading = (*m_textLoadingSlots) [i];
				UIImage                     * const icon        = (*m_iconSlots)        [i];
				//UIPage                      * const qualityBar  = (*m_qualityBarSlots)  [i];

				const bool                valid = i < numSlots;

				viewer->SetVisible (valid);
				viewer->SetEnabled (valid);

				if (icon)
				{
					icon->SetVisible (valid);
					icon->SetEnabled (false);
				}

				if (valid)
					viewer->SetName    (sv [i].name.getDebugString ().c_str ());
		
				text->SetVisible     (valid);
				text->SetName        (viewer->GetName ());
				textLoading->SetName (viewer->GetName ());

				updateQualityBar(i);
			}
		}
	}
	
	if (proto && proto->getAppearance ())
	{
		m_viewer->setAlterObjects (false);
		m_viewer->addObject            (*proto);
		m_viewer->setRotateSpeed       (1.0f);
		m_viewer->setCameraForceTarget (true);
		m_viewer->recomputeZoom        ();
		m_viewer->setCameraForceTarget (false);
	}

	m_hopperInfo->activate ();

	for (ViewerVector::iterator it = m_viewerSlots->begin (); it != m_viewerSlots->end (); ++it)
	{
		(*it)->AddCallback (this);
	}
	
	UIDataSource * const tab_ds = m_tabs->GetDataSource ();
	NOT_NULL (tab_ds);
	
	ClientObject * const hopperInput = CuiCraftManager::getCraftingStationHopperInput ();
	
	if (hopperInput)
	{
		CuiInventoryManager::requestItemOpen (*hopperInput, std::string (), 0, 0, false, true);
		
		if (tab_ds->GetChildCount () == 1)
		{
			UIData * const data = new UIData;
			data->SetProperty (UITabbedPane::DataProperties::DATA_NAME, CuiStringIdsCraft::tab_hop.localize ());
			tab_ds->AddChild (data);
		}
		m_containerHopper->activate ();
	}
	
	else
	{
		if (tab_ds->GetChildCount () == 2)
		{
			UIData * const data = tab_ds->GetChildByPosition (1);
			if (data)
				tab_ds->RemoveChild (data);
		}
		m_containerHopper->deactivate ();
	}

	m_containerHopper->setContainerObject (hopperInput, std::string ());

	m_tabs->SetActiveTab (0);
	OnTabbedPaneChanged  (m_tabs);

	m_containerProviderInventory->setContentDirty (true);
	m_containerProviderHopper->setContentDirty    (true);

	resetSlots ();
	enableSlots ();

	CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
	if (player != NULL)
	{
		ClientObject * const inventory = player->getInventoryObject();
		if (inventory != NULL)
		{
//			CuiInventoryManager::requestItemOpen (*inventory, std::string (), 0, 0, false, true);
			m_containerInventory->setContainerObject (inventory, std::string ());
		}
	}

	
	m_callback->connect (*this, &SwgCuiCraftAssembly::onStageChange,                static_cast<CuiCraftManager::Messages::StageChange *>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onManufactureSchematicChange, static_cast<ManufactureSchematicObject::Messages::ChangeSignal *>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onSlotTransferStatusChanged,  static_cast<CuiCraftManager::Messages::SlotTransferStatusChanged*>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onComplexityChange,           static_cast<ClientObject::Messages::ComplexityChange *>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onSessionEnded,               static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onAssembling,                 static_cast<CuiCraftManager::Messages::Assembling*>     (0));
	m_callback->connect (*this, &SwgCuiCraftAssembly::onDoneAssembling,             static_cast<CuiCraftManager::Messages::DoneAssembling*>     (0));
	m_callback->connect (m_containerInventory->getTransceiverSelection (),    *this, &SwgCuiCraftAssembly::onSelectionChanged);
	m_callback->connect (m_containerHopper->getTransceiverSelection (),       *this, &SwgCuiCraftAssembly::onSelectionChanged);

	m_viewer->setPaused (false);

	checkSizes ();
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_viewer->setPaused (true);

	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onStageChange,                static_cast<CuiCraftManager::Messages::StageChange *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onComplexityChange,           static_cast<ClientObject::Messages::ComplexityChange *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onManufactureSchematicChange, static_cast<ManufactureSchematicObject::Messages::ChangeSignal *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onSlotTransferStatusChanged,  static_cast<CuiCraftManager::Messages::SlotTransferStatusChanged*>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onSessionEnded,               static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onAssembling,                 static_cast<CuiCraftManager::Messages::Assembling*>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftAssembly::onDoneAssembling,             static_cast<CuiCraftManager::Messages::DoneAssembling*>     (0));
	m_callback->disconnect (m_containerInventory->getTransceiverSelection (), *this, &SwgCuiCraftAssembly::onSelectionChanged);
	m_callback->disconnect (m_containerHopper->getTransceiverSelection (),    *this, &SwgCuiCraftAssembly::onSelectionChanged);

	m_hopperInfo->deactivate ();

	if (m_containerInventory->isActive ())
		m_containerHopper->setViewType (m_containerInventory->getViewType ());
	else
		m_containerInventory->setViewType (m_containerHopper->getViewType ());

	m_containerHopper->deactivate  ();
	m_containerInventory->deactivate ();

	if (m_containerInventory && m_containerInventory->getVolumePage ())
		m_containerInventory->getVolumePage ()->RemoveCallback (this);
	if (m_containerHopper && m_containerHopper->getVolumePage ())
		m_containerHopper->getVolumePage ()->RemoveCallback (this);

	for (ViewerVector::iterator it = m_viewerSlots->begin (); it != m_viewerSlots->end (); ++it)
	{
		(*it)->RemoveCallback (this);
	}

	ClientObject * const proto = CuiCraftManager::getSchematicPrototype ();
	if (proto)
		m_viewer->removeObject (*proto);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonBack)
	{
		const ClientObject * const craftingTool = CuiCraftManager::getCraftingTool ();
		CuiCraftManager::stopCrafting  (true);
		deactivate ();

		if (craftingTool)
			CuiCraftManager::startCrafting (craftingTool->getNetworkId ());
		else
			WARNING (true, ("No tool"));

	}
	else if (context == m_buttonNext)
	{
		CuiCraftManager::assemble (false);
	}
}

//----------------------------------------------------------------------

bool SwgCuiCraftAssembly::close ()
{
	CuiCraftManager::stopCrafting (true);
	deactivate ();
	return false;
}

//-----------------------------------------------------------------

bool SwgCuiCraftAssembly::OnMessage( UIWidget *context, const UIMessage & msg )
{

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::DragOver)
	{
		if (msg.DragSource == context)
		{
			context->SetDropFlagOk (false);
			return true;
		}
		
		context->SetDropFlagOk (true);
		
		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		if (m_containerHopper->getVolumePage () == context)
		{
		}
		else if (m_containerInventory->getVolumePage () == context)
		{
		}
		
		else
		{
			CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
			if (slotViewer)
			{
				CuiWidget3dObjectListViewer * const dragViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragTarget);
				if (dragViewer)
				{
					ClientObject * const obj = dynamic_cast<ClientObject *>(dragViewer->getLastObject ());
					if (obj)
					{
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragStart)
	{
		if (msg.DragSource != context)
			return false;

		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		CuiWidget3dObjectListViewer * const dragViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragObject);
		if (dragViewer)
		{
			const bool fromSlot = findSlotViewer (dragViewer) != 0;

			Object * const obj = dragViewer->getLastObject ();
			ClientObject * const clientObj = dynamic_cast<ClientObject *>(obj);
			if (obj)
			{
				if (!fromSlot)
				{
					updateIconState (clientObj, false, true);
					m_pageUnloadHopper->SetEnabled    (false);
					m_pageUnloadInventory->SetEnabled (false);
				}
				else
				{
					updateIconState (0, false, false);

					const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
					const int numSlots = static_cast<int>(sv.size ());
					for (int i = 0; i < numSlots; ++i)
					{
						CuiWidget3dObjectListViewer * const viewer = (*m_viewerSlots) [i];
						viewer->SetEnabled   (false);
						viewer->SetActivated (false);
					}

					m_pageUnloadHopper->SetEnabled    (true);
					m_pageUnloadInventory->SetEnabled (true);

				}
				m_hopperInfo->getPage ().SetEnabled (!fromSlot);
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragCancel)
	{
		enableSlots ();

		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		updateIconState (getActiveContainer ()->getLastSelection (), true, false);
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragEnd)
	{
		if (msg.DragSource == context)
			return false;
		
		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		enableSlots ();

		updateIconState (getActiveContainer ()->getLastSelection (), true, false);

		//-- dropping back into the inventory or input hopper
		if (context == m_pageUnloadHopper || context == m_pageUnloadInventory)
		{
			CuiWidget3dObjectListViewer * const oldSlotViewer = findSlotViewer (msg.DragSource);
			if (oldSlotViewer)
			{
				Object * const obj = oldSlotViewer->getLastObject ();
				if (obj)
				{
					if (Game::getSinglePlayer ())
						oldSlotViewer->clearObjects ();
					
					const int slotIndex = findSlotViewerIndex (oldSlotViewer);
					const ClientObject * const containerObject = getActiveContainer ()->getContainerObject ();
					if (containerObject)
						CuiCraftManager::removeFromSlot (slotIndex, containerObject->getNetworkId ());
					else
						CuiCraftManager::removeFromSlot (slotIndex, NetworkId::cms_invalid);
					return false;
				}
			}
		}

		//-- we are dropping an item onto a schematic slot
		else
		{
			CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
			if (slotViewer)
			{
				CuiWidget3dObjectListViewer * const dragViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragObject);
				if (dragViewer)
				{
					ClientObject * const obj = dynamic_cast<ClientObject *>(dragViewer->getLastObject ());
					if (obj)
					{
						if (Game::getSinglePlayer ())
						{
							slotViewer->setObject (obj);
							CuiWidget3dObjectListViewer * const oldSlotViewer = findSlotViewer (msg.DragSource);
							if (oldSlotViewer)
								oldSlotViewer->clearObjects ();
						}
						
						const int slotIndex = findSlotViewerIndex (slotViewer);
						m_dialogOption->activate ();
						m_dialogOption->setSlot (*obj, slotIndex);
						return false;
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
		if (slotViewer)
		{
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove)
	{
		CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
		if (slotViewer)
		{
			hoverSlot (slotViewer);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseEnter)
	{
		CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
		if (slotViewer)
		{
			hoverSlot (slotViewer);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseExit)
	{
		CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
		if (slotViewer)
		{
			hoverSlot (0);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);
		if (viewer)
		{
			NOT_NULL(m_containerHopper);
			NOT_NULL(m_containerInventory);

			const bool fromSlot = findSlotViewer (viewer) != 0;
			
			//-- remove from slot
			if (!fromSlot)
				return true;

			ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
			if (!manf_schem)
				return true;

			ClientObject const * const containerObject = getActiveContainer ()->getContainerObject ();
							
			if (containerObject)
			{
				const int slotIndex = findSlotViewerIndex (viewer);

				//-- slot is already processing a transfer, skip it
				if (CuiCraftManager::getSlotTransferStatus (slotIndex))
					return true;

				const CuiCraftManager::Slot * const slot = CuiCraftManager::getSlot (slotIndex);

				if (slot)
				{
					Crafting::IngredientSlot ingredient;
					if (manf_schem->getIngredient(slot->name, ingredient) && ingredient.ingredientType != Crafting::IT_invalid && !ingredient.ingredients.empty ())
					{
						CuiCraftManager::removeFromSlot (slotIndex, containerObject->getNetworkId ());
						return false;
					}
				}
			}
		}
	}
	
	//----------------------------------------------------------------------

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCraftAssembly::overrideDoubleClick (const UIWidget & viewerWidget)
{
	const CuiWidget3dObjectListViewer * const viewer = dynamic_cast<const CuiWidget3dObjectListViewer *>(&viewerWidget);
	if (viewer)
	{
		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		const bool fromSlot = findSlotViewer (viewer) != 0;
		
		//-- put into slot if one is available
		if (!fromSlot)
		{
			ClientObject * const obj = const_cast<ClientObject *>(dynamic_cast<const ClientObject *>(viewer->getLastObject ()));
			if (obj)
			{
				//-- don't override double click on containers & factory crates
				if (obj->getGameObjectType () == SharedObjectTemplate::GOT_misc_container || 
					obj->getGameObjectType () == SharedObjectTemplate::GOT_misc_container_wearable)
				{
					CuiInventoryManager::requestItemOpen (*obj, std::string (), 0, 0, false, true);
					getActiveContainer ()->setContainerObject (obj, std::string ());
					return true;
				}

				CuiCraftManager::IntVector validOptions;	
				
				int slotContents    = 0;
				int slotMaxContents = 0;
				int optionIndex     = 0;
				
				const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
				const int numSlots = std::min (m_maxNumSlots, static_cast<int>(sv.size ()));
				for (int i = 0; i < numSlots; ++i)
				{
					//-- slot is already processing a transfer, skip it
					if (CuiCraftManager::getSlotTransferStatus (i))
						continue;

					if (objectCanGoInSlot (*obj, i, slotContents, slotMaxContents, optionIndex))
					{
						if (optionIndex < 0 || slotContents < slotMaxContents)
						{
							m_dialogOption->activate ();
							m_dialogOption->setSlot (*obj, i, optionIndex);
							return true;
						}
					}
				}
				
				//@todo: sound buzzer
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiCraftAssembly::findSlotViewer (const UIWidget * context)
{
	const ViewerVector::iterator it = std::find (m_viewerSlots->begin (), m_viewerSlots->end (), context);
	return it != m_viewerSlots->end () ? *it : 0;
}

//-----------------------------------------------------------------

int SwgCuiCraftAssembly::findSlotViewerIndex (const UIWidget * context)
{
	const ViewerVector::iterator it = std::find (m_viewerSlots->begin (), m_viewerSlots->end (), context);
	if (it == m_viewerSlots->end ())
		return -1;
	
	return static_cast<int>(std::distance (m_viewerSlots->begin (), it));
}

//-----------------------------------------------------------------

void SwgCuiCraftAssembly::hoverSlot  (CuiWidget3dObjectListViewer * slotViewer)
{
	const int slotNum = findSlotViewerIndex (slotViewer);
	
	if (slotNum != m_hoverSlot)
	{
		if (slotNum == m_selectedSlot)
			m_hoverSlot = -1;
		else
		{
			m_hoverSlot   = slotNum;
			m_hoverTarget = findSlotPosition_o (m_hoverSlot);
		}
	}
}

//-----------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiCraftAssembly::findSlotViewer (int index)
{
	if (index < 0 || index >= static_cast<int>(m_viewerSlots->size ()))
		return 0;

	return (*m_viewerSlots) [index];
}

//-----------------------------------------------------------------

const CuiWidget3dObjectListViewer * SwgCuiCraftAssembly::findSlotViewer (int index) const
{
	return const_cast<SwgCuiCraftAssembly *>(this)->findSlotViewer (index);
}

//----------------------------------------------------------------------

Vector SwgCuiCraftAssembly::findSlotPosition_o (int index)
{
	UNREF (index);
	return Vector (Random::randomReal (0.0f, 0.02f), Random::randomReal (0.0f, 0.02f), Random::randomReal (0.0f, 0.02f));
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onComplexityChange (const ClientObject & target)
{
	if (&target == CuiCraftManager::getManufactureSchematic ())
	{
		Unicode::String str;
		if (UIUtils::FormatLong (str, static_cast<long>(target.getComplexity ())))
		{
			m_textComplexity->SetLocalText (str);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onManufactureSchematicChange (const ManufactureSchematicObject & schem)
{
	if (&schem == CuiCraftManager::getManufactureSchematic ())
	{
		resetSlots ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onStageChange (const int & stage)
{
	if (stage == Crafting::CS_experiment || stage == Crafting::CS_customize)
	{
		deactivate ();
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftSummary);
	}
	else if (stage == Crafting::CS_selectDraftSchematic)
	{
		deactivate ();
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftDraft);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onAttributesChanged (const ObjectAttributeManager::Messages::AttributesChanged::Payload & id)
{
	UNREF(id);
	updateSchematicText();
	for(int i = 0; i < m_maxNumSlots; i++)
		updateQualityBar(i);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::resetSlots ()
{
	const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
	const int numSlots = std::min (m_maxNumSlots, static_cast<int>(sv.size ()));

	ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (!manf_schem)
	{
		WARNING (true, ("SwgCuiCraftAssembly::resetSlots called with no manf schem"));
		CuiMessageBox::createMessageBox (Unicode::narrowToWide ("Crafting error: no manufacture schematic"));
		closeNextFrame ();
		return;
	}
 
	//-- force a complexity update
	onComplexityChange (*manf_schem);

	const ClientObject * const proto = CuiCraftManager::getSchematicPrototype ();
	if (!proto)
	{
		WARNING (true, ("SwgCuiCraftAssembly::resetSlots called with no test prototype"));
		CuiMessageBox::createMessageBox (Unicode::narrowToWide ("Crafting error: no test prototype"));
		closeNextFrame ();
		return;
	}

	m_textSchematic->SetLocalText (proto->getLocalizedName ());

	updateSchematicText();

	Crafting::IngredientSlot ingredient;
	for (int i = 0; i < numSlots; ++i)
	{
		CuiWidget3dObjectListViewer * const viewer = (*m_viewerSlots)      [i];
		UIText * const label                       = (*m_textSlots)        [i];
		UIText * const textLoading                 = (*m_textLoadingSlots) [i];
		UIImage * const icon                       = (*m_iconSlots)        [i];
		//UIPage * const qualityBar                  = (*m_qualityBarSlots)  [i];

		const CuiCraftManager::Slot & slot     = sv [i];

		viewer->SetEnabled   (true);
		viewer->SetActivated (true);

		if (icon)
		{
			icon->SetEnabled   (false);
			icon->SetOpacity   (0.0f);
		}

		//-- there is something in it
		if (manf_schem->getIngredient(slot.name, ingredient) && ingredient.ingredientType != Crafting::IT_invalid && !ingredient.ingredients.empty ())
		{
			const Crafting::SimpleIngredient * simpleIngredient = ingredient.ingredients.front().get();
			NOT_NULL(simpleIngredient);

			const CachedNetworkId id(simpleIngredient->ingredient);
			Object * ingredientObject = id.getObject ();

			if (!ingredientObject)
			{
				if (ingredient.ingredientType == Crafting::IT_resourceType || ingredient.ingredientType == Crafting::IT_resourceClass)
				{
					ingredientObject = ResourceIconManager::getObjectForType (id);
					WARNING (!ingredientObject, ("Unable to load resource ingredient object on client: %s", id.getValueString ().c_str ()));
				}
				else
				{
					WARNING (true, ("Unable to load ingredient object on client: %s", id.getValueString ().c_str ()));
				}

				if (!ingredientObject)
					ingredientObject = m_dummyObject;
			}
			
			viewer->setObject (ingredientObject);
			
			const MessageQueueDraftSlotsDataOption & option = slot.options [ingredient.draftSlotOption];
			const int requiredCount = option.amountNeeded;
			if (requiredCount != 1 || (option.type == Crafting::IT_resourceType || option.type == Crafting::IT_resourceClass))
			{
				int count = 0;
				
				if (option.type == Crafting::IT_template || option.type == Crafting::IT_templateGeneric || option.type == Crafting::IT_schematic || option.type == Crafting::IT_schematicGeneric)
					count = static_cast<int>(ingredient.ingredients.size());
				else if (option.type == Crafting::IT_resourceType || option.type == Crafting::IT_resourceClass)
				{
					for (Crafting::Ingredients::const_iterator iter = ingredient.ingredients.begin(); iter != ingredient.ingredients.end(); ++iter)
					{
						count += (*iter)->count;
					}
				}

				char buf [128];
				snprintf (buf, sizeof (buf), "%d/%d", count, requiredCount);
				viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, Unicode::narrowToWide (buf), UIColor::white);
			}
			else
				viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, Unicode::emptyString, UIColor::white);

			if (slot.optional)
				viewer->SetBackgroundTint (m_colorOptionalFilled);
			else
				viewer->SetBackgroundTint (m_colorRequiredFilled);

			viewer->SetGetsInput (true);
			viewer->SetDragable  (true);
		}
		
		//-- there is not
		else
		{
			if (slot.options.empty ())
				WARNING (true, ("slot.options.empty ()"));
			else
			{
				const MessageQueueDraftSlotsDataOption & option = slot.options.front ();
				const int requiredCount = option.amountNeeded;
				
				if (requiredCount != 1 || option.type == Crafting::IT_resourceType || option.type == Crafting::IT_resourceClass)
				{
					char buf [128];
					snprintf (buf, sizeof (buf), "0/%d", requiredCount);
					viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, Unicode::narrowToWide (buf), UIColor::white);
				}
				else
					viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, Unicode::emptyString, UIColor::white);
				
				if (option.type == Crafting::IT_resourceType)
				{
					const NetworkId & typeId = ResourceTypeManager::findTypeByName (option.ingredient);
					Object * const obj = ResourceIconManager::getObjectForType (typeId);
					viewer->setObject (obj);
				}
				else if (option.type == Crafting::IT_resourceClass)
				{
					Object * const obj = ResourceIconManager::getObjectForClass (Unicode::wideToNarrow (option.ingredient));
					viewer->setObject (obj);
				}
				else
					viewer->setObject (0);

				if (slot.optional)
					viewer->SetBackgroundTint (m_colorOptionalEmpty);
				else
					viewer->SetBackgroundTint (m_colorRequiredEmpty);	
			}

			viewer->SetGetsInput (true);
			viewer->SetDragable  (false);
		}
		
		Unicode::String one, two, three;
		DraftSchematicInfo::createSlotDescription (slot, manf_schem, one, two, three);

		one.append (1, '\n');
		one.append (two);
		one.append (1, '\n');
		one.append (three);

		viewer->SetLocalTooltip (one);

		label->SetPreLocalized (true);

		Unicode::String str;
		LocalizationManager::getManager ().getLocalizedStringValue (slot.name, str);
		label->SetLocalText (str);

		const int slotTransferStatus = CuiCraftManager::getSlotTransferStatus (i);
		textLoading->SetVisible (slotTransferStatus != 0);

		if (slotTransferStatus > 0)
			textLoading->SetLocalText (Unicode::narrowToWide ("Loading ..."));
		else if (slotTransferStatus < 0)
			textLoading->SetLocalText (Unicode::narrowToWide ("Unloading ..."));

		updateQualityBar(i);
	}

	NOT_NULL(m_containerHopper);
	NOT_NULL(m_containerInventory);

	updateIconState (getActiveContainer ()->getLastSelection (), true, false);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::enableSlots ()
{
	//- re-enable the slot viewers
	const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
	const int numSlots = std::min (m_maxNumSlots, static_cast<int>(sv.size ()));
	for (int i = 0; i < numSlots; ++i)
	{
		CuiWidget3dObjectListViewer * const viewer = (*m_viewerSlots) [i];
		viewer->SetEnabled   (true);
		viewer->SetActivated (true);
	}
	
	m_pageUnloadHopper->SetEnabled    (false);
	m_pageUnloadInventory->SetEnabled (false);

	m_hopperInfo->getPage ().SetEnabled (true);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::updateIconState (const ClientObject * const selectedObj, bool , bool handleViewer)
{
	const ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (!manf_schem)
		return;

	//- enable the icons
	const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
	const int numSlots = std::min (m_maxNumSlots, static_cast<int>(sv.size ()));
	CuiCraftManager::IntVector validOptions;
	for (int i = 0; i < numSlots; ++i)
	{
		const CuiCraftManager::Slot & slot = sv [i];
		CuiWidget3dObjectListViewer * const viewer = (*m_viewerSlots) [i];

		NOT_NULL (viewer);

		//----------------------------------------------------------------------
				
		Crafting::IngredientSlot ingredient;
		bool hasContents = manf_schem->getIngredient(slot.name, ingredient) && ingredient.ingredientType != Crafting::IT_invalid && !ingredient.ingredients.empty ();
		bool isFilled = false;
		
		if (hasContents)
		{
			const MessageQueueDraftSlotsDataOption & option = slot.options [ingredient.draftSlotOption];
			const int requiredCount = option.amountNeeded;
			if (requiredCount > 0)
			{
				int count = 0;
				
				if (option.type == Crafting::IT_template || option.type == Crafting::IT_templateGeneric || option.type == Crafting::IT_schematic || option.type == Crafting::IT_schematicGeneric)
					count = static_cast<int>(ingredient.ingredients.size());
				else if (option.type == Crafting::IT_resourceType || option.type == Crafting::IT_resourceClass)
				{
					for (Crafting::Ingredients::const_iterator iter = ingredient.ingredients.begin(); iter != ingredient.ingredients.end(); ++iter)
						count += (*iter)->count;
				}
				
				isFilled    = count == requiredCount;
			}
		}


		if (selectedObj)
		{
			int slotContents    = 0;
			int slotMaxContents = 0;
			int optionIndex     = 0;
			const bool canGo    = objectCanGoInSlot (*selectedObj, i, slotContents, slotMaxContents, optionIndex);
	
			if (canGo)
			{
				//-- slot has contents, contents must match
				if (isFilled)
				{
					viewer->SetActivated (false);
					if (slot.optional)
						viewer->SetBackgroundTint (m_colorOptionalFilled);
					else
						viewer->SetBackgroundTint (m_colorRequiredFilled);
				}
									
				//-- slot is partially filled with matching stuff
				else
				{
					viewer->SetActivated (true);
					if (slot.optional)
						viewer->SetBackgroundTint (m_colorOptionalEmpty);
					else
						viewer->SetBackgroundTint (m_colorRequiredEmpty);
				}
			}
			
			//-- object cannot go in this slot
			else
			{
				viewer->SetActivated (false);

				if (isFilled)
				{
					if (slot.optional)
						viewer->SetBackgroundTint (m_colorOptionalFilled);
					else
						viewer->SetBackgroundTint (m_colorRequiredFilled);
				}
									
				//-- slot is partially filled with matching stuff
				else
				{
					if (slot.optional)
						viewer->SetBackgroundTint (m_colorOptionalEmptyInvalid);
					else
						viewer->SetBackgroundTint (m_colorRequiredEmptyInvalid);
				}
			}
		}

		//-- no object selected
		else
		{
			viewer->SetActivated (false);
			

			if (isFilled)
			{
				if (slot.optional)
					viewer->SetBackgroundTint (m_colorOptionalFilled);
				else
					viewer->SetBackgroundTint (m_colorRequiredFilled);
			}
			
			//-- slot is partially filled with matching stuff
			else
			{
				if (slot.optional)
					viewer->SetBackgroundTint (m_colorOptionalEmptyInvalid);
				else
					viewer->SetBackgroundTint (m_colorRequiredEmptyInvalid);
			}
		}

		const bool ok = viewer->IsActivated ();

		if (handleViewer)
		{
			viewer->SetEnabled (ok);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiCraftAssembly::objectCanGoInSlot (const ClientObject & obj, int slotNum, int & slotContents, int & slotMaxContents, int & optionIndex)
{
	slotContents     = 0;
	slotMaxContents  = 0;
	optionIndex      = -1;

	const ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (!manf_schem)
		return false;

	static CuiCraftManager::IntVector validOptions;
	
	validOptions.clear ();
	const bool hasValidSlotOptions = CuiCraftManager::getValidSlotOptions (obj, slotNum, validOptions) && !validOptions.empty ();
	
	if (hasValidSlotOptions)
	{
		const CuiCraftManager::Slot * const slot = CuiCraftManager::getSlot (slotNum);
		NOT_NULL (slot);
		
		Crafting::IngredientSlot isdata;
		const bool loaded = manf_schem->getIngredient (slot->name, isdata);

		optionIndex = isdata.draftSlotOption;
		// we don't really support slot options, so if one hasn't been chosen, use 0
		if (optionIndex < 0)
			optionIndex = 0;
		if (static_cast<size_t>(optionIndex) >= slot->options.size())
		{
			WARNING(true, ("SwgCuiCraftAssembly::objectCanGoInSlot optionIndex %d "
				"is out of range (%u) for slot %s", optionIndex, slot->options.size(), 
				slot->name.getCanonicalRepresentation().c_str()));
			return false;
		}

		const MessageQueueDraftSlotsDataOption & option = slot->options [optionIndex];
		slotMaxContents = option.amountNeeded;

		if (option.type == Crafting::IT_template || option.type == Crafting::IT_schematic || option.type == Crafting::IT_schematicGeneric)
		{
			// only crafted item can be used
			const TangibleObject * const tangibleObject = obj.asTangibleObject();
			if (tangibleObject && !tangibleObject->isCrafted())
				return false;
		}

		if (loaded && !isdata.ingredients.empty ())
		{
			for (Crafting::Ingredients::const_iterator iter = 
				isdata.ingredients.begin(); iter != isdata.ingredients.end(); ++iter)
			{
				slotContents += (*iter)->count;
			}
			if (slotContents >= slotMaxContents)
			{
				// the slot is full
				return false;
			}

			const Crafting::SimpleIngredient * simpleIngredient = isdata.ingredients.front().get();
			NOT_NULL(simpleIngredient);

//			slotContents    = simpleIngredient->count;
//			slotMaxContents = option.amountNeeded;
			
			//-- resource partially filled
			if (isdata.ingredientType == Crafting::IT_resourceClass || isdata.ingredientType == Crafting::IT_resourceType)
			{
				const ResourceContainerObject * const rco = dynamic_cast<const ResourceContainerObject *>(&obj);
				if (rco)
					return (simpleIngredient->ingredient == rco->getResourceType ()) && rco->getQuantity ();

				return false;
			}
			
			//-- component partially filled
			else if (isdata.ingredientType == Crafting::IT_template)
			{
				//@todo: this needs to be a check against the id of the manf schematic from which the object was manufactured,
				// not a template check
				const FactoryObject * const factoryObject = dynamic_cast<const FactoryObject *>(&obj);
				if (factoryObject)
				{
					const ObjectTemplate * const ot = factoryObject->getContainedObjectTemplate ();
					if (ot)
						return ot->derivesFrom (Unicode::wideToNarrow (option.ingredient));
					else
						return false;
				}
				return CuiCraftManager::objectDerivesFromTemplate (obj, Unicode::wideToNarrow (option.ingredient));
			}
			else if (isdata.ingredientType == Crafting::IT_templateGeneric)
			{
				const FactoryObject * const factoryObject = dynamic_cast<const FactoryObject *>(&obj);
				if (factoryObject)
				{
					const ObjectTemplate * const ot = factoryObject->getContainedObjectTemplate ();
					if (ot)
						return ot->derivesFrom (Unicode::wideToNarrow (option.ingredient));
					else
						return false;
				}

				return CuiCraftManager::objectDerivesFromTemplate (obj, Unicode::wideToNarrow (option.ingredient));
			}

			
			//-- something else entirely
			return false;
		}
		//-- slot not filled
		else
		{
			return true;
		}
	}
	//-- no valid slot options
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::createObjectTooltipString (const ClientObject & obj, Unicode::String & str)
{
	const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
	const int numSlots = static_cast<int>(sv.size ());
	CuiCraftManager::IntVector validOptions;

	Unicode::String validStr;

	for (int i = 0; i < numSlots; ++i)
	{				
		validOptions.clear ();
		const bool hasValidSlotOptions = CuiCraftManager::getValidSlotOptions (obj, i, validOptions) && !validOptions.empty ();
		
		if (hasValidSlotOptions)
		{
			const CuiCraftManager::Slot * slot = CuiCraftManager::getSlot (i);	
			if (!slot)
				continue;
			
			if (!validStr.empty ())
				validStr.append (1, '\n');
			
			validStr.append (4, ' ');
			validStr += slot->name.localize ();
		}
	}
	
	if (validStr.empty ())
		str = CuiStringIdsCraft::draft_input_item_unusable.localize ();
	else
	{
		str = CuiStringIdsCraft::draft_input_item_usable.localize ();
		str.append (1, '\n');
		str += validStr;
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onSelectionChanged      (const CuiContainerSelectionChanged::Payload & payload)
{
	ClientObject * const obj = payload.second;
	updateIconState (obj, true, false);

	updateSchematicText();	
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::OnTabbedPaneChanged (UIWidget * context)
{
	if (m_tabs == context)
	{
		if (m_tabs->GetActiveTab () == 0)
		{
			NOT_NULL(m_containerHopper);
			NOT_NULL(m_containerInventory);

			m_containerHopper->deactivate ();
			
			m_containerInventory->activate ();
			m_containerInventory->setViewType (m_containerHopper->getViewType ());
		}
		else
		{
			NOT_NULL(m_containerHopper);
			NOT_NULL(m_containerInventory);

			m_containerInventory->deactivate ();
			
			m_containerHopper->activate ();
			m_containerHopper->setViewType (m_containerInventory->getViewType ());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::OnCheckboxSet   (UIWidget *context)
{
	if (context == m_checkFilter)
	{
		m_containerProviderInventory->setContentDirty (true);
		m_containerProviderHopper->setContentDirty    (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::OnCheckboxUnset (UIWidget *context)
{
	if (context == m_checkFilter)
	{
		m_containerProviderInventory->setContentDirty (true);
		m_containerProviderHopper->setContentDirty    (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onSessionEnded (const bool &)
{
	closeNextFrame ();
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onAssembling(const int &)
{
	m_buttonNext->SetEnabled(false);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onDoneAssembling(const int &)
{
	m_buttonNext->SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::checkSizes ()
{
	const std::pair<uint32, uint32> & current = CuiCraftManager::getCurrentDraftSchematicCrc ();

	if (current != m_currentDraftSchematic)
	{
		getPage ().ForcePackChildren ();
		m_currentDraftSchematic = current;

		UISize scrollExtent;
		m_volumeSlots->GetScrollExtent (scrollExtent);
		const UISize & volumeSize = m_volumeSlots->GetSize ();

		if (volumeSize.y < scrollExtent.y)
		{
			getPage ().SetHeight (getPage ().GetHeight () + (scrollExtent.y - volumeSize.y));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::onSlotTransferStatusChanged (const CuiCraftManager::Messages::SlotTransferStatusChanged::Payload & )
{
	resetSlots ();
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::updateSchematicText(void)
{
	{
		m_textDesc->Clear ();

		const DraftSchematicInfo * const dsi = DraftSchematicManager::findDraftSchematic (CuiCraftManager::getCurrentDraftSchematicCrc ());

		if (dsi)
		{
			ClientObject * const ds = dsi->getClientObject ();

			if (ds)
			{
				Unicode::String header, desc, attribs;

				int dummy = 0;
				ClientObject * containerObject = getActiveContainer ()->getLastSelection ();

				ResourceContainerObject *resourceObject = dynamic_cast<ResourceContainerObject*>(containerObject);				
								
				if (DraftSchematicManager::formatDescriptionIfNewer (*ds, header, desc, attribs, false, dummy, resourceObject))
				{
					Unicode::String str = header;
					str.push_back ('\n');
					str += desc;
					str.push_back ('\n');
					str += attribs;
					m_textDesc->SetLocalText (str);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssembly::updateQualityBar(int i)
{
	if(!m_qualityBarSlots)
		return;

	const CuiCraftManager::SlotVector & sv = CuiCraftManager::getSlots ();
	const CuiCraftManager::Slot & slot     = sv [i];

	UIPage * const qualityBarParent        = (*m_qualityBarSlots)  [i];
	
	const UIBaseObject::UIObjectList & olist = qualityBarParent->GetChildrenRef ();
	if(olist.empty ())
		return;
	
	UIWidget * const qualityBar = safe_cast<UIWidget *>(olist.front ());
					
	Crafting::IngredientSlot ingredient;
	
	qualityBar->SetVisible(false);
	ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	NetworkId resourceTypeId;
	bool foundResource = false;
	if (manf_schem && manf_schem->getIngredient(slot.name, ingredient) && ingredient.ingredientType != Crafting::IT_invalid && !ingredient.ingredients.empty ())
	{
		if (ingredient.ingredientType == Crafting::IT_resourceType || ingredient.ingredientType == Crafting::IT_resourceClass)
		{
			const Crafting::SimpleIngredient * simpleIngredient = ingredient.ingredients.front().get();
			NOT_NULL(simpleIngredient);

			resourceTypeId = simpleIngredient->ingredient;
			foundResource = true;					
		}
	}
	else
	{
		NOT_NULL(m_containerHopper);
		NOT_NULL(m_containerInventory);

		//Slot empty - use currently selected item from inventory if there is one
		ClientObject * containerObject = getActiveContainer ()->getLastSelection ();

		ResourceContainerObject *resourceObject = containerObject ? dynamic_cast<ResourceContainerObject*>(containerObject) : NULL;
		if(resourceObject)
		{
			int slotContents    = 0;
			int slotMaxContents = 0;
			int optionIndex     = 0;			
			const bool canGo    = objectCanGoInSlot (*resourceObject, i, slotContents, slotMaxContents, optionIndex);
			if(canGo)
			{
				resourceTypeId = resourceObject->getResourceType();		
				foundResource = true;
			}
		}
	}

	if(foundResource)
	{
		const DraftSchematicInfo * const dsi = DraftSchematicManager::findDraftSchematic (CuiCraftManager::getCurrentDraftSchematicCrc ());
		if(dsi)
		{
			float quality = dsi->calculateResourceMatch(resourceTypeId, i);
			qualityBar->SetVisible(true);

			const long usableHeight = qualityBarParent->GetHeight ();					
			
			if(quality < 0.33f)
				qualityBar->SetColor(UIColor::red);
			else if(quality < 0.66f)
				qualityBar->SetColor(UIColor::yellow);
			else
				qualityBar->SetColor(UIColor::green);
			const long desiredHeight = static_cast<long>(static_cast<float>(usableHeight) * quality);
			qualityBar->SetHeight (desiredHeight);
			qualityBar->SetLocation (UIPoint (qualityBar->GetLocation ().x, usableHeight - desiredHeight));

			Unicode::String temp;
			UIUtils::FormatFloat(temp, (quality * 100));
			temp += Unicode::narrowToWide("%");
			qualityBar->SetTooltip(temp);
		}
	}
}

SwgCuiInventoryContainer * SwgCuiCraftAssembly::getActiveContainer()
{
	NOT_NULL(m_containerHopper);
	NOT_NULL(m_containerInventory);
	if (m_containerHopper->isActive ())
	{
		return m_containerHopper;
	}
	return m_containerInventory;
}


//======================================================================
