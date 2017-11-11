//======================================================================
//
// SwgCuiRoadmap.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiRoadmap.h"

#include "clientGame/Game.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsResource.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/DraftSchematicGroupManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiRoadmap_Profession.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPie.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"

#include <map>
#include <vector>

namespace SwgCuiRoadmapNamespace
{
	const int NUM_PROFESSIONS = 9;
	std::string const ROADMAP_STRING_TABLE("ui_roadmap");

	const char * s_command_breaker = "+";

	const StringId s_experienceTitleDone(ROADMAP_STRING_TABLE, "experience_title_done");
	const StringId s_experienceTitleInProgress(ROADMAP_STRING_TABLE, "experience_title_in_progress");
	const StringId s_experienceTitleNotYet(ROADMAP_STRING_TABLE, "experience_title_not_yet");

	const StringId s_rewardTitleAchieveStep(ROADMAP_STRING_TABLE, "reward_title_achieve_step");	
	const StringId s_rewardTitleAchievePhase(ROADMAP_STRING_TABLE, "reward_title_achieve_phase");
	
	const std::string s_emptyString;
	const std::string s_phaseString("phase");

	const StringId s_basicsTab(ROADMAP_STRING_TABLE, "tab_basics");
	const StringId s_spaceTab(ROADMAP_STRING_TABLE, "tab_space");
	const StringId s_politicianTab(ROADMAP_STRING_TABLE, "tab_politician");
	const StringId s_storytellerTab(ROADMAP_STRING_TABLE, "tab_storyteller");

	const StringId s_exp(ROADMAP_STRING_TABLE, "exp");

	std::string const s_politicianTemplateName("politician");
	std::string const s_storytellerTemplateName("chronicles");

	const int MAX_NUM_PHASE_ICONS = 4;
	
	const UILowerString s_PalProp  = UILowerString         ("PalColor");
}

using namespace SwgCuiRoadmapNamespace;

//----------------------------------------------------------------------

