//======================================================================
//
// SwgCuiQuestBuilder.cpp
// copyright(c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiQuestBuilder.h"


#include "clientGame/ClientPlayerQuestObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientGame/PlayerObject.h"

#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiRecipeManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"


#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"

#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/TextManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueCreateSaga.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"

#include "StringId.h"
#include "UIBaseObject.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIList.h"
#include "UILowerString.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIWidget.h"

#include <vector>
#include <map>
// ---------------------------------------------------------------------

#define MAX_TASKS 12
#define MAX_TASKS_PER_PAGE 25

const UILowerString SwgCuiQuestBuilder::KillCreatureProperty = UILowerString("KillCreature");
const UILowerString SwgCuiQuestBuilder::KillSpeciesProperty = UILowerString("KillSpecies");
const UILowerString SwgCuiQuestBuilder::TemplateProperty = UILowerString("StoredTemplate");
const UILowerString SwgCuiQuestBuilder::FilterProperty = UILowerString("Filter");

namespace SwgCuiQuestBuilderNamespace
{
	int getSkillModValue(std::string const & skillModName)
	{
		CreatureObject const * const playerCreature = Game::getPlayerCreature();

		if(!playerCreature)
			return 0;

		CreatureObject::SkillModMap::const_iterator iterSkillModMap = playerCreature->getSkillModMap().find(skillModName);

		if (iterSkillModMap != playerCreature->getSkillModMap().end())
		{
			int const baseValue = iterSkillModMap->second.first;
			int const modValue = iterSkillModMap->second.second;

			return (baseValue + modValue);
		}
		return 0;
	}

	
	const char * ms_relicPage = "saga_relic_page";

	enum TaskType
	{
		TT_None = 0,
		TT_Kill,
		TT_Location,
		TT_Loot,
		TT_PvPKill,
		TT_Perform,
		TT_Comm,
		TT_Craft,
		TT_PvPDestroy,
		TT_KillLoot,
		TT_Spawn,
		TT_TotalTypes,
	};
	
	int ms_requiredFlags[] = { 1 << 0,
							1 << 1,
							1 << 2,
							1 << 3,
							1 << 4,
							1 << 5,
							1 << 6};

	int const ms_totalRequiredFlags = sizeof(ms_requiredFlags)/sizeof(ms_requiredFlags[0]);

	const char * const ms_requiredFlagColors[] = {"\\#FFFFFFCombat",   // Combat
												"\\#00FF00Crafting",   // Crafting
												"\\#00FFFFSocial",   // Social
												"\\#FF0000PvP",   // Pvp
												"\\#0000FFGroup", // Group 
												"\\#FFFF00Kashyyyk",   // Kashyyyk
												"\\#FF00FFMustafar",   // Mustafar
												}; 

	struct TaskInfo
	{
		TaskInfo()
		{
			taskPage = 0;
			closeButton = 0;
			moveUpButton = 0;
			moveDownButton = 0;
			captureLocationButton = 0;
		}

		TaskType taskType;
		UIPage * taskPage;
		UIButton * closeButton;
		UIButton * moveUpButton;
		UIButton * moveDownButton;
		UIButton * captureLocationButton;
		
		CrcLowerString slotCrc;
		std::string    slotName;
		int			   flags;
		int			   level;
	};


	typedef std::map<UIPage *, TaskInfo *> TaskPageMap;
	TaskPageMap ms_taskButtonsToTaskPages;

	std::map<std::string, CrcLowerString> ms_SlotNamesToSlotCrcMap;

	const unsigned int ms_KillType = CrcLowerString::calculateCrc("kill");
	const unsigned int ms_LocationType = CrcLowerString::calculateCrc("location");
	const unsigned int ms_LootType = CrcLowerString::calculateCrc("loot");
	const unsigned int ms_PvpKillType = CrcLowerString::calculateCrc("pvpkill");
	const unsigned int ms_PerformType = CrcLowerString::calculateCrc("perform");
	const unsigned int ms_CommType = CrcLowerString::calculateCrc("message");
	const unsigned int ms_CraftType = CrcLowerString::calculateCrc("craft");
	const unsigned int ms_PvpDestroyType = CrcLowerString::calculateCrc("pvpdestroy");
	const unsigned int ms_KillLootType = CrcLowerString::calculateCrc("killloot");
	const unsigned int ms_SpawnType = CrcLowerString::calculateCrc("spawn");

	std::string  ms_currentFilter;

	const char * const ms_ellipses = "...";
	const char * const ms_shared = "shared_";
	const char         ms_delimiter = '~';
	const char * const ms_speciesTable = "mob/pgc_social_group";
	const char * const ms_creatureTable = "mob/creature_names";
	const char * const ms_taskAmountSkill = "chronicles_max_tasks";
	const char * const ms_shareSkill = "chronicles_max_quest_shared";

	bool TruncateTitleAndDescription(std::string & title, std::string & description);
	void SetTitleAndDescription(UIPage * page, std::string const & title, std::string const & description);

	struct WaypointText
	{
		std::string x;
		std::string y;
		std::string z;
		std::string planet;
		std::string cellName;
		std::string npcName;
	};

	void parseCategoryData(std::string inputString, int & flags);
	void parseQuestData(std::string inputString, int & level, int & flags);
	void ParseWaypointString(std::string inputString, WaypointText & outWaypoint);
	void parseString(std::string input, char delimiter, std::vector<std::string> & output);
	std::string ObjectTemplateToSharedTemplate(std::string objectString);

	std::vector<Object const *> ms_tempObjects;
	void clearTempObjects();

	const int ms_numOfLootViewers = 4;
	const int ms_maxCreditsAllowed = 1000000; // one million
	const int ms_maxCountAllowed = 100; // For tasks that require some count - kill, craft, whatever.
	const int ms_maxSpawnCount = 10;

	StringId  ms_confirmMessage("ui_quest", "clear_confirm_message");
	StringId  ms_rewardsConfirmMessage("ui_quest", "clear_rewards_message");
	StringId  ms_changeDraftConfirmMessage("ui_quest", "clear_draft_message");

	int   ms_currentPage = 0;
	NetworkId ms_currentRecipeID = NetworkId::cms_invalid;
}
using namespace SwgCuiQuestBuilderNamespace;

SwgCuiQuestBuilder::SwgCuiQuestBuilder (UIPage & page):
CuiMediator            ("QuestBuilder", page),
m_callback( new MessageDispatch::Callback ),
m_close(NULL),
m_addTask(NULL),
m_createQuest(NULL),
m_clearRewards(NULL),
m_taskTable(NULL),
m_taskCategory(NULL),
m_taskFilter(NULL),
m_killTask(NULL),
m_locationTask(NULL),
m_lootTask(NULL),
m_pvpKillTask(NULL),
m_performTask(NULL),
m_commTask(NULL),
m_craftTask(NULL),
m_pvpDestroyTask(NULL),
m_killLootTask(NULL),
m_spawnTask(NULL),
m_taskComposite(NULL),
m_questTitle(NULL),
m_questDescription(NULL),
m_questLevel(NULL),
m_questTaskCount(NULL),
m_requiredText(NULL),
m_creditsText(NULL),
m_hideTasksCheck(NULL),
m_showOnlyRecipeTasksCheck(NULL),
m_recipeCheck(NULL),
m_overrideCheck(NULL),
m_shareCheck(NULL),
m_shareCombo(NULL),
m_prevTaskPage(NULL),
m_nextTaskPage(NULL),
m_slotMap(),
m_performSongs(),
m_performDances()
{
	getCodeDataObject(TUIButton, m_close, "close");
	registerMediatorObject(*m_close, true);

	getCodeDataObject(TUIButton, m_createQuest, "createQuestButton");
	registerMediatorObject(*m_createQuest, true);

	getCodeDataObject(TUITable, m_taskTable, "tasksTable");
	registerMediatorObject(*m_taskTable, true);

	getCodeDataObject(TUIComboBox, m_taskCategory, "tasksCategory");
	registerMediatorObject(*m_taskCategory, true);

	getCodeDataObject(TUITextbox, m_taskFilter, "taskFilter");
	registerMediatorObject(*m_taskFilter, true);

	getCodeDataObject(TUIButton, m_addTask, "tasksAddTask");
	registerMediatorObject(*m_addTask, true);

	getCodeDataObject(TUIButton, m_clearRewards, "rewardClear");
	registerMediatorObject(*m_clearRewards, true);

	getCodeDataObject(TUIButton, m_clearQuest, "clearQuestButton");
	registerMediatorObject(*m_clearQuest, true);

	getCodeDataObject(TUIPage, m_killTask, "KillTask");
	m_killTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_locationTask, "LocationTask");
	m_locationTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_lootTask, "LootTask");
	m_lootTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_performTask, "PerformTask");
	m_performTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_commTask, "CommTask");
	m_commTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_craftTask, "CraftTask");
	m_craftTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_pvpKillTask, "KillPlayerTask");
	m_pvpKillTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_pvpDestroyTask, "PvpDestroyTask");
	m_pvpDestroyTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_killLootTask, "KillLootTask");
	m_killLootTask->SetVisible(false);

	getCodeDataObject(TUIPage, m_spawnTask, "SpawnTask");
	m_spawnTask->SetVisible(false);

	getCodeDataObject(TUIComposite, m_taskComposite, "tasksHolder");

	getCodeDataObject(TUITextbox, m_questTitle, "recipeTitle");

	getCodeDataObject(TUIText, m_questDescription, "recipeDescription");

	getCodeDataObject(TUIText, m_questLevel, "questInfoLevel");

	getCodeDataObject(TUIText, m_questTaskCount, "questInfoTasks");

	getCodeDataObject(TUIText, m_requiredText, "questInfoRequires");

	getCodeDataObject(TUITextbox, m_creditsText, "rewardCredits");

	getCodeDataObject(TUICheckbox, m_hideTasksCheck, "hideTaskCheck");
	registerMediatorObject(*m_hideTasksCheck, true);

	getCodeDataObject(TUICheckbox, m_showOnlyRecipeTasksCheck, "showRecipeTasks");
	registerMediatorObject(*m_showOnlyRecipeTasksCheck, true);

	getCodeDataObject(TUICheckbox, m_recipeCheck, "recipeCheck");
	registerMediatorObject(*m_recipeCheck, true);

	getCodeDataObject(TUICheckbox, m_overrideCheck, "overrideCheck");
	registerMediatorObject(*m_overrideCheck, true);

	getCodeDataObject(TUICheckbox, m_shareCheck, "sharingCheck");
	registerMediatorObject(*m_shareCheck, true);

	getCodeDataObject(TUIComboBox, m_shareCombo, "shareCombo");

	getCodeDataObject(TUIButton, m_prevTaskPage, "tasksPrevPage");
	registerMediatorObject(*m_prevTaskPage, true);

	getCodeDataObject(TUIButton, m_nextTaskPage, "tasksNextPage");
	registerMediatorObject(*m_nextTaskPage, true);

	m_overrideCheck->SetVisible(false);
	m_shareCheck->SetVisible(true);
	m_shareCombo->SetVisible(true);
	m_shareCombo->SetEnabled(false);

	for(int i = 0; i < ms_numOfLootViewers; ++i)
	{
		char buffer[32];
		memset(buffer, 0, 32);
		sprintf(buffer, "rewardViewer%d", i + 1);

		getCodeDataObject(TUI3DObjectListViewer, m_lootViewers[i], buffer);

		NOT_NULL(m_lootViewers[i]);

		m_lootViewers[i]->setCameraLookAtCenter (true);
		m_lootViewers[i]->setPaused             (false);
		m_lootViewers[i]->setCameraForceTarget  (true);
		m_lootViewers[i]->setCameraLodBias      (3.0f);
		m_lootViewers[i]->setCameraLodBiasOverride(true);

		registerMediatorObject(*m_lootViewers[i], true);
	}

	registerMediatorObject(getPage(), true);

	m_questDescription->SetPreLocalized(true);
	m_questLevel->SetPreLocalized(true);
	m_questTaskCount->SetPreLocalized(true);
	m_requiredText->SetPreLocalized(true);

	m_taskFilter->SetText(Unicode::emptyString);
	m_taskFilter->SetLocalText(Unicode::emptyString);
	m_hideTasksCheck->SetChecked(false, false);
	m_recipeCheck->SetChecked(false, false);
	m_showOnlyRecipeTasksCheck->SetChecked(false, false);
	m_shareCheck->SetChecked(false);
	
	// Push back all our song choices
	m_performSongs.push_back("Star Wars I");
	m_performSongs.push_back("Rock");
	m_performSongs.push_back("Star Wars II");
	m_performSongs.push_back("Folk");
	m_performSongs.push_back("Star Wars III");
	m_performSongs.push_back("Ceremonial");
	m_performSongs.push_back("Ballad");
	m_performSongs.push_back("Waltz");
	m_performSongs.push_back("Jazz");
	m_performSongs.push_back("Virtuoso");
	m_performSongs.push_back("Western");
	m_performSongs.push_back("Star Wars IV");
	m_performSongs.push_back("Funk");
	m_performSongs.push_back("Swing");
	m_performSongs.push_back("Calypso");
	m_performSongs.push_back("Zydeco");
	m_performSongs.push_back("Carnival");
	m_performSongs.push_back("Dirge");
	m_performSongs.push_back("Pop");
	m_performSongs.push_back("Boogie");

	// Push back all our dance choices
	m_performDances.push_back("Basic I");
	m_performDances.push_back("Basic II");
	m_performDances.push_back("Rhythmic I");
	m_performDances.push_back("Rhythmic II");
	m_performDances.push_back("Footloose I");
	m_performDances.push_back("Footloose II");
	m_performDances.push_back("Formal I");
	m_performDances.push_back("Formal II");
	m_performDances.push_back("Pop Lock I");
	m_performDances.push_back("Pop Lock II");
	m_performDances.push_back("Popular I");
	m_performDances.push_back("Popular II");
	m_performDances.push_back("Bunduki I");
	m_performDances.push_back("Bunduki II");
	m_performDances.push_back("Tumble I");
	m_performDances.push_back("Tumble II");
	m_performDances.push_back("Lyrical I");
	m_performDances.push_back("Lyrical II");
	m_performDances.push_back("Breakdance I");
	m_performDances.push_back("Breakdance II");
	m_performDances.push_back("Exotic I");
	m_performDances.push_back("Exotic II");
	m_performDances.push_back("Exotic III");
	m_performDances.push_back("Exotic IV");
	m_performDances.push_back("Theatrical I");
	m_performDances.push_back("Theatrical II");
	m_performDances.push_back("Peiyi");
	m_performDances.push_back("Freestyle I");
	m_performDances.push_back("Freestyle II");
	m_performDances.push_back("Jazzy I");
	m_performDances.push_back("Jazzy II");
	
	clearTaskRecipe(false);
	createSlotMap();
	populateTasksTable();

	if(m_taskCategory->GetItemCount() > 0)
		m_taskCategory->SetSelectedIndex(0);

}

