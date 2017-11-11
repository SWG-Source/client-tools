//======================================================================
//
// SwgCuiBuffBuilderBuffer.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffer.h"

#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSourceContainer.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UITreeView.h"

bool SwgCuiBuffBuilderBuffer::m_failedLastVerification = false;

namespace SwgCuiBuffBuilderBufferNamespace
{
	// points related constants
	const int ms_basePoints = 8;
	int ms_additionalPoints = 0;
	const std::string ms_creativeExpertiseName = "expertise_en_creativity_1";
	const std::string ms_pointIncreaseExpertiseSkillModName = "expertise_en_inspire_base_point_increase";

	// attribute related constants
	int ms_attributeBonusFromExpertise = 0;
	const std::string ms_inspiredFitnessExpertiseName = "expertise_en_inspired_fitness_1";
	const std::string ms_attributeBonusExpertiseSkillModName = "expertise_en_inspire_attrib_increase";

	// resistance related constants
	int ms_resistanceBonusFromExpertise = 0;
	const std::string ms_inspiredResilienceExpertiseName = "expertise_en_inspired_resilience_1";
	const std::string ms_resistanceBonusExpertiseSkillModName = "expertise_en_inspire_resist_increase";

	// trader related constants
	int ms_traderBonusFromExpertise = 0;
	const std::string ms_inspiredIndustryExpertiseName = "expertise_en_inspired_industry_1";
	const std::string ms_traderBonusExpertiseSkillModName = "expertise_en_inspire_trader_increase";

	// proc related constants
	int ms_procBonusFromExpertise = 0;
	const std::string ms_inspiredReactionsExpertiseName = "expertise_en_inspired_reactions_1";
	const std::string ms_procBonusExpertiseSkillModName = "expertise_en_inspire_proc_chance_increase";
	const std::string ms_reactiveSecondChanceComponentName = "reactive_second_chance";

	// combat related constants
	int ms_combatBonusFromExpertise = 0;
	const std::string ms_inspiredWarfareExpertiseName = "expertise_en_inspired_warfare_1";
	const std::string ms_combatBonusExpertiseSkillModName = "expertise_en_combat_buff_increase";

	int ms_totalLogos = 0;
	int ms_consecutiveFails = 0;
}

using namespace SwgCuiBuffBuilderBufferNamespace;

SwgCuiBuffBuilderBuffer::SwgCuiBuffBuilderBuffer (UIPage & page) :
CuiMediator       ("SwgCuiBuffBuilderBuffer", page),
UIEventCallback   (),
m_callback        (new MessageDispatch::Callback),
m_recipientId(),
m_clearButton(NULL),
m_cancelButton(NULL),
m_acceptButton(NULL),
m_addButton(NULL),
m_removeButton(NULL),
m_buffTree(NULL),
m_buffList(NULL),
m_buffeeList(NULL),
m_coverChargeTextBox(NULL),
m_pointsLeft(NULL),
m_totalCost(NULL),
m_componentDescription(NULL),
m_recipientName(NULL),
m_committed(false)
{
	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject(TUIButton, m_clearButton, "buttonClear");
	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	getCodeDataObject(TUIButton, m_acceptButton, "buttonAccept");
	getCodeDataObject(TUIButton, m_addButton, "buttonAdd");
	getCodeDataObject(TUIButton, m_removeButton, "buttonRemove");
	getCodeDataObject(TUITreeView,m_buffTree,"buffTree");
	getCodeDataObject(TUIList, m_buffList, "buffList");
	getCodeDataObject(TUIList, m_buffeeList, "buffeeList");
	getCodeDataObject (TUITextbox, m_coverChargeTextBox, "coverChargeTextBox");
	getCodeDataObject (TUIText, m_pointsLeft, "pointsLeft");
	getCodeDataObject (TUIText, m_totalCost, "totalCost");
	getCodeDataObject (TUIText, m_componentDescription, "labelComponentDescription");
	getCodeDataObject (TUIText, m_recipientName, "recipientName");

	m_callback->connect (*this, &SwgCuiBuffBuilderBuffer::onBuffBuilderChangeReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderChangeReceived *>(0));
	m_callback->connect (*this, &SwgCuiBuffBuilderBuffer::onBuffBuilderCancelReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderCancelReceived *>(0));

	registerMediatorObject (*m_clearButton, true);
	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_acceptButton, true);
	registerMediatorObject (*m_addButton, true);
	registerMediatorObject (*m_removeButton, true);
	registerMediatorObject (*m_buffTree, true);
	registerMediatorObject (*m_buffList, true);
	registerMediatorObject (*m_buffeeList, true);
	registerMediatorObject (*m_coverChargeTextBox, true);

	initializeBuffTree();
	initializeExpertiseModifiers();
	m_buffeeList->Clear();
	m_buffList->Clear();
	m_coverChargeTextBox->SetText(Unicode::narrowToWide("0"));
	m_acceptButton->SetEnabled(true);
	updatePointsFromSession();
	updateAddRemoveButtons();
}

