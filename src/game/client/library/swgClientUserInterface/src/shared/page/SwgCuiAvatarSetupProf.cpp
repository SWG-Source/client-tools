//======================================================================
//
// SwgCuiAvatarSetupProf.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarSetupProf.h"

#include "UIButton.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModel.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerCreationManagerClient.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiAnimationManager.h"
#include "clientUserInterface/CuiAvatarCreationEmotes.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiTurntableAdapter.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Container.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"


//======================================================================

namespace
{
	const std::string s_emotegroup_accept = "accept";
	const std::string s_emotegroup_reject = "reject";

	namespace Properties
	{
		const UILowerString attrib_localdesc = UILowerString ("localdesc");
	}
}

//----------------------------------------------------------------------

SwgCuiAvatarSetupProf::SwgCuiAvatarSetupProf (UIPage &page ) :
CuiMediator             ("SwgCuiAvatarSetupProf", page),
UIEventCallback         (),
m_buttonBack            (0),
m_buttonNext            (0),
m_combo                 (0),
m_pageDescription       (0),
m_textDescription       (0),
m_pageSkills            (0),
m_textSkills            (0),
m_listSkills            (0),
m_pageAttributes        (0),
m_textAttributes        (0),
m_tableAttributes       (0),
m_viewer                (0),
m_selectedIndex         (0),
m_creatures             (new CreatureVector),
m_turntable             (0),
m_lastSelectedAttribute (0),
m_transitionFinished    (false),
m_begunWookieeTransition(false),
m_callback(new MessageDispatch::Callback),
m_pendingCreationFinished(false),
m_aborted(false)
{
	getCodeDataObject (TUIButton,  m_buttonBack,      "buttonBack");
	getCodeDataObject (TUIButton,  m_buttonNext,      "buttonNext");

	getCodeDataObject (TUIComboBox,  m_combo,         "combo");

	getCodeDataObject (TUIPage,  m_pageDescription,   "pageDescription");
	getCodeDataObject (TUIText,  m_textDescription,    "textDescription");
	getCodeDataObject (TUIPage,  m_pageSkills,        "pageSkills");
	getCodeDataObject (TUIText,  m_textSkills,        "textSkills");
	getCodeDataObject (TUIList,  m_listSkills,        "listSkills");
	getCodeDataObject (TUIPage,  m_pageAttributes,    "pageAttributes");
	getCodeDataObject (TUIText,  m_textAttributes,    "textAttributes");
	getCodeDataObject (TUITable, m_tableAttributes,   "tableAttributes");

	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget,  widget,        "viewer");
		
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
		m_viewer->setCameraLodBias(10.0f);
		m_viewer->setCameraLodBiasOverride(true);
		m_viewer->setCameraPitch(0.3f);
		m_viewer->setAutoZoomOutOnly(true);
	}

	m_turntable = new CuiTurntableAdapter (*m_viewer);

	m_textAttributes->SetPreLocalized  (true);
	m_textSkills->SetPreLocalized      (true);
	m_textDescription->SetPreLocalized (true);

	registerMediatorObject (*m_buttonBack,         true);
	registerMediatorObject (*m_buttonNext,         true);
	registerMediatorObject (*m_combo,              true);
	registerMediatorObject (*m_pageDescription,    true);
	registerMediatorObject (*m_textDescription,    true);
	registerMediatorObject (*m_pageSkills,         true);
	registerMediatorObject (*m_listSkills,         true);
	registerMediatorObject (*m_pageAttributes,     true);
	registerMediatorObject (*m_tableAttributes,    true);
	registerMediatorObject (*m_viewer,             true);
	registerMediatorObject (getPage (),            true);
}

//-----------------------------------------------------------------

SwgCuiAvatarSetupProf::~SwgCuiAvatarSetupProf ()
{
	delete m_turntable;
	m_turntable = 0;

	delete m_creatures;
	m_creatures = 0;

	m_buttonBack     = 0;
	m_buttonNext     = 0;
	
	m_combo = 0;
	m_pageDescription = 0;
	m_textDescription = 0;
	m_pageSkills = 0;
	m_textSkills = 0;
	m_listSkills = 0;
	m_pageAttributes = 0;
	m_textAttributes = 0;
	m_tableAttributes = 0;

	m_viewer = 0;

	delete m_callback;
	m_callback = 0;
}

//-----------------------------------------------------------------

