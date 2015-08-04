// ======================================================================
//
// SwgCuiDataStorage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiDataStorage.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSetName.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDraft.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "swgClientUserInterface/SwgCuiContainerProviderPoi.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UnicodeUtils.h"

// ======================================================================

namespace SwgCuiDataStorageNamespace
{
	namespace Tabs
	{
		enum Id
		{
			I_waypoints,
			I_groupWaypoints,
			I_data,
			I_poi,
			I_draftschematics,
			I_count
		};

		Id getId (const UITabbedPane & tabs)
		{
			const long index = tabs.GetActiveTab ();

			if (index >= 0 && index < I_count)
				return static_cast<Id>(index);
			return I_count;
		}
	}

	ClientObject * findPlayerDataStorage ()
	{
		ClientObject * const player = Game::getClientPlayer ();

		if (!player)
			return 0;

		SlottedContainer *const equipmentContainer = NON_NULL(player->getSlottedContainerProperty());

		const SlotId inventorySlot = SlotIdManager::findSlotId (ConstCharCrcLowerString ("datapad"));

		DEBUG_FATAL (inventorySlot == SlotId::invalid, ("invalid inventory slot\n"));
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		CachedNetworkId dataStorageId (equipmentContainer->getObjectInSlot(inventorySlot, tmp));

		ClientObject * dataStorage = safe_cast<ClientObject *>(dataStorageId.getObject ());

		if (!dataStorage && Game::getSinglePlayer ())
		{
			ClientObject * inventoryObject = safe_cast<ClientObject*>(ObjectTemplate::createObject("object/tangible/datapad/shared_character_datapad.iff"));
			if (!inventoryObject)
				return 0;

			DEBUG_FATAL(!inventoryObject, ("Could not create an inventory for the creature"));

			dataStorage = inventoryObject;

			// fake the endBaselines() call
			ClientObject *clientObject = safe_cast<ClientObject*> (inventoryObject);
			clientObject->endBaselines ();

			if (!equipmentContainer)
			{
				DEBUG_FATAL(true, ("player does not have an slotted container for equipment.\n"));
				return 0;
			}

			int validArrangementIndex = -1;
			Container::ContainerErrorCode tmp = Container::CEC_Success;

			if (!equipmentContainer->getFirstUnoccupiedArrangement(*inventoryObject, validArrangementIndex, tmp) || (validArrangementIndex < 0))
			{
				DEBUG_FATAL(true,("Must be able to create the inventory"));
				return 0;
			}

			if (!ContainerInterface::transferItemToSlottedContainer(*player, *inventoryObject, static_cast<int>(validArrangementIndex)))
			{
				DEBUG_FATAL(true,("Must be able to install the inventory"));
				return 0;
			}

			//-----------------------------------------------------------------

			const char * const hackObjectNames [] =
			{
				"object/intangible/data_item/shared_data_rebel.iff",
				"object/intangible/data_item/shared_data_imperial.iff"
			};

			const int arraySize = sizeof (hackObjectNames) / sizeof (hackObjectNames [0]);

			{
				for (int i = 0; i < arraySize; ++i)
				{
					ClientObject * const cobj = safe_cast<ClientObject *>(ObjectTemplate::createObject (hackObjectNames [i]));

					if (cobj)
					{
						cobj->endBaselines ();
						IGNORE_RETURN(ContainerInterface::transferItemToVolumeContainer (*inventoryObject, *cobj));
					}
				}
			}
		}

		return dataStorage;
	}

