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
#include "sharedDebug/InstallTimer.h"
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
	DirectInput::install(instanceHandle, window, menuKey, isWindowedMode);

	Os::setInputLanguageChangedHookFunction(inputLanguageChangedHook);

	ForceFeedbackEffectTemplateList::install();
}

// ======================================================================