SwgCuiQuestBuilder::~SwgCuiQuestBuilder()
{
	if(m_callback)
		delete m_callback;

	for(int i = 0; i < ms_numOfLootViewers; ++i)
	{
			m_lootViewers[i]->clearObjects();
			m_lootViewers[i]->SetTooltip(UIString());
	}
	
	TaskPageMap::iterator iter = ms_taskButtonsToTaskPages.begin();
	for(; iter != ms_taskButtonsToTaskPages.end(); ++iter)
	{
		TaskInfo * deleteMe = (*iter).second;
		delete deleteMe;
		(*iter).second = NULL;
	}

	ms_taskButtonsToTaskPages.clear();
	ms_SlotNamesToSlotCrcMap.clear();

	m_slotMap.clear();

	clearTempObjects();

}


void SwgCuiQuestBuilder::performActivate()
{
	refreshCollectionData();

	if(CuiRecipeManager::getRecipeOID() != NetworkId::cms_invalid )
	{
		if(ms_currentRecipeID != CuiRecipeManager::getRecipeOID())
		{
			ms_currentRecipeID = CuiRecipeManager::getRecipeOID();

			clearTaskRecipe();

			populateTasksTable();

			m_taskCategory->SetSelectedIndex(0);

			loadTasksFromRecipeData();
		}
	}

	updateQuestInfo();
	

	CuiManager::requestPointer(true);
	m_callback->connect(*this, &SwgCuiQuestBuilder::OnCollectionsChanged, static_cast<PlayerObject::Messages::CollectionsChanged *>(0));
}

void SwgCuiQuestBuilder::performDeactivate()
{
	CuiManager::requestPointer(false);
	m_callback->disconnect(*this, &SwgCuiQuestBuilder::OnCollectionsChanged, static_cast<PlayerObject::Messages::CollectionsChanged *>(0));
}

void SwgCuiQuestBuilder::OnButtonPressed(UIWidget * context)
{
	if(context == m_close)
	{
		closeNextFrame();
		return;
	}
	else if (context == m_clearQuest)
	{
		CuiMessageBox * const box = CuiMessageBox::createYesNoBox (ms_confirmMessage.localize());
		m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiQuestBuilder::OnConfirmClearQuest);
		return;
	}
	else if (context == m_addTask)
	{
		addTask();
		return;
	}
	else if (context == m_createQuest)
	{
		requestCreateQuest();
		return;
	}
	else if (context == m_clearRewards)
	{
		CuiMessageBox * const box = CuiMessageBox::createYesNoBox(ms_rewardsConfirmMessage.localize());
		m_callback->connect(box->getTransceiverClosed(), *this, &SwgCuiQuestBuilder::OnConfirmClearRewards);
		return;
	}
	else if (context == m_nextTaskPage)
	{
		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;

		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);

		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			int pageNum = ++ms_currentPage;
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str(), pageNum);
		}		
	}
	else if (context == m_prevTaskPage)
	{
		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;

		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);

		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			--ms_currentPage;
			if(ms_currentPage < 0)
				ms_currentPage = 0;

			int pageNum = ms_currentPage;
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str(), pageNum);
		}		
	}
	
	TaskPageMap::iterator iter = ms_taskButtonsToTaskPages.find(static_cast<UIPage*>(context->GetParentWidget()));
	if(iter != ms_taskButtonsToTaskPages.end())
	{
		TaskInfo * task = (*iter).second;
		if(context == task->closeButton)
		{
			SlotMap::iterator removeIter = m_slotMap.find(task->slotCrc);
			if(removeIter == m_slotMap.end())
			{
				DEBUG_WARNING(true, ("Tried to remove Saga Task with slot name %s, but it wasn't in our Slot Map!", task->slotCrc.getString()));
				return;
			}
			// Increment our charge count.
			if((*removeIter).second.recipeRequirement != 0)
				(*removeIter).second.recipeRequirement--;

			updateTableTaskCount(task->slotCrc.getString(), (*removeIter).second.quantity, (*removeIter).second.recipeRequirement);

			m_taskComposite->RemoveChild(task->taskPage);
			ms_taskButtonsToTaskPages.erase(iter);

			delete task;

			updateQuestInfo();
		}
		else if (context == task->moveUpButton)
		{
			m_taskComposite->MoveChild(task->taskPage, UIBaseObject::Up);
		}
		else if (context == task->moveDownButton)
		{
			m_taskComposite->MoveChild(task->taskPage, UIBaseObject::Down);
		}
		else if (context && context == task->captureLocationButton)
		{
			captureLocation(task->taskPage, task->flags);
		}

	}
	
}

void SwgCuiQuestBuilder::populateTasksTable(const char * stringNameFilter, const char * secondNameFilter, int currentPage)
{
	PlayerObject * player = Game::getPlayerObject();

	UITableModelDefault * tableModel = dynamic_cast<UITableModelDefault *>(m_taskTable->GetTableModel());

	if(!tableModel || !player)
		return;

	if(stringNameFilter && !_stricmp(stringNameFilter, "all"))
		stringNameFilter = NULL;

	tableModel->ClearData();
	
	int currentCount = 0;

	ms_currentPage = currentPage;

	m_prevTaskPage->SetEnabled(false);
	m_nextTaskPage->SetEnabled(false);

	if(m_showOnlyRecipeTasksCheck->IsChecked())
	{
		UIBaseObject::UIObjectList taskList;
		m_taskComposite->GetChildren(taskList);

		UIBaseObject::UIObjectList::iterator iter = taskList.begin();
		for(; iter != taskList.end(); ++iter)
		{
			TaskPageMap::iterator pageIter = NULL;

			if((*iter)->IsA(TUIPage))
			{
				pageIter = ms_taskButtonsToTaskPages.find(static_cast<UIPage*>((*iter)));

				if(pageIter == ms_taskButtonsToTaskPages.end())
					continue;
			}
			else
				continue;

			TaskInfo * currentTask = (*pageIter).second;

			if(stringNameFilter && currentTask->slotName.find(stringNameFilter) == std::string::npos)
				continue;

			StringId displayString("collection_n", currentTask->slotName);
			std::string narrowDisplayString = Unicode::toLower( Unicode::wideToNarrow(displayString.localize()) );

			if(secondNameFilter && narrowDisplayString.find( Unicode::toLower(secondNameFilter).c_str() ) == std::string::npos)
				continue;

			TaskSlotInfo * currentSlotInfo = NULL;

			std::map<std::string, CrcLowerString>::iterator slotConvIter = ms_SlotNamesToSlotCrcMap.find(currentTask->slotName);
			if(slotConvIter!= ms_SlotNamesToSlotCrcMap.end())
			{
				SlotMap::iterator slotIter = m_slotMap.find((*slotConvIter).second);
				if(slotIter != m_slotMap.end())
				{
					currentSlotInfo = &(*slotIter).second;

					tableModel->AppendCell(0, NULL, displayString.localize());

					char displayBuffer[32];
					memset(displayBuffer, 0, 32);

					if(currentSlotInfo->recipeRequirement == 0)
						sprintf(displayBuffer, "%d", currentSlotInfo->quantity - 1);
					else
						sprintf(displayBuffer, "%s%d(-%d)",  ( currentSlotInfo->quantity - 1 ) - currentSlotInfo->recipeRequirement < 0 ? "\\#FF0000" : "\\#00FF00", currentSlotInfo->quantity - 1, currentSlotInfo->recipeRequirement);

					tableModel->AppendCell(1, NULL, Unicode::narrowToWide(displayBuffer));
					
				} 
				else
					continue;
			}
			else
				continue;
		}

		return;
	}

	SlotMap::const_iterator iter = m_slotMap.begin();
	for(; iter != m_slotMap.end(); ++iter)
	{

		if((*iter).second.quantity == 0)
			continue;

		if(m_hideTasksCheck->IsChecked() && (*iter).second.quantity <= 1)
			continue;

		if(stringNameFilter && (*iter).second.slotName.find(stringNameFilter) == std::string::npos)
			continue;

		StringId displayString("collection_n", (*iter).second.slotName);
		std::string narrowDisplayString = Unicode::toLower( Unicode::wideToNarrow(displayString.localize()) );

		if(secondNameFilter && narrowDisplayString.find( Unicode::toLower(secondNameFilter).c_str() ) == std::string::npos)
			continue;

		++currentCount;

		if(currentPage && currentCount < MAX_TASKS_PER_PAGE * currentPage)
			continue;

		if(currentPage && currentCount > (MAX_TASKS_PER_PAGE * currentPage + MAX_TASKS_PER_PAGE) )
		{
			m_prevTaskPage->SetEnabled(true);
			m_nextTaskPage->SetEnabled(true);

			continue;
		}

		if(currentPage == 0 && currentCount > MAX_TASKS_PER_PAGE)
		{
			m_nextTaskPage->SetEnabled(true);
			continue;
		}
		
		tableModel->AppendCell(0, NULL, displayString.localize());

		char displayBuffer[32];
		memset(displayBuffer, 0, 32);

		if((*iter).second.recipeRequirement == 0)
			sprintf(displayBuffer, "%d", (*iter).second.quantity - 1);
		else
			sprintf(displayBuffer, "%s%d(-%d)",  ( (*iter).second.quantity - 1 ) - (*iter).second.recipeRequirement < 0 ? "\\#FF0000" : "\\#00FF00", (*iter).second.quantity - 1, (*iter).second.recipeRequirement);
		
		tableModel->AppendCell(1, NULL, Unicode::narrowToWide(displayBuffer));
	}

	if(currentPage == 0 && currentCount < MAX_TASKS_PER_PAGE)
		m_prevTaskPage->SetEnabled(false);

	if(currentPage != 0)
		m_prevTaskPage->SetEnabled(true);


	tableModel->fireDataChanged();
}