	void saveSortSettings(SwgCuiInventoryContainer const * container,
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	void loadSortSettings(SwgCuiInventoryContainer * container,
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	namespace Settings
	{
		const std::string waypointsCurrentPlanetOnly    = "waypointsCurrentPlanetOnly";
		const std::string waypointsSingleWaypointMode   = "waypointsSingleWaypointMode";
		const std::string datapadTabSelection           = "datapadTabSelection";
		const std::string datapadWaypointsSortColumn    = "datapadWaypointsSortColumn";
		const std::string datapadWaypointsSortSelection = "datapadWaypointsSortSelection";
		const std::string datapadDataSortColumn         = "datapadDataSortColumn";
		const std::string datapadDataSortSelection      = "datapadDataSortSelection";
		const std::string datapadPoiSortColumn          = "datapadPoiSortColumn";
		const std::string datapadPoiSortSelection       = "datapadPoiSortSelection";
		const std::string datapadDraftSortColumn        = "datapadDraftSortColumn";
		const std::string datapadDraftSortSelection     = "datapadDraftSortSelection";
	}

}

//----------------------------------------------------------------------

void SwgCuiDataStorageNamespace::saveSortSettings(SwgCuiInventoryContainer const * const container,
														std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	int column = 0;
	UITableModel::SortDirection direction;

	if (container != 0)
	{
		container->getCurrentSortOnColumnState(column, direction);
		CuiSettings::saveInteger(mediatorName, columnName, column);
		CuiSettings::saveInteger(mediatorName, sortDirectionName, direction);
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorageNamespace::loadSortSettings(SwgCuiInventoryContainer * const container,
														std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	if ((container != 0) && (CuiPreferences::getAutoSortDataPadContents()))
	{
		int column = container->getDefaultSortColumn();
		int direction = 0;

		CuiSettings::loadInteger(mediatorName, columnName, column);
		CuiSettings::loadInteger(mediatorName, sortDirectionName, direction);

		container->setCurrentSortOnColumnState(column, static_cast<UITableModel::SortDirection>(direction));
	}
}

using namespace SwgCuiDataStorageNamespace;

//----------------------------------------------------------------------

struct SwgCuiDataStorage::FilterWps :
public SwgCuiContainerProviderFilter
{
	FilterWps();
	~FilterWps () {}

	bool showObject (const ClientObject & obj) const;
	void setShowCurrentPlanetOnly(bool currentOnly) {m_currentPlanetOnly = currentOnly;}
	void setShowGroupWaypoints(bool enable) {m_showGroupWaypoints = enable;}
	void setSingleWaypointMode(bool enable) {m_singleWaypointMode = enable;}

	Tabs::Id m_id;

private:
	bool m_currentPlanetOnly;
	bool m_showGroupWaypoints;
	bool m_singleWaypointMode;
};

//----------------------------------------------------------------------

SwgCuiDataStorage::FilterWps::FilterWps () :
	m_id(Tabs::I_count),
	m_currentPlanetOnly(false),
	m_showGroupWaypoints(false),
	m_singleWaypointMode(true)
{
}

//----------------------------------------------------------------------

bool SwgCuiDataStorage::FilterWps::showObject (const ClientObject & obj) const
{
	ClientWaypointObject const * const wp = dynamic_cast<ClientWaypointObject const * const>(&obj);
	if (   wp
	    && wp->isGroupWaypoint() == m_showGroupWaypoints
			&& (   !m_currentPlanetOnly
	        || wp->getPlanetName () == Game::getNonInstanceSceneId()))
		return true;
	return false;
}

//----------------------------------------------------------------------

struct SwgCuiDataStorage::FilterData :
public SwgCuiContainerProviderFilter
{
	FilterData ();
	~FilterData () {}

	bool showObject (const ClientObject & obj) const;

	Tabs::Id m_id;
};

//----------------------------------------------------------------------

SwgCuiDataStorage::FilterData::FilterData () :
m_id (Tabs::I_count)
{
}

//----------------------------------------------------------------------

bool SwgCuiDataStorage::FilterData::showObject (const ClientObject & obj) const
{
	const ClientWaypointObject* const wp = dynamic_cast<const ClientWaypointObject* const>(&obj);
	if(wp)
		return false;
	else
		return true;
}

//----------------------------------------------------------------------

struct SwgCuiDataStorage::FilterPoi :
public SwgCuiContainerProviderFilter
{
	FilterPoi ();
	~FilterPoi () {}

	bool showObject (const ClientObject & obj) const;

	Tabs::Id m_id;
};

//----------------------------------------------------------------------

SwgCuiDataStorage::FilterPoi::FilterPoi () :
m_id (Tabs::I_count)
{
}

//----------------------------------------------------------------------

bool SwgCuiDataStorage::FilterPoi::showObject (const ClientObject & obj) const
{
	const ClientWaypointObject* const wp = dynamic_cast<const ClientWaypointObject* const>(&obj);
	if(wp)
	{
		if (wp->getPlanetName () == Game::getNonInstanceSceneId())
			return true;
		else
			return false;
	}
	else
		return false;
}

//-----------------------------------------------------------------

class SwgCuiDataStorage::CallbackReceiverShowGroupWaypoints: public CallbackReceiver
{
public:
	CallbackReceiverShowGroupWaypoints(SwgCuiDataStorage &_mediator) :
		CallbackReceiver(),
		m_mediator(&_mediator)
	{
	}

	void performCallback()
	{
		m_mediator->updateGroupWaypointTab();
	}

	SwgCuiDataStorage *m_mediator;

private:
	CallbackReceiverShowGroupWaypoints();
	CallbackReceiverShowGroupWaypoints & operator=(CallbackReceiverShowGroupWaypoints const &);
};

//-----------------------------------------------------------------

SwgCuiDataStorage::SwgCuiDataStorage (UIPage & page) :
CuiMediator         ("SwgCuiDataStorage", page),
UIEventCallback     (),
m_infoMediator      (0),
m_containerWaypoints(0),
m_containerData     (0),
m_containerPoi      (0),
m_containerDraft    (0),
m_capacityBar       (0),
m_capacityLabel     (0),
m_capacityBarWp     (0),
m_capacityLabelWp   (0),
m_buttonNewWaypoint (0),
m_tabs              (0),
m_waypointsCurrentPlanetOnly(0),
m_waypointsSingleWaypointMode(0),
m_waypointsOnScreen(0),
m_callback          (new MessageDispatch::Callback),
m_contentsDirty     (false),
m_filterWps         (new FilterWps),
m_filterData        (new FilterData),
m_filterPoi         (new FilterPoi),
m_containerObject   (new CachedNetworkId),
m_containerProviderWaypoints(new SwgCuiContainerProviderDefault),
m_containerProviderData   (new SwgCuiContainerProviderDefault),
m_containerProviderPoi    (new SwgCuiContainerProviderPoi),
m_containerProviderDraft  (new SwgCuiContainerProviderDraft),
m_callbackReceiverShowGroupWaypoints(0)
{
	m_callbackReceiverShowGroupWaypoints = new CallbackReceiverShowGroupWaypoints(*this);

	getCodeDataObject (TUIPage,       m_capacityBar,       "capacityBar");
	getCodeDataObject (TUIText,       m_capacityLabel,     "capacityLabel");
	getCodeDataObject (TUIPage,       m_capacityBarWp,     "capacityBarwp");
	getCodeDataObject (TUIText,       m_capacityLabelWp,   "capacityLabelwp");
	getCodeDataObject (TUITabbedPane, m_tabs,              "tabs");
	getCodeDataObject (TUIButton,     m_buttonNewWaypoint, "buttonNewWaypoint");
	getCodeDataObject (TUICheckbox,   m_waypointsCurrentPlanetOnly, "checkWaypointCurrentPlanetOnly");
	getCodeDataObject (TUICheckbox,   m_waypointsSingleWaypointMode, "checkWaypointSingleWaypointMode");
	getCodeDataObject (TUICheckbox,   m_waypointsOnScreen, "checkWaypointShowWaypointIndicators");
	

	m_waypointsCurrentPlanetOnly->SetChecked(false);
	m_waypointsSingleWaypointMode->SetChecked(false);
	m_waypointsOnScreen->SetChecked(CuiPreferences::getShowWaypointArrowsOnscreen());

	UIPage * infoPage = 0;

	getCodeDataObject (TUIPage, infoPage, "info");
	m_infoMediator      = new SwgCuiInventoryInfo (*infoPage);
	m_infoMediator->fetch ();

	//set up wp tab
	UIPage * p = 0;
	getCodeDataObject (TUIPage, p, "containerWaypoints");
	m_containerWaypoints = new SwgCuiInventoryContainer (*p, SwgCuiInventoryContainer::T_waypointsSelf);
	m_containerWaypoints->fetch     ();
	m_containerProviderWaypoints->setFilter (m_filterWps);
	m_containerWaypoints->setContainerProvider (m_containerProviderWaypoints);

	//set up data tab
	getCodeDataObject (TUIPage, p, "containerData");
	m_containerData = new SwgCuiInventoryContainer (*p, SwgCuiInventoryContainer::T_datapadSelf);
	m_containerData->fetch     ();
	m_containerProviderData->setFilter (m_filterData);
	m_containerData->setContainerProvider (m_containerProviderData);

	//set up poi tab
	getCodeDataObject (TUIPage, p, "containerPoi");
	m_containerPoi = new SwgCuiInventoryContainer (*p, SwgCuiInventoryContainer::T_poi);
	m_containerPoi->setOwnedByUI(true);
	m_containerPoi->fetch ();
	m_containerProviderPoi->setFilter (m_filterPoi);
	m_containerPoi->setContainerProvider (m_containerProviderPoi);

	//set up draft schematics tab
	getCodeDataObject (TUIPage, p, "containerDraft");
	m_containerDraft = new SwgCuiInventoryContainer (*p, SwgCuiInventoryContainer::T_draftSchematics);
	m_containerDraft->setOwnedByUI(true);
	m_containerDraft->fetch ();
	m_containerDraft->setContainerProvider (m_containerProviderDraft);

	//-- connect the info mediator to selection changes in the container mediator
	m_infoMediator->connectToSelectionTransceiver (m_containerWaypoints->getTransceiverSelection ());
	m_infoMediator->connectToSelectionTransceiver (m_containerData->getTransceiverSelection      ());
	m_infoMediator->connectToSelectionTransceiver (m_containerPoi->getTransceiverSelection       ());
	m_infoMediator->connectToSelectionTransceiver (m_containerDraft->getTransceiverSelection     ());

	setContainerObject(findPlayerDataStorage());

	registerMediatorObject (*m_tabs,              true);
	registerMediatorObject (*m_buttonNewWaypoint, true);
	registerMediatorObject (*m_waypointsCurrentPlanetOnly, true);
	registerMediatorObject (*m_waypointsSingleWaypointMode, true);
	registerMediatorObject (*m_waypointsOnScreen, true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

} //lint !e429 icon not freed or returned, ownership is passed off

//-----------------------------------------------------------------

SwgCuiDataStorage::~SwgCuiDataStorage ()
{
	delete m_containerObject;
	m_containerObject = 0;

	delete m_callback;
	m_callback = 0;

	m_capacityBar     = 0;
	m_capacityLabel   = 0;
	m_capacityBarWp   = 0;
	m_capacityLabelWp = 0;
	m_tabs            = 0;
	m_buttonNewWaypoint = 0;
	m_waypointsCurrentPlanetOnly = 0;
	m_waypointsSingleWaypointMode = 0;
	m_waypointsOnScreen = 0;

	if (m_infoMediator)
	{
		m_infoMediator->disconnectFromSelectionTransceiver (m_containerWaypoints->getTransceiverSelection ());
		m_infoMediator->disconnectFromSelectionTransceiver (m_containerData->getTransceiverSelection      ());
		m_infoMediator->disconnectFromSelectionTransceiver (m_containerPoi->getTransceiverSelection       ());
		m_infoMediator->disconnectFromSelectionTransceiver (m_containerDraft->getTransceiverSelection     ());
		m_infoMediator->release ();
		m_infoMediator = 0;
	}

	m_containerWaypoints->setFilter (0);
	m_containerWaypoints->release ();
	m_containerWaypoints = 0;

	m_containerData->setFilter (0);
	m_containerData->release ();
	m_containerData = 0;

	m_containerPoi->setFilter (0);
	m_containerPoi->release ();
	m_containerPoi = 0;

	m_containerDraft->setFilter (0);
	m_containerDraft->release ();
	m_containerDraft = 0;

	m_containerProviderWaypoints->setFilter (0);
	delete m_containerProviderWaypoints;
	m_containerProviderWaypoints = 0;

	m_containerProviderData->setFilter (0);
	delete m_containerProviderData;
	m_containerProviderData = 0;

	m_containerProviderPoi->setFilter (0);
	delete m_containerProviderPoi;
	m_containerProviderPoi = 0;

	m_containerProviderDraft->setFilter (0);
	delete m_containerProviderDraft;
	m_containerProviderDraft = 0;

	delete m_filterWps;
	m_filterWps = 0;
	delete m_filterData;
	m_filterData = 0;
	delete m_filterPoi;
	m_filterPoi = 0;

	delete m_callbackReceiverShowGroupWaypoints;
	m_callbackReceiverShowGroupWaypoints = 0;
}

//-----------------------------------------------------------------

void SwgCuiDataStorage::performActivate ()
{
	CuiPreferences::getShowGroupWaypointsCallback().attachReceiver(*m_callbackReceiverShowGroupWaypoints);

	updateGroupWaypointTab();

	m_infoMediator->activate ();

	OnTabbedPaneChanged (m_tabs);

	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiDataStorage::onContentsChanged,         static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->connect (*this, &SwgCuiDataStorage::onContentsChanged,         static_cast<ClientObject::Messages::RemovedFromContainer *> (0));

	setContainerObject (findPlayerDataStorage ());

	populate();

	removeState(MS_settingsLoaded);
}

//-----------------------------------------------------------------

void SwgCuiDataStorage::performDeactivate ()
{
	CuiPreferences::getShowGroupWaypointsCallback().detachReceiver(*m_callbackReceiverShowGroupWaypoints);

	m_callback->disconnect (*this, &SwgCuiDataStorage::onContentsChanged,         static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->disconnect (*this, &SwgCuiDataStorage::onContentsChanged,         static_cast<ClientObject::Messages::RemovedFromContainer *> (0));

	m_infoMediator->deactivate   ();

	if (m_containerData->isActive ())
	{
		m_containerWaypoints->setViewType (m_containerData->getViewType ());
		m_containerPoi->setViewType       (m_containerData->getViewType ());
		m_containerDraft->setViewType     (m_containerData->getViewType ());
	}
	else if (m_containerDraft->isActive ())
	{
		m_containerWaypoints->setViewType (m_containerDraft->getViewType ());
		m_containerData->setViewType      (m_containerDraft->getViewType ());
		m_containerPoi->setViewType       (m_containerDraft->getViewType ());
	}
	else if (m_containerPoi->isActive ())
	{
		m_containerWaypoints->setViewType (m_containerPoi->getViewType ());
		m_containerData->setViewType      (m_containerPoi->getViewType ());
		m_containerDraft->setViewType     (m_containerPoi->getViewType ());
	}
	else
	{
		m_containerData->setViewType  (m_containerWaypoints->getViewType ());
		m_containerPoi->setViewType   (m_containerPoi->getViewType       ());
		m_containerDraft->setViewType (m_containerWaypoints->getViewType ());
	}

	m_containerWaypoints->deactivate();
	m_containerData->deactivate     ();
	m_containerPoi->deactivate      ();
	m_containerDraft->deactivate    ();

	CuiManager::requestPointer (false);
}

//-----------------------------------------------------------------

void SwgCuiDataStorage::OnButtonPressed( UIWidget *context )
{
	if(context == m_buttonNewWaypoint)
	{
		ClientObject * const player = Game::getClientPlayer ();
		if (player)
		{
			ClientWaypointObject::requestWaypoint (Unicode::emptyString, player->getPosition_w ());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::OnCheckboxSet                (UIWidget * context)
{
	bool updateContainer = false;

	if (context == m_waypointsCurrentPlanetOnly)
	{
		m_filterWps->setShowCurrentPlanetOnly(m_waypointsCurrentPlanetOnly->IsChecked());
		updateContainer = true;
	}
	else if (context == m_waypointsSingleWaypointMode)
	{
		m_filterWps->setSingleWaypointMode(true);
		ClientWaypointObject::setSingleWaypointMode(true, NULL);
		updateContainer = true;
	}
	else if (context == m_waypointsOnScreen)
	{
		CuiPreferences::setShowWaypointArrowsOnscreen(m_waypointsOnScreen->IsChecked());
	}

	if(updateContainer) 
	{
		m_containerProviderWaypoints->updateObjectVector();
		m_containerProviderWaypoints->setContentDirty(true);
		m_containerProviderWaypoints->setProviderDirty(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::OnCheckboxUnset              (UIWidget * context)
{
	bool updateContainer = false;

	if (context == m_waypointsCurrentPlanetOnly)
	{
		m_filterWps->setShowCurrentPlanetOnly(m_waypointsCurrentPlanetOnly->IsChecked());
	}
	else if (context == m_waypointsSingleWaypointMode)
	{
		m_filterWps->setSingleWaypointMode(false);
		ClientWaypointObject::setSingleWaypointMode(false, NULL);
		updateContainer = true;
	}
	else if (context == m_waypointsOnScreen)
	{
		CuiPreferences::setShowWaypointArrowsOnscreen(m_waypointsOnScreen->IsChecked());
	}
	
	if (updateContainer) 
	{
		m_containerProviderWaypoints->updateObjectVector();
		m_containerProviderWaypoints->setContentDirty(true);
		m_containerProviderWaypoints->setProviderDirty(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::OnTabbedPaneChanged (UIWidget * context)
{
	if (m_tabs == context)
	{
		SwgCuiInventoryContainer* newPage = NULL;
		std::vector<SwgCuiInventoryContainer*> otherPages;

		if (m_tabs->GetActiveTab () == Tabs::I_waypoints || m_tabs->GetActiveTab () == Tabs::I_groupWaypoints)
		{
			m_filterWps->setShowGroupWaypoints(m_tabs->GetActiveTab() == Tabs::I_groupWaypoints);
			m_containerProviderWaypoints->updateObjectVector();
			m_containerProviderWaypoints->setContentDirty(true);
			m_containerProviderWaypoints->setProviderDirty(true);
			newPage = m_containerWaypoints;
			otherPages.push_back(m_containerData);
			otherPages.push_back(m_containerPoi);
			otherPages.push_back(m_containerDraft);
		}
		else if(m_tabs->GetActiveTab () == Tabs::I_data)
		{
			newPage = m_containerData;
			otherPages.push_back(m_containerWaypoints);
			otherPages.push_back(m_containerPoi);
			otherPages.push_back(m_containerDraft);
		}
		else if(m_tabs->GetActiveTab () == Tabs::I_poi)
		{
			newPage = m_containerPoi;
			otherPages.push_back(m_containerWaypoints);
			otherPages.push_back(m_containerData);
			otherPages.push_back(m_containerDraft);
		}
		else if(m_tabs->GetActiveTab () == Tabs::I_draftschematics)
		{
			newPage = m_containerDraft;
			otherPages.push_back(m_containerWaypoints);
			otherPages.push_back(m_containerData);
			otherPages.push_back(m_containerPoi);
		}
		else
		{
			DEBUG_FATAL(true, ("Bad tab number"));
		}

		for (std::vector<SwgCuiInventoryContainer*>::iterator i = otherPages.begin(); i != otherPages.end(); ++i)
		{
			SwgCuiInventoryContainer* current = *i;
			if(current)
			{
				if(current->isActive())
					newPage->setViewType(current->getViewType ());
				current->deactivate();
			}
		}
		newPage->activate();
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::setContainerObject (ClientObject * containerObject)
{
	m_containerData->setContainerObject (containerObject, std::string ());
	m_containerWaypoints->setContainerObject (containerObject, std::string ());

	if (containerObject)
		*m_containerObject = CachedNetworkId (*containerObject);
	else
		*m_containerObject = NetworkId::cms_invalid;

	populate();
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::onContentsChanged (const ClientObject::Messages::ContainerMsg & msg)
{
	ClientObject * const containerObject = m_containerData->getContainerObject ();

	if (containerObject)
	{
		const NetworkId & id = containerObject->getNetworkId ();
		if (msg.first && msg.first->getNetworkId () == id)
			populate();
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::populate()
{
	const ClientObject * const obj = m_containerData->getContainerObject ();
	const VolumeContainer * const container = obj ? ContainerInterface::getVolumeContainer (*obj) : 0;

	if (container)
	{
		float ratio = 0.0f;
		float wpRatio = 0.0f;

		const int totalVolume   = container->getTotalVolume ();
		const int currentVolume = container->getCurrentVolume ();

		PlayerObject const * const playerObj = Game::getPlayerObject();

		int waypointTotal = ConfigClientGame::getMaxWaypoints();
		int waypointCurrent = playerObj ? playerObj->getTotalWaypoints() : 0;

		if (totalVolume < 0 || currentVolume < 0)
		{
			WARNING (true, ("Data Pad total or current volume < 0 (%d total,%d current)", totalVolume, currentVolume));
		}
		else if (totalVolume)
		{
			ratio = static_cast<float>(currentVolume) / static_cast<float>(totalVolume);
		}

		if(waypointTotal)
			wpRatio = static_cast<float>(waypointCurrent) / static_cast<float>(waypointTotal);

		{
			const UIPoint & barLoc = m_capacityBar->GetLocation ();
			NOT_NULL (m_capacityBar->GetParent ());
			const long parentWidth = NON_NULL (safe_cast<UIWidget *>(m_capacityBar->GetParent ()))->GetWidth ();
			const long availableWidth = parentWidth - (barLoc.x * 2L);

			m_capacityBar->SetWidth (static_cast<long>(availableWidth * ratio));
		}

		{
			char buf [32];
			snprintf (buf, 32, "%3d%% (%d/%d)", totalVolume ? (currentVolume * 100 / totalVolume) : 0, currentVolume, totalVolume);
			m_capacityLabel->SetPreLocalized (true);
			m_capacityLabel->SetLocalText (Unicode::narrowToWide (buf));
		}

		{
			const UIPoint & barLoc = m_capacityBarWp->GetLocation ();
			NOT_NULL (m_capacityBarWp->GetParent ());
			const long parentWidth = NON_NULL (safe_cast<UIWidget *>(m_capacityBarWp->GetParent ()))->GetWidth ();
			const long availableWidth = parentWidth - (barLoc.x * 2L);

			m_capacityBarWp->SetWidth (static_cast<long>(availableWidth * wpRatio));
		}

		{
			char buf [32];
			snprintf (buf, 32, "%3d%% (%d/%d)", waypointTotal ? (waypointCurrent * 100 / waypointTotal) : 0, waypointCurrent, waypointTotal);
			m_capacityLabelWp->SetPreLocalized (true);
			m_capacityLabelWp->SetLocalText (Unicode::narrowToWide (buf));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::queryWaypointColor  (ClientWaypointObject const & cwo)
{
	UNREF(cwo);
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::saveSettings () const
{
	CuiMediator::saveSettings();

	CuiSettings::saveInteger (getMediatorDebugName (), Settings::datapadTabSelection, m_tabs->GetActiveTab ());
	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::waypointsCurrentPlanetOnly, m_waypointsCurrentPlanetOnly->IsChecked ());
	CuiSettings::saveBoolean(getMediatorDebugName(), Settings::waypointsSingleWaypointMode, m_waypointsSingleWaypointMode->IsChecked());

	saveSortSettings(m_containerWaypoints, getMediatorDebugName(), Settings::datapadWaypointsSortColumn, Settings::datapadWaypointsSortSelection);
	saveSortSettings(m_containerData, getMediatorDebugName(), Settings::datapadDataSortColumn, Settings::datapadDataSortSelection);
	saveSortSettings(m_containerPoi, getMediatorDebugName(), Settings::datapadPoiSortColumn, Settings::datapadPoiSortSelection);
	saveSortSettings(m_containerDraft, getMediatorDebugName(), Settings::datapadDraftSortColumn, Settings::datapadDraftSortSelection);
}


//----------------------------------------------------------------------

void SwgCuiDataStorage::loadSettings ()
{
	bool planetOnly = false;
	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::waypointsCurrentPlanetOnly, planetOnly))
	{
		m_waypointsCurrentPlanetOnly->SetChecked (planetOnly);
		if (planetOnly)
			OnCheckboxSet (m_waypointsCurrentPlanetOnly);
		else
			OnCheckboxUnset (m_waypointsCurrentPlanetOnly);
	}

	bool singleWaypoint = false;
	if (CuiSettings::loadBoolean(getMediatorDebugName(), Settings::waypointsSingleWaypointMode, singleWaypoint))
	{
		m_waypointsSingleWaypointMode->SetChecked(singleWaypoint);
		if (singleWaypoint)
			OnCheckboxSet(m_waypointsSingleWaypointMode);
		else
			OnCheckboxUnset(m_waypointsSingleWaypointMode);
	}

	loadSortSettings(m_containerWaypoints, getMediatorDebugName(), Settings::datapadWaypointsSortColumn, Settings::datapadWaypointsSortSelection);
	loadSortSettings(m_containerData, getMediatorDebugName(), Settings::datapadDataSortColumn, Settings::datapadDataSortSelection);
	loadSortSettings(m_containerPoi, getMediatorDebugName(), Settings::datapadPoiSortColumn, Settings::datapadPoiSortSelection);
	loadSortSettings(m_containerDraft, getMediatorDebugName(), Settings::datapadDraftSortColumn, Settings::datapadDraftSortSelection);

	int tabSel = 0;
	if (CuiSettings::loadInteger (getMediatorDebugName (), Settings::datapadTabSelection, tabSel))
	{
		m_tabs->SetActiveTab(tabSel);
		updateGroupWaypointTab();
		OnTabbedPaneChanged(m_tabs);
	}

	m_waypointsOnScreen->SetChecked(CuiPreferences::getShowWaypointArrowsOnscreen());
	ClientWaypointObject::setSingleWaypointMode(m_waypointsSingleWaypointMode->IsChecked(), m_containerWaypoints->getLastSelection(), true);

	CuiMediator::loadSettings();
}

//----------------------------------------------------------------------

void SwgCuiDataStorage::updateGroupWaypointTab()
{
	UIButton * const groupTab = m_tabs->GetTabButton(Tabs::I_groupWaypoints);
	if (groupTab)
	{
		bool const wasVisible = groupTab->IsVisible();
		if (wasVisible != CuiPreferences::getShowGroupWaypoints())
		{
			if (wasVisible && m_tabs->GetActiveTab() == Tabs::I_groupWaypoints)
				m_tabs->SetActiveTab(Tabs::I_waypoints);
			groupTab->SetVisible(!wasVisible);
			groupTab->SetEnabled(!wasVisible);
			m_tabs->SetSize(m_tabs->GetSize());
		}
	}
}

// ======================================================================
