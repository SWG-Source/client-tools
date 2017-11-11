//======================================================================
//
// SwgCuiChatWindow_TabChanged.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow_TabChanged.h"

//======================================================================

SwgCuiChatWindow::TabChanged::TabChanged (const SwgCuiChatWindow * _pointerWindow, const Tab * _pointerTab, const Tab & _updatedTab) :
pointerWindow  (_pointerWindow),
pointerTab     (_pointerTab),
updatedTab     (_updatedTab)
{
}

//----------------------------------------------------------------------

SwgCuiChatWindow::TabChanged::~TabChanged ()
{
	pointerWindow  = 0;
	pointerTab     = 0;
}

//======================================================================
