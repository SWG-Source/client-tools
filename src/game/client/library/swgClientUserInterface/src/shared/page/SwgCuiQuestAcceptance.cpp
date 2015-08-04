//======================================================================
//
// SwgCuiQuestAcceptance.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiQuestAcceptance.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStaticLootItemManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsQuest.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectTemplateList.h"
#include "swgClientUserInterface/SwgCuiQuestJournal.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIDataSourceContainer.h"
#include "UIText.h"
#include "UITreeView.h"

#include <string>

namespace SwgCuiQuestAcceptanceNamespace
{
	char const * const cms_soloType = "solo";

	int const cms_maxItems = 10;

	Object * ms_itemRewardObjects[cms_maxItems];
	std::string ms_itemRewards[cms_maxItems];

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

	float const cms_itemWaitTime = 0.5f;

	SwgCuiQuestAcceptance::WindowType ms_windowType = SwgCuiQuestAcceptance::WT_requestAccept;
	bool ms_isExclusiveReward = false;
	float ms_needsUpdateIn = 0.0f;

	std::string ms_rewardSelection;
}

using namespace SwgCuiQuestAcceptanceNamespace;

//----------------------------------------------------------------------

SwgCuiQuestAcceptance::SwgCuiQuestAcceptance   (UIPage & page) :
CuiMediator           ("SwgCuiQuestAcceptance", page)
 , UIEventCallback()
 , m_callback         (new MessageDispatch::Callback)
 , m_titleText      (NULL)
 , m_descriptionText(NULL)
 , m_acceptButton   (NULL)
 , m_okButton       (NULL)
 , m_completeButton (NULL)
 , m_cancelButton   (NULL)
 , m_levelText      (NULL)
 , m_typeText       (NULL)
 , m_questCrc       (0)
 , m_typePage       (NULL)
 , m_windowCaptionText   (NULL)
 , m_experienceRewardText(NULL)
 , m_factionRewardText   (NULL)
 , m_moneyRewardText     (NULL)
 , m_rewardLabel         (NULL)
 , m_inclusiveText       (NULL)
 , m_exclusiveText       (NULL)
 , m_chooseOneText       (NULL)
 , m_completionPanel           (NULL)
{
	getCodeDataObject(TUIText,    m_titleText,             "texttitle");
	getCodeDataObject(TUIText,    m_descriptionText,       "textdescription");
	getCodeDataObject(TUIButton,  m_acceptButton,          "buttonAccept");
	getCodeDataObject(TUIButton,  m_okButton,              "buttonOk");
	getCodeDataObject(TUIButton,  m_completeButton,        "buttonComplete");
	getCodeDataObject(TUIButton,  m_cancelButton,          "buttonCancel");
	getCodeDataObject(TUIText,    m_levelText,             "textLevel");
	getCodeDataObject(TUIText,    m_typeText,              "textType");
	getCodeDataObject(TUIPage,    m_typePage,              "typePage");
	getCodeDataObject(TUIText,    m_windowCaptionText,     "windowcaptiontext");
	getCodeDataObject(TUIText,    m_experienceRewardText,  "experienceRewardText");
	getCodeDataObject(TUIText,    m_factionRewardText,     "factionRewardText");
	getCodeDataObject(TUIText,    m_moneyRewardText,       "moneyRewardText");
	getCodeDataObject(TUIText,    m_rewardLabel,           "rewardLabel");
	getCodeDataObject(TUIText,    m_inclusiveText,         "inclusiveText");
	getCodeDataObject(TUIText,    m_exclusiveText,         "exclusiveText");
	getCodeDataObject(TUIText,    m_chooseOneText,         "chooseOneText");
	getCodeDataObject(TUIPage,    m_completionPanel,       "completionPanel");

	std::string codeDataName;
   int i;
	for(i = 0; i < cms_maxItems; ++i)
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
		m_rewardItemSelectionButtons[i] = dynamic_cast<UIButton *>(m_rewardItemPages[i]->GetChild("button"));
		registerMediatorObject(*m_rewardItemSelectionButtons[i], true);

		ms_itemRewardObjects[i] = NULL;
	}

	registerMediatorObject (*m_acceptButton, true);
	registerMediatorObject (*m_completeButton, true);
	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_okButton, true);

	m_titleText->Clear();
	m_descriptionText->Clear();
	m_titleText->SetPreLocalized (true);
	m_descriptionText->SetPreLocalized (true);
	m_levelText->SetPreLocalized (true);
	m_levelText->Clear();
	m_typeText->SetPreLocalized (true);
	m_typeText->Clear();
	m_typePage->SetVisible(false);
	m_windowCaptionText->SetPreLocalized (true);
	m_experienceRewardText->SetPreLocalized(true);
	m_experienceRewardText->Clear();
	m_experienceRewardText->SetVisible(false);
	m_factionRewardText->SetPreLocalized(true);
	m_factionRewardText->Clear();
	m_factionRewardText->SetVisible(false);
	m_moneyRewardText->SetPreLocalized(true);
	m_moneyRewardText->Clear();
	m_moneyRewardText->SetVisible(false);
	m_inclusiveText->SetVisible(false);
	m_exclusiveText->SetVisible(false);
	m_chooseOneText->SetVisible(false);
	m_completionPanel->SetVisible(false);
	for(int i2 = 0; i < cms_maxItems; ++i2)
	{
		m_rewardItemPages[i2]->SetVisible(false);
	}

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	setStickyVisible(true);
}

