// ======================================================================
//
// DirectInput.h
// copyright 2001-2003, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DirectInput_H
#define INCLUDED_DirectInput_H

// ======================================================================

class ForceFeedbackEffectTemplate;
struct IDirectInputEffect;

// ======================================================================

/**
 * Provides low-level user input support through DirectInput.
 *
 * This class will optionally handle keyboard, mouse and joystick
 * input.  Only one joystick is currently supported, although the
 * client may specify which of the attached joysticks is used for
 * input.
 */

class DirectInput
{
public:

	class ForceFeedbackEffect
	{
	public:
		virtual ~ForceFeedbackEffect();

		virtual bool play(int const iterations) const = 0;
		virtual bool stop() const = 0;
		virtual bool isPlaying() const = 0;

	protected:
		ForceFeedbackEffect();
	};

public:

	typedef bool (*ScreenShotFunction)();
	typedef void (*ToggleWindowedModeFunction)();
	typedef bool (*IsWindowedFunction)();
	typedef void (*RequestDebugMenuFunction)();

public:

	static void install(HINSTANCE instanceHandle, HWND window, DWORD menuKey, IsWindowedFunction isWindowedFunction);
	static void remove();

	static bool isInstalled();
	static void suspendInput();
	static void resumeInput();

	static bool getFullscreenWindowsKeyEnabled();
	static void setFullscreenWindowsKeyEnabled(bool enabled);
	static bool getWindowedWindowsKeyEnabled();
	static void setWindowedWindowsKeyEnabled(bool enabled);

	static void setScreenShotFunction(ScreenShotFunction screenShotFunction);
	static void setToggleWindowedModeFunction(ToggleWindowedModeFunction toggleWindowedModeFunction);
	static void setRequestDebugMenuFunction(RequestDebugMenuFunction requestDebugMenuFunction);

	static void unacquireAllDevices();
	static void inputLanguageChanged();
	static bool getScanCodeKeyName(uint8 scanCode, std::string &keyName);

	static bool enumForceFeedbackEffectIntoTemplate(std::string const & file, ForceFeedbackEffectTemplate * effectTemplate);
	static bool destroyForceFeedbackEffect(ForceFeedbackEffect * effect);

	// input update
	static void update();

	// joystick interface
	static int  getMaximumNumberOfJoysticks();
	static int  getMaximumNumberOfSlidersPerJoystick();

	static int getNumberOfJoysticksAvailable();
	static bool isJoystickAvailable(int joystickIndex);
	static bool isSliderAvailable(int joystickIndex, int slider);

	static void setJoystickTranslationRange(int joystickIndex, float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip);
	static void setJoystickTranslationDeadZone(int joystickIndex, float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter);
	static void setJoystickTranslationSaturation(int joystickIndex, float xSaturationPercent, float ySaturationPercent, float zSaturationPercent);

	static void setJoystickRotationRange(int joystickIndex, float xCenterVal, float xHalfRange, bool xFlip, float yCenterVal, float yHalfRange, bool yFlip, float zCenterVal, float zHalfRange, bool zFlip);
	static void setJoystickRotationDeadZone(int joystickIndex, float xPercentAroundCenter, float yPercentAroundCenter, float zPercentAroundCenter);
	static void setJoystickRotationSaturation(int joystickIndex, float xSaturationPercent, float ySaturationPercent, float zSaturationPercent);

	static void setJoystickSliderRange(int joystickIndex, int sliderIndex, float centerVal, float halfRange, bool flip);
	static void setJoystickSliderDeadZone(int joystickIndex, int sliderIndex, float percentAroundCenter);
	static void setJoystickSliderSaturation(int joystickIndex, int sliderIndex, float saturationPercent);

	static void reaquireJoystick();

	// mouse interface
	static bool isMouseAvailable();
	static void setMouseScale(float xMickeysPerUnit, bool xFlip, float yMickeysPerUnit, bool yFlip, float zMickeysPerUnit, bool zFlip);

	static void setIgnoreLAlt(bool ignoreLAlt);
};

// ======================================================================

#endif
