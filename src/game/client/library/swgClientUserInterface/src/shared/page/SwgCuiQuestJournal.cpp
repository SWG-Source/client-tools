//======================================================================
//
// SwgCuiQuestJournal.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiQuestJournal.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiPlayerQuestManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStaticLootItemManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsMission.h"
#include "clientUserInterface/CuiStringIdsQuest.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/QuestTask.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UIDataSourceContainer.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITreeView.h"
#include "UIUtils.h"

//======================================================================

namespace SwgCuiQuestJournalNamespace
{
	static const UILowerString QuestNameProperty = UILowerString("QuestNameProperty");
	static const UILowerString MissionIdProperty = UILowerString("MissionIdProperty");
	static const UILowerString CanGetRewardProperty = UILowerString("CanGetRewardProperty");
	static const UILowerString PlayerQuestIdProperty = UILowerString("PlayerQuestIdProperty");

	static const StringId noForceSensitiveQuests("ui_quest","no_quests");
	
	char const * const cms_completedIconStyle = "/styles.icon.misc.granted";

	Unicode::String const cms_expandAllStyle(Unicode::narrowToWide("/Styles.tree.image.expanded"));
	Unicode::String const cms_collapseAllStyle(Unicode::narrowToWide("/Styles.tree.image.collapsed"));

	Unicode::String cms_newline(Unicode::narrowToWide("\n"));

	namespace Settings
	{
		const std::string s_completed("completed");
	}

	std::string const cms_task("task");
	std::string const cms_colorWhiteStr("\\#ffffff");
	std::string const cms_colorGreyStr("\\#888888");
	std::string const cms_slashes("\\");

	std::string const cms_emptyString();

	char const * const cms_soloType = "solo";

	std::vector<UIText *> ms_currentCounters;
	std::vector<UIText *> ms_currentTimers;

	float ms_timeSinceUpdate = 0.0f;
	float const cms_updateTime = 1.0f;

	int const cms_taskSpacerHeight = 10;

	UIText const * ms_lastCounter = NULL;

	bool ms_isFirstOpen = true;

	std::set<std::string> ms_openedRows;

	bool ms_internalTreeUpdating = false;

	int const cms_maxItems = 10;

	Object * ms_itemRewardObjects[cms_maxItems];

	float const cms_itemWaitTime = 0.5f;

	float ms_needsUpdateIn = 0.0f;

	std::string convertServerObjectTemplateNameToSharedObjectTemplateName (std::string const & serverObjectTemplateName)
	{
		size_t const sharedpos = serverObjectTemplateName.rfind ('/');
		if (sharedpos != std::string::npos)
		{
			return serverObjectTemplateName.substr (0, sharedpos + 1) + "shared_" + serverObjectTemplateName.substr (sharedpos + 1);
		}

		WARNING (true, ("bad template name"));
		return std::string ();
	}
}

using namespace SwgCuiQuestJournalNamespace;

//----------------------------------------------------------------------

