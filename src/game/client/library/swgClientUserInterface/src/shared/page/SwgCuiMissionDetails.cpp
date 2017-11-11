//======================================================================
//
// SwgCuiMissionDetails.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMissionDetails.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsMission.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedObject/Container.h"
#include "sharedObject/ContainedByProperty.h"
#include "clientGame/ContainerInterface.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"

#include <ctime>
#include <cstdio>

//-----------------------------------------------------------------

namespace SwgCuiMissionDetailsNamespace
{
	namespace MissionType
	{
		enum Id
		{
			I_none,
			I_Destroy,
			I_Deliver,
			I_Bounty,
			I_Entertainer,
			I_Survey,
			I_Crafting,
			I_Hunting,
			I_Assassin
		};
		
		namespace Names
		{
			const std::string Destroy ("Destroy");
			const std::string Deliver ("Deliver");
			const std::string Bounty  ("Bounty");
			const std::string Entertainer ("Entertainer");
			const std::string Survey      ("Survey");
			const std::string Crafting    ("Crafting");
			const std::string Hunting     ("Hunting");
			const std::string Assassin    ("Assassin");
		}
		

		Id findId (const std::string & str)
		{
			if (!_stricmp (str.c_str (), Names::Destroy.c_str ()))
				return I_Destroy;
			else if (!_stricmp (str.c_str (), Names::Deliver.c_str ()))
				return I_Deliver;
			else if (!_stricmp (str.c_str (), Names::Bounty.c_str ()))
				return I_Bounty;
			else if (!_stricmp (str.c_str (), Names::Entertainer.c_str ()))
				return I_Entertainer;
			else if (!_stricmp (str.c_str (), Names::Survey.c_str ()))
				return I_Survey;
			else if (!_stricmp (str.c_str (), Names::Crafting.c_str ()))
				return I_Crafting;
			else if (!_stricmp (str.c_str (), Names::Hunting.c_str ()))
				return I_Hunting;
			else if (!_stricmp (str.c_str (), Names::Assassin.c_str ()))
				return I_Assassin;
			else
				return I_none;
		}
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

	namespace Properties
	{
		const Unicode::String str_false (Unicode::narrowToWide ("false"));
		const Unicode::String str_true  (Unicode::narrowToWide ("true"));
	}

