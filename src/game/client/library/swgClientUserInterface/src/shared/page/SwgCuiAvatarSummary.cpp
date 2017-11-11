//======================================================================
//
// SwgCuiAvatarSummary.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarSummary.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiCreatureSkillsList.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//======================================================================

namespace
{
	namespace Properties
	{
		const UILowerString OneNameTemplates = UILowerString ("OneNameTemplates");
	}

	int cutString (const Unicode::String & str, Unicode::String & one, Unicode::String & two)
	{
		size_t endpos = 0;
		if (Unicode::getFirstToken (str, 0, endpos, one))
		{
			if (endpos != Unicode::String::npos && Unicode::getFirstToken (str, endpos + 1, endpos, two)) //lint !e650 !e737 //stupid msvc
				return 2;

			return 1;
		}

		return 0;
	}

	const std::string s_sharedTemplatePrefix = "object/creature/player/shared_";
	const std::string s_sharedTemplateSuffix = ".iff";
}

//lint -esym(534, __ns_D__projects_swg_current_src_game_client_library_swgClientUserInterface_project_lnt::cutString)

//----------------------------------------------------------------------


SwgCuiAvatarSummary::SwgCuiAvatarSummary (UIPage & page) :
CuiMediator               ("SwgCuiAvatarSummary", page),
UIEventCallback           (),
m_buttonBack              (0),
m_buttonNext              (0),
m_buttonRandomName        (0),
m_textboxName             (0),
m_textboxSurname          (0),
m_textBio                 (0),
m_viewer                  (0),
m_skills                  (0),
m_callback                (new MessageDispatch::Callback),
m_pendingCreationFinished (false),
m_nameModified            (false),
m_oneNameTemplateVector   (new StringVector),
m_aborted                 (false),
m_tabs                    (0),
m_checkNewbieTutorial     (0)
{

	//----------------------------------------------------------------------
	//-- find the template names that only have one name (wookiees)

	const UIData * const codeData = NON_NULL (getCodeData ());

	if (codeData)
	{
		std::string s;
		if (codeData->GetPropertyNarrow (Properties::OneNameTemplates, s))
		{
			size_t endpos = 0;
			std::string token;
			while (Unicode::getFirstToken (s, endpos, endpos, token))
			{
				m_oneNameTemplateVector->push_back (s_sharedTemplatePrefix + token + s_sharedTemplateSuffix);

				if (endpos == Unicode::String::npos) //lint !e650 !e737 //stupid msvc
					break;

				++endpos;
			}
		}
	}

	std::sort (m_oneNameTemplateVector->begin (), m_oneNameTemplateVector->end ());
	
	getCodeDataObject (TUIButton,     m_buttonBack,          "buttonBack");
	getCodeDataObject (TUIButton,     m_buttonNext,          "buttonNext");
	getCodeDataObject (TUIButton,     m_buttonRandomName,    "buttonRandomName");

	getCodeDataObject (TUITextbox,    m_textboxName,         "textboxName");
	getCodeDataObject (TUITextbox,    m_textboxSurname,      "textboxSurname");

	getCodeDataObject (TUIText,       m_textBio,              "textBio");

	getCodeDataObject (TUITabbedPane, m_tabs,                 "tabs");
	getCodeDataObject (TUICheckbox,   m_checkNewbieTutorial,  "checkNewbieTutorial");

	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget,  widget,        "viewer");
		
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
		m_viewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
	}

	{
		UIPage * skillsPage = 0;
		getCodeDataObject (TUIPage,  skillsPage,        "pageskills");
		
		m_skills = new CuiCreatureSkillsList (*skillsPage);
		m_skills->fetch ();
	}

	registerMediatorObject (*m_checkNewbieTutorial, false);

	registerMediatorObject (*m_buttonBack,          true);
	registerMediatorObject (*m_buttonNext,          true);
	registerMediatorObject (*m_buttonRandomName,    true);
	registerMediatorObject (*m_textboxName,         true);
	registerMediatorObject (*m_textboxSurname,      true);

	m_tabs->SetActiveTab (-1);
	m_tabs->SetActiveTab (0);
}

