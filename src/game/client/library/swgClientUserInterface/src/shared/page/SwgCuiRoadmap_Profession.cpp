//======================================================================
//
// SwgCuiRoadmap_Profession.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiRoadmap_Profession.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedObject/Controller.h"
#include "sharedSkillSystem/LevelManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiRoadmap.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIPie.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"

#include <map>
#include <vector>

namespace SwgCuiRoadmap_ProfessionNamespace
{
	//You can't just change these 3 numbers without checking the rest of this code
	const int MAX_NUM_BRANCHES = 4; //Numbered 0-based
	const int PHASE_BREAK = 0;  //Phase at which the system breaks	
	const int MAX_NUM_PHASES_IN_EACH_TRACK = 4; //Phases in a single track
	const int PHASES_AFTER_BREAK = MAX_NUM_PHASES_IN_EACH_TRACK - PHASE_BREAK; // Num phases after break in each track
	const int NUM_GROUND_PHASES = 4; //0-based; 4 normal phases, plus 3 extra; phase 5 is phase 2 of track 1
	const int FIRST_SPACE_PHASE = NUM_GROUND_PHASES;
	const int NUM_SPACE_PHASES = 1;
	const int FIRST_POLITICIAN_PHASE = NUM_GROUND_PHASES + NUM_SPACE_PHASES;
	const int NUM_POLITICIAN_PHASES = 1;
	const int FIRST_STORYTELLER_PHASE = NUM_GROUND_PHASES + NUM_SPACE_PHASES + NUM_POLITICIAN_PHASES;
	const int NUM_STORYTELLER_PHASES = 1;
	const int TOTAL_NUM_PHASES = NUM_GROUND_PHASES + NUM_SPACE_PHASES + NUM_POLITICIAN_PHASES + NUM_STORYTELLER_PHASES;

	const UILowerString SKILL_NAME = UILowerString("skillname");
	const char * skillButtonText = "SkillButton";
	UIColor GREY(128,128,128);

	const std::string s_emptyString;
	
	const float STARTING_ANGLE = 0.33f;
	const float TOTAL_ANGLE = 0.85f;

	const int CHICLET_ICONS_DISTANCE = 48;
	const int CHICLET_LINKER_DISTANCE = 13;
	const int LINES_DISTANCE = 90;
	const int SKILL_ICON_DISTANCE = 2;

	std::string const ROADMAP_STRING_FILE("ui_roadmap");

	const int NUM_CHICLET_STEPS = 4;  // The fact that this is equal to MAX_NUM_BRANCHES is just a coincidence

	std::string const s_buttonImageSource("ui_profession_");
	std::string const s_buttonImageSourcePostfix("_a1");
	std::string const s_buttonSelected("_select");
	std::string const s_spaceButtonImage("space");
	std::string const s_politicianButtonImage("politician");
	std::string const s_storytellerButtonImage("chronical_master_a1");

	const int HIGHLIGHT_PAGE_OFFSET_X = 4;
	const int HIGHLIGHT_PAGE_OFFSET_Y = 4;
	
	const UILowerString LEVEL = UILowerString("level");	
	const int MAX_NUMBER_SKILLS_BEFORE_USING_SMALL_ICON = 12;

	const UISize SMALL_HIGHLIGHT_PAGE_SIZE(34,36);
	const UISize BIG_HIGHLIGHT_PAGE_SIZE(54,54);

	const UILowerString s_PalProp  = UILowerString         ("PalColor");

}

using namespace SwgCuiRoadmap_ProfessionNamespace;

//----------------------------------------------------------------------

SwgCuiRoadmap_Profession::SwgCuiRoadmap_Profession (UIPage & page, SwgCuiRoadmap *parentMediator) :
CuiMediator         ("SwgCuiRoadmap_Profession", page),
UIEventCallback     (),
MessageDispatch::Receiver (),
m_zoomedInPages(0),
m_zoomedInRootPages(0),
m_swooshes(0),
m_phaseForwardButton(0),
m_phaseBackwardButton(0),
m_showingPhase(0),
m_highlightPage(0),
m_selectedPage(0),
m_backgroundImages(0),
m_levelBackgroundImages(0),
m_levelLabels(0),
m_parentMediator(parentMediator),
m_completedSkillsThisPhase(0),
m_uncompletedSkillsThisPhase(0),
m_playersCurrentPhase(-1),
m_sampleIconPage(0),
m_sampleSmallIconPage(0),
m_sampleFilledChicletPages(0),
m_sampleEmptyChicletPages(0),
m_sampleChicletTextPage(0),
m_sampleChicletLinkerImage(0),
m_sampleLinePage(0),
m_centerIcon(0),
m_centerText(0),
m_centerXPText(0),
m_storytellerExpPage(0),
m_storytellerExpBar(0),
m_experienceValue(0.0f),
m_curNumBranches(0),
m_okToSetUnchecked(false),
m_innerXpValue(0.0f),
m_needlePages(0),
m_professionImages(0),
m_phasePages(0),
m_currentCategory(static_cast<SwgCuiRoadmap::TabType>(0)),
m_usingSmallIcons(0)
{
	m_zoomedInPages = new UIPage *[TOTAL_NUM_PHASES];   //The last page is the space page
	m_zoomedInRootPages = new UIPage *[TOTAL_NUM_PHASES];   
	m_swooshes = new UIPie *[TOTAL_NUM_PHASES];
	m_phaseTitles = new UIText *[TOTAL_NUM_PHASES];
	m_roadmapTitles = new UIText *[TOTAL_NUM_PHASES];
	m_phaseNumbers = new UIText *[TOTAL_NUM_PHASES];
	m_needlePages = new UIPage *[TOTAL_NUM_PHASES];
	m_backgroundImages = new UIImage *[TOTAL_NUM_PHASES];
	m_levelBackgroundImages = new UIImage *[TOTAL_NUM_PHASES];
	m_levelLabels = new UIText *[TOTAL_NUM_PHASES];
	m_professionImages = new UIImage *[TOTAL_NUM_PHASES];
	m_phasePages = new UIPage *[TOTAL_NUM_PHASES];
	m_sampleEmptyChicletPages = new UIPage *[NUM_CHICLET_STEPS];	
	m_sampleFilledChicletPages = new UIPage *[NUM_CHICLET_STEPS];
	
	getCodeDataObject(TUIPage, m_zoomedInPages[0], "zoomedInPage");
	int i;		
	
	for(i = 1; i < TOTAL_NUM_PHASES; ++i)
	{
		m_zoomedInPages[i] = NON_NULL (static_cast<UIPage *>(m_zoomedInPages[0]->DuplicateObject ())); //lint !e1774 //stfu noob
		
		m_zoomedInPages[0]->GetParent()->AddChild(m_zoomedInPages[i]);
		m_zoomedInPages[i]->Link ();
		m_zoomedInPages[i]->SetVisible(true);

		char tmp[512];
		sprintf(tmp, "ZoomIn%d", i);
		m_zoomedInPages[i]->SetName(UINarrowString(tmp));
	}
	
	for(i = 0; i < NUM_CHICLET_STEPS; ++i)
	{
		char pagePath[512];
		sprintf(pagePath, "sampleFilledChiclet%d", i+1);		
		getCodeDataObject(TUIPage, m_sampleFilledChicletPages[i], pagePath);
		m_sampleFilledChicletPages[i]->SetVisible(false);
		sprintf(pagePath, "sampleEmptyChiclet%d", i+1);		
		getCodeDataObject(TUIPage, m_sampleEmptyChicletPages[i], pagePath);		
		m_sampleEmptyChicletPages[i]->SetVisible(false);		
	}
	getCodeDataObject(TUIText, m_sampleChicletTextPage, "sampleLevelNumber");
	getCodeDataObject(TUIImage, m_sampleChicletLinkerImage, "sampleChicletLinker");
	getCodeDataObject(TUIPage, m_sampleLinePage, "sampleLine");
	m_sampleChicletTextPage->SetVisible(false);
	m_sampleChicletLinkerImage->SetVisible(false);
	m_sampleLinePage->SetVisible(false);

	for(i = 0; i < TOTAL_NUM_PHASES; ++i)
	{		
		const UIData * const codeData = safe_cast<UIData *>(m_zoomedInPages[0]->GetChild ("CodeData"));
		NOT_NULL (codeData);
		m_zoomedInRootPages[i] = m_zoomedInPages[i];
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIPie, m_swooshes[i], "swoosh");	
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIText, m_phaseTitles[i], "phaseTitle");
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIText, m_roadmapTitles[i], "roadmapTitle");
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIText, m_phaseNumbers[i], "phaseNumber");						
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIPage, m_needlePages[i], "needle");				
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIImage, m_professionImages[i], "professionImage");
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIPage, m_phasePages[i], "phasePage");							
		
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIImage, m_backgroundImages[i], "background");	
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIImage, m_levelBackgroundImages[i], "levelBackground");				
		getCodeDataObject(m_zoomedInPages[i], codeData, TUIText, m_levelLabels[i], "levelLabel");				
		
	}

	getCodeDataObject(TUIPage, m_highlightPage, "highlight");
	getCodeDataObject(TUIPage, m_selectedPage, "selected");

	getCodeDataObject(TUIButton, m_phaseForwardButton, "phaseForwardButton");
	getCodeDataObject(TUIButton, m_phaseBackwardButton, "phaseBackwardButton");
	registerMediatorObject(*m_phaseForwardButton, true);
	registerMediatorObject(*m_phaseBackwardButton, true);

	getCodeDataObject(TUIPage, m_sampleIconPage, "sampleIconPage");
	m_sampleIconPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_sampleSmallIconPage, "sampleSmallIconPage");
	m_sampleSmallIconPage->SetVisible(false);

	getCodeDataObject(TUIImage, m_centerIcon, "CenterIcon");
	getCodeDataObject(TUIText, m_centerText, "CenterText");
	getCodeDataObject(TUIText, m_centerXPText, "CenterXPText");	

	getCodeDataObject(TUIPage, m_storytellerExpPage, "ChronExperiencePage");
	getCodeDataObject(TUIPage, m_storytellerExpBar, "ChronExperienceBar");
	m_storytellerExpPage->SetVisible(false);
	
	m_usingSmallIcons = new bool[TOTAL_NUM_PHASES];
}


