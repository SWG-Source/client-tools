// ======================================================================
//
// SdlJoystickInput.cpp
// Native multi-device controller support (SDL3 backend)
//
// ======================================================================

#include "clientDirectInput/FirstClientDirectInput.h"
#include "clientDirectInput/SdlJoystickInput.h"

#include "clientDirectInput/ConfigClientDirectInput.h"
#include "sharedIoWin/IoWinManager.h"

#include <SDL3/SDL.h>
#include <math.h>

// ======================================================================

namespace SdlJoystickInputNamespace
{
	// Engine joystick capacity (mirror of InputMap limits).
	constexpr int cms_maxAxes = 6; // X,Y,Z,RX,RY,RZ
	constexpr int cms_maxSliders = 2;
	constexpr int cms_maxButtons = 32;
	constexpr int cms_maxHats = 4;
	constexpr float cms_signedAxisMaximum = 32767.0f;
	constexpr float cms_signedAxisBias = 32768.0f;
	constexpr float cms_signedAxisRange = 65535.0f;
	constexpr float cms_pi = 3.14159265358979323846f;

	// Normalization to match the legacy DirectInput driver behavior so flight
	// "feel" is unchanged on day one (10% dead zone, 90% saturation).
	constexpr float cms_axisDeadZone = 0.10f;
	constexpr float cms_axisSaturation = 0.90f;

	struct Device
	{
		bool used;
		SDL_JoystickID instanceId;
		SDL_Joystick *joy; // always valid for an open device
		SDL_Gamepad *pad;  // non-null when opened as a mapped gamepad
		bool isGamepad;
		std::string guid;
		std::string name;

		bool lastButton[cms_maxButtons];
		int lastHat[cms_maxHats]; // SDL_HAT_* value last emitted
		int lastDpadHat;		  // gamepad d-pad rendered as hat 0
		float axisValue[cms_maxAxes];
		float sliderValue[cms_maxSliders];
	};

	bool ms_installed = false;
	Device ms_devices[SdlJoystickInput::cms_maxDevices];
	SdlJoystickInput::DeviceChangeCallback ms_callback = 0;

	// ------------------------------------------------------------------

	float normalizeAxis(Sint16 raw)
	{
		float v = static_cast<float>(raw) / cms_signedAxisMaximum;
		if (v > 1.0f)
			v = 1.0f;
		if (v < -1.0f)
			v = -1.0f;

		float const a = fabsf(v);
		if (a <= cms_axisDeadZone)
			return 0.0f;
		if (a >= cms_axisSaturation)
			return (v < 0.0f) ? -1.0f : 1.0f;

		float const scaled = (a - cms_axisDeadZone) / (cms_axisSaturation - cms_axisDeadZone);
		return (v < 0.0f) ? -scaled : scaled;
	}

	// A throttle / trigger reported as a full axis, mapped to a 0..1 slider range.
	float axisToSlider(Sint16 raw)
	{
		float v = (static_cast<float>(raw) + cms_signedAxisBias) / cms_signedAxisRange;
		if (v < 0.0f)
			v = 0.0f;
		if (v > 1.0f)
			v = 1.0f;
		return v;
	}

	// Xbox/PS5 trigger: SDL reports 0..32767, map to 0..1.
	float triggerToSlider(Sint16 raw)
	{
		float v = static_cast<float>(raw) / cms_signedAxisMaximum;
		if (v < 0.0f)
			v = 0.0f;
		if (v > 1.0f)
			v = 1.0f;
		return v;
	}

	// Convert an SDL hat bitmask to the engine's POV angle (radians, 0=up, CW),
	// returning -1 for centered.
	float hatToAngle(int hat)
	{
		switch (hat)
		{
			case SDL_HAT_UP:
				return 0.0f;
			case SDL_HAT_RIGHTUP:
				return cms_pi * 0.25f;
			case SDL_HAT_RIGHT:
				return cms_pi * 0.50f;
			case SDL_HAT_RIGHTDOWN:
				return cms_pi * 0.75f;
			case SDL_HAT_DOWN:
				return cms_pi;
			case SDL_HAT_LEFTDOWN:
				return cms_pi * 1.25f;
			case SDL_HAT_LEFT:
				return cms_pi * 1.50f;
			case SDL_HAT_LEFTUP:
				return cms_pi * 1.75f;
			default:
				return -1.0f; // SDL_HAT_CENTERED
		}
	}

