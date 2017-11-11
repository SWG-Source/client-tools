//======================================================================
//
// CuiManagerManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiManagerManager.h"

#include "clientGame/AuctionManagerClient.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiCSManagerListener.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiCharacterLoadoutManager.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiChatterSpeechManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiDataDrivenPageManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiInputNames.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiInventoryState.h"
#include "clientUserInterface/CuiKnowledgeBaseManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiMinigameManager.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiModifierManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPlayerQuestManager.h"
#include "clientUserInterface/CuiPoiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiRecipeManager.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiSurveyManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedGame/SpatialChatManager.h"

#include "UIPage.h"

//======================================================================


namespace CuiManagerManagerNamespace
{
	bool s_testSkillSystem = false;
	bool s_testSkillSystemVerbose = false;
}

using namespace CuiManagerManagerNamespace;

//----------------------------------------------------------------------


void CuiManagerManager::install (UIPage & rootPage)
{
	InstallTimer const installTimer("CuiManagerManager");

	CuiActionManager::install            ();
	Cui::MenuInfoTypes::install          ();
	CuiMessageQueueManager::install      ();
	CuiMediatorFactory::install          ();
	CuiSpatialChatManager::install       ();
	CuiTextManager::install              (rootPage);
	CuiInputNames::install               ();
	CuiInventoryManager::install         ();
	CuiCombatManager::install            ();
	CuiConversationManager::install      ();
	CuiCharacterLoadoutManager::install  ();
	CuiRadialMenuManager::install        ();
	CuiDataDrivenPageManager::install    (); 
	CuiChatBubbleManager::install        (rootPage);
	CuiMessageBox::install               ();
	CuiIconManager::install              ();
	CuiSkillManager::install             ();
	CuiSocialsManager::install           ();
	CuiCraftManager::install             ();
	CuiResourceManager::install          ();
	CuiChatManager::install              ();
	CuiChatRoomManager::install          ();
	CuiInstantMessageManager::install    ();
	CuiVoiceChatManager::install         ();
	CuiMissionManager::install           ();
	CuiObjectTextManager::install        ();
	CuiPersistentMessageManager::install ();
	CuiLoginManager::install             ();
	CuiSystemMessageManager::install     ();
	CuiDamageManager::install            ();
	CuiAuctionManager::install           ();
	CuiCSManagerListener::install        ();
	CuiConsentManager::install           ();
	CuiSurveyManager::install            ();
	CuiPreferences::install              ();
	CuiSettings::install                 ();
	CuiKnowledgeBaseManager::install     ();
	CuiCachedAvatarManager::install      ();
	CuiPoiManager::install               ();
	CuiModifierManager::install          ();
	CuiChatterSpeechManager::install     ();
	CuiMinigameManager::install          ();
	CuiPlayerQuestManager::install		 ();
	CuiRecipeManager::install			 ();

	DebugFlags::registerFlag (s_testSkillSystem,        "ClientUserInterface", "testSkillSystem");
	DebugFlags::registerFlag (s_testSkillSystemVerbose, "ClientUserInterface", "testSkillSystemVerbose");

	if (s_testSkillSystem)
		CuiSkillManager::testSkillSystem (s_testSkillSystemVerbose);
}

//----------------------------------------------------------------------

void CuiManagerManager::remove  ()
{
	CuiRecipeManager::remove			();
	CuiPlayerQuestManager::remove		();
	CuiMinigameManager::remove          ();
	CuiChatterSpeechManager::remove     ();
	CuiModifierManager::remove          ();
	CuiPoiManager::remove               ();
	CuiCachedAvatarManager::remove      ();
	CuiKnowledgeBaseManager::remove     ();
	CuiSettings::remove                 ();
	CuiSurveyManager::remove            ();
	CuiConsentManager::remove           ();
	CuiCSManagerListener::remove        ();
	CuiAuctionManager::remove           ();
	CuiSystemMessageManager::remove     ();
	CuiLoginManager::remove             ();
	CuiPersistentMessageManager::remove ();
	CuiObjectTextManager::remove        ();
	CuiMissionManager::remove           ();
	CuiVoiceChatManager::remove         ();
	CuiInstantMessageManager::remove    ();
	CuiChatRoomManager::remove          ();
	CuiResourceManager::remove          ();
	CuiCraftManager::remove             ();
	CuiSocialsManager::remove           ();
	CuiSkillManager::remove             ();
	CuiIconManager::remove              ();
	CuiMessageBox::remove               ();
	CuiChatBubbleManager::remove        ();
	CuiDataDrivenPageManager::remove    ();
	CuiRadialMenuManager::remove        ();
	CuiCharacterLoadoutManager::remove  ();
	CuiConversationManager::remove      ();
	CuiCombatManager::remove            ();
	CuiInventoryManager::remove         ();
	CuiInputNames::remove               ();
	CuiTextManager::remove              ();
	CuiSpatialChatManager::remove       ();

	CuiMediatorFactory::remove          ();
	Cui::MenuInfoTypes::remove          ();

	int mediatorCount = CuiMediator::getRemainingMediatorCount ();

	while (mediatorCount)
	{
		CuiMediator::garbageCollect         (true);
		const int curMediatorCount = CuiMediator::getRemainingMediatorCount ();

		if (curMediatorCount == mediatorCount)
		{
			WARNING (true, ("Possibly unable to garbage collect all mediators:"));
			std::string str;
			CuiMediator::debugPrintMediators (str);
			WARNING (true, ("\n%s", str.c_str ()));
			break;
		}

		mediatorCount = curMediatorCount;
	}

	CuiPreferences::remove              ();
	CuiChatManager::remove              ();
	CuiMessageQueueManager::remove      ();
	CuiActionManager::remove            ();
	CuiDamageManager::remove            ();

}

//----------------------------------------------------------------------

void CuiManagerManager::update (float deltaTimeSecs)
{
	CuiVoiceChatManager::update           (deltaTimeSecs);
	CuiObjectTextManager::update          ();
	CuiIconManager::update                (deltaTimeSecs);
	AuctionManagerClient::update          (deltaTimeSecs);
	CuiSettings::update                   (deltaTimeSecs);
	CuiMissionManager::update             (deltaTimeSecs);
	CuiCachedAvatarManager::update        (deltaTimeSecs);
	CuiRadialMenuManager::update          ();
	CuiInventoryManager::update           (deltaTimeSecs);
	CuiPersistentMessageManager::update   (deltaTimeSecs);
	CuiInventoryState::saveInventoryState ();
	CuiModifierManager::update            (deltaTimeSecs);
}

//======================================================================
