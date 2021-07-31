//======================================================================
//
// SwgCuiOptUi.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptUi.h"

#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIComboBox.h"
#include "clientDirectInput/DirectInput.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include <vector>


//======================================================================

namespace SwgCuiOptUiNamespace
{
	 //lint -esym(752,SwgCuiOptUiNamespace::getPaletteName)  //not ref (wrong)
	 //lint -esym(752,SwgCuiOptUiNamespace::setPaletteName)  //not ref (wrong)
	 //lint -esym(752,SwgCuiOptUiNamespace::getPaletteNames)  //not ref (wrong)

	static const std::string & getPaletteName (); //lint !e1929 //returning ref
	static bool                setPaletteName (const std::string & paletteName);

	typedef stdvector<std::string>::fwd StringVector;
	static const StringVector & getPaletteNames (); //lint !e1929 //returning ref
	
	//----------------------------------------------------------------------

	int getIndexForPalette (const std::string & thePaletteName)
	{
		const CuiPreferences::StringVector & sv = CuiPreferences::getPaletteNames (true);
		int index = 0;
		for (CuiPreferences::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it, ++index)
		{
			const std::string & paletteName = *it;
			if (!_stricmp (paletteName.c_str (), thePaletteName.c_str ()))
				return index;
		}
		
		WARNING (true, ("SwgCuiOptUi getIndexForPalette no such palette [%s]", thePaletteName.c_str ()));
		return 0;
	}

	//----------------------------------------------------------------------

	int  getDefaultPalette (const SwgCuiOptBase & , const UIComboBox &) 
	{ 
		const std::string & pal = ConfigClientUserInterface::getPaletteName ();
		return getIndexForPalette (pal);
	}

	//----------------------------------------------------------------------

	bool getAllowOverheadMapRotationDefault ()
	{
		return false;
	}

	//----------------------------------------------------------------------

	void setupPaletteCombo (UIComboBox & combo)
	{
		combo.Clear();

		const CuiPreferences::StringVector & sv = CuiPreferences::getPaletteNames (true);

		for (CuiPreferences::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const std::string & paletteName = *it;
			combo.AddItem (Unicode::narrowToWide (paletteName), paletteName);
		}
	}

	//----------------------------------------------------------------------

	int onComboSecondaryTargetGet(const SwgCuiOptBase &, const UIComboBox &)
	{
		return CuiPreferences::getSecondaryTargetMode();
	}

	//----------------------------------------------------------------------

	void onComboSecondaryTargetSet(const SwgCuiOptBase &, const UIComboBox &, int value)
	{
		CuiPreferences::setSecondaryTargetMode(value);
	}

	//----------------------------------------------------------------------

	int  getDefaultSecondaryTargetMode(const SwgCuiOptBase &, const UIComboBox &) 
	{
		return CuiPreferences::getSecondaryTargetMode();
	}
}

using namespace SwgCuiOptUiNamespace;

class SwgCuiOptUi::CallbackReceiverWaypointMonitor : public CallbackReceiver 
{
public:
	CallbackReceiverWaypointMonitor (SwgCuiOptUi & _mediator) :
	  CallbackReceiver (),
	  m_mediator (&_mediator)
	  {
	  }

	void performCallback ()
	{
		if(m_mediator->isActive())
		{
			m_mediator->deactivate ();
			m_mediator->activate   ();
		}
	}

	SwgCuiOptUi * m_mediator;
};

class SwgCuiOptUi::CallbackReceiverExpMonitor : public CallbackReceiver 
{
public:
	CallbackReceiverExpMonitor  (SwgCuiOptUi & _mediator) : 
	  CallbackReceiver (),
	  m_mediator (&_mediator) 
	  {
	  }
	
	void performCallback () 
	{
		if(m_mediator->isActive())
		{
			m_mediator->deactivate ();
			m_mediator->activate   ();
		}
	}
		
	SwgCuiOptUi * m_mediator;
};

class SwgCuiOptUi::CallbackReceiverLocationDisplay : public CallbackReceiver 
{
public:
	CallbackReceiverLocationDisplay  (SwgCuiOptUi & _mediator) : 
	  CallbackReceiver (),
		  m_mediator (&_mediator) 
	  {
	  }

	  void performCallback () 
	  {
		  if(m_mediator->isActive())
		  {
			  m_mediator->deactivate ();
			  m_mediator->activate   ();
		  }
	  }

	  SwgCuiOptUi * m_mediator;
};

