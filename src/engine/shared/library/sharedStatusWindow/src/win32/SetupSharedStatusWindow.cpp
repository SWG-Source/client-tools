// ======================================================================
//
// SetupSharedStatusWindow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedStatusWindow/FirstSharedStatusWindow.h"
#include "sharedStatusWindow/SetupSharedStatusWindow.h"

#include "sharedStatusWindow/ConfigSharedStatusWindow.h"
#include "sharedStatusWindow/StatusWindow.h"

// ======================================================================

void SetupSharedStatusWindow::install(HINSTANCE hinstance)
{
	ConfigSharedStatusWindow::install();
	NOT_NULL(hinstance);
	StatusWindow::install(hinstance);
}

// ======================================================================
