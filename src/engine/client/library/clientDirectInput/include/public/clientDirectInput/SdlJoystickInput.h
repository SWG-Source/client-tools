// ======================================================================
//
// SdlJoystickInput.h
// Native multi-device controller support
//
// SDL3-backed joystick / gamepad input.  Enumerates ALL attached game
// controllers (HOTAS sticks, throttles, rudder pedals, Xbox/PS5/Switch Pro
// gamepads), and feeds each one's axes / sliders / buttons / POV hats into the
// engine through the existing IoWinManager::queueJoystick* path, tagged with a
// stable per-device "number" (event->arg1).  The sharedInputMap binding layer
// then routes each device to its own binding slot, so a separate stick and
// throttle can be bound independently without vJoy / Joystick Gremlin.
//
// Keyboard and mouse remain on DirectInput8; this backend only owns joysticks.
//
// ======================================================================

#ifndef INCLUDED_SdlJoystickInput_H
#define INCLUDED_SdlJoystickInput_H

// ======================================================================

#include <string>

class SdlJoystickInput
{
public:
	typedef void (*DeviceChangeCallback)();

	// Up to this many simultaneous devices (matches InputMap::MAX_JOYSTICKS).
	static constexpr int cms_maxDevices = 8;

	static void install();
	static void remove();
	static bool isInstalled();

	// Pump SDL and emit this frame's joystick events.  Call once per frame.
	static void update();

	// Force a re-enumeration of attached devices (Options > "Find Controllers").
	static void rescan();

	// Invoked (if set) whenever the set of attached devices changes, so the
	// input map can re-resolve which physical device feeds each binding slot.
	static void setDeviceChangeCallback(DeviceChangeCallback callback);

	// --- device registry queries (used by InputScheme slot resolution & UI) ---

	// Number of currently-attached, usable devices.
	static int getNumberOfDevices();

	// Look up the i-th attached device (0..getNumberOfDevices()-1).
	// 'number' is the value emitted as IoEvent::arg1 for this device.
	static bool getDeviceByIndex(int index, int &number, std::string &guid, std::string &name, bool &isGamepad);

	// The emitted device number for a given stable guid, or -1 if not attached.
	static int getDeviceNumberForGuid(const char *guid);

	// --- live normalized state, for the configuration UI activity bars (M3) ---
	static float getAxisState(int number, int axis);	 // axis 0..5,  range -1..1
	static float getSliderState(int number, int slider); // slider 0..1, range 0..1
};

// ======================================================================

#endif
