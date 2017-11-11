// ======================================================================
//
// ConfigSharedStatusWindow.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedStatusWindow/FirstSharedStatusWindow.h"
#include "sharedStatusWindow/ConfigSharedStatusWindow.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedStatusWindow", #a,   (b)))

// ======================================================================

namespace ConfigSharedStatusWindowNamespace
{
	bool ms_sample;
}

using namespace ConfigSharedStatusWindowNamespace;

// ======================================================================

void ConfigSharedStatusWindow::install()
{
	KEY_BOOL(sample, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedStatusWindow::getSample()
{
	return ms_sample;
}

// ======================================================================