	void emitAxis(int number, int axisIndex, float value)
	{
		switch (axisIndex)
		{
			case 0:
				IoWinManager::queueJoystickTranslateX(number, value);
				break;
			case 1:
				IoWinManager::queueJoystickTranslateY(number, value);
				break;
			case 2:
				IoWinManager::queueJoystickTranslateZ(number, value);
				break;
			case 3:
				IoWinManager::queueJoystickRotateX(number, value);
				break;
			case 4:
				IoWinManager::queueJoystickRotateY(number, value);
				break;
			case 5:
				IoWinManager::queueJoystickRotateZ(number, value);
				break;
			default:
				break;
		}
	}

	void emitHat(int number, int hatIndex, int sdlHat)
	{
		float const angle = hatToAngle(sdlHat);
		if (angle < 0.0f)
			IoWinManager::queueJoystickPOVHatCentered(number, hatIndex);
		else
			IoWinManager::queueJoystickPOVHatOffset(number, hatIndex, angle);
	}

	int findDeviceByInstance(SDL_JoystickID id)
	{
		for (int i = 0; i < SdlJoystickInput::cms_maxDevices; ++i)
			if (ms_devices[i].used && ms_devices[i].instanceId == id)
				return i;
		return -1;
	}

	int findFreeSlot()
	{
		for (int i = 0; i < SdlJoystickInput::cms_maxDevices; ++i)
			if (!ms_devices[i].used)
				return i;
		return -1;
	}

	void resetDeviceState(Device &d)
	{
		int i;
		for (i = 0; i < cms_maxButtons; ++i)
			d.lastButton[i] = false;
		for (i = 0; i < cms_maxHats; ++i)
			d.lastHat[i] = SDL_HAT_CENTERED;
		d.lastDpadHat = SDL_HAT_CENTERED;
		for (i = 0; i < 6; ++i)
			d.axisValue[i] = 0.0f;
		d.sliderValue[0] = 0.0f;
		d.sliderValue[1] = 0.0f;
	}

	bool openDevice(SDL_JoystickID id)
	{
		int const slot = findFreeSlot();
		if (slot < 0)
			return false; // already at capacity

		Device &d = ms_devices[slot];

		d.isGamepad = SDL_IsGamepad(id);
		d.pad = 0;
		d.joy = 0;

		if (d.isGamepad)
		{
			d.pad = SDL_OpenGamepad(id);
			if (d.pad)
				d.joy = SDL_GetGamepadJoystick(d.pad);
		}
		else
		{
			d.joy = SDL_OpenJoystick(id);
		}

		if (!d.joy)
		{
			if (d.pad)
				SDL_CloseGamepad(d.pad);
			d.pad = 0;
			return false;
		}

		SDL_GUID const sdlGuid = SDL_GetJoystickGUID(d.joy);
		char guidBuf[64];
		guidBuf[0] = '\0';
		SDL_GUIDToString(sdlGuid, guidBuf, sizeof(guidBuf));

		char const *const nm = d.isGamepad ? SDL_GetGamepadName(d.pad) : SDL_GetJoystickName(d.joy);

		d.used = true;
		d.instanceId = id;
		d.guid = guidBuf;
		d.name = nm ? nm : "Controller";
		resetDeviceState(d);

		DEBUG_REPORT_LOG(true, ("SdlJoystickInput: opened device %d '%s' guid=%s %s\n",
								slot, d.name.c_str(), d.guid.c_str(), d.isGamepad ? "(gamepad)" : "(joystick)"));

		return true;
	}

	void closeDevice(int slot)
	{
		Device &d = ms_devices[slot];
		if (!d.used)
			return;

		// NOTE: do not emit IoWin events here -- closeDevice can run during
		// install (before IoWinManager is ready) and during shutdown.  The
		// input-reset for a hot-unplug is issued from update() instead.

		if (d.pad)
			SDL_CloseGamepad(d.pad);
		else if (d.joy)
			SDL_CloseJoystick(d.joy);

		d.used = false;
		d.instanceId = 0;
		d.joy = 0;
		d.pad = 0;
		d.isGamepad = false;
		d.guid.clear();
		d.name.clear();
	}

	// Re-sync the open device set against what SDL currently reports attached.
	// Returns true if the set changed.
	bool syncDeviceList()
	{
		bool changed = false;

		int count = 0;
		SDL_JoystickID *ids = SDL_GetJoysticks(&count);

		bool present[SdlJoystickInput::cms_maxDevices];
		for (int i = 0; i < SdlJoystickInput::cms_maxDevices; ++i)
			present[i] = false;

		if (ids)
		{
			for (int i = 0; i < count; ++i)
			{
				int slot = findDeviceByInstance(ids[i]);
				if (slot < 0)
				{
					if (openDevice(ids[i]))
					{
						slot = findDeviceByInstance(ids[i]); // slot it was opened into
						changed = true;
					}
				}
				// mark BOTH existing and newly-opened devices as present so the
				// cleanup pass below does not immediately close a fresh device
				if (slot >= 0)
					present[slot] = true;
			}
			SDL_free(ids);
		}

		for (int i = 0; i < SdlJoystickInput::cms_maxDevices; ++i)
		{
			if (ms_devices[i].used && !present[i])
			{
				closeDevice(i);
				changed = true;
			}
		}

		return changed;
	}