void SwgCuiAvatarSetupProf::performActivate ()
{
	m_callback->connect(*this, &SwgCuiAvatarSetupProf::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->connect(*this, &SwgCuiAvatarSetupProf::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));

	m_pendingCreationFinished = false;
	m_aborted = false;

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);
	
	const std::string & last_profession = SwgCuiAvatarCreationHelper::getProfession ();
	
	CreatureObject * current = SwgCuiAvatarCreationHelper::getCreature ();

#if 1
	//-- hack for testing
	if (!current)
	{
		current = dynamic_cast<CreatureObject *>(ObjectTemplate::createObject ("object/creature/player/shared_human_male.iff"));
		NOT_NULL (current);
		SwgCuiAvatarCreationHelper::setCreature (*current);
	}
#endif
	
	if (current)  //lint !e774 //testing hack
	{
		UIDataSource * comboDataSource = m_combo->GetDataSource ();
		
		if (!comboDataSource)
		{
			m_combo->SetDataSource (new UIDataSource);
			comboDataSource = NON_NULL (m_combo->GetDataSource ());
		}
		
		comboDataSource->Clear ();

		PlayerCreationManager::StringVector sv;

		PlayerCreationManager::getProfessionVector (sv, "");
		
		WARNING (sv.empty (), ("No professions available"));

		int numProfessions = 0;
		if (CuiLoginManager::canCreateRegularCharacter())
			numProfessions += sv.size() - 1;
		
		m_creatures->reserve(numProfessions);
		
		int indexSelection = 0;
		
		SwgCuiAvatarCreationHelper::CreatureVector cv;
		SwgCuiAvatarCreationHelper::getCreaturesFromPool (cv, static_cast<int>(sv.size ()));
		
		if (sv.size () == cv.size ())
		{
			int index = 0;
			for (PlayerCreationManager::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it, ++index)
			{
				const std::string & professionName = *it;
				
				if (professionName == "jedi")
				{
					--index;       // prevent index from incrementing on loop
					continue;
				}
				
				if (last_profession == professionName)
					indexSelection = index;
				
				const SkillObject * profession_skill = SkillManager::getInstance ().getSkill (professionName);
				
				if (profession_skill)
				{
					UIData * const data = new UIData;
					data->SetName (professionName);
					Unicode::String localName;
					CuiSkillManager::localizeSkillName (professionName, localName);
					data->SetProperty (UIList::DataProperties::LOCALTEXT, localName);
					comboDataSource->AddChild (data);
					
					CreatureObject * const duplicateCreature = cv [static_cast<size_t>(index)];
					if (duplicateCreature)
					{
						PlayerCreationManagerClient::setupPlayer (*duplicateCreature, professionName);
						m_viewer->addObject(*duplicateCreature);
					}
					
					CreatureObjectWatcher toPush(duplicateCreature);
					m_creatures->push_back(toPush);
				} //lint !e429 // data not a leak
				else
					WARNING (true, ("Unable to find profession: [%s] for profession selection", professionName.c_str ()));
				
				m_combo->SetSelectedIndex(indexSelection);
			}
		}
		else
			DEBUG_FATAL (true, ("wtf"));
	}

	m_turntable->arrange ();
	m_turntable->turnTo (-1, false);
	
	updateProfessionInfo (false);

	m_listSkills->SelectRow      (0);
	m_tableAttributes->SelectRow (-1);

	const UITableModelDefault * const model = dynamic_cast<UITableModelDefault *>(m_tableAttributes->GetTableModel ());
		
	if (model)
	{
		m_tableAttributes->SelectRow (model->GetVisualDataRowIndex (m_lastSelectedAttribute));
		OnGenericSelectionChanged (m_tableAttributes);
	}

	m_transitionFinished = false;
	m_viewer->SetVisible (false);

	m_begunWookieeTransition = false;

	setIsUpdating (true);
}

//-----------------------------------------------------------------

void SwgCuiAvatarSetupProf::performDeactivate ()
{
	m_callback->disconnect(*this, &SwgCuiAvatarSetupProf::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->disconnect(*this, &SwgCuiAvatarSetupProf::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));

	setIsUpdating (false);

	setPointerInputActive  (false);
	setKeyboardInputActive (false);
	setInputToggleActive   (true);	

	for (CreatureVector::iterator it = m_creatures->begin (); it != m_creatures->end (); ++it)
	{
		CreatureObject * const creature = *it;
		if (creature != 0)
		{
			m_viewer->removeObject(*creature);
		}
	}

	m_creatures->clear ();

	m_viewer->setPaused (true);
}

