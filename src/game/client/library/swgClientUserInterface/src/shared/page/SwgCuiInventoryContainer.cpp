// ======================================================================
//
// SwgCuiInventoryContainer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"

#include "UIButton.h"
#include "UICursor.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/GroupObject.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiDragManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMoney.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/VolumeContainer.h"
#include "swgClientUserInterface/SwgCuiContainerProvider.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerDetails.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerIcons.h"

#include <list>
#include <algorithm>

namespace SwgCuiInventoryContainerNamespace
{
	const StringId s_capacityTooltip("ui", "capacity_tooltip");
};

using namespace SwgCuiInventoryContainerNamespace;
using namespace Cui::MenuInfoTypes;


// ======================================================================

SwgCuiInventoryContainer::SwgCuiInventoryContainer (UIPage & page, Type type) :
CuiMediator                ("SwgCuiInventoryContainer", page),
UIEventCallback            (),
m_label                    (0),
m_equipCursor              (0),
m_contentsDirty            (false),
m_forceUpdate              (false),
m_buttonUp                 (0),
m_viewer                   (0),
m_callback                 (new MessageDispatch::Callback),
m_filter                   (0),
m_details                  (0),
m_icons                    (0),
m_objects                  (new ObjectWatcherVector),
m_selection                (new ObjectWatcherVector),
m_viewers                  (new ViewerMap),
m_transceiverListReset     (new Transceivers::ListReset),
m_transceiverSelection     (new CuiContainerSelectionChanged::TransceiverType),
m_buttonView               (0),
m_buttonLootAll            (0),
m_defaultButton            (0),
m_containerLocalizedName   (),
m_viewType                 (VT_icon),
m_containerProvider        (0),
m_objectDragType           (CuiDragInfo::DragTypes::GameObject),
m_disableRadial            (false),
m_disabledRadialAllowsOpen (false),
m_type                     (type),
m_money                    (0),
m_capacityBar              (0),
m_capacityContainedLimitBar (0),
m_capacityText              (0),
m_ownedByUI                 (false),
m_capacityArea              (0),
m_saveInventoryOrder        (true)
{
	getCodeDataObject (TUIText,         m_label,         "label",             true);
	getCodeDataObject (TUIButton,       m_buttonView,    "buttonView",        true);
	getCodeDataObject (TUIButton,       m_buttonLootAll, "lootAll",           true);
	getCodeDataObject (TUIButton,       m_defaultButton, "defaultButton",     true);

	getCodeDataObject (TUIPage,         m_capacityBar,               "capacityBar",    true);
	getCodeDataObject (TUIPage,         m_capacityContainedLimitBar, "capacityContainedLimitBar", true);
	getCodeDataObject (TUIText,         m_capacityText,              "capacityText",   true);
	getCodeDataObject (TUIText,         m_capacityArea,              "capacityArea",   true);

	if (m_capacityBar)
		m_capacityBar->SetWidth (0L);

	if (m_capacityContainedLimitBar)
		m_capacityContainedLimitBar->SetVisible (false);

	if (m_capacityText)
	{
		m_capacityText->SetPreLocalized (true);
		m_capacityText->Clear ();
	}

	if (m_buttonView)
		m_buttonView->SetProperty (UIButton::PropertyName::OnPress, Unicode::emptyString);

	if (m_buttonLootAll)
		m_buttonLootAll->SetProperty (UIButton::PropertyName::OnPress, Unicode::emptyString);

	if (m_defaultButton)
		m_defaultButton->SetProperty (UIButton::PropertyName::OnPress, Unicode::emptyString);

	//-- optionals
	getCodeDataObject (TUIButton,       m_buttonUp,      "buttonUp",  true);

	UIWidget *widget = 0;
	if (getCodeDataObject (TUIWidget, widget, "Icon", true))
	{
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
		m_viewer->setCameraLookAtCenter   (true);
		m_viewer->setCameraTransformToObj (true);
		m_viewer->setDragYawOk            (false);
		m_viewer->setDragPitchOk          (false);
		m_viewer->SetDragable             (false);
		m_viewer->setRotateSpeed          (0.0f);
		m_viewer->setCameraForceTarget    (true);
		m_viewer->SetGetsInput            (false);
		m_viewer->setCameraLodBias        (4.0f);

		m_viewer->SetGetsInput (true);
		m_viewer->SetPropertyNarrow (UIWidget::PropertyName::DragAccepts, CuiDragInfo::DragTypes::GameObject);
		registerMediatorObject (*m_viewer, true);
	}

	{
		UIPage * pageIcons = 0;
		if (getCodeDataObject (TUIPage, pageIcons, "icons", true))
		{
			m_icons = new SwgCuiInventoryContainerIcons (*pageIcons);
			m_icons->fetch ();
		}
	}

	{
		UIPage * pageDetails = 0;
		if (getCodeDataObject (TUIPage, pageDetails, "details", true))
		{
			m_details = new SwgCuiInventoryContainerDetails (*pageDetails);
			m_details->fetch ();
			m_details->setType (m_type);
			pageDetails->SetVisible (false);
		}
	}

	{
		UIPage * pageMoney = 0;
		if (getCodeDataObject (TUIPage, pageMoney, "money", true))
		{
			m_money = new CuiMoney (*pageMoney);
			m_money->fetch ();
			pageMoney->SetVisible (false);
		}
	}

	if (m_label)
	{
		m_label->Clear ();
		m_label->SetPreLocalized (true);
	}

	if (m_icons)
		m_icons->setWasVisible (true);
	else if (m_details)
		m_details->setWasVisible (true);
	else
		DEBUG_FATAL (true, ("SwgCuiInventoryContainer needs at least one view"));

	m_equipCursor = static_cast<UICursor*>(getPage ().GetObjectFromPath ("/styles.cursors.equip", TUICursor));
	if (m_equipCursor)
		m_equipCursor->Attach (0);

	if (m_viewer)
	{
//		m_viewer->setAutoComputeMinimumVectorFromExtent (true);
		m_viewer->setAlterObjects (false);
	}

	const UIData * const codeData = getCodeData ();
	if (codeData)
	{
		codeData->GetPropertyNarrow  (UILowerString ("objectDragType"), m_objectDragType);

		if (m_objectDragType == ".")
			m_objectDragType.clear ();

		static const UILowerString prop_DisableRadial             ("DisableRadial");
		static const UILowerString prop_DisabledRadialAllowsOpen  ("DisabledRadialAllowsOpen");

		codeData->GetPropertyBoolean (prop_DisableRadial,  m_disableRadial);
		codeData->GetPropertyBoolean (prop_DisabledRadialAllowsOpen,  m_disabledRadialAllowsOpen);
	}
	registerMediatorObject (getPage (), true);
	if(m_buttonView)
		registerMediatorObject (*m_buttonView, true);
}