	// Render the gamepad d-pad (4 buttons) as a single POV hat.
	int dpadToHat(SDL_Gamepad *pad)
	{
		int hat = SDL_HAT_CENTERED;
		if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP))
			hat |= SDL_HAT_UP;
		if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
			hat |= SDL_HAT_DOWN;
		if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
			hat |= SDL_HAT_LEFT;
		if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
			hat |= SDL_HAT_RIGHT;
		return hat;
	}

	bool isDpadButton(int gamepadButton)
	{
		return gamepadButton == SDL_GAMEPAD_BUTTON_DPAD_UP || gamepadButton == SDL_GAMEPAD_BUTTON_DPAD_DOWN || gamepadButton == SDL_GAMEPAD_BUTTON_DPAD_LEFT || gamepadButton == SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
	}

	void updateGamepad(int number, Device &d)
	{
		SDL_Gamepad *const pad = d.pad;

		// axes (continuous): left stick -> X/Y, right stick -> RX/RY
		d.axisValue[0] = normalizeAxis(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX));
		d.axisValue[1] = normalizeAxis(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY));
		d.axisValue[3] = normalizeAxis(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX));
		d.axisValue[4] = normalizeAxis(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY));
		emitAxis(number, 0, d.axisValue[0]);
		emitAxis(number, 1, d.axisValue[1]);
		emitAxis(number, 3, d.axisValue[3]);
		emitAxis(number, 4, d.axisValue[4]);

		// triggers -> sliders 0 / 1
		d.sliderValue[0] = triggerToSlider(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
		d.sliderValue[1] = triggerToSlider(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
		IoWinManager::queueJoystickSlider(number, 0, d.sliderValue[0]);
		IoWinManager::queueJoystickSlider(number, 1, d.sliderValue[1]);

		// buttons (edge), d-pad handled separately as a POV hat
		int const buttonCount = static_cast<int>(SDL_GAMEPAD_BUTTON_COUNT);
		for (int b = 0; b < buttonCount && b < cms_maxButtons; ++b)
		{
			if (isDpadButton(b))
				continue;

			bool const down = SDL_GetGamepadButton(pad, static_cast<SDL_GamepadButton>(b));
			if (down != d.lastButton[b])
			{
				if (down)
					IoWinManager::queueJoystickButtonDown(number, b);
				else
					IoWinManager::queueJoystickButtonUp(number, b);
				d.lastButton[b] = down;
			}
		}

		// d-pad -> POV hat 0 (edge)
		int const dpad = dpadToHat(pad);
		if (dpad != d.lastDpadHat)
		{
			emitHat(number, 0, dpad);
			d.lastDpadHat = dpad;
		}
	}

	void updateJoystick(int number, Device &d)
	{
		SDL_Joystick *const joy = d.joy;

		int const numAxes = SDL_GetNumJoystickAxes(joy);

		// axes 0..5 -> engine axes (continuous)
		for (int i = 0; i < numAxes && i < cms_maxAxes; ++i)
		{
			d.axisValue[i] = normalizeAxis(SDL_GetJoystickAxis(joy, i));
			emitAxis(number, i, d.axisValue[i]);
		}
		// extra axes 6..7 -> sliders 0..1 (e.g. additional throttle / rudder rocker)
		for (int i = cms_maxAxes; i < numAxes && (i - cms_maxAxes) < cms_maxSliders; ++i)
		{
			int const s = i - cms_maxAxes;
			d.sliderValue[s] = axisToSlider(SDL_GetJoystickAxis(joy, i));
			IoWinManager::queueJoystickSlider(number, s, d.sliderValue[s]);
		}

		// buttons (edge)
		int const numButtons = SDL_GetNumJoystickButtons(joy);
		for (int b = 0; b < numButtons && b < cms_maxButtons; ++b)
		{
			bool const down = SDL_GetJoystickButton(joy, b);
			if (down != d.lastButton[b])
			{
				if (down)
					IoWinManager::queueJoystickButtonDown(number, b);
				else
					IoWinManager::queueJoystickButtonUp(number, b);
				d.lastButton[b] = down;
			}
		}

		// POV hats (edge)
		int const numHats = SDL_GetNumJoystickHats(joy);
		for (int h = 0; h < numHats && h < cms_maxHats; ++h)
		{
			int const hat = SDL_GetJoystickHat(joy, h);
			if (hat != d.lastHat[h])
			{
				emitHat(number, h, hat);
				d.lastHat[h] = hat;
			}
		}
	}
} // namespace SdlJoystickInputNamespace