//-----------------------------------------------------------------

void SwgCuiAvatarSetupProf::OnButtonPressed   (UIWidget *context)
{
	UNREF (context);
	
	NOT_NULL (context);
	
	if (context == m_buttonBack)
	{
		returnToPreviousScreen(false);
	}
	else if (context == m_buttonNext)
	{
		finishAndCreateCharacter();
	}
}

//----------------------------------------------------------------------

bool SwgCuiAvatarSetupProf::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (context == m_viewer)
	{	
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			const ClientObject * obj = m_viewer->getObjectAt (msg.MouseCoords);
			if (obj)
			{
				const CreatureVector::iterator fit = std::find (m_creatures->begin (), m_creatures->end (), obj);
				if (fit != m_creatures->end ())
				{
					const int index = std::distance (m_creatures->begin (), fit);
					m_combo->SetSelectedIndex (index);
				}
				return false;
			}
		}
	}
	else if (context == &getPage ())
	{
		if (msg.Type == UIMessage::KeyDown)
		{
			if (msg.Keystroke == UIMessage::RightArrow)
			{
				const int itemCount = m_combo->GetItemCount ();
				int index = m_combo->GetSelectedIndex ();
				--index;

				if (index < 0)
					index = itemCount - 1;
	
				m_combo->SetSelectedIndex (index);
				return false;
			}
			else if (msg.Keystroke == UIMessage::LeftArrow)
			{
				const int itemCount = m_combo->GetItemCount ();
				int index = m_combo->GetSelectedIndex ();
				++index;

				if (index >= itemCount)
					index = 0;
	
				m_combo->SetSelectedIndex (index);
				return false;
			}
		}
	}
	
	return true;
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::OnShow   (UIWidget *context)
{
	if (context == m_pageDescription)
	{
		updateProfessionInfo (false);
	}
	else if (context == m_pageSkills)
	{
		OnGenericSelectionChanged (m_listSkills);
	}
	else if (context == m_pageAttributes)
	{
		OnGenericSelectionChanged (m_tableAttributes);
	}
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_combo)
	{
		updateProfessionInfo (true);
	}
	else if (context == m_listSkills)
	{
		const UIData * const data = m_listSkills->GetDataAtRow (m_listSkills->GetLastSelectedRow ());
		if (data)
		{
			Unicode::String localDesc;
			CuiSkillManager::localizeSkillDescription (data->GetName (), localDesc);
			m_textSkills->SetLocalText (localDesc);
		}
	}
	else if (context == m_tableAttributes)
	{
		const UITableModelDefault * const model = dynamic_cast<UITableModelDefault *>(m_tableAttributes->GetTableModel ());

		if (model)
		{
			const long rowVis = m_tableAttributes->GetLastSelectedRow ();
			m_lastSelectedAttribute = model->GetLogicalDataRowIndex (rowVis);
			const UIData * const data = model->GetCellDataVisual (rowVis, 0);

			if (data)
			{
				UIString localDesc;
				data->GetProperty (Properties::attrib_localdesc, localDesc);
				m_textAttributes->SetLocalText (localDesc);
			}
			else
				m_textAttributes->SetLocalText (Unicode::String ());
		}
	}
} //lint !e818  //stfu noob

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::updateProfessionInfo (bool playSound)
{
	int oldSelectedIndex = m_selectedIndex;

	m_selectedIndex = std::max (0L, m_combo->GetSelectedIndex ());
	const UIData * const data    = m_combo->GetDataAtIndex (m_selectedIndex);
	const UIData * const oldData = m_combo->GetDataAtIndex (oldSelectedIndex);

	const std::string profession    = data    ? data->GetName ()    : std::string ();
	const std::string oldProfession = oldData ? oldData->GetName () : std::string ();

	SwgCuiAvatarCreationHelper::setProfession (profession);

	if (static_cast<int>(m_creatures->size ()) <= m_selectedIndex ||
		static_cast<int>(m_creatures->size ()) <= oldSelectedIndex)
	{
		return;
	}
	
	CreatureObject * const creature = (*m_creatures) [static_cast<size_t>(m_selectedIndex)];

	if (!creature)
	{
		WARNING (true, ("no creature"));
		return;
	}
	else
	{
		SwgCuiAvatarCreationHelper::setCreature (*creature);
		m_turntable->turnTo                     (m_selectedIndex, playSound);
		
		std::string emote;
		if (CuiAvatarCreationEmotes::findRandomEmote (s_emotegroup_accept, profession, emote))
			CuiAnimationManager::attemptPlayEmote (*creature, 0, emote);
	}
	
	if (oldSelectedIndex >= 0 &&
		oldSelectedIndex != m_selectedIndex &&
		oldSelectedIndex < static_cast<int>(m_creatures->size ()))
	{
		CreatureObject * const oldCreature = (*m_creatures) [static_cast<size_t>(oldSelectedIndex)];
		
		if (oldCreature)
		{
			std::string emote;
			if (CuiAvatarCreationEmotes::findRandomEmote (s_emotegroup_reject, oldProfession, emote))
				CuiAnimationManager::attemptPlayEmote (*oldCreature, 0, emote);
		}
	}
	
	Unicode::String localName;
	CuiSkillManager::localizeSkillDescription (SwgCuiAvatarCreationHelper::getProfession (), localName);
	m_textDescription->SetLocalText (localName);
	
	//----------------------------------------------------------------------
	//-- update skills
	{
		UIDataSource * dataSource = m_listSkills->GetDataSource ();
		if (!dataSource)
		{
			m_listSkills->SetDataSource (new UIDataSource);
			dataSource = NON_NULL (m_listSkills->GetDataSource ());
		}
		
		dataSource->Clear ();
		
		const CreatureObject::SkillList & skills = creature->getSkills ();

		for (CreatureObject::SkillList::const_iterator it = skills.begin (); it != skills.end (); ++it)
		{
			const SkillObject * const skill = *it;
			if (skill)
			{
				UIData * const skillData = new UIData;

				Unicode::String localSkillName;
				CuiSkillManager::localizeSkillName (*skill, localSkillName);

				skillData->SetName (skill->getSkillName ());
				skillData->SetProperty (UIList::DataProperties::LOCALTEXT, localSkillName);
				dataSource->AddChild (skillData);
			}
		} //lint !e429 //skillData not a leak
		
		m_listSkills->SelectRow (0);
	}
	
	//----------------------------------------------------------------------
	//-- update attributes
	{

//@todo: figure out why this hangs release build
//		m_tableAttributes->SelectRow (0);

		UITableModelDefault * const model = dynamic_cast<UITableModelDefault *>(m_tableAttributes->GetTableModel ());
		
		if (model)
		{
			UITable::FloatVector proportions;
			m_tableAttributes->GetColumnWidthProportions (proportions);

			m_tableAttributes->SetTableModel (0);

			const CreatureObject * c = SwgCuiAvatarCreationHelper::getCreature ();
			const std::string templateName = c->getObjectTemplateName();

			for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
			{
				Attributes::Value val = creature->getAttribute (i);
				UIWidget * displayWidget = 0;
				model->GetValueAtWidget (i, 2, displayWidget);
				
				UIData * const cellData = model->GetCellDataLogical (i, 1);

				if (cellData)
				{
					Unicode::String str;
					UIUtils::FormatLong (str, val);
					cellData->SetProperty (UITableModelDefault::DataProperties::Value, str);
				}
				if (displayWidget)
				{

					UIString istr;
					UIUtils::FormatInteger (istr, val);
					displayWidget->SetProperty (UITableModel::DataProperties::WidgetValue, istr);

					UIWidget * const valueWidget = dynamic_cast<UIWidget *>(displayWidget->GetChild ("value"));
					if (valueWidget)
					{
						valueWidget->SetWidth    (displayWidget->GetWidth () * val / 1000);
					}
				}
			}

			m_tableAttributes->SetTableModel (model);
			m_tableAttributes->RestoreColumnWidthProportions (proportions);

		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (!m_transitionFinished)
	{
		bool finished = true;

		float radius = 0.0f;

		for (CreatureVector::const_iterator it = m_creatures->begin (); it != m_creatures->end (); ++it)
		{
			CreatureObject * const obj = *it;
			if (obj == 0)
			{
				continue;			
			}
			
			finished = SwgCuiAvatarCreationHelper::areAllDetailLevelsAvailable (*obj) && finished;
			if (finished)
			{
				const float myRadius = obj->getAppearanceSphereRadius ();
				radius   = std::max (radius, myRadius);
			}
		}
		
		if (finished)
		{
			m_turntable->arrange ();
			m_turntable->turnTo (-1, false);
			
			m_viewer->setCameraLookAt (Vector (0.0f, radius, 0.0f));
			
			m_viewer->setPaused    (false);
			m_viewer->SetVisible   (true);
						
			const UITableModelDefault * const model = dynamic_cast<UITableModelDefault *>(m_tableAttributes->GetTableModel ());
			
			if (model)
			{
				m_tableAttributes->SelectRow (model->GetVisualDataRowIndex (m_lastSelectedAttribute));
				OnGenericSelectionChanged    (m_tableAttributes);
			}
			
			m_viewer->setViewDirty             (true);
			m_viewer->setCameraForceTarget     (true);
			m_viewer->recomputeZoom            ();
			m_viewer->setCameraForceTarget     (false);			
			
			m_transitionFinished = true;
			CuiTransition::signalTransitionReady(CuiMediatorTypes::AvatarSetupProf);
		}		
	}

	CreatureObject * current = SwgCuiAvatarCreationHelper::getCreature();
	if(!m_begunWookieeTransition && current && current->getSpecies() == SharedCreatureObjectTemplate::SP_wookiee)
	{
		m_begunWookieeTransition = true;
		finishAndCreateCharacter();
	}

	m_turntable->update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::onCreationAborted(bool)
{
	m_aborted = true;

	if (m_pendingCreationFinished)
	{
		WARNING (true,("Got abort message in the same frame as pending creation finished message ."));
	}
	else
	{
		GameNetwork::setAcceptSceneCommand(false);
		GameNetwork::disconnectConnectionServer();
		CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);
		SwgCuiAvatarCreationHelper::requestRandomName(false);
		SwgCuiAvatarCreationHelper::purgePool();
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::onCreationFinished(bool b)
{
	if (b)
	{
		if (m_aborted)
		{
			WARNING (true,("Got creation finished message in the same frame as user abort message."));
		}
		else
		{
			m_pendingCreationFinished = false;

			CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature();

			Unicode::String playerName;

			if (player == 0)		
			{
				if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(playerName))
				{
					WARNING(true, ("No player"));
					return;
				}
			}
			else
			{
				playerName = player->getObjectName();
			}

			CreatureVector::iterator ii = m_creatures->begin();
			CreatureVector::iterator iiEnd = m_creatures->end();

			for (; ii != iiEnd; ++ii)
			{
				CreatureObject * const creature = *ii;
				if (creature != 0)
				{
					m_viewer->removeObject(*creature);
				}
			}

			m_creatures->clear();

			ConfigClientGame::setLauncherAvatarName(std::string());
			ConfigClientGame::setLauncherClusterId(CuiLoginManager::getConnectedClusterId ());
			ConfigClientGame::setAvatarName(Unicode::wideToNarrow (playerName));
			ConfigClientGame::setCentralServerName(CuiLoginManager::getConnectedClusterName ());
			ConfigClientGame::setNextAutoConnectToGameServer(true);

			CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);

			SwgCuiAvatarCreationHelper::purgePool();
			deactivate();
		}
	}
	else
	{
		returnToPreviousScreen(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::finishAndCreateCharacter()
{
	CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature();

	Unicode::String playerName;

	if (!player)		
	{
		if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(playerName))
		{
			WARNING (true, ("No player"));
			return;
		}
	}
	else
	{
		playerName = player->getObjectName();
	}

	SwgCuiAvatarCreationHelper::purgeExtraPoolMembers();
	SwgCuiAvatarCreationHelper::setCreatureCustomized(true);

	if (SwgCuiAvatarCreationHelper::finishCreation())
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSetupProf::returnToPreviousScreen(bool const dueToError)
{
	for (CreatureVector::iterator it = m_creatures->begin (); it != m_creatures->end (); ++it)
	{
		CreatureObject * const creature = *it;
		if (creature != 0)
		{
			m_viewer->removeObject(*creature);
		}
	}
	
	m_creatures->clear();

	std::string const whereTo((dueToError) ? CuiMediatorTypes::AvatarSimple : CuiMediatorTypes::AvatarCustomize);

	CuiTransition::startTransition(CuiMediatorTypes::AvatarSetupProf, whereTo);
}

//======================================================================
