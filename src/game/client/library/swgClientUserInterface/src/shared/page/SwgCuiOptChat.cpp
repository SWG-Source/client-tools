//======================================================================
//
// SwgCuiOptChat.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptChat.h"

#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//======================================================================

namespace SwgCuiOptChatNamespace
{
	int getDefaultCombatSpamFilter(const SwgCuiOptBase &, const UIComboBox &)
	{
		return CuiPreferences::getCombatSpamFilterDefault();
	}

	// enable/disable combat spam range filter
	void enableOrDisableCombatSpamRangeFilter(const SwgCuiOptBase & self)
	{
		UIPage * combatSpamRangeFilterPage = 0;
		self.getCodeDataObject(TUIPage, combatSpamRangeFilterPage, "sliderCombatSpamRangeFilterPage");
		if (combatSpamRangeFilterPage)
		{
			int const csf = CuiPreferences::getCombatSpamFilter();
			combatSpamRangeFilterPage->SetEnabled(((csf != static_cast<int>(CombatDataTable::CSFT_Self)) && (csf != static_cast<int>(CombatDataTable::CSFT_None))));
		}
	}

	bool s_isActivated = false;
	time_t s_combatSpamRangeFilterTimeToUpdateServer = 0;
	void setCombatSpamRangeFilter(int combatSpamRangeFilter)
	{
		CuiPreferences::setCombatSpamRangeFilter(combatSpamRangeFilter);

		if (!s_isActivated)
		{
			GenericValueTypeMessage<int> const msg("SetCombatSpamRangeFilter", CuiPreferences::getCombatSpamRangeFilter());
			GameNetwork::send(msg, true);
		}
		else
		{
			s_combatSpamRangeFilterTimeToUpdateServer = ::time(NULL) + 3;
		}
	}
}

using namespace SwgCuiOptChatNamespace;

//======================================================================

SwgCuiOptChat::SwgCuiOptChat (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptChat", page)
{
	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;
	UIComboBox * combo = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderChatBoxFontSizeDefault");
	registerSlider    (*slider, CuiChatManager::setChatWindowFontSizeDefaultIndex, CuiChatManager::getChatWindowFontSizeDefaultIndex, ConfigClientUserInterface::getChatWindowFontSizeDefaultIndex, 0, ConfigClientUserInterface::getChatWindowFontSizesCount ());
	getCodeDataObject (TUISliderbar, slider, "sliderChatBubbleDuration");
	registerSlider    (*slider, CuiChatBubbleManager::setBubblesDurationScale, CuiChatBubbleManager::getBubblesDurationScale, ConfigClientUserInterface::getChatBubbleDurationScale, 0.0f, 1.0f);
	getCodeDataObject (TUISliderbar, slider, "sliderChatBubbleFontSize");
	registerSlider    (*slider, CuiChatBubbleManager::setBubbleFontSize, CuiChatBubbleManager::getBubbleFontSize, ConfigClientUserInterface::getChatBubbleFontSize, 0.10f, 3.0f);
	getCodeDataObject (TUISliderbar, slider, "sliderChatBubbleRangeFactor");
	registerSlider    (*slider, CuiChatBubbleManager::setChatBubbleRangeFactor, CuiChatBubbleManager::getChatBubbleRangeFactor, ConfigClientUserInterface::getChatBubbleRangeFactor, 0.10f, 2.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBoxKeyClick");
	registerCheckbox (*checkbox, CuiChatManager::setChatBoxKeyClick, CuiChatManager::getChatBoxKeyClick, ConfigClientUserInterface::getChatBoxKeyClick);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBrief");
	registerCheckbox (*checkbox, CuiChatManager::setChatBrief, CuiChatManager::isChatBrief, CuiChatManager::isChatBriefDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatDiku");
	registerCheckbox (*checkbox, CuiChatManager::setChatDiku, CuiChatManager::isChatDiku, CuiChatManager::isChatDikuDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatProse");
	registerCheckbox (*checkbox, CuiChatManager::setChatProse, CuiChatManager::isChatProse, CuiChatManager::isChatProseDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleEmotes");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleEmotesEnabled, CuiChatBubbleManager::getBubbleEmotesEnabled, ConfigClientUserInterface::getChatBubbleEmotesEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleProse");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleProse, CuiChatBubbleManager::getBubbleProse, CuiChatBubbleManager::getBubbleProseDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowAll");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowAll, CuiChatBubbleManager::getBubbleShowAll, CuiChatBubbleManager::getBubbleShowAllDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowGroup");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowGroup, CuiChatBubbleManager::getBubbleShowGroup, CuiChatBubbleManager::getBubbleShowGroupDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowGuild");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowGuild, CuiChatBubbleManager::getBubbleShowGuild, CuiChatBubbleManager::getBubbleShowGuildDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowNPC");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowNPC, CuiChatBubbleManager::getBubbleShowNPC, CuiChatBubbleManager::getBubbleShowNPCDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowPlayer");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowPlayer, CuiChatBubbleManager::getBubbleShowPlayer, CuiChatBubbleManager::getBubbleShowPlayerDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleShowSelf");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleShowSelf, CuiChatBubbleManager::getBubbleShowSelf, CuiChatBubbleManager::getBubbleShowSelfDefault);
	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubbleSocials");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubbleSocialsEnabled, CuiChatBubbleManager::getBubbleSocialsEnabled, CuiChatBubbleManager::getBubbleSocialsEnabledDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowAFKSpeech");
	registerCheckbox (*checkbox, CuiPreferences::setShowAFKSpeech, CuiPreferences::getShowAFKSpeech, SwgCuiOptBase::getTrue);