void SwgCuiQuestBuilder::OnCollectionsChanged(PlayerObject const & player)
{
	if(&player == Game::getPlayerObject())
	{
		refreshCollectionData();
	}
}

void SwgCuiQuestBuilder::OnGenericSelectionChanged(UIWidget* context)
{
	if(context == m_taskCategory)
	{
		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;

		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);


		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str());
		}
	}
}

void SwgCuiQuestBuilder::OnTextboxChanged(UIWidget *context)
{
	if(context == m_taskFilter)
	{
		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);

		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;


		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str());
		}
	}
}

void SwgCuiQuestBuilder::OnCheckboxSet(UIWidget *context)
{
	if(context == m_hideTasksCheck || context == m_showOnlyRecipeTasksCheck)
	{
		// Reset the combo box if we're showing recipe tasks only
		if(context == m_showOnlyRecipeTasksCheck)
			m_taskCategory->SetSelectedIndex(0);

		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);

		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;


		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str());
		}		
	}
	else if (context == m_shareCheck)
	{
		m_shareCombo->SetEnabled(true);
	}
	else if (context == m_recipeCheck)
	{
		bool needConfirm = false;
		for(int i = 0; i < ms_numOfLootViewers; ++i)
		{
			if(m_lootViewers[i]->getLastObject() != NULL)
			{
				needConfirm = true;
				break;
			}
		}

		if(!needConfirm)
		{
			std::string creditsString = Unicode::wideToNarrow(m_creditsText->GetLocalText());
			int creditAmount = atoi(creditsString.c_str());

			if(creditAmount != 0)
				needConfirm = true;
		}

		if(needConfirm)
		{
			m_recipeCheck->SetChecked(false, false);
			CuiMessageBox * const box = CuiMessageBox::createYesNoBox (ms_changeDraftConfirmMessage.localize());
			m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiQuestBuilder::OnConfirmChangeDraft);
			return;
		}

		// Draft mode, disable sharing.
		m_shareCheck->SetChecked(false);
		m_shareCheck->SetEnabled(false);
	}
}

void SwgCuiQuestBuilder::OnCheckboxUnset(UIWidget *context)
{
	if(context == m_hideTasksCheck || context == m_showOnlyRecipeTasksCheck)
	{
		// Reset the combo box if we're showing recipe tasks only
		if(context == m_showOnlyRecipeTasksCheck)
			m_taskCategory->SetSelectedIndex(0);

		UIString filterText = m_taskFilter->GetLocalText();

		std::string narrowFilter = Unicode::wideToNarrow(filterText);

		const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
		if (!data)
			return;


		UINarrowString textProperty;
		if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
		{
			populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str());
		}		
	}
	else if (context == m_recipeCheck)
	{
		// Draft mode, disable sharing.
		m_shareCheck->SetEnabled(true);
	}
	else if (context == m_shareCheck)
	{
		m_shareCombo->SetEnabled(false);
	}
}

bool SwgCuiQuestBuilder::OnMessage(UIWidget *context, const UIMessage & msg )
{
	if(msg.Type == UIMessage::LeftMouseDoubleClick && context == m_taskTable)
	{
		addTask();
		return true;
	}
	else if (msg.Type == UIMessage::DragEnd)
	{
		if(m_recipeCheck->IsChecked())
		{
			StringId invalidDrop("ui_quest", "invalid_drag_recipe");
			CuiSystemMessageManager::sendFakeSystemMessage(invalidDrop.localize());
			return true;
		}

		for(int i = 0; i < ms_numOfLootViewers; ++i)
		{
			if(context == m_lootViewers[i])
			{
				CuiDragInfo droppingInfo (*msg.DragObject);
			
				ClientObject * dragObj = droppingInfo.getClientObject();

				if(!dragObj)
					return false;
				
				// Make sure this comes from the player's inventory.
				ClientObject* dragObjContainer = dynamic_cast<ClientObject*>(ContainerInterface::getContainedByObject(*dragObj));
				bool validDrop = false;
				while(dragObjContainer)
				{
					if(dragObjContainer == CuiInventoryManager::getPlayerInventory())
					{
						validDrop = true;
						break;
					}
					dragObjContainer = dynamic_cast<ClientObject*>(ContainerInterface::getContainedByObject(*dragObjContainer));

				}
				
				if(!validDrop)
					return false;

				if(ContainerInterface::getContainer(*dragObj) &&  ContainerInterface::getContainer(*dragObj)->getNumberOfItems() != 0 &&
					dragObj->getGameObjectType() != SharedObjectTemplate::GOT_chronicles_quest_holocron)
				{
					CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Rewards that are containers must be empty to be added to a quest."));
					return false;
				}

				// Last check, make sure we don't already have this object in one of our viewers.
				for(int j = 0; j < ms_numOfLootViewers; ++j)
				{
					if(m_lootViewers[j]->getLastObject() != NULL && m_lootViewers[j]->getLastObject() == dragObj)
						return false;
				}

				m_lootViewers[i]->clearObjects();

				m_lootViewers[i]->addObject(*dragObj);

				m_lootViewers[i]->SetTooltip(dragObj->getLocalizedName());

				return true;
			}
		}
	}

	return true;
}

void SwgCuiQuestBuilder::addTask()
{
	int totalPlayerTasks = getSkillModValue(ms_taskAmountSkill);

	if(ms_taskButtonsToTaskPages.size() == MAX_TASKS || ms_taskButtonsToTaskPages.size() >= static_cast<unsigned int>(totalPlayerTasks))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("You have reached your limit for tasks. Please remove a task to add a new one."));
		return;
	}

	UITableModelDefault * tableModel = dynamic_cast<UITableModelDefault *>(m_taskTable->GetTableModel());

	if(!tableModel)
		return;

	long selectedRow = m_taskTable->GetLastSelectedRow();

	if(selectedRow < 0)
		return;

	UIString taskName;
	tableModel->GetValueAtText(selectedRow, 0, taskName);

	SlotMap::iterator iter = m_slotMap.find(CrcLowerString(Unicode::wideToNarrow(taskName).c_str()));
	if(iter == m_slotMap.end())
	{
		DEBUG_WARNING(true, ("Tried to add the Saga Task for slot %s but it wasn't in our slot map!", Unicode::wideToNarrow(taskName).c_str()));
		return;
	}
		
	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName((*iter).second.slotName);
	if(!slotInfo)
		return;

	TaskSlotInfo const * currentInfo = &(*iter).second;

	std::string::size_type uiType = slotInfo->categories[0].find("UIType");
	std::string::size_type colonIndex = slotInfo->categories[0].find(':');
	if(colonIndex == std::string::npos || uiType == std::string::npos)
	{
		DEBUG_WARNING(true, ("Invalid UIType specifier for User Generated Content Task %s in Category 0.", taskName.c_str()));
		return;
	}

	std::string UITypeString = slotInfo->categories[0].substr(colonIndex + 1);

	CrcLowerString uiTaskType(UITypeString.c_str());
	UIPage * newPage = NULL;

	if(uiTaskType.getCrc() == ms_KillType)
	{
		newPage = addKillTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_LocationType)
	{
		newPage = addLocationTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_LootType)
	{
		newPage = addLootTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_PvpKillType)
	{
		newPage = addPvpKillTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_PerformType)
	{
		newPage = addPerformTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_CommType)
	{
		newPage = addCommTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_CraftType)
	{
		newPage = addCraftTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_PvpDestroyType)
	{
		newPage = addPvpDestroyTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_KillLootType)
	{
		newPage = addKillLootTask(currentInfo);
	}
	else if (uiTaskType.getCrc() == ms_SpawnType)
	{
		newPage = addSpawnTask(currentInfo);
	}
	else
	{
		// Failed to find an acceptable task.
		DEBUG_WARNING(true, ("Invalid UIType: %s", UITypeString.c_str()));
		return;
	}
	
	// Actually update the quantity.
	if(newPage)
	{
		(*iter).second.recipeRequirement++;
		updateTableTaskCount(Unicode::wideToNarrow(taskName).c_str(), (*iter).second.quantity, (*iter).second.recipeRequirement);
	}
	else
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Failed to add task to Quest."));

	
	updateQuestInfo();

}

UIPage* SwgCuiQuestBuilder::addKillTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	
	std::string slotName;
	std::vector<std::string> parsedData;
	
	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;

	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_killTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	UIText * creatureField = NULL;
	newPage->GetCodeDataObject(TUIText, creatureField, "creatureText");
	creatureField->SetPreLocalized(true);

	std::string::size_type creatureKill = slotInfo->categories[2].find("creature_name");
	if(creatureKill != std::string::npos)
	{
		// Creature Kill
		std::string creatureName = slotInfo->categories[2].substr(strlen("creature_name:"));
		
		std::string::size_type comma = creatureName.find(',');
		if(comma != std::string::npos)
			creatureName = creatureName.substr(0, comma);

		StringId creature(ms_creatureTable, creatureName);
		creatureField->SetLocalText(creature.localize());
		newPage->SetProperty(SwgCuiQuestBuilder::KillCreatureProperty, Unicode::narrowToWide(creatureName));
	}
	else
	{
		// Species Kill
		std::string speciesType = slotInfo->categories[2].substr(strlen("social_group:"));
		
		std::string::size_type comma = speciesType.find(',');
		if(comma != std::string::npos)
			speciesType = speciesType.substr(0, comma);

		StringId social(ms_speciesTable, speciesType);
		creatureField->SetLocalText(social.localize());
		newPage->SetProperty(SwgCuiQuestBuilder::KillSpeciesProperty, Unicode::narrowToWide(speciesType));
	}
	creatureField->SetEditable(false);


	UIText * count = NULL;
	newPage->GetCodeDataObject(TUIText, count, "countValue");
	if(count)
	{
		std::string valueString = slotInfo->categories[4].substr(strlen("count:"));
		int value = atoi(valueString.c_str());
		if(value > 0)
		{
			count->SetEditable(false);
			count->SetLocalText(Unicode::narrowToWide(valueString.c_str()));
		}
		else
		{
			count->SetEditable(true);

			if(taskData.empty())
				count->SetLocalText(Unicode::narrowToWide(valueString.c_str()));
			else
				count->SetLocalText(Unicode::narrowToWide(parsedData[1]));
		}

	}

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;
	
	newTaskInfo->taskType = TT_Kill;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;

}