//----------------------------------------------------------------------

SwgCuiRoadmap_Profession::~SwgCuiRoadmap_Profession ()
{
	delete [] m_zoomedInPages;
	delete [] m_zoomedInRootPages;
	delete [] m_swooshes;
	delete [] m_sampleEmptyChicletPages;
	delete [] m_sampleFilledChicletPages;
	delete [] m_needlePages;
	delete [] m_backgroundImages;
	delete [] m_levelBackgroundImages;
	delete [] m_levelLabels;
	delete [] m_professionImages;
	delete [] m_phasePages;
	delete [] m_roadmapTitles;
	delete [] m_phaseNumbers;
	delete [] m_phaseTitles;
	delete [] m_usingSmallIcons;
	deactivate();
}

//-----------------------------------------------------------------

void SwgCuiRoadmap_Profession::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	UNREF(message);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::performActivate()
{
	m_currentCategory = SwgCuiRoadmap::TT_GROUND;
	updateContextRoadmap();
	showZoomedInPhase(getPlayersCurrentPhase());
	setIsUpdating(true);
	int numPhases = getNumberOfPhases(m_curNumBranches);
	for(int i = 0; i < numPhases; ++i)
	{
		char tmp[512];
		sprintf(tmp, "long_title_%s_phase%d", m_parentMediator->getContextRoadmap().c_str(), i);
		StringId phaseTitle(ROADMAP_STRING_FILE, tmp);
		m_phaseTitles[i]->SetLocalText(phaseTitle.localize());
		sprintf(tmp, "title_%s", m_parentMediator->getContextRoadmap().c_str(), i);
		StringId roadmapTitle(ROADMAP_STRING_FILE, tmp);
		m_roadmapTitles[i]->SetLocalText(roadmapTitle.localize());
		int track;
		int phaseOut;
		convertPhaseToTrackAndPhase(i, track, phaseOut);
		sprintf(tmp, "number_title_phase%d", phaseOut);
		StringId phaseNumberId(ROADMAP_STRING_FILE, tmp);
		m_phaseNumbers[i]->SetLocalText(phaseNumberId.localize());		
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::performDeactivate()
{
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::showZoomedInPhase(int phase)
{
	phase = std::max(0, phase);
	m_showingPhase = phase;
	if(m_showingPhase >= TOTAL_NUM_PHASES)
		DEBUG_FATAL(true, ("m_showingPhases is invalid"));
	int i;
	for(i = 0; i < TOTAL_NUM_PHASES; ++i)
		m_zoomedInPages[i]->SetVisible(false);
	m_zoomedInPages[phase]->SetVisible(true);
	m_phaseBackwardButton->SetVisible(m_showingPhase != 0);	
	m_phaseBackwardButton->SetEnabled(m_showingPhase != 0);
	m_phaseForwardButton->SetVisible(!isFinalPhaseOfTrack(m_showingPhase));
	m_phaseForwardButton->SetEnabled(!isFinalPhaseOfTrack(m_showingPhase));
	updateIconStatus();
	m_centerIcon->SetVisible(false);
	m_centerIcon->GetParentWidget()->GetParentWidget()->SetVisible(false);
	m_centerText->SetVisible(false);
	m_centerXPText->SetVisible(false);
	int currentPhase = getPlayersCurrentPhase();
	if(m_showingPhase == getPlayersCurrentPhase())
	{	
		m_parentMediator->setContextSkill(CuiSkillManager::getWorkingSkill());
	}
	else if(m_showingPhase > currentPhase)
	{		
		//Get last skill name
		UIString lastSkillName = Unicode::emptyString;
		UIBaseObject::UIObjectList children;
		m_zoomedInRootPages[m_showingPhase]->GetChildren(children);
		UIBaseObject::UIObjectList::iterator i;
		for(i = children.begin(); i != children.end(); ++i)
		{
			UIBaseObject *curPtr = *i;
			if(curPtr->IsA(TUIPage))
			{
				UIPage *curPage = safe_cast<UIPage *>(curPtr);
				if(curPage->HasProperty(SKILL_NAME))
				{
					curPage->GetProperty(SKILL_NAME, lastSkillName);
				}
			}		
		}
		m_parentMediator->setContextSkill(Unicode::wideToNarrow(lastSkillName));
	}
	else
	{
		//Get first skill name
		UIString firstSkillName = Unicode::emptyString;
		UIBaseObject::UIObjectList children;
		m_zoomedInRootPages[m_showingPhase]->GetChildren(children);
		UIBaseObject::UIObjectList::iterator i;
		bool found = false;
		for(i = children.begin(); !found && (i != children.end()); ++i)
		{
			UIBaseObject *curPtr = *i;
			if(curPtr->IsA(TUIPage))
			{
				UIPage *curPage = safe_cast<UIPage *>(curPtr);
				if(curPage->HasProperty(SKILL_NAME))
				{
					curPage->GetProperty(SKILL_NAME, firstSkillName);
					found = true;
				}
			}		
		}
		m_parentMediator->setContextSkill(Unicode::wideToNarrow(firstSkillName));
	}
	updatePhaseStatus();
}

//----------------------------------------------------------------------

//Returns true if this phase number represents the final phase of a track
bool SwgCuiRoadmap_Profession::isFinalPhaseOfTrack(int phase)
{
	for(int i = 0; i < m_curNumBranches; ++i)
	{
		if(phase == (MAX_NUM_PHASES_IN_EACH_TRACK + i * PHASES_AFTER_BREAK - 1))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

int SwgCuiRoadmap_Profession::getNextPhase(int phase)
{
	int nextPhase = phase + 1;	
	return nextPhase;
}

//----------------------------------------------------------------------

int SwgCuiRoadmap_Profession::getPreviousPhase(int phase)
{
	int nextPhase = phase - 1;	
	return nextPhase;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::OnButtonPressed   (UIWidget * context)
{
	if(context == m_phaseForwardButton)	
	{		
		showZoomedInPhase(getNextPhase(m_showingPhase));	
	}
	else if(context == m_phaseBackwardButton)
	{		
		showZoomedInPhase(getPreviousPhase(m_showingPhase));
	}
	else if(context->IsA(TUIButton))
	{
		UIWidget *contextParent = context->GetParentWidget();
		if(contextParent->HasProperty(SKILL_NAME))
		{
			UIString value;
			contextParent->GetProperty(SKILL_NAME, value);
			m_parentMediator->setContextSkill(Unicode::wideToNarrow(value));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::setCenterSkill(std::string const &skillName)
{
	UIString localizedSkillName;
	CuiSkillManager::localizeSkillName(skillName, localizedSkillName);
	m_centerText->SetPreLocalized(true);
	m_centerText->SetLocalText(localizedSkillName);
	UIColor c;
	RoadmapManager::getCurrentColorForSkill(skillName, c.r, c.g, c.b);
	m_centerText->SetColor(c);

	const std::string & iconPath = RoadmapManager::getIconPathForSkill(skillName);
	UIImageStyle *newStyle = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(iconPath.c_str(), TUIImageStyle));
	m_centerIcon->SetStyle(newStyle);

	//Move the selected box to highlight the selected skill if possible

	m_selectedPage->SetVisible(true);
	
	UIBaseObject::UIObjectList children;
	m_zoomedInRootPages[m_showingPhase]->GetChildren(children);
	UIBaseObject::UIObjectList::iterator i;
	for(i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		if(curPtr->IsA(TUIPage))
		{
			UIPage *curPage = safe_cast<UIPage *>(curPtr);
			if(curPage->HasProperty(SKILL_NAME))
			{
				UIString skillName;
				curPage->GetProperty(SKILL_NAME, skillName);

				const SkillObject * const skill = SkillManager::getInstance ().getSkill (Unicode::wideToNarrow(skillName));
				if(skill)
				{								
					// If this is the working skill, position the highlight bar around it
					if(strcmp(m_parentMediator->getContextSkill().c_str(), Unicode::wideToNarrow(skillName).c_str()) == 0)
					{
						UIPoint curPos = curPage->GetLocation();
						m_selectedPage->SetVisible(true);							
						m_selectedPage->GetParentWidget()->MoveChild(m_selectedPage, UIBaseObject::Top );
						m_selectedPage->SetLocation(curPos.x + HIGHLIGHT_PAGE_OFFSET_X, curPos.y + HIGHLIGHT_PAGE_OFFSET_Y);						
					}					
				}
			}
		}		
	}
}

//----------------------------------------------------------------------

//Go through the currently selected page, if it's a zoomed in view.  Look
// at all the children.  If any of them have the SKILL_NAME property, they
// represent an icon.  We need to do two things - if it's a skill they do not
// have, set the palette of the icon to baselight, otherwise clear the palette.
// If it is their currently working skill, put the highlight page around it.  The
// image is the first child of the first child of the child page.
void SwgCuiRoadmap_Profession::updateIconStatus()
{
	if(m_showingPhase == -1)
		return;
	const CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;
	const PlayerObject * const playerAsPlayer = Game::getPlayerObject();
	if(!playerAsPlayer)
		return;

	
	std::string workingSkillString;
	if(m_currentCategory == SwgCuiRoadmap::TT_GROUND)
	{
		if(RoadmapManager::isLevelBasedTemplate(m_parentMediator->getContextTemplate()))
		{
			m_levelBackgroundImages[m_showingPhase]->SetVisible(true);
			m_levelLabels[m_showingPhase]->SetVisible(true);
			m_backgroundImages[m_showingPhase]->SetVisible(false);
			workingSkillString = RoadmapManager::getNextSkillForLevelTemplate();
		}
		else
		{			
			m_levelBackgroundImages[m_showingPhase]->SetVisible(false);
			m_levelLabels[m_showingPhase]->SetVisible(false);
			m_backgroundImages[m_showingPhase]->SetVisible(true);
			workingSkillString = CuiSkillManager::getWorkingSkill();
		}
	}
	else if(m_currentCategory == SwgCuiRoadmap::TT_SPACE)
	{	
		m_levelBackgroundImages[m_showingPhase]->SetVisible(false);
		m_levelLabels[m_showingPhase]->SetVisible(false);
		m_backgroundImages[m_showingPhase]->SetVisible(true);
		workingSkillString = RoadmapManager::getPlayerSpaceWorkingSkill();
	}
	else if(m_currentCategory == SwgCuiRoadmap::TT_POLITICIAN)
	{	
		m_levelBackgroundImages[m_showingPhase]->SetVisible(false);
		m_levelLabels[m_showingPhase]->SetVisible(false);
		m_backgroundImages[m_showingPhase]->SetVisible(true);
		workingSkillString = RoadmapManager::getPlayerPoliticianWorkingSkill();
	}
	else if(m_currentCategory == SwgCuiRoadmap::TT_STORYTELLER)
	{
		m_levelBackgroundImages[m_showingPhase]->SetVisible(false);
		m_levelLabels[m_showingPhase]->SetVisible(false);
		m_backgroundImages[m_showingPhase]->SetVisible(true);
		workingSkillString = RoadmapManager::getPlayerStorytellerWorkingSkill();
	}

	setCenterSkill(workingSkillString);

	m_highlightPage->SetVisible(false);
	m_selectedPage->SetVisible(false);
	
	m_completedSkillsThisPhase = 0;
	m_uncompletedSkillsThisPhase = 0;
	UIBaseObject::UIObjectList children;
	m_zoomedInRootPages[m_showingPhase]->GetChildren(children);
	UIBaseObject::UIObjectList::iterator i;
	for(i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		if(curPtr->IsA(TUIPage))
		{
			UIPage *curPage = safe_cast<UIPage *>(curPtr);
			if(curPage->HasProperty(SKILL_NAME))
			{
				UIString skillName;
				curPage->GetProperty(SKILL_NAME, skillName);

				const SkillObject * const skill = SkillManager::getInstance ().getSkill (Unicode::wideToNarrow(skillName));
				if(skill)
				{		
					//Get the icon for this skill
					UIImage *icon = NULL;
					UIImage *background = NULL;
					UIBaseObject::UIObjectList subChildren;
					curPage->GetChildren(subChildren);
					if(!subChildren.empty())
					{
						UIBaseObject *subPtr = *(subChildren.begin());
						if(strcmp(subPtr->GetName().c_str(), "SkillButton") == 0)
						{
							UIBaseObject::UIObjectList::iterator tmpI = subChildren.begin();
							tmpI++;
							tmpI++;  //Icon is two down from SkillButton, in between is IconBackground
							subPtr = *tmpI;
							tmpI++;
							background = safe_cast<UIImage *>(*tmpI);
						}
						UIBaseObject::UIObjectList subSubChildren;
						subPtr->GetChildren(subSubChildren);
						if(!subSubChildren.empty())
						{
							UIBaseObject *subSubPtr = *(subSubChildren.begin());
							DEBUG_FATAL(!subSubPtr->IsA(TUIImage), ("fatal in roadmap; someone rearranged the UI pages so that Icon is not two after SkillButton"));
							if(subSubPtr->IsA(TUIImage))
								icon = safe_cast<UIImage *>(subSubPtr);							
						}
					}	
					if(icon)
					{
						// Get whether we have this skill or not					
						icon->SetProperty(s_PalProp, Unicode::emptyString);	
						if(player->hasSkill(*skill))
						{
							m_completedSkillsThisPhase++;
							icon->SetPropertyColor(UIWidget::PropertyName::Color, UIColor::white);
							background->SetPropertyColor(UIWidget::PropertyName::Color, UIColor::white);
							background->SetProperty(s_PalProp, Unicode::narrowToWide("contrast3"));							
						}
						else
						{								
							m_uncompletedSkillsThisPhase++;
							icon->SetPropertyColor(UIWidget::PropertyName::Color, GREY);
							background->SetPropertyColor(UIWidget::PropertyName::Color, UIColor::white);
							background->SetProperty(s_PalProp, Unicode::narrowToWide("contrast3h"));
						}

						// If this is the working skill, position the highlight bar around it
						if(strcmp(workingSkillString.c_str(), Unicode::wideToNarrow(skillName).c_str()) == 0)
						{
							UIPoint curPos = curPage->GetLocation();
							m_highlightPage->SetVisible(true);							
							m_highlightPage->GetParentWidget()->MoveChild(m_highlightPage, UIBaseObject::Top );
							m_highlightPage->SetLocation(curPos.x + HIGHLIGHT_PAGE_OFFSET_X, curPos.y + HIGHLIGHT_PAGE_OFFSET_Y);
							m_parentMediator->setContextSkill(Unicode::wideToNarrow(skillName));													
						}
					}
				}
			}
		}		
	}
	updateChicletStatus();
	updateMainArc();
	if(!(m_currentCategory == SwgCuiRoadmap::TT_GROUND && m_parentMediator->isCurrentTemplateLevelBased()) && m_usingSmallIcons[m_showingPhase])
	{
		m_highlightPage->SetSize(SMALL_HIGHLIGHT_PAGE_SIZE);
		m_selectedPage->SetSize(SMALL_HIGHLIGHT_PAGE_SIZE);
	}
	else
	{
		m_highlightPage->SetSize(BIG_HIGHLIGHT_PAGE_SIZE);
		m_selectedPage->SetSize(BIG_HIGHLIGHT_PAGE_SIZE);
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::updateChicletStatus()
{
	if(m_currentCategory != SwgCuiRoadmap::TT_GROUND)
		return;
	UIBaseObject::UIObjectList children;
	m_zoomedInRootPages[m_showingPhase]->GetChildren(children);
	UIBaseObject::UIObjectList::iterator i;
	
	const CreatureObject * const player = Game::getPlayerCreature();
	int16 playerLevel = player->getLevel();
	for(i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		if(curPtr->IsA(TUIPage))
		{
			UIPage *curPage = safe_cast<UIPage *>(curPtr);
			if(strcmp(curPage->GetName().c_str(), "EmptyChicletPage") == 0)
			{
				if(curPage->HasProperty(LEVEL))
				{
					int level;
					curPage->GetPropertyInteger(LEVEL, level);
					curPage->SetVisible(playerLevel < level);
				}
			}
			else if(strcmp(curPage->GetName().c_str(), "FilledChicletPage") == 0)
			{		
				if(curPage->HasProperty(LEVEL))
				{
					int level;
					curPage->GetPropertyInteger(LEVEL, level);
					curPage->SetVisible(playerLevel >= level);
				}
			}
		}
		else if(curPtr->IsA(TUIText))
		{
			UIText *curText = safe_cast<UIText *>(curPtr);
			if(strcmp(curText->GetName().c_str(), "ChicletText") == 0)
			{		
				if(curText->HasProperty(LEVEL))
				{
					int level;
					curText->GetPropertyInteger(LEVEL, level);
					curText->SetPropertyBoolean(UIText::PropertyName::DropShadow, playerLevel >= level);
					if(playerLevel >= level)
					{
						curText->SetProperty(s_PalProp, Unicode::emptyString);
						curText->SetProperty(UIWidget::PropertyName::Color, Unicode::narrowToWide("#E4C86B"));
					}
					else
					{						
						curText->SetProperty(s_PalProp, Unicode::narrowToWide("contrast3h"));
					}
				}
			}
		}
	}
}
			
//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::updatePhaseStatus()
{
	bool levelBased = m_parentMediator->isCurrentTemplateLevelBased();
	if(levelBased)
		m_playersCurrentPhase = RoadmapManager::getPlayerCurrentPhase();
	else
		m_playersCurrentPhase = 0; //If they have no skills at all they're in phase 0
	int numPhases = getNumberOfPhases(m_curNumBranches);	
	float *phaseProgress = new float[numPhases];

	if(levelBased)
	{
		for(int i = 0; i < numPhases; i++)
		{
			if(i == m_playersCurrentPhase)
				phaseProgress[i] = RoadmapManager::getPlayerXpProgressForCurrentPhase();
			else if(i < m_playersCurrentPhase)
				phaseProgress[i] = 1.0f;
			else
				phaseProgress[i] = 0.0f;
		}
	}
	else
	{	
		for(int i = 0; i < numPhases; i++)
		{
			int completedSkills;
			int uncompletedSkills;
			getCompletedSkillsInPhase(i, completedSkills, uncompletedSkills);
			if(completedSkills == 0)
			{
				phaseProgress[i] = 0.0f;				
			}
			else
			{
				if(uncompletedSkills == 0)		
				{				
					phaseProgress[i] = 1.0f;
				}
				else
				{
					m_playersCurrentPhase = i;
					float val = completedSkills / (completedSkills + uncompletedSkills + 0.0f);
					phaseProgress[i] = val;
				}
			}
		}
	}
	
	if(m_currentCategory == SwgCuiRoadmap::TT_GROUND)
	{	
		int track, phaseOut;
		convertPhaseToTrackAndPhase(m_showingPhase, track, phaseOut);  
		m_parentMediator->configPhaseIconDisplay(MAX_NUM_PHASES_IN_EACH_TRACK, phaseProgress, phaseOut);
	}
	else
		m_parentMediator->disablePhaseIconDisplay();
	delete [] phaseProgress;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::getCompletedSkillsInPhase  (int phase, int &completedSkills, int &uncompletedSkills)
{
	if(phase == -1)
		return;
	const CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;
	const PlayerObject * const playerAsPlayer = Game::getPlayerObject();
	if(!playerAsPlayer)
		return;
	completedSkills = 0;
	uncompletedSkills = 0;
	UIBaseObject::UIObjectList children;
	m_zoomedInRootPages[phase]->GetChildren(children);
	for(UIBaseObject::UIObjectList::iterator i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		if(curPtr->IsA(TUIPage))
		{
			UIPage *curPage = safe_cast<UIPage *>(curPtr);
			if(curPage->HasProperty(SKILL_NAME))
			{
				UIString skillName;
				curPage->GetProperty(SKILL_NAME, skillName);

				const SkillObject * const skill = SkillManager::getInstance ().getSkill (Unicode::wideToNarrow(skillName));
				if(skill)
				{				
					if(player->hasSkill(*skill))
					{
						completedSkills++;
					}
					else
					{								
						uncompletedSkills++;
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::hideBar(UIPage *bar)
{
	bar->GetParentWidget()->GetParentWidget()->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::setProgressBar(UIPage *bar, float val)
{
	bar->GetParentWidget()->GetParentWidget()->SetVisible(true);
	bar->SetWidth(static_cast<unsigned long>(bar->GetParentWidget()->GetWidth() * val));
}

//----------------------------------------------------------------------

int SwgCuiRoadmap_Profession::getPlayersCurrentPhase()
{
	if(m_playersCurrentPhase == -1)
		updatePhaseStatus();
	return m_playersCurrentPhase;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::layoutIconsUsing(UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, 
												const std::string &roadmapName, int phase)
{
	std::vector<std::string> skillVector;
	std::vector<int> xpVector;
	int xpToFirstSkill;
	int track = RoadmapManager::getPlayerBranch(roadmapName, m_parentMediator->getContextTemplate());
	RoadmapManager::getSkillList(roadmapName, track, phase, skillVector, xpVector, xpToFirstSkill);
	layoutIconsUsing(rootPage, samplePage, sampleSmallPage, skillVector, xpVector, xpToFirstSkill);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::layoutIconsUsing(UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, 
												const std::string &templateName)
{	
	std::vector<std::string> skillVector;
	std::vector<int> xpVector;
	int xpToFirstSkill;
	RoadmapManager::getSkillList(templateName, skillVector, xpVector, xpToFirstSkill);
	layoutIconsUsing(rootPage, samplePage, sampleSmallPage, skillVector, xpVector, xpToFirstSkill);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::layoutIconsUsing(UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, 
												const std::vector<std::string> &skillVector,
												const std::vector<int> &xpVector,
												const int xpToFirstSkill)
{
	unsigned int numSkills = skillVector.size();

	erasePreviousLayoutIcons(rootPage);

	//Add in new icons
	int r;
	bool useSmallPage = (numSkills > MAX_NUMBER_SKILLS_BEFORE_USING_SMALL_ICON);
	for(r = 0; r < TOTAL_NUM_PHASES; ++r)
	{
		if(m_zoomedInRootPages[r] == rootPage)
			m_usingSmallIcons[r] = useSmallPage;
	}
	for(r = 0; r < static_cast<int>(numSkills); ++r)
	{
		const std::string & skillName = skillVector[r];
		const std::string & iconPath = RoadmapManager::getIconPathForSkill(skillName);
		
		UIPage *newLayoutPage = NON_NULL (static_cast<UIPage *>((useSmallPage ? sampleSmallPage : samplePage)->DuplicateObject ())); //lint !e1774 //stfu noob
		UIButton *newLayoutPageSkillButton = NON_NULL (GET_UI_OBJ ((*newLayoutPage), UIButton, "SkillButton"));
		UIPage *newLayoutPageIconPage = NON_NULL (GET_UI_OBJ ((*newLayoutPage), UIPage, "Icon"));
		UIImage *newLayoutPageImage = NON_NULL (GET_UI_OBJ ((*newLayoutPageIconPage), UIImage, "Image"));

		rootPage->AddChild(newLayoutPage);
		rootPage->MoveChild( newLayoutPage, UIBaseObject::Top );
		newLayoutPage->Link ();
		newLayoutPage->SetVisible(true);

		newLayoutPage->SetProperty(SKILL_NAME, Unicode::narrowToWide(skillName));

		UIImageStyle *newStyle = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(iconPath.c_str(), TUIImageStyle));
		newLayoutPageImage->SetStyle(newStyle);

		positionLayoutPage(newLayoutPage, r + 1, numSkills + 1);

		newLayoutPage->SetTooltip(getSkillTooltip(skillName));

		newLayoutPage->SetName(UINarrowString("LaidOutPage"));
		registerMediatorObject(*newLayoutPageSkillButton, true);						
	}
	if(m_currentCategory == SwgCuiRoadmap::TT_GROUND)
	{
		for(int level = 2; level < LevelManager::getMaxLevel(); ++level)
		{
			int xpToLevel = LevelManager::getRequiredXpToReachLevel(static_cast<int16>(level));
			if(xpToLevel < xpToFirstSkill)
				continue;
			if(xpToLevel >= xpVector[numSkills - 1])
				break;
			bool found = false;
			for(int s = 0; (s < static_cast<int>(numSkills)) && !found; ++s)
			{
				if(xpToLevel < xpVector[s])
				{
					found = true;
					//This is the right skill
					int prevXp = (s == 0) ? 0 : xpVector[s - 1];
					int xpLeftOver = xpToLevel - prevXp;
					float distance = (xpLeftOver + 0.0f) / (xpVector[s] - prevXp);
					int step = s - 1; // It's OK if step is -1
					{			
						UIPage *newEmptyChicletPage = NON_NULL (static_cast<UIPage *>(m_sampleEmptyChicletPages[0]->DuplicateObject ())); //lint !e1774 //stfu noob
					
						rootPage->AddChild(newEmptyChicletPage);
						rootPage->MoveChild(newEmptyChicletPage, UIBaseObject::Top );
						newEmptyChicletPage->Link ();
						newEmptyChicletPage->SetVisible(true);
						positionChicletPage(newEmptyChicletPage, step, distance, numSkills + 1);
						newEmptyChicletPage->SetName(UINarrowString("EmptyChicletPage"));
						newEmptyChicletPage->SetPropertyInteger(LEVEL, level);

						UIPage *newFilledChicletPage = NON_NULL (static_cast<UIPage *>(m_sampleFilledChicletPages[0]->DuplicateObject ())); //lint !e1774 //stfu noob
					
						rootPage->AddChild(newFilledChicletPage);
						rootPage->MoveChild(newFilledChicletPage, UIBaseObject::Top );
						newFilledChicletPage->Link ();
						newFilledChicletPage->SetVisible(true);
						positionChicletPage(newFilledChicletPage, step, distance, numSkills + 1);
						newFilledChicletPage->SetName(UINarrowString("FilledChicletPage"));
						newFilledChicletPage->SetPropertyInteger(LEVEL, level);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

// -- The following 5 functions are used to position the different visual widgets around the outside
// of the arc. 
void SwgCuiRoadmap_Profession::positionLayoutPage(UIPage *page, int i, int n)
{
	const float WIDTH = 478.0f;
	const float HEIGHT = 478.0f;
	const float XOFFSET = 30.0f;
	const float YOFFSET = 26.0f;
	DEBUG_FATAL(n == 1, ("n can't be 1"));
	float eachAngle = TOTAL_ANGLE / (n - 1);
	float angleInUnits = STARTING_ANGLE + i * eachAngle;
	float angleInRadians = angleInUnits * 2.0f * 3.14159f;
	float actualAngleInRadians = angleInRadians;
	float x = XOFFSET + (WIDTH / 2) + (WIDTH / 2- SKILL_ICON_DISTANCE) * cos(actualAngleInRadians);
	float y = YOFFSET + (HEIGHT / 2) + (HEIGHT / 2- SKILL_ICON_DISTANCE) * sin(actualAngleInRadians);
	page->SetLocation(static_cast<long>(x - page->GetWidth() / 2), static_cast<long>(y - page->GetHeight() / 2));
	return;
}


//----------------------------------------------------------------------

//n is the total number of them around the arc, i is the index of this one, and then c is a float from 0.0 to 1.0
//indicating progress towards the next.  It's really like (i + c) towards n.
void SwgCuiRoadmap_Profession::positionChicletPage(UIPage *page, int i, float c, int n)
{
	const float WIDTH = 484.0f;
	const float HEIGHT = 484.0f;
	const float XOFFSET = 30.0f;
	const float YOFFSET = 26.0f;
	DEBUG_FATAL(n == 1, ("n can't be 1"));
	float eachAngle = TOTAL_ANGLE / (n - 1);
	float angleInUnits = STARTING_ANGLE + (i + c + 1) * eachAngle;
	float angleInRadians = angleInUnits * 2.0f * 3.14159f;
	float actualAngleInRadians = angleInRadians;
	float x = XOFFSET + (WIDTH / 2) + (WIDTH / 2 - CHICLET_ICONS_DISTANCE) * cos(actualAngleInRadians);
	float y = YOFFSET + (HEIGHT / 2) + (HEIGHT / 2 - CHICLET_ICONS_DISTANCE) * sin(actualAngleInRadians);
	page->SetLocation(static_cast<long>(x - page->GetWidth() / 2), static_cast<long>(y - page->GetHeight() / 2));
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::positionChicletText(UIText *page, int i, float c, int n)
{
	const float WIDTH = 484.0f;
	const float HEIGHT = 484.0f;
	const float XOFFSET = 29.0f;
	const float YOFFSET = 26.0f;
	DEBUG_FATAL(n == 1, ("n can't be 1"));
	float eachAngle = TOTAL_ANGLE / (n - 1);
	float angleInUnits = STARTING_ANGLE + (i + c + 1) * eachAngle;
	float angleInRadians = angleInUnits * 2.0f * 3.14159f;
	float actualAngleInRadians = angleInRadians;
	float x = XOFFSET + (WIDTH / 2) + (WIDTH / 2 - CHICLET_ICONS_DISTANCE) * cos(actualAngleInRadians);
	float y = YOFFSET + (HEIGHT / 2) + (HEIGHT / 2 - CHICLET_ICONS_DISTANCE) * sin(actualAngleInRadians);
	page->SetLocation(static_cast<long>(x - page->GetWidth() / 2), static_cast<long>(y - page->GetHeight() / 2));
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::positionChicletLinker(UIImage *page, int i, float c, int n)
{
	const float WIDTH = 484.0f;
	const float HEIGHT = 484.0f;
	const float XOFFSET = 28.0f;
	const float YOFFSET = 24.0f;
	DEBUG_FATAL(n == 1, ("n can't be 1"));
	float eachAngle = TOTAL_ANGLE / (n - 1);
	float angleInUnits = STARTING_ANGLE + (i + c + 1) * eachAngle;
	float angleInRadians = angleInUnits * 2.0f * 3.14159f;
	float actualAngleInRadians = angleInRadians;
	float x = XOFFSET + (WIDTH / 2) + (WIDTH / 2 - CHICLET_LINKER_DISTANCE) * cos(actualAngleInRadians);
	float y = YOFFSET + (HEIGHT / 2) + (HEIGHT / 2 - CHICLET_LINKER_DISTANCE) * sin(actualAngleInRadians);
	page->SetLocation(static_cast<long>(x - page->GetWidth() / 2), static_cast<long>(y - page->GetHeight() / 2));
	page->SetRotation(angleInUnits + 0.25f);
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::positionLayoutLine(UIPage *page, int i, float c, int n)
{
	const float WIDTH = 484.0f;
	const float HEIGHT = 484.0f;
	const float XOFFSET = 29.0f;
	const float YOFFSET = 26.0f;
	DEBUG_FATAL(n == 1, ("n can't be 1"));
	float eachAngle = TOTAL_ANGLE / (n - 1);
	float angleInUnits = STARTING_ANGLE + (i + c + 1) * eachAngle;
	float angleInRadians = angleInUnits * 2.0f * 3.14159f;
	float actualAngleInRadians = angleInRadians;
	float x = XOFFSET + (WIDTH / 2) + (WIDTH / 2 - LINES_DISTANCE) * cos(actualAngleInRadians);
	float y = YOFFSET + (HEIGHT / 2) + (HEIGHT / 2 - LINES_DISTANCE) * sin(actualAngleInRadians);
	page->SetLocation(static_cast<long>(x - page->GetWidth() / 2), static_cast<long>(y - page->GetHeight() / 2));
	page->SetRotation(angleInUnits);
	return;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::updateContextRoadmap()
{
	//Layout all the pages
	std::string const &roadmapName = m_parentMediator->getContextRoadmap();
	m_curNumBranches = RoadmapManager::getNumberOfBranchesInRoadmap(roadmapName);
	if(m_curNumBranches < 1)
	{	
		WARNING(m_curNumBranches < 1, ("error in SwgCuiRoadmap_Profession::updateContextRoadmap: m_curNumBranches is less than 1, %d", m_curNumBranches));
		deactivate();
		return;
	}
	int numPhases = getNumberOfPhases(m_curNumBranches);
	int i;
	for(i = 0; i < numPhases; ++i)
	{
		if(RoadmapManager::isLevelBasedTemplate(m_parentMediator->getContextTemplate()))
		{
			int startLevel, endLevel;
			RoadmapManager::getLevelLimits(i, startLevel, endLevel);
			layoutIconsLevelBased(m_zoomedInRootPages[i], m_sampleIconPage, startLevel, endLevel);
		}
		else
			layoutIconsUsing(m_zoomedInRootPages[i], m_sampleIconPage, m_sampleSmallIconPage, m_parentMediator->getContextRoadmap(), i);		
			
		std::string imageSource(s_buttonImageSource + roadmapName);						
		imageSource += s_buttonSelected;		
		imageSource += s_buttonImageSourcePostfix;
		
		IGNORE_RETURN(m_professionImages[i]->SetSourceResource(Unicode::narrowToWide(imageSource)));		
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::setInnerExperienceBarValue (float value)
{
	if(m_showingPhase < 0)
		return;
	float angle = TOTAL_ANGLE * value;
	UNREF(angle);
	char tmp[512];
	sprintf(tmp, "%4.1f%%", value * 100.0f);
	m_centerXPText->SetLocalText(Unicode::narrowToWide(tmp));
	m_innerXpValue = value;	
	updateChicletStatus();
	updateMainArc();
}

void SwgCuiRoadmap_Profession::SetStorytellerXPBar(int xpCurrent, int xpMax, int previousMax)
{
	if(!m_storytellerExpPage->IsEnabled())
		return;

	char tooltip[256];
	memset(tooltip, 0, 256);
	sprintf(tooltip, "Experience: %d/%d", xpCurrent, xpMax);
	m_storytellerExpBar->SetTooltip(Unicode::narrowToWide(tooltip));
	m_storytellerExpPage->SetTooltip(Unicode::narrowToWide(tooltip));
	int cappedCurrent = xpCurrent > xpMax ? xpMax : xpCurrent;
	float fraction = static_cast<float>(cappedCurrent - previousMax) / static_cast<float>(xpMax - previousMax);
	float newWidth = static_cast<float>(m_storytellerExpBar->GetParentWidget()->GetWidth()) * fraction;
	m_storytellerExpBar->SetWidth( static_cast<UIScalar>(newWidth) );
}

void SwgCuiRoadmap_Profession::HideStorytellerXPBar()
{
	if(m_storytellerExpPage == NULL)
		return;

	m_storytellerExpPage->SetVisible(false);
}
	
//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::setCenterXPTextValue (float value)
{
	char tmp[512];
	sprintf(tmp, "%4.1f%%", value * 100.0f);
	m_centerXPText->SetLocalText(Unicode::narrowToWide(tmp));
}
	
//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::setInnerExperienceBarColor   (UIColor color)
{
	UIColor fullColor(static_cast<unsigned char>(color.r / 2), static_cast<unsigned char>(color.g / 2), static_cast<unsigned char>(color.b / 2));
}

//----------------------------------------------------------------------

int SwgCuiRoadmap_Profession::getShowingPhase()
{
	return m_showingPhase;
}

// Updates the checkboxes and colors (of text and phase displays) based on the branching)
void SwgCuiRoadmap_Profession::updateBranchDisplay()
{
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::convertPhaseToTrackAndPhase   (int phase, int &track, int &phaseOut)
{
	if(phase < PHASE_BREAK)
	{
		track = 0;
		phaseOut = phase;
		return;
	}
	phase -= PHASE_BREAK;
	track = phase / PHASES_AFTER_BREAK;
	phase -= track * PHASES_AFTER_BREAK;
	phase += PHASE_BREAK;
	phaseOut = phase;
}

//----------------------------------------------------------------------

UIString SwgCuiRoadmap_Profession::getSkillTooltip(std::string const &skillName)
{
	UIString result;
	CuiSkillManager::localizeSkillName(skillName, result);

	UIString skillDescription;
	CuiSkillManager::localizeSkillDescription(skillName, skillDescription);
	result.append(Unicode::narrowToWide("\n\n"));
	result.append(skillDescription);
	return result;
}

//----------------------------------------------------------------------

UIString SwgCuiRoadmap_Profession::getPhaseTooltip(std::string const &roadmapName, int phase)
{
	char tmp[512];
	sprintf(tmp, "tooltip_phase_%s_%d", roadmapName.c_str(), phase);
	StringId s(ROADMAP_STRING_FILE, tmp);
	return s.localize();
}

//----------------------------------------------------------------------

UIString SwgCuiRoadmap_Profession::getPhaseTitle(std::string const &roadmapName, int phase)
{
	char tmp[512];
	sprintf(tmp, "long_title_%s_phase%d", roadmapName.c_str(), phase);
	StringId s(ROADMAP_STRING_FILE, tmp);
	return s.localize();
}

//----------------------------------------------------------------------

int SwgCuiRoadmap_Profession::getNumberOfPhases (int numberOfBranches)
{
	UNREF(numberOfBranches);
	return MAX_NUM_PHASES_IN_EACH_TRACK;
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::onSkillsChanged (const CreatureObject & creature)
{
	if (&creature == static_cast<const Object *>(Game::getPlayer ()))
	{		
		if(m_currentCategory == SwgCuiRoadmap::TT_SPACE)
			showCategory(m_currentCategory);
		else if(m_showingPhase != -1)
			showZoomedInPhase(m_showingPhase);
	}	
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::onPlayerExpChanged (const PlayerObject & player)
{
	if (&player == static_cast<const Object *>(Game::getPlayerObject ()))
	{
		updatePhaseStatus();
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::updateMainArc()
{
	float linearFactor;
	if(m_parentMediator->isCurrentTemplateLevelBased() && (m_currentCategory == SwgCuiRoadmap::TT_GROUND))
	{
		if(m_showingPhase == RoadmapManager::getPlayerCurrentPhase())
			linearFactor = RoadmapManager::getPlayerXpProgressForCurrentPhase();
		else if(m_showingPhase < RoadmapManager::getPlayerCurrentPhase())
			linearFactor = 1.0f;
		else
			linearFactor = 0.0f;
	}
	else
	{	
		linearFactor = ((m_completedSkillsThisPhase  + m_parentMediator->getMainlineXpValue()) / (m_completedSkillsThisPhase + m_uncompletedSkillsThisPhase));
	}
	if(linearFactor < 0.0f)
		linearFactor = 0.0f;
	if(linearFactor > 1.0f)
		linearFactor = 1.0f;
	float trueAngle = 6.28f * 0.33f + 6.28f * 0.85f * linearFactor;
	UIPoint needleLocation(static_cast<long>(222.0f + 146.0f * cos(trueAngle)), static_cast<long>(262.0f + 146.0f * sin(trueAngle)));
	m_needlePages[m_showingPhase]->SetLocation(needleLocation);
	m_needlePages[m_showingPhase]->SetPropertyFloat(UIWidget::PropertyName::Rotation, 0.33f + 0.85f * linearFactor);	
	float swooshStartAngle = trueAngle;
	float swooshValue = linearFactor;
	float swooshClipAngle = 6.28f * 0.325f;
	m_swooshes[m_showingPhase]->SetVisible(true);
	m_swooshes[m_showingPhase]->SetPropertyFloat(UIPie::PropertyName::PieAngleClipFinish, swooshClipAngle);
	m_swooshes[m_showingPhase]->SetPropertyFloat(UIPie::PropertyName::PieAngleStart, swooshStartAngle);
	m_swooshes[m_showingPhase]->SetPropertyFloat(UIPie::PropertyName::PieValue, swooshValue);
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::showCategory(SwgCuiRoadmap::TabType category)
{	
	m_currentCategory = category;

	if(category == SwgCuiRoadmap::TT_GROUND)
	{
		m_storytellerExpPage->SetVisible(false);
		m_storytellerExpBar->SetVisible(false);

		showZoomedInPhase(getPlayersCurrentPhase());		
	}
	else if (category == SwgCuiRoadmap::TT_SPACE)
	{
		for(int i = 0; i < TOTAL_NUM_PHASES; ++i)
		{
			m_zoomedInPages[i]->SetVisible(false);
		}
		m_zoomedInPages[FIRST_SPACE_PHASE]->SetVisible(true);
		m_showingPhase = FIRST_SPACE_PHASE;
		layoutIconsUsing(m_zoomedInRootPages[FIRST_SPACE_PHASE], m_sampleIconPage, m_sampleSmallIconPage, m_parentMediator->getContextSpaceTemplate());
		{
			//Get first skill name
			UIString firstSkillName = Unicode::emptyString;
			UIBaseObject::UIObjectList children;
			m_zoomedInRootPages[FIRST_SPACE_PHASE]->GetChildren(children);
			UIBaseObject::UIObjectList::iterator i;
			bool found = false;
			for(i = children.begin(); !found && (i != children.end()); ++i)
			{
				UIBaseObject *curPtr = *i;
				if(curPtr->IsA(TUIPage))
				{
					UIPage *curPage = safe_cast<UIPage *>(curPtr);
					if(curPage->HasProperty(SKILL_NAME))
					{
						curPage->GetProperty(SKILL_NAME, firstSkillName);
						found = true;
					}
				}		
			}
			m_parentMediator->setContextSkill(Unicode::wideToNarrow(firstSkillName));
		}
		updateIconStatus();
		char tmp[512];
		sprintf(tmp, "template_title_%s", m_parentMediator->getContextSpaceTemplate().c_str());
		StringId templateTitle(ROADMAP_STRING_FILE, tmp);
		m_roadmapTitles[FIRST_SPACE_PHASE]->SetLocalText(Unicode::emptyString);
		m_phaseTitles[FIRST_SPACE_PHASE]->SetLocalText(templateTitle.localize());
		m_phaseBackwardButton->SetVisible(false);
		m_phaseBackwardButton->SetEnabled(false);
		m_phaseForwardButton->SetVisible(false);
		m_phaseForwardButton->SetEnabled(false);
		m_storytellerExpPage->SetVisible(false);
		m_storytellerExpBar->SetVisible(false);
		m_parentMediator->disablePhaseIconDisplay();

		std::string imageSource(s_buttonImageSource + m_parentMediator->getContextSpaceTemplate());		
		IGNORE_RETURN(m_professionImages[FIRST_SPACE_PHASE]->SetSourceResource(Unicode::narrowToWide(imageSource)));
		m_phasePages[FIRST_SPACE_PHASE]->SetVisible(false);
	}
	else if(category == SwgCuiRoadmap::TT_POLITICIAN)
	{
		for(int i = 0; i < TOTAL_NUM_PHASES; ++i)
		{
			m_zoomedInPages[i]->SetVisible(false);
		}

		m_zoomedInPages[FIRST_POLITICIAN_PHASE]->SetVisible(true);
		m_showingPhase = FIRST_POLITICIAN_PHASE;
		layoutIconsUsing(m_zoomedInRootPages[FIRST_POLITICIAN_PHASE], m_sampleIconPage, m_sampleSmallIconPage, m_parentMediator->getPoliticianTemplate());
		{
			//Get first skill name
			UIString firstSkillName = Unicode::emptyString;
			UIBaseObject::UIObjectList children;
			m_zoomedInRootPages[FIRST_POLITICIAN_PHASE]->GetChildren(children);
			UIBaseObject::UIObjectList::iterator i;
			bool found = false;
			for(i = children.begin(); !found && (i != children.end()); ++i)
			{
				UIBaseObject *curPtr = *i;
				if(curPtr->IsA(TUIPage))
				{
					UIPage *curPage = safe_cast<UIPage *>(curPtr);
					if(curPage->HasProperty(SKILL_NAME))
					{
						curPage->GetProperty(SKILL_NAME, firstSkillName);
						found = true;
					}
				}		
			}
			m_parentMediator->setContextSkill(Unicode::wideToNarrow(firstSkillName));
		}
		updateIconStatus();
		StringId templateTitle(ROADMAP_STRING_FILE, "template_title_politician");
		m_roadmapTitles[FIRST_POLITICIAN_PHASE]->SetLocalText(Unicode::emptyString);
		m_phaseTitles[FIRST_POLITICIAN_PHASE]->SetLocalText(templateTitle.localize());
		m_phaseBackwardButton->SetVisible(false);
		m_phaseBackwardButton->SetEnabled(false);
		m_phaseForwardButton->SetVisible(false);
		m_phaseForwardButton->SetEnabled(false);
		m_storytellerExpPage->SetVisible(false);
		m_storytellerExpBar->SetVisible(false);
		m_parentMediator->disablePhaseIconDisplay();

		std::string imageSource(s_buttonImageSource + s_politicianButtonImage);		
		IGNORE_RETURN(m_professionImages[FIRST_POLITICIAN_PHASE]->SetSourceResource(Unicode::narrowToWide(imageSource)));
		m_phasePages[FIRST_POLITICIAN_PHASE]->SetVisible(false);
	}
	else if(category == SwgCuiRoadmap::TT_STORYTELLER)
	{
		for(int i = 0; i < TOTAL_NUM_PHASES; ++i)
		{
			m_zoomedInPages[i]->SetVisible(false);
		}

		m_zoomedInPages[FIRST_STORYTELLER_PHASE]->SetVisible(true);
		m_showingPhase = FIRST_STORYTELLER_PHASE;
		layoutIconsUsing(m_zoomedInRootPages[FIRST_STORYTELLER_PHASE], m_sampleIconPage, m_sampleSmallIconPage, m_parentMediator->getStorytellerTemplate());
		{
			//Get first skill name
			UIString firstSkillName = Unicode::emptyString;
			UIBaseObject::UIObjectList children;
			m_zoomedInRootPages[FIRST_STORYTELLER_PHASE]->GetChildren(children);
			UIBaseObject::UIObjectList::iterator i;
			bool found = false;
			for(i = children.begin(); !found && (i != children.end()); ++i)
			{
				UIBaseObject *curPtr = *i;
				if(curPtr->IsA(TUIPage))
				{
					UIPage *curPage = safe_cast<UIPage *>(curPtr);
					if(curPage->HasProperty(SKILL_NAME))
					{
						curPage->GetProperty(SKILL_NAME, firstSkillName);
						found = true;
					}
				}		
			}
			m_parentMediator->setContextSkill(Unicode::wideToNarrow(firstSkillName));
		}
		updateIconStatus();
		StringId templateTitle(ROADMAP_STRING_FILE, "template_title_storyteller");
		m_roadmapTitles[FIRST_STORYTELLER_PHASE]->SetLocalText(Unicode::emptyString);
		m_phaseTitles[FIRST_STORYTELLER_PHASE]->SetLocalText(templateTitle.localize());
		m_phaseBackwardButton->SetVisible(false);
		m_phaseBackwardButton->SetEnabled(false);
		m_phaseForwardButton->SetVisible(false);
		m_phaseForwardButton->SetEnabled(false);
		m_storytellerExpPage->SetVisible(true);
		m_storytellerExpBar->SetVisible(true);
		m_parentMediator->disablePhaseIconDisplay();

		std::string imageSource(s_buttonImageSource + s_storytellerButtonImage);		
		IGNORE_RETURN(m_professionImages[FIRST_STORYTELLER_PHASE]->SetSourceResource(Unicode::narrowToWide(imageSource)));
		m_phasePages[FIRST_STORYTELLER_PHASE]->SetVisible(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::layoutIconsLevelBased(UIPage *rootPage, UIPage *samplePage, const int startLevel, const int endLevel)
{
	erasePreviousLayoutIcons(rootPage);

	//Add in level chiclets
	DEBUG_FATAL(m_currentCategory != SwgCuiRoadmap::TT_GROUND, ("can't layout level based except for ground yet"));

	for(int level = startLevel + 1; level <= endLevel; ++level)
	{					
		UIPage *newLine = NON_NULL (static_cast<UIPage *>(m_sampleLinePage->DuplicateObject ()));

		rootPage->AddChild(newLine);
		rootPage->MoveChild(newLine, UIBaseObject::Bottom);
		newLine->Link ();
		newLine->SetVisible(true);
		positionLayoutLine(newLine, level - startLevel - 1, 0.0f, endLevel - startLevel + 1);
		newLine->SetName(UINarrowString("LayoutLine"));

		UIPage *newEmptyChicletPage = NON_NULL (static_cast<UIPage *>(m_sampleEmptyChicletPages[0]->DuplicateObject ())); //lint !e1774 //stfu noob
	
		rootPage->AddChild(newEmptyChicletPage);
		rootPage->MoveChild(newEmptyChicletPage, UIBaseObject::Top );
		newEmptyChicletPage->Link ();
		newEmptyChicletPage->SetVisible(true);
		positionChicletPage(newEmptyChicletPage, level - startLevel - 1, 0.0f, endLevel - startLevel + 1);
		newEmptyChicletPage->SetName(UINarrowString("EmptyChicletPage"));
		newEmptyChicletPage->SetPropertyInteger(LEVEL, level);

		UIPage *newFilledChicletPage = NON_NULL (static_cast<UIPage *>(m_sampleFilledChicletPages[0]->DuplicateObject ())); //lint !e1774 //stfu noob
	
		rootPage->AddChild(newFilledChicletPage);
		rootPage->MoveChild(newFilledChicletPage, UIBaseObject::Top );
		newFilledChicletPage->Link ();
		newFilledChicletPage->SetVisible(true);
		positionChicletPage(newFilledChicletPage, level - startLevel - 1, 0.0f, endLevel - startLevel + 1);
		newFilledChicletPage->SetName(UINarrowString("FilledChicletPage"));
		newFilledChicletPage->SetPropertyInteger(LEVEL, level);

		UIText *newChicletText = NON_NULL (static_cast<UIText *>(m_sampleChicletTextPage->DuplicateObject ()));

		rootPage->AddChild(newChicletText);
		rootPage->MoveChild(newChicletText, UIBaseObject::Top);
		newChicletText->Link ();
		newChicletText->SetVisible(true);
		positionChicletText(newChicletText, level - startLevel - 1, 0.0f, endLevel - startLevel + 1);
		newChicletText->SetName(UINarrowString("ChicletText"));
		newChicletText->SetPropertyInteger(LEVEL, level);
		char tmp[512];
		sprintf(tmp, "%d", level);
		newChicletText->SetText(Unicode::narrowToWide(tmp));

		std::string const &skillName = RoadmapManager::getSkillForLevel(m_parentMediator->getContextTemplate(), level);
		if(!skillName.empty())
		{
			const std::string & iconPath = RoadmapManager::getIconPathForSkill(skillName);
			
			UIPage *newLayoutPage = NON_NULL (static_cast<UIPage *>(samplePage->DuplicateObject ())); //lint !e1774 //stfu noob
			UIButton *newLayoutPageSkillButton = NON_NULL (GET_UI_OBJ ((*newLayoutPage), UIButton, "SkillButton"));
			UIPage *newLayoutPageIconPage = NON_NULL (GET_UI_OBJ ((*newLayoutPage), UIPage, "Icon"));
			UIImage *newLayoutPageImage = NON_NULL (GET_UI_OBJ ((*newLayoutPageIconPage), UIImage, "Image"));

			rootPage->AddChild(newLayoutPage);
			rootPage->MoveChild( newLayoutPage, UIBaseObject::Top );
			newLayoutPage->Link ();
			newLayoutPage->SetVisible(true);

			newLayoutPage->SetProperty(SKILL_NAME, Unicode::narrowToWide(skillName));

			UIImageStyle *newStyle = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(iconPath.c_str(), TUIImageStyle));
			newLayoutPageImage->SetStyle(newStyle);

			positionLayoutPage(newLayoutPage, level - startLevel, endLevel - startLevel + 1);

			newLayoutPage->SetTooltip(getSkillTooltip(skillName));

			newLayoutPage->SetName(UINarrowString("LaidOutPage"));
			registerMediatorObject(*newLayoutPageSkillButton, true);	

			UIImage *newChicletLinker = NON_NULL (static_cast<UIImage *>(m_sampleChicletLinkerImage->DuplicateObject ()));

			rootPage->AddChild(newChicletLinker);
			rootPage->MoveChild(newChicletLinker, UIBaseObject::Bottom);
			newChicletLinker->Link ();
			newChicletLinker->SetVisible(true);
			positionChicletLinker(newChicletLinker, level - startLevel - 1, 0.0f, endLevel - startLevel + 1);
			newChicletLinker->SetName(UINarrowString("ChicletLinker"));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiRoadmap_Profession::erasePreviousLayoutIcons(UIPage *rootPage)
{
	//Clear all previously laid out icons
	UIBaseObject::UIObjectList children;
	rootPage->GetChildren(children);
	UIBaseObject::UIObjectList childrenToRemove;
	UIBaseObject::UIObjectList::iterator i;
	for(i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		if( (strcmp(curPtr->GetName().c_str(), "LaidOutPage") == 0) ||
			(strcmp(curPtr->GetName().c_str(), "EmptyChicletPage") == 0) ||
			(strcmp(curPtr->GetName().c_str(), "FilledChicletPage") == 0)||
			(strcmp(curPtr->GetName().c_str(), "ChicletText") == 0) ||
			(strcmp(curPtr->GetName().c_str(), "ChicletLinker") == 0) ||
			(strcmp(curPtr->GetName().c_str(), "LayoutLine") == 0) )
		{
			childrenToRemove.push_back(curPtr);
		}						
	}
	for(i = childrenToRemove.begin(); i != childrenToRemove.end(); ++i)
	{
		UIBaseObject *curPtr = *i;
		rootPage->RemoveChild(curPtr);						
	}
}