//----------------------------------------------------------------------

SwgCuiBuffBuilderBuffer::~SwgCuiBuffBuilderBuffer ()
{

	m_callback->disconnect (*this, &SwgCuiBuffBuilderBuffer::onBuffBuilderCancelReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderCancelReceived *>(0));
	m_callback->disconnect (*this, &SwgCuiBuffBuilderBuffer::onBuffBuilderChangeReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderChangeReceived *>(0));

	m_clearButton = NULL;
	m_cancelButton = NULL;
	m_acceptButton = NULL;
	m_addButton = NULL;
	m_removeButton = NULL;
	m_buffTree              = 0;
	m_buffList = 0;
	m_buffeeList = 0;

	delete m_callback;
	m_callback      = 0;
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::performActivate   ()
{
	CuiManager::requestPointer (true);
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::performDeactivate ()
{
	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

bool SwgCuiBuffBuilderBuffer::OnMessage(UIWidget *context, const UIMessage & msg )
{
	UNREF(context);

	if(msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if(context == m_buffTree)
		{
			addBuffToList();
			return true;
		}
		else if (context == m_buffList)
		{
			removeBuffFromList();
			return true;
		}
	}

	return true;

}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::OnButtonPressed( UIWidget *context )
{
	//tell the server to cancel the session if necessary
	if(context == m_cancelButton)
	{
		closeThroughWorkspace ();
	}
	//send the update packet
	else if(context == m_acceptButton)
	{
		if(m_failedLastVerification || Random::random(1, 5) <= 2) // 40% chance
		{
			CuiStringVariablesData csvd;
			Object const * sourceObj = Game::getPlayer();
			Object const * receptObj = NetworkIdManager::getObjectById(m_recipientId);

			if(!sourceObj || !receptObj)
				return;

			csvd.source = sourceObj->asClientObject();
			csvd.target = receptObj->asClientObject();

			Unicode::String str;
			StringId promptId("ui_buffbuilder", "verify_prompt");
			CuiStringVariablesManager::process (promptId, csvd, str);

			ms_totalLogos = Random::random(1, 6);
			CuiMessageBox * const box = CuiMessageBox::createOkCancelBoxWithInput(str);
			box->generateVerificationImage(ms_totalLogos);

			m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiBuffBuilderBuffer::onVerifyPromptClosed);

		}
		else
		{
			buildAndSendUpdateToServer(true);
			m_committed = true;
			m_acceptButton->SetEnabled(false);
		}
	}
	else if(context == m_clearButton)
	{
		CreatureObject const * const player = Game::getPlayerCreature ();
		DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::OnButtonPressed - in clear- player is null"));
		if(!player) 
			return;
		SharedBuffBuilderManager::Session session;
		bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
					
		if(result)
		{
			session.clearBuffComponents();
			session.accepted = false;
			m_committed = false;
			SharedBuffBuilderManager::updateSession(session);
			updateBuffListFromSession();
			updateBuffeeListFromSession();
			updatePointsFromSession();					
			m_acceptButton->SetEnabled(false);					
			buildAndSendUpdateToServer(false);
		}
		updateAddRemoveButtons();
		updateAcceptButton();
	}
	else if(context == m_addButton)
	{
		addBuffToList();
	}
	else if(context == m_removeButton)
	{
		removeBuffFromList();
	}
} 

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::OnGenericSelectionChanged (UIWidget * context)
{
	if(context == m_buffTree || context == m_buffList)
	{
		updateAddRemoveButtons();
		updateComponentDescription();				
	}

	// hack to handle a bug in the tree control - non selectable elements (folders) can end up being selected if
	//  an element inside of them is selected when the folder is closed
	if(context == m_buffTree)
	{
		long selectedRow = m_buffTree->GetLastSelectedRow();
		if(selectedRow != -1)
		{
			UIDataSourceContainer* selectedContainer = m_buffTree->GetDataSourceContainerAtRow  (selectedRow);
			if(selectedContainer)
			{
				UIString internalName;
				if( selectedContainer->GetProperty(UILowerString("Name"),internalName) )
				{
					bool isSelectable;
					if(selectedContainer->GetPropertyBoolean(UITreeView::DataProperties::Selectable,isSelectable))
					{
						if(!isSelectable)
						{
							m_buffTree->SelectRow(-1);
						}
					}
				}
			}
		}
	}

	if(context == m_buffeeList)
	{
		long selectedRow = m_buffeeList->GetLastSelectedRow();
		if(selectedRow != -1)
		{
			m_buffeeList->SelectRow(-1);
		}
	}
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::setupPage()
{
	CreatureObject const * const player = Game::getPlayerCreature ();
	DEBUG_FATAL(!player, ("No player in SwgCuiBuffBuilderBuffer::setupPage"));
	if(!player) 
		return;

	SharedBuffBuilderManager::Session newSession;
	newSession.bufferId = player->getNetworkId();
	newSession.recipientId = m_recipientId;
	newSession.startingTime = Os::getRealSystemTime();
	SharedBuffBuilderManager::startSession(newSession);

	updateBuffListFromSession();
	updateBuffeeListFromSession();
	updatePointsFromSession();	

	CreatureObject * const recipientCreature = getRecipientCreature();
	if(recipientCreature)
	{
		m_recipientName->SetText(recipientCreature->getLocalizedName());
	}

	if(Game::getSinglePlayer())
	{
		//single player
		setRecipient(player->getNetworkId());
	}
	else
	{
		//make sure the buffer has some target
		if(!getRecipientCreature())
		{
			CuiMessageBox::createInfoBox(SharedStringIds::buffbuilder_no_target.localize());
			closeThroughWorkspace();
			return;
		}
	}

	setAssociatedObjectId(getRecipientId());
	setMaxRangeFromObject(16.0f);
	buildAndSendUpdateToServer(false);				

	updateAcceptButton();
	updateAddRemoveButtons();


}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::setRecipient(NetworkId const & recipientId)
{
	m_recipientId = recipientId;
	setupPage();
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::onBuffBuilderChangeReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload)
{
	if(Game::getPlayer()->getNetworkId() != payload.bufferId)
		return;

	if(payload.accepted == true)
	{
		closeThroughWorkspace();
	}
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::onBuffBuilderCancelReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload)
{
	if(Game::getPlayer()->getNetworkId() != payload.bufferId)
		return;

	SharedBuffBuilderManager::endSession(payload.bufferId);
	//no need to tell server
	m_committed = true;
	closeThroughWorkspace();
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiBuffBuilderBuffer::getRecipientCreature() const
{
	Object * const o = NetworkIdManager::getObjectById(m_recipientId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	return co ? co->asCreatureObject() : NULL;
}

//----------------------------------------------------------------------

NetworkId const & SwgCuiBuffBuilderBuffer::getRecipientId() const
{
	return m_recipientId;
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::buildAndSendUpdateToServer(bool const accepted) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
	
	if(result)
	{
		BuffBuilderChangeMessage * const msg = new BuffBuilderChangeMessage;
		SharedBuffBuilderManager::populateChangeMessage(session, *msg);
		msg->setOrigin(BuffBuilderChangeMessage::O_BUFFER);
		msg->setAccepted(accepted);
		player->getController()->appendMessage (CM_buffBuilderChange, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
	
}

//----------------------------------------------------------------------

bool SwgCuiBuffBuilderBuffer::close()
{
	//tell the server to cancel the session if necessary
	if(!m_committed)
	{
		CreatureObject * const buffer = Game::getPlayerCreature();
		if(buffer)
		{
			SharedBuffBuilderManager::Session session;
			bool const result = SharedBuffBuilderManager::getSession(buffer->getNetworkId(), session);
			if(result)
			{
				SharedBuffBuilderManager::endSession(session.bufferId);
				Object const * const bufferObj = NetworkIdManager::getObjectById(session.bufferId);
				if(bufferObj && bufferObj == buffer)
				{
					BuffBuilderChangeMessage * msg = new BuffBuilderChangeMessage();
					SharedBuffBuilderManager::populateChangeMessage(session, *msg);
					msg->setOrigin(BuffBuilderChangeMessage::O_BUFFER);
					msg->setAccepted(false);
					buffer->getController()->appendMessage (CM_buffBuilderCancel, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
				}
			}
		}
	}

	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::initializeBuffTree()
{
	m_buffTree->ClearData();

	std::vector<std::string> categoryList;
	std::vector<std::string> buffList;

	SharedBuffBuilderManager::getCategoryNames(categoryList);
	std::sort(categoryList.begin(),categoryList.end());
	SharedBuffBuilderManager::getRecordNames(buffList);
	std::sort(categoryList.begin(),categoryList.end());

	int numCategories = categoryList.size();
	int numBuffs = buffList.size();

	UIDataSourceContainer * const mainDsc = NON_NULL(m_buffTree->GetDataSourceContainer());
	mainDsc->Attach(0);
	m_buffTree->SetDataSourceContainer(0);
	mainDsc->Clear();
	m_buffTree->SetPropertyBoolean(UITreeView::PropertyName::ShowIcon, true);
	char tmpString[512];

	for(int categoryIndex = 0; categoryIndex < numCategories; ++categoryIndex)
	{
		Unicode::String categoryInternalNameWide = Unicode::narrowToWide(categoryList[categoryIndex].c_str());
		UIDataSourceContainer * const categoryDsc = new UIDataSourceContainer;
		categoryDsc->SetProperty(UILowerString("Name"), categoryInternalNameWide);
		StringId const categoryDisplayName("buff_builder", categoryList[categoryIndex].c_str());
		categoryDsc->SetProperty(UITreeView::DataProperties::Text, categoryDisplayName.localize());
		categoryDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, false);
		categoryDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
		for(int buffIndex = 0; buffIndex < numBuffs; ++buffIndex)
		{
			std::string buffCategory = SharedBuffBuilderManager::getCategoryNameForRecordName(buffList[buffIndex]);
			if(categoryList[categoryIndex] == buffCategory)
			{
				std::string requiredExpertise = SharedBuffBuilderManager::getRequiredExpertiseNameForRecordName(buffList[buffIndex]);
				
				if(!requiredExpertise.empty())
				{
					requiredExpertise = std::string("expertise_en_") + requiredExpertise + std::string("_1");
				}

				// expertise check 
				if(requiredExpertise.empty() || ClientExpertiseManager::playerHasExpertise(requiredExpertise))
				{
					Unicode::String buffInternalNameWide = Unicode::narrowToWide(buffList[buffIndex].c_str());
					UIDataSourceContainer * const buffDsc = new UIDataSourceContainer;
					buffDsc->SetProperty(UILowerString("Name"),buffInternalNameWide);
					StringId const buffDisplayName("buff_builder",buffList[buffIndex].c_str());
				
					const int buffComponentCost = SharedBuffBuilderManager::getCostForRecordName(buffList[buffIndex]);
					sprintf(tmpString, " - cost %d", buffComponentCost);
				
					buffDsc->SetProperty(UITreeView::DataProperties::Text, buffDisplayName.localize() + Unicode::narrowToWide(tmpString));
					buffDsc->SetPropertyBoolean(UITreeView::DataProperties::Selectable, true);
					buffDsc->SetPropertyBoolean(UITreeView::DataProperties::Expanded, false);
					categoryDsc->AddChild(buffDsc);
				}
			}
		}

		// delete empty categories (if they didn't have required expertise)
		if(categoryDsc->GetChildCount())
		{
			mainDsc->AddChild(categoryDsc);
		}
		else
		{
			delete categoryDsc;
		}
	}

	m_buffTree->SetDataSourceContainer(mainDsc);
	mainDsc->Detach(0);
	m_buffTree->SelectRow(-1);
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::initializeExpertiseModifiers()
{
	ms_additionalPoints				= getExpertiseSkillModValue(ms_creativeExpertiseName,ms_pointIncreaseExpertiseSkillModName);
	ms_attributeBonusFromExpertise	= getExpertiseSkillModValue(ms_inspiredFitnessExpertiseName,ms_attributeBonusExpertiseSkillModName);
	ms_resistanceBonusFromExpertise = getExpertiseSkillModValue(ms_inspiredResilienceExpertiseName,ms_resistanceBonusExpertiseSkillModName);
	ms_traderBonusFromExpertise		= getExpertiseSkillModValue(ms_inspiredIndustryExpertiseName,ms_traderBonusExpertiseSkillModName);
	ms_procBonusFromExpertise		= getExpertiseSkillModValue(ms_inspiredReactionsExpertiseName,ms_procBonusExpertiseSkillModName);
	ms_combatBonusFromExpertise		= getExpertiseSkillModValue(ms_inspiredWarfareExpertiseName,ms_combatBonusExpertiseSkillModName);
}

//----------------------------------------------------------------------

int SwgCuiBuffBuilderBuffer::getExpertiseSkillModValue(const std::string & expertiseName, const std::string & skillModName)
{
	int value = 0;
	ClientExpertiseManager::ExpertiseSkillModStruct skillMods;

	int expertiseRank = ClientExpertiseManager::getExpertiseRankForPlayer(expertiseName,true);
	
	if(expertiseRank)
	{
		ClientExpertiseManager::getExpertiseSkillMods(expertiseName,skillMods);

		for(int i = 0; i < ClientExpertiseManager::MAX_NUM_SKILL_MODS_PER_EXPERTISE; ++i)
		{
			if(skillMods.names[i] == skillModName)
			{
				for(int j = 0; j < expertiseRank; ++j)
				{
					value += (skillMods.values[j + i*ClientExpertiseManager::MAX_NUM_EXPERTISE_RANKS]);
				}
				break;
			}
		}
	}

	return value;
}

//----------------------------------------------------------------------

int SwgCuiBuffBuilderBuffer::getExpertiseModifierForBuffComponent(const std::string & buffComponentName)
{
	int value = 0;

	const std::string& categoryName = SharedBuffBuilderManager::getCategoryNameForRecordName(buffComponentName);

	if(categoryName == "attributes")
	{
		value = ms_attributeBonusFromExpertise;
	}
	else if(categoryName == "resistances")
	{
		value = ms_resistanceBonusFromExpertise;
	}
	else if(categoryName == "trade")
	{
		value = ms_traderBonusFromExpertise;
	}
	else if(categoryName == "combat")
	{
		value = ms_combatBonusFromExpertise;
	}
	else if (buffComponentName == ms_reactiveSecondChanceComponentName)
	{
		value = ms_procBonusFromExpertise;
	}

	return value;
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::clearBuffList()
{
	m_buffList->Clear();
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::updateBuffListFromSession()
{

	CreatureObject const * const player = Game::getPlayerCreature ();
	DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::updateBuffListFromSession() player is null"));
	if(!player) 
		return;	

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
					
	if(result)
	{
		char tmpString[512];
		clearBuffList();
		for(std::map<std::string, std::pair<int,int> >::iterator j = session.buffComponents.begin(); j != session.buffComponents.end(); ++j)
		{
			
			sprintf(tmpString, "%d/%d:", j->second.first, SharedBuffBuilderManager::getMaxTimesAppliedForRecordName(j->first));
			StringId const buffDisplayName("buff_builder",j->first);
			m_buffList->AddRow(
				Unicode::narrowToWide(tmpString) + buffDisplayName.localize(),
				j->first
			);
		}	
	}
	else
	{
		DEBUG_FATAL(true,("SwgCuiBuffBuilderBuffer::updateBuffListFromSession()  - unable to find session"));
	}
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::updateBuffeeListFromSession()
{
	CreatureObject const * const player = Game::getPlayerCreature ();
	DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::updateBuffeeListFromSession() player is null"));
	if(!player) 
		return;	

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
	
	if(result)
	{
		m_buffeeList->Clear();
		char tmp[512];
		for(std::map<std::string, std::pair<int,int> >::iterator buffIter = session.buffComponents.begin(); buffIter != session.buffComponents.end(); ++buffIter)
		{
			snprintf(tmp, 512, "%s_prose", buffIter->first.c_str());
			std::string tmpS(tmp);
			ProsePackage pp;
			pp.stringId = StringId("buff_builder",tmpS);

			const int affectAmount = SharedBuffBuilderManager::getAffectAmountForRecordName(buffIter->first);
			const int buffCount = buffIter->second.first;
			const int expertiseModifier = buffIter->second.second;
			const int adjustedAmount = SharedBuffBuilderManager::computeAdjustedAffectAmount(buffIter->first,affectAmount,expertiseModifier);
			pp.digitInteger = buffCount * adjustedAmount;

			Unicode::String resultStr;
			IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(pp, resultStr));
			m_buffeeList->AddRow(resultStr,buffIter->first);
		}
	}
}

//----------------------------------------------------------------------



void SwgCuiBuffBuilderBuffer::OnTextboxChanged(UIWidget * const context)
{
	if (context == m_coverChargeTextBox)
	{
		SharedBuffBuilderManager::Session session;
		bool const result = SharedBuffBuilderManager::getSession(Game::getPlayer()->getNetworkId(), session);
		if(result)
		{
			int value = m_coverChargeTextBox->GetNumericIntegerValue();
			if(value < 0)
				value = 0;
			session.bufferRequiredCredits = value;
			session.accepted = false;
			m_committed = false;
			m_acceptButton->SetEnabled(true);
			SharedBuffBuilderManager::updateSession(session);
			buildAndSendUpdateToServer(false);
		}
	}
}


void SwgCuiBuffBuilderBuffer::updatePointsFromSession()
{
	CreatureObject const * const player = Game::getPlayerCreature ();
	DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::updatePointsFromSession() player is null"));
	if(!player) 
		return;	

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
	
	int totalCost = 0;

	if(result)
	{
		for(std::map<std::string, std::pair<int,int> >::iterator buffIter = session.buffComponents.begin(); buffIter != session.buffComponents.end(); ++buffIter)
		{
			totalCost += (buffIter->second.first * SharedBuffBuilderManager::getCostForRecordName(buffIter->first));
		}
	}

	char buf[256];
	_itoa(totalCost, buf, 10);
	m_totalCost->SetText(Unicode::narrowToWide(buf));

	
	int totalPoints = ms_basePoints + ms_additionalPoints;
	
	_itoa(totalPoints - totalCost, buf, 10);
	m_pointsLeft->SetText(Unicode::narrowToWide(buf));
}

//----------------------------------------------------------------------

int SwgCuiBuffBuilderBuffer::getPointsLeft()
{
	UIString pointsLeftString;
	m_pointsLeft->GetText(pointsLeftString);
	return atoi(Unicode::wideToNarrow(pointsLeftString).c_str());
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::updateAddRemoveButtons()
{
	// enable/disable add and remove buttons based on points remaining and selected items

	m_addButton->SetEnabled(false);
	m_removeButton->SetEnabled(false);
	
	long selectedTreeRow = m_buffTree->GetLastSelectedRow();
	if(selectedTreeRow != -1)
	{
		UIDataSourceContainer* selectedContainer = m_buffTree->GetDataSourceContainerAtRow(selectedTreeRow);
		if(selectedContainer)
		{
			UIString internalName;
			if(selectedContainer->GetProperty(UILowerString("Name"),internalName))
			{
				const int pointsLeftToSpend = getPointsLeft();
				const int costForComponent = SharedBuffBuilderManager::getCostForRecordName(Unicode::wideToNarrow(internalName));
				if(pointsLeftToSpend >= costForComponent)
				{
					CreatureObject const * const player = Game::getPlayerCreature ();
					DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::updateAddRemoveButtons player is null"));
					if(!player) 
						return;	

					SharedBuffBuilderManager::Session session;
					bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);

					if(result)
					{
						const int maxCountForComponent = SharedBuffBuilderManager::getMaxTimesAppliedForRecordName(Unicode::wideToNarrow(internalName));
						const int currentCount = session.getBuffComponentValue(Unicode::wideToNarrow(internalName));
						if(currentCount < maxCountForComponent)
						{
							m_addButton->SetEnabled(true);
						}
					}
				}
			}
		}
	}

	long selectedListRow = m_buffList->GetLastSelectedRow();
	if(selectedListRow != -1)
	{
		m_removeButton->SetEnabled(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::updateAcceptButton()
{
	//turn activate the accept button if and only if we have at least one
	//buff in the list
	CreatureObject const * const player = Game::getPlayerCreature ();
	if(!player) 
		return;	

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);

	if(result)
	{
		bool haveBuff = !session.buffComponents.empty();
		m_acceptButton->SetEnabled(haveBuff);
	}
	else
	{
		m_acceptButton->SetEnabled(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffer::updateComponentDescription()
{
	long selectedTreeRow = m_buffTree->GetLastSelectedRow();
	if(selectedTreeRow != -1)
	{
		UIDataSourceContainer* selectedContainer = m_buffTree->GetDataSourceContainerAtRow(selectedTreeRow);
		if(selectedContainer)
		{
			UIString internalName;
			if(selectedContainer->GetProperty(UILowerString("Name"),internalName))
			{
				char tmp[512];
				snprintf(tmp, 512, "%s_d_prose", Unicode::wideToNarrow(internalName).c_str());
				std::string tmpS(tmp);
				ProsePackage pp;
				pp.stringId = StringId("buff_builder",tmpS);

				const int affectAmount = SharedBuffBuilderManager::getAffectAmountForRecordName(Unicode::wideToNarrow(internalName));
				const int buffCount = 1;
				const int expertiseModifier = getExpertiseModifierForBuffComponent(Unicode::wideToNarrow(internalName));
				const int adjustedAmount = SharedBuffBuilderManager::computeAdjustedAffectAmount(Unicode::wideToNarrow(internalName),affectAmount,expertiseModifier);
				pp.digitInteger = buffCount * adjustedAmount;

				Unicode::String resultStr;
				IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(pp, resultStr));
				m_componentDescription->SetLocalText(resultStr);
			}
		}
	}
	else
	{
		m_componentDescription->SetText(Unicode::narrowToWide(""));
	}
}

void SwgCuiBuffBuilderBuffer::onVerifyPromptClosed(const CuiMessageBox & box)
{
	// Get the text input and verify the code here.
	if(box.completedAffirmative())
	{
		UIText & inputText = box.getInputText();
		UIString inputStr;
		inputText.GetLocalText(inputStr);
		std::string const narrowInput = Unicode::wideToNarrow(inputStr);

		int answer = atoi(narrowInput.c_str());

		if(answer == ms_totalLogos)
		{
			buildAndSendUpdateToServer(true);
			m_committed = true;
			m_acceptButton->SetEnabled(false);
			m_failedLastVerification = false;
			ms_consecutiveFails = 0;
		}
		else
		{
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Incorrect confirmation code entered. Please try again."));
			m_failedLastVerification = true;
			++ms_consecutiveFails;
		}

	}
	else
	{
		m_failedLastVerification = true;
		++ms_consecutiveFails;
	}

	if(ms_consecutiveFails >= 3)
		closeNextFrame();
}

void SwgCuiBuffBuilderBuffer::addBuffToList()
{
	long selectedRow = m_buffTree->GetLastSelectedRow();
	if(selectedRow != -1)
	{
		UIDataSourceContainer* selectedContainer = m_buffTree->GetDataSourceContainerAtRow  (selectedRow);
		if(selectedContainer)
		{
			UIString internalName;
			if(selectedContainer->GetProperty(UILowerString("Name"),internalName))
			{
				CreatureObject const * const player = Game::getPlayerCreature ();
				DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::OnButtonPressed - in add- player is null"));

				if(!player) 
					return;

				const int pointsLeftToSpend = getPointsLeft();
				const int costForComponent = SharedBuffBuilderManager::getCostForRecordName(Unicode::wideToNarrow(internalName));
				if(pointsLeftToSpend >= costForComponent)
				{
					SharedBuffBuilderManager::Session session;
					bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
					if(result)
					{
						int expertiseModifier = getExpertiseModifierForBuffComponent(Unicode::wideToNarrow(internalName));
						const bool addResult = session.addBuffComponent(Unicode::wideToNarrow(internalName),expertiseModifier);
						if(addResult)
						{
							session.accepted = false;
							m_committed = false;
							SharedBuffBuilderManager::updateSession(session);
							updateBuffListFromSession();
							updateBuffeeListFromSession();
							updatePointsFromSession();
							m_acceptButton->SetEnabled(true);
							buildAndSendUpdateToServer(false);
						}
					}
					else
					{
						DEBUG_FATAL(true,("SwgCuiBuffBuilderBuffer::OnButtonPressed - add  - unable to find session"));
					}
				}
			}
		}
	}
	updateAddRemoveButtons();
	updateAcceptButton();
}

void SwgCuiBuffBuilderBuffer::removeBuffFromList()
{
	long selectedRow = m_buffList->GetLastSelectedRow();
	if(selectedRow != -1)
	{
		UIData* selectedData = m_buffList->GetDataAtRow(selectedRow);
		if(selectedData)
		{
			UIString internalName;
			if(selectedData->GetProperty(UILowerString("Name"),internalName))
			{
				CreatureObject const * const player = Game::getPlayerCreature ();
				DEBUG_FATAL(!player, ("SwgCuiBuffBuilderBuffer::OnButtonPressed - in remove - player is null"));

				if(!player) 
					return;

				SharedBuffBuilderManager::Session session;
				bool const result = SharedBuffBuilderManager::getSession(player->getNetworkId(), session);
				if(result)
				{
					session.accepted = false;
					m_committed = false;	
					session.removeBuffComponent(Unicode::wideToNarrow(internalName));
					SharedBuffBuilderManager::updateSession(session);
					updateBuffListFromSession();
					updateBuffeeListFromSession();
					updatePointsFromSession();
					m_acceptButton->SetEnabled(true);
					buildAndSendUpdateToServer(false);

					// select element back or next element if totally removed
					const long numElements = m_buffList->GetRowCount();
					if(numElements != 0)
					{
						if(selectedRow == numElements)
						{
							m_buffList->SelectRow(selectedRow - 1);
						}
						else
						{
							m_buffList->SelectRow(selectedRow);
						}
					}
				}
				else
				{
					DEBUG_FATAL(true,("SwgCuiBuffBuilderBuffer::OnButtonPressed - remove - unable to find session"));
				}
			}
		}
	}
	updateAddRemoveButtons();
	updateAcceptButton();
}
//----------------------------------------------------------------------