using namespace SdlJoystickInputNamespace;

// ======================================================================

void SdlJoystickInput::install()
{
	if (ms_installed)
		return;

	for (int i = 0; i < cms_maxDevices; ++i)
	{
		ms_devices[i].used = false;
		ms_devices[i].joy = 0;
		ms_devices[i].pad = 0;
	}

	// Background events not needed: we poll device state every frame from the
	// game loop.  Keep SDL's own helper window so hot-plug is detected.
	if (!SDL_Init(SDL_INIT_GAMEPAD))
	{
		WARNING(true, ("SdlJoystickInput: SDL_Init(SDL_INIT_GAMEPAD) failed: %s", SDL_GetError()));
		return;
	}

	ms_installed = true;

	// open whatever is already attached
	IGNORE_RETURN(syncDeviceList());

	DEBUG_REPORT_LOG(true, ("SdlJoystickInput: installed, %d device(s) attached\n", getNumberOfDevices()));
}

// ----------------------------------------------------------------------

void SdlJoystickInput::remove()
{
	if (!ms_installed)
		return;

	for (int i = 0; i < cms_maxDevices; ++i)
		closeDevice(i);

	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
	SDL_Quit();

	ms_installed = false;
	ms_callback = 0;
}

// ----------------------------------------------------------------------

bool SdlJoystickInput::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------

void SdlJoystickInput::setDeviceChangeCallback(DeviceChangeCallback callback)
{
	ms_callback = callback;
}

// ----------------------------------------------------------------------

void SdlJoystickInput::update()
{
	if (!ms_installed)
		return;

	// refresh device state and the attached-device list
	SDL_UpdateGamepads();
	SDL_UpdateJoysticks();

	bool const changed = syncDeviceList();

	// a device was attached/removed this frame: clear any held input so a stick
	// that went away doesn't leave the ship turning, and re-resolve binding slots
	if (changed)
		IoWinManager::queueInputReset();

	for (int i = 0; i < cms_maxDevices; ++i)
	{
		Device &d = ms_devices[i];
		if (!d.used)
			continue;

		if (d.isGamepad && d.pad)
			updateGamepad(i, d);
		else if (d.joy)
			updateJoystick(i, d);
	}

	if (changed && ms_callback)
		ms_callback();
}

// ----------------------------------------------------------------------

void SdlJoystickInput::rescan()
{
	if (!ms_installed)
		return;

	SDL_UpdateGamepads();
	SDL_UpdateJoysticks();

	if (syncDeviceList() && ms_callback)
		ms_callback();
}

// ----------------------------------------------------------------------

int SdlJoystickInput::getNumberOfDevices()
{
	int n = 0;
	for (int i = 0; i < cms_maxDevices; ++i)
		if (ms_devices[i].used)
			++n;
	return n;
}

// ----------------------------------------------------------------------

bool SdlJoystickInput::getDeviceByIndex(int index, int &number, std::string &guid, std::string &name, bool &isGamepad)
{
	int n = 0;
	for (int i = 0; i < cms_maxDevices; ++i)
	{
		if (!ms_devices[i].used)
			continue;

		if (n == index)
		{
			number = i; // emitted IoEvent::arg1 for this device
			guid = ms_devices[i].guid;
			name = ms_devices[i].name;
			isGamepad = ms_devices[i].isGamepad;
			return true;
		}
		++n;
	}
	return false;
}

// ----------------------------------------------------------------------

int SdlJoystickInput::getDeviceNumberForGuid(const char *guid)
{
	if (!guid || !*guid)
		return -1;
	for (int i = 0; i < cms_maxDevices; ++i)
		if (ms_devices[i].used && ms_devices[i].guid == guid)
			return i;
	return -1;
}

// ----------------------------------------------------------------------

float SdlJoystickInput::getAxisState(int number, int axis)
{
	if (number < 0 || number >= cms_maxDevices || !ms_devices[number].used)
		return 0.0f;
	if (axis < 0 || axis >= 6)
		return 0.0f;
	return ms_devices[number].axisValue[axis];
}

// ----------------------------------------------------------------------

float SdlJoystickInput::getSliderState(int number, int slider)
{
	if (number < 0 || number >= cms_maxDevices || !ms_devices[number].used)
		return 0.0f;
	if (slider < 0 || slider >= 2)
		return 0.0f;
	return ms_devices[number].sliderValue[slider];
}

// ======================================================================
