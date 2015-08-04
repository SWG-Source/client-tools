//======================================================================
//
// SwgCuiExpertise.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiExpertise.h"

#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedSkillSystem/ExpertiseManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIEllipse.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UITabbedPane.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiExpertiseNamespace
{
	std::string const cs_componentImageStyleNamespace("/Styles.Expertise.component");
	std::string const cs_backgroundImageStyleNamespace("/Styles.Expertise.background");

	UILowerString const cs_gridPropertySkillName = UILowerString("SkillName");
	UILowerString const cs_gridPropertyX = UILowerString("GridX");
	UILowerString const cs_gridPropertyY = UILowerString("GridY");

	void setTextToNumber(UIText * uiText, int number);
	void setTextToRankNumbers(UIText * uiText, int spentRank, int rank, int numberMode);

	UIString getExpertiseTooltip(std::string const & skillName);

	int getTreeIdForTab(long tab);

	std::string s_emptyString;

	StringId s_noExpertiseTitle("expertise_n", "main_title");
	StringId s_noExpertiseDescription("expertise_d", "empty_description");

	const int DARK_LEVEL = 92;

	StringId s_rank("expertise_n", "rank_title");

	StringId s_requiresTag("expertise_n", "requires_tag");
	StringId s_pointsIn("expertise_n", "points_in");
	StringId s_pointIn("expertise_n", "point_in");
	StringId s_none("expertise_n", "none");

	const UIColor s_grey(154, 154, 154);
	const UIColor s_yellow(255, 251, 85);


}

using namespace SwgCuiExpertiseNamespace;

//======================================================================

void SwgCuiExpertiseNamespace::setTextToNumber(UIText * uiText, int number)
{
	uiText->SetPreLocalized(true);
	uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("%d", number)));
}

void SwgCuiExpertiseNamespace::setTextToRankNumbers(UIText * uiText, int spentRank, int rank, int numberMode)
{
	uiText->SetPreLocalized(true);
	UIEllipse   * bkgrd1 =  NON_NULL(GET_UI_OBJ((*uiText->GetParentWidget()), UIEllipse, "bkgrd1"));
	UIPage   * bkgrd2 =  NON_NULL(GET_UI_OBJ((*uiText->GetParentWidget()), UIPage, "bkgrd2"));
	uiText->SetDropShadow(true);
	if(numberMode == 2)
	{
		bkgrd1->SetVisible(true);
		bkgrd2->SetVisible(false);
		if(rank > spentRank)
		{
			bkgrd1->SetColor(s_yellow);
			bkgrd1->SetInnerRadiusColor(s_yellow);
			uiText->SetDropShadow(false);
			uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#000000%d", rank)));
		}
		else
		{
			bkgrd1->SetColor(UIColor::black);
			bkgrd1->SetInnerRadiusColor(UIColor::black);
			uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#CECECE%d", rank)));
		}
	}
	else if(numberMode == 1)
	{
		bkgrd1->SetVisible(false);
		bkgrd2->SetVisible(true);
		if(spentRank > 0)
		{
			if(rank > spentRank)
				uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#CECECE%d \\#CECECE(\\#FFFB55%d\\#CECECE)", spentRank, rank)));
			else
				uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#CECECE%d", spentRank)));
		}
		else
			uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#CECECE(\\#FFFB55%d\\#CECECE)", rank)));
	}
	else if (numberMode == 0)
	{		
		bkgrd1->SetVisible(true);
		bkgrd2->SetVisible(false);
		if(rank > spentRank)
			uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#FFFB55%d", rank)));
		else
			uiText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#CECECE%d", rank)));
	}
}

//----------------------------------------------------------------------

UIString SwgCuiExpertiseNamespace::getExpertiseTooltip(std::string const & skillName)
{
	UIString result;
	CuiSkillManager::localizeSkillName(skillName, result);

	int const rankCurrent = ClientExpertiseManager::getExpertiseRankForPlayer(skillName);
	int const rankMax     = ExpertiseManager::getExpertiseRankMax(skillName);
	
	result.append(Unicode::narrowToWide(" ("));
	char buffer[64];
	_itoa(rankCurrent, buffer, 10);
	result.append(Unicode::narrowToWide(buffer));
	result.append(Unicode::narrowToWide("/"));
	_itoa(rankMax, buffer, 10);
	result.append(Unicode::narrowToWide(buffer));
	result.append(Unicode::narrowToWide(")\n"));

	UIString skillDescription;
	CuiSkillManager::localizeSkillDescription(skillName, skillDescription);
	result.append(Unicode::narrowToWide("\n\n"));
	result.append(skillDescription);

	return result;
}

//----------------------------------------------------------------------

int SwgCuiExpertiseNamespace::getTreeIdForTab(long tab)
{
	int result = 0;

	ClientExpertiseManager::TreeIdList treeIdList = ClientExpertiseManager::getExpertiseTreesForPlayer();

	if(treeIdList.empty())
		return 0;

	// bounds check
	if (tab >= 0 || static_cast<ClientExpertiseManager::TreeIdList::size_type>(tab) < treeIdList.size())
	{
		result = treeIdList[tab];
	}

	return result;
}

//======================================================================