//-----------------------------------------------------------------

SwgCuiInventoryContainer::~SwgCuiInventoryContainer ()
{
	
	if (m_details)
	{
		m_details->release ();
		m_details = 0;
	}

	if (m_icons)
	{
		m_icons->release ();
		m_icons = 0;
	}

	m_capacityBar               = 0;
	m_capacityContainedLimitBar = 0;
	m_capacityText              = 0;

	delete m_objects;
	m_objects = 0;

	delete m_selection;
	m_selection = 0;

	delete m_viewers;
	m_viewers = 0;

	delete m_callback;
	m_callback = 0;

	//----------------------------------------------------------------------

	if (m_equipCursor)
	{
		m_equipCursor->Detach (0);
		m_equipCursor = 0;
	}

	//----------------------------------------------------------------------

	if (m_money)
	{
		m_money->release ();
		m_money = 0;
	}

	//----------------------------------------------------------------------

	m_label = 0;
	m_buttonUp = 0;
	m_viewer   = 0;
	m_buttonView = 0;
	m_buttonLootAll = 0;
	m_defaultButton = 0;

	delete m_transceiverListReset;
	m_transceiverListReset = 0;

	delete m_transceiverSelection;
	m_transceiverSelection = 0;

	m_filter = 0;
	m_containerProvider = 0;
}

//-----------------------------------------------------------------

void SwgCuiInventoryContainer::performActivate ()
{
	CuiMediator::garbageCollect(false);

	getPage ().SetEnabled (true);

	if(getContainerObject())
	{
		loadInventoryOrder();
	}
	updateContents ();

	if (m_money)
	{
		const Object * const player = Game::getPlayer ();
		m_money->setId (player ? player->getNetworkId () : NetworkId::cms_invalid);
	}

	if (m_icons && (m_icons->wasVisible () || (m_viewType == VT_icon)))
		m_icons->activateInContainer(this);
	else if (m_details && (m_details->wasVisible () || (m_viewType == VT_detail)))
		m_details->activateInContainer(this);

	if (m_buttonUp)
		m_buttonUp->AddCallback   (this);

	if (m_viewer)
		m_viewer->AddCallback     (this);

	if (m_viewer)
		m_viewer->setPaused (false);

	if (m_buttonLootAll)
		m_buttonLootAll->AddCallback (this);

	if (m_defaultButton)
		m_defaultButton->AddCallback (this);

	setIsUpdating (true);

	if (m_containerProvider)
		m_containerProvider->onOpeningAndClosing (true);

	updateContainerInfo ();
}

//-----------------------------------------------------------------

