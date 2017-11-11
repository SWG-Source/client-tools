//======================================================================
//
// SwgCuiMissionBrowser.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMissionBrowser_TableModel.h"

#include "UIButton.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiStringIdsMission.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/PopulateMissionBrowserMessage.h"
#include "swgClientUserInterface/SwgCuiMissionDetails.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"

#include <vector>

//======================================================================

namespace SwgCuiMissionBrowserNamespace
{
	namespace Tabnames
	{
		const char * const Destroy  = "Destroy";
		const char * const Deliver  = "Deliver";
		const char * const Bounty   = "Bounty";
		const char * const Hunting  = "Hunting";
		const char * const Assassin = "Assassin";
		const char * const Recon    = "Recon";
		const char * const Dancer   = "Dancer";
		const char * const Musician = "Musician";
		const char * const Crafting = "Crafting";
		const char * const Survey   = "Survey";
	}
	
	namespace MissionTypes
	{
		const unsigned int destroy         = CrcLowerString::calculateCrc("destroy");
		const unsigned int recon           = CrcLowerString::calculateCrc("recon");
		const unsigned int deliver         = CrcLowerString::calculateCrc("deliver");
		const unsigned int escorttocreator = CrcLowerString::calculateCrc("escorttocreator");
		const unsigned int escort          = CrcLowerString::calculateCrc("escort");
		const unsigned int bounty          = CrcLowerString::calculateCrc("bounty");
		const unsigned int survey          = CrcLowerString::calculateCrc("survey");
		const unsigned int crafting        = CrcLowerString::calculateCrc("crafting");
		const unsigned int musician        = CrcLowerString::calculateCrc("musician");
		const unsigned int dancer          = CrcLowerString::calculateCrc("dancer");
		const unsigned int hunting         = CrcLowerString::calculateCrc("hunting");
		const unsigned int assassin        = CrcLowerString::calculateCrc("assassin");
	}

	namespace MissionTabName
	{
		const UILowerString name("Name");
		const UILowerString text("Text");
	}

	const float maxRange = 32.0f;
}

using namespace SwgCuiMissionBrowserNamespace;

//----------------------------------------------------------------------

SwgCuiMissionBrowser::SwgCuiMissionBrowser (UIPage & page) :
CuiMediator                        ("SwgCuiMissionBrowser", page),
UIEventCallback                    (),
m_table                            (0),
m_buttonExit                       (0),
m_buttonRefresh                    (0),
m_buttonAccept                     (0),
m_buttonDelete                     (0),
m_buttonDetails                    (0),
m_tabbedPane                       (0),
m_callback                         (new MessageDispatch::Callback),
m_tableModel                       (new TableModel),
m_requestListType                  (0),
m_currentTab                       (-1L),
m_details                          (0),
m_detailsRequestListResponse       (new MessageQueueMissionListResponseData),
m_terminal                         ()
{
	m_tableModel->Attach (0);

	getCodeDataObject (TUITable,      m_table,         "table");
	getCodeDataObject (TUIButton,     m_buttonExit,    "buttonExit");
	getCodeDataObject (TUIButton,     m_buttonRefresh, "buttonRefresh");
	getCodeDataObject (TUIButton,     m_buttonAccept,  "buttonAccept");
	getCodeDataObject (TUIButton,     m_buttonDelete,  "buttonDelete");
	getCodeDataObject (TUIButton,     m_buttonDetails, "buttonDetails");
	getCodeDataObject (TUITabbedPane, m_tabbedPane,    "tabs");

	m_tableModel->SetName("BrowserTable");
	getPage().AddChild(m_tableModel);
	m_table->SetTableModel (m_tableModel);
	m_tableModel->updateTableColumnSizes (*m_table);
	m_tableModel->fireColumnsChanged();

	m_tabbedPane->SetActiveTab (m_currentTab);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	//@todo: this mediator should stop listening for these when it is deactivated
	m_callback->connect (*this, &SwgCuiMissionBrowser::onClientMissionObjectReadyForDisplay, static_cast<ClientMissionObject::Messages::EnteredMissionBag *> (0));
	m_callback->connect (*this, &SwgCuiMissionBrowser::onClientMissionObjectRemovedFromBrowser, static_cast<ClientMissionObject::Messages::LeftMissionBag *> (0));
}