SwgCuiRoadmap::SwgCuiRoadmap (UIPage & page) :
CuiMediator         ("SwgCuiRoadmap", page),
UIEventCallback     (),
MessageDispatch::Receiver (),
m_professionMediator(0),
m_nextActivityName(0),
m_skillName(),
m_nextActivityNameAurabesh(0),
m_spaceTemplateName(),
m_templateName(),
m_roadmapName(),
m_nextActivityDescription(0),
m_experienceBar(0),
m_rewardsTable(0),
m_iconPathDraftGranted (),
m_stylePhaseComplete(0),
m_stylePhaseIncomplete(0),
m_stylePhaseCompleteSelected(0),
m_stylePhaseIncompleteSelected(0),
m_activityIcon(0),
m_numProfessions(0),
m_professionList(),
m_titleText(0),
m_callback(new MessageDispatch::Callback),
m_itemRewardPage(0),
m_itemRewardText(0),
m_itemRewardViewer(0),
m_itemRewardObject(0),
m_categoryTabs(0),
m_currentTabType(static_cast<TabType>(0)),
m_phaseIcons(0),
m_phaseIconsStartingY(0),
m_phaseButtons(0),
m_phaseNumbers(0),
m_phaseSwooshes(0),
m_showingSpaceTab(false),
m_showingPoliticianTab(false),
m_showingStorytellerTab(false)
{
	m_numProfessions = RoadmapManager::getNumberOfRoadmaps();
	RoadmapManager::getRoadmapList(m_professionList);
	UIPage *mainProfessionPage;
	getCodeDataObject(TUIPage, mainProfessionPage, "Roadmap");
	m_professionMediator = new SwgCuiRoadmap_Profession(*mainProfessionPage, this);		
	m_professionMediator->fetch();
	
	getCodeDataObject(TUIText, m_nextActivityName, "NextActivity");
	getCodeDataObject(TUIText, m_nextActivityNameAurabesh, "NextActivityAurabesh");	
	getCodeDataObject(TUIText, m_nextActivityDescription, "ActivityDescription");
	getCodeDataObject(TUIPage, m_experienceBar, "ExperienceBar");
	getCodeDataObject(TUITable, m_rewardsTable, "RewardTable");
	getCodeDataObject(TUIImage, m_activityIcon, "ActivityIcon");
	getCodeDataObject(TUIText, m_titleText, "TitleText");
	getCodeDataObject(TUIPage, m_itemRewardPage, "itemReward");
	getCodeDataObject(TUIText, m_itemRewardText, "itemRewardText");	
	getCodeDataObject(TUI3DObjectListViewer, m_itemRewardViewer, "itemRewardViewer");
	getCodeDataObject(TUITabbedPane, m_categoryTabs, "categoryTabs");

	m_phaseIcons = new UIImage *[MAX_NUM_PHASE_ICONS];
	m_phaseButtons = new UIButton *[MAX_NUM_PHASE_ICONS];
	m_phaseNumbers = new UIText *[MAX_NUM_PHASE_ICONS];
	m_phaseSwooshes = new UIPie *[MAX_NUM_PHASE_ICONS];
	getCodeDataObject(TUIImage, m_phaseIcons[0], "phaseIcon");
	m_phaseIconsStartingY = m_phaseIcons[0]->GetLocation().y;
	getCodeDataObject(TUIButton, m_phaseButtons[0], "phaseButton");
	getCodeDataObject(TUIText, m_phaseNumbers[0], "phaseNumber");
	getCodeDataObject(TUIPie, m_phaseSwooshes[0], "phaseSwoosh");	
	registerMediatorObject(*m_phaseButtons[0], true);

	for(int i = 1; i < MAX_NUM_PHASE_ICONS; ++i)
	{
		m_phaseIcons[i] = NON_NULL (static_cast<UIImage *>(m_phaseIcons[0]->DuplicateObject ())); //lint !e1774 //stfu noob
		
		m_phaseIcons[0]->GetParent()->AddChild(m_phaseIcons[i]);
		m_phaseIcons[i]->Link ();
		m_phaseIcons[i]->SetVisible(true);
		char tmp[512];
		sprintf(tmp, "phaseIcon%d", i);
		m_phaseIcons[i]->SetName(UINarrowString(tmp));	
		m_phaseIcons[i]->GetParentWidget()->MoveChild(m_phaseIcons[i], UIBaseObject::Top );						
		
		m_phaseSwooshes[i] = NON_NULL (static_cast<UIPie *>(m_phaseSwooshes[0]->DuplicateObject ())); //lint !e1774 //stfu noob		
		m_phaseSwooshes[0]->GetParent()->AddChild(m_phaseSwooshes[i]);
		m_phaseSwooshes[i]->Link ();
		m_phaseSwooshes[i]->SetVisible(true);
		sprintf(tmp, "phaseSwoosh%d", i);
		m_phaseSwooshes[i]->SetName(UINarrowString(tmp));
		m_phaseSwooshes[i]->GetParentWidget()->MoveChild(m_phaseSwooshes[i], UIBaseObject::Top );						
		
		m_phaseNumbers[i] = NON_NULL (static_cast<UIText *>(m_phaseNumbers[0]->DuplicateObject ())); //lint !e1774 //stfu noob		
		m_phaseNumbers[0]->GetParent()->AddChild(m_phaseNumbers[i]);
		m_phaseNumbers[i]->Link ();
		m_phaseNumbers[i]->SetVisible(true);
		sprintf(tmp, "phaseNumber%d", i);
		m_phaseNumbers[i]->SetName(UINarrowString(tmp));
		m_phaseNumbers[i]->GetParentWidget()->MoveChild(m_phaseNumbers[i], UIBaseObject::Top );						
		
		m_phaseButtons[i] = NON_NULL (static_cast<UIButton *>(m_phaseButtons[0]->DuplicateObject ())); //lint !e1774 //stfu noob		
		m_phaseButtons[0]->GetParent()->AddChild(m_phaseButtons[i]);
		m_phaseButtons[i]->Link ();
		m_phaseButtons[i]->SetVisible(true);
		sprintf(tmp, "phaseButton%d", i);
		m_phaseButtons[i]->SetName(UINarrowString(tmp));
		m_phaseButtons[i]->GetParentWidget()->MoveChild(m_phaseButtons[i], UIBaseObject::Top );						
		
		registerMediatorObject(*m_phaseButtons[i], true);
		
	}
	
	registerMediatorObject(*m_categoryTabs, true);

	const UIData * const codeData = getCodeData ();
	NOT_NULL (codeData);

	if (!codeData->GetProperty (UILowerString("iconPathDraftGranted"), m_iconPathDraftGranted))
		DEBUG_FATAL (true, ("SwgCuiRoadmap no such property [iconPathDraftGranted]"));

	if (!page.GetObjectFromPath (m_iconPathDraftGranted.c_str (), TUIImageStyle))
		DEBUG_FATAL (true, ("SwgCuiRoadmap [%s] not found", m_iconPathDraftGranted.c_str ()));

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	if(Game::getSinglePlayer())
	{
		CuiSkillManager::setSkillTemplate("smuggler_2a");
		CuiSkillManager::setWorkingSkill("combat_brawler_2handmelee_01");
	}

	Unicode::String value;
	if (!codeData->GetProperty (UILowerString("stylePathPhaseComplete"), value))
		DEBUG_FATAL (true, ("SwgCuiRoadmap no such property [stylePathPhaseComplete]"));
	m_stylePhaseComplete = safe_cast<UIImageStyle *>(page.GetObjectFromPath (value.c_str (), TUIImageStyle));
	if (!m_stylePhaseComplete)
		DEBUG_FATAL (true, ("SwgCuiRoadmap [%s] not found", value.c_str ()));

	if (!codeData->GetProperty (UILowerString("stylePathPhaseIncomplete"), value))
		DEBUG_FATAL (true, ("SwgCuiRoadmap no such property [stylePathPhaseIncomplete]"));
	m_stylePhaseIncomplete = safe_cast<UIImageStyle *>(page.GetObjectFromPath (value.c_str (), TUIImageStyle));
	if (!m_stylePhaseIncomplete)
		DEBUG_FATAL (true, ("SwgCuiRoadmap [%s] not found", value.c_str ()));

	if (!codeData->GetProperty (UILowerString("stylePathPhaseCompleteSelected"), value))
		DEBUG_FATAL (true, ("SwgCuiRoadmap no such property [stylePathPhaseCompleteSelected]"));
	m_stylePhaseCompleteSelected = safe_cast<UIImageStyle *>(page.GetObjectFromPath (value.c_str (), TUIImageStyle));
	if (!m_stylePhaseCompleteSelected)
		DEBUG_FATAL (true, ("SwgCuiRoadmap [%s] not found", value.c_str ()));

	if (!codeData->GetProperty (UILowerString("stylePathPhaseIncompleteSelected"), value))
		DEBUG_FATAL (true, ("SwgCuiRoadmap no such property [stylePathPhaseIncompleteSelected]"));
	m_stylePhaseIncompleteSelected = safe_cast<UIImageStyle *>(page.GetObjectFromPath (value.c_str (), TUIImageStyle));
	if (!m_stylePhaseIncompleteSelected)
		DEBUG_FATAL (true, ("SwgCuiRoadmap [%s] not found", value.c_str ()));
}


