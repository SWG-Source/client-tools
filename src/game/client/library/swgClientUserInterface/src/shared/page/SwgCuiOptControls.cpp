//======================================================================
//
// SwgCuiOptControls.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptControls.h"

#include "UIPage.h"
#include "UIData.h"
#include "UISliderbar.h"
#include "UICheckbox.h"
#include "clientDirectInput/DirectInput.h"
#include "clientGame/ClientHeadTracking.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

//======================================================================
#include "StringId.h"

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsOptControls
{
	MAKE_STRING_ID(ui_opt, controller_found);
	MAKE_STRING_ID(ui_opt, controller_notfound);
};

#undef MAKE_STRING_ID

//----------------------------------------------------------------------

namespace SwgCuiOptControlsNamespace
{
	//-- wrappers for bool->enum conversion

	bool getPilotMouseModeAuto() { return CuiPreferences::PMM_automatic == CuiPreferences::getPilotMouseMode(); }
	bool getPilotMouseModeVirtualJoystick() { return CuiPreferences::PMM_virtualJoystick == CuiPreferences::getPilotMouseMode(); }
	bool getPilotMouseModeCockpitCamera() { return CuiPreferences::PMM_cockpitCamera == CuiPreferences::getPilotMouseMode(); }

	void setPilotMouseModeAuto(bool b) { if (b) CuiPreferences::setPilotMouseMode(CuiPreferences::PMM_automatic); }
	void setPilotMouseModeVirtualJoystick(bool b) { if (b) CuiPreferences::setPilotMouseMode(CuiPreferences::PMM_virtualJoystick); }
	void setPilotMouseModeCockpitCamera(bool b) { if (b) CuiPreferences::setPilotMouseMode(CuiPreferences::PMM_cockpitCamera); }


	//-- wrappers for bool->enum conversion

	bool getPovHatModeSnap() { return CuiPreferences::PHM_snap == CuiPreferences::getPovHatMode(); }
	bool getPovHatModePan() { return CuiPreferences::PHM_pan == CuiPreferences::getPovHatMode(); }

	void setPovHatModeSnap(bool b) { if (b) CuiPreferences::setPovHatMode(CuiPreferences::PHM_snap); }
	void setPovHatModePan(bool b) { if (b) CuiPreferences::setPovHatMode(CuiPreferences::PHM_pan); }

	float getJoystickDeadZoneDefault()
	{
		return 0.1f;
	}

	float getPovHatModeSnapAngleDefault()
	{
		return 60.0f;
	}

	bool getClientHeadTrackingDefault()
	{
		return false;
	}

	void reaquireJoysticks()
	{
		int joysticks = DirectInput::getNumberOfJoysticksAvailable();
		
		DirectInput::reaquireJoystick(); 
		
		if(joysticks < DirectInput::getNumberOfJoysticksAvailable())
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsOptControls::controller_found.localize());
		else
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsOptControls::controller_notfound.localize());
	}
}

using namespace SwgCuiOptControlsNamespace;

//----------------------------------------------------------------------