	enum DetailsType
	{
		DT_ClientMissionObject,
		DT_None
	};
	DetailsType s_detailsType;
}

//-----------------------------------------------------------------

SwgCuiMissionDetails::SwgCuiMissionDetails (UIPage & page) :
CuiMediator               ("SwgCuiMissionDetails", page),
UIEventCallback           (),
MessageDispatch::Receiver (),
m_pageDestroy             (0),
m_pageDeliver             (0),
m_pageBounty              (0),
m_pageSurvey              (0),
m_pageEntertainer         (0),
m_pageCrafting            (0),
m_pageHunting             (0),
m_pageAssassin            (0),
m_textDescription         (0),
m_textTitle               (0),
m_buttonExit              (0),
m_buttonAccept            (0),
m_buttonAbort             (0),
m_buttonWaypoint          (0),
m_messageBox              (0),
m_callback                (new MessageDispatch::Callback),
m_missionObjectDetails    (0),
m_viewer                  (0),
m_target                  (0),
m_waypoint                (0),
m_defaultViewerPitch      (0)
{
	SwgCuiMissionDetailsNamespace::s_detailsType = SwgCuiMissionDetailsNamespace::DT_None;

	getCodeDataObject (TUIButton,     m_buttonExit,    "buttonExit");
	getCodeDataObject (TUIButton,     m_buttonAccept,  "buttonAccept");
	getCodeDataObject (TUIButton,     m_buttonAbort,   "buttonDelete");
	getCodeDataObject (TUIButton,     m_buttonWaypoint,"buttonWaypoint");

	getCodeDataObject (TUIPage,  m_pageDestroy,     "compDestroy");
	getCodeDataObject (TUIPage,  m_pageDeliver,     "compDeliver");
	getCodeDataObject (TUIPage,  m_pageBounty,      "compBounty");
	getCodeDataObject (TUIPage,  m_pageSurvey,      "compSurvey");
	getCodeDataObject (TUIPage,  m_pageEntertainer, "compEntertainer");
	getCodeDataObject (TUIPage,  m_pageCrafting,    "compCrafting");
	getCodeDataObject (TUIPage,  m_pageHunting,     "compHunting");
	getCodeDataObject (TUIPage,  m_pageAssassin,    "compAssassin");
	getCodeDataObject (TUIText,  m_textDescription, "textDescription");
	getCodeDataObject (TUIText,  m_textTitle,       "labelTitle");

	UIWidget * viewerWidget = 0;

	getCodeDataObject (TUIWidget, viewerWidget,     "viewer");
	m_viewer = safe_cast<CuiWidget3dObjectListViewer *>(viewerWidget);

	if (m_viewer)
	{
		m_viewer->setCameraLodBias (2.0f);
		m_defaultViewerPitch = m_viewer->getCameraPitch ();
		m_viewer->SetLocalTooltip    (CuiStringIds::tooltip_viewer_3d_controls.localize ());
		m_viewer->setAutoZoomOutOnly       (false);
		m_viewer->setCameraZoomInWhileTurn (false);
		m_viewer->setAlterObjects          (true);
		m_viewer->setCameraLookAtCenter    (true);
		m_viewer->setDragYawOk             (true);
		m_viewer->setPaused                (false);
		m_viewer->SetDragable              (false);	
		m_viewer->SetContextCapable        (true, false);
		m_viewer->setRotateSpeed           (1.0f);
		m_viewer->setCameraForceTarget     (false);
		m_viewer->setCameraTransformToObj  (true);
		m_viewer->setCameraLodBias         (3.0f);
		m_viewer->setCameraLodBiasOverride (true);
	}

	//hide all the details pages
	showDetailsPage(NULL);
}

//-----------------------------------------------------------------

SwgCuiMissionDetails::~SwgCuiMissionDetails ()
{
	m_pageDestroy      = 0;
	m_pageDeliver      = 0;
	m_pageBounty       = 0;
	m_pageSurvey       = 0;
	m_pageHunting      = 0;
	m_pageAssassin     = 0;
	m_buttonExit       = 0;
	m_buttonAccept     = 0;
	m_buttonAbort      = 0;
	m_buttonWaypoint   = 0;
	m_textDescription  = 0;
	m_messageBox       = 0;
	m_viewer           = 0;
	m_missionObjectDetails  = 0;
	m_waypoint              = 0;

	if(m_target)
		delete m_target;
	m_target = 0;

	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void SwgCuiMissionDetails::performActivate   ()
{
	m_buttonExit->AddCallback     (this);
	m_buttonAbort->AddCallback    (this);
	m_buttonWaypoint->AddCallback (this);
	m_buttonAccept->AddCallback   (this);

	CuiManager::requestPointer (true);

	m_viewer->setPaused (false);

	updateDetails();

	if(m_waypoint)
	{
		//show the waypoint button if there's a waypoint for this mission type
		m_buttonWaypoint->SetVisible(true);
		//set the text to be "Activate" or "Deactivate" depending on the waypoint status
		if(m_waypoint->isWaypointActive())
		{
			m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_deactivate.localize());
		}
		else
			m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_activate.localize());
	}
	else
		m_buttonWaypoint->SetVisible(false);

	IGNORE_RETURN (setState (MS_closeable));
	IGNORE_RETURN (setState (MS_closeDeactivates));

	m_textDescription->ScrollToPoint(UIPoint(0, 0));

	CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
	if(workspace)
	{
		workspace->focusMediator(*this, true);
	}

	m_callback->connect (*this, &SwgCuiMissionDetails::onMissionAccept,       static_cast<CuiMissionManager::Messages::ResponseAccept *>  (0));
	m_callback->connect (*this, &SwgCuiMissionDetails::onMissionRemove,       static_cast<CuiMissionManager::Messages::ResponseRemove *>  (0));
	m_callback->connect (*this, &SwgCuiMissionDetails::onMissionAbort,        static_cast<CuiMissionManager::Messages::ResponseAbort  *>  (0));

	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//-----------------------------------------------------------------

void SwgCuiMissionDetails::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiMissionDetails::onMissionAccept,       static_cast<CuiMissionManager::Messages::ResponseAccept *>  (0));
	m_callback->disconnect (*this, &SwgCuiMissionDetails::onMissionRemove,       static_cast<CuiMissionManager::Messages::ResponseRemove *>  (0));
	m_callback->disconnect (*this, &SwgCuiMissionDetails::onMissionAbort,        static_cast<CuiMissionManager::Messages::ResponseAbort  *>  (0));

	m_viewer->setPaused (true);

	CuiManager::requestPointer (false);

	m_buttonExit->RemoveCallback   (this);
	m_buttonAbort->RemoveCallback (this);
	m_buttonWaypoint->RemoveCallback (this);
	m_buttonAccept->RemoveCallback (this);

	m_waypoint   = NULL;
}