SwgCuiQuestJournal::SwgCuiQuestJournal(UIPage & page) :
CuiMediator                  ("SwgCuiQuestJournal", page)
 , UIEventCallback           ()
 , m_callback                (new MessageDispatch::Callback)
 , m_showCompletedCheckbox   (0)
 , m_description             (0)
 , m_titleText               (0)
 , m_experienceRewardText    (0)
 , m_factionRewardText       (0)
 , m_moneyRewardText         (0)
 , m_rewardLabel             (0)
 , m_questTree               (0)
 , m_okButton                (0)
 , m_firstTime               (true)
 , m_descriptionComp         (0)
 , m_buttonAbandonQuest      (0)
 , m_tasksSample             (0)
 , m_descLabel               (0)
 , m_sortCombo               (0)
 , m_sortType                (ST_category)
 , m_levelText               (0)
 , m_levelLabel              (0)
 , m_postTaskSpacer          (0)
 , m_buttonExpandAll         (0)
 , m_typePage                (0)
 , m_typeText                (0)
 , m_inclusiveText           (0)
 , m_exclusiveText           (0)
 , m_chooseOneText           (0)
 , m_abandonMissionButton    (0)
 , m_questDescriptionPage    (0)
 , m_missionDescriptionPage  (0)
 , m_missionTitleText        (0)
 , m_missionDescriptionText  (0)
 , m_missionMoneyRewardText  (0)
 , m_missionDescriptionComp  (0)
 , m_missionLevelText        (0)
 , m_missionTypeText         (0)
 , m_missionCreatorText      (0)
 , m_missionTargetText       (0)
 , m_missionStartLocText     (0)
 , m_missionDestLocText      (0)
 , m_missionEfficiencyText   (0)
 , m_playerQuestTitleText    (0)
 , m_playerQuestDescriptionText (0)
 , m_playerQuestDescriptionComp (0)
 , m_playerQuestCreatorText  (0)
 , m_playerQuestDescriptionPage (0)
 , m_playerQuestTaskSample (0)
 , m_playerQuestTaskComposite (0)
 , m_playerQuestDifficultyText (0)
{
	//hook to UI elements
	getCodeDataObject(TUICheckbox,     m_showCompletedCheckbox,       "checkShowMission");
	getCodeDataObject(TUIText,         m_description,                 "descQuest");
	getCodeDataObject(TUITreeView,     m_questTree,                   "treeQuests");
	getCodeDataObject(TUIButton,       m_okButton,                    "buttonOK");
	getCodeDataObject(TUIComposite,    m_descriptionComp,             "descriptionComp");
	getCodeDataObject(TUIButton,       m_buttonAbandonQuest,          "buttonAbandon");
	getCodeDataObject(TUIText,         m_titleText,                   "texttitle");
	getCodeDataObject(TUIText,         m_experienceRewardText,        "experienceRewardText");
	getCodeDataObject(TUIText,         m_factionRewardText,           "factionRewardText");
	getCodeDataObject(TUIText,         m_moneyRewardText,             "moneyRewardText");
	getCodeDataObject(TUIText,         m_rewardLabel,                 "rewardLabel");
	getCodeDataObject(TUIComposite,    m_tasksSample,                 "sampleTask");
	getCodeDataObject(TUIText,         m_descLabel,                   "descLabel");
	getCodeDataObject(TUIComboBox,     m_sortCombo,                   "sortCombo");
	getCodeDataObject(TUIText,         m_levelText,                   "textlevel");
	getCodeDataObject(TUIText,         m_levelLabel,                  "textLevelLabel");
	getCodeDataObject(TUIText,         m_postTaskSpacer,              "postTaskSpacer");
	getCodeDataObject(TUIButton,       m_buttonExpandAll,             "buttonExpandAll");
	getCodeDataObject(TUIPage,         m_typePage,                    "typepage");
	getCodeDataObject(TUIText,         m_typeText,                    "typetext");
	getCodeDataObject(TUIText,         m_inclusiveText,               "inclusiveText");
	getCodeDataObject(TUIText,         m_exclusiveText,               "exclusiveText");
	getCodeDataObject(TUIText,         m_chooseOneText,               "chooseOneText");
	getCodeDataObject(TUIButton,       m_abandonMissionButton,        "buttonabandonmission");
	getCodeDataObject(TUIPage,         m_questDescriptionPage,        "questdescriptionpage");
	getCodeDataObject(TUIPage,         m_missionDescriptionPage,      "missiondescriptionpage");
	getCodeDataObject(TUIText,         m_missionTitleText,            "missiontitletext");
	getCodeDataObject(TUIText,         m_missionDescriptionText,      "missiondescriptiontext");
	getCodeDataObject(TUIText,         m_missionMoneyRewardText,      "missionmoneyrewardtext");
	getCodeDataObject(TUIComposite,    m_missionDescriptionComp,      "missionDescriptionComp");
	getCodeDataObject(TUIText,         m_missionLevelText,            "missiontextlevel");
	getCodeDataObject(TUIText,         m_missionTypeText,             "missiontypetext");

	getCodeDataObject(TUIText,         m_missionCreatorText,          "missioncreatortext");
	getCodeDataObject(TUIText,         m_missionTargetText,           "missiontargettext");
	getCodeDataObject(TUIText,         m_missionStartLocText,         "missionstartloctext");
	getCodeDataObject(TUIText,         m_missionDestLocText,          "missiondestloctext");
	getCodeDataObject(TUIText,         m_missionEfficiencyText,       "missionefficiencytext");

	getCodeDataObject(TUIText,         m_playerQuestTitleText,        "playerquesttitletext");
	getCodeDataObject(TUIText,		   m_playerQuestDescriptionText,  "playerquestdescriptiontext");
	getCodeDataObject(TUIComposite,    m_playerQuestDescriptionComp,  "playerquestdetailscomp");
	getCodeDataObject(TUIText,		   m_playerQuestCreatorText,      "playerquestcreatortext");
	getCodeDataObject(TUIPage,		   m_playerQuestDescriptionPage,  "playerquestpage");
	getCodeDataObject(TUIPage,		   m_playerQuestTaskSample,       "playerquestsampletask");
	getCodeDataObject(TUIComposite,    m_playerQuestTaskComposite,    "playerquesttaskcomposite");
	getCodeDataObject(TUIText,		   m_playerQuestDifficultyText,   "playerquestdifficultytext");

	std::string codeDataName;
	for(int i = 0; i < cms_maxItems; ++i)
	{
		codeDataName = "itempage";
		char buffer[16];
		//add once, since codedatas are 1-based
		_itoa(i+1, buffer, 10);
		codeDataName += buffer;
		getCodeDataObject(TUIPage, m_rewardItemPages[i], codeDataName.c_str());
		UIBaseObject * const comp = m_rewardItemPages[i]->GetChild("comp");
		UIBaseObject * const icon = comp->GetChild("icon");
		m_rewardItemViewers[i] = dynamic_cast<CuiWidget3dObjectListViewer *>(icon->GetChild("v"));
		m_rewardItemViewers[i]->setRotateSpeed(1.0f);
		m_rewardItemNames[i] = dynamic_cast<UIText *>(comp->GetChild("rewardname"));
		m_rewardItemNames[i]->SetPreLocalized(true);
		m_rewardItemNames[i]->Clear();
		m_rewardItemTexts[i] = dynamic_cast<UIText *>(comp->GetChild("text"));
		m_rewardItemTexts[i]->SetPreLocalized(true);
		m_rewardItemSelectionBoxes[i] = dynamic_cast<UIPage *>(m_rewardItemPages[i]->GetChild("selection"));
		m_rewardItemSelectionBoxes[i]->SetVisible(false);

		ms_itemRewardObjects[i] = NULL;
	}

	//initialize UI state
	registerMediatorObject(*m_showCompletedCheckbox, true);
	registerMediatorObject(*m_questTree, true);
	registerMediatorObject(*m_okButton, true);
	registerMediatorObject(*m_buttonAbandonQuest, true);
	registerMediatorObject(*m_abandonMissionButton, true);
	registerMediatorObject(*m_sortCombo, true);
	registerMediatorObject(*m_buttonExpandAll, true);
	m_showCompletedCheckbox->SetChecked(false);
	m_description->SetPreLocalized(true);
	m_titleText->SetPreLocalized(true);
	m_titleText->Clear();
	m_experienceRewardText->SetPreLocalized(true);
	m_experienceRewardText->Clear();
	m_experienceRewardText->SetVisible(false);
	m_factionRewardText->SetPreLocalized(true);
	m_factionRewardText->Clear();
	m_factionRewardText->SetVisible(false);
	m_moneyRewardText->SetPreLocalized(true);
	m_moneyRewardText->Clear();
	m_moneyRewardText->SetVisible(false);
	m_buttonAbandonQuest->SetVisible(false);
	NON_NULL(m_questTree)->ClearData();
	m_sortCombo->SetSelectedIndex(0, true);
	m_levelText->Clear();
	m_levelText->SetPreLocalized(true);
	m_typePage->SetVisible(false);
	m_typeText->Clear();
	m_typeText->SetPreLocalized(true);
	m_tasksSample->SetVisible(false);
	m_inclusiveText->SetVisible(false);
	m_exclusiveText->SetVisible(false);
	m_chooseOneText->SetVisible(false);

	m_missionTitleText->Clear();
	m_missionDescriptionText->Clear();
	m_missionMoneyRewardText->Clear();
	m_missionLevelText->Clear();
	m_missionTypeText->Clear();
	m_missionCreatorText->Clear();
	m_missionTargetText->Clear();
	m_missionStartLocText->Clear();
	m_missionDestLocText->Clear();
	m_missionEfficiencyText->Clear();
	m_missionTitleText->SetPreLocalized(true);
	m_missionDescriptionText->SetPreLocalized(true);
	m_missionMoneyRewardText->SetPreLocalized(true);
	m_missionLevelText->SetPreLocalized(true);
	m_missionTypeText->SetPreLocalized(true);
	m_missionCreatorText->SetPreLocalized(true);
	m_missionTargetText->SetPreLocalized(true);
	m_missionStartLocText->SetPreLocalized(true);
	m_missionDestLocText->SetPreLocalized(true);
	m_missionEfficiencyText->SetPreLocalized(true);

	m_missionStartLocText->SetVisible(false);
	m_missionDestLocText->SetVisible(false);
	m_missionEfficiencyText->SetVisible(false);

	m_playerQuestCreatorText->SetPreLocalized(true);
	m_playerQuestDescriptionText->SetPreLocalized(true);
	m_playerQuestTitleText->SetPreLocalized(true);
	m_playerQuestDifficultyText->SetPreLocalized(true);

	m_playerQuestTaskSample->SetVisible(false);

	//page can be closed
	setState(MS_closeable);
	setState(MS_closeDeactivates);

	//updates per-frame
	setIsUpdating(true);

	//watch for dynamic data changes
	m_callback->connect(*this, &SwgCuiQuestJournal::onCountersChanged, static_cast<QuestJournalManager::Messages::CountersChanged*>(0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onTimersChanged, static_cast<QuestJournalManager::Messages::TimersChanged*>(0));
}

//----------------------------------------------------------------------

SwgCuiQuestJournal::~SwgCuiQuestJournal()
{
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onTimersChanged, static_cast<QuestJournalManager::Messages::TimersChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onCountersChanged, static_cast<QuestJournalManager::Messages::CountersChanged*>(0));

	delete m_callback;
	m_callback = 0;

	QuestJournalManager::clearTimerData();

	setIsUpdating(false);

	deleteTaskData();
	 
	m_buttonAbandonQuest     = 0;
	m_showCompletedCheckbox  = 0;
	m_description            = 0;
	m_questTree              = 0;
	m_okButton               = 0;
	m_descriptionComp        = 0;
	m_titleText              = 0;
	m_experienceRewardText   = 0;
	m_factionRewardText      = 0;
	m_moneyRewardText        = 0;
	m_rewardLabel            = 0;
	m_tasksSample            = 0;
	m_descLabel              = 0;
	m_levelText              = 0;
	m_levelLabel             = 0;
	m_postTaskSpacer         = 0;
	m_buttonExpandAll        = 0;
	m_typePage               = 0;
	m_typeText               = 0;
	m_inclusiveText          = 0;
	m_exclusiveText          = 0;
	m_chooseOneText          = 0;
	m_abandonMissionButton   = 0;
	m_missionTitleText       = 0;
	m_missionDescriptionText = 0;
	m_missionMoneyRewardText = 0;
	m_missionDescriptionComp = 0;
	m_missionLevelText       = 0;
	m_missionTypeText        = 0;
	m_missionCreatorText     = 0;
	m_missionTargetText      = 0;
	m_missionStartLocText    = 0;
	m_missionDestLocText     = 0;
	m_missionEfficiencyText  = 0;

	for(int i = 0; i < cms_maxItems; ++i)
	{
		delete ms_itemRewardObjects[i];
		ms_itemRewardObjects[i]       = NULL;
		m_rewardItemPages[i]          = NULL;
		m_rewardItemViewers[i]        = NULL;
		m_rewardItemNames[i]          = NULL;
		m_rewardItemTexts[i]          = NULL;
		m_rewardItemSelectionBoxes[i] = NULL;
	}
}


//----------------------------------------------------------------------