void SwgCuiQuestBuilder::createSlotMap()
{
	m_slotMap.clear();
	ms_SlotNamesToSlotCrcMap.clear();

	PlayerObject * player = Game::getPlayerObject();
	if(!player)
		return;

	std::vector<CollectionsDataTable::CollectionInfoCollection const *> pageCollections = CollectionsDataTable::getCollectionsInPage(ms_relicPage);
	for(unsigned int i = 0; i < pageCollections.size(); ++i)
	{
		int categoryFlags = 0;
		parseCategoryData(pageCollections[i]->categories[0], categoryFlags);

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> collectionSlots = CollectionsDataTable::getSlotsInCollection(pageCollections[i]->name);
		for(unsigned int j = 0; j < collectionSlots.size(); ++j)
		{
			if(/*collectionSlots[j]->hidden || */collectionSlots[j]->showIfNotYetEarned == CollectionsDataTable::SE_none)
				continue;

			unsigned long collectionSlotValue;
			player->getCollectionSlotValue(*collectionSlots[j], collectionSlotValue);

			TaskSlotInfo newSlotInfo;

			newSlotInfo.quantity = collectionSlotValue;
			newSlotInfo.slotName = collectionSlots[j]->name;
			std::string displayString = Unicode::wideToNarrow(StringId("collection_n", newSlotInfo.slotName).localize());
			newSlotInfo.slotCrc = CrcLowerString(displayString.c_str());
			newSlotInfo.categoryFlags = categoryFlags;
			newSlotInfo.recipeRequirement = 0;
			
			std::pair< std::map<CrcLowerString, TaskSlotInfo>::iterator, bool > retVal = m_slotMap.insert(std::make_pair<CrcLowerString, TaskSlotInfo>(newSlotInfo.slotCrc, newSlotInfo));
			
			if(retVal.second == false)
			{
				WARNING(true, ("Failed to add slot %s with the CRC %d to our slot map.", newSlotInfo.slotName, newSlotInfo.slotCrc.getCrc()));
			}
			
			std::pair< std::map<std::string, CrcLowerString>::iterator, bool > secondRetVal = ms_SlotNamesToSlotCrcMap.insert(std::make_pair<std::string, CrcLowerString>(newSlotInfo.slotName, newSlotInfo.slotCrc));
		
			if(secondRetVal.second == false)
			{
				WARNING(true, ("Failed to add slot %s with the CRC %d to our Name-To-Slot map.", newSlotInfo.slotName, newSlotInfo.slotCrc.getCrc()));
			}
		}
	}

	// Populate the Share combo box.
	int playerMaxShare = getSkillModValue(ms_shareSkill);
	m_shareCombo->Clear();
	for(int i = 0; i < playerMaxShare; ++i)
	{
		char buffer[32];
		memset(buffer, 0, 32);
		sprintf(buffer, "%d", i + 1);
		std::string stringBuffer = buffer;
		m_shareCombo->AddItem(Unicode::narrowToWide(buffer), stringBuffer);
	}
}

void SwgCuiQuestBuilder::updateTableTaskCount(const char *taskDisplayName, int newQuantity, int recipeCount)
{
	CrcLowerString task(taskDisplayName);

	UITableModelDefault * tableModel = dynamic_cast<UITableModelDefault *>(m_taskTable->GetTableModel());

	if(!tableModel)
		return;

	for(long i = 0; i < tableModel->GetRowCount(); ++ i)
	{
		UIString value;
		tableModel->GetValueAtText(i, 0, value);

		if(task == CrcLowerString(Unicode::wideToNarrow(value).c_str()))
		{
			char displayBuffer[32];
			memset(displayBuffer, 0, 32);

			if(recipeCount == 0)
				sprintf(displayBuffer, "%d", newQuantity - 1);
			else
				sprintf(displayBuffer, "%s%d(-%d)", (newQuantity - 1) - recipeCount < 0 ? "\\#FF0000" : "\\#00FF00", newQuantity - 1, recipeCount);

			tableModel->SetValueAtText(i, 1, Unicode::narrowToWide(displayBuffer));
		}
	}

}