//-----------------------------------------------------------------

bool SwgCuiMissionDetails::OnMessage( UIWidget *context, const UIMessage & msg )
{
	UNREF (context);
	UNREF (msg);
	return true;
}

//-----------------------------------------------------------------

/* Close the details page if we accept the mission
 */
void SwgCuiMissionDetails::onMissionAccept(const MessageQueueMissionGenericResponse&)
{
	deactivate();
}

//-----------------------------------------------------------------

/* Close the details page if we abort or otherwise remove the mission
 */
void SwgCuiMissionDetails::onMissionRemove(const MessageQueueMissionGenericResponse&)
{
	deactivate();
}

//-----------------------------------------------------------------

/* Close the details page if we abort or otherwise remove the mission
 */
void SwgCuiMissionDetails::onMissionAbort(const MessageQueueNetworkId&)
{
	deactivate();
}

//-----------------------------------------------------------------

void SwgCuiMissionDetails::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonExit)
	{
		deactivate ();
	}
	//accept a mission
	else if (context == m_buttonAccept)
	{
		if(m_missionObjectDetails)
			CuiMissionManager::acceptMission (m_missionObjectDetails->getNetworkId(), false);
	}
	//abort an active mission
	else if (context == m_buttonAbort)
	{
		WARNING_STRICT_FATAL(SwgCuiMissionDetailsNamespace::s_detailsType != SwgCuiMissionDetailsNamespace::DT_ClientMissionObject, ("Bad details type for mission"));
		if(m_missionObjectDetails)
			CuiMissionManager::removeMission (m_missionObjectDetails->getNetworkId(), false);
	}
	//toggle the waypoint status
	else if (context == m_buttonWaypoint)
	{
		if(m_waypoint && !m_waypoint->isWaypointActive())
		{
			m_waypoint->setWaypointActive(true);
			m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_deactivate.localize());
		}
		else if(m_waypoint && m_waypoint->isWaypointActive())
		{
			m_waypoint->setWaypointActive(false);
			m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_activate.localize());
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiMissionDetails::receiveMessage (const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & )
{
}

//-----------------------------------------------------------------