//----------------------------------------------------------------------

SwgCuiQuestAcceptance::~SwgCuiQuestAcceptance ()
{
	delete m_callback;
	m_callback = 0;

	m_titleText            = NULL;
	m_descriptionText      = NULL;
	m_acceptButton         = NULL;
	m_okButton             = NULL;
	m_completeButton       = NULL;
	m_cancelButton         = NULL;
	m_levelText            = NULL;
	m_typeText             = NULL;
	m_typePage             = NULL;
	m_experienceRewardText = NULL;
	m_factionRewardText    = NULL;
	m_moneyRewardText      = NULL;
	m_rewardLabel          = NULL;
	m_inclusiveText        = NULL;
	m_exclusiveText        = NULL;
	m_chooseOneText        = NULL;
	m_completionPanel      = NULL;
	for(int i = 0; i < cms_maxItems; ++i)
	{
		m_rewardItemPages[i]            = NULL;
		m_rewardItemViewers[i]          = NULL;
		m_rewardItemNames[i]            = NULL;
		m_rewardItemTexts[i]            = NULL;
		m_rewardItemSelectionButtons[i] = NULL;
		m_rewardItemSelectionBoxes[i]   = NULL;
		delete ms_itemRewardObjects[i];
		ms_itemRewardObjects[i]         = NULL;
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::performActivate   ()
{
	setIsUpdating(true);
	m_callback->connect (*this, &SwgCuiQuestAcceptance::onAttributesChanged, static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));
	m_callback->connect (*this, &SwgCuiQuestAcceptance::onAttributesChanged, static_cast<CuiStaticLootItemManager::Messages::AttributesChanged *> (0));
	CuiManager::requestPointer(true); 
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::performDeactivate ()
{
	CuiManager::requestPointer(false); 
	m_callback->disconnect (*this, &SwgCuiQuestAcceptance::onAttributesChanged, static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));
	m_callback->disconnect (*this, &SwgCuiQuestAcceptance::onAttributesChanged, static_cast<CuiStaticLootItemManager::Messages::AttributesChanged *> (0));
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::OnButtonPressed (UIWidget * context)
{
	char buffer[256];
	_itoa(m_questCrc, buffer, 10);
	std::string params = buffer;
	if(context == m_acceptButton)
	{
		static uint32 const hash_acceptQuest = Crc::normalizeAndCalculate ("acceptQuest");
		ClientCommandQueue::enqueueCommand (hash_acceptQuest, NetworkId::cms_invalid, Unicode::narrowToWide (params.c_str()));
		closeThroughWorkspace();
	}
	else if (context == m_completeButton)
	{
		if(ms_isExclusiveReward && ms_rewardSelection.empty())
		{
			CuiMessageBox::createInfoBox (CuiStringIdsQuest::pick_a_reward.localize());
			return;
		}
		params += " ";
		params += getRewardSelection();
		static uint32 const hash_acceptQuest = Crc::normalizeAndCalculate ("completeQuest");
		ClientCommandQueue::enqueueCommand (hash_acceptQuest, NetworkId::cms_invalid, Unicode::narrowToWide (params.c_str()));
		closeThroughWorkspace();
	}
	else if (context == m_cancelButton)
	{
		closeThroughWorkspace();
	}
	else if (context == m_okButton)
	{
		closeThroughWorkspace();
	}
	else
	{
		//only do reward selection on completion windows
		if((ms_windowType == WT_requestComplete) && ms_isExclusiveReward)
		{
			for(int i = 0; i < cms_maxItems; ++i)
			{
				UIButton * const b = m_rewardItemSelectionButtons[i];
				if(context == b)
				{
					m_rewardItemSelectionBoxes[i]->SetVisible(true);
					ms_rewardSelection = ms_itemRewards[i];
				}
				else
				{
					m_rewardItemSelectionBoxes[i]->SetVisible(false);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::update(float const deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	if(ms_needsUpdateIn > 0.0f)
	{
		ms_needsUpdateIn -= deltaTimeSecs;
		if(ms_needsUpdateIn <= 0.0f)
		{
			Quest const * const q = QuestManager::getQuest(m_questCrc);
			if(q)
			{
				updateDecriptionReward(*q);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::setData(WindowType const type, uint32 const questCrc)
{
	ms_windowType = type;

	Quest const * const q = QuestManager::getQuest(questCrc);
	if(q)
	{
		std::vector<std::string> const & inclusiveLootItemNames = q->getInclusiveStaticLootItemNames();
		std::vector<std::string> const & exclusiveLootItemNames = q->getExclusiveStaticLootItemNames();
		ms_isExclusiveReward = (!exclusiveLootItemNames.empty()) ? true : false;

		if(ms_windowType == WT_requestAccept)
		{
			m_windowCaptionText->SetLocalText(CuiStringIdsQuest::acceptance.localize());
			m_acceptButton->SetVisible(true);
			m_completeButton->SetVisible(false);
			m_okButton->SetVisible(false);
			m_cancelButton->SetVisible(true);
			m_completionPanel->SetVisible(false);

			//at start of quest, show the "you will get all" or the "you will get one" text
			if(!inclusiveLootItemNames.empty())
			{
				m_inclusiveText->SetVisible(true);
				m_exclusiveText->SetVisible(false);
				m_chooseOneText->SetVisible(false);
			}
			else if(!exclusiveLootItemNames.empty() && ms_isExclusiveReward)
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
		}
		else if(ms_windowType == WT_forceAccept)
		{
			m_windowCaptionText->SetLocalText(CuiStringIdsQuest::acceptance.localize());
			m_acceptButton->SetVisible(false);
			m_completeButton->SetVisible(false);
			m_okButton->SetVisible(true);
			m_cancelButton->SetVisible(false);
			m_completionPanel->SetVisible(false);

			//at start of quest, show the "you will get all" or the "you will get one" text
			if(!inclusiveLootItemNames.empty())
			{
				m_inclusiveText->SetVisible(true);
				m_exclusiveText->SetVisible(false);
				m_chooseOneText->SetVisible(false);
			}
			else if(!exclusiveLootItemNames.empty() && ms_isExclusiveReward)
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
		}
		else if(ms_windowType == WT_requestComplete)
		{
			m_windowCaptionText->SetLocalText(CuiStringIdsQuest::completion.localize());
			m_acceptButton->SetVisible(false);
			m_completeButton->SetVisible(true);
			m_okButton->SetVisible(false);
			m_cancelButton->SetVisible(true);
			m_completionPanel->SetVisible(true);

			//this is the end of a quest, for rewards, either show the "you will get all" or the "pick one" text
			if(!inclusiveLootItemNames.empty())
			{
				m_inclusiveText->SetVisible(true);
				m_exclusiveText->SetVisible(false);
				m_chooseOneText->SetVisible(false);
			}
			else if(!exclusiveLootItemNames.empty() && ms_isExclusiveReward)
			{
				m_inclusiveText->SetVisible(false);
				m_exclusiveText->SetVisible(false);
				m_chooseOneText->SetVisible(true);
			}
			else
			{
				m_inclusiveText->SetVisible(false);
				m_exclusiveText->SetVisible(false);
				m_chooseOneText->SetVisible(false);
			}
		}
		else
		{
			DEBUG_FATAL(true, ("Unknown window type"));
		}

		StringId const & titleSid    = q->getJournalEntryTitle();
		StringId const & description = q->getJournalEntryDescription();

		//the set quest title
		std::string const & questColor = SwgCuiQuestJournal::getQuestColor(q->getId());
		Unicode::String title = Unicode::narrowToWide(questColor.c_str());
		title += titleSid.localize();
		m_titleText->SetLocalText(title);

		m_descriptionText->SetLocalText(description.localize());
		char buffer[256];
		_itoa(q->getLevel(), buffer, 10);
		m_levelText->SetLocalText(Unicode::narrowToWide(buffer));

		std::string const & type = q->getType();
		if((!q->getType().empty()) && (q->getType() != cms_soloType))
		{
			StringId typeSid("quest/quest_types", type);
			Unicode::String const & typeWide = Unicode::narrowToWide(" [") + typeSid.localize() + Unicode::narrowToWide("]");
			m_typeText->SetLocalText(typeWide);
			m_typePage->SetVisible(true);
		}
		else
		{
			m_typePage->SetVisible(false);
		}

		m_questCrc = questCrc;

		updateDecriptionReward(*q);
	}
	else
	{
		m_titleText->Clear();
		m_descriptionText->Clear();
		m_inclusiveText->SetVisible(false);
		m_exclusiveText->SetVisible(false);
		m_chooseOneText->SetVisible(false);

		CuiMessageBox::createInfoBox (Unicode::narrowToWide("[Debug] Unknown questCrc"));
		m_questCrc = 0;
	}

	//set no item selection
	for(int i = 0; i < cms_maxItems; ++i)
	{
		m_rewardItemSelectionBoxes[i]->SetVisible(false);
	}

	ms_rewardSelection.clear();
}

//----------------------------------------------------------------------

/**
 * Update the reward portion of the quest description window
 */
void SwgCuiQuestAcceptance::updateDecriptionReward(Quest const & quest)
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
	DEBUG_FATAL(!inclusiveLootItemNames.empty() && !exclusiveLootItemNames.empty(), ("Both AND and OR rewards type set in quest [%s], this is not allowed.  Use only one!", quest.getName().getString()));

	std::vector<std::string> const & lootItems = (!inclusiveLootItemNames.empty()) ? inclusiveLootItemNames : exclusiveLootItemNames;

	for(int i = 0; i < cms_maxItems; ++i)
		ms_itemRewards[i].clear();

	for(std::vector<std::string>::const_iterator it = lootItems.begin(); it != lootItems.end(); ++it)
	{
		bool showedItem = false;
		std::string const & lootItem = *it;
		ms_itemRewards[currentItem] = lootItem;

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
					DEBUG_WARNING(true, ("SwgCuiQuestAcceptance::updateDecriptionReward: items [%s] has itemData, but not a template_name", lootItem.c_str()));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("SwgCuiQuestAcceptance::updateDecriptionReward: items [%s] does not exist in CuiStaticLootItemManager", lootItem.c_str()));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("SwgCuiQuestAcceptance::updateDecriptionReward: LootItem is empty"));
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
		m_rewardItemNames[currentItem]->SetLocalText(StringId("static_item_n", lootItem).localize());

		showedSomeReward = true;
		++currentItem;
	}

	m_rewardLabel->SetVisible(showedSomeReward);
}

//----------------------------------------------------------------------

std::string const & SwgCuiQuestAcceptance::getRewardSelection() const
{
	return ms_rewardSelection;
}

//----------------------------------------------------------------------

void SwgCuiQuestAcceptance::onAttributesChanged (const ObjectAttributeManager::Messages::StaticItemAttributesChanged::Payload & modifiedStaticItemName)
{
	for(int i = 0; i < cms_maxItems; ++i)
	{
		std::string const & staticItemName = ms_itemRewards[i];
		if(staticItemName == modifiedStaticItemName)
		{
			Quest const * const q = QuestManager::getQuest(m_questCrc);
			if(q)
			{
				updateDecriptionReward(*q);
				break;
			}
		}
	}
}

//----------------------------------------------------------------------