SwgCuiExpertise::SwgCuiExpertise (UIPage & page) :
CuiMediator("SwgCuiExpertise", page),
UIEventCallback(),
MessageDispatch::Receiver(),
m_treeTabs(0),
m_currentTab(0),
m_acceptButton(0),
m_spentPointsText(0),
m_allocatedPointsText(0),
m_clearTreeButton(0),
m_clearAllButton(0),
m_switchNumbersButton(0),
m_treeGrid(0),
m_sampleIconPage(0),
m_sampleComponentPage(0),
m_callback(new MessageDispatch::Callback),
m_lastExpertiseClickedOn(0),
m_expertiseDisplayName(0),
m_expertiseDisplayDescription(0),
m_expertiseDisplayRequirements(0),
m_currentSkillName(),
m_currentBaseSkillName(),
m_currentRankText(0),
m_mainIconImage(0),
m_commandType(0),
m_commandTypeIcon(0),
m_commandTypeName(0),
m_commandTypeDescription(0),
m_schematicType(0),
m_skillModType(0),
m_numberMode(2),
m_messageBoxTrainExpertises(0),
m_treeBackgroundImage(0)
{
	getCodeDataObject(TUIImage, m_treeBackgroundImage, "BackgroundLeft");

	getCodeDataObject(TUITabbedPane, m_treeTabs, "TreeTabs");
	registerMediatorObject(*m_treeTabs, true);

	getCodeDataObject(TUIText, m_spentPointsText, "SpentPoints");

	getCodeDataObject(TUIText, m_allocatedPointsText, "AllocatedPoints");

	getCodeDataObject(TUIButton, m_acceptButton, "AcceptButton");
	registerMediatorObject(*m_acceptButton, true);

	getCodeDataObject(TUIButton, m_clearTreeButton, "ClearTreeButton");
	registerMediatorObject(*m_clearTreeButton, true);

	getCodeDataObject(TUIButton, m_clearAllButton, "ClearAllButton");
	registerMediatorObject(*m_clearAllButton, true);

	getCodeDataObject(TUIButton, m_switchNumbersButton, "SwitchNumbersButton");
	registerMediatorObject(*m_switchNumbersButton, true);

	getCodeDataObject(TUIPage, m_sampleIconPage, "sampleIconPage");
	m_sampleIconPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_sampleComponentPage, "sampleComponentPage");
	m_sampleComponentPage->SetVisible(false);

	getCodeDataObject(TUIText, m_expertiseDisplayName, "ExpertiseDisplayName");
	m_expertiseDisplayName->SetPreLocalized(true);

	getCodeDataObject(TUIText, m_expertiseDisplayDescription, "ExpertiseDisplayDescription");
	m_expertiseDisplayDescription->SetPreLocalized(true);

	getCodeDataObject(TUIText, m_expertiseDisplayRequirements, "RequiresText");
	m_expertiseDisplayRequirements->SetPreLocalized(true);

	getCodeDataObject(TUIText, m_currentRankText, "CurrentRank");
	m_currentRankText->SetPreLocalized(true);

	getCodeDataObject(TUIPage, m_commandType, "CommandType");
	m_commandType->SetVisible(false);

	getCodeDataObject(TUIPage, m_schematicType, "SchematicType");
	m_commandType->SetVisible(false);

	getCodeDataObject(TUIPage, m_skillModType, "SkillModType");
	m_commandType->SetVisible(false);

	getCodeDataObject(TUIImage, m_mainIconImage, "MainIconImage");

	getCodeDataObject(TUIImage, m_commandTypeIcon, "CommandType_Icon");
	getCodeDataObject(TUIText, m_commandTypeName, "CommandType_Name");
	m_commandTypeName->SetPreLocalized(true);
	getCodeDataObject(TUIText, m_commandTypeDescription, "CommandType_Description");
	m_commandTypeDescription->SetPreLocalized(true);

	getCodeDataObject(TUIPage, m_treeGrid, "TreeGrid");

	for (int x = 1; x <= ExpertiseManager::getNumExpertiseColumns(); ++x)
	{
		for (int y = 1; y <= ExpertiseManager::getNumExpertiseTiers(); ++y)
		{
			UIPage * box;

			getCodeDataObject(TUIPage, box, FormattedString<128>().sprintf("Grid_%d_%d", x, y));

			// Store which coordinate each box sits at.
			box->SetPropertyInteger(cs_gridPropertyX, x);
			box->SetPropertyInteger(cs_gridPropertyY, y);
		}
	}

	for(int i = 0; i < MAX_NUM_SKILL_MODS_PER_EXPERTISE ; ++i)
	{
		char tmp[512];
		sprintf(tmp, "SkillModType_Main_%d", i + 1);
		getCodeDataObject(TUIPage, m_skillModTypeMain[i], tmp);
		sprintf(tmp, "SkillModType_Name_%d", i + 1);
		getCodeDataObject(TUIText, m_skillModTypeNames[i], tmp);
		m_skillModTypeNames[i]->SetPreLocalized(true);
		sprintf(tmp, "SkillModType_Description_%d", i + 1);
		getCodeDataObject(TUIText, m_skillModTypeDescriptions[i], tmp);
		m_skillModTypeDescriptions[i]->SetPreLocalized(true);
		for(int j = 0; j < MAX_NUM_EXPERTISE_RANKS; ++j)
		{
			sprintf(tmp, "SkillModType_Amount_%d_%d", i + 1, j + 1);
			getCodeDataObject(TUIText, m_skillModTypeAmounts[i][j], tmp);
			m_skillModTypeAmounts[i][j]->SetPreLocalized(true);

			sprintf(tmp, "SkillModType_Marker_%d_%d", i + 1, j + 1);
			getCodeDataObject(TUIPage, m_skillModTypeMarkers[i][j], tmp);
			m_skillModTypeMarkers[i][j]->SetVisible(false);
		}
	}

	ClientExpertiseManager::clearAllocatedExpertises();
	m_currentSkillName = s_emptyString;
	m_currentBaseSkillName = s_emptyString;
	m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
	m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());

	m_commandType->SetVisible(false);
	m_skillModType->SetVisible(false);
	m_schematicType->SetVisible(false);

	m_expertiseDisplayRequirements->Clear();
	m_currentRankText->Clear();

	setState(MS_closeable);
}


//----------------------------------------------------------------------

SwgCuiExpertise::~SwgCuiExpertise ()
{
	m_lastExpertiseClickedOn = 0;
	m_currentSkillName = s_emptyString;
	m_currentBaseSkillName = s_emptyString;
	delete m_callback;
	m_callback = 0;
	m_messageBoxTrainExpertises = 0;

	deactivate();
}

