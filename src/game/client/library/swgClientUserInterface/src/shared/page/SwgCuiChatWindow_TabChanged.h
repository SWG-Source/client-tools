//======================================================================
//
// SwgCuiChatWindow_TabChanged.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatWindow_TabChanged_H
#define INCLUDED_SwgCuiChatWindow_TabChanged_H

//======================================================================

#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"

class SwgCuiChatWindow::TabChanged
{
public:
	const SwgCuiChatWindow * pointerWindow;
	const Tab              * pointerTab;
	const Tab                updatedTab;

	TabChanged (const SwgCuiChatWindow * _pointerWindow, const Tab * _pointerTab, const Tab & _updatedTab);
	~TabChanged ();

private:
	TabChanged ();
	TabChanged & operator=(const TabChanged & rhs);
	TabChanged (const TabChanged & rhs);
};

//======================================================================

#endif
