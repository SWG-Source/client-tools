//======================================================================
//
// SwgCuiOptMisc.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptMisc.h"

#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UISliderbar.h"
#include "UIData.h"
#include "UIPage.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//======================================================================

namespace SwgCuiOptMiscNamespace
{
	bool s_isActivated = false;
	time_t s_furnitureRotationDegreeTimeToUpdateServer = 0;

	void setFurnitureRotationDegree(int degree)
	{
		CuiPreferences::setFurnitureRotationDegree(degree);

		if (!s_isActivated)
		{
			GenericValueTypeMessage<int> const msg("SetFurnitureRotationDegree", CuiPreferences::getFurnitureRotationDegree());
			GameNetwork::send(msg, true);
		}
		else
		{
			s_furnitureRotationDegreeTimeToUpdateServer = ::time(NULL) + 3;
		}
	}

	int onComboCurrencyFormatGet(const SwgCuiOptBase &, const UIComboBox &)
	{
		return CuiPreferences::getCurrencyFormat();
	}

	//----------------------------------------------------------------------

	void onComboCurrencyFormatSet(const SwgCuiOptBase &, const UIComboBox &, int index)
	{
		CuiPreferences::setCurrencyFormat(static_cast<CuiPreferences::CurrencyFormat>(index));
	}

	//----------------------------------------------------------------------

	int  getDefaultCurrencyFormat(const SwgCuiOptBase &, const UIComboBox &) 
	{
		return CuiPreferences::getCurrencyFormat();
	}
}

using namespace SwgCuiOptMiscNamespace;

//======================================================================

SwgCuiOptMisc::SwgCuiOptMisc (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptMisc", page)
{

	UICheckbox * checkbox = 0;
	UISliderbar * slider  = 0;
	UIComboBox * combobox = 0;

	getCodeDataObject (TUICheckbox, checkbox, "checkAutoInviteReject");
	registerCheckbox (*checkbox, CuiPreferences::setAutoInviteReject, CuiPreferences::getAutoInviteReject, ConfigClientGame::getAutoInviteReject);

	getCodeDataObject (TUICheckbox, checkbox, "checkNetStatus");
	registerCheckbox (*checkbox, CuiPreferences::setNetStatusEnabled, CuiPreferences::getNetStatusEnabled, ConfigClientUserInterface::getNetStatusEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkConfirmObjDelete");
	registerCheckbox (*checkbox, CuiPreferences::setConfirmObjDelete, CuiPreferences::getConfirmObjDelete, ConfigClientUserInterface::getConfirmObjDelete);

	getCodeDataObject (TUICheckbox, checkbox, "checkConfirmCrafting");
	registerCheckbox (*checkbox, CuiPreferences::setConfirmCrafting, CuiPreferences::getConfirmCrafting, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUISliderbar, slider, "sliderObjectNameFontSize");
	registerSlider (*slider, CuiPreferences::setObjectNameFontSizeFactor, CuiPreferences::getObjectNameFontSizeFactor, ConfigClientUserInterface::getObjectNameFontSizeFactor, 0.25f, 4.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderObjectNameRange");
	registerSlider (*slider, CuiPreferences::setObjectNameRange, CuiPreferences::getObjectNameRange, ConfigClientUserInterface::getObjectNameRange, 8.0f, 128.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowAll");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNames, CuiPreferences::getDrawObjectNames, ConfigClientUserInterface::getDrawObjectNames);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowPlayers");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesPlayers, CuiPreferences::getDrawObjectNamesPlayers, ConfigClientUserInterface::getDrawObjectNamesPlayers);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowGroup");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesGroup, CuiPreferences::getDrawObjectNamesGroup, ConfigClientUserInterface::getDrawObjectNamesGroup);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowGuild");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesGuild, CuiPreferences::getDrawObjectNamesGuild, ConfigClientUserInterface::getDrawObjectNamesGuild);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowNpcs");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesNpcs, CuiPreferences::getDrawObjectNamesNpcs, ConfigClientUserInterface::getDrawObjectNamesNpcs);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowSigns");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesSigns, CuiPreferences::getDrawObjectNamesSigns, ConfigClientUserInterface::getDrawObjectNamesSigns);
	
	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowMyName");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesMyName, CuiPreferences::getDrawObjectNamesMyName, ConfigClientUserInterface::getDrawObjectNamesMyName);

	getCodeDataObject (TUICheckbox, checkbox, "checkObjectNameShowBeasts");
	registerCheckbox (*checkbox, CuiPreferences::setDrawObjectNamesBeasts, CuiPreferences::getDrawObjectNamesBeasts, ConfigClientUserInterface::getDrawObjectNamesBeasts);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowSystemMessages");
	registerCheckbox (*checkbox, CuiPreferences::setShowSystemMessages, CuiPreferences::getShowSystemMessages, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUISliderbar, slider, "sliderSystemMessageTimeout");
	registerSlider (*slider, CuiPreferences::setSystemMessageDuration, CuiPreferences::getSystemMessageDuration, SwgCuiOptBase::getOne, 0.10f, 2.0f);