//----------------------------------------------------------------------

SwgCuiAvatarSummary::~SwgCuiAvatarSummary ()
{

	delete m_oneNameTemplateVector;
	m_oneNameTemplateVector = 0;

	m_buttonBack       = 0;
	m_buttonNext       = 0;
	m_buttonRandomName = 0;
	m_textboxName      = 0;
	m_textboxSurname   = 0;
	m_textBio          = 0;
	m_viewer           = 0;

	m_skills->release ();
	m_skills           = 0; 

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::performActivate   ()
{
	m_aborted                 = false;
	m_pendingCreationFinished = false;
	m_nameModified            = false;

	m_textBio->SetLocalText (SwgCuiAvatarCreationHelper::getBiography ());
	m_checkNewbieTutorial->SetChecked (CuiPreferences::getUseNewbieTutorial ());

	m_skills->activate     ();

	//-- activation forces pages visible, revert that
	m_skills->getPage ().SetVisible     (false);

	//-- refresh the active tab
	const int activeTab = m_tabs->GetActiveTab ();
	m_tabs->SetActiveTab (-1);
	m_tabs->SetActiveTab (activeTab);

	CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature        ();

	DEBUG_FATAL (!player, ("SwgCuiAvatarSummary activated but there is no player creature."));

	if (player)
	{
		m_skills->update      (*player);
		m_viewer->addObject   (*player);
		player->resetRotateTranslate_o2p ();
		
		if (Game::getSinglePlayer ())
			player->setObjectName    (Unicode::narrowToWide (ConfigClientGame::getPlayerName ()));
		else
		{
			const Unicode::String & randomName = SwgCuiAvatarCreationHelper::getRandomName ();
			player->setObjectName    (randomName);		//-- default name
		}

		if (std::binary_search (m_oneNameTemplateVector->begin (), m_oneNameTemplateVector->end (), std::string (player->getObjectTemplateName ())))
			m_textboxSurname->SetVisible (false);
		else
			m_textboxSurname->SetVisible (true);

		Unicode::String name;
		Unicode::String surname;

		cutString (player->getObjectName (), name, surname);

		m_textboxName->SetLocalText    (name);
		m_textboxSurname->SetLocalText (surname);
	}

	m_viewer->setPaused                (false);
	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	m_textboxName->SetFocus ();

	m_callback->connect (*this, &SwgCuiAvatarSummary::onCreationAborted,          static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>               (0));
	m_callback->connect (*this, &SwgCuiAvatarSummary::onCreationFinished,         static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>     (0));
	m_callback->connect (*this, &SwgCuiAvatarSummary::onRandomNameChanged,        static_cast<SwgCuiAvatarCreationHelper::Messages::RandomNameChanged *>    (0));

	setIsUpdating (true);

	CuiTransition::signalTransitionReady (CuiMediatorTypes::AvatarSummary);

	m_checkNewbieTutorial->SetVisible (true);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::performDeactivate ()
{
	setIsUpdating (false);

	m_callback->disconnect (*this, &SwgCuiAvatarSummary::onCreationAborted,          static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>     (0));
	m_callback->disconnect (*this, &SwgCuiAvatarSummary::onCreationFinished,         static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>     (0));
	m_callback->disconnect (*this, &SwgCuiAvatarSummary::onRandomNameChanged,        static_cast<SwgCuiAvatarCreationHelper::Messages::RandomNameChanged *>    (0));

	m_skills->deactivate ();

	m_viewer->clearObjects ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::OnButtonPressed   (UIWidget *context)
{
	CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature ();
	
	const bool warnBadName = (context == m_buttonNext);

	if (player)
	{
		Unicode::String name = Unicode::getTrim (m_textboxName->GetLocalText ());
		
		if(name.empty())
		{
			if (warnBadName)
			{
				CuiMessageBox::createInfoBox(CuiStringIds::avatar_err_name_none.localize());
				return;
			}
		}

		{
			const size_t spacepos = name.find (' ');
			if (spacepos != Unicode::String::npos)  //lint !e650 !e737 //stupid msvc
			{
				if (warnBadName)
				{
					CuiMessageBox::createInfoBox (CuiStringIds::avatar_err_name_space.localize ());
					return;
				}
				else
					name = name.substr (0, spacepos);
			}
		}
		
		Unicode::String surname = Unicode::getTrim (m_textboxSurname->GetLocalText ());

		if (!surname.empty ())
		{
			const size_t spacepos = surname.find (' ');
			if (spacepos != Unicode::String::npos) //lint !e650 !e737 //stupid msvc
			{
				if (warnBadName)
				{
					CuiMessageBox::createInfoBox (CuiStringIds::avatar_err_surname_space.localize ());
					return;
				}
				else
					surname = surname.substr (0, spacepos);
			}
			name.push_back (' ');
			name += surname;
		}

		player->setObjectName (name);

		SwgCuiAvatarCreationHelper::setBiography (m_textBio->GetLocalText ());
		CuiPreferences::setUseNewbieTutorial (m_checkNewbieTutorial->IsChecked ());
	}

	if (context == m_buttonBack)
	{
		CreatureObject * current = SwgCuiAvatarCreationHelper::getCreature();
		if(current && current->getSpecies() == SharedCreatureObjectTemplate::SP_wookiee)
			CuiTransition::startTransition (CuiMediatorTypes::AvatarSummary, CuiMediatorTypes::AvatarCustomize);
		else
			CuiTransition::startTransition (CuiMediatorTypes::AvatarSummary, CuiMediatorTypes::AvatarSetupProf);
	}
	else if (context == m_buttonNext)
	{	
		const bool retval = SwgCuiAvatarCreationHelper::finishCreation();

		if (retval)
		{
			deactivate ();
		}
		// login
	}
	else if (context == m_buttonRandomName)
	{
		m_nameModified = false;
		SwgCuiAvatarCreationHelper::requestRandomName        (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::onCreationAborted (bool)
{
	m_aborted = true;
	WARNING (m_pendingCreationFinished, ("Got abort message in the same frame as pending creation finished message ."));
	m_pendingCreationFinished = false;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::onCreationFinished (bool b)
{
	if (b)
	{
		if (m_aborted)
			WARNING (true, ("Got creation finished message in the same frame as user abort message."));
		else
			m_pendingCreationFinished = true;
	}	
}

//----------------------------------------------------------------------

void SwgCuiAvatarSummary::onRandomNameChanged  (const Unicode::String & name)
{
	if (m_nameModified)
		return;

	CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature ();
	if (player)
		player->setObjectName (name);

	Unicode::String firstname;
	Unicode::String surname;

	cutString (name, firstname, surname);

	m_textboxName->SetLocalText    (firstname);
	m_textboxSurname->SetLocalText (surname);
}

//----------------------------------------------------------------------

void  SwgCuiAvatarSummary::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_aborted)
	{
		GameNetwork::setAcceptSceneCommand      (false);
		GameNetwork::disconnectConnectionServer ();
		CuiMediatorFactory::activate            (CuiMediatorTypes::AvatarSelection);
		deactivate ();
		return;
	}

	if (m_pendingCreationFinished)
	{
		m_pendingCreationFinished = false;

		CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature ();

		Unicode::String playerName;

		if (!player)		
		{
			if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic (playerName))
			{
				WARNING (true, ("No player"));
				return;
			}
		}
		else
			playerName = player->getObjectName ();

		ConfigClientGame::setLauncherAvatarName          (std::string ());
		ConfigClientGame::setLauncherClusterId           (CuiLoginManager::getConnectedClusterId ());
		ConfigClientGame::setAvatarName                  (Unicode::wideToNarrow (playerName));
		ConfigClientGame::setCentralServerName           (CuiLoginManager::getConnectedClusterName ());

		ConfigClientGame::setNextAutoConnectToGameServer (true);

		CuiMediatorFactory::activate (CuiMediatorTypes::AvatarSelection);

		SwgCuiAvatarCreationHelper::purgePool ();
		deactivate ();
	}
}

//======================================================================