//----------------------------------------------------------------------

SwgCuiRoadmap::~SwgCuiRoadmap ()
{
	delete m_callback;
	m_callback = 0;
	
	m_professionMediator->release();
	m_professionMediator = 0;
	
	if(m_itemRewardObject)
		delete m_itemRewardObject;
	m_itemRewardObject = 0;

	delete [] m_phaseIcons;
	delete [] m_phaseButtons;
	delete [] m_phaseNumbers;
	delete [] m_phaseSwooshes;
	deactivate();
}

//----------------------------------------------------------------------

bool SwgCuiRoadmap::OnMessage( UIWidget *context, const UIMessage & msg )
{
	UNREF(context);
	UNREF(msg);
	return true;
}

//-----------------------------------------------------------------

void SwgCuiRoadmap::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	UNREF(message);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::performActivate()
{
	CuiManager::requestPointer (true); 
	setIsUpdating(true);

	if(!RoadmapManager::playerIsOnRoadmap())
	{
		deactivate();
		return;
	}

	if(Game::getSinglePlayer())
	{
		setContextTemplate("smuggler_2a");
		setContextSkill("combat_brawler_2handmelee_01");
	}
	else
	{
		PlayerObject const * player = Game::getPlayerObject();
		CreatureObject const * playerCreature = Game::getPlayerCreature();
		if(!player || !playerCreature)	
		{
			deactivate();
			return;		
		}
		setContextTemplate(CuiSkillManager::getSkillTemplate());

		m_professionMediator->activate();
		setContextSkill(CuiSkillManager::getWorkingSkill());

		m_categoryTabs->Clear();
		const SkillObject * const politicianSkill = SkillManager::getInstance ().getSkill ("social_politician_novice");	
		const SkillObject * const storytellerSkill = SkillManager::getInstance().getSkill ("class_chronicles_novice");

		m_showingPoliticianTab = playerCreature->hasSkill(*politicianSkill);
		m_showingSpaceTab = !RoadmapManager::getPlayerSpaceTemplateName().empty();
		m_showingStorytellerTab = playerCreature->hasSkill(*storytellerSkill);

		if(m_showingSpaceTab || m_showingPoliticianTab || m_showingStorytellerTab )
			m_categoryTabs->AppendTab(s_basicsTab.localize(), NULL);
		if(m_showingPoliticianTab)
			m_categoryTabs->AppendTab(s_politicianTab.localize(), NULL);
		if(m_showingSpaceTab)
			m_categoryTabs->AppendTab(s_spaceTab.localize(), NULL);
		if(m_showingStorytellerTab)
			m_categoryTabs->AppendTab(s_storytellerTab.localize(), NULL);
		m_experienceBar->GetParentWidget()->GetParentWidget()->SetVisible(false);
		m_professionMediator->HideStorytellerXPBar();
	}
	m_callback->connect (*this, &SwgCuiRoadmap::onSkillsChanged,                  static_cast<CreatureObject::Messages::SkillsChanged *>(0));
	m_callback->connect (*this, &SwgCuiRoadmap::onPlayerExpChanged,               static_cast<PlayerObject::Messages::ExperienceChanged *>        (0));	
	m_callback->connect (*this, &SwgCuiRoadmap::onLevelChanged,                   static_cast<CreatureObject::Messages::LevelChanged *>(0));
	
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::performDeactivate()
{
	CuiManager::requestPointer (false); 
	setIsUpdating(false);	
	m_professionMediator->deactivate();
	m_callback->disconnect (*this, &SwgCuiRoadmap::onSkillsChanged,                  static_cast<CreatureObject::Messages::SkillsChanged *>(0));	
	m_callback->disconnect (*this, &SwgCuiRoadmap::onPlayerExpChanged,               static_cast<PlayerObject::Messages::ExperienceChanged *>        (0));	
	m_callback->disconnect (*this, &SwgCuiRoadmap::onLevelChanged,                   static_cast<CreatureObject::Messages::LevelChanged *>(0));
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::setContextSkill(std::string const & skillName)
{
	m_skillName = skillName;
	if(skillName.empty())
	{
		showZoomedOutDisplay();
		std::string const &iconPath = RoadmapManager::getIconPathForSkill(s_phaseString);
		UIImageStyle *style = safe_cast<UIImageStyle *>(getPage().GetObjectFromPath(iconPath.c_str()));
		m_activityIcon->SetStyle(style);
		updateItemReward();
	}
	else
	{	
		UIString localizedSkillName;
		CuiSkillManager::localizeSkillName(skillName, localizedSkillName);
		m_nextActivityName->SetLocalText(localizedSkillName);

		// Force english to be used for all aurabesh values.  This is the star wars glyph font, so it
		// only has english characters in it.
		UIString aurabeshSkillName;
		CuiSkillManager::localizeSkillName(skillName, aurabeshSkillName, true);
		m_nextActivityNameAurabesh->SetLocalText(aurabeshSkillName);

		UIString skillDescription;
		CuiSkillManager::localizeSkillDescription(skillName, skillDescription);
		m_nextActivityDescription->SetLocalText(skillDescription);
		updateExperienceBar();
		updateRewardsTable();
		
		std::string const &iconPath = RoadmapManager::getIconPathForSkill(skillName);
		UIImageStyle *style = safe_cast<UIImageStyle *>(getPage().GetObjectFromPath(iconPath.c_str()));
		m_activityIcon->SetStyle(style);
		m_professionMediator->setCenterSkill(skillName);

		updateItemReward();
	}
}


//----------------------------------------------------------------------

void SwgCuiRoadmap::setContextTemplate(std::string const & templateName)
{
	m_templateName = templateName;
	m_roadmapName = RoadmapManager::getRoadmapNameForTemplateName(templateName);

	char tmp[512];
	
	sprintf(tmp, "title_%s", m_roadmapName.c_str());
	StringId stringId(ROADMAP_STRING_TABLE, tmp);
	m_titleText->SetLocalText(stringId.localize());
		
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::setContextSpaceTemplate(std::string const & templateName)
{
	m_spaceTemplateName = templateName;
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::setExperienceBar(float val)
{
	UNREF(val);
	/*UIWidget *parent = m_experienceBar->GetParentWidget();
	parent->SetVisible(true);	
	const long usableHeight = parent->GetWidth ();					
	m_experienceBar->SetWidth (static_cast<long>(usableHeight * val));*/
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::updateExperienceBar()
{	
	const CreatureObject * const player = Game::getPlayerCreature ();
	UIWidget *parent = m_experienceBar->GetParentWidget();
	if(!parent)
		return;
	UIWidget *grandParent = parent->GetParentWidget();
	if(!grandParent)
		return;
	parent->SetVisible(false);	
	PlayerObject const * playerAsPlayer = Game::getPlayerObject();
	if(!playerAsPlayer)
		return;
	const SkillObject * skill;
	if(m_currentTabType == TT_GROUND)
	{		
		grandParent->SetVisible(false);
		skill = SkillManager::getInstance ().getSkill (m_skillName);
		if(!skill || !player)		
			return;			
	}
	else if(m_currentTabType == TT_SPACE)
	{
		grandParent->SetVisible(true);
		parent->SetVisible(true);
		std::string const &workingSkill = RoadmapManager::getPlayerSpaceWorkingSkill();		
		skill = SkillManager::getInstance ().getSkill (workingSkill);
		if(!skill || !player)		
			return;		
	}
	else if(m_currentTabType == TT_STORYTELLER)
	{
		grandParent->SetVisible(false);
		std::string const &workingSkill = RoadmapManager::getPlayerStorytellerWorkingSkill();		
		skill = SkillManager::getInstance ().getSkill (workingSkill);
		if(!skill || !player)		
			return;	
	}
	else
	{
		grandParent->SetVisible(false);
		std::string const &workingSkill = RoadmapManager::getPlayerPoliticianWorkingSkill();		
		skill = SkillManager::getInstance ().getSkill (workingSkill);
		if(!skill || !player)		
			return;	
	}

	const SkillObject::ExperiencePair * const exp = skill->getPrerequisiteExperience ();
					
	if (exp)
	{
		const long usableHeight = parent->GetWidth ();					
		
		const std::string & expName  = exp->first;
		const int           expValue = exp->second.first;
		
		char tmp[512];

		if (expValue > 0)
		{
			int expCurrent = 0;
			if (!player->getExperience (expName, expCurrent))
				expCurrent = 0;
			
			Unicode::String tooltip = s_exp.localize();
			sprintf(tmp, "%d/%d", expCurrent, expValue);
			tooltip.append(Unicode::narrowToWide(tmp));
			parent->SetLocalTooltip(tooltip);
			parent->SetVisible (true);
			
			if(m_currentTabType == TT_STORYTELLER)
			{
				int nextLevelCap = expValue;
				int previousCap = 0;
				SkillObject const * const previousSkill = skill->getPrevSkill();
				if(previousSkill)
				{
					const SkillObject::ExperiencePair * const prevExp = previousSkill->getPrerequisiteExperience ();
					if(prevExp)
						previousCap = prevExp->second.first;
				}

				m_professionMediator->SetStorytellerXPBar(expCurrent, nextLevelCap, previousCap);
			}


			if (expCurrent >= expValue)
			{
				m_experienceBar->SetWidth (usableHeight);
				m_professionMediator->setInnerExperienceBarValue(1.0f);
			}
			else
			{
				const long desiredHeight = static_cast<long>(static_cast<float>(usableHeight) * expCurrent / expValue);
				m_professionMediator->setInnerExperienceBarValue(expCurrent / (expValue + 0.0f));
				m_professionMediator->setCenterXPTextValue(expCurrent / (expValue + 0.0f));
				m_experienceBar->SetWidth (desiredHeight);
			}
		}
		else
		{			
			m_experienceBar->SetWidth (0);
			m_professionMediator->setInnerExperienceBarValue(0.0f);
			m_professionMediator->setCenterXPTextValue(0.0f);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::updateRewardsTable()
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_rewardsTable->GetTableModel ());
	NOT_NULL (model);
	
	const SkillObject * const skill = SkillManager::getInstance ().getSkill (m_skillName);
	if(!skill)
		return;
	
	model->Attach (0);
	m_rewardsTable->SetTableModel (0);
	model->ClearData ();
	
	if(skill)
	{

		{
			const SkillObject::GenericModVector & skillMods = skill->getStatisticModifiers ();
			
			Unicode::String tmp;
			static char buf [128];
			static const size_t buf_size = sizeof (buf);
			
			for (SkillObject::GenericModVector::const_iterator it = skillMods.begin (); it != skillMods.end (); ++it)
			{
				const std::string & skillModName  = (*it).first;
				const int           skillModValue = (*it).second;
				
				//-- skip private skill mods
				if (!_strnicmp (skillModName.c_str (), "private_", 8))
					continue;

				tmp.clear ();
				snprintf (buf, buf_size, "%+d ", skillModValue);
				tmp = Unicode::narrowToWide(buf);
				UIString tmp2;
				CuiSkillManager::localizeSkillModName (skillModName, tmp2);
				tmp.append(tmp2);				
				UIData * const data1 = model->AppendCell (0, skillModName.c_str(), Unicode::emptyString);
				UIData * const data0 = model->AppendCell (1, 0, tmp);
				
				static Unicode::String desc;
				desc.clear ();
				CuiSkillManager::localizeSkillModDesc (skillModName, desc);

				tmp.push_back ('\n');
				tmp += desc;

				data0->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tmp);
				data1->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tmp);
			}
		}
		
		addSchematicGroups (skill->getSchematicsGranted (), *model);
		
		{
			Unicode::String tmp;
			static char buf [128];
			static const size_t buf_size = sizeof (buf);

			{
				const SkillObject::StringVector & commands = skill->getCommandsProvided ();	
			
				for (SkillObject::StringVector::const_iterator it = commands.begin (); it != commands.end (); ++it)
				{
					std::string cmdName( Unicode::toLower ((*it)) );

					
					size_t endpos = 0;
					std::string cmd_first_word = cmdName;
					Unicode::getFirstToken (cmdName, 0, endpos, cmd_first_word, s_command_breaker);

					//-- skip private skill mods
					if (!_strnicmp (cmd_first_word.c_str (), "private_", 8))
						continue;					

					const Command & command = CommandTable::getCommand (Crc::normalizeAndCalculate (cmd_first_word.c_str ()));
					const bool isCommand    = !command.isNull ();
					
					std::string iconPath;
					
					if (isCommand)
					{
						if (command.m_visibleToClients < 2)
							continue;

						CuiDragInfo cdi;
						cdi.type = CuiDragInfoTypes::CDIT_command;
						cdi.str  = cmdName;
						
						UIImageStyle * icon = CuiIconManager::findIconImageStyle (cdi);
						
						if (icon)
						{
							iconPath = icon->GetFullPath ();							
						}
					}
					
					UIData * const data0 = model->AppendCell (0, cmdName.c_str (), Unicode::narrowToWide (iconPath));
					
					tmp.clear ();
					CuiSkillManager::localizeCmdName (cmdName, tmp);
					UIData * const data1 = model->AppendCell (1, 0, tmp);
					
					CuiSkillManager::localizeCmdDescription (cmdName, tmp);
					data0->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tmp);
					data1->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tmp);
				}
			}
			
		}
	}


	m_rewardsTable->SetTableModel (model);
	model->Detach (0);

	model->sortOnColumn (0, UITableModel::SD_down);

	UIWidget * const rewardsTableParent = m_rewardsTable->GetParentWidget();

	if (rewardsTableParent)
	{
		int const rewardsTableHeight = m_rewardsTable->GetRowCount() * (m_rewardsTable->GetCellHeight() + m_rewardsTable->GetCellPadding().y);

		rewardsTableParent->SetHeight(rewardsTableHeight);
		m_rewardsTable->SetHeight(rewardsTableHeight);

		UIComposite * const rewardsTableParentParent = dynamic_cast<UIComposite *>(rewardsTableParent->GetParentWidget());

		if (rewardsTableParentParent)
			rewardsTableParentParent->Pack();
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::addSchematicGroups (const SkillObject::StringVector & sv, UITableModelDefault & model)
{
	static std::string                 modifiedDraftName;
	static Unicode::String             modifiedDraftNameLocalized;

	// @note: revoked schematics were removed from the skill system.
	// there used to be an m_iconPathDraftRevoked and a '-' prefix here to support that.
	const Unicode::String & iconPath = m_iconPathDraftGranted;
	const  char             prefix   = '+';
	
	static DraftSchematicGroupManager::SchematicVector drafts;

	for (SkillObject::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & group = Unicode::toLower (*it);

		drafts.clear ();
		if (!DraftSchematicGroupManager::getSchematicsForGroup (group, drafts))
		{
			WARNING (true, ("SwgCuiRoadmap skill calls for invalid schematic group [%s]", group.c_str ()));
			continue;
		}

		for (DraftSchematicGroupManager::SchematicVector::const_iterator dit = drafts.begin (); dit != drafts.end (); ++dit)
		{
			const std::pair<uint32, uint32> & draftCrc = *dit;
			
			const char * sharedDraftTemplateName = ObjectTemplateList::lookUp(draftCrc.second).getString();
			modifiedDraftName = prefix;
			modifiedDraftName += sharedDraftTemplateName;

			UIData * const data0 = model.AppendCell (0, modifiedDraftName.c_str (), iconPath);
						
			const DraftSchematicInfo * const dsi = DraftSchematicManager::cacheDraftSchematic (draftCrc);
			
			UIData * data1 = 0;
			
			if (dsi)
			{
				modifiedDraftNameLocalized.clear ();
				modifiedDraftNameLocalized.push_back (prefix);
				modifiedDraftNameLocalized += dsi->getLocalizedName ();
				data1 = model.AppendCell (1, 0, modifiedDraftNameLocalized);

				static Unicode::String header;
				static Unicode::String desc;
				static Unicode::String attribs;
				header.clear ();
				desc.clear ();
				attribs.clear ();

				dsi->formatDescription (header, desc, attribs, true);

				header = modifiedDraftNameLocalized;
				header.push_back ('\n');
				header += desc;
				data0->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, header);
				data1->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, header);
			}
			else
			{
				WARNING (true, ("SwgCuiRoadmap unable to find draft schematic [%lu]", draftCrc.first));
				data1 = model.AppendCell (1, 0, Unicode::narrowToWide (sharedDraftTemplateName));
			}

			UNREF (data1);
			UNREF (data0);
		}

	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::showZoomedOutDisplay()
{
	PlayerObject const * playerAsPlayer = Game::getPlayerObject();
	if(!playerAsPlayer)
		return;
	std::string const &skillTemplate = CuiSkillManager::getSkillTemplate();
	char tmp[512];
	int phase = m_professionMediator->getShowingPhase();
	if(phase == -1)
		phase = m_professionMediator->getPlayersCurrentPhase();
	phase = std::max(0, phase);
	std::string const &roadmapName = RoadmapManager::getRoadmapNameForTemplateName(skillTemplate);
	sprintf(tmp, "phase_title_%s_%d", roadmapName.c_str(), phase);
	StringId phaseName(ROADMAP_STRING_TABLE, tmp);
	UIString result = phaseName.localize();
	m_nextActivityName->SetLocalText(result);
	m_nextActivityNameAurabesh->SetLocalText(result);
	m_nextActivityDescription->SetLocalText(Unicode::emptyString);
	
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_rewardsTable->GetTableModel ());
	NOT_NULL (model);
	model->ClearData();	
	m_rewardsTable->SetTableModel (model);
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getContextRoadmap()
{
	return m_roadmapName;
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getContextTemplate()
{
	return m_templateName;
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getContextSpaceTemplate()
{
	return m_spaceTemplateName;
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getPoliticianTemplate()
{
	return s_politicianTemplateName;
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getStorytellerTemplate()
{
	return s_storytellerTemplateName;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::onSkillsChanged (const CreatureObject & creature)
{
	if (&creature == static_cast<const Object *>(Game::getPlayer ()))
	{
		setContextTemplate(CuiSkillManager::getSkillTemplate());
		setContextSkill(CuiSkillManager::getWorkingSkill());
		updateExperienceBar();
		m_professionMediator->onSkillsChanged(creature);
		m_professionMediator->showZoomedInPhase(m_professionMediator->getShowingPhase());
	}	
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::onLevelChanged (const CreatureObject & creature)
{
	if (&creature == static_cast<const Object *>(Game::getPlayer ()))
	{
		setContextTemplate(CuiSkillManager::getSkillTemplate());
		setContextSkill(CuiSkillManager::getWorkingSkill());
		updateExperienceBar();
		m_professionMediator->onSkillsChanged(creature);
		m_professionMediator->showZoomedInPhase(m_professionMediator->getShowingPhase());
	}	
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::onPlayerExpChanged (const PlayerObject & player)
{
	if (&player == static_cast<const Object *>(Game::getPlayerObject ()))
	{
		setContextTemplate(CuiSkillManager::getSkillTemplate());
		setContextSkill(CuiSkillManager::getWorkingSkill());
		updateExperienceBar();
		m_professionMediator->showZoomedInPhase(m_professionMediator->getShowingPhase());
		m_professionMediator->onPlayerExpChanged(player);
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::updateItemReward()
{
	std::string templateName;

	if (m_currentTabType == TT_GROUND)
		templateName = RoadmapManager::getTemplateForRoadmapTrack(m_roadmapName, RoadmapManager::getPlayerBranch());
	else if (m_currentTabType == TT_SPACE)
		templateName = m_spaceTemplateName;
	else if (m_currentTabType == TT_STORYTELLER)
		templateName = s_storytellerTemplateName;
	else
		templateName = s_politicianTemplateName;

	const std::string &appearanceName = RoadmapManager::getItemRewardAppearance(templateName, m_skillName);
	const std::string &stringIdName = RoadmapManager::getItemRewardTextId(templateName, m_skillName);
	
	if(m_itemRewardObject)
	{
		delete m_itemRewardObject;
		m_itemRewardObject = 0;
	}
	if(appearanceName.empty())	
	{
		m_itemRewardPage->SetVisible(false);
		return;		
	}
	m_itemRewardPage->SetVisible(true);
	
	Appearance* const appearance = AppearanceTemplateList::createAppearance (appearanceName.c_str ());
	Object* object = new Object ();
	object->setAppearance (appearance);
	m_itemRewardViewer->setObject(object);
	m_itemRewardObject = object;
	
	StringId s(stringIdName.c_str());
	m_itemRewardText->SetText(s.localize());
}

//----------------------------------------------------------------------
float SwgCuiRoadmap::getXpValue(const std::string &workingSkill)
{
	const CreatureObject * const player = Game::getPlayerCreature ();
	const SkillObject * const skill = SkillManager::getInstance ().getSkill (workingSkill);
	if(!skill || !player)
	{
		return 0.0f;
	}
	const SkillObject::ExperiencePair * const exp = skill->getPrerequisiteExperience ();
				
	if (exp)
	{			
		const std::string & expName  = exp->first;
		const int           expValue = exp->second.first;
		
		if (expValue > 0)
		{
			int expCurrent = 0;
			if (!player->getExperience (expName, expCurrent))
				expCurrent = 0;
			
			if (expCurrent >= expValue)
			{
				return 1.0f;
			}
			else
			{
				return expCurrent / (expValue + 0.0f);					
			}
		}
		else
		{						
			return 0.0f;
		}
	}
	return 0.0f;
}

float SwgCuiRoadmap::getMainlineXpValue()
{
	if(m_currentTabType == TT_GROUND)
	{	
		if(m_professionMediator->getShowingPhase() == m_professionMediator->getPlayersCurrentPhase())
		{
			std::string const &workingSkill = CuiSkillManager::getWorkingSkill();
			return getXpValue(workingSkill);
		}
	}
	else if(m_currentTabType == TT_SPACE)
	{
		std::string const &workingSkill = RoadmapManager::getPlayerSpaceWorkingSkill();
		float r = getXpValue(workingSkill);
		return r;
	}
	else if(m_currentTabType == TT_POLITICIAN)
	{
		std::string const &workingSkill = RoadmapManager::getPlayerPoliticianWorkingSkill();
		return getXpValue(workingSkill);
	}
	else if(m_currentTabType == TT_STORYTELLER)
	{
		std::string const &workingSkill = RoadmapManager::getPlayerStorytellerWorkingSkill();
		return getXpValue(workingSkill);
	}
	return 0.0f;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::OnTabbedPaneChanged(UIWidget * Context)
{
	if(Context == m_categoryTabs)
	{
		m_currentTabType = static_cast<TabType>(m_categoryTabs->GetActiveTab());		
		//The index could be wrong if space is shown in the second slot, because it is index 3
		if(m_showingSpaceTab && !m_showingPoliticianTab && !m_showingStorytellerTab && (m_currentTabType == TT_POLITICIAN || m_currentTabType == TT_STORYTELLER))
		{
			m_currentTabType = TT_SPACE;
		}
		// Index isn't correct, space isn't shown so ST is off.
		if(!m_showingSpaceTab && !m_showingPoliticianTab && (m_currentTabType == TT_POLITICIAN))
		{
			m_currentTabType = TT_STORYTELLER;
		}

		// Sigh...we're 1 off if we're showing space but not politician.
		if(m_showingSpaceTab && !m_showingPoliticianTab && (m_currentTabType == TT_POLITICIAN || m_currentTabType == TT_SPACE ))
		{
			m_currentTabType =static_cast<SwgCuiRoadmap::TabType>( static_cast<int>(m_currentTabType) + 1 );
		}

		// Again, fixing indices...Assuming this index will always match the correct enumed Tab Type is a horrible assumption.
		if(!m_showingSpaceTab && m_showingPoliticianTab && (m_currentTabType == TT_SPACE))
			m_currentTabType = TT_STORYTELLER;

		if(m_currentTabType == TT_GROUND)
		{
			setContextTemplate(CuiSkillManager::getSkillTemplate());

			setContextSkill(CuiSkillManager::getWorkingSkill());
		}
		else if (m_currentTabType == TT_SPACE)
		{		
			setContextSpaceTemplate(RoadmapManager::getPlayerSpaceTemplateName());
			setContextSkill(RoadmapManager::getPlayerSpaceWorkingSkill());
		}
		else if (m_currentTabType == TT_STORYTELLER)
		{
			setContextSkill(RoadmapManager::getPlayerStorytellerWorkingSkill());
		}
		else
		{
			setContextSkill(RoadmapManager::getPlayerPoliticianWorkingSkill());
		}
		m_professionMediator->showCategory(m_currentTabType);		
	}
}
	
//----------------------------------------------------------------------

void SwgCuiRoadmap::configPhaseIconDisplay(int numPhaseIconsDisplayed, float *phaseProgress, int selectedPhase)
{
	m_phaseIcons[0]->GetParentWidget()->SetVisible(true);
	int i;
	for(i = 0; i < MAX_NUM_PHASE_ICONS; ++i)
	{
	
		bool selected = (i == selectedPhase);
		bool complete = (phaseProgress[i] >= 1.0f);
		if(i < numPhaseIconsDisplayed)
		{
			m_phaseIcons[i]->SetVisible(true);
			m_phaseButtons[i]->SetEnabled(true);
			float swooshStartAngle = 6.28f * 0.33f + 6.28f * 0.85f * phaseProgress[i];
			float swooshValue = phaseProgress[i];
			if(swooshValue >= 1.0f)
				swooshValue = 0.0f;
			m_phaseSwooshes[i]->SetVisible(true);
			m_phaseSwooshes[i]->SetPropertyFloat(UIPie::PropertyName::PieAngleClipFinish, 6.28f * 0.33f);
			m_phaseSwooshes[i]->SetPropertyFloat(UIPie::PropertyName::PieAngleStart, swooshStartAngle);
			m_phaseSwooshes[i]->SetPropertyFloat(UIPie::PropertyName::PieValue, swooshValue);
			char tmp[512];
			sprintf(tmp, "%d", i+1);
			m_phaseNumbers[i]->SetLocalText(Unicode::narrowToWide(tmp));
			m_phaseNumbers[i]->SetVisible(true);
			m_phaseIcons[i]->SetLocation(15 + 90 * i - (selected ? 10 : 0), m_phaseIconsStartingY - (selected ? 10 : 0));
			m_phaseButtons[i]->SetLocation(15 + 90 * i, m_phaseButtons[i]->GetLocation().y);
			m_phaseSwooshes[i]->SetLocation(15 + 90 * i, m_phaseSwooshes[i]->GetLocation().y);
			m_phaseNumbers[i]->SetLocation(15 + 90 * i, m_phaseNumbers[i]->GetLocation().y);
		}
		else
		{			
			m_phaseButtons[i]->SetEnabled(false);
			m_phaseIcons[i]->SetVisible(false);
			m_phaseSwooshes[i]->SetVisible(false);
			m_phaseNumbers[i]->SetVisible(false);
		}
		if(complete)
		{
			m_phaseNumbers[i]->SetProperty(s_PalProp, Unicode::emptyString);
			m_phaseNumbers[i]->SetProperty(UIWidget::PropertyName::Color, Unicode::narrowToWide("#E4C86B"));
		}
		else
		{			
			m_phaseNumbers[i]->SetProperty(s_PalProp, Unicode::narrowToWide("contrast3h"));
		}
		if(selected)
		{
			m_phaseIcons[i]->SetSize(UISize(70,70));
		}
		else
		{
			m_phaseIcons[i]->SetSize(UISize(50,50));
		}
		if(selected && complete)
			m_phaseIcons[i]->SetStyle(m_stylePhaseCompleteSelected);
		else if(selected && !complete)
			m_phaseIcons[i]->SetStyle(m_stylePhaseIncompleteSelected);
		else if(!selected && complete)
			m_phaseIcons[i]->SetStyle(m_stylePhaseComplete);
		else if(!selected && !complete)
			m_phaseIcons[i]->SetStyle(m_stylePhaseIncomplete);
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::disablePhaseIconDisplay()
{
	m_phaseIcons[0]->GetParentWidget()->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap::OnButtonPressed   (UIWidget * context)
{
	for(int i = 0; i < MAX_NUM_PHASE_ICONS; ++i)
	{	
		if (context == m_phaseButtons[i])
		{
			m_professionMediator->showZoomedInPhase(i);
			return;
		}		
	}
}

//----------------------------------------------------------------------

std::string const &SwgCuiRoadmap::getContextSkill()
{
	return m_skillName;
}
	
//----------------------------------------------------------------------
	
bool SwgCuiRoadmap::isCurrentTemplateLevelBased()
{
	return RoadmapManager::isLevelBasedTemplate(m_templateName);
}