#if 0
	getCodeDataObject (TUICheckbox, checkbox, "checkShowGroupWaypoints");
	registerCheckbox (*checkbox, CuiPreferences::setShowGroupWaypoints, CuiPreferences::getShowGroupWaypoints, SwgCuiOptBase::getTrue);
#endif

	getCodeDataObject (TUICheckbox, checkbox, "checkScreenShake");
	registerCheckbox (*checkbox, CuiPreferences::setScreenShake, CuiPreferences::getScreenShake, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowCockpit");
	registerCheckbox (*checkbox, CockpitCamera::setShowCockpit, CockpitCamera::getShowCockpit, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkCollectionShowServerFirst");
	registerCheckbox (*checkbox, CuiPreferences::setCollectionShowServerFirst, CuiPreferences::getCollectionShowServerFirst, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUISliderbar, slider, "sliderFurnitureRotationDegree");
	registerSlider    (*slider, setFurnitureRotationDegree, CuiPreferences::getFurnitureRotationDegree, CuiPreferences::getFurnitureRotationDegreeDefault, 1, 180);

	getCodeDataObject(TUIComboBox, combobox, "currencyFormat");
	registerComboBox  (*combobox, onComboCurrencyFormatSet, onComboCurrencyFormatGet, getDefaultCurrencyFormat);

	getCodeDataObject(TUICheckbox, checkbox, "checkAutoLootCorpses");
	registerCheckbox (*checkbox, CuiPreferences::setAutoLootCorpses, CuiPreferences::getAutoLootCorpses, SwgCuiOptBase::getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "checkShowCorpseLootIcon");
	registerCheckbox (*checkbox, CuiPreferences::setShowCorpseLootIcon, CuiPreferences::getShowCorpseLootIcon, SwgCuiOptBase::getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkShowBackpack");
	registerCheckbox (*checkbox, CuiPreferences::setShowBackpack, CuiPreferences::getShowBackpack, SwgCuiOptBase::getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkShowHelmet");
	registerCheckbox (*checkbox, CuiPreferences::setShowHelmet, CuiPreferences::getShowHelmet, SwgCuiOptBase::getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkHideAppearanceItems");
	registerCheckbox(*checkbox, CuiPreferences::setDefaultExamineHideAppearance, CuiPreferences::getDefaultExamineHideAppearance, SwgCuiOptBase::getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "checkDoubleClickAppearance");
	registerCheckbox(*checkbox, CuiPreferences::setDoubleClickUnequipAppearance, CuiPreferences::getDoubleClickUnequipAppearance, SwgCuiOptBase::getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkShowAppearanceInventory");
	registerCheckbox(*checkbox, CuiPreferences::setShowAppearanceInventory, CuiPreferences::getShowAppearanceInventory, SwgCuiOptBase::getTrue);
}

//----------------------------------------------------------------------

void SwgCuiOptMisc::update(float deltaTimeSecs)
{
	if ((s_furnitureRotationDegreeTimeToUpdateServer > 0) && (s_furnitureRotationDegreeTimeToUpdateServer <= ::time(NULL)))
	{
		s_furnitureRotationDegreeTimeToUpdateServer = 0;

		GenericValueTypeMessage<int> const msg("SetFurnitureRotationDegree", CuiPreferences::getFurnitureRotationDegree());
		GameNetwork::send(msg, true);
	}

	SwgCuiOptBase::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiOptMisc::performActivate()
{
	s_furnitureRotationDegreeTimeToUpdateServer = 0;

	SwgCuiOptBase::performActivate();
	setIsUpdating(true);
	s_isActivated = true;
}

//----------------------------------------------------------------------

void SwgCuiOptMisc::performDeactivate()
{
	if (s_furnitureRotationDegreeTimeToUpdateServer > 0)
	{
		s_furnitureRotationDegreeTimeToUpdateServer = 0;

		GenericValueTypeMessage<int> const msg("SetFurnitureRotationDegree", CuiPreferences::getFurnitureRotationDegree());
		GameNetwork::send(msg, true);
	}

	SwgCuiOptBase::performDeactivate();
	setIsUpdating(false);
	s_isActivated = false;
}

//======================================================================