void SwgCuiMissionDetails::clearDetails()
{
	m_textTitle->Clear();
	m_textDescription->Clear();
	delete m_target;
	m_target = 0;
	m_viewer->clearObjects();

	std::vector<UIPage*> pages;
	pages.push_back(m_pageDestroy);
	pages.push_back(m_pageDeliver);
	pages.push_back(m_pageBounty);
	pages.push_back(m_pageSurvey);
	pages.push_back(m_pageCrafting);
	pages.push_back(m_pageEntertainer);
	pages.push_back(m_pageHunting);
	pages.push_back(m_pageAssassin);

	for(std::vector<UIPage*>::iterator i = pages.begin(); i != pages.end(); ++i)
	{
		(*i)->SetProperty (UILowerString ("creator.text"),      Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("target.text"),       Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("reward.text"),       Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("location.text"),     Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("destlocation.text"), Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("location.text"),     Unicode::emptyString);
		(*i)->SetProperty (UILowerString ("efficiency.text"),   Unicode::emptyString);
	}
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::updateDetails()
{
	clearDetails();

	bool result = CuiMissionManager::getDetailsType();
	if(result)
	{
		if(CuiMissionManager::getMission())
			setDetails(*CuiMissionManager::getMission());
		else
			deactivate();
	}
	else
	{
		deactivate();
	}

	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setSharedDetails(UIPage* page, const ClientMissionObject & details)
{
	showDetailsPage(page);

	UIPage * namePage          = page;
	if (!namePage)
	{
		DEBUG_WARNING(true, ("Can't get namePage for details page"));
		return; 
	}
	namePage->SetVisible (page != NULL);
	namePage->SetScrollLocation(UIPoint(0,0));

	uint32 crc = details.getTargetAppearanceCrc();
	ConstCharCrcString ccs = ObjectTemplateList::lookUp(crc);
	if(m_target)
		delete m_target;
	m_target = safe_cast<ClientObject*>(ObjectTemplateList::createObject(ccs));
	if(m_target)
		m_target->endBaselines ();

	m_textDescription->SetText(details.getDescription().localize());
	m_textTitle->SetText(details.getTitle().localize());
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addCreatorToDetails(UIPage* namePage, const ClientMissionObject & details) const
{
	Unicode::String creatorName = details.getMissionCreator();

	if (creatorName.empty ())
	{
		Unicode::String creatorStr = CuiStringIdsMission::dynamic_mission.localize();
		namePage->SetProperty (UILowerString ("creator.text"),           creatorStr);
	}
	else
	{
		namePage->SetProperty (UILowerString ("creator.text"),           creatorName);
	}
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addRewardToDetails(UIPage* namePage, const ClientMissionObject & details) const
{
	Unicode::String moneyStr;
	CuiMissionManager::formatMoney (moneyStr, details.getReward());

	int const reward = details.getReward();

	if(reward == 0)
	{
		moneyStr = CuiStringIdsMission::unknown_reward.localize();
	}

	namePage->SetProperty (UILowerString ("reward.visible"),        SwgCuiMissionDetailsNamespace::Properties::str_true);
	namePage->SetProperty (UILowerString ("reward.text"),           moneyStr);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addTargetToDetails(UIPage* namePage, const ClientMissionObject & details, bool pitch)
{
	UNREF(pitch);
	Unicode::String tmp = Unicode::narrowToWide(details.getTargetName());

	if(m_target)
	{
		if (m_viewer)
		{
			m_viewer->setCameraForceTarget   (true);
			m_viewer->setObject              (m_target);
			m_viewer->recomputeZoom ();
			m_viewer->setCameraForceTarget   (false);
		}
	}
	else
		m_viewer->setObject (0);
	namePage->SetProperty (UILowerString ("target.text"), tmp);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addStartLocationToDetails(UIPage* namePage) const
{
	Unicode::String tmp;
	if (namePage)
	{
		CuiMissionManager::formatLocationString (tmp, m_missionObjectDetails->getStartLocation());
		namePage->SetProperty (UILowerString ("location.text"), tmp);
	}
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addDestLocationToDetails(UIPage* namePage) const
{
	Unicode::String tmp;
	if (namePage)
	{
		CuiMissionManager::formatLocationString (tmp, m_missionObjectDetails->getEndLocation());
		namePage->SetProperty (UILowerString ("destlocation.text"), tmp);
	}
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::addDifficultyToDetails(UIPage* namePage, const ClientMissionObject & details) const
{
	Unicode::String difficultyStr;
	CuiMissionManager::formatDifficulty (difficultyStr, details.getDifficulty());
	
	namePage->SetProperty (UILowerString ("difficulty.visible"),        SwgCuiMissionDetailsNamespace::Properties::str_true);
	namePage->SetProperty (UILowerString ("difficulty.text"),           difficultyStr);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setDestroyDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageDestroy;
	setSharedDetails(namePage, details);
	addTargetToDetails(namePage, details, false);
	addStartLocationToDetails(namePage);
	addCreatorToDetails(namePage, details);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setDeliverDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageDeliver;
	setSharedDetails(namePage, details);
	addTargetToDetails(namePage, details, true);
	addStartLocationToDetails(namePage);
	addDestLocationToDetails(namePage);
	addCreatorToDetails(namePage, details);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setBountyDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageBounty;
	setSharedDetails(namePage, details);
	addTargetToDetails(namePage, details, false);

	//set the location as unknown
	namePage->SetProperty (UILowerString ("location.text"), CuiStringIdsMission::unknown_planet.localize());

	addCreatorToDetails(namePage, details);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setSurveyDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageSurvey;
	setSharedDetails(namePage, details);
	addCreatorToDetails(namePage, details);

	//efficiency field
	Unicode::String tmp;
	IGNORE_RETURN (UIUtils::FormatLong (tmp, details.getDifficulty()));
	namePage->SetProperty (UILowerString ("efficiency.text"), tmp);

	addTargetToDetails(namePage, details, false);
	addRewardToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setEntertainerDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageEntertainer;
	setSharedDetails(namePage, details);
	addStartLocationToDetails(namePage);
	addCreatorToDetails(namePage, details);
	addTargetToDetails(namePage, details, false);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setCraftingDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageCrafting;
	setSharedDetails(namePage, details);
	addStartLocationToDetails(namePage);
	addDestLocationToDetails(namePage);
	addCreatorToDetails(namePage, details);
	addTargetToDetails(namePage, details, false);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setHuntingDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageHunting;
	setSharedDetails(namePage, details);
	addCreatorToDetails(namePage, details);
	addTargetToDetails(namePage, details, false);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setAssassinDetails (const ClientMissionObject & details)
{
	UIPage * namePage = m_pageAssassin;
	setSharedDetails(namePage, details);
	addTargetToDetails(namePage, details, false);
	addStartLocationToDetails(namePage);
	addCreatorToDetails(namePage, details);
	addRewardToDetails(namePage, details);
	addDifficultyToDetails(namePage, details);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::showDetailsPage(UIPage* page)
{
	//first hide all the pages
	m_pageDeliver->SetVisible (false);
	m_pageDestroy->SetVisible (false);
	m_pageBounty->SetVisible  (false);
	m_pageSurvey->SetVisible  (false);
	m_pageEntertainer->SetVisible  (false);
	m_pageCrafting->SetVisible  (false);
	m_pageHunting->SetVisible  (false);
	m_pageAssassin->SetVisible  (false);

	//set the one we want (which should be one of the above, or NULL) visible
	if(page)
		page->SetVisible(true);
}

//-------------------------------------------------------------------------

void SwgCuiMissionDetails::setDetails (const ClientMissionObject & details)
{
	SwgCuiMissionDetailsNamespace::s_detailsType = SwgCuiMissionDetailsNamespace::DT_ClientMissionObject;

	Container* c = ContainerInterface::getContainer(const_cast<ClientMissionObject & >(details));
	if(!c)
	{
		DEBUG_WARNING(true, ("ClientMissionObject has no container in SwgCuiMissionDetails::setDetails"));
		deactivate();
		return;
	}

	m_missionObjectDetails = &details;

//	bool foundClientMissionListEntryObject = false;
	for(ContainerIterator i = c->begin(); i != c->end(); ++i)
	{
		Container::ContainedItem item = *i;
		Object* o = item.getObject();
		ClientWaypointObject* w = dynamic_cast<ClientWaypointObject*>(o);
		if(w && w->getColor() != Waypoint::getColorNameById(Waypoint::Invisible))
		{
			m_waypoint = w;
			if(m_waypoint->isWaypointActive())
			{
				m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_deactivate.localize());
			}
			else
			{
				m_buttonWaypoint->SetText(CuiStringIdsMission::waypoint_activate.localize());
			}
		}
	}

	if(details.getType() == SwgCuiMissionDetailsNamespace::MissionTypes::destroy || details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::recon)
		setDestroyDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::deliver || details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::escorttocreator || details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::escort)
		setDeliverDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::bounty)
		setBountyDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::survey)
		setSurveyDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::dancer || details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::musician)
		setEntertainerDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::crafting)
		setCraftingDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::hunting)
		setHuntingDetails(details);
	else if(details.getType() ==  SwgCuiMissionDetailsNamespace::MissionTypes::assassin)
		setAssassinDetails(details);
	else
	{
		WARNING_STRICT_FATAL(true, ("Bad mission details type %i:", details.getType()));
	}
	const ContainedByProperty* containedBy = ContainerInterface::getContainedByProperty(details);

	//if the object's in the datapad, setup buttons
	if(containedBy->getContainedBy() == CuiInventoryManager::getPlayerDatapad ())
	{
		m_buttonAccept->SetVisible  (false);
		m_buttonAbort->SetVisible   (true);
		m_buttonWaypoint->SetVisible(true);
	}
	else
	{
		m_buttonAccept->SetVisible  (true);
		m_buttonAbort->SetVisible   (false);
		m_buttonWaypoint->SetVisible(false);
	}

	if(!getPage().IsEnabled())
		getPage().SetEnabled(true);
}

//-----------------------------------------------------------------------

SwgCuiMissionDetails* SwgCuiMissionDetails::createInto (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/Mission.details"));
	return new SwgCuiMissionDetails (*dupe);
}

//======================================================================