void SwgCuiQuestJournal::performActivate()
{
	//clear UI to be re-initialized
	if(m_description)
		m_description->Clear();

	m_callback->connect(*this, &SwgCuiQuestJournal::onActiveQuestsChanged,     static_cast<PlayerObject::Messages::ActiveQuestsChanged *>        (0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onCompletedQuestsChanged,  static_cast<PlayerObject::Messages::CompletedQuestsChanged *>     (0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onQuestsChanged,           static_cast<PlayerObject::Messages::QuestsChanged *>              (0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onPlayerSetup,             static_cast<CreatureObject::Messages::PlayerSetup *>              (0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onContainerChange,         static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->connect(*this, &SwgCuiQuestJournal::onContainerChange,         static_cast<ClientObject::Messages::RemovedFromContainer *> (0));
	
	CuiManager::requestPointer(true); 

	updateAll();

	if(m_firstTime)
	{
		m_firstTime = false;
		expandAllCategories(true);
	}
	else
	{
		int const childrenCount = static_cast<int>(m_questTree->GetRowCount());
		for(int i = 0; i < childrenCount; ++i)
		{
			UIDataSourceContainer const * const dsc = m_questTree->GetDataSourceContainerAtRow(i);
			UIString name;
			dsc->GetProperty(UILowerString("Name"), name);
			if(ms_openedRows.find(Unicode::wideToNarrow(name)) != ms_openedRows.end())
				m_questTree->SetRowExpanded(i, true);
		}
	}

	updateSelection();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::performDeactivate()
{
	CuiManager::requestPointer(false); 

	m_callback->disconnect(*this, &SwgCuiQuestJournal::onActiveQuestsChanged,     static_cast<PlayerObject::Messages::ActiveQuestsChanged *>        (0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onCompletedQuestsChanged,  static_cast<PlayerObject::Messages::CompletedQuestsChanged *>     (0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onQuestsChanged,           static_cast<PlayerObject::Messages::QuestsChanged *>              (0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onPlayerSetup,             static_cast<CreatureObject::Messages::PlayerSetup *>              (0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onContainerChange,         static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->disconnect(*this, &SwgCuiQuestJournal::onContainerChange,         static_cast<ClientObject::Messages::RemovedFromContainer *> (0));

	ms_isFirstOpen = false;
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onPlayerSetup(const CreatureObject & creature)
{
	if(&creature == Game::getPlayerCreature())
	{
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onActiveQuestsChanged(const PlayerObject & player)
{
	if(&player == Game::getPlayerObject())
	{
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onCompletedQuestsChanged(const PlayerObject & player)
{
	if(&player == Game::getPlayerObject())
	{
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onQuestsChanged(const PlayerObject & player)
{
	if(&player == Game::getPlayerObject())
	{
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onContainerChange(ClientObject::Messages::ContainerMsg const & msg)
{
	// See if something was just added/removed to/from the datapad
	ClientObject const * const container = msg.first;
	if (container == CuiInventoryManager::getPlayerDatapad())
	{
		// It may have been a mission that was added or removed so update everything...
		updateAll();
	}
}

//----------------------------------------------------------------------

/**
 * Update the entire UI
 */
void SwgCuiQuestJournal::updateAll()
{	
	PlayerObject const * const player = Game::getPlayerObject();
	if(!player)
	{
		return;
	}
	updateQuestTree();
	updateDescription();
	updateSelection();
}


//----------------------------------------------------------------------

/**
 * Update the current selection if there is none
 */
void SwgCuiQuestJournal::updateSelection()
{
	std::string const & selectedQuest = getSelectedQuest();
	NetworkId const & missionId = getSelectedMission();
	NetworkId const & playerQuestId = getSelectedPlayerQuest();
	if(selectedQuest.empty() && !missionId.isValid() && !playerQuestId.isValid())
	{
		PlayerObject const * const playerObj = Game::getPlayerObject();
		if(playerObj)
		{
			std::vector<Quest const *> allQuests;
			playerObj->questGetAllQuests(allQuests);
			for(int i = 0; i < static_cast<int>(m_questTree->GetRowCount()); ++i)
			{
				std::string const & questName = getQuestAtRow(i);
				if(!questName.empty())
				{
					m_questTree->SelectRow(i);
					m_questTree->SetRowExpanded(i, true);
					storeRowExpanded(true, i);

					OnGenericSelectionChanged(m_questTree);
					break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

/**
 * Update the lower, quest description window
 */
void SwgCuiQuestJournal::updateDescription()
{
	std::string const & selectedQuest = getSelectedQuest();
	uint32 const questCrc = getSelectedQuestCrc();
	NetworkId const & missionNid = getSelectedMission();
	NetworkId const & playerQuestId = getSelectedPlayerQuest();
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;

	//are we displaying a quest or a mission?
	bool const displayingQuest   = !selectedQuest.empty() && !missionNid.isValid() && !playerQuestId.isValid();
	bool const displayingMission = missionNid.isValid();
	bool const displayingPlayerQuest = playerQuestId.isValid();

	if(displayingQuest)
	{
		m_buttonAbandonQuest->SetVisible(QuestManager::isQuestAbandonable(selectedQuest) && playerObj->questHasActiveQuest(questCrc) && !playerObj->questHasCompletedQuest(questCrc));
		m_abandonMissionButton->SetVisible(false);
		m_questDescriptionPage->SetVisible(true);
		m_missionDescriptionPage->SetVisible(false);
		m_playerQuestDescriptionPage->SetVisible(false);
		updateQuestDescription();
	}
	else if(displayingMission)
	{
		m_buttonAbandonQuest->SetVisible(false);
		m_abandonMissionButton->SetVisible(true);
		m_questDescriptionPage->SetVisible(false);
		m_missionDescriptionPage->SetVisible(true);
		m_playerQuestDescriptionPage->SetVisible(false);
		updateMissionDescription();
	}
	else if(displayingPlayerQuest)
	{
		m_buttonAbandonQuest->SetVisible(true);
		m_abandonMissionButton->SetVisible(false);
		m_questDescriptionPage->SetVisible(false);
		m_missionDescriptionPage->SetVisible(false);
		m_playerQuestDescriptionPage->SetVisible(true);
		updatePlayerQuestDescription();
	}
	else
	{
		m_buttonAbandonQuest->SetVisible(false);
		m_abandonMissionButton->SetVisible(false);
		m_questDescriptionPage->SetVisible(true);
		m_missionDescriptionPage->SetVisible(false);
		m_playerQuestDescriptionPage->SetVisible(false);
		clearQuestDescription();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::updateQuestDescription()
{
	std::string const & selectedQuest = getSelectedQuest();
	uint32 const questCrc = getSelectedQuestCrc();
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;

	Quest const * const q = QuestManager::getQuest(selectedQuest);
	m_descriptionComp->SetVisible(q != NULL);
	if(!q)
		return;

	//the set quest title
	std::string const & questColor = getQuestColor(q->getId());
	Unicode::String title = Unicode::narrowToWide(questColor.c_str());
	title += q->getJournalEntryTitle().localize();
	m_titleText->SetVisible(true);
	m_titleText->SetLocalText(title);

	//show type if not solo
	if(!q->getType().empty() && (q->getType() != cms_soloType))
	{
		m_typePage->SetVisible(true);
		m_typeText->SetVisible(true);
		m_typeText->SetLocalText(StringId("ui_quest", "type_pre").localize() + StringId("ui_quest", q->getType()).localize());
	}
	else
	{
		m_typePage->SetVisible(false);
	}

	//set the quest label
	int const level = q->getLevel();
	char buffer[10];
	_itoa(level, buffer, 10);
	m_levelLabel->SetVisible(true);
	m_levelText->SetVisible(true);
	m_levelText->SetLocalText(Unicode::narrowToWide(buffer));

	//set the quest description
	StringId const & sid = (playerObj->questHasCompletedQuest(questCrc) && q->getJournalEntryCompetionSummary().isValid())? q->getJournalEntryCompetionSummary() : q->getJournalEntryDescription();
	m_descLabel->SetVisible(true);
	m_description->SetVisible(true);
	m_description->SetLocalText(sid.localize()); //lint!e613 !e831 possibly null, no, it would fatal in constructor

	clearTaskPages();
	if(!playerObj->questHasCompletedQuest(q->getId()))
	{
		updateDecriptionTasks(*q);
		updateDecriptionReward(*q);
	}
	else
	{
		m_rewardLabel->SetVisible(false);
		m_experienceRewardText->SetVisible(false);
		m_factionRewardText->SetVisible(false);
		m_moneyRewardText->SetVisible(false);
		for(int i2 = 0; i2 < cms_maxItems; ++i2)
		{
			m_rewardItemPages[i2]->SetVisible(false);
		}
	}

	m_descriptionComp->Link();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::clearQuestDescription()
{
	m_titleText->SetVisible(false);
	m_typePage->SetVisible(false);
	m_typeText->SetVisible(false);
	m_levelLabel->SetVisible(false);
	m_levelText->SetVisible(false);
	m_descLabel->SetVisible(false);
	m_description->SetVisible(false);
	m_inclusiveText->SetVisible(false);
	m_exclusiveText->SetVisible(false);
	clearTaskPages();
	m_rewardLabel->SetVisible(false);
	m_experienceRewardText->SetVisible(false);
	m_factionRewardText->SetVisible(false);
	m_moneyRewardText->SetVisible(false);
	for(int i2 = 0; i2 < cms_maxItems; ++i2)
	{
		m_rewardItemPages[i2]->SetVisible(false);
	}
	m_descriptionComp->Link();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::updateMissionDescription()
{
	NetworkId const & missionNid = getSelectedMission();
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;
	CreatureObject const * const creatureObj = Game::getPlayerCreature();
	if(!creatureObj)
		return;

	Object const * const o = NetworkIdManager::getObjectById(missionNid);
	ClientMissionObject const * const m = dynamic_cast<ClientMissionObject const *>(o);
	if(m)
	{
		int const playerLevel = creatureObj->getGroupLevel();
		UIColor const & conColor = CuiCombatManager::getConColor(playerLevel, m->getDifficulty());
		std::string colorResult;
		UIUtils::FormatColor(colorResult, conColor);
		Unicode::String const & titleText = Unicode::narrowToWide(cms_slashes + colorResult) + m->getTitle().localize();
		m_missionTitleText->SetLocalText(titleText);

		m_missionDescriptionText->SetLocalText(m->getDescription().localize());

		char buffer[16];
		_itoa(m->getDifficulty(), buffer, 10);
		m_missionLevelText->SetLocalText(Unicode::narrowToWide(buffer));

		std::string const & missionTypeString = m->getMissionTypeString();
		m_missionTypeText->SetLocalText(StringId("ui_quest", "type_pre").localize() + StringId("mission_types_n", missionTypeString).localize());

		m_missionTargetText->SetLocalText(CuiStringIdsQuest::target.localize() + StringId::decodeString(Unicode::narrowToWide(m->getTargetName())));

		m_missionCreatorText->SetLocalText(CuiStringIdsQuest::creator.localize() + m->getMissionCreator());

		_itoa(m->getReward(), buffer, 10);
		Unicode::String const & creditsText = CuiStringIdsQuest::reward.localize() + Unicode::narrowToWide(buffer) + CuiStringIdsQuest::credits.localize();
		m_missionMoneyRewardText->SetLocalText(creditsText);
		m_missionMoneyRewardText->SetVisible(true);

		updateSpecificMissionDescriptionData(missionTypeString);
	}

	m_missionDescriptionComp->Link();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::updateSpecificMissionDescriptionData(std::string const & missionType)
{
	m_missionStartLocText->SetVisible(false);
	m_missionDestLocText->SetVisible(false);
	m_missionEfficiencyText->SetVisible(false);

	NetworkId const & missionNid = getSelectedMission();
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;
	CreatureObject const * const creatureObj = Game::getPlayerCreature();
	if(!creatureObj)
		return;

	Object const * const o = NetworkIdManager::getObjectById(missionNid);
	ClientMissionObject const * const m = dynamic_cast<ClientMissionObject const *>(o);
	if(m)
	{
			Unicode::String tmp;
		//some mission types use more than one of these fields, so they're ifs, not else ifs
		if(
			missionType == "destroy" ||
			missionType == "deliver" ||
			missionType == "entertainer" ||
			missionType == "crafting" ||
			missionType == "assassin"
			)
		{
			m_missionStartLocText->SetVisible(true);
			CuiMissionManager::formatLocationString (tmp, m->getStartLocation());
			m_missionStartLocText->SetLocalText(CuiStringIdsQuest::startloc.localize() + tmp);
		}
		if(
			missionType == "deliver" ||
			missionType == "crafting"
			)
		{
			m_missionDestLocText->SetVisible(true);
			CuiMissionManager::formatLocationString (tmp, m->getEndLocation());
			m_missionDestLocText->SetLocalText(CuiStringIdsQuest::startloc.localize() + tmp);
		}
		if(
			missionType == "survey"
			)
		{
			m_missionEfficiencyText->SetVisible(true);
			IGNORE_RETURN (UIUtils::FormatLong (tmp, m->getDifficulty()));
			m_missionEfficiencyText->SetLocalText(CuiStringIdsQuest::efficiency.localize() + tmp);
		}
	}
}

//----------------------------------------------------------------------

/**
 * Update the list of active or completed tasks for the selected quest
 */
void SwgCuiQuestJournal::updateDecriptionTasks(Quest const & quest)
{
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;

	for(int taskId = 0; taskId < quest.getNumberOfTasks(); ++taskId)
	{
		QuestTask const * const qt = quest.getTask(taskId);
		if(!qt->isVisible())
			continue;

		bool taskActive     = playerObj->questHasActiveQuestTask(quest.getId(), taskId);
		bool taskCompleted  = playerObj->questHasCompletedQuestTask(quest.getId(), taskId);
		bool questCompleted = playerObj->questHasCompletedQuest(quest.getId());

		if(Game::getSinglePlayer())
		{
			taskCompleted = (taskId % 2 == 0);
			taskActive    = (taskId % 2 == 1);
		}

		//we might not show the task, based on the data it holds
		bool showTask = false;

		if(taskActive || taskCompleted)
		{
			UIComposite * const dupePage = safe_cast<UIComposite *>(m_tasksSample->DuplicateObject ());
			NOT_NULL (dupePage);
			dupePage->SetVisible(true);

			QuestTask const * const task = quest.getTask(taskId);

			//if godmode, show the taskid and name as a tooltip
			Unicode::String taskTooltip;
			if(playerObj->isAdmin()) //lint !e1705
			{
				static char buffer[16];
				_itoa(task->getId(), buffer, 10);
				taskTooltip += Unicode::narrowToWide("(TaskID: ") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide(", Task Name: ") + Unicode::narrowToWide(")");
			}
			dupePage->SetTooltip(taskTooltip);
			dupePage->SetTooltipDelay(false);

			UIPage * const p = dynamic_cast<UIPage *>(dupePage->GetChild("page"));
			NOT_NULL(p);
			UIText * const textTaskName = dynamic_cast<UIText *>(p->GetChild("texttask"));
			NOT_NULL(textTaskName);
			textTaskName->Clear();
			UIText * const textTaskDesc = dynamic_cast<UIText *>(dupePage->GetChild("description"));
			NOT_NULL(textTaskDesc);
			textTaskDesc->Clear();
			UIText * const sampleCounter = dynamic_cast<UIText *>(dupePage->GetChild("labelCounter"));
			NOT_NULL(sampleCounter);
			sampleCounter->Clear();
			sampleCounter->SetVisible(false);

			Unicode::String taskTextPre;
			if(taskCompleted)
				taskTextPre += Unicode::narrowToWide(cms_colorGreyStr);
			textTaskName->SetLocalText(taskTextPre + task->getJournalEntryTitle().localize());
			textTaskDesc->SetLocalText(taskTextPre + task->getJournalEntryDescription().localize());
		
			//pack the page now to get textTaskDesc set to the right value
			dupePage->Pack();
		
			//space quests have "invisible tasks" that tag themselves with an empty task name
			if((!textTaskName->GetLocalText().empty()) && (textTaskName->GetLocalText() != Unicode::narrowToWide("[]:")))
				showTask = true;

			UIImage * const completedImage = dynamic_cast<UIImage *>(p->GetChild("completedImage"));
			NOT_NULL(completedImage);
			completedImage->SetVisible(taskCompleted || questCompleted);

			UIScalar pageHeight = p->GetHeight() + textTaskDesc->GetHeight() + cms_taskSpacerHeight;
			//make minimum and maximum page sizes grow to fit new widget
			UISize minimumSize = dupePage->GetMinimumSize();
			minimumSize.y = pageHeight;
			dupePage->SetMinimumSize(minimumSize);
			UISize maximumSize = dupePage->GetMaximumSize();
			maximumSize.y = pageHeight;
			dupePage->SetMaximumSize(maximumSize);
			//now make the actual page the right size
			dupePage->SetHeight(pageHeight);

			//add the counter or timer to the task widget
			int numberOfCounters = 0;
			if(!questCompleted && !taskCompleted)
			{
				std::vector<QuestJournalManager::CounterData> const & result = QuestJournalManager::getCounters(quest.getId(), taskId);
				for(std::vector<QuestJournalManager::CounterData>::const_iterator ii = result.begin(); ii != result.end(); ++ii)
				{
					UIText * newCounter = safe_cast<UIText *>(sampleCounter->DuplicateObject ());
					newCounter->SetVisible(true);
					dupePage->AddChild(newCounter);
					//set below last existing counter
					if(ms_lastCounter)
					{
						UIPoint counterLocation = ms_lastCounter->GetLocation();
						counterLocation.y += ms_lastCounter->GetHeight();
						newCounter->SetLocation(counterLocation);
					}
					pageHeight += newCounter->GetHeight();

					//make minimum and maximum page sizes grow to fit new widget
					UISize minimumSize2 = dupePage->GetMinimumSize();
					minimumSize2.y = pageHeight;
					dupePage->SetMinimumSize(minimumSize2);
					UISize maximumSize2 = dupePage->GetMaximumSize();
					maximumSize2.y = pageHeight;
					dupePage->SetMaximumSize(maximumSize2);
					//now make the actual page the right size
					dupePage->SetHeight(pageHeight);

					QuestJournalManager::CounterData const data = *ii;
					Unicode::String resultStr = StringId(Unicode::wideToNarrow(data.sourceName)).localize();
					resultStr += Unicode::narrowToWide(": ");
					char buffer[256];
					_itoa(data.counter, buffer, 10);
					resultStr += Unicode::narrowToWide(buffer);
					resultStr += Unicode::narrowToWide("/");
					_itoa(data.counterMax, buffer, 10);
					resultStr += Unicode::narrowToWide(buffer);
					newCounter->SetLocalText(resultStr);
					ms_lastCounter = newCounter;
					++numberOfCounters;
				}

				std::vector<QuestJournalManager::TimerData> const & timerResult = QuestJournalManager::getTimers(quest.getId(), taskId);
				for(std::vector<QuestJournalManager::TimerData>::const_iterator ii2 = timerResult.begin(); ii2 != timerResult.end(); ++ii2)
				{
					UIText * newCounter = safe_cast<UIText *>(sampleCounter->DuplicateObject ());
					newCounter->SetVisible(true);
					dupePage->AddChild(newCounter);
					//set below last existing counter
					if(ms_lastCounter)
					{
						UIPoint counterLocation = ms_lastCounter->GetLocation();
						counterLocation.y += ms_lastCounter->GetHeight();
						newCounter->SetLocation(counterLocation);
					}
					pageHeight += newCounter->GetHeight();

					//make minimum and maximum page sizes grow to fit new widget
					UISize minimumSize2 = dupePage->GetMinimumSize();
					minimumSize2.y = pageHeight;
					dupePage->SetMinimumSize(minimumSize2);
					UISize maximumSize2 = dupePage->GetMaximumSize();
					maximumSize2.y = pageHeight;
					dupePage->SetMaximumSize(maximumSize2);
					//now make the actual page the right size
					dupePage->SetHeight(pageHeight);

					QuestJournalManager::TimerData const data = *ii2;
					Unicode::String resultStr = StringId(Unicode::wideToNarrow(data.sourceName)).localize();

					int const playedTimeTimerEnd = data.playedTimeTimerEnd;
					int const playedTime = static_cast<int>(playerObj->getPlayedTime());
					int timeLeftSeconds = playedTimeTimerEnd - playedTime;
					timeLeftSeconds = std::max(0, timeLeftSeconds);
					Unicode::String timeWide;
					IGNORE_RETURN(CuiUtils::FormatTimeDuration(timeWide, static_cast<unsigned int>(timeLeftSeconds), false, false, true, true, true));
					resultStr += Unicode::narrowToWide(" ");
					resultStr += timeWide;
					resultStr += Unicode::narrowToWide(".");
					newCounter->SetLocalText(resultStr);
					ms_lastCounter = newCounter;
					ms_lastCounter = newCounter;
					++numberOfCounters;
				}
			}
			if(numberOfCounters == 0)
				sampleCounter->SetVisible(false);

			if (dupePage && showTask)
			{
				//set the name of the page to "task" so we can find and delete them later
				dupePage->SetName(cms_task);
				IGNORE_RETURN(m_descriptionComp->InsertChildBefore(dupePage, m_postTaskSpacer));
			}
		}
	}
}

//----------------------------------------------------------------------

/**
 * Update the reward portion of the quest description window
 */
void SwgCuiQuestJournal::updateDecriptionReward(Quest const & quest)
{
	static char buffer[256];

	//handle rewards
	std::string const & experienceType = quest.getExperienceRewardType();
	int const experienceAmount = quest.getExperienceRewardAmount();
	std::string const & factionType = quest.getFactionRewardType();
	int const factionAmount = quest.getFactionRewardAmount();
	int const credits = quest.getMoneyRewardCredits();
	bool showedSomeReward = false;
	
	if(!experienceType.empty() && experienceAmount != 0)
	{
		Unicode::String experienceText = CuiStringIdsQuest::experience.localize();
		_itoa(experienceAmount, buffer, 10);
		experienceText += Unicode::narrowToWide(buffer) + Unicode::narrowToWide("(");
		Unicode::String localizedExperienceType;
		IGNORE_RETURN(CuiSkillManager::localizeExpName(experienceType, localizedExperienceType));
		experienceText += localizedExperienceType + Unicode::narrowToWide(")");
		m_experienceRewardText->SetLocalText(experienceText);
		m_experienceRewardText->SetVisible(true);
		showedSomeReward = true;
	}
	else
	{
		m_experienceRewardText->SetVisible(false);
	}

	if(!factionType.empty() && factionAmount != 0)
	{
		Unicode::String factionText = CuiStringIdsQuest::faction.localize();
		_itoa(factionAmount, buffer, 10);
		factionText += Unicode::narrowToWide(buffer) + Unicode::narrowToWide("(");
		StringId const s("faction/faction_names", Unicode::toLower(factionType));
		factionText += s.localize() + Unicode::narrowToWide(")");
		m_factionRewardText->SetLocalText(factionText);
		m_factionRewardText->SetVisible(true);
		showedSomeReward = true;
	}
	else
	{
		m_factionRewardText->SetVisible(false);
	}

	if(credits != 0)
	{
		Unicode::String creditsText = CuiStringIdsQuest::money.localize();
		_itoa(credits, buffer, 10);
		creditsText += Unicode::narrowToWide(buffer) + CuiStringIdsQuest::credits.localize();
		m_moneyRewardText->SetLocalText(creditsText);
		m_moneyRewardText->SetVisible(true);
		showedSomeReward = true;
	}
	else
	{
		m_moneyRewardText->SetVisible(false);
	}

	for(int i2 = 0; i2 < cms_maxItems; ++i2)
	{
		m_rewardItemPages[i2]->SetVisible(false);
	}

	int currentItem = 0;
	std::vector<std::string> const & inclusiveLootItemNames = quest.getInclusiveStaticLootItemNames();
	std::vector<std::string> const & exclusiveLootItemNames = quest.getExclusiveStaticLootItemNames();
	std::vector<int> const & inclusiveLootCounts = quest.getInclusiveStaticLootItemCounts();
	std::vector<int> const & exclusiveLootCounts = quest.getExclusiveStaticLootItemCounts();
	DEBUG_FATAL(!inclusiveLootItemNames.empty() && !exclusiveLootItemNames.empty(), ("Both AND and OR rewards type set in quest [%s], this is not allowed.  Use only one!", quest.getName().getString()));
	std::vector<std::string> const & lootItems = (!inclusiveLootItemNames.empty()) ? inclusiveLootItemNames : exclusiveLootItemNames;;
	std::vector<int> const & lootCounts = (!inclusiveLootItemNames.empty()) ? inclusiveLootCounts : exclusiveLootCounts;

	if(!inclusiveLootItemNames.empty())
	{
		m_inclusiveText->SetVisible(true);
		m_exclusiveText->SetVisible(false);
		m_chooseOneText->SetVisible(false);
	}
	else if(!exclusiveLootItemNames.empty())
	{
		m_inclusiveText->SetVisible(false);
		m_exclusiveText->SetVisible(true);
		m_chooseOneText->SetVisible(false);
	}
	else
	{
		m_inclusiveText->SetVisible(false);
		m_exclusiveText->SetVisible(false);
		m_chooseOneText->SetVisible(false);
	}

	for(std::vector<std::string>::const_iterator it = lootItems.begin(); it != lootItems.end(); ++it)
	{
		bool showedItem = false;
		std::string const & lootItem = *it;

		m_rewardItemViewers[currentItem]->setObject(NULL);
		delete ms_itemRewardObjects[currentItem];
		m_rewardItemPages[currentItem]->SetVisible(true);

		if(!lootItem.empty())
		{
			CuiStaticLootItemManager::ItemDictionary const * const dataDict = CuiStaticLootItemManager::getItemData(lootItem);
			if(dataDict)
			{
				CuiStaticLootItemManager::ItemDictionary::const_iterator const i = dataDict->find(Unicode::narrowToWide("template_name"));
				if(i != dataDict->end())
				{
					std::string const & sharedTemplateName = convertServerObjectTemplateNameToSharedObjectTemplateName(Unicode::wideToNarrow(i->second));
					ConstCharCrcString const & crcString = ObjectTemplateList::lookUp(sharedTemplateName.c_str());
					ms_itemRewardObjects[currentItem] = ObjectTemplateList::createObject(crcString);
					if(ms_itemRewardObjects[currentItem])
					{
						ClientObject * const co = ms_itemRewardObjects[currentItem]->asClientObject();
						if(co)
							co->endBaselines();
						RenderWorld::addObjectNotifications(*ms_itemRewardObjects[currentItem]);
					}
					showedItem = true;
				}
				else
				{
					DEBUG_WARNING(true, ("SwgCuiQuestJournal::updateDecriptionReward: items [%s] has itemData, but not a template_name", lootItem.c_str()));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("SwgCuiQuestJournal::updateDecriptionReward: items [%s] does not exist in CuiStaticLootItemManager", lootItem.c_str()));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("SwgCuiQuestJournal::updateDecriptionReward: LootItem is empty"));
		}

		if(!showedItem)
		{
			ms_needsUpdateIn = cms_itemWaitTime;
			ms_itemRewardObjects[currentItem] = NULL;
		}

		m_rewardItemViewers[currentItem]->setObject(ms_itemRewardObjects[currentItem]);
		Unicode::String const & result = CuiStaticLootItemManager::getTooltipAttributeString(lootItem);
		m_rewardItemViewers[currentItem]->GetParentWidget()->GetParentWidget()->GetParentWidget()->SetLocalTooltip(result);
		m_rewardItemViewers[currentItem]->recomputeZoom();
		m_rewardItemViewers[currentItem]->setViewDirty(true);
		
		int p = std::distance(lootItems.begin(), it);
		if (lootCounts[p] > 1) {
			char pr[16];
			snprintf(pr, sizeof(pr) - 1, "%d", lootCounts[p]);
			std::string countMessage = "(";
			countMessage += pr;
			countMessage += "x) ";
			m_rewardItemNames[currentItem]->SetLocalText(Unicode::narrowToWide(countMessage) + StringId("static_item_n", lootItem).localize());
		}
		else {
			m_rewardItemNames[currentItem]->SetLocalText(StringId("static_item_n", lootItem).localize());
		}

		showedSomeReward = true;
		++currentItem;
	}

	m_rewardLabel->SetVisible(showedSomeReward);
}

//----------------------------------------------------------------------

/**
 * Update the upper, quest list view
 */
void SwgCuiQuestJournal::updateQuestTree()
{
	ms_internalTreeUpdating = true;

	std::map<Unicode::String, std::vector<Quest const *> > questGroups;

	const PlayerObject * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;

	std::vector<Quest const *> allQuests;

	//in single player, grab some random quest to play with
	if(Game::getSinglePlayer())
	{
		allQuests.push_back(QuestManager::getQuest("quest/test_blood_sample"));
		allQuests.push_back(QuestManager::getQuest("quest/test_craft"));
		allQuests.push_back(QuestManager::getQuest("quest/test_destroy_multiple"));
		allQuests.push_back(QuestManager::getQuest("quest/test_encounter"));
		allQuests.push_back(QuestManager::getQuest("quest/test_escort"));
		allQuests.push_back(QuestManager::getQuest("quest/test_go_to_location"));
		allQuests.push_back(QuestManager::getQuest("quest/test_perform"));
		allQuests.push_back(QuestManager::getQuest("quest/test_various"));
		allQuests.push_back(QuestManager::getQuest("quest/test_various2"));
	}
	else
	{
		playerObj->questGetAllQuests(allQuests);
	}

	for(std::vector<Quest const *>::const_iterator i = allQuests.begin(); i != allQuests.end(); ++i)
	{
		Quest const * const q = *i;
		NOT_NULL(q);
		if(!q) //lint !e774 always false, no it isn't
			continue;
		Unicode::String grouping;
		if(m_sortType == ST_category)
		{
			grouping = q->getCategory().localize();
		}
		else if(m_sortType == ST_type)
		{
			std::string const & type = q->getType();
			StringId sid("ui_quest", type);
			grouping = sid.localize();
		}
		else if(m_sortType == ST_level)
		{
			int const level = q->getLevel();
			char buffer[10];
			_itoa(level, buffer, 10);
			grouping = CuiStringIdsQuest::level.localize() + Unicode::narrowToWide(buffer);
		}
		else
		{
			DEBUG_FATAL(true,(""));
		}

		//okay to create the entry if it isn't there
		std::vector<Quest const *> quests = questGroups[grouping];
		quests.push_back(q);
		questGroups[grouping] = quests;
	}

	std::vector<NetworkId> const & activeMissions = CuiMissionManager::getActiveMissions();

	if(m_sortType == ST_category && activeMissions.size() > 0)
	{
		questGroups[CuiStringIdsQuest::terminal_missions.localize()] = std::vector<Quest const *>();
	}

	std::vector<NetworkId> const & activePlayerCreatedQuests = CuiPlayerQuestManager::getActivePlayerQuests();
	if(m_sortType == ST_category && activePlayerCreatedQuests.size() > 0)
	{
		questGroups[CuiStringIdsQuest::player_quest.localize()] = std::vector<Quest const *>();
	}

	//empty the tree
	UIDataSourceContainer * const mainDsc = NON_NULL(m_questTree->GetDataSourceContainer());
	mainDsc->Attach(0);
	m_questTree->SetDataSourceContainer(0);
	mainDsc->Clear();
	m_questTree->SetPropertyBoolean(UITreeView::PropertyName::ShowIcon, true);

	if(playerObj == NULL)
		return;

	int randomCounter = 0;

	//for each quest "group" (as picked above), add a folder and put all the quests in that group as entries in the tree
	for(std::map<Unicode::String, std::vector<Quest const *> >::const_iterator i2 = questGroups.begin(); i2 != questGroups.end(); ++i2)
	{
		Unicode::String const & grouping = i2->first;
		std::vector<Quest const *> const & quests = i2->second;
		UIDataSourceContainer * const groupDsc = new UIDataSourceContainer;
		groupDsc->SetProperty(UILowerString("Name"), grouping);
		groupDsc->SetProperty(UITreeView::DataProperties::Text, grouping);
		groupDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, true);
		groupDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
		
		if(grouping == CuiStringIdsQuest::terminal_missions.localize())
		{
			for(std::vector<NetworkId>::const_iterator j = activeMissions.begin(); j != activeMissions.end(); ++j)
			{
				Object const * const o = NetworkIdManager::getObjectById(*j);
				ClientMissionObject const * const m = dynamic_cast<ClientMissionObject const *>(o);
				if(m && !m->isSpaceMission())
				{
					UIDataSourceContainer * const questDsc = new UIDataSourceContainer;
					questDsc->SetProperty(UITreeView::DataProperties::Text, m->getTitle().localize());
					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, true);
					questDsc->SetPropertyInteger(UITreeView::DataProperties::ColorIndex, 1);
					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
					questDsc->SetProperty(MissionIdProperty, Unicode::narrowToWide(m->getNetworkId().getValueString()));
					groupDsc->AddChild(questDsc);
				}
			}
		}
		else if (grouping == CuiStringIdsQuest::player_quest.localize())
		{
			for(std::vector<NetworkId>::const_iterator iter = activePlayerCreatedQuests.begin(); iter != activePlayerCreatedQuests.end(); ++iter)
			{
				Object const * const pqObj = NetworkIdManager::getObjectById(*iter);
				ClientPlayerQuestObject const * const playerQuest = dynamic_cast<ClientPlayerQuestObject const *>(pqObj);
				if(playerQuest)
				{
					CreatureObject const * const creatureObj = Game::getPlayerCreature();
					if(!creatureObj)
						continue;

					int const playerLevel = creatureObj->getGroupLevel();
					UIColor const & conColor = CuiCombatManager::getConColor(playerLevel, playerQuest->getQuestDifficulty());
					std::string colorResult;
					UIUtils::FormatColor(colorResult, conColor);
					Unicode::String const & titleText = Unicode::narrowToWide(cms_slashes + colorResult) + Unicode::narrowToWide(playerQuest->getQuestTitle());

					UIDataSourceContainer * const questDsc = new UIDataSourceContainer;
					questDsc->SetProperty(UITreeView::DataProperties::Text, titleText);
					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, true);
					questDsc->SetPropertyInteger(UITreeView::DataProperties::ColorIndex, 1);
					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
					questDsc->SetProperty(PlayerQuestIdProperty, Unicode::narrowToWide(playerQuest->getNetworkId().getValueString()));
					groupDsc->AddChild(questDsc);
				}
			}
		}
		else
		{
			//for each quest in the group, add an entry
			for(std::vector<Quest const *>::const_iterator i3 = quests.begin(); i3 != quests.end(); ++i3)
			{
				Quest const * const q = *i3;
				NOT_NULL(q);
				if(!q) //lint !e774 always false, no it isn't
					continue;

				bool canGetReward = false;
				PlayerQuestData const * const playerQuestData = playerObj->getQuestData(q->getId());
				if(playerQuestData)
					canGetReward = !playerQuestData->hasReceivedReward();
				else
					canGetReward = false;

				bool questIsActive = playerObj->questHasActiveQuest(q->getId());
				//in single player, make some quests active
				if(Game::getSinglePlayer())
				{
					questIsActive =(randomCounter++) % 2;
				}

				//only show completed quests if they want them.
				if(!m_showCompletedCheckbox->IsChecked() && !questIsActive && !canGetReward)
					continue;

				if(q->isVisible())
				{
					//show quest type if not solo
					Unicode::String typeWide;
					if(m_sortType != ST_type)
					{
						std::string const & type = q->getType();
						if((!q->getType().empty()) && (q->getType() != cms_soloType))
						{
							StringId typeSid("quest/quest_types", type);
							typeWide = Unicode::narrowToWide(" [") + typeSid.localize() + Unicode::narrowToWide("]");
						}
					}
					Unicode::String const & questName = q->getJournalEntryTitle().localize();
					std::string const & questColor = getQuestColor(q->getId());
					Unicode::String title = Unicode::narrowToWide(questColor.c_str());
					title += questName + typeWide;

					if(playerQuestData)
					{
						if(playerQuestData->isCompleted() && !playerQuestData->hasReceivedReward())
						{
							title += CuiStringIdsQuest::click_to_complete.localize();
						}
					}

					if(playerObj->isAdmin()) //lint !e1705 accessing static member
					{
						title += Unicode::narrowToWide("(") + Unicode::narrowToWide(q->getName().getString()) + Unicode::narrowToWide(")");
					}

					UIDataSourceContainer * const questDsc = new UIDataSourceContainer;
					questDsc->SetProperty(UITreeView::DataProperties::Text, title);

					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, true);
					if(playerObj->questHasActiveQuest(q->getId()))
					{
						questDsc->SetPropertyInteger(UITreeView::DataProperties::ColorIndex, 1);
					}
					else
					{
						questDsc->SetPropertyInteger(UITreeView::DataProperties::ColorIndex, 2);
						questDsc->SetPropertyNarrow(UITreeView::DataProperties::Icon, cms_completedIconStyle);
					}
					questDsc->SetPropertyBoolean(CanGetRewardProperty, canGetReward);
					questDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
					questDsc->SetProperty(QuestNameProperty, Unicode::narrowToWide(q->getName().getString()));
					groupDsc->AddChild(questDsc);
				}
			}
		}
		mainDsc->AddChild(groupDsc);
	}

	//now, get rid of any groupings we didn't actually use
	bool done = false;
	while(!done)
	{
		int const childrenCount = static_cast<int>(mainDsc->GetChildCount());
		if(childrenCount == 0)
		{
			done = true;
		}

		for(int i3 = 0; i3 < childrenCount; ++i3)
		{
			UIDataSourceBase * const data = mainDsc->GetChildByPositionLinear(static_cast<unsigned long>(i3));
			if(data && data->GetChildCount() == 0)
			{
				mainDsc->RemoveChild(data);
				break;
			}
			if(i3 == childrenCount-1)
			{
				done = true;
			}
		}
	}

	m_questTree->SetDataSourceContainer(mainDsc);
	mainDsc->Detach(0);

	//since this can get called after performActivate(), make sure that we expand the category list on the first open frame
	if(ms_isFirstOpen)
		expandAllCategories(true);

	ms_internalTreeUpdating = false;
}

//----------------------------------------------------------------------

/**
 * Return the name of the quest selected in the quest list window
 */
const std::string SwgCuiQuestJournal::getSelectedQuest() const
{
	long const selectedRow = m_questTree->GetLastSelectedRow();
	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(selectedRow);
	if(!selectedDsc)
	{
		static std::string emptyString;
		return emptyString;
	}
	std::string selectedQuest;
	selectedDsc->GetPropertyNarrow(QuestNameProperty, selectedQuest);
	return selectedQuest;
}

//----------------------------------------------------------------------

bool SwgCuiQuestJournal::selectedQuestCanClaimReward() const
{
	long const selectedRow = m_questTree->GetLastSelectedRow();
	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(selectedRow);
	if(selectedDsc)
	{
		uint32 const questCrc = getSelectedQuestCrc();
		PlayerObject const * const playerObj = Game::getPlayerObject();
		if(playerObj)
		{
			bool const questIsComplete = playerObj->questHasCompletedQuest(questCrc);
			if(questIsComplete)
			{
				bool canClaimReward = false;
				selectedDsc->GetPropertyBoolean(CanGetRewardProperty, canClaimReward);
				return canClaimReward;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

std::string const SwgCuiQuestJournal::getQuestAtRow(int const row) const
{
	static std::string emptyString;
	if(row < 0)
		return emptyString;

	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(row);
	if(!selectedDsc)
	{
		return emptyString;
	}
	std::string selectedQuest;
	selectedDsc->GetPropertyNarrow(QuestNameProperty, selectedQuest);
	return selectedQuest;
}

//----------------------------------------------------------------------

/**
 * Return the Crc of the quest selected in the quest list window
 */
uint32 const SwgCuiQuestJournal::getSelectedQuestCrc() const
{
	return Crc::calculate(getSelectedQuest().c_str());
}

//----------------------------------------------------------------------

bool SwgCuiQuestJournal::isSelectedQuestAMission() const
{
	long const selectedRow = m_questTree->GetLastSelectedRow();
	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(selectedRow);
	if(!selectedDsc)
	{
		return false;
	}
	std::string missionId;
	selectedDsc->GetPropertyNarrow(MissionIdProperty, missionId);
	return !missionId.empty();
}

//----------------------------------------------------------------------

NetworkId SwgCuiQuestJournal::getSelectedMission() const
{
	long const selectedRow = m_questTree->GetLastSelectedRow();
	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(selectedRow);
	if(!selectedDsc)
	{
		return NetworkId();
	}
	std::string missionId;
	selectedDsc->GetPropertyNarrow(MissionIdProperty, missionId);
	NetworkId const nid(missionId);
	return nid;
}

//----------------------------------------------------------------------

NetworkId SwgCuiQuestJournal::getSelectedPlayerQuest() const
{
	long const selectedRow = m_questTree->GetLastSelectedRow();
	UIDataSourceContainer const * selectedDsc = m_questTree->GetDataSourceContainerAtRow(selectedRow);
	if(!selectedDsc)
	{
		return NetworkId();
	}
	std::string pqId;
	selectedDsc->GetPropertyNarrow(PlayerQuestIdProperty, pqId);
	NetworkId const nid(pqId);
	return nid;
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::OnGenericSelectionChanged(UIWidget * context)
{
	if(context == m_questTree)
	{
		updateDescription();
		char questCrcString[64];
		uint32 const questCrc = getSelectedQuestCrc();
		_itoa(static_cast<int>(questCrc), questCrcString, 10);
		NetworkId const & missionNid = getSelectedMission();
		NetworkId const & playerQuestId = getSelectedPlayerQuest();
		if(missionNid.isValid())
		{
			QuestJournalManager::setSelectedMissionNid(missionNid);
		}
		else if (playerQuestId.isValid())
		{
			QuestJournalManager::setSelectedPlayerQuestId(playerQuestId);
		}
		else
		{
			QuestJournalManager::setSelectedQuestCrc(questCrc);
		}
		IGNORE_RETURN(CuiActionManager::performAction(CuiActions::questHelper, Unicode::emptyString));

		if(selectedQuestCanClaimReward())
			requestOpenCompletionWindow(questCrc);
	}
	else if(context == m_sortCombo)
	{
		std::string selectedName;
		m_sortCombo->GetSelectedIndexName(selectedName);
		if(selectedName == "Level")
		{
			m_sortType = ST_level;
		}
		else if(selectedName == "Category")
		{
			m_sortType = ST_category;
		}
		else if(selectedName == "Type")
		{
			m_sortType = ST_type;
		}

		ms_openedRows.clear();

		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::OnButtonPressed(UIWidget * context)
{
	if(context == m_okButton)
	{
		closeThroughWorkspace();
	}
	else if(context == m_buttonAbandonQuest)
	{
		CuiMessageBox * const messageBox = CuiMessageBox::createYesNoBox(CuiStringIds::quest_abandon_question.localize());
		m_callback->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiQuestJournal::onQuestAbandonResponse);
	}
	else if(context == m_buttonExpandAll)
	{
		UIString value;
		IGNORE_RETURN(m_buttonExpandAll->GetProperty(UITreeView::DataProperties::Text, value));
		expandAllCategories(value == cms_expandAllStyle);
	}
	else if(context == m_abandonMissionButton)
	{
		NetworkId const & nid = getSelectedMission();
		if(nid.isValid())
		{
			Object const * const o = NetworkIdManager::getObjectById(nid);
			ClientMissionObject const * const m = dynamic_cast<ClientMissionObject const *>(o);
			if(m)
			{
				CuiMissionManager::removeMission(m->getNetworkId(), false);
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiQuestJournal::onQuestAbandonResponse(const CuiMessageBox &messageBox)
{
	if(messageBox.completedAffirmative())
	{
		std::string const & selectedQuest = getSelectedQuest();
		if(!selectedQuest.empty())
		{
			//send up "0 <questname>", the 0 is an enumeration of the quest "type", i.e. mission terminal quest, datapad quest, etc.  Only 0 is currently used. 
			ClientCommandQueue::enqueueCommand("abandonQuest",  NetworkId::cms_invalid, Unicode::narrowToWide("0 ") + Unicode::narrowToWide(selectedQuest));
		}

		NetworkId const & pqId = getSelectedPlayerQuest();
		if(pqId.isValid())
		{
			ClientObject * const player = Game::getClientPlayer ();
			Controller * const controller = player ? NON_NULL (player->getController()) : 0;

			if (!controller)
				return;

			controller->appendMessage (
				CM_abandonPlayerQuest, 
				0.0f, 
				new MessageQueueGenericValueType<NetworkId>(pqId), 
				GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
		}
	}
} //lint !e1762 can be const, it can't because it's used in a connect call

//----------------------------------------------------------------------

void SwgCuiQuestJournal::OnCheckboxSet(UIWidget * context)
{
	if(context == m_showCompletedCheckbox)
	{
		saveSettings();
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::OnCheckboxUnset(UIWidget * context)
{
	if(context == m_showCompletedCheckbox)
	{
		saveSettings();
		updateAll();
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::saveSettings() const
{
	CuiMediator::saveSettings();
	CuiSettings::saveBoolean(getMediatorDebugName(), Settings::s_completed, m_showCompletedCheckbox->IsChecked());
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::loadSettings()
{
	CuiMediator::loadSettings();
	
	bool completedCheckbox = false;
	if(CuiSettings::loadBoolean(getMediatorDebugName(), Settings::s_completed, completedCheckbox))
	{
		m_showCompletedCheckbox->SetChecked(completedCheckbox);
		updateAll();
	}
}

//----------------------------------------------------------------------

/**
 * Delete the active counters and timers
 */
void SwgCuiQuestJournal::deleteTaskData()
{
	for(std::vector<UIText *>::iterator i = ms_currentCounters.begin(); i != ms_currentCounters.end(); ++i)
	{
		UIText * const t = *i;
		m_descriptionComp->RemoveChild(t);
	}
	ms_currentCounters.clear();

	for(std::vector<UIText *>::iterator i2 = ms_currentTimers.begin(); i2 != ms_currentTimers.end(); ++i2)
	{
		UIText * const t = *i2;
		m_descriptionComp->RemoveChild(t);
	}
	ms_currentTimers.clear();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onCountersChanged(const bool & payload)
{
	UNREF(payload);
	updateDescription();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::onTimersChanged(const bool & payload)
{
	UNREF(payload);
	updateDescription();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::update(float const deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	updateTimers(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::updateTimers(float const deltaTimeSecs)
{
	ms_timeSinceUpdate += deltaTimeSecs;
	if(ms_timeSinceUpdate >= cms_updateTime)
	{
		if(isActive())
			updateDescription();
		ms_timeSinceUpdate = 0.0f;
	}
}

//----------------------------------------------------------------------

/**
 * Compare the player level to the quest level, and return the quest color
 */
std::string SwgCuiQuestJournal::getQuestColor(uint32 const questCrc)
{
	Quest const * const q = QuestManager::getQuest(questCrc);
	CreatureObject const * const playerObj = Game::getPlayerCreature();
	if(!playerObj || !q || (q->getLevel() <= 0))
		return cms_colorWhiteStr;

	int const playerLevel = playerObj->getGroupLevel();

	UIColor const & conColor = CuiCombatManager::getConColor(playerLevel, q->getLevel());
 
	std::string colorResult;
	UIUtils::FormatColor(colorResult, conColor);
	return cms_slashes + colorResult;
}

//----------------------------------------------------------------------

/**
 * Examine the quest description window for UI pages that represent tasks, and delete them.
 */
void SwgCuiQuestJournal::clearTaskPages()
{
	//all task pages are named "task" so find and delete them
	UIBaseObject * taskPage = m_descriptionComp->GetChild(cms_task.c_str());
	while(taskPage)
	{
		m_descriptionComp->RemoveChild(taskPage);
		taskPage = m_descriptionComp->GetChild(cms_task.c_str());
	}
}

//----------------------------------------------------------------------

/**
 * Expand or collapse all the quest category nodes
 */
void SwgCuiQuestJournal::expandAllCategories(bool const expand)
{
	int const childrenCount = static_cast<int>(m_questTree->GetRowCount());
	for(int i = 0; i < childrenCount; ++i)
	{
		m_questTree->SetRowExpanded(i, expand);
		storeRowExpanded(expand, i);
	}

	Unicode::String const & newStyle = expand ? cms_collapseAllStyle : cms_expandAllStyle;
	IGNORE_RETURN(m_buttonExpandAll->SetProperty(UITreeView::DataProperties::Text, newStyle));
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::storeRowExpanded(bool const isExpanded, int const row)
{
	UIDataSourceContainer * const dsrc = m_questTree->GetDataSourceContainerAtRow(row);
	if(dsrc)
	{
		UIString s;
		dsrc->GetProperty(UILowerString("Name"), s);
		storeRowExpanded(isExpanded, Unicode::wideToNarrow(s));
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::storeRowExpanded(bool const isExpanded, std::string const & rowName)
{
	if(isExpanded)
	{
		ms_openedRows.insert(rowName);
	}
	else
	{
		ms_openedRows.erase(rowName);
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::OnTreeRowExpansionToggled(UIWidget * context, int row)
{
	if(context == m_questTree)
	{
		if(ms_internalTreeUpdating)
			return;

		bool const isExpanded = m_questTree->GetRowExpanded(row);
		storeRowExpanded(isExpanded, row);
	}
}


//----------------------------------------------------------------------

void SwgCuiQuestJournal::requestOpenCompletionWindow(uint32 const questCrc)
{
	Unicode::String params;
	params =  Unicode::narrowToWide("1");
	params += Unicode::narrowToWide(" ");
	char buffer[64];
	_itoa(questCrc, buffer, 10);
	params += Unicode::narrowToWide(buffer);
	CuiActionManager::performAction(CuiActions::questAcceptance, params);
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::updatePlayerQuestDescription()
{
	NetworkId const & pqId = getSelectedPlayerQuest();
	PlayerObject const * const playerObj = Game::getPlayerObject();
	if(!playerObj)
		return;
	CreatureObject const * const creatureObj = Game::getPlayerCreature();
	if(!creatureObj)
		return;

	clearPlayerQuestTasks();

	Object const * const o = NetworkIdManager::getObjectById(pqId);
	ClientPlayerQuestObject const * const pqObj = dynamic_cast<ClientPlayerQuestObject const *>(o);
	if(pqObj)
	{

		int const playerLevel = creatureObj->getGroupLevel();
		UIColor const & conColor = CuiCombatManager::getConColor(playerLevel, pqObj->getQuestDifficulty());
		std::string colorResult;
		UIUtils::FormatColor(colorResult, conColor);
		Unicode::String const & titleText = Unicode::narrowToWide(cms_slashes + colorResult) + Unicode::narrowToWide(pqObj->getQuestTitle());

		m_playerQuestTitleText->SetLocalText(titleText);

		m_playerQuestDescriptionText->SetLocalText(Unicode::narrowToWide(pqObj->getQuestDescription()));

		m_playerQuestCreatorText->SetLocalText(CuiStringIdsQuest::creator.localize() + Unicode::narrowToWide(pqObj->getCreatorName()));

		m_playerQuestDifficultyText->SetLocalText(Unicode::intToWide(pqObj->getQuestDifficulty()));
	}

	for(int i = 0; i < pqObj->getTotalTasks(); ++i)
	{

		if(pqObj->getTaskStatus(i) == ClientPlayerQuestObject::TS_Inactive)
			continue;

		UIPage * newPage = dynamic_cast<UIPage * >(m_playerQuestTaskSample->DuplicateObject());
		NOT_NULL(newPage);

		UIImage * completedTask = dynamic_cast<UIImage *>(newPage->GetChild("completedImage"));
		NOT_NULL(completedTask);

		UIText * taskTitle = dynamic_cast<UIText *>(newPage->GetChild("textTask"));
		NOT_NULL(taskTitle);

		UIText * taskDesc = dynamic_cast<UIText *>(newPage->GetChild("textTaskDesc"));
		NOT_NULL(taskDesc);

		UIText * counterText = dynamic_cast<UIText *>(newPage->GetChild("labelCounter"));
		NOT_NULL(counterText);

		UIText * originalName = dynamic_cast<UIText *>(newPage->GetChild("originalName"));
		NOT_NULL(originalName);

		newPage->SetVisible(true);
		
		completedTask->SetVisible( pqObj->getTaskStatus(i) == ClientPlayerQuestObject::TS_Completed ? true : false );

		taskTitle->SetPreLocalized(true);
		taskDesc->SetPreLocalized(true);
		counterText->SetPreLocalized(true);
		originalName->SetPreLocalized(true);

		taskTitle->SetLocalText(Unicode::narrowToWide(pqObj->getTaskTitle(i)));
		taskDesc->SetLocalText(Unicode::narrowToWide(pqObj->getTaskDescription(i)));

		std::pair<int, int> const & counters = pqObj->getTaskCounterPair(i);

		if(counters.first >= 0 && counters.second >= 0)
		{
			counterText->SetVisible(true);
			char buffer[64];
			memset(buffer, 0, 64);
			sprintf(buffer, "%d / %d", counters.first, counters.second);

			counterText->SetLocalText(Unicode::narrowToWide(buffer));
		}
		else
			counterText->SetVisible(false);

		newPage->SetName(cms_task);

		UIComposite * pageComp = dynamic_cast<UIComposite *>(newPage);
		NOT_NULL(pageComp);
		pageComp->Pack();

		if(pqObj->isRecipe())
		{
			std::string recipeData = pqObj->getTaskData(i);
			const char delimit = '~';
			std::string::size_type loc = recipeData.find(delimit);
			std::string relicTask;
			if(loc != std::string::npos)
				relicTask = recipeData.substr(0, loc);
			else
				relicTask = recipeData;

			StringId taskName("collection_n", relicTask.c_str());
			UIString displayString = Unicode::narrowToWide("[") + taskName.localize() + Unicode::narrowToWide("]");
			
			originalName->SetLocalText(displayString);
			
		}
		else
		{
			StringId taskName("collection_n", pqObj->getTaskData(i).c_str());
			UIString displayString = Unicode::narrowToWide("[") + taskName.localize() + Unicode::narrowToWide("]");

			originalName->SetLocalText(displayString);
		}

		m_playerQuestTaskComposite->AddChild(newPage);
	}

	m_playerQuestDescriptionComp->Link();
}

//----------------------------------------------------------------------

void SwgCuiQuestJournal::clearPlayerQuestTasks()
{
	UIBaseObject * taskPage = m_playerQuestTaskComposite->GetChild(cms_task.c_str());
	while(taskPage)
	{
		m_playerQuestTaskComposite->RemoveChild(taskPage);
		taskPage = m_playerQuestTaskComposite->GetChild(cms_task.c_str());
	}
}

//----------------------------------------------------------------------