//----------------------------------------------------------------------

SwgCuiMissionBrowser::~SwgCuiMissionBrowser ()
{
	m_details = 0;

	delete m_callback;
	m_callback = 0;

	m_table = 0;
	m_buttonExit = 0;
	m_buttonRefresh = 0;
	m_buttonAccept = 0;
	m_buttonDelete = 0;
	m_buttonDetails = 0;
	m_tabbedPane = 0;
	
	m_tableModel->Detach (0);
	m_tableModel = 0; //lint !e423 creating a leak, no we're not (we pass ownership)

	delete m_detailsRequestListResponse;
	m_detailsRequestListResponse = 0;

	m_currentTab = -1L;
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::performActivate   ()
{
	m_table->AddCallback (this);
	m_buttonExit->AddCallback (this);
	m_buttonRefresh->AddCallback (this);
	m_buttonAccept->AddCallback (this);
	m_buttonDelete->AddCallback (this);
	m_buttonDetails->AddCallback (this);
	m_tabbedPane->AddCallback (this);

	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiMissionBrowser::onReceiveList,         static_cast<CuiMissionManager::Messages::ReceiveList *>     (0));
	m_callback->connect (*this, &SwgCuiMissionBrowser::onMissionAccept,       static_cast<CuiMissionManager::Messages::ResponseAccept *>  (0));
	m_callback->connect (*this, &SwgCuiMissionBrowser::onReceiveDetails,      static_cast<CuiMissionManager::Messages::ReceiveDetails *>  (0));

	setAssociatedObjectId(CuiMissionManager::getTerminalId());
	setMaxRangeFromObject(maxRange);

	if (m_details)
		m_details->deactivate ();

	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::performDeactivate ()
{
	if (m_details)
		m_details->deactivate ();

	m_callback->disconnect (*this, &SwgCuiMissionBrowser::onReceiveList,         static_cast<CuiMissionManager::Messages::ReceiveList *>     (0));
	m_callback->disconnect (*this, &SwgCuiMissionBrowser::onMissionAccept,       static_cast<CuiMissionManager::Messages::ResponseAccept *>  (0));
	m_callback->disconnect (*this, &SwgCuiMissionBrowser::onReceiveDetails,      static_cast<CuiMissionManager::Messages::ReceiveDetails *>  (0));

	CuiManager::requestPointer (false);

	m_table->RemoveCallback         (this);
	m_buttonExit->RemoveCallback    (this);
	m_buttonRefresh->RemoveCallback (this);
	m_buttonAccept->RemoveCallback  (this);
	m_buttonDelete->RemoveCallback  (this);
	m_buttonDetails->RemoveCallback (this);
	m_tabbedPane->RemoveCallback    (this);

	setIsUpdating(false);
}

//----------------------------------------------------------------------

bool SwgCuiMissionBrowser::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == m_table)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			viewMission ();
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::OnButtonPressed (UIWidget *context)
{
	if (context == m_buttonExit)
	{
		deactivate ();
	}
	else if (context == m_buttonRefresh)
	{
		refreshList ();
	}
	else if (context == m_buttonAccept)
	{
		acceptMission ();
	}	
	else if (context == m_buttonDelete)
	{
		deleteMission ();
	}	
	else if (context == m_buttonDetails)
	{
		viewMission ();
	}
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabbedPane)
	{
		long activeTab = m_tabbedPane->GetActiveTab ();
		if (activeTab >= 0)
		{
			std::string name;
			if (m_tabbedPane->GetTabName   (activeTab, name))
			{
				if (!_stricmp (name.c_str (), Tabnames::Destroy))
				{
					m_tableModel->setType (TableModel::T_Destroy);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Deliver))
				{
					m_tableModel->setType (TableModel::T_Deliver);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Bounty))
				{
					m_tableModel->setType (TableModel::T_Bounty);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Hunting))
				{
					m_tableModel->setType (TableModel::T_Hunting);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Assassin))
				{
					m_tableModel->setType (TableModel::T_Assassin);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Recon))
				{
					m_tableModel->setType (TableModel::T_Recon);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Dancer))
				{
					m_tableModel->setType (TableModel::T_Dancer);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Musician))
				{
					m_tableModel->setType (TableModel::T_Musician);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Crafting))
				{
					m_tableModel->setType (TableModel::T_Crafting);
					m_buttonAccept->SetVisible (true);
				}
				else if (!_stricmp (name.c_str (), Tabnames::Survey))
				{
					m_tableModel->setType (TableModel::T_Survey);
					m_buttonAccept->SetVisible (true);
				}
				else
				{
					WARNING (true, ("bad tab name %s", name.c_str ()));
				}
			}
			activeTab = m_tabbedPane->GetActiveTab ();
			m_currentTab = activeTab;
		}
	}
	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::refreshList ()
{
	//hide details screen if open to prevent stale data from being shown
	if (m_details && m_details->isActive ())
	{
		m_details->deactivate();
	}

	m_tableModel->clearAll ();

	//save and put back the user's currently selected tab (since the callback mechanism overrides the one in OnTabbedPaneChanged
	long tempCurrentTab = m_currentTab;
	m_tabbedPane->GetDataSource()->Clear();
	m_currentTab = tempCurrentTab;

	CuiMissionManager::requestList (false);
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::acceptMission () const
{
	const long index = m_table->GetLastSelectedRow ();
	const SwgCuiMissionBrowser::TableModel::DataElement * const elem = m_tableModel->findDataElement (index);

	if (!elem)
	{
		displayNoSelection ();
	}
	else
	{
		CuiMissionManager::acceptMission (elem->getNetworkId(), false);
	}
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::deleteMission () const
{
	const long index = m_table->GetLastSelectedRow ();
	const SwgCuiMissionBrowser::TableModel::DataElement * const elem = m_tableModel->findDataElement (index);

	if (!elem)
	{
		displayNoSelection ();
	}
	else
	{
		CuiMissionManager::removeMission (elem->getNetworkId(), false);
	}
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::viewMission ()
{
	const long index = m_table->GetLastSelectedRow ();
	const SwgCuiMissionBrowser::TableModel::DataElement * const elem = m_tableModel->findDataElement (index);

	if (!elem)
	{
		displayNoSelection ();
	}
	else
	{
		CuiMissionManager::setDetails(elem);
			
		if (!m_details)
			m_details = NON_NULL(dynamic_cast<SwgCuiMissionDetails*>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::MissionDetails)));
	
		m_details->deactivate();
		m_details->updateDetails();
		m_details->activate();
	}
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::displayNoSelection () const
{
	CuiMessageBox::createInfoBox (CuiStringIdsMission::err_no_mission_selected.localize ());
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::onReceiveList    (const PopulateMissionBrowserMessage & )
{
	switch (m_requestListType)
	{	
	case TableModel::T_Destroy:
		m_tableModel->setData (TableModel::T_Destroy, CuiMissionManager::getCurrentDestroyMissions());
		break;
	case TableModel::T_Deliver:
		m_tableModel->setData (TableModel::T_Deliver, CuiMissionManager::getCurrentDeliverMissions());
		break;
	case TableModel::T_Bounty:
		m_tableModel->setData (TableModel::T_Bounty,  CuiMissionManager::getCurrentBountyMissions());
		break;
	case TableModel::T_Hunting:
		m_tableModel->setData (TableModel::T_Hunting,  CuiMissionManager::getCurrentHuntingMissions());
		break;
	case TableModel::T_Assassin:
		m_tableModel->setData (TableModel::T_Assassin,  CuiMissionManager::getCurrentAssassinMissions());
		break;
	case TableModel::T_Recon:
		m_tableModel->setData (TableModel::T_Recon,  CuiMissionManager::getCurrentReconMissions());
		break;
	case TableModel::T_Dancer:
		m_tableModel->setData (TableModel::T_Dancer,  CuiMissionManager::getCurrentDancerMissions());
		break;
	case TableModel::T_Musician:
		m_tableModel->setData (TableModel::T_Musician,  CuiMissionManager::getCurrentMusicianMissions());
		break;
	case TableModel::T_Crafting:
		m_tableModel->setData (TableModel::T_Crafting,  CuiMissionManager::getCurrentCraftingMissions());
		break;
	case TableModel::T_Survey:
		m_tableModel->setData (TableModel::T_Survey,  CuiMissionManager::getCurrentSurveyMissions());
		break;
	default:
		WARNING (true, ("Bad request state\n"));
		break;
	}

	m_tableModel->updateTableColumnSizes (*m_table);
	m_tableModel->fireColumnsChanged();

	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::onReceiveDetails (const MessageQueueMissionDetailsResponse &)
{
	if (!m_details)
		m_details = NON_NULL(dynamic_cast<SwgCuiMissionDetails*>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::MissionDetails)));
}

//-----------------------------------------------------------------

void SwgCuiMissionBrowser::onMissionAccept(const MessageQueueMissionGenericResponse&)
{
	//return to destroy page after an accept
	m_tableModel->setData (TableModel::T_Destroy, CuiMissionManager::getCurrentDestroyMissions());
	m_tableModel->updateTableColumnSizes (*m_table);
	m_tableModel->fireColumnsChanged();
}

//-----------------------------------------------------------------

void SwgCuiMissionBrowser::onClientMissionObjectReadyForDisplay(const ClientMissionObject & mission)
{
	if(mission.isSettingBaselines())
		return;

	long tempCurrentTab = m_currentTab;

	//see if it's a destroy-type mission
	if(mission.getType() == MissionTypes::destroy)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Destroy);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Destroy);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::destroy_tab.localize()));
			ds->AddChildByPosition(newData, 0);
		}

		CuiMissionManager::addCurrentDestroyMission(&mission);
		m_tableModel->setData (TableModel::T_Destroy, CuiMissionManager::getCurrentDestroyMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}

	//see if it's a deliver-type mission
	else if(mission.getType() == MissionTypes::deliver || mission.getType() == MissionTypes::escorttocreator || mission.getType() == MissionTypes::escort)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Deliver);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Deliver);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::deliver_tab.localize()));
			ds->AddChildByPosition(newData, 1);
		}

		CuiMissionManager::addCurrentDeliverMission(&mission);
		m_tableModel->setData (TableModel::T_Deliver, CuiMissionManager::getCurrentDeliverMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}

	//see if it's a bounty-type mission
	else if(mission.getType() == MissionTypes::bounty)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Bounty);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Bounty);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::bounty_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentBountyMission(&mission);

		m_tableModel->setData (TableModel::T_Bounty, CuiMissionManager::getCurrentBountyMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::survey)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Survey);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Survey);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::survey_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentSurveyMission(&mission);

		m_tableModel->setData (TableModel::T_Survey, CuiMissionManager::getCurrentSurveyMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::dancer)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Dancer);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Dancer);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::dancer_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentDancerMission(&mission);

		m_tableModel->setData (TableModel::T_Dancer, CuiMissionManager::getCurrentDancerMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::musician)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Musician);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Musician);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::musician_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentMusicianMission(&mission);

		m_tableModel->setData (TableModel::T_Musician, CuiMissionManager::getCurrentMusicianMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::crafting)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Crafting);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Crafting);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::crafting_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentCraftingMission(&mission);

		m_tableModel->setData (TableModel::T_Crafting, CuiMissionManager::getCurrentCraftingMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::hunting)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Hunting);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Hunting);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::hunting_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentHuntingMission(&mission);

		m_tableModel->setData (TableModel::T_Hunting, CuiMissionManager::getCurrentHuntingMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::assassin)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Assassin);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Assassin);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::assassin_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentAssassinMission(&mission);

		m_tableModel->setData (TableModel::T_Assassin, CuiMissionManager::getCurrentAssassinMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else if(mission.getType() == MissionTypes::recon)
	{
		UIDataSource* ds = m_tabbedPane->GetDataSource();
		UIBaseObject* base = ds->GetChild(Tabnames::Recon);
		if(!base)
		{
			UIData* newData = new UIData();
			newData->SetName(Tabnames::Recon);
			IGNORE_RETURN(newData->SetProperty(MissionTabName::text, CuiStringIdsMission::recon_tab.localize()));
			ds->AddChildByPosition(newData, 2);
		}

		CuiMissionManager::addCurrentReconMission(&mission);

		m_tableModel->setData (TableModel::T_Recon, CuiMissionManager::getCurrentReconMissions());
		if (isActive ())
		{
			m_tableModel->updateTableColumnSizes (*m_table);
			m_tableModel->fireColumnsChanged();
		}
	}
	else
		DEBUG_WARNING(true, ("Unknown mission type: %lu received", mission.getType()));

	m_currentTab = tempCurrentTab;
	//make sure the correct tab is showing
	long numTabs = m_tabbedPane->GetTabCount();
	//only change tabs if we have tabs
	if(numTabs > 0 && m_currentTab < numTabs)
	{
		m_tabbedPane->SetActiveTab (-1);
		m_tabbedPane->SetActiveTab(m_currentTab);
		OnTabbedPaneChanged(m_tabbedPane);
	}
	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//-----------------------------------------------------------------

void SwgCuiMissionBrowser::onClientMissionObjectRemovedFromBrowser(const ClientMissionObject & mission)
{
	if(mission.getType() == MissionTypes::destroy || mission.getType() == MissionTypes::recon)
	{
		CuiMissionManager::removeDestroyMission(&mission);
		m_tableModel->setData (TableModel::T_Destroy, CuiMissionManager::getCurrentDestroyMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::deliver || mission.getType() == MissionTypes::escorttocreator || mission.getType() == MissionTypes::escort)
	{
		CuiMissionManager::removeDeliverMission(&mission);
		m_tableModel->setData (TableModel::T_Deliver, CuiMissionManager::getCurrentDeliverMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::bounty)
	{
		CuiMissionManager::removeBountyMission(&mission);
		m_tableModel->setData (TableModel::T_Bounty, CuiMissionManager::getCurrentBountyMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::dancer)
	{
		CuiMissionManager::removeDancerMission(&mission);
		m_tableModel->setData (TableModel::T_Dancer, CuiMissionManager::getCurrentDancerMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::musician)
	{
		CuiMissionManager::removeMusicianMission(&mission);
		m_tableModel->setData (TableModel::T_Musician, CuiMissionManager::getCurrentMusicianMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::crafting)
	{
		CuiMissionManager::removeCraftingMission(&mission);
		m_tableModel->setData (TableModel::T_Crafting, CuiMissionManager::getCurrentCraftingMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::survey)
	{
		CuiMissionManager::removeSurveyMission(&mission);
		m_tableModel->setData (TableModel::T_Survey, CuiMissionManager::getCurrentSurveyMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::hunting)
	{
		CuiMissionManager::removeHuntingMission(&mission);
		m_tableModel->setData (TableModel::T_Hunting, CuiMissionManager::getCurrentHuntingMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::assassin)
	{
		CuiMissionManager::removeAssassinMission(&mission);
		m_tableModel->setData (TableModel::T_Assassin, CuiMissionManager::getCurrentAssassinMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
	else if(mission.getType() == MissionTypes::recon)
	{
		CuiMissionManager::removeReconMission(&mission);
		m_tableModel->setData (TableModel::T_Recon, CuiMissionManager::getCurrentReconMissions());
		m_tableModel->updateTableColumnSizes (*m_table);
		m_tableModel->fireColumnsChanged();
		m_table->RemoveRowSelection(0);
	}
}

//======================================================================