//----------------------------------------------------------------------

bool SwgCuiExpertise::OnMessage(UIWidget * context, UIMessage const & msg)
{
	UNREF(context);
	UNREF(msg);
	//Process right-clicks on buttons
	if (context->IsA(TUIButton))
	{
		if(context->HasProperty(cs_gridPropertySkillName))
		{
			if(msg.Type == UIMessage::RightMouseUp)
			{
				CuiSoundManager::play (CuiSounds::negative);
				onSkillIconClicked(context, true);
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------

void SwgCuiExpertise::receiveMessage(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message)
{
	UNREF(message);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::performActivate()
{
	CuiManager::requestPointer(true); 
	setIsUpdating(true);

	PlayerObject const * player = Game::getPlayerObject();
	CreatureObject const * playerCreature = Game::getPlayerCreature();
	if(!player || !playerCreature)	
	{
		deactivate();
		return;		
	}

	if(!ClientExpertiseManager::hasExpertiseTrees())
	{
		deactivate();
		return;
	}

	displayExpertisePoints();
	displayExpertiseTabs();
	displayExpertiseTree();
	updateButtonStatus();

	updateExpertiseDisplayName();
	updateExpertiseDescriptionSection();
	updateExpertiseRequirementsSection();
	updateButtonStatus();
	updateExpertiseIconHighlighting();

	m_callback->connect(*this, &SwgCuiExpertise::onLevelChanged,  static_cast<CreatureObject::Messages::LevelChanged *>(0));
	m_callback->connect(*this, &SwgCuiExpertise::onSkillsChanged, static_cast<CreatureObject::Messages::SkillsChanged *>(0));
	m_callback->connect(*this, &SwgCuiExpertise::onSkillTemplateChanged, static_cast<CuiSkillManager::Messages::ProfessionTemplateChanged *>(0));
}

//----------------------------------------------------------------------

void SwgCuiExpertise::performDeactivate()
{
	if (m_messageBoxTrainExpertises)
	{
		m_callback->disconnect(m_messageBoxTrainExpertises->getTransceiverClosed(), *this, &SwgCuiExpertise::onMessageBoxClosed);
		m_messageBoxTrainExpertises->closeMessageBox();
	}

	m_lastExpertiseClickedOn = 0;
	CuiManager::requestPointer(false); 
	setIsUpdating(false);	
	m_callback->disconnect(*this, &SwgCuiExpertise::onLevelChanged,  static_cast<CreatureObject::Messages::LevelChanged *>(0));	
	m_callback->disconnect(*this, &SwgCuiExpertise::onSkillsChanged, static_cast<CreatureObject::Messages::SkillsChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiExpertise::onSkillTemplateChanged, static_cast<CuiSkillManager::Messages::ProfessionTemplateChanged *>(0));	
}

//----------------------------------------------------------------------

void SwgCuiExpertise::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::OnTabbedPaneChanged(UIWidget * context)
{
	if(context == m_treeTabs)
	{
		m_lastExpertiseClickedOn = 0;
		m_currentSkillName = s_emptyString;
		m_currentBaseSkillName = s_emptyString;
		m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
		m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());
		m_currentTab = m_treeTabs->GetActiveTab();
		displayExpertiseTree();

		m_commandType->SetVisible(false);
		m_skillModType->SetVisible(false);
		m_schematicType->SetVisible(false);

		m_expertiseDisplayRequirements->Clear();
		m_currentRankText->Clear();
		updateButtonStatus();
	}
}
	
//----------------------------------------------------------------------

void SwgCuiExpertise::OnButtonPressed(UIWidget * context)
{	
	if (context == m_acceptButton)
	{
		if(ClientExpertiseManager::getExpertisePointsAllocatedForPlayer() > 1)
			m_messageBoxTrainExpertises = CuiMessageBox::createYesNoBox(CuiStringIds::confirm_train_expertises.localize());
		else
			m_messageBoxTrainExpertises = CuiMessageBox::createYesNoBox(CuiStringIds::confirm_train_expertise.localize());
		m_callback->connect(m_messageBoxTrainExpertises->getTransceiverClosed(), *this, &SwgCuiExpertise::onMessageBoxClosed);
	}
	else if (context == m_clearTreeButton)
	{
		int tree = getTreeIdForTab(m_currentTab);
		ClientExpertiseManager::clearAllocatedExpertisesInTree(tree);
		m_lastExpertiseClickedOn = 0;

		m_currentSkillName = s_emptyString;
		m_currentBaseSkillName = s_emptyString;
		m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
		m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());
		m_commandType->SetVisible(false);
		m_skillModType->SetVisible(false);
		m_schematicType->SetVisible(false);
		m_expertiseDisplayRequirements->Clear();
		m_currentRankText->Clear();
		displayExpertiseTree();
		displayExpertisePoints();
		updateButtonStatus();
		updateExpertiseIconHighlighting();
	}
	else if (context == m_clearAllButton)
	{
		ClientExpertiseManager::clearAllocatedExpertises();
		m_lastExpertiseClickedOn = 0;

		m_currentSkillName = s_emptyString;
		m_currentBaseSkillName = s_emptyString;
		m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
		m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());
		m_commandType->SetVisible(false);
		m_skillModType->SetVisible(false);
		m_schematicType->SetVisible(false);
		m_expertiseDisplayRequirements->Clear();
		m_currentRankText->Clear();
		displayExpertiseTree();
		displayExpertisePoints();
		updateButtonStatus();
		updateExpertiseIconHighlighting();
	}
	else if (context == m_switchNumbersButton)
	{
		m_numberMode++;
		if(m_numberMode == 3) m_numberMode = 0;
		m_lastExpertiseClickedOn = 0;
		displayExpertiseTree();
		displayExpertisePoints();
	}
	else if (context->IsA(TUIButton))
	{
		if(context->HasProperty(cs_gridPropertySkillName))
		{
			CuiSoundManager::play (CuiSounds::select_popup);
			onSkillIconClicked(context, false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::onLevelChanged(CreatureObject const & creature)
{
	if (&creature == static_cast<Object const *>(Game::getPlayer()))
	{
		displayExpertisePoints();
	}	
}

//----------------------------------------------------------------------

void SwgCuiExpertise::onSkillsChanged(CreatureObject const & creature)
{
	if (&creature == static_cast<Object const *>(Game::getPlayer()))
	{		
		m_lastExpertiseClickedOn = 0;
		displayExpertiseTree();
		displayExpertisePoints();
		updateButtonStatus();
		updateExpertiseIconHighlighting();
	}	
}

//----------------------------------------------------------------------

void SwgCuiExpertise::onSkillTemplateChanged(const std::string &)
{
	deactivate();
}

//======================================================================

void SwgCuiExpertise::displayExpertisePoints()
{
	setTextToNumber(m_spentPointsText,     ClientExpertiseManager::getExpertisePointsSpentForPlayer() - ClientExpertiseManager::getExpertisePointsAllocatedForPlayer());
	int points = ClientExpertiseManager::getExpertisePointsRemainingForPlayer();
	int totalPoints = ClientExpertiseManager::getExpertisePointsAllocatedForPlayer() + points;
	if(points == 0)
		m_allocatedPointsText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#FF0000%d\\#. / %d", points, totalPoints)));
	else
		m_allocatedPointsText->SetLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("\\#.%d / %d", points, totalPoints)));
}

//----------------------------------------------------------------------

void SwgCuiExpertise::displayExpertiseTabs()
{
	m_lastExpertiseClickedOn = 0;
	m_treeTabs->Clear();

	ClientExpertiseManager::TreeIdList treeIdList = ClientExpertiseManager::getExpertiseTreesForPlayer();

	for (ClientExpertiseManager::TreeIdList::const_iterator i = treeIdList.begin(); i != treeIdList.end(); ++i)
	{
		UIString treeName;
		ClientExpertiseManager::localizeExpertiseTreeNameFromId(*i, treeName);
		m_treeTabs->AppendTab(treeName, NULL);	
	}
	int indx = 0;
	for (ClientExpertiseManager::TreeIdList::const_iterator i = treeIdList.begin(); i != treeIdList.end(); ++i)
	{	
		UIString treeDescription;
		ClientExpertiseManager::localizeExpertiseTreeDescriptionFromId(*i, treeDescription);
		UIButton *tabButton = m_treeTabs->GetTabButton(indx);	
		tabButton->SetLocalTooltip(treeDescription);
		++indx;
	}

	// preserve previous active tab (if still valid)
	if (m_currentTab < m_treeTabs->GetTabCount())
	{
		m_treeTabs->SetActiveTab(m_currentTab);
	}
	else
	{
		m_currentTab = m_treeTabs->GetActiveTab();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateExpertiseIconHighlighting()
{
	CreatureObject const * player = Game::getPlayerCreature();
	if (!player)
	{
		return;
	}

	int tree = getTreeIdForTab(m_currentTab);
	int tier = 0;
	int grid = 0;

	UIBaseObject::UIObjectList boxes;
	m_treeGrid->GetChildren(boxes);
	for (UIBaseObject::UIObjectList::iterator i = boxes.begin(); i != boxes.end(); ++i)
	{
		UIPage * box = static_cast<UIPage *>(*i);

		// Determine which coordinate this box sits at.
		box->GetPropertyInteger(cs_gridPropertyY, tier);
		box->GetPropertyInteger(cs_gridPropertyX, grid);

		SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid);
		if (skill)
		{
			bool const useBright = player->hasSkill(*skill) || ClientExpertiseManager::hasAllocatedExpertise(skill->getSkillName()) ||
				ClientExpertiseManager::canAllocateExpertise(skill->getSkillName());

			updateExpertiseTreeComponentHighlight(box, "iconBorder", useBright);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::displayExpertiseTree()
{
	CreatureObject const * player = Game::getPlayerCreature();
	if (!player)
	{
		return;
	}

	int tree = getTreeIdForTab(m_currentTab);
	int tier = 0;
	int grid = 0;

	//Show background image
	std::string const & path = ExpertiseManager::getExpertiseTreeUiBackgroundIdFromId(tree);
	std::string fullPath(cs_backgroundImageStyleNamespace + "." + path);
	UIImageStyle * style = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(fullPath.c_str(), TUIImageStyle));

	if(style)
		m_treeBackgroundImage->SetStyle(style);
	else
	{
		DEBUG_WARNING(true, ("could not find background image for expertise window %s", fullPath.c_str()));
	}

	UIBaseObject::UIObjectList boxes;
	m_treeGrid->GetChildren(boxes);
	for (UIBaseObject::UIObjectList::iterator i = boxes.begin(); i != boxes.end(); ++i)
	{
		UIPage * box = static_cast<UIPage *>(*i);

		// Clear any pre-existing contents of box.
		UIBaseObject::UIObjectList children;
		box->GetChildren(children);
		for (UIBaseObject::UIObjectList::iterator it = children.begin(); it != children.end(); ++it)
		{
			box->RemoveChild(*it);
		}

		// Determine which coordinate this box sits at.
		box->GetPropertyInteger(cs_gridPropertyY, tier);
		box->GetPropertyInteger(cs_gridPropertyX, grid);

		ClientExpertiseManager::ExpertiseArrowFlags const arrowFlags = ClientExpertiseManager::getArrowFlagsAt(tree, tier, grid);

		SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid);
		if (skill)
		{
			bool const useBright = player->hasSkill(*skill) || ClientExpertiseManager::hasAllocatedExpertise(skill->getSkillName()) ||
				ClientExpertiseManager::canAllocateExpertise(skill->getSkillName());
			bool const useBrightIcon = player->hasSkill(*skill) || ClientExpertiseManager::hasAllocatedExpertise(skill->getSkillName());

			displayExpertiseTreeComponent(box, "iconBorder", useBright);

			if (arrowFlags & ClientExpertiseManager::EAF_Up)
			{
				displayExpertiseTreeComponent(box, "arrowHeadDown", useBright);
			}
			if (arrowFlags & ClientExpertiseManager::EAF_LeftIncoming)
			{
				displayExpertiseTreeComponent(box, "arrowHeadLeft", useBright);
			}
			if (arrowFlags & ClientExpertiseManager::EAF_RightIncoming)
			{
				displayExpertiseTreeComponent(box, "arrowHeadRight", useBright);
			}

			if (arrowFlags & ClientExpertiseManager::EAF_Down)
			{
				displayExpertiseTreeComponent(box, "arrowShaftBottom", useBright);
			}
			if (arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing)
			{
				displayExpertiseTreeComponent(box, "arrowShaftLeft", useBright);
			}
			if (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing)
			{
				displayExpertiseTreeComponent(box, "arrowShaftRight", useBright);
			}

			displayExpertiseTreeIcon(box, skill, useBrightIcon);
		}
		else
		{
			bool useBright = false;

			// Use bright if this arrow is leading to a skill the player has.
			ClientExpertiseManager::PostreqList const & postreqList = ClientExpertiseManager::getPostreqListAt(tree, tier, grid);
			for (ClientExpertiseManager::PostreqList::const_iterator it = postreqList.begin(); it != postreqList.end(); ++it)
			{
				SkillObject const * postreqSkill = SkillManager::getInstance().getSkill(*it);
				if (player->hasSkill(*postreqSkill))
				{
					useBright = true;
					break;
				}
			}

			if (arrowFlags & ClientExpertiseManager::EAF_Up)
			{
				if ((arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing) && (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing))
				{
					displayExpertiseTreeComponent(box, "arrowTeeTop", useBright);
				}
				else if (arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing)
				{
					displayExpertiseTreeComponent(box, "arrowElbowTL", useBright);
				}
				else if (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing)
				{
					displayExpertiseTreeComponent(box, "arrowElbowTR", useBright);
				}
				else if (arrowFlags & ClientExpertiseManager::EAF_Down)
				{
					displayExpertiseTreeComponent(box, "arrowShaftVertical", useBright);
				}
			}
			else if (arrowFlags & ClientExpertiseManager::EAF_Down)
			{
				if ((arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing) && (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing))
				{
					displayExpertiseTreeComponent(box, "arrowTeeBottom", useBright);
				}
				else if (arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing)
				{
					displayExpertiseTreeComponent(box, "arrowElbowBL", useBright);
				}
				else if (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing)
				{
					displayExpertiseTreeComponent(box, "arrowElbowBR", useBright);
				}
			}
			else if ((arrowFlags & ClientExpertiseManager::EAF_LeftOutgoing) && (arrowFlags & ClientExpertiseManager::EAF_RightOutgoing))
			{
				displayExpertiseTreeComponent(box, "arrowShaftHorizontal", useBright);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::displayExpertiseTreeIcon(UIPage * box, SkillObject const * skill, bool useBright)
{
	NOT_NULL(box);
	NOT_NULL(skill);

	std::string const & skillName = skill->getSkillName();
	std::string const & iconPath  = RoadmapManager::getIconPathForSkill(skillName);

	UIPage   * skillIconPage    = NON_NULL(static_cast<UIPage *>(m_sampleIconPage->DuplicateObject())); //lint !e1774
	UIButton * skillIconButton  = NON_NULL(GET_UI_OBJ((*skillIconPage), UIButton, "SkillButton"));
	UIPage   * skillIconBorder =  NON_NULL(GET_UI_OBJ((*skillIconPage), UIPage, "Border"));
	UIText   * skillIconText    = NON_NULL(GET_UI_OBJ((*skillIconPage), UIText, "RankText"));
	UIPage   * skillIconSubPage = NON_NULL(GET_UI_OBJ((*skillIconPage), UIPage, "Icon"));
	UIImage  * skillIconImage   = NON_NULL(GET_UI_OBJ((*skillIconSubPage), UIImage, "Image"));

	box->AddChild(skillIconPage);
	box->MoveChild(skillIconPage, UIBaseObject::Top);

	skillIconPage->Link();
	skillIconPage->SetVisible(true);
	skillIconPage->SetName(UINarrowString("IconPage_" + skillName));

	skillIconBorder->SetVisible(false);

	UIImageStyle * skillIconStyle = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(iconPath.c_str(), TUIImageStyle));
	skillIconImage->SetStyle(skillIconStyle);

	UIString tooltipText = getExpertiseTooltip(skillName);
	skillIconButton->SetTooltip(tooltipText);
	skillIconButton->SetTooltipDelay(false);
	skillIconButton->SetProperty(cs_gridPropertySkillName, Unicode::narrowToWide(skillName));
	skillIconButton->SetEnabled(true);
	registerMediatorObject(*skillIconButton, true);	

	if (useBright)
	{
		skillIconText->SetVisible(true);
		int rank = ClientExpertiseManager::getExpertiseRankForPlayer(skillName);
		int spentRank = ClientExpertiseManager::getExpertiseRankForPlayer(skillName, false);
		setTextToRankNumbers(skillIconText, spentRank, rank, m_numberMode);
	}
	else
	{
		UIEllipse   * bkgrd1 =  NON_NULL(GET_UI_OBJ((*skillIconText->GetParentWidget()), UIEllipse, "bkgrd1"));
		UIPage   * bkgrd2 =  NON_NULL(GET_UI_OBJ((*skillIconText->GetParentWidget()), UIPage, "bkgrd2"));
		bkgrd1->SetVisible(false);
		bkgrd2->SetVisible(false);
		skillIconText->SetVisible(false);
		skillIconImage->SetPropertyColor(UIWidget::PropertyName::Color, UIColor(DARK_LEVEL, DARK_LEVEL, DARK_LEVEL));
	}

}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateExpertiseTreeComponentHighlight(UIPage * box, std::string const & path, bool useBright)
{
	NOT_NULL(box);

	std::string fullPath(cs_componentImageStyleNamespace + "." + path);

	if (useBright)
	{
		fullPath += "_Bright";
	}

	UIImageStyle * style = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(fullPath.c_str(), TUIImageStyle));

	UIPage   * componentPage    = NON_NULL(GET_UI_OBJ((*box), UIPage, "IconBorder"));
	UIPage   * componentSubPage = NON_NULL(GET_UI_OBJ((*componentPage), UIPage, "Icon"));
	UIImage  * componentImage   = NON_NULL(GET_UI_OBJ((*componentSubPage), UIImage, "Image"));

	componentImage->SetStyle(style);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::displayExpertiseTreeComponent(UIPage * box, std::string const & path, bool useBright)
{
	NOT_NULL(box);

	bool isIconBorder = (path == "iconBorder");

	std::string fullPath(cs_componentImageStyleNamespace + "." + path);

	if (useBright)
	{
		fullPath += "_Bright";
	}

	UIImageStyle * style = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(fullPath.c_str(), TUIImageStyle));

	UIPage   * componentPage    = NON_NULL(static_cast<UIPage *>(m_sampleComponentPage->DuplicateObject())); //lint !e1774
	UIPage   * componentSubPage = NON_NULL(GET_UI_OBJ((*componentPage), UIPage, "Icon"));
	UIImage  * componentImage   = NON_NULL(GET_UI_OBJ((*componentSubPage), UIImage, "Image"));

	box->AddChild(componentPage);
	box->MoveChild(componentPage, UIBaseObject::Top);

	componentPage->Link();
	componentPage->SetVisible(true);
	componentPage->SetName(isIconBorder ? "IconBorder" : "ComponentPage");
	
	componentImage->SetStyle(style);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateExpertiseDisplayName()
{
	if(m_currentSkillName.empty())
	{
		m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
		m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());
		return;
	}

	Unicode::String localizedDisplayName;
	ClientExpertiseManager::localizeExpertiseName(m_currentBaseSkillName, localizedDisplayName);
	m_expertiseDisplayName->SetLocalText(localizedDisplayName);
	Unicode::String localizedDisplayDescription;
	ClientExpertiseManager::localizeExpertiseDescription(m_currentBaseSkillName, localizedDisplayDescription);
	m_expertiseDisplayDescription->SetLocalText(localizedDisplayDescription);
	
	std::string const & iconPath  = RoadmapManager::getIconPathForSkill(m_currentSkillName);
	UIImageStyle * skillIconStyle = safe_cast<UIImageStyle*>(getPage().GetObjectFromPath(iconPath.c_str(), TUIImageStyle));
	if(skillIconStyle)
		m_mainIconImage->SetStyle(skillIconStyle);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::displaySkillSelectBorder (UIWidget *context, bool display)
{
	UIWidget * parentContext      = context->GetParentWidget();

	UIPage * skillIconPage = safe_cast<UIPage *>(parentContext);
	
	UIPage * skillIconBorder = NON_NULL(GET_UI_OBJ((*skillIconPage), UIPage, "Border"));

	if(!display)
		skillIconBorder->SetVisible(false);
	else
	{
		UIWidget * grandParentContext = parentContext->GetParentWidget();
		UIPage * box           = safe_cast<UIPage *>(grandParentContext);
		int tree = getTreeIdForTab(m_currentTab);
		int tier = 0;
		int grid = 0;
		
		// Determine which coordinate this box sits at.
		box->GetPropertyInteger(cs_gridPropertyY, tier);
		box->GetPropertyInteger(cs_gridPropertyX, grid);
		
		skillIconBorder->SetVisible(true);

		if (ClientExpertiseManager::playerHasExpertiseOrHasAllocated(m_currentBaseSkillName) || ClientExpertiseManager::getExpertisePlayerIsEligibleForAt(tree, tier, grid))
		{
			skillIconBorder->SetPropertyColor(UIWidget::PropertyName::Color, UIColor::green);
		}
		else
		{
			skillIconBorder->SetPropertyColor(UIWidget::PropertyName::Color, UIColor::red);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::onSkillIconClicked(UIWidget * context, bool isRightMouseButton)
{
	Unicode::String skillName;
	context->GetProperty(cs_gridPropertySkillName, skillName);
	std::string narrowSkillName = Unicode::wideToNarrow(skillName);
	std::string currentTopSkillName = ClientExpertiseManager::getTopExpertiseNameGivenBaseExpertise(narrowSkillName);
	std::string nextSkillName = ClientExpertiseManager::getNextExpertiseNameGivenBaseExpertise(narrowSkillName);
	
	if(context == m_lastExpertiseClickedOn)
	{
		if(!isRightMouseButton && !nextSkillName.empty() && ClientExpertiseManager::allocateExpertise(nextSkillName))
		{		
			displayExpertisePoints();

			//Update the tooltip and the number for this icon
			UIString tooltipText = getExpertiseTooltip(narrowSkillName);
			context->SetTooltip(tooltipText);

			UIWidget *skillIconPage = context->GetParentWidget();
			UIText   * skillIconText    = NON_NULL(GET_UI_OBJ((*skillIconPage), UIText, "RankText"));
			skillIconText->SetVisible(true);
			int rank = ClientExpertiseManager::getExpertiseRankForPlayer(narrowSkillName);
			int spentRank = ClientExpertiseManager::getExpertiseRankForPlayer(narrowSkillName, false);
			setTextToRankNumbers(skillIconText, spentRank, rank, m_numberMode);

			UIPage   * skillIconSubPage = NON_NULL(GET_UI_OBJ((*skillIconPage), UIPage, "Icon"));
			UIImage  * skillIconImage   = NON_NULL(GET_UI_OBJ((*skillIconSubPage), UIImage, "Image"));
			if(rank > 0)
				skillIconImage->SetPropertyColor(UIWidget::PropertyName::Color, UIColor(255, 255, 255));
			else
				skillIconImage->SetPropertyColor(UIWidget::PropertyName::Color, UIColor(DARK_LEVEL, DARK_LEVEL, DARK_LEVEL));
		}
		else if(isRightMouseButton && !currentTopSkillName.empty() && ClientExpertiseManager::deallocateExpertise(currentTopSkillName))
		{
			displayExpertisePoints();

			//Update the tooltip and the number for this icon
			UIString tooltipText = getExpertiseTooltip(narrowSkillName);
			context->SetTooltip(tooltipText);

			int rank = ClientExpertiseManager::getExpertiseRankForPlayer(narrowSkillName);
			int spentRank = ClientExpertiseManager::getExpertiseRankForPlayer(narrowSkillName, false);
			UIWidget *skillIconPage = context->GetParentWidget();
			UIText   * skillIconText    = NON_NULL(GET_UI_OBJ((*skillIconPage), UIText, "RankText"));
			if(rank > 0)
			{
				skillIconText->SetVisible(true);
				setTextToRankNumbers(skillIconText, spentRank, rank, m_numberMode);
			}
			else
			{
				UIEllipse   * bkgrd1 =  NON_NULL(GET_UI_OBJ((*skillIconText->GetParentWidget()), UIEllipse, "bkgrd1"));
				UIPage   * bkgrd2 =  NON_NULL(GET_UI_OBJ((*skillIconText->GetParentWidget()), UIPage, "bkgrd2"));
				bkgrd1->SetVisible(false);
				bkgrd2->SetVisible(false);
				skillIconText->SetVisible(false);
			}

			UIPage   * skillIconSubPage = NON_NULL(GET_UI_OBJ((*skillIconPage), UIPage, "Icon"));
			UIImage  * skillIconImage   = NON_NULL(GET_UI_OBJ((*skillIconSubPage), UIImage, "Image"));
			if(rank > 0)
				skillIconImage->SetPropertyColor(UIWidget::PropertyName::Color, UIColor(255, 255, 255));
			else
				skillIconImage->SetPropertyColor(UIWidget::PropertyName::Color, UIColor(DARK_LEVEL, DARK_LEVEL, DARK_LEVEL));
		}
		updateExpertiseDescriptionSection();
		updateExpertiseRequirementsSection();
		updateButtonStatus();
		updateExpertiseIconHighlighting();
	}
	else
	{
		if(m_lastExpertiseClickedOn)
			displaySkillSelectBorder(m_lastExpertiseClickedOn, false);
		m_lastExpertiseClickedOn = context;
		if(currentTopSkillName.empty())
			m_currentSkillName = narrowSkillName;
		else
			m_currentSkillName = currentTopSkillName;
		m_currentBaseSkillName = narrowSkillName;
		displaySkillSelectBorder(context, true);

		updateExpertiseDisplayName();
		updateExpertiseDescriptionSection();
		updateExpertiseRequirementsSection();
		updateButtonStatus();
		updateExpertiseIconHighlighting();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateExpertiseDescriptionSection()
{
	if(m_currentSkillName.empty())
	{
		m_commandType->SetVisible(false);
		m_skillModType->SetVisible(false);
		m_schematicType->SetVisible(false);

		m_currentRankText->Clear();
		return;
	}

	Unicode::String s = s_rank.localize();

	int rank = ClientExpertiseManager::getExpertiseRankForPlayer(m_currentSkillName);
	int spentRank = ClientExpertiseManager::getExpertiseRankForPlayer(m_currentSkillName, false);
	int maxRank = ExpertiseManager::getExpertiseRankMax(m_currentSkillName);
	s.append(Unicode::narrowToWide(FormattedString<128>().sprintf("%d/%d", rank, maxRank)));
	m_currentRankText->SetLocalText(s);

	m_commandType->SetVisible(false);
	m_schematicType->SetVisible(false);
	m_skillModType->SetVisible(false);
	if(ClientExpertiseManager::isExpertiseCommandType(m_currentSkillName))
	{	
		m_commandType->SetVisible(true);
		std::string commandName = ClientExpertiseManager::getExpertiseCommand(m_currentSkillName);

		Unicode::String tmp;
		CuiSkillManager::localizeCmdName (commandName, tmp);
		m_commandTypeName->SetLocalText(tmp);
		CuiSkillManager::localizeCmdDescription (commandName, tmp);
		m_commandTypeDescription->SetLocalText(tmp);

		CuiDragInfo item;
		item.type = CuiDragInfoTypes::CDIT_command;
		item.str = commandName;		
		UIImageStyle * const imageStyle = CuiIconManager::findIconImageStyle (item);
		if(imageStyle)
			m_commandTypeIcon->SetStyle(imageStyle);
	}
	else if(ClientExpertiseManager::isExpertiseSkillModType(m_currentSkillName))
	{
		m_skillModType->SetVisible(true);
		ClientExpertiseManager::ExpertiseSkillModStruct expertiseSkillMods;
		ClientExpertiseManager::getExpertiseSkillMods(m_currentBaseSkillName, expertiseSkillMods);
		for(int i = 0; i < MAX_NUM_SKILL_MODS_PER_EXPERTISE ; ++i)
		{
			if(expertiseSkillMods.names[i].empty())
			{
				m_skillModTypeMain[i]->SetVisible(false);
			}
			else
			{
				m_skillModTypeMain[i]->SetVisible(true);
				Unicode::String skillModName;
				CuiSkillManager::localizeSkillModName(expertiseSkillMods.names[i], skillModName);
				m_skillModTypeNames[i]->SetLocalText(skillModName);
				Unicode::String skillModDescription;
				CuiSkillManager::localizeSkillModDesc(expertiseSkillMods.names[i], skillModDescription);
				m_skillModTypeDescriptions[i]->SetLocalText(skillModDescription);
				int amountSoFar = 0;
				for(int j = 0; j < MAX_NUM_EXPERTISE_RANKS; ++j)
				{
					int value = expertiseSkillMods.values[(i * MAX_NUM_EXPERTISE_RANKS) + j];
					if(value != ClientExpertiseManager::SKILL_MOD_TAG_VALUE)
					{					
						amountSoFar += value;
						setTextToNumber(m_skillModTypeAmounts[i][j], abs(amountSoFar));
					}
					else
					{
						m_skillModTypeAmounts[i][j]->SetLocalText(Unicode::narrowToWide("--"));
					}
					if(j < spentRank)
					{
						m_skillModTypeMarkers[i][j]->SetVisible(true);
						m_skillModTypeMarkers[i][j]->SetBackgroundColor(s_grey);
						m_skillModTypeAmounts[i][j]->SetTextColor(UIColor::white);
					}
					else if(j < rank)
					{
						m_skillModTypeMarkers[i][j]->SetVisible(true);
						m_skillModTypeMarkers[i][j]->SetBackgroundColor(s_yellow);
						m_skillModTypeAmounts[i][j]->SetTextColor(UIColor::black);
					}
					else
					{
						m_skillModTypeMarkers[i][j]->SetVisible(false);
						m_skillModTypeAmounts[i][j]->SetTextColor(UIColor::white);
					}
				}
			}
		}
	}
	else if(ClientExpertiseManager::isExpertiseSchematicType(m_currentSkillName))
	{
		m_schematicType->SetVisible(true);
	}
	else
	{
		DEBUG_WARNING(true, ("expertise %s grants nothing!", m_currentSkillName.c_str()));
	}
}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateExpertiseRequirementsSection()
{
	if(m_currentSkillName.empty())
	{
		m_expertiseDisplayRequirements->Clear();
		return;
	}

	int tier = ExpertiseManager::getExpertiseTier(m_currentSkillName);
	int tierPoints = (tier - 1) * MAX_NUM_EXPERTISE_RANKS;

	std::string prevSkillName = ClientExpertiseManager::getPreviousExpertise(m_currentBaseSkillName);
	std::string prevBaseSkillName = ClientExpertiseManager::getBaseExpertiseNameForExpertise(prevSkillName);

	int tree = ExpertiseManager::getExpertiseTree(m_currentSkillName);
	UIString treeName;
	ClientExpertiseManager::localizeExpertiseTreeNameFromId(tree, treeName);
	
	m_expertiseDisplayRequirements->Clear();
	m_expertiseDisplayRequirements->SetLocalText(s_requiresTag.localize());
	if(tierPoints > 0)
	{
		int pointsInTree = ClientExpertiseManager::getExpertisePointsSpentForPlayerInTree(tree);
		if(pointsInTree < tierPoints)
			m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\\#ff0000"));
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide(" \\>070"));
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("%d", tierPoints)));
		m_expertiseDisplayRequirements->AppendLocalText(s_pointsIn.localize());
		m_expertiseDisplayRequirements->AppendLocalText(treeName);
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\n"));
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\\#."));
	}
	if(!prevSkillName.empty())
	{
		Unicode::String localizedDisplayName;
		ClientExpertiseManager::localizeExpertiseName(prevBaseSkillName, localizedDisplayName);
		int numPreviousPoints = ExpertiseManager::getExpertiseRankMax(prevSkillName);
		int numSpent = ClientExpertiseManager::getExpertiseRankForPlayer(prevSkillName);
		if(numSpent < numPreviousPoints)
			m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\\#ff0000"));

		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\\>070"));
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide(FormattedString<128>().sprintf("%d", numPreviousPoints)));
		m_expertiseDisplayRequirements->AppendLocalText((numPreviousPoints == 1) ? s_pointIn.localize() : s_pointsIn.localize());
		m_expertiseDisplayRequirements->AppendLocalText(localizedDisplayName);
		m_expertiseDisplayRequirements->AppendLocalText(Unicode::narrowToWide("\\#."));
	}
	if((tierPoints <= 0) && (prevSkillName.empty()))
		m_expertiseDisplayRequirements->AppendLocalText(s_none.localize());
}

//----------------------------------------------------------------------

void SwgCuiExpertise::updateButtonStatus()
{
	m_acceptButton->SetEnabled(ClientExpertiseManager::getExpertisePointsAllocatedForPlayer() > 0);
	m_clearTreeButton->SetEnabled(ClientExpertiseManager::getExpertisePointsAllocatedForPlayerInTree(getTreeIdForTab(m_currentTab)) > 0);
	m_clearAllButton->SetEnabled(ClientExpertiseManager::getExpertisePointsAllocatedForPlayer() > 0);
}

//----------------------------------------------------------------------

void SwgCuiExpertise::onMessageBoxClosed (const CuiMessageBox & box)
{
	if (&box == m_messageBoxTrainExpertises)
	{
		m_callback->disconnect(m_messageBoxTrainExpertises->getTransceiverClosed(), *this, &SwgCuiExpertise::onMessageBoxClosed);
		m_messageBoxTrainExpertises = 0;

		if (box.completedAffirmative ())
		{
			ClientExpertiseManager::sendAllocatedExpertiseListAndClear();
			
			m_currentSkillName = s_emptyString;
			m_currentBaseSkillName = s_emptyString;
			m_expertiseDisplayName->SetLocalText(s_noExpertiseTitle.localize());
			m_expertiseDisplayDescription->SetLocalText(s_noExpertiseDescription.localize());
			m_commandType->SetVisible(false);
			m_skillModType->SetVisible(false);
			m_schematicType->SetVisible(false);
			m_expertiseDisplayRequirements->Clear();
			m_currentRankText->Clear();
			m_lastExpertiseClickedOn = 0;
			displayExpertiseTree();
			updateButtonStatus();
			updateExpertiseIconHighlighting();
		}
	}
}

//======================================================================