void SwgCuiInventoryContainer::performDeactivate ()
{
	saveInventoryOrder();

	if (m_money)
		m_money->deactivate ();

	if (m_buttonUp)
		m_buttonUp->RemoveCallback   (this);

	if (m_buttonLootAll)
		m_buttonLootAll->RemoveCallback (this);

	if (m_defaultButton)
		m_defaultButton->RemoveCallback (this);

	if (m_viewer)
	{
		m_viewer->RemoveCallback     (this);
		m_viewer->setPaused (true);
	}

	setIsUpdating (false);

	if (m_icons && m_icons->isActive ())
	{
		m_icons->deactivate ();
		m_icons->setWasVisible (true);
	}
	else if (m_details && m_details->isActive ())
	{
		m_details->deactivate ();
		m_details->setWasVisible (true);
	}

	clearObjectsAndViewers ();

	if (m_containerProvider)
		m_containerProvider->onOpeningAndClosing (false);
}

//-----------------------------------------------------------------

bool SwgCuiInventoryContainer::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (!context)
		return true;

	if (m_filter && m_filter->overrideMessage (context, msg))
		return false;

	if (msg.Type == UIMessage::DragEnd && msg.DragObject)
	{
		if (context == m_viewer || context == m_buttonUp)
		{
			const CuiDragInfo cdi (*msg.DragObject);
			ClientObject * const dropDestination = findDropDestination (*context);
			if (dropDestination)
			{
				IGNORE_RETURN(CuiDragManager::handleDrop (cdi, *dropDestination, std::string ()));
			}
		}
		/*else
		{
			const CuiDragInfo cdi (*msg.DragObject);
			ClientObject * const container = getContainerObject();
			if (container)
			{
				IGNORE_RETURN(CuiDragManager::handleDrop (cdi, *container, std::string ()));
			}
		}*/
	}
	/*else if (msg.Type == UIMessage::DragOver)
	{
		context->SetDropFlagOk (true);
	}*/
	else if (context == &getPage () && msg.Type == UIMessage::MouseWheel)
	{
		if (msg.Modifiers.isControlDown ())
		{
			float iconSize = CuiPreferences::getObjectIconSize ();
			iconSize -= static_cast<float>(msg.Data) * 0.10f;
			CuiPreferences::setObjectIconSize (iconSize);
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainer::findDropDestination (UIWidget & context)
{
	if (&context == m_buttonUp || &context == m_viewer)
	{
		return findParentContainer ();
	}
	else
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(&context);

		if (viewer)
			return safe_cast<ClientObject *>(viewer->getLastObject ());
	}

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setViewType (ViewType vt)
{
	if (vt == VT_icon && m_icons)
	{
		if (isActive ())
		{
			if (m_details)
				m_details->deactivate ();

			m_viewType = vt;
			loadInventoryOrder();
			updateContents ();

			m_icons->activateInContainer(this);
		}
	}
	else if (vt == VT_detail && m_details)
	{
		if (isActive ())
		{
			if (m_icons)
				m_icons->deactivate ();

			m_viewType = vt;
			loadInventoryOrder();
			updateContents ();

			for (ViewerMap::const_iterator it = m_viewers->begin (); it != m_viewers->end (); ++it)
			{
				CuiWidget3dObjectListViewer * const wid = (*it).second;
				if (wid)
				{
					wid->SetActivated (false);
					wid->SetMaximumSize (UISize (16384L, 16384L));
					wid->SetMinimumSize (UISize::zero);
					wid->SetLocation    (UIPoint::zero);
				}
			}

			m_details->activateInContainer(this);
		}
	}

	m_viewType = vt;
}

//-----------------------------------------------------------------

void SwgCuiInventoryContainer::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonUp)
	{
		//this assumes that a container always knows about its parents,
		//and ignores the result of the open

		ClientObject * const containedByObject = findParentContainer ();

		if (containedByObject)
		{
			CuiInventoryManager::requestItemOpen  (*containedByObject, std::string (), 0, 0, true, true);
			setContainerObject (containedByObject, std::string ());
		}
	}
	else if (context == m_buttonView)
	{
		if (m_viewType == VT_icon)
			setViewType (VT_detail);
		else
			setViewType (VT_icon);

		CuiMediator::garbageCollect(false);
	}
	else if (context == m_buttonLootAll)
	{
		  ClientObject * const containedByObject = findParentContainer ();
	
			if (containedByObject)
			{
				ClientObject * const parentContainedByObject = containedByObject->getContainedBy();

				if (parentContainedByObject)
				{
					NetworkId const & networkId = parentContainedByObject->getNetworkId();

					ClientCommandQueue::enqueueCommand ("loot", networkId, Unicode::emptyString);

				}
			}
		CuiSoundManager::play (CuiSounds::radial_complete);
	}
	else if (context == m_defaultButton)
	{
		for (ObjectWatcherVector::iterator it = m_selection->begin (); it != m_selection->end (); ++it)
		{
			Object * const obj = *it;
			if (obj)
				CuiRadialMenuManager::performDefaultAction (*obj, m_disableRadial && m_disabledRadialAllowsOpen, 0, true);
		}

		CuiSoundManager::play (CuiSounds::radial_complete);
	}
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainer::findParentContainer () const
{
	if (!m_containerProvider)
		return 0;

	if (!m_containerProvider->getSlotName().empty ())
		return 0;

	const ClientObject * const container  = getContainerObject ();

	if (!container || CuiInventoryManager::isPlayerInventory (*container))
		return 0;

	ClientObject * const containedBy = const_cast<ClientObject *>(safe_cast<const ClientObject *>(ContainerInterface::getContainedByObject (*container)));

	if (containedBy)
	{
		CreatureObject * const creature = containedBy->asCreatureObject ();
		if (creature)
		{
			return creature->getInventoryObject ();
		}
		else
			return containedBy;
	}

	return 0;
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer *     SwgCuiInventoryContainer::getViewerForObject (ClientObject & obj, bool create)
{
	CuiWidget3dObjectListViewer * viewer = 0;

	WARNING(obj.getNetworkId() == NetworkId::cms_invalid, ("SwgCuiInventoryContainer attempting to get a viewer for an object with an invalid id."));

	CachedNetworkId networkId(obj.getNetworkId());
	const ViewerMap::iterator it = m_viewers->find(networkId);

	if (it != m_viewers->end ())
	{
		viewer = (*it).second;
	}

	if(viewer)
	{	
		Object * objInCachedViewer = viewer->getLastObject();
		if(!objInCachedViewer)
		{
			//For some reason, this object has been deleted in the meantime, invalidate the viewer
			viewer = NULL;
			m_viewers->erase(it);
		}
	}

	if (!viewer)
	{
		if (!create)
			return  0;

		if (m_objectDragType.empty ())
			viewer = CuiIconManager::createObjectIcon (obj, 0);
		else
			viewer = CuiIconManager::createObjectIcon (obj, m_objectDragType.c_str ());

		NOT_NULL(viewer);
		
		if(m_ownedByUI)
			viewer->setAlterObjects(true);

		viewer->Attach(0);
		CuiIconManager::registerObjectIcon (*viewer, m_filter ? m_filter->iconCallback : 0);
		(*m_viewers)[networkId] = viewer;

		viewer->setDrawName (m_viewType == VT_icon);
	}

	return viewer;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::updateContainerInfo ()
{
	if (!m_containerProvider)
		return;

	//-- erase selection that is no longer in the container
	{
		for (ObjectWatcherVector::iterator it = m_selection->begin (); it != m_selection->end ();)
		{
			Object * const obj = *it;

			if (!obj || std::find (m_objects->begin (), m_objects->end (), *it) == m_objects->end ())
				it = m_selection->erase (it);
			else
				++it;
		}
	}

	//@todo: make provider release container ref
	//@todo: make provider save inventory state
	//@todo: make provider load inventory state

	ClientObject * const containerObject = m_containerProvider->getContainerObject ();

	if (m_viewer)
		m_viewer->clearObjects ();

	if (containerObject)
	{
		const bool isInventory       = CuiInventoryManager::isPlayerInventory (*containerObject);
		const bool isNestedInventory = isInventory || CuiInventoryManager::isNestedInventory (*containerObject);
		const bool isNestedEquipped  = !isNestedInventory && CuiInventoryManager::isNestedEquipped  (*containerObject);

		if (m_money)
		{
			if (isInventory)
				m_money->activate ();
			else
				m_money->deactivate ();
		}

		if (m_buttonUp)
		{
			bool enabled = false;

			if (!isInventory)
			{
				const Object * const containedByObj = ContainerInterface::getContainedByObject (*containerObject);

				if (containedByObj)
				{
					if (ContainerInterface::getVolumeContainer (*containedByObj) ||
						CuiInventoryManager::isNestedEquipped (*containerObject, 0, false))
							enabled = true;
				}
			}

			m_buttonUp->SetEnabled (enabled);
		}

		Unicode::String assignedName;
		StringId        nameStringId;

		ClientObject * displayObj = containerObject;

		for (; displayObj; )
		{
			if (assignedName.empty ())
				assignedName = displayObj->getObjectName ();

			if (assignedName.empty () && nameStringId.isInvalid ())
				nameStringId = displayObj->getObjectNameStringId   ();

			const Appearance * const app = displayObj->getAppearance ();
			if (app)
				break;

			Object * const object = ContainerInterface::getContainedByObject(*displayObj);
			displayObj = object ? object->asClientObject() : NULL;
		}

		if (m_viewer && displayObj)
		{
			m_viewer->setCameraPitch (PI / 8.0f);
			m_viewer->addObject (*displayObj);
		}

		if (!assignedName.empty ())
			m_containerLocalizedName = assignedName;
		else if (!nameStringId.isInvalid ())
			m_containerLocalizedName = nameStringId.localize ();
		else
			m_containerLocalizedName = containerObject->getLocalizedName ();

		if (isNestedInventory)
			IGNORE_RETURN(m_containerLocalizedName.append (CuiStringIds::inv_container_suffix_inventory.localize ()));
		else if (isNestedEquipped)
			IGNORE_RETURN(m_containerLocalizedName.append (CuiStringIds::inv_container_suffix_equipped.localize ()));

		if (m_label)
			m_label->SetLocalText (m_containerLocalizedName);
	}
	else
	{
		if (m_money)
			m_money->deactivate ();

		m_containerLocalizedName.clear ();
		if (m_label)
			m_label->SetLocalText (CuiStringIds::container_none.localize ());
	}

	m_containerProvider->setProviderDirty (false);
	saveInventoryOrder();
}

//-----------------------------------------------------------------

void  SwgCuiInventoryContainer::setContainerObject (ClientObject * containerObject, const std::string & slotName)
{
	if (m_containerProvider)
	{
		m_containerProvider->setContainerObject (containerObject, slotName);
		loadInventoryOrder();
		updateContents();

		if (m_defaultButton && containerObject)
		{
			std::string defaultButtonText = containerObject->getObjectNameStringId().getCanonicalRepresentation() + "_use1";
			m_defaultButton->SetText(StringId(defaultButtonText).localize());
		}
	}
	else
		WARNING (true, ("SwgCuiInventoryContainer attempt to set container object with no container provider"));
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::clearObjectsAndViewers ()
{
	m_objects->clear   ();
	for (ViewerMap::const_iterator it = m_viewers->begin (); it != m_viewers->end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = NON_NULL((*it).second);

		if (viewer)
		{
			CuiIconManager::unregisterObjectIcon (*viewer);
			viewer->Detach (0);
		}
	}

	m_viewers->clear ();

	saveInventoryOrder();
}

//-----------------------------------------------------------------

void SwgCuiInventoryContainer::updateContents ()
{
	ObjectWatcherVector owv;
	if (m_containerProvider)
	{
		m_containerProvider->getObjectVector (owv);
	}

	const bool changed = (owv != *m_objects);

	*m_objects = owv;

	if (changed || m_forceUpdate)
		m_transceiverListReset->emitMessage (true);

	setSelection (*m_selection);

	m_forceUpdate = false;

	saveInventoryOrder();

}

//-----------------------------------------------------------------

void SwgCuiInventoryContainer::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_containerProvider)
	{
		if (m_containerProvider->isContentDirty ())
			updateContents ();

		//-- update container info must happen after contents
		if (m_containerProvider->isProviderDirty ())
			updateContainerInfo ();

		float ratio        = 0.0f;
		float limitedRatio = 0.0f;

		const int totalVolume                 = m_containerProvider->getTotalVolume ();
		const int currentVolume               = m_containerProvider->getCurrentVolume ();
		const int totalVolumeLimitedByParents = m_containerProvider->getTotalVolumeLimitedByParents ();


		if (totalVolume < 0 || currentVolume < 0)
		{
			WARNING (true, ("Total or current volume < 0 (%d total,%d current)", totalVolume, currentVolume));
		}
		else if (totalVolume)
		{
			ratio = static_cast<float>(currentVolume) / static_cast<float>(totalVolume);

			if (totalVolumeLimitedByParents >= 0 && totalVolumeLimitedByParents < totalVolume)
			{
				limitedRatio = static_cast<float>(totalVolume - totalVolumeLimitedByParents) / static_cast<float>(totalVolume);
			}
		}

		int margin = 0L;

		if (m_capacityBar)
		{
			margin = m_capacityBar->GetLocation ().x;
			NOT_NULL (m_capacityBar->GetParentWidget ());
			const long parentWidth    = m_capacityBar->GetParentWidget ()->GetWidth ();

			const long availableWidth = parentWidth - (margin * 2L);

			m_capacityBar->SetWidth (static_cast<long>(availableWidth * ratio));
		}

		if (m_capacityContainedLimitBar)
		{
			if (limitedRatio <= 0.0f)
				m_capacityContainedLimitBar->SetVisible (false);
			else
			{
				m_capacityContainedLimitBar->SetVisible (true);
				NOT_NULL (m_capacityContainedLimitBar->GetParentWidget ());
				const long parentWidth    = m_capacityContainedLimitBar->GetParentWidget ()->GetWidth ();
				const long availableWidth = parentWidth - (margin * 2L);

				m_capacityContainedLimitBar->SetWidth    (static_cast<long>(availableWidth * limitedRatio));
				m_capacityContainedLimitBar->SetLocation (UIPoint (parentWidth - margin - m_capacityContainedLimitBar->GetWidth (), m_capacityContainedLimitBar->GetLocation ().y));
			}
		}

		if (m_capacityText)
		{			
			char buf [64];
			const size_t buf_size = sizeof (buf);
			if (totalVolumeLimitedByParents >= 0 && totalVolumeLimitedByParents != totalVolume)
			{
				m_capacityText->SetLocalTooltip(s_capacityTooltip.localize());
				snprintf (buf, buf_size, "\\#. (%d/%d \\#pcontrast2 [%d]\\#.)", currentVolume, totalVolume, totalVolumeLimitedByParents);
			}
			else
			{
				m_capacityText->SetLocalTooltip(Unicode::emptyString);
				snprintf (buf, buf_size, "(%d/%d)", currentVolume, totalVolume);
			}

			m_capacityText->SetPreLocalized (true);
			m_capacityText->SetLocalText (Unicode::narrowToWide (buf));
		}

	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setContainerProvider (SwgCuiContainerProvider * provider)
{
	if (m_containerProvider != provider)
	{
		DEBUG_FATAL (!m_viewers->empty (), ("Can't change providers with non empty viewer list"));

		m_containerProvider = provider;
		if (m_containerProvider)
			m_containerProvider->setContentDirty (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setFilter (const SwgCuiInventoryContainerFilter * filter)
{
	if (m_filter != filter)
	{
		DEBUG_FATAL (!m_viewers->empty (), ("Can't change filters with non empty viewer list"));

		m_filter = filter;
		m_contentsDirty = true;
	}
}

//----------------------------------------------------------------------

UIVolumePage * SwgCuiInventoryContainer::getVolumePage ()
{
	if (m_icons)
		return m_icons->getVolumePage ();

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setSelection (const ObjectWatcherVector & cidv)
{
	*m_selection = cidv;

	ClientObject * const sel = getLastSelection ();

	m_transceiverSelection->emitMessage (CuiContainerSelectionChanged::Payload (static_cast<int>(m_selection->size ()) - 1, sel));
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setObjectSorting             (const IntVector & iv)
{
	if (m_containerProvider)
	{
		if (m_containerProvider->isContentDirty ())
			WARNING (true, ("cant' set object sorting into a dirty container provider."));
		else
			m_containerProvider->setObjectSorting (iv);
	}
}

//----------------------------------------------------------------------

const SwgCuiInventoryContainer::ObjectWatcherVector & SwgCuiInventoryContainer::getObjects () const
{
	if (m_containerProvider && m_containerProvider->isContentDirty ())
		const_cast<SwgCuiInventoryContainer *>(this)->updateContents ();

	return *NON_NULL(m_objects);
}

//----------------------------------------------------------------------

const ClientObject * SwgCuiInventoryContainer::getLastSelection () const
{
	if (m_selection->empty ())
		return 0;
	else
		return m_selection->back ().getPointer ();
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainer::getLastSelection ()
{
	if (m_selection->empty ())
		return 0;
	else
		return m_selection->back ().getPointer ();
}

//----------------------------------------------------------------------

const ClientObject * SwgCuiInventoryContainer::getContainerObject () const
{
	if (m_containerProvider)
		return m_containerProvider->getContainerObject ();
	return 0;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainer::getContainerObject ()
{
	if (m_containerProvider)
		return m_containerProvider->getContainerObject ();
	return 0;
}

//----------------------------------------------------------------------

const std::string & SwgCuiInventoryContainer::getContainerSlotName         () const
{
	static const std::string emptyStr;
	if (m_containerProvider)
		return m_containerProvider->getSlotName ();

	return emptyStr;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainer::getObjectAtPosition          (int index)
{
	if (m_containerProvider && m_containerProvider->isContentDirty ())
		updateContents ();

	if (index < 0 || index >= static_cast<int>(m_objects->size ()))
		return 0;

	return (*m_objects) [index].getPointer ();
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiInventoryContainer::getWidgetAtPosition          (int index)
{
	ClientObject * const obj = getObjectAtPosition (index);

	if (obj)
		return getViewerForObject (*obj);

	return 0;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainer::handleDoubleClick (CuiWidget3dObjectListViewer & viewer) const
{

	if (m_filter && m_filter->iconCallback)
	{
		if (m_filter->iconCallback->overrideDoubleClick (viewer))
			return true;
	}

	if (m_disableRadial && !m_disabledRadialAllowsOpen)
		return false;

	ClientObject * const moving_obj = safe_cast<ClientObject *>(viewer.getLastObject ());

	if (moving_obj)
	{
		CreatureObject const * const player = Game::getPlayerCreature();
		
		if (player)
		{
			GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());

			if (group)
			{
				//if we are in a group and the group mode is lottery or random then we
				//need to call the loot function, otherwise we just grab the one item 
				//we are selecting
				if (group->getLootRule() == GroupObject::LR_lottery || group->getLootRule() == GroupObject::LR_random)
				{
					ClientObject const * const clientObjectParent = moving_obj->getContainedBy();

					if (clientObjectParent)
					{
						ClientObject const * const clientObjectGrandparent = clientObjectParent->getContainedBy();

						if (clientObjectGrandparent)
						{
							CreatureObject const * const creature = clientObjectGrandparent->asCreatureObject();

							if (creature && creature->isDead())
							{
								ClientCommandQueue::enqueueCommand ("loot", creature->getNetworkId(), Unicode::emptyString);	
								return true;
							}
						}
					}
				}
			}
		}
		
		CuiRadialMenuManager::performDefaultAction (*moving_obj, m_disableRadial && m_disabledRadialAllowsOpen);
		CuiSoundManager::play (CuiSounds::radial_complete);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainer::handleRadialMenu (CuiWidget3dObjectListViewer & viewer, const UIPoint & pt) const
{
	if (m_disableRadial)
		return false;

	ClientObject * const moving_obj = safe_cast<ClientObject *>(viewer.getLastObject ()) ;

	if (moving_obj)
		return !CuiRadialMenuManager::createMenu (*moving_obj, pt);

	return false;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::handleDrop (const CuiDragInfo & cdinfo, ClientObject & destination, const std::string & slotname, bool tryDragManager, bool askOnGiveItem)
{
	if (!tryDragManager || !CuiDragManager::handleDrop (cdinfo, destination, slotname, askOnGiveItem))
	{
		ClientObject * const container = getContainerObject ();

		if(cdinfo.getClientObject())
		{
			NetworkId source = cdinfo.getClientObject()->getNetworkId();

			//Check if source comes from this container
			bool found = false;
			for(unsigned int i = 0; i < m_objects->size(); ++i)
			{
				ClientObject *obj = (*m_objects) [i].getPointer ();
				if(obj && obj->getNetworkId() == source)
				{
					found = true;
				}
			}

			if(found)
			{
				if (!container || (container == &destination))
					m_containerProvider->tryToPlaceItemBefore(source, NetworkId::cms_invalid);				
				else
					m_containerProvider->tryToPlaceItemBefore(source, destination.getNetworkId());
				return;
			}
		}

		//-- just try to drop it into the container itself		
		if (!container || (container != &destination && (!tryDragManager || !CuiDragManager::handleDrop (cdinfo, *container, std::string ())) ))
		{
			// @todo beep
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainer::preHandleDragOver            (const CuiDragInfo & cdinfo, bool & ok)
{
	if (m_containerProvider)
		return m_containerProvider->handleDragOver (cdinfo, ok);

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainer::preHandleDrop                (const CuiDragInfo & cdinfo)
{
	if (m_containerProvider)
		return m_containerProvider->handleDrop (cdinfo);

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainer::preHandleDragStart (const CuiDragInfo & cdinfo)
{
	if (m_containerProvider)
		return m_containerProvider->handleDragStart (cdinfo);

	return false;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setType (Type type)
{
	m_type = type;
	if (m_details)
		 m_details->setType (m_type);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setCurrentSortOnColumnState(int const column, UITableModel::SortDirection const direction)
{
	if (m_details != 0)
	{
		bool const wasActive = m_details->isActive();

		if (!wasActive)
		{
			m_details->activateInContainer(this);
		}

		m_details->setSortOnColumnState(column, direction);

		// the sorting actually happens on deactivate for now...
		m_details->deactivate();
		m_details->setWasVisible(false);

		if (wasActive)
		{
			m_details->activateInContainer(this);
		}
	}

	if (m_viewType == VT_icon)
	{
		updateContents();
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::getCurrentSortOnColumnState(int & column, UITableModel::SortDirection & direction) const
{
	if (m_details != 0)
	{
		if (!m_details->getSortOnColumnState(column, direction))
		{
			column = m_details->getDefaultSortColumn();
		}
	}
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainer::getDefaultSortColumn() const
{
	if (m_details != 0)
	{
		return m_details->getDefaultSortColumn();
	}
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::clearViewers()
{
	for (ViewerMap::const_iterator it = m_viewers->begin (); it != m_viewers->end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = NON_NULL((*it).second);
		
		if (viewer)
		{
			CuiIconManager::unregisterObjectIcon(*viewer);
			viewer->Detach(0);
		}
	}
	
	m_viewers->clear ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::removeViewer(CachedNetworkId const & id, CuiWidget3dObjectListViewer * const viewer)
{
	if (viewer && m_viewers)
	{
		m_viewers->erase(id);
		CuiIconManager::unregisterObjectIcon(*viewer);
		viewer->Detach(0);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::setInventoryType(SwgCuiInventory::InventoryType type)
{
	switch(type)
	{
	case SwgCuiInventory::IT_NORMAL:		
		if(m_capacityArea)
			m_capacityArea->SetVisible(true);
		m_saveInventoryOrder = true;
		break;
	case SwgCuiInventory::IT_PUBLIC:
		m_objectDragType.clear();
		if(m_capacityArea)
			m_capacityArea->SetVisible(false);
		m_saveInventoryOrder = false;

		m_viewType = VT_detail;
		if (m_icons)
			m_icons->setWasVisible(false);
		if (m_details)
			m_details->setWasVisible(true);

		m_details->setDragable(false);

		break;
	case SwgCuiInventory::IT_LOOT:
		if(m_capacityArea)
			m_capacityArea->SetVisible(false);
		m_saveInventoryOrder = false;
		break;
	case SwgCuiInventory::IT_LIGHTSABER:		
		if(m_capacityArea)
			m_capacityArea->SetVisible(true);
		m_saveInventoryOrder = true;
		break;
	case SwgCuiInventory::IT_CRAFTING:
		if(m_capacityArea)
			m_capacityArea->SetVisible(true);
		m_saveInventoryOrder = false;
		break;
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::loadInventoryOrder ()                      
{
	if(!getContainerObject())
	{
		//DEBUG_WARNING(true, ("SwgCuiInventoryContainer::loadInventoryOrder without container"));
		return;
	}

	if(m_viewType != VT_icon)
		return;

	Unicode::String savedStringWide;
	std::string savedString;
	std::string ownerName("InvOrder_");
	ownerName.append(getContainerObject()->getNetworkId().getValueString());

	m_inventoryOrder.clear();
	CuiSettings::setPrefixString("");
	CuiSettings::loadString(ownerName, "InvOrder", savedStringWide);
	Game::SceneType sceneType = Game::getHudSceneType();	
	switch(sceneType)
	{
		case Game::ST_space:
			CuiSettings::setPrefixString ("space_");
			break;
		default:
			CuiSettings::setPrefixString ("");
			break;
	}
	savedString = Unicode::wideToNarrow(savedStringWide);

	//DEBUG_WARNING(true, ("SwgCuiInventoryContainer::loadInventoryOrder loading in %s %s", ownerName.c_str(), savedString.c_str()));

	while(!savedString.empty())
	{
		NetworkId netId(savedString);
		if(netId.isValid())
		{
			//DEBUG_WARNING(true, ("SwgCuiInventoryContainer::loadInventoryOrder got %s", netId.getValueString().c_str()));
			m_inventoryOrder.push_back(netId);
		}
		std::string::iterator curI = savedString.begin();
		++curI;
		while(curI != savedString.end() && (*curI != ' '))
			++curI;
		savedString.erase(savedString.begin(), curI);
	}
	m_containerProvider->tryToApplyOrdering(m_inventoryOrder);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::saveInventoryOrder ()
{
	if(!m_saveInventoryOrder)
		return;
	if(m_viewType != VT_icon)
		return;
	if(!getContainerObject())
	{
		//DEBUG_WARNING(true, ("saveInventoryOrder with no container object"));
		return;
	}
	if(m_objects->empty())
	{
		//DEBUG_WARNING(true, ("saveInventoryOrder with empty object list"));
		return;
	}
	std::string savedString;
	for(unsigned int i = 0; i < m_objects->size(); ++i)
	{
		ClientObject *obj = (*m_objects) [i].getPointer ();
		if(obj)
		{
			savedString.append(obj->getNetworkId().getValueString());
			savedString.append(" ");
		}
	}

	std::string ownerName("InvOrder_");
	ownerName.append(getContainerObject()->getNetworkId().getValueString());
	CuiSettings::setPrefixString("");
	CuiSettings::saveString(ownerName, "InvOrder", Unicode::narrowToWide(savedString));
	CuiSettings::save();
	Game::SceneType sceneType = Game::getHudSceneType();	
	switch(sceneType)
	{
	case Game::ST_space:
		CuiSettings::setPrefixString ("space_");
		break;
	default:
		CuiSettings::setPrefixString ("");
		break;
	}

	//DEBUG_WARNING(true, ("SwgCuiInventoryContainer::saveInventoryOrder saving out %s %s", ownerName.c_str(), savedString.c_str()));
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::handleDropThrough(UIWidget * const obj)
{
	CuiDragInfo cdinfo (*obj);

	if (preHandleDrop (cdinfo))
		return;

	if (cdinfo.type == CuiDragInfoTypes::CDIT_object)
	{
		std::string slotname;
		ClientObject * destination = getContainerObject();
		if(destination)
		{
			handleDrop (cdinfo, *destination, slotname);			
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainer::openSelectedRadial()
{
	if(m_icons)
		m_icons->openSelectedRadial();
}

// ======================================================================