SwgCuiOptControls::SwgCuiOptControls (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptControls", page),
m_reaquireJoysticks ( NULL )
{
	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderCameraZoomSpeed");
	registerSlider (*slider, FreeChaseCamera::setCameraZoomSpeed, FreeChaseCamera::getCameraZoomSpeed, ConfigClientGame::getFreeChaseCameraZoomSpeed, 0.2f, 1.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderMouseSensitivity");
	registerSlider (*slider, GroundScene::setMouseSensitivity, GroundScene::getMouseSensitivity, ConfigClientGame::getMouseSensitivity, 0.20f, 3.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderMouseInertia");
	registerSlider (*slider, CuiPreferences::setCameraInertia, CuiPreferences::getCameraInertia, ConfigClientUserInterface::getCameraInertia, 0.0f, 30.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderDeadZone");
	registerSlider (*slider, CuiIoWin::setDeadZoneSize, CuiIoWin::getDeadZoneSize, ConfigClientUserInterface::getReticleDeadZoneSizeX, 0, 700);

	getCodeDataObject (TUICheckbox, checkbox, "checkInvertMouseLook");
	registerCheckbox (*checkbox, GroundScene::setInvertMouseLook, GroundScene::getInvertMouseLook, ConfigClientGame::getInvertMouse);

	getCodeDataObject (TUICheckbox, checkbox, "checkMouseMode");
	registerCheckbox (*checkbox, CuiPreferences::setMouseModeDefault, CuiPreferences::getMouseModeDefault, ConfigClientUserInterface::getMouseModeDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkRunDefault");
	registerCheckbox (*checkbox, PlayerCreatureController::setRunWhenMoving, PlayerCreatureController::getRunWhenMoving, ConfigClientGame::getRunWhenMoving);

	getCodeDataObject (TUICheckbox, checkbox, "checkTargetUntargets");
	registerCheckbox (*checkbox, CuiPreferences::setTargetNothingUntargets, CuiPreferences::getTargetNothingUntargets, ConfigClientUserInterface::getTargetNothingUntargets);

	getCodeDataObject (TUICheckbox, checkbox, "checkTurnStrafes");
	registerCheckbox (*checkbox, CuiPreferences::setTurnStrafesDuringMouseModeToggle, CuiPreferences::getTurnStrafesDuringMouseModeToggle, ConfigClientUserInterface::getTurnStrafesDuringMouseModeToggle);

	getCodeDataObject (TUICheckbox, checkbox, "checkCanFireSecondariesFromToolbar");
	registerCheckbox (*checkbox, CuiPreferences::setCanFireSecondariesFromToolbar, CuiPreferences::getCanFireSecondariesFromToolbar, ConfigClientUserInterface::getCanFireSecondariesFromToolbar);

	getCodeDataObject (TUICheckbox, checkbox, "checkShipAutolevel");
	registerCheckbox (*checkbox, CuiPreferences::setShipAutolevel, CuiPreferences::getShipAutolevel, ConfigClientGame::getShipAutolevelDefault);

	// -- Gimbal Setting
	getCodeDataObject (TUICheckbox, checkbox, "checkGimbal");
	registerCheckbox (*checkbox, CuiPreferences::setEnableGimbal, CuiPreferences::getEnableGimbal, ConfigClientUserInterface::getEnableGimbal);

	//-- ship pilot mouse mode

	getCodeDataObject (TUICheckbox, checkbox, "checkPilotMouseModeAuto");
	registerCheckbox (*checkbox, SwgCuiOptControlsNamespace::setPilotMouseModeAuto, SwgCuiOptControlsNamespace::getPilotMouseModeAuto, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkPilotMouseModeVirtualJoystick");
	registerCheckbox (*checkbox, SwgCuiOptControlsNamespace::setPilotMouseModeVirtualJoystick, SwgCuiOptControlsNamespace::getPilotMouseModeVirtualJoystick, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkPilotMouseModeCockpitCamera");
	registerCheckbox (*checkbox, SwgCuiOptControlsNamespace::setPilotMouseModeCockpitCamera, SwgCuiOptControlsNamespace::getPilotMouseModeCockpitCamera, SwgCuiOptBase::getFalse);

	//-- joystick invert
	getCodeDataObject (TUICheckbox, checkbox, "checkJoystickInvert");
	registerCheckbox (*checkbox, CuiPreferences::setJoystickInverted, CuiPreferences::isJoystickInverted, SwgCuiOptBase::getFalse);

	//-- joystick sensitivity
	getCodeDataObject (TUISliderbar, slider, "sliderJoystickSensitivity");
	registerSlider (*slider, CuiPreferences::setJoystickSensitivity, CuiPreferences::getJoystickSensitivity, SwgCuiOptBase::getOne, 0.1f, 2.0f);

	//-- joystick deadzone
	getCodeDataObject (TUISliderbar, slider, "sliderJoystickDeadzone");
	registerSlider (*slider, CuiPreferences::setJoystickDeadZone, CuiPreferences::getJoystickDeadZone, SwgCuiOptControlsNamespace::getJoystickDeadZoneDefault, 0.0f, 0.5f);

	//-- ship pov hat mode (snap)
	getCodeDataObject (TUICheckbox, checkbox, "checkPovHatModeSnap");
	registerCheckbox (*checkbox, SwgCuiOptControlsNamespace::setPovHatModeSnap, SwgCuiOptControlsNamespace::getPovHatModeSnap, SwgCuiOptBase::getTrue);

	//-- pov hat snap angle
	getCodeDataObject (TUISliderbar, slider, "sliderPovHatModeSnapAngle");
	registerSlider (*slider, CuiPreferences::setPovHatSnapAngleDegrees, CuiPreferences::getPovHatSnapAngleDegrees, SwgCuiOptControlsNamespace::getPovHatModeSnapAngleDefault, 30.0f, 180.0f);

	//-- ship pov hat mode (pan)
	getCodeDataObject (TUICheckbox, checkbox, "checkPovHatModePan");
	registerCheckbox (*checkbox, SwgCuiOptControlsNamespace::setPovHatModePan, SwgCuiOptControlsNamespace::getPovHatModePan, SwgCuiOptBase::getFalse);

	//-- pov hat pan speed
	getCodeDataObject (TUISliderbar, slider, "sliderPovHatModePanSpeed");
	registerSlider (*slider, CuiPreferences::setPovHatPanSpeed, CuiPreferences::getPovHatPanSpeed, SwgCuiOptBase::getOne, 0.1f, 2.0f);

	//-- recenter cockpit camera on ship movement
	getCodeDataObject (TUICheckbox, checkbox, "checkCockpitCameraRecenter");
	registerCheckbox (*checkbox, CuiPreferences::setCockpitCameraRecenterOnShipMovement, CuiPreferences::getCockpitCameraRecenterOnShipMovement, SwgCuiOptBase::getTrue);

	//-- cockpit camera snap speed
	getCodeDataObject (TUISliderbar, slider, "sliderCockpitCameraSnapSpeed");
	registerSlider (*slider, CuiPreferences::setCockpitCameraSnapSpeed, CuiPreferences::getCockpitCameraSnapSpeed, SwgCuiOptBase::getOne, 0.1f, 2.0f);

	//-- cockpit camera y offset
	getCodeDataObject (TUISliderbar, slider, "sliderCockpitCameraYOffset");
	registerSlider (*slider, CuiPreferences::setCockpitCameraYOffset, CuiPreferences::getCockpitCameraYOffset, SwgCuiOptBase::getOne, -10.0f, 10.0f);

	//-- cockpit camera zoom multiplier
	getCodeDataObject (TUISliderbar, slider, "sliderCockpitCameraZoomMultiplier");
	registerSlider (*slider, CuiPreferences::setCockpitCameraZoomMultiplier, CuiPreferences::getCockpitCameraZoomMultiplier, SwgCuiOptBase::getOne, 1.0f, 3.0f);

	//-- Client head tracking
	getCodeDataObject(TUICheckbox, checkbox, "checkTrackIr");
	registerCheckbox (*checkbox, ClientHeadTracking::setEnabled, ClientHeadTracking::getEnabled, getClientHeadTrackingDefault);
	if (!ClientHeadTracking::isSupported())
		checkbox->SetEnabled(false);

	// Reaquire Joysticks
	getCodeDataObject(TUIButton, m_reaquireJoysticks, "ButtonFindControllers");
	if(m_reaquireJoysticks)
		registerMediatorObject (*m_reaquireJoysticks, true);
}

//----------------------------------------------------------------------
void
SwgCuiOptControls::OnButtonPressed(UIWidget *context)
{
	if(context == m_reaquireJoysticks)
		SwgCuiOptControlsNamespace::reaquireJoysticks();
	else
		SwgCuiOptBase::OnButtonPressed(context);

}
//======================================================================