void SwgCuiQuestBuilder::requestCreateQuest()
{
	if(ms_taskButtonsToTaskPages.empty())
		return;

	Controller * controller = Game::getPlayer () ? Game::getPlayer ()->getController () : 0;

	if(!controller)
		return;

	MessageQueueCreateSaga * const msg = new MessageQueueCreateSaga();

	std::string narrowQuestTitle = Unicode::wideToNarrow(TextManager::filterText(m_questTitle->GetLocalText()));
	std::string narrowQuestDescription = Unicode::wideToNarrow(TextManager::filterText(m_questDescription->GetLocalText()));

	if(narrowQuestTitle.empty() || narrowQuestDescription.empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Quest Title or Description is empty. Please enter a valid string."));
		return;
	}

	if(narrowQuestTitle.size() > 128)
		narrowQuestTitle = narrowQuestTitle.substr(0, 128);

	if(narrowQuestDescription.size() > 1000)
		narrowQuestDescription = narrowQuestDescription.substr(0, 1000);

	msg->setQuestName(narrowQuestTitle);
	msg->setQuestDescription(narrowQuestDescription);

	UIBaseObject::UIObjectList taskList;
	m_taskComposite->GetChildren(taskList);
	
	UIBaseObject::UIObjectList::iterator iter = taskList.begin();
	for(; iter != taskList.end(); ++iter)
	{
		TaskPageMap::iterator pageIter = NULL;

		if((*iter)->IsA(TUIPage))
		{
			pageIter = ms_taskButtonsToTaskPages.find(static_cast<UIPage*>((*iter)));

			if(pageIter == ms_taskButtonsToTaskPages.end())
				continue;
		}
		else
			continue;

		TaskInfo * currentTask = (*pageIter).second;
		std::vector<std::string> testString;
		testString.push_back(currentTask->slotName);

		switch(currentTask->taskType)
		{
		case TT_Kill:
			if(!getKillTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Location:
			if(!getLocationTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Loot:
			if(!getLootTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_PvPKill:
			if(!getPvpKillTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Perform:
			if(!getPerformTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Comm:
			if(!getCommTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Craft:
			if(!getCraftTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_PvPDestroy:
			if(!getPvpDestroyTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_KillLoot:
			if(!getKillLootTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		case TT_Spawn:
			if(!getSpawnTaskInfo(currentTask->taskPage, testString))
			{
				delete msg;
				return;
			}
			break;
		default:
			DEBUG_WARNING(true, ("Failed to find getTaskInfo function for %d", currentTask->taskType));
			break;
		}

		msg->addTask(static_cast<int>(currentTask->taskType), testString);
	}

	std::string rewardString;
	for(int i = 0; i < ms_numOfLootViewers; ++i)
	{
		NetworkId oid = NetworkId::cms_invalid;
		if(m_lootViewers[i]->getLastObject() != NULL)
		{
			oid = m_lootViewers[i]->getLastObject()->getNetworkId();
		}

		if(!rewardString.empty())
			rewardString.append(1, '~');
		
		if(oid != NetworkId::cms_invalid)
			rewardString.append(oid.getValueString());
		else
			rewardString.append(1, '0');
	}

	std::string creditsString = Unicode::wideToNarrow(m_creditsText->GetLocalText());
	int creditAmount = atoi(creditsString.c_str());
	int playerBank, playerCredits = 0;
	PlayerMoneyManagerClient::getPlayerMoney(playerCredits, playerBank);
	int totalCredits = playerBank + playerCredits;
	if(creditAmount < 0 || creditAmount >  ms_maxCreditsAllowed)
	{
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "Invalid credit amount entered. Select a value between 0 and %d", ms_maxCreditsAllowed);
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(buffer));
		delete msg;
		return;
	}
	else if (creditAmount > totalCredits)
	{
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "Invalid credit amount entered. You currently only have %d credits total (this value includes what is in your bank).", totalCredits);
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(buffer));
		delete msg;
		return;
	}
	// Make sure the credits value is valid.
	char creditBuffer[32];
	memset(creditBuffer, 0, 32);
	sprintf(creditBuffer, "%d", creditAmount);
	creditsString = creditBuffer;

	rewardString.append(1, '~');
	rewardString.append(creditsString);

	msg->setRewards(rewardString);

	msg->setRecipe(m_recipeCheck->IsChecked());

	msg->setRecipeOID( !m_overrideCheck->IsChecked() ? ms_currentRecipeID : NetworkId::cms_invalid );

	int shareAmount = 0;
	if(m_shareCheck->IsChecked() && m_shareCombo->GetSelectedIndex() < 0)
	{
		StringId shareInvalid("ui_quest", "invalid_share_amount");
		CuiSystemMessageManager::sendFakeSystemMessage(shareInvalid.localize());
		delete msg;
		return;
	}
	else if(m_shareCheck->IsChecked() && m_shareCombo->GetSelectedIndex() >= 0)
	{
		shareAmount = m_shareCombo->GetSelectedIndex() + 1;
	}

	msg->setShareAmount(shareAmount);

	m_overrideCheck->SetChecked(false, false);
	m_overrideCheck->SetVisible(false);

	m_recipeCheck->SetChecked(false);

	ms_currentRecipeID = NetworkId::cms_invalid;

	//-- enqueue message
	controller->appendMessage (CM_createSaga, 0.0f, msg, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

	//clearTaskRecipe();
	closeNextFrame();
}

void SwgCuiQuestBuilder::clearTaskRecipe(bool doCleanup)
{
	if(doCleanup)
	{
		TaskPageMap::iterator iter = ms_taskButtonsToTaskPages.begin();
		for(; iter != ms_taskButtonsToTaskPages.end(); ++iter)
		{
			TaskInfo * task = (*iter).second;
			m_taskComposite->RemoveChild(task->taskPage);
			delete task;

		}

		ms_taskButtonsToTaskPages.clear();
	}

	// Reset the Quest name and description.
	m_questTitle->SetText(Unicode::narrowToWide("My Quest Title"));
	m_questDescription->SetLocalText(Unicode::narrowToWide("Enter a description here."));

	if(doCleanup)
	{
		clearTempObjects();

		for(int i = 0; i < ms_numOfLootViewers; ++i)
		{
				m_lootViewers[i]->clearObjects();
				m_lootViewers[i]->SetTooltip(UIString());
		}

	}

	m_creditsText->SetLocalText(Unicode::narrowToWide("0"));

	m_recipeCheck->SetChecked(false);
	m_overrideCheck->SetChecked(false);
	m_overrideCheck->SetVisible(false);
}

UIPage* SwgCuiQuestBuilder::addLocationTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;

	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_locationTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special Location task stuff...
	UIText * xField;
	newPage->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	newPage->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	newPage->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	newPage->GetCodeDataObject(TUIText, planetField, "planet");

	UIText * nameField;
	newPage->GetCodeDataObject(TUIText, nameField, "genericNameField");

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	UIButton * captureLoc;
	newPage->GetCodeDataObject(TUIButton, captureLoc, "captureButton");
	registerMediatorObject(*captureLoc, true);

	xField->SetPreLocalized(true);
	yField->SetPreLocalized(true);
	zField->SetPreLocalized(true);
	planetField->SetPreLocalized(true);
	nameField->SetPreLocalized(true);

	if(slotName.find("generic") == std::string::npos)
	{
		captureLoc->SetEnabled(false);
		nameField->SetEditable(false);
		nameField->SetEnabled(false);
	}
	else
	{
		captureLoc->SetEnabled(true);
		nameField->SetEnabled(true);
		nameField->SetEditable(true);
		xField->SetLocalText(Unicode::intToWide(0));
		yField->SetLocalText(Unicode::intToWide(0));
		zField->SetLocalText(Unicode::intToWide(0));
		planetField->SetLocalText(Unicode::emptyString);
	}

	WaypointText waypointInfo;

	if(parsedData.size() > 1 )
		ParseWaypointString(parsedData[1], waypointInfo);
	else
		ParseWaypointString(slotInfo->categories[2], waypointInfo);

	xField->SetLocalText(Unicode::narrowToWide(waypointInfo.x));
	yField->SetLocalText(Unicode::narrowToWide(waypointInfo.y));
	zField->SetLocalText(Unicode::narrowToWide(waypointInfo.z));
	planetField->SetLocalText(Unicode::narrowToWide(waypointInfo.planet));
	nameField->SetLocalText(Unicode::narrowToWide(waypointInfo.npcName));

	xField->SetEditable(false);
	yField->SetEditable(false);
	zField->SetEditable(false);
	planetField->SetEditable(false);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Location;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->captureLocationButton = captureLoc;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addLootTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{

	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_lootTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special Loot task stuff...
	UIText * item;
	newPage->GetCodeDataObject(TUIText, item, "item");

	UIText * count;
	newPage->GetCodeDataObject(TUIText, count, "number");

	item->SetPreLocalized(true);
	count->SetPreLocalized(true);

	std::string templateString = slotInfo->categories[2].substr(strlen("template:"));
	newPage->SetProperty(SwgCuiQuestBuilder::TemplateProperty, Unicode::narrowToWide(templateString));
	templateString = ObjectTemplateToSharedTemplate(templateString);

	ObjectTemplate const * itemTemplate = ObjectTemplateList::fetch(templateString);
	if(itemTemplate)
	{
		SharedObjectTemplate const * sharedItem = itemTemplate->asSharedObjectTemplate();
		if(sharedItem)
		{
			item->SetLocalText(sharedItem->getObjectName().localize());
			item->SetEditable(false);
		}
		else
			item->SetLocalText(Unicode::narrowToWide("None"));

		itemTemplate->releaseReference();
	}
	else
		item->SetLocalText(Unicode::narrowToWide("None"));

	std::string countString = slotInfo->categories[5].substr(strlen("count:"));
	count->SetLocalText(Unicode::narrowToWide(countString));
	if(atoi(countString.c_str()) > 0)
		count->SetEditable(false);
	else
	{
		count->SetEditable(true);
		if(!taskData.empty())
		{
			count->SetLocalText(Unicode::narrowToWide(parsedData[1]));
		}
	}


	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Loot;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addPvpKillTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_pvpKillTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special PvP Kill task stuff...
	UIText * rankText;
	newPage->GetCodeDataObject(TUIText, rankText, "rankfield");

	std::string rank = slotInfo->categories[3].substr(strlen("rank:"));
	rankText->SetLocalText(Unicode::narrowToWide(rank));
	rankText->SetEditable(false);

	UIText * numText;
	newPage->GetCodeDataObject(TUIText, numText, "numfield");
	std::string num = slotInfo->categories[2].substr(strlen("count:"));
	numText->SetLocalText(Unicode::narrowToWide(num));

	if(atoi(num.c_str()) > 0)
		numText->SetEditable(false);
	else
	{
		if(!taskData.empty())
			numText->SetLocalText(Unicode::narrowToWide(parsedData[1]));

		numText->SetEditable(true);
	}

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_PvPKill;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;
	
	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addPerformTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_performTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special Perform task stuff...
	UIText * xField;
	newPage->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	newPage->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	newPage->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	newPage->GetCodeDataObject(TUIText, planetField, "planet");

	UIText * venue;
	newPage->GetCodeDataObject(TUIText, venue, "venue");

	UIComboBox * sets;
	newPage->GetCodeDataObject(TUIComboBox, sets, "sets");

	UIComboBox * actionCombo;
	newPage->GetCodeDataObject(TUIComboBox, actionCombo, "actioncombo");

	xField->SetPreLocalized(true);
	yField->SetPreLocalized(true);
	zField->SetPreLocalized(true);
	planetField->SetPreLocalized(true);
	venue->SetPreLocalized(true);
	sets->Clear();
	actionCombo->Clear();

	for(int i = 3; i <=10; ++i)
	{
		char buffer[32];
		memset(buffer, 0, 32);
		sprintf(buffer, "%d", i);
		std::string narrowBuffer = buffer;
		sets->AddItem(Unicode::intToWide(i), narrowBuffer);
	}

	if(sets->GetItemCount() > 0)
		sets->SetSelectedIndex(0);

	WaypointText waypointInfo;
	ParseWaypointString(slotInfo->categories[6], waypointInfo);

	xField->SetLocalText(Unicode::narrowToWide(waypointInfo.x));
	yField->SetLocalText(Unicode::narrowToWide(waypointInfo.y));
	zField->SetLocalText(Unicode::narrowToWide(waypointInfo.z));
	planetField->SetLocalText(Unicode::narrowToWide(waypointInfo.planet));

	std::string venueName = slotInfo->categories[4].substr(strlen("cell_name:"));
	venue->SetLocalText(Unicode::narrowToWide(venueName));

	std::string actionName = slotInfo->categories[5].substr(strlen("what:"));
	if(actionName.compare("dance") == 0)
	{
		for(unsigned int i = 0; i < m_performDances.size(); ++i)
		{
			actionCombo->AddItem(Unicode::narrowToWide(m_performDances[i]), m_performDances[i]);
		}

		if(actionCombo->GetItemCount() > 0)
			actionCombo->SetSelectedIndex(0);
	}
	else
	{
		for(unsigned int i = 0; i < m_performSongs.size(); ++i)
		{
			actionCombo->AddItem(Unicode::narrowToWide(m_performSongs[i]), m_performSongs[i]);
		}

		if(actionCombo->GetItemCount() > 0)
			actionCombo->SetSelectedIndex(0);
	}

	if(parsedData.size() > 1)
	{
		// Just assume the data is packed in such a way that its action index followed by set index at the very end of the parameters.
		int actionIndex = atoi(parsedData[parsedData.size() - 2].c_str());
		if(actionIndex >= 0)
			actionCombo->SetSelectedIndex(actionIndex);

		int setValue = atoi(parsedData[parsedData.size() - 1].c_str());
		if(setValue >= 0)
			sets->SetSelectedIndex(setValue - 3);
	}
	

	xField->SetEditable(false);
	yField->SetEditable(false);
	zField->SetEditable(false);
	planetField->SetEditable(false);
	venue->SetEditable(false);

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Perform;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addCommTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_commTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special Comm task stuff...
	UIText * npc;
	newPage->GetCodeDataObject(TUIText, npc, "npc");

	UIText * message;
	newPage->GetCodeDataObject(TUIText, message, "message");

	npc->SetPreLocalized(true);
	message->SetPreLocalized(true);

	std::string talker = slotInfo->categories[4].substr(strlen("display:"));
	npc->SetLocalText(Unicode::narrowToWide(talker));
	npc->SetEditable(false);
	
	bool messageEditable = false;
	if(!slotInfo->categories[3].empty())
	{
		std::string commMessage = slotInfo->categories[3].substr(strlen("message:"));
		messageEditable = (commMessage.find("none") != std::string::npos);
		StringId commStringId(commMessage);

		message->SetLocalText(commStringId.localize());

		if(!taskData.empty() && parsedData.size() > 1)
			message->SetLocalText(Unicode::narrowToWide(parsedData[1]));
	}

	message->SetEditable(messageEditable);

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Comm;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addCraftTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_craftTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special Craft task stuff...
	UIText * item;
	newPage->GetCodeDataObject(TUIText, item, "item");

	UIText * number;
	newPage->GetCodeDataObject(TUIText, number, "number");

	item->SetPreLocalized(true);
	number->SetPreLocalized(true);
	
	// Store the template name before we change it to a shared template version and all that jazz.
	std::string templateName = slotInfo->categories[2].substr(strlen("template:"));
	newPage->SetProperty(SwgCuiQuestBuilder::TemplateProperty, Unicode::narrowToWide(templateName));
	
	templateName = ObjectTemplateToSharedTemplate(templateName);
	ObjectTemplate const * itemTemplate = ObjectTemplateList::fetch(templateName);
	if(itemTemplate == NULL)
	{
		// Maybe we have a display override. Some items don't follow the shared_ nomenculture.
		std::string overrideString = slotInfo->categories[4].substr(strlen("display:"));
		itemTemplate = ObjectTemplateList::fetch(overrideString);
	}
	if(itemTemplate)
	{
		SharedObjectTemplate const * sharedItem = itemTemplate->asSharedObjectTemplate();
		if(sharedItem)
		{
			item->SetLocalText(sharedItem->getObjectName().localize());

			item->SetEditable(false);

			CuiWidget3dObjectListViewer * viewer;
			newPage->GetCodeDataObject(TUI3DObjectListViewer, viewer, "viewer");

			Object * newObject = itemTemplate->createObject();

			if(viewer && newObject)
			{
				viewer->setCameraLookAtCenter (true);
				viewer->setPaused             (false);
				viewer->setCameraForceTarget  (true);
				viewer->setCameraLodBias      (3.0f);
				viewer->setCameraLodBiasOverride(true);

				viewer->setObject(newObject);

				ms_tempObjects.push_back(newObject);
			}
		}
		else
			item->SetLocalText(Unicode::narrowToWide("None"));

		itemTemplate->releaseReference();
	}
	else
		item->SetLocalText(Unicode::narrowToWide("None"));


	std::string countString = slotInfo->categories[3].substr(strlen("count:"));
	number->SetLocalText(Unicode::narrowToWide(countString.c_str()));

	if(atoi(countString.c_str()) > 0)
		number->SetEditable(false);
	else
	{
		number->SetEditable(true);
		if(!taskData.empty())
			number->SetLocalText(Unicode::narrowToWide(parsedData[1]));
	}

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Craft;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;

	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addPvpDestroyTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_pvpDestroyTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	// Special PvP Destroy task stuff...
	UIText * structureText;
	newPage->GetCodeDataObject(TUIText, structureText, "structurefield");

	std::string structure = slotInfo->categories[3].substr(strlen("objective:"));
	structureText->SetLocalText(Unicode::narrowToWide(structure));

	structureText->SetEditable(false);

	UIText * numText;
	newPage->GetCodeDataObject(TUIText, numText, "numfield");
	std::string num = slotInfo->categories[2].substr(strlen("count:"));
	numText->SetLocalText(Unicode::narrowToWide(num));

	if(atoi(num.c_str()) > 0)
		numText->SetEditable(false);
	else
	{
		numText->SetEditable(true);
		if(!taskData.empty())
			numText->SetLocalText(Unicode::narrowToWide(parsedData[1]));
	}

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);

	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_PvPDestroy;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addKillLootTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_killLootTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	UIText * creatureField = NULL;
	newPage->GetCodeDataObject(TUIText, creatureField, "creatureText");
	creatureField->SetPreLocalized(true);

	std::string::size_type creatureKill = slotInfo->categories[2].find("creature_name");
	if(creatureKill != std::string::npos)
	{
		// Creature Kill
		std::string creatureName = slotInfo->categories[2].substr(strlen("creature_name:"));

		std::string::size_type comma = creatureName.find(',');
		if(comma != std::string::npos)
			creatureName = creatureName.substr(0, comma);

		StringId creature(ms_creatureTable, creatureName);
		creatureField->SetLocalText(creature.localize());
		newPage->SetProperty(SwgCuiQuestBuilder::KillCreatureProperty, Unicode::narrowToWide(creatureName));
	}
	else
	{
		// Species Kill
		std::string speciesType = slotInfo->categories[2].substr(strlen("social_group:"));

		std::string::size_type comma = speciesType.find(',');
		if(comma != std::string::npos)
			speciesType = speciesType.substr(0, comma);

		StringId social(ms_speciesTable, speciesType);
		creatureField->SetLocalText(social.localize());
		newPage->SetProperty(SwgCuiQuestBuilder::KillSpeciesProperty, Unicode::narrowToWide(speciesType));
	}
	creatureField->SetEditable(false);

	UIText * itemField  = NULL;
	newPage->GetCodeDataObject(TUIText, itemField, "itemfield");

	StringId itemStringId(slotInfo->categories[3].substr(strlen("message:")));
	itemField->SetLocalText(itemStringId.localize());

	if(!taskData.empty())
		itemField->SetLocalText(Unicode::narrowToWide(parsedData[2]));

	UIText * dropField = NULL;
	newPage->GetCodeDataObject(TUIText, dropField, "dropfield");
	std::string narrowDropRate = slotInfo->categories[5].substr(strlen("drop_rate:"));

	dropField->SetLocalText(Unicode::narrowToWide(narrowDropRate));
	if(atoi(narrowDropRate.c_str()) > 0)
		dropField->SetEditable(false);
	else
	{
		dropField->SetEditable(true);
		if(!taskData.empty())
			dropField->SetLocalText(Unicode::narrowToWide(parsedData[3]));
	}

	UIText * count = NULL;
	newPage->GetCodeDataObject(TUIText, count, "countValue");
	if(count)
	{
		std::string valueString = slotInfo->categories[4].substr(strlen("count:"));
		int value = atoi(valueString.c_str());
		if(value > 0)
		{
			count->SetEditable(false);
			count->SetLocalText(Unicode::narrowToWide(valueString.c_str()));
		}
		else
		{
			count->SetEditable(true);
			if(taskData.empty())
				count->SetLocalText(Unicode::narrowToWide(valueString.c_str()));
			else
				count->SetLocalText(Unicode::narrowToWide(parsedData[1]));
		}

	}

	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);



	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_KillLoot;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;
	

	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

UIPage* SwgCuiQuestBuilder::addSpawnTask(TaskSlotInfo const * taskInfo, std::string const taskData)
{
	std::string slotName;
	std::vector<std::string> parsedData;

	if(!taskInfo && taskData.empty())
		return NULL;

	if(!taskInfo)
	{
		parseString(taskData, ms_delimiter, parsedData);

		slotName = parsedData[0];
	}
	else
		slotName = taskInfo->slotName;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if(!slotInfo)
		return NULL;


	UIPage * newPage = NULL;
	newPage = dynamic_cast<UIPage *>(m_spawnTask->DuplicateObject());
	if(!newPage)
		return NULL;

	m_taskComposite->AddChild(newPage);
	newPage->Link();
	newPage->SetVisible(true);
	newPage->SetEnabled(false);
	newPage->SetEnabled(true);

	// Spawn task stuff here.
	UIText * titleField = NULL;
	newPage->GetCodeDataObject(TUIText, titleField, "title");
	StringId defaultTitle("collection_n", slotName);
	titleField->SetLocalText(defaultTitle.localize());

	UIText * xField;
	newPage->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	newPage->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	newPage->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	newPage->GetCodeDataObject(TUIText, planetField, "planet");

	UIText * nameField;
	newPage->GetCodeDataObject(TUIText, nameField, "creaturename");

	UIText * countField;
	newPage->GetCodeDataObject(TUIText, countField, "countValue");

	xField->SetPreLocalized(true);
	yField->SetPreLocalized(true);
	zField->SetPreLocalized(true);
	planetField->SetPreLocalized(true);
	nameField->SetPreLocalized(true);
	countField->SetPreLocalized(true);

	xField->SetLocalText(Unicode::intToWide(0));
	yField->SetLocalText(Unicode::intToWide(0));
	zField->SetLocalText(Unicode::intToWide(0));
	countField->SetLocalText(Unicode::intToWide(-1));

	xField->SetEditable(false);
	yField->SetEditable(false);
	zField->SetEditable(false);
	planetField->SetEditable(false);


	// Populate with Slot data here.


	UIButton * closeButton = NULL;
	newPage->GetCodeDataObject(TUIButton, closeButton, "close");
	registerMediatorObject(*closeButton, true);

	UIButton * moveUpButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveUpButton, "buttonMoveUp");
	registerMediatorObject(*moveUpButton, true);

	UIButton * moveDownButton = NULL;
	newPage->GetCodeDataObject(TUIButton, moveDownButton, "buttonMoveDown");
	registerMediatorObject(*moveDownButton, true);


	TaskInfo * newTaskInfo = new TaskInfo;

	newTaskInfo->taskType = TT_Spawn;
	newTaskInfo->closeButton = closeButton;
	newTaskInfo->moveUpButton = moveUpButton;
	newTaskInfo->moveDownButton = moveDownButton;
	newTaskInfo->taskPage = newPage;
	std::string displayString = Unicode::wideToNarrow(StringId("collection_n", slotInfo->name).localize());
	newTaskInfo->slotCrc = CrcLowerString(displayString.c_str());
	newTaskInfo->slotName = slotInfo->name;


	int level, flags;
	parseQuestData(slotInfo->categories[1], level, flags);
	newTaskInfo->level = level;

	// Try and find the task info.
	if(taskInfo == NULL)
	{
		std::map<std::string, CrcLowerString>::iterator iter = ms_SlotNamesToSlotCrcMap.find(slotName);
		if(iter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*iter).second);
			if(slotIter != m_slotMap.end())
			{
				taskInfo = &(*slotIter).second;
			} 
		}
	}

	newTaskInfo->flags = flags ^ ( taskInfo ? taskInfo->categoryFlags : 0 );

	ms_taskButtonsToTaskPages.insert(std::make_pair<UIPage*, TaskInfo *>(newPage, newTaskInfo));

	return newPage;
}

bool SwgCuiQuestBuilder::getKillTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * titleText;
	page->GetCodeDataObject(TUIText, titleText, "title");

	std::string narrowTitle = Unicode::wideToNarrow(titleText->GetLocalText());

	UIText * description;
	page->GetCodeDataObject(TUIText, description, "descriptiontext");

	std::string descriptionString = Unicode::wideToNarrow(description->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, descriptionString))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(descriptionString);
	
	UIText * countText;
	page->GetCodeDataObject(TUIText, countText, "countValue");
	
	UIString propertyValue;
	if(page->HasProperty(SwgCuiQuestBuilder::KillCreatureProperty))
	{
		if(page->GetProperty(SwgCuiQuestBuilder::KillCreatureProperty, propertyValue))
		{
			std::string killString = "creature:";
			killString.append(Unicode::wideToNarrow(propertyValue));
			parameters.push_back(killString);
		}
		else
			DEBUG_FATAL(true, ("Had a property but couldn't get to it."));
	}
	else if (page->HasProperty(SwgCuiQuestBuilder::KillSpeciesProperty))
	{
		if(page->GetProperty(SwgCuiQuestBuilder::KillSpeciesProperty, propertyValue))
		{
			std::string killString = "species:";
			killString.append(Unicode::wideToNarrow(propertyValue));
			parameters.push_back(killString);
		}
		else
			DEBUG_FATAL(true, ("Had a property but couldn't get to it."));
	}
	else
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Failed to find a valid species or creature for a kill task."));
		return false;
	}

	UIString countValue = countText->GetLocalText();
	int debugInt = Unicode::toInt(countValue);
	if(debugInt <= 0 || debugInt > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Kill task has an invalid kill count. Please enter a valid amount ( between 1 and 100) and try again."));
		return false;
	}
	countValue = Unicode::intToWide(debugInt);

	parameters.push_back(Unicode::wideToNarrow(countValue));

	return true;
}

