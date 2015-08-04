//======================================================================
//
// SwgCuiOptCombat.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptCombat.h"

#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiCombatManager.h"

//======================================================================

SwgCuiOptCombat::SwgCuiOptCombat (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptCombat", page)
{
	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderDamageDoneToMe");
	registerSlider (*slider, CuiCombatManager::setDamageDoneToMeSizeModifier, CuiCombatManager::getDamageDoneToMeSizeModifier, SwgCuiOptBase::getOne, 0.25f, 2.0f);
	
	getCodeDataObject (TUISliderbar, slider, "sliderDamageDoneToOthers");
	registerSlider (*slider, CuiCombatManager::setDamageDoneToOthersSizeModifier, CuiCombatManager::getDamageDoneToOthersSizeModifier, SwgCuiOptBase::getOne, 0.25f, 2.0f);
	
	getCodeDataObject (TUISliderbar, slider, "sliderNonDamageDoneToMe");
	registerSlider (*slider, CuiCombatManager::setNonDamageDoneToMeSizeModifier, CuiCombatManager::getNonDamageDoneToMeSizeModifier, SwgCuiOptBase::getOne, 0.25f, 2.0f);
	
	getCodeDataObject (TUISliderbar, slider, "sliderNonDamageDoneToOthers");
	registerSlider (*slider, CuiCombatManager::setNonDamageDoneToOthersSizeModifier, CuiCombatManager::getNonDamageDoneToOthersSizeModifier, SwgCuiOptBase::getOne, 0.25f, 2.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageNumOthers");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageOverHeadOthers, CuiCombatManager::getShowDamageOverHeadOthers, ConfigClientUserInterface::getShowDamageOverHeadOthers);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageNumSelf");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageOverHeadSelf, CuiCombatManager::getShowDamageOverHeadSelf, ConfigClientUserInterface::getShowDamageOverHeadSelf);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageSnare");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageSnare, CuiCombatManager::getShowDamageSnare, ConfigClientUserInterface::getShowDamageSnare);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageGlancingBlow");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageGlancingBlow, CuiCombatManager::getShowDamageGlancingBlow, SwgCuiOptBase::getTrue);
	
	getCodeDataObject (TUICheckbox, checkbox, "checkDamageCriticalHit");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageCriticalHit, CuiCombatManager::getShowDamageCriticalHit, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageLucky");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageLucky, CuiCombatManager::getShowDamageLucky, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageDot");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageDot, CuiCombatManager::getShowDamageDot, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageBleed");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageBleed, CuiCombatManager::getShowDamageBleed, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageHeal");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageHeal, CuiCombatManager::getShowDamageHeal, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDamageFreeshot");
	registerCheckbox (*checkbox, CuiCombatManager::setShowDamageFreeshot, CuiCombatManager::getShowDamageFreeshot, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkEnteringCombat");
	registerCheckbox (*checkbox, CuiCombatManager::setShowEnteringCombat, CuiCombatManager::getShowEnteringCombat, ConfigClientUserInterface::getShowEnteringCombatFlyText);
}

//======================================================================
