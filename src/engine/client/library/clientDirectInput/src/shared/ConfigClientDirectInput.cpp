// ======================================================================
//
// ConfigClientDirectInput.cpp
// copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "ClientDirectInput/FirstClientDirectInput.h"
#include "ClientDirectInput/ConfigClientDirectInput.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientDirectInput", #a, b))
#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientDirectInput", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("ClientDirectInput", #a, b))

// ======================================================================

namespace ConfigClientDirectInputNamespace
{
	bool         ms_useJoysticks;
	int          ms_useJoystick;
	bool         ms_useMouse;
	bool         ms_windowsKeyEnabledWhenFullscreen;
	bool         ms_windowsKeyEnabledWhenWindowed;
	char const * ms_keyboardTranslationTable;
	bool         ms_enableForceFeedback;
}
using namespace ConfigClientDirectInputNamespace;

// ======================================================================

void ConfigClientDirectInput::install()
{
	KEY_BOOL(useJoysticks,                    true);
	KEY_INT(useJoystick,                      0);
	KEY_BOOL(useMouse,                        true);
	KEY_BOOL(windowsKeyEnabledWhenFullscreen, false);
	KEY_BOOL(windowsKeyEnabledWhenWindowed,   true);
	KEY_STRING(keyboardTranslationTable,      NULL);
	KEY_BOOL(enableForceFeedback,             true);
}

// ----------------------------------------------------------------------

bool ConfigClientDirectInput::getUseJoysticks()
{
	return ms_useJoysticks;
}

// ----------------------------------------------------------------------

int ConfigClientDirectInput::getUseJoystick()
{
	return ms_useJoystick;
}

// ----------------------------------------------------------------------

bool ConfigClientDirectInput::getUseMouse()
{
	return ms_useMouse;
}

// ----------------------------------------------------------------------

bool ConfigClientDirectInput::getWindowsKeyEnabledWhenFullscreen()
{
	return ms_windowsKeyEnabledWhenFullscreen;
}

// ----------------------------------------------------------------------

bool ConfigClientDirectInput::getWindowsKeyEnabledWhenWindowed()
{
	return ms_windowsKeyEnabledWhenWindowed;
}

// ----------------------------------------------------------------------

char const * ConfigClientDirectInput::getKeyboardTranslationTable()
{
	return ms_keyboardTranslationTable;
}

// ----------------------------------------------------------------------

bool ConfigClientDirectInput::getEnableForceFeedback()
{
	return ms_enableForceFeedback;
}

// ======================================================================
