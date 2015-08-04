// ======================================================================
//
// SwgCuiWebBrowserManager.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiWebBrowserManager_H
#define INCLUDED_SwgCuiWebBrowserManager_H

// ======================================================================
#include "swgClientUserInterface/SwgCuiWebBrowserWindow.h"
#include <string>

class SwgCuiWebBrowserManager
{
public:
	static void install();
	static void remove();
	static void update(float deltaTimeSecs);

	//protected:
	static void createWebBrowserPage(bool useHomePage = true);
	static void debugOutput();
	static void setURL(std::string url, bool clearCachedURL = false, char const * postData = NULL, int postDataLength = 0);

	static void navigateForward();
	static void navigateBack();
	static void navigateStop();
	static void refreshPage();

	static void setHomePage(std::string const & home);

private: //disabled
	SwgCuiWebBrowserManager();
	SwgCuiWebBrowserManager(const SwgCuiWebBrowserManager &rhs);
	SwgCuiWebBrowserManager& operator= (const SwgCuiWebBrowserManager &rhs);

};

// ======================================================================

#endif