bool SwgCuiQuestBuilder::getLocationTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * xField;
	page->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	page->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	page->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	page->GetCodeDataObject(TUIText, planetField, "planet");
	
	UIText * description;
	page->GetCodeDataObject(TUIText, description, "descriptiontext");

	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * nameField;
	page->GetCodeDataObject(TUIText, nameField, "genericNameField");

	if(planetField->GetLocalText().empty())
	{
		StringId invalidPlanet("ui_quest", "invalid_planet");
		CuiSystemMessageManager::sendFakeSystemMessage(invalidPlanet.localize());

		return false;
	}

	if(nameField->GetLocalText().empty())
	{
		StringId invalidLocName("ui_quest", "invalid_location_name");
		CuiSystemMessageManager::sendFakeSystemMessage(invalidLocName.localize());

		return false;
	}

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());

	std::string narrowDesc = Unicode::wideToNarrow(description->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	Unicode::String wideName = TextManager::filterText(nameField->GetLocalText());
	std::string narrowName = Unicode::wideToNarrow(wideName);

	if(narrowName.size() > 128)
		narrowName = narrowName.substr(0, 128);

	std::string coordinates = Unicode::wideToNarrow(xField->GetLocalText());
	coordinates.append(1, ':');
	coordinates += Unicode::wideToNarrow(yField->GetLocalText());
	coordinates.append(1, ':');
	coordinates += Unicode::wideToNarrow(zField->GetLocalText());
	coordinates.append(1, ':');
	coordinates += Unicode::wideToNarrow(planetField->GetLocalText());
	coordinates.append(1, ':');
	coordinates += narrowName;

	parameters.push_back(coordinates);

	return true;
}

bool SwgCuiQuestBuilder::getLootTaskInfo(UIPage *page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	UIText * count;
	page->GetCodeDataObject(TUIText, count, "number");

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());
	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	std::string narrowCount = Unicode::wideToNarrow(count->GetLocalText());

	int value = atoi(narrowCount.c_str());

	if(value <= 0 || value > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Invalid number of items specified for loot task, must be greater than 0 but no greater than 100."));
		return false;
	}
	char itoaBuffer[32];
	_itoa(value, itoaBuffer, 10);
	narrowCount = itoaBuffer;

	UIString templateName;
	page->GetProperty(SwgCuiQuestBuilder::TemplateProperty, templateName);

	std::string narrowItem = Unicode::wideToNarrow(templateName);
	parameters.push_back(narrowItem);
	parameters.push_back(narrowCount);

	return true;
}

bool SwgCuiQuestBuilder::getPvpKillTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	UNREF(parameters);
	
	if(!page)
		return false;

	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());

	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	UIText * number;
	page->GetCodeDataObject(TUIText, number, "numfield");

	std::string narrowNum = Unicode::wideToNarrow(number->GetLocalText());

	int value = atoi(narrowNum.c_str());
	if(value <= 0 || value > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Invalid count specified for PvP Kill task. Please use a value greater than 0 and no larger than 100."));
		return false;
	}

	char itoaBuffer[32];
	_itoa(value, itoaBuffer, 10);
	narrowNum = itoaBuffer;

	parameters.push_back(narrowNum);

	UIText * rank;
	page->GetCodeDataObject(TUIText, rank, "rankfield");
	std::string narrowRank = Unicode::wideToNarrow(rank->GetLocalText());

	parameters.push_back(narrowRank);

	return true;
}

bool SwgCuiQuestBuilder::getPerformTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());

	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	UIText * xField;
	page->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	page->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	page->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	page->GetCodeDataObject(TUIText, planetField, "planet");

	UIComboBox * sets;
	page->GetCodeDataObject(TUIComboBox, sets, "sets");

	UIComboBox * actionCombo;
	page->GetCodeDataObject(TUIComboBox, actionCombo, "actioncombo");

	std::string coordinates = Unicode::wideToNarrow(xField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(yField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(zField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(planetField->GetLocalText());

	parameters.push_back(coordinates);

	UIText * venue;
	page->GetCodeDataObject(TUIText, venue, "venue");

	std::string narrowVenue = Unicode::wideToNarrow(venue->GetLocalText());

	parameters.push_back(narrowVenue);

	// Action
	int selectedIndex = actionCombo->GetSelectedIndex();
	char buffer[32];
	memset(buffer, 0, 32);
	sprintf(buffer, "%d", selectedIndex);
	std::string narrowIndex = buffer;

	parameters.push_back(narrowIndex);

	//Sets
	selectedIndex = sets->GetSelectedIndex();
	memset(buffer, 0, 32);
	sprintf(buffer, "%d", selectedIndex + 3);
	narrowIndex = buffer;

	parameters.push_back(narrowIndex);

	return true;
}

bool SwgCuiQuestBuilder::getCommTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;
	
	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());
	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	UIText * npc;
	page->GetCodeDataObject(TUIText, npc, "npc");
	std::string narrowNPC = Unicode::wideToNarrow(npc->GetLocalText());

	parameters.push_back(narrowNPC);

	UIText * message;
	page->GetCodeDataObject(TUIText, message, "message");
	std::string narrowMessage = Unicode::wideToNarrow(TextManager::filterText(message->GetLocalText()));

	if(narrowMessage.size() > 1024 || narrowMessage.empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Comm message is too long or invalid. Message must be more than 1 character and less than 1024."));
		return false;
	}

	parameters.push_back(narrowMessage);

	return true;
}

bool SwgCuiQuestBuilder::getCraftTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;
	
	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	UIText * number;
	page->GetCodeDataObject(TUIText, number, "number");

	std::string narrowNumber = Unicode::wideToNarrow(number->GetLocalText());
	int intNumber = atoi(narrowNumber.c_str());

	if(intNumber <= 0 || intNumber > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Invalid count specified for crafting task. Please use a value greater than 0 but no greater than 100."));
		return false;
	}
	char itoaBuffer[32];
	_itoa(intNumber, itoaBuffer, 10);
	narrowNumber = itoaBuffer;

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());
	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	UIString templateName;
	page->GetProperty(SwgCuiQuestBuilder::TemplateProperty, templateName);

	std::string narrowItem = Unicode::wideToNarrow(templateName);
	parameters.push_back(narrowItem);
	parameters.push_back(narrowNumber);

	return true;
}