//----------------------------------------------------------------------

SwgCuiOptUi::SwgCuiOptUi (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptUi", page),
m_combo       (0),
m_callbackReceiverWaypointMonitor (0),
m_callbackReceiverExpMonitor (0)
{
	m_callbackReceiverWaypointMonitor = new CallbackReceiverWaypointMonitor (*this);
	CuiPreferences::getUseWaypointMonitorCallback ().attachReceiver    (*m_callbackReceiverWaypointMonitor);

	m_callbackReceiverExpMonitor = new CallbackReceiverExpMonitor (*this);
	CuiPreferences::getUseExpMonitorCallback ().attachReceiver (*m_callbackReceiverExpMonitor);
	
	m_callbackReceiverLocationDisplay = new CallbackReceiverLocationDisplay (*this);
	CuiPreferences::getLocationDisplayEnabledCallback ().attachReceiver (*m_callbackReceiverLocationDisplay);

	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;
	UIComboBox * combo = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderHudOpacity");
	registerSlider (*slider, CuiPreferences::setHudOpacity, CuiPreferences::getHudOpacity, ConfigClientUserInterface::getHudOpacity, 0.1f, 1.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderOverheadMapOpacity");
	registerSlider (*slider, CuiPreferences::setOverheadMapOpacity, CuiPreferences::getOverheadMapOpacity, SwgCuiOptBase::getOne, 0.1f, 1.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkOverheadMapShowWaypoints");
	registerCheckbox(*checkbox, CuiPreferences::setOverheadMapShowWaypoints, CuiPreferences::getOverheadMapShowWaypoints, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkOverheadMapShowCreatures");
	registerCheckbox(*checkbox, CuiPreferences::setOverheadMapShowCreatures, CuiPreferences::getOverheadMapShowCreatures, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkOverheadMapShowPlayer");
	registerCheckbox(*checkbox, CuiPreferences::setOverheadMapShowPlayer, CuiPreferences::getOverheadMapShowPlayer, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkOverheadMapShowBuildings");
	registerCheckbox(*checkbox, CuiPreferences::setOverheadMapShowBuildings, CuiPreferences::getOverheadMapShowBuildings, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkOverheadMapShowLabels");
	registerCheckbox(*checkbox, CuiPreferences::setOverheadMapShowLabels, CuiPreferences::getOverheadMapShowLabels, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDoubleToolbar");
	registerCheckbox (*checkbox, CuiPreferences::setUseDoubleToolbar, CuiPreferences::getUseDoubleToolbar, SwgCuiOptBase::getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "checkShowToolbarCommandCooldownTimer");
	registerCheckbox(*checkbox, CuiPreferences::setShowToolbarCooldownTimer, CuiPreferences::getShowToolbarCooldownTimer, SwgCuiOptBase::getFalse);
	
	getCodeDataObject (TUICheckbox, checkbox, "checkLocationDisplay");
	registerCheckbox (*checkbox, CuiPreferences::setLocationDisplayEnabled, CuiPreferences::getLocationDisplayEnabled, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDpsMeter");
	registerCheckbox (*checkbox, CuiPreferences::setDpsMeterEnabled, CuiPreferences::getDpsMeterEnabled, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkRadarTerrain");
	registerCheckbox (*checkbox, CuiPreferences::setGroundRadarTerrainEnabled, CuiPreferences::getGroundRadarTerrainEnabled, ConfigClientUserInterface::getGroundRadarTerrainEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkRadarBlinkCombat");
	registerCheckbox (*checkbox, CuiPreferences::setGroundRadarBlinkCombatEnabled, CuiPreferences::getGroundRadarBlinkCombatEnabled, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowLookAtTargetStatusWindow");
	registerCheckbox (*checkbox, CuiPreferences::setShowLookAtTargetStatusWindowEnabled, CuiPreferences::getShowLookAtTargetStatusWindowEnabled, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowStatusOverIntendedTarget");
	registerCheckbox (*checkbox, CuiPreferences::setShowStatusOverIntendedTarget, CuiPreferences::getShowStatusOverIntendedTarget, ConfigClientUserInterface::getShowStatusOverIntendedTarget);

	getCodeDataObject (TUICheckbox, checkbox, "checkWinKeyWindowed");
	registerCheckbox (*checkbox, DirectInput::setWindowedWindowsKeyEnabled, DirectInput::getWindowedWindowsKeyEnabled, getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkWinKeyFullscreen");
	registerCheckbox (*checkbox, DirectInput::setFullscreenWindowsKeyEnabled, DirectInput::getFullscreenWindowsKeyEnabled, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowNotifications");
	registerCheckbox (*checkbox, CuiPreferences::setShowNotifications, CuiPreferences::getShowNotifications, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDragOntoContainers");
	registerCheckbox (*checkbox, CuiPreferences::setDragOntoContainers, CuiPreferences::getDragOntoContainers, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowGameObjectArrowsOnRadar");
	registerCheckbox (*checkbox, CuiPreferences::setShowGameObjectArrowsOnRadar, CuiPreferences::getShowGameObjectArrowsOnRadar, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowRadarNPCs");
	registerCheckbox (*checkbox, CuiPreferences::setShowRadarNPCs, CuiPreferences::getShowRadarNPCs, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkAlwaysShowRangeInGroundRadar");
	registerCheckbox (*checkbox, CuiPreferences::setAlwaysShowRangeInGroundRadar, CuiPreferences::getAlwaysShowRangeInGroundRadar, SwgCuiOptBase::getFalse);

	//----------------------------------------------------------------------

	getCodeDataObject (TUIComboBox, combo, "comboSecondaryTarget");
	registerComboBox (*combo, onComboSecondaryTargetSet, onComboSecondaryTargetGet, getDefaultSecondaryTargetMode);

	getCodeDataObject (TUIComboBox, m_combo, "comboPalette");
	{
		setupPaletteCombo (*m_combo);
	}
	registerComboBox (*m_combo, SwgCuiOptUi::onComboPaletteSet, SwgCuiOptUi::onComboPaletteGet, getDefaultPalette);

	getCodeDataObject (TUICheckbox, checkbox, "checkIconNames");
	registerCheckbox (*checkbox, CuiPreferences::setShowIconNames, CuiPreferences::getShowIconNames, SwgCuiOptBase::getTrue);
	
	getCodeDataObject (TUISliderbar, slider, "sliderFlyTextSize");
	registerSlider (*slider, CuiPreferences::setFlyTextSize, CuiPreferences::getFlyTextSize, SwgCuiOptBase::getOne, 0.33f, 3.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderObjectIconSize");
	registerSlider (*slider, CuiPreferences::setObjectIconSize, CuiPreferences::getObjectIconSize, SwgCuiOptBase::getOne, CuiPreferences::getObjectIconMinSize (), CuiPreferences::getObjectIconMaxSize ());
	
	getCodeDataObject (TUISliderbar, slider, "sliderCommandButtonOpacity");
	registerSlider (*slider, CuiPreferences::setCommandButtonOpacity, CuiPreferences::getCommandButtonOpacity, SwgCuiOptBase::getOne, 0.0f, 1.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkAllowOverheadMapRotation");
	registerCheckbox (*checkbox, CuiPreferences::setRotateMap, CuiPreferences::getRotateMap, getAllowOverheadMapRotationDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkRotateInventoryObjects");
	registerCheckbox (*checkbox, CuiPreferences::setRotateInventoryObjects, CuiPreferences::getRotateInventoryObjects, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowInterestingAppearance");
	registerCheckbox (*checkbox, CuiPreferences::setShowInterestingAppearance, CuiPreferences::getShowInterestingAppearance, CuiPreferences::getShowInterestingAppearance);

	getCodeDataObject(TUICheckbox, checkbox, "entangle_resistance");
	registerCheckbox(*checkbox, CuiIconManager::setEntangleResistance, CuiIconManager::getEntangleResistance, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_cold_resist");
	registerCheckbox(*checkbox, CuiIconManager::setColdResist, CuiIconManager::getColdResist, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_conductivity");
	registerCheckbox(*checkbox, CuiIconManager::setConductivity, CuiIconManager::getConductivity, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_decay_resist");
	registerCheckbox(*checkbox, CuiIconManager::setDecayResist, CuiIconManager::getDecayResist, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_flavor");
	registerCheckbox(*checkbox, CuiIconManager::setFlavor, CuiIconManager::getFlavor, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_heat_resist");
	registerCheckbox(*checkbox, CuiIconManager::setHeatResist, CuiIconManager::getHeatResist, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_malleability");
	registerCheckbox(*checkbox, CuiIconManager::setMalleability, CuiIconManager::getMalleability, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_potential_energy");
	registerCheckbox(*checkbox, CuiIconManager::setPotentialEnergy, CuiIconManager::getPotentialEnergy, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_quality");
	registerCheckbox(*checkbox, CuiIconManager::setOverallQuality, CuiIconManager::getOverallQuality, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_shock_resistance");
	registerCheckbox(*checkbox, CuiIconManager::setShockResistance, CuiIconManager::getShockResistance, getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "res_toughness");
	registerCheckbox(*checkbox, CuiIconManager::setToughness, CuiIconManager::getToughness, getFalse);

	getCodeDataObject (TUISliderbar, slider, "sliderSpaceCameraElasticity");
	registerSlider (*slider, CuiPreferences::setSpaceCameraElasticity, CuiPreferences::getSpaceCameraElasticity, CuiPreferences::getSpaceCameraElasticityDefault, CuiPreferences::getSpaceCameraElasticityMinSize (), CuiPreferences::getSpaceCameraElasticityMaxSize ());

	getCodeDataObject (TUISliderbar, slider, "sliderVariableTargetingReticlePercentage");
	registerSlider(*slider,
		CuiPreferences::setVariableTargetingReticlePercentage,
		CuiPreferences::getVariableTargetingReticlePercentage,
		CuiPreferences::getVariableTargetingReticlePercentage,
		CuiPreferences::getVariableTargetingReticlePercentageMinimumSize(),
		CuiPreferences::getVariableTargetingReticlePercentageMaximumSize());

	getCodeDataObject(TUICheckbox, checkbox, "checkRenderVariableTargetingReticle");
	registerCheckbox(*checkbox,
		CuiPreferences::setRenderVariableTargetingReticle,
		CuiPreferences::getRenderVariableTargetingReticle,
		getFalse);

	getCodeDataObject(TUICheckbox, checkbox, "checkAutoSortInventoryContents");
	registerCheckbox(*checkbox,
		CuiPreferences::setAutoSortInventoryContents,
		CuiPreferences::getAutoSortInventoryContents,
		getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkAutoSortDataPadContents");
	registerCheckbox(*checkbox,
		CuiPreferences::setAutoSortDataPadContents,
		CuiPreferences::getAutoSortDataPadContents,
		getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkExpMonitor");
	registerCheckbox (*checkbox, CuiPreferences::setUseExpMonitor, CuiPreferences::getUseExpMonitor, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkWaypointMonitor");
	registerCheckbox (*checkbox, CuiPreferences::setUseWaypointMonitor, CuiPreferences::getUseWaypointMonitor, SwgCuiOptBase::getFalse);
	
	getCodeDataObject (TUICheckbox, checkbox, "checkWaypointOnscreen");
	registerCheckbox (*checkbox, CuiPreferences::setShowWaypointArrowsOnscreen, CuiPreferences::getShowWaypointArrowsOnscreen, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkChatBarFadesOut");
	registerCheckbox (*checkbox, CuiPreferences::setChatBarFadesOut, CuiPreferences::getChatBarFadesOut, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowTargetArrow");
	registerCheckbox (*checkbox, CuiPreferences::setTargetArrow, CuiPreferences::getTargetArrow, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowDamagerArrow");
	registerCheckbox (*checkbox, CuiPreferences::setDamagerArrow, CuiPreferences::getDamagerArrow, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowVisibleEnemyDamagerArrow");
	registerCheckbox (*checkbox, CuiPreferences::setVisibleEnemyDamagerArrow, CuiPreferences::getVisibleEnemyDamagerArrow, SwgCuiOptBase::getTrue);

	getCodeDataObject(TUICheckbox, checkbox, "checkNewVendorExamine");
	registerCheckbox (*checkbox, CuiPreferences::setNewVendorDoubleClickExamine, CuiPreferences::getNewVendorDoubleClickExamine, ConfigClientUserInterface::GetEnableNewVendorExamine);

	getCodeDataObject(TUISliderbar, slider, "sliderBuffIconSizeStatus");
	registerSlider(*slider, CuiPreferences::setBuffIconSizeStatus, CuiPreferences::getBuffIconSizeStatus, CuiPreferences::getBuffIconSizeStatusDefault, CuiPreferences::getBuffIconSizeSliderMin(), CuiPreferences::getBuffIconSizeSliderMax());

	getCodeDataObject(TUISliderbar, slider, "sliderBuffIconSizeTarget");
	registerSlider(*slider, CuiPreferences::setBuffIconSizeTarget, CuiPreferences::getBuffIconSizeTarget, CuiPreferences::getBuffIconSizeTargetDefault, CuiPreferences::getBuffIconSizeSliderMin(), CuiPreferences::getBuffIconSizeSliderMax());

	getCodeDataObject(TUISliderbar, slider, "sliderBuffIconSizeSecondaryTarget");
	registerSlider(*slider, CuiPreferences::setBuffIconSizeSecondaryTarget, CuiPreferences::getBuffIconSizeSecondaryTarget, CuiPreferences::getBuffIconSizeSecondaryTargetDefault, CuiPreferences::getBuffIconSizeSliderMin(), CuiPreferences::getBuffIconSizeSliderMax());

	getCodeDataObject(TUISliderbar, slider, "sliderBuffIconSizePet");
	registerSlider(*slider, CuiPreferences::setBuffIconSizePet, CuiPreferences::getBuffIconSizePet, CuiPreferences::getBuffIconSizePetDefault, CuiPreferences::getBuffIconSizeSliderMin(), CuiPreferences::getBuffIconSizeSliderMax());

	//getCodeDataObject(TUISliderbar, slider, "sliderBuffIconSizeGroup");
	//registerSlider(*slider, CuiPreferences::setBuffIconSizeGroup, CuiPreferences::getBuffIconSizeGroup, ConfigClientUserInterface::getBuffIconSizeGroup, ConfigClientUserInterface::getBuffIconSizeSliderMin(), ConfigClientUserInterface::getBuffIconSizeSliderMax());

	getCodeDataObject(TUISliderbar, slider, "sliderBuffIconWhirlygigOpacity");
	registerSlider(*slider, CuiPreferences::setBuffIconWhirlygigOpacity, CuiPreferences::getBuffIconWhirlygigOpacity, CuiPreferences::getBuffIconWhirlygigOpacityDefault, 0.0f, 1.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkShowQuestHelper");
	registerCheckbox (*checkbox, CuiPreferences::setShowQuestHelper, CuiPreferences::getShowQuestHelper, SwgCuiOptBase::getTrue);

}

//----------------------------------------------------------------------

void SwgCuiOptUi::performActivate ()
{
	setupPaletteCombo (*m_combo);
	SwgCuiOptBase::performActivate ();
}

//----------------------------------------------------------------------

void SwgCuiOptUi::performDeactivate ()
{
	SwgCuiOptBase::performDeactivate ();
}

//----------------------------------------------------------------------

int  SwgCuiOptUi::onComboPaletteGet (const SwgCuiOptBase & , const UIComboBox &)
{
	static const std::string & curPaletteName = CuiPreferences::getPaletteName ();
	return getIndexForPalette (curPaletteName);
}

//----------------------------------------------------------------------

void SwgCuiOptUi::onComboPaletteSet    (const SwgCuiOptBase & , const UIComboBox & , int value)
{
	const CuiPreferences::StringVector & sv = CuiPreferences::getPaletteNames (true);
	int index = 0;
	for (CuiPreferences::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it, ++index)
	{
		if (index == value)
		{
			const std::string & paletteName = *it;
			IGNORE_RETURN(CuiPreferences::setPaletteName (paletteName, true));
			return;
		}
	}

	WARNING (true, ("SwgCuiOptUi::onComboPaletteSet no such palette"));
}
//----------------------------------------------------------------------

void SwgCuiOptUi::resetDefaults   (bool confirmed)
{
	SwgCuiOptBase::resetDefaults(confirmed);
	if(confirmed)
	{
		if(CuiWorkspace::getGameWorkspace())
			CuiWorkspace::getGameWorkspace()->resetAllToDefaultSizeAndLocation();	
	}
}

//----------------------------------------------------------------------

SwgCuiOptUi::~SwgCuiOptUi ()
{
	CuiPreferences::getUseWaypointMonitorCallback ().detachReceiver    (*m_callbackReceiverWaypointMonitor);
	delete m_callbackReceiverWaypointMonitor;
	m_callbackReceiverWaypointMonitor = 0;

	CuiPreferences::getUseExpMonitorCallback ().detachReceiver    (*m_callbackReceiverExpMonitor);
	delete m_callbackReceiverExpMonitor;
	m_callbackReceiverExpMonitor = 0;

	CuiPreferences::getLocationDisplayEnabledCallback ().detachReceiver    (*m_callbackReceiverLocationDisplay);
	delete m_callbackReceiverLocationDisplay;
	m_callbackReceiverLocationDisplay = 0;
}

//======================================================================
