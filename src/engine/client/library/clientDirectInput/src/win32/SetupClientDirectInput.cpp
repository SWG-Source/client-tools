// ======================================================================
//
// SetupClientDirectInput.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientDirectInput/FirstClientDirectInput.h"
#include "clientDirectInput/SetupClientDirectInput.h"

#include "clientDirectInput/DirectInput.h"
#include "clientDirectInput/ConfigClientDirectInput.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientDirectInput/SdlJoystickInput.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"

//-------------------------------------------------------------------

namespace SetupClientDirectInputNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void inputLanguageChangedHook()
	{
		DirectInput::inputLanguageChanged();
	}
};

using namespace SetupClientDirectInputNamespace;

// ======================================================================

void SetupClientDirectInput::install(HINSTANCE instanceHandle, HWND window, DWORD menuKey, IsWindowedMode isWindowedMode)
{
	InstallTimer const installTimer("SetupClientDirectInput::install");

	ConfigClientDirectInput::install();

	// Install the SDL joystick/gamepad backend BEFORE DirectInput so that
	// DirectInput's joystick enumeration knows to stand down (keyboard/mouse
	// stay on DirectInput).  Falls back to DirectInput joysticks if SDL fails.
	if (ConfigClientDirectInput::getUseSdlInput())
	{
		SdlJoystickInput::install();
		if (SdlJoystickInput::isInstalled())
			ExitChain::add(SdlJoystickInput::remove, "SdlJoystickInput");
	}

	DirectInput::install(instanceHandle, window, menuKey, isWindowedMode);

	Os::setInputLanguageChangedHookFunction(inputLanguageChangedHook);

	ForceFeedbackEffectTemplateList::install();
}

// ======================================================================