bool SwgCuiQuestBuilder::getPvpDestroyTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	UNREF(parameters);

	if(!page)
		return false;

	UIText * title;
	page->GetCodeDataObject(TUIText, title, "title");

	UIText * desc;
	page->GetCodeDataObject(TUIText, desc, "descriptiontext");

	std::string narrowTitle = Unicode::wideToNarrow(title->GetLocalText());

	std::string narrowDesc = Unicode::wideToNarrow(desc->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, narrowDesc))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(narrowDesc);

	UIText * number;
	page->GetCodeDataObject(TUIText, number, "numfield");

	std::string narrowNum = Unicode::wideToNarrow(number->GetLocalText());

	int value = atoi(narrowNum.c_str());
	if(value <= 0 || value > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Invalid count specified for PvP Destroy task. Please use a value greater than 0 but no greater than 100."));
		return false;
	}

	char itoaBuffer[32];
	_itoa(value, itoaBuffer, 10);
	narrowNum = itoaBuffer;

	parameters.push_back(narrowNum);

	UIText * structure;
	page->GetCodeDataObject(TUIText, structure, "structurefield");
	std::string narrowStructure = Unicode::wideToNarrow(structure->GetLocalText());

	parameters.push_back(narrowStructure);

	return true;
}

bool SwgCuiQuestBuilder::getKillLootTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * titleText;
	page->GetCodeDataObject(TUIText, titleText, "title");

	std::string narrowTitle = Unicode::wideToNarrow(titleText->GetLocalText());

	UIText * description;
	page->GetCodeDataObject(TUIText, description, "descriptiontext");

	std::string descriptionString = Unicode::wideToNarrow(description->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, descriptionString))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(descriptionString);

	UIText * countText;
	page->GetCodeDataObject(TUIText, countText, "countValue");

	UIString propertyValue;
	if(page->HasProperty(SwgCuiQuestBuilder::KillCreatureProperty))
	{
		if(page->GetProperty(SwgCuiQuestBuilder::KillCreatureProperty, propertyValue))
		{
			std::string killString = "creature:";
			killString.append(Unicode::wideToNarrow(propertyValue));
			parameters.push_back(killString);
		}
		else
			DEBUG_FATAL(true, ("Had a property but couldn't get to it."));
	}
	else if (page->HasProperty(SwgCuiQuestBuilder::KillSpeciesProperty))
	{
		if(page->GetProperty(SwgCuiQuestBuilder::KillSpeciesProperty, propertyValue))
		{
			std::string killString = "species:";
			killString.append(Unicode::wideToNarrow(propertyValue));
			parameters.push_back(killString);
		}
		else
			DEBUG_FATAL(true, ("Had a property but couldn't get to it."));
	}
	else
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Failed to find a valid species or creature for a kill loot task."));
		return false;
	}

	UIString countValue = countText->GetLocalText();
	int debugValue = Unicode::toInt(countValue);
	if( debugValue <= 0 || debugValue > ms_maxCountAllowed)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Kill Loot task has an invalid kill count. Please enter a valid amount ( between 1 - 100 ) and try again."));
		return false;
	}

	countValue = Unicode::intToWide(debugValue);

	parameters.push_back(Unicode::wideToNarrow(countValue));

	UIText * itemField;
	page->GetCodeDataObject(TUIText, itemField, "itemfield");

	std::string narrowItem = Unicode::wideToNarrow(itemField->GetLocalText());
	if(narrowItem.size() > 128)
		narrowItem = narrowItem.substr(0, 128);
	if(narrowItem.empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Kill Loot task has an invalid item specified."));
		return false;
	}

	parameters.push_back(narrowItem);

	UIText * dropField;
	page->GetCodeDataObject(TUIText, dropField, "dropfield");
	UIString dropWide = dropField->GetLocalText();
	int dropInt = Unicode::toInt(dropWide);
	if( dropInt <= 0 || dropInt > 100)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Kill Loot task has an invalid drop rate. Please enter a valid amount ( 1 - 100) and try again."));
		return false;
	}
	dropWide = Unicode::intToWide(dropInt);

	std::string narrowDrop = Unicode::wideToNarrow(dropWide);

	parameters.push_back(narrowDrop);

	return true;
}

bool SwgCuiQuestBuilder::getSpawnTaskInfo(UIPage * page, std::vector<std::string> & parameters)
{
	if(!page)
		return false;

	UIText * titleText;
	page->GetCodeDataObject(TUIText, titleText, "title");

	std::string narrowTitle = Unicode::wideToNarrow(titleText->GetLocalText());

	UIText * description;
	page->GetCodeDataObject(TUIText, description, "descriptiontext");

	std::string descriptionString = Unicode::wideToNarrow(description->GetLocalText());

	if(!TruncateTitleAndDescription(narrowTitle, descriptionString))
		return false;

	parameters.push_back(narrowTitle);
	parameters.push_back(descriptionString);

	UIText * xField;
	page->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	page->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	page->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	page->GetCodeDataObject(TUIText, planetField, "planet");

	UIText * nameField;
	page->GetCodeDataObject(TUIText, nameField, "creaturename");

	UIText * countField;
	page->GetCodeDataObject(TUIText, countField, "countValue");

	std::string countNarrow = Unicode::wideToNarrow(countField->GetLocalText());
	int debugValue = atoi(countNarrow.c_str());
	if(debugValue <= 0 || debugValue > ms_maxSpawnCount)
	{
		StringId invalidCount("ui_quest", "invalid_spawn_count");
		CuiSystemMessageManager::sendFakeSystemMessage(invalidCount.localize());

		return false;
	}

	char validBuffer[32];
	memset(validBuffer, 0, 32);
	sprintf(validBuffer, "%d", debugValue);
	countNarrow = validBuffer;

	parameters.push_back(countNarrow);

	std::string coordinates = Unicode::wideToNarrow(xField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(yField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(zField->GetLocalText());
	coordinates.append(1, ' ');
	coordinates += Unicode::wideToNarrow(planetField->GetLocalText());

	parameters.push_back(coordinates);

	Unicode::String filteredName = TextManager::filterText(nameField->GetLocalText());

	parameters.push_back(Unicode::wideToNarrow(filteredName));

	return true;
}

void SwgCuiQuestBuilder::updateQuestInfo()
{
	int maxLevel = 0;
	int totalFlags = 0;

	TaskPageMap::iterator iter = ms_taskButtonsToTaskPages.begin();
	for(; iter != ms_taskButtonsToTaskPages.end(); ++iter)
	{
		TaskInfo * taskInfo = (*iter).second;
		if(!taskInfo)
			continue;

		if(taskInfo->level > maxLevel)
			maxLevel = taskInfo->level;
		
		if((totalFlags & taskInfo->flags) == 0)
			totalFlags ^= taskInfo->flags;
	}

	std::string requiredString;

	int flagCount = 0;
	for(int i = 0; i < ms_totalRequiredFlags; ++i)
	{
		if(totalFlags & ms_requiredFlags[i])
		{
			if(requiredString.empty())
				requiredString.append(ms_requiredFlagColors[i]);
			else
			{
				if(flagCount % 2 == 0)
					requiredString.append(1, '\n');
				else
					requiredString.append(5, ' ');

				requiredString.append(ms_requiredFlagColors[i]);
			}

			++flagCount;
		}
	}

	int totalPlayerTasks = getSkillModValue(ms_taskAmountSkill);
	char taskCountBuffer[32];
	memset(taskCountBuffer, 0, 32);
	sprintf(taskCountBuffer, "%d/%d", ms_taskButtonsToTaskPages.size(), totalPlayerTasks);
	m_questLevel->SetLocalText(Unicode::intToWide(maxLevel));
	m_questTaskCount->SetLocalText(Unicode::narrowToWide(taskCountBuffer));
	m_requiredText->SetLocalText(Unicode::narrowToWide(requiredString));

}

bool SwgCuiQuestBuilderNamespace::TruncateTitleAndDescription(std::string & title, std::string & description)
{
	if(title.size() > 110)
	{
		title = title.substr(0, 110);
		title.append(ms_ellipses);
	}

	if(description.size() > 1000)
	{
		description = description.substr(0, 1000);
		description.append(ms_ellipses);
	}

	if(title.empty() || description.empty())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("A Task Title or Description is empty. Please enter a valid string."));
		return false;
	}

	Unicode::String filteredTitle = TextManager::filterText(Unicode::narrowToWide(title));
	Unicode::String filteredDesc = TextManager::filterText(Unicode::narrowToWide(description));

	title = Unicode::wideToNarrow(filteredTitle);
	description = Unicode::wideToNarrow(filteredDesc);

	return true;
}

void SwgCuiQuestBuilderNamespace::ParseWaypointString(std::string inputString, WaypointText & outWaypoint)
{
	if(inputString.empty())
		return;

	// The input string expected to be in the format: waypoint:planetName,X,Y,Z,CellName,NpcName
	std::string::size_type i = 0 + strlen("waypoint:");

	// Planet Name;
	std::string::size_type commaToken = inputString.find(',', i);
	outWaypoint.planet = inputString.substr(i, (commaToken - i));
	i = commaToken + 1;

	// X
	commaToken = inputString.find(',', i);
	outWaypoint.x = inputString.substr(i, (commaToken - i));
	i = commaToken + 1;

	// Y
	commaToken = inputString.find(',', i);
	outWaypoint.y = inputString.substr(i, (commaToken - i));
	i = commaToken + 1;

	// Z
	commaToken = inputString.find(',', i);
	outWaypoint.z = inputString.substr(i, (commaToken - i));
	i = commaToken + 1;

	// Cell Name
	commaToken = inputString.find(',', i);
	outWaypoint.cellName = inputString.substr(i, (commaToken - i));
	i = commaToken + 1;

	// NPC Name
	outWaypoint.npcName = inputString.substr(i, (inputString.size() - i));

}

std::string SwgCuiQuestBuilderNamespace::ObjectTemplateToSharedTemplate(std::string objectString)
{
	// Just need to find the last '/' and add 'shared_' after it.
	std::string::size_type loc = objectString.find_last_of('/');
	if(loc == std::string::npos)
		return std::string();

	std::string output = objectString;
	output.insert(loc + 1, ms_shared);

	return output;

}

void SwgCuiQuestBuilderNamespace::clearTempObjects()
{
	std::vector<Object const *>::iterator iter = ms_tempObjects.begin();
	for(; iter != ms_tempObjects.end(); ++iter)
	{
		if((*iter))
			delete (*iter);
	}

	ms_tempObjects.clear();
}

void SwgCuiQuestBuilderNamespace::parseQuestData(std::string inputString, int & level, int & flags)
{
	flags = 0;
	char delimiter = ':';

	std::string::size_type loc = inputString.find(delimiter);

	if(loc == std::string::npos)
		return;

	//First field should be quest_data, we don't really care about that.
	// TO_DO: Put a sanity check here?
	++loc;
	std::string::size_type nextLoc = inputString.find(delimiter, loc);
	
	// Level
	std::string levelValue = inputString.substr(loc, (nextLoc - loc));
	level = atoi(levelValue.c_str());

	// Now we can automate this lovely process.
	loc = nextLoc + 1;
	nextLoc = inputString.find(delimiter, loc);
	int flagIndex = 4; // <- change this most likely.
	while(nextLoc != std::string::npos)
	{
		std::string flagValue = inputString.substr(loc, (nextLoc - loc));
		int flagInt = atoi(flagValue.c_str());
		if(flagInt)
			flags = flags ^ ms_requiredFlags[flagIndex];
		
		++flagIndex;
		loc = nextLoc + 1;
		nextLoc = inputString.find(delimiter, loc);
	}

	std::string flagValue = inputString.substr(loc, (inputString.size() - loc));
	int flagInt = atoi(flagValue.c_str());
	if(flagInt)
		flags = flags ^ ms_requiredFlags[flagIndex];
}

void SwgCuiQuestBuilderNamespace::parseCategoryData(std::string inputString, int & flags)
{
	flags = 0;
	char delimiter = ':';

	std::string::size_type loc = inputString.find(delimiter);

	if(loc == std::string::npos)
		return;

	//First field should be quest_data, we don't really care about that.
	// TO_DO: Put a sanity check here?
	++loc;
	std::string::size_type nextLoc = inputString.find(delimiter, loc);

	// Now we can automate this lovely process.
	//loc = nextLoc + 1;
	//nextLoc = inputString.find(delimiter, loc);
	int flagIndex = 0; // <- change this most likely.
	while(nextLoc != std::string::npos)
	{
		std::string flagValue = inputString.substr(loc, (nextLoc - loc));
		int flagInt = atoi(flagValue.c_str());
		if(flagInt)
			flags = flags ^ ms_requiredFlags[flagIndex];

		++flagIndex;
		loc = nextLoc + 1;
		nextLoc = inputString.find(delimiter, loc);
	}

	std::string flagValue = inputString.substr(loc, (inputString.size() - loc));
	int flagInt = atoi(flagValue.c_str());
	if(flagInt)
		flags = flags ^ ms_requiredFlags[flagIndex];

};