#if 0	
	//disabled temporarily until we have a solution for npc conversation where the very last message is sent spatially
	getCodeDataObject (TUICheckbox, checkbox, "checkShowNPCSpeech");
	registerCheckbox (*checkbox, CuiPreferences::setShowNPCSpeech, CuiPreferences::getShowNPCSpeech, SwgCuiOptBase::getTrue);
#endif

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBubblesEnable");
	registerCheckbox (*checkbox, CuiChatBubbleManager::setBubblesEnabled, CuiChatBubbleManager::getBubblesEnabled, ConfigClientUserInterface::getChatBubblesEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatTimestamp");
	registerCheckbox (*checkbox, CuiChatManager::setChatBoxTimestamp, CuiChatManager::getChatBoxTimestamp, ConfigClientUserInterface::getChatTimestampEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkProfanity");
	registerCheckbox (*checkbox, Game::setProfanityFiltered, Game::isProfanityFiltered, ConfigClientGame::isProfanityFiltered);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatAutoEmote");
	registerCheckbox (*checkbox, CuiPreferences::setChatAutoEmote, CuiPreferences::getChatAutoEmote, SwgCuiOptBase::getTrue);

	// combat spam options
	getCodeDataObject (TUIComboBox, combo, "comboCombatSpamFilter");
	registerComboBox (*combo, onCombatSpamFilterSet, onCombatSpamFilterGet, getDefaultCombatSpamFilter);
	getCodeDataObject (TUISliderbar, slider, "sliderCombatSpamRangeFilter");
	registerSlider    (*slider, setCombatSpamRangeFilter, CuiPreferences::getCombatSpamRangeFilter, CuiPreferences::getCombatSpamRangeFilterDefault, 1, 128);
	getCodeDataObject (TUICheckbox, checkbox, "checkCombatSpamBrief");
	registerCheckbox (*checkbox, CuiPreferences::setCombatSpamBrief, CuiPreferences::getCombatSpamBrief, SwgCuiOptBase::getTrue);
	getCodeDataObject (TUICheckbox, checkbox, "checkCombatSpamVerbose");
	registerCheckbox (*checkbox, CuiPreferences::setCombatSpamVerbose, CuiPreferences::getCombatSpamVerbose, SwgCuiOptBase::getFalse);
	getCodeDataObject (TUICheckbox, checkbox, "spamShowWeapon");
	registerCheckbox (*checkbox, CuiPreferences::setSpamShowWeapon, CuiPreferences::getSpamShowWeapon, SwgCuiOptBase::getTrue);
	getCodeDataObject (TUICheckbox, checkbox, "spamShowDamageDetail");
	registerCheckbox (*checkbox, CuiPreferences::setSpamShowDamageDetail, CuiPreferences::getSpamShowDamageDetail, SwgCuiOptBase::getTrue);
	getCodeDataObject (TUICheckbox, checkbox, "spamShowArmorAbsorption");
	registerCheckbox (*checkbox, CuiPreferences::setSpamShowArmorAbsorption, CuiPreferences::getSpamShowArmorAbsorption, SwgCuiOptBase::getTrue);
}

//----------------------------------------------------------------------

int SwgCuiOptChat::onCombatSpamFilterGet(const SwgCuiOptBase & self, const UIComboBox &)
{
	enableOrDisableCombatSpamRangeFilter(self);
	return CuiPreferences::getCombatSpamFilter();
}

//----------------------------------------------------------------------

void SwgCuiOptChat::onCombatSpamFilterSet(const SwgCuiOptBase & self, const UIComboBox &, int index)
{
	GenericValueTypeMessage<int> const msg("SetCombatSpamFilter", index);
	GameNetwork::send(msg, true);

	CuiPreferences::setCombatSpamFilter(index);
	enableOrDisableCombatSpamRangeFilter(self);
}

//----------------------------------------------------------------------

void SwgCuiOptChat::update(float deltaTimeSecs)
{
	if ((s_combatSpamRangeFilterTimeToUpdateServer > 0) && (s_combatSpamRangeFilterTimeToUpdateServer <= ::time(NULL)))
	{
		s_combatSpamRangeFilterTimeToUpdateServer = 0;

		GenericValueTypeMessage<int> const msg("SetCombatSpamRangeFilter", CuiPreferences::getCombatSpamRangeFilter());
		GameNetwork::send(msg, true);
	}

	SwgCuiOptBase::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiOptChat::performActivate()
{
	s_combatSpamRangeFilterTimeToUpdateServer = 0;

	SwgCuiOptBase::performActivate();
	setIsUpdating(true);
	s_isActivated = true;
}

//----------------------------------------------------------------------

void SwgCuiOptChat::performDeactivate()
{
	if (s_combatSpamRangeFilterTimeToUpdateServer > 0)
	{
		s_combatSpamRangeFilterTimeToUpdateServer = 0;

		GenericValueTypeMessage<int> const msg("SetCombatSpamRangeFilter", CuiPreferences::getCombatSpamRangeFilter());
		GameNetwork::send(msg, true);
	}

	SwgCuiOptBase::performDeactivate();
	setIsUpdating(false);
	s_isActivated = false;
}

//======================================================================