void SwgCuiQuestBuilder::loadTasksFromRecipeData()
{
	m_overrideCheck->SetVisible(false);
	m_overrideCheck->SetChecked(false);

	NetworkId const & id = CuiRecipeManager::getRecipeOID();
	if(id == NetworkId::cms_invalid)
		return;

	Object * obj = NetworkIdManager::getObjectById(id);

	if(!obj)
		return;

	ClientPlayerQuestObject * recipeObj = dynamic_cast<ClientPlayerQuestObject *>(obj);

	if(!recipeObj)
		return;

	// Now populate the tasks.
	int totalTasks = recipeObj->getTotalTasks();

	// Check for invalid draft object.
	if(totalTasks != recipeObj->getTaskDataCount())
	{
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "Chronicle Draft for the quest '%s' is corrupt and cannot be loaded into the editor.", recipeObj->getQuestTitle().c_str());
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(buffer));
		return;
	}

	m_overrideCheck->SetVisible(true);

	// Set the Quest Title
	m_questTitle->SetLocalText(Unicode::narrowToWide(recipeObj->getQuestTitle()));

	// Set Quest Description.
	m_questDescription->SetLocalText(Unicode::narrowToWide(recipeObj->getQuestDescription()));

	for(int i = 0; i < totalTasks; ++i)
	{
		std::string taskData = recipeObj->getTaskData(i);

		char delimiter = '~';

		std::string::size_type iterPos = taskData.find(delimiter);

		if(iterPos == std::string::npos && taskData.empty())
			continue;
		else if(iterPos == std::string::npos)
		{
			iterPos = taskData.size(); // Edge case. We have 1 task with no additional info (i.e. pre-written comm messages).
		}

		std::string slotName = taskData.substr(0, iterPos);

		CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
		if(!slotInfo)
			continue;

		std::string::size_type uiType = slotInfo->categories[0].find("UIType");
		std::string::size_type colonIndex = slotInfo->categories[0].find(':');
		if(colonIndex == std::string::npos || uiType == std::string::npos)
		{
			continue;
		}

		std::string UITypeString = slotInfo->categories[0].substr(colonIndex + 1);
		UIPage * newTask = NULL;

		CrcLowerString uiTaskType(UITypeString.c_str());

		if(uiTaskType.getCrc() == ms_KillType)
		{
			newTask = addKillTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_LocationType)
		{
			newTask = addLocationTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_LootType)
		{
			newTask = addLootTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_PvpKillType)
		{
			newTask = addPvpKillTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_PerformType)
		{
			newTask = addPerformTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_CommType)
		{
			newTask = addCommTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_CraftType)
		{
			newTask = addCraftTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_PvpDestroyType)
		{
			newTask = addPvpDestroyTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_KillLootType)
		{
			newTask = addKillLootTask(NULL, taskData);
		}
		else if (uiTaskType.getCrc() == ms_SpawnType)
		{
			newTask = addSpawnTask(NULL, taskData);
		}
		else
		{
			// Failed to find an acceptable task.
			DEBUG_WARNING(true, ("Invalid UIType: %s", UITypeString.c_str()));
			continue;
		}

		if(newTask)
		{
			SetTitleAndDescription(newTask, recipeObj->getTaskTitle(i), recipeObj->getTaskDescription(i));
		}

	}

	std::string const & rewardsData = recipeObj->getRewardData();

	if(!rewardsData.empty())
	{
		char delimiter = '~';

		std::string::size_type iterPos = rewardsData.find(delimiter);
		std::string::size_type oldPos = 0;

		while(iterPos != std::string::npos)
		{
			std::string oidString = rewardsData.substr(oldPos, (iterPos - oldPos));
			
			NetworkId objID(oidString);

			Object * rewardObj = NetworkIdManager::getObjectById(objID);
			ClientObject * rewardClientObj = rewardObj ? rewardObj->asClientObject() : NULL;
			if(rewardClientObj)
			{				// Last check, make sure we don't already have this object in one of our viewers.
				for(int j = 0; j < ms_numOfLootViewers; ++j)
				{
					if(m_lootViewers[j]->getLastObject() != NULL)
						continue;

					m_lootViewers[j]->clearObjects();

					m_lootViewers[j]->addObject(*rewardClientObj);

					m_lootViewers[j]->SetTooltip(rewardClientObj->getLocalizedName());

					break;
				}
			}

			oldPos = iterPos + 1;
			iterPos = rewardsData.find(delimiter, oldPos);
		}

		// Credits
		std::string creditsString = rewardsData.substr(oldPos, (rewardsData.size() - oldPos));

		m_creditsText->SetLocalText(Unicode::narrowToWide(creditsString));
	}

	m_recipeCheck->SetChecked(true);

	m_shareCheck->SetChecked(false);
	m_shareCheck->SetEnabled(false);


	CuiRecipeManager::setRecipeObject(NetworkId::cms_invalid); // Done with this recipe.
	
}

void SwgCuiQuestBuilderNamespace::SetTitleAndDescription(UIPage * page, std::string const & title, std::string const & description)
{
	if(!page)
		return;

	UIText * pageTitle;
	page->GetCodeDataObject(TUIText, pageTitle, "title");

	UIText * pageDesc;
	page->GetCodeDataObject(TUIText, pageDesc, "descriptiontext");

	if(!pageTitle || !pageDesc)
		return;

	pageTitle->SetLocalText(Unicode::narrowToWide(title));
	pageDesc->SetLocalText(Unicode::narrowToWide(description));
}

void SwgCuiQuestBuilderNamespace::parseString(std::string input, char delimiter, std::vector<std::string> & output)
{
	output.clear();

	std::string::size_type iter = input.find(delimiter);

	if(iter == std::string::npos && input.empty())
		return;
	else if (iter == std::string::npos)
	{
		output.push_back(input); // Edge case.
		return;
	}

	std::string::size_type oldIter = 0;

	do 
	{
		std::string subString = input.substr(oldIter, (iter - oldIter));
		
		output.push_back(subString);

		oldIter = iter + 1;

		iter = input.find(delimiter, oldIter);

	} while(iter != std::string::npos);

	// one final string.
	std::string finalString = input.substr(oldIter, (input.size() - oldIter));

	output.push_back(finalString);
}

void SwgCuiQuestBuilder::OnConfirmClearQuest(const CuiMessageBox & box)
{
	if(box.completedAffirmative())
	{
		clearTaskRecipe();
		createSlotMap();
		populateTasksTable();
		updateQuestInfo();

		m_taskCategory->SetSelectedIndex(0);
		m_recipeCheck->SetChecked(false);
		m_overrideCheck->SetChecked(false);

		ms_currentRecipeID = NetworkId::cms_invalid;
	}
}

void SwgCuiQuestBuilder::OnConfirmClearRewards(const CuiMessageBox &box)
{
	if(box.completedAffirmative())
	{
		for(int i = 0; i < ms_numOfLootViewers; ++i)
			m_lootViewers[i]->clearObjects();

		m_creditsText->SetLocalText(Unicode::narrowToWide("0"));
	}
}

void SwgCuiQuestBuilder::OnConfirmChangeDraft(const CuiMessageBox &box)
{
	if(box.completedAffirmative())
	{
		for(int i = 0; i < ms_numOfLootViewers; ++i)
			m_lootViewers[i]->clearObjects();

		m_creditsText->SetLocalText(Unicode::narrowToWide("0"));

		m_recipeCheck->SetChecked(true, false);

		// Draft mode, disable sharing.
		m_shareCheck->SetChecked(false);
		m_shareCheck->SetEnabled(false);
	}
}

void SwgCuiQuestBuilder::refreshCollectionData()
{
	createSlotMap();

	UIBaseObject::UIObjectList taskList;
	m_taskComposite->GetChildren(taskList);

	UIBaseObject::UIObjectList::iterator iter = taskList.begin();
	for(; iter != taskList.end(); ++iter)
	{
		TaskPageMap::iterator pageIter = NULL;

		if((*iter)->IsA(TUIPage))
		{
			pageIter = ms_taskButtonsToTaskPages.find(static_cast<UIPage*>((*iter)));

			if(pageIter == ms_taskButtonsToTaskPages.end())
				continue;
		}
		else
			continue;

		TaskInfo * currentTask = (*pageIter).second;
		
		TaskSlotInfo * currentSlotInfo = NULL;

		std::map<std::string, CrcLowerString>::iterator slotConvIter = ms_SlotNamesToSlotCrcMap.find(currentTask->slotName);
		if(slotConvIter!= ms_SlotNamesToSlotCrcMap.end())
		{
			SlotMap::iterator slotIter = m_slotMap.find((*slotConvIter).second);
			if(slotIter != m_slotMap.end())
			{
				currentSlotInfo = &(*slotIter).second;

				currentSlotInfo->recipeRequirement++;
			}
		}
	}

	// Repopulate our tasks table.
	UIString filterText = m_taskFilter->GetLocalText();

	std::string narrowFilter = Unicode::wideToNarrow(filterText);

	const UIData* const data = m_taskCategory->GetDataAtIndex (m_taskCategory->GetSelectedIndex());
	if (!data)
		return;


	UINarrowString textProperty;
	if (data->GetPropertyNarrow (SwgCuiQuestBuilder::FilterProperty, textProperty))
	{
		populateTasksTable(textProperty.c_str(), narrowFilter.empty() ? NULL : narrowFilter.c_str());
	}		

}

void SwgCuiQuestBuilder::captureLocation(UIPage *page, int & taskFlags)
{
	if(!page)
		return;

	if(Game::isSpace())
	{
		StringId spaceInvalid("ui_quest", "space_capture");
		CuiSystemMessageManager::sendFakeSystemMessage(spaceInvalid.localize());
		return;
	}

	CreatureObject * playerCreature = Game::getPlayerCreature();

	if(!playerCreature)
		return;

	if(!playerCreature->isInWorldCell())
	{
		StringId cellInvalid("ui_quest", "cell_capture");
		CuiSystemMessageManager::sendFakeSystemMessage(cellInvalid.localize());
		return;
	}

	Vector const playerWorldPos = playerCreature->getPosition_w();
	std::string currentScene = Game::getSceneId();

	UIText * xField;
	page->GetCodeDataObject(TUIText, xField, "x");

	UIText * yField;
	page->GetCodeDataObject(TUIText, yField, "y");

	UIText * zField;
	page->GetCodeDataObject(TUIText, zField, "z");

	UIText * planetField;
	page->GetCodeDataObject(TUIText, planetField, "planet");

	xField->SetPreLocalized(true);
	yField->SetPreLocalized(true);
	zField->SetPreLocalized(true);
	planetField->SetPreLocalized(true);

	char buffer[128];
	memset(buffer, 0, 128);
	sprintf(buffer, "%d", static_cast<int>(playerWorldPos.x));
	xField->SetLocalText(Unicode::narrowToWide(buffer));

	memset(buffer, 0, 128);
	sprintf(buffer, "%d", static_cast<int>(playerWorldPos.y));
	yField->SetLocalText(Unicode::narrowToWide(buffer));

	memset(buffer, 0, 128);
	sprintf(buffer, "%d", static_cast<int>(playerWorldPos.z));
	zField->SetLocalText(Unicode::narrowToWide(buffer));

	planetField->SetLocalText(Unicode::narrowToWide(currentScene));

	if(currentScene.compare("kashyyyk") == 0)  // Kash = 6, Must = 7
		taskFlags = taskFlags ^ ms_requiredFlags[6];

	if(currentScene.compare("mustafar") == 0)
		taskFlags = taskFlags ^ ms_requiredFlags[7];
}