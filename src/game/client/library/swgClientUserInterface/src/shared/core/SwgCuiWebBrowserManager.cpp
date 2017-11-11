// ======================================================================
//
// SwgCuiWebBrowserManager.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"

#include "clientGame/Game.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "libMozilla/libMozilla.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWidget.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWindow.h"

#include "UIData.h"
#include "UIManager.h"
#include "UiPage.h"
#include "UIText.h"

#include <list>

namespace SwgCuiWebBrowserManagerNamespace
{
	bool s_installed;

	enum webBrowserType
	{
		WBT_Invalid = 0,
		WBT_Ground,
		WBT_Space,
	};

	webBrowserType                                s_type;

	SwgCuiWebBrowserWindow * s_Window = NULL;
	SwgCuiWebBrowserWidget * s_Widget = NULL;

	std::string              s_homePage;
	std::string              s_URL;
	std::string              s_postData;
	int                      s_postDataLength;

	Timer                    s_timer;
}

using namespace SwgCuiWebBrowserManagerNamespace;

void SwgCuiWebBrowserManager::install()
{
	DEBUG_FATAL(s_installed, ("SwgCuiWebBrowserManager already installed.\n"));
	
	s_installed = true;
	s_Widget = NULL;
	s_Window = NULL;
	s_type = WBT_Invalid;
	s_timer.setExpireTime(0.1f);

	libMozilla::setUserAgent("StarWarsGalaxies");
	libMozilla::enableMemoryCache(true);
	libMozilla::enableDiskCache(true, 50 * 1024);
}

void SwgCuiWebBrowserManager::remove()
{
	DEBUG_FATAL(!s_installed, ("SwgCuiWebBrowserManager not installed. Failed Remove.\n"));
	s_installed = false;

	if(s_Window)
	{

		CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
		if (workspace)
		{
			workspace->removeMediator(*s_Window);
		}

		s_Window->release();
	}

	s_Widget = NULL;

}

void SwgCuiWebBrowserManager::update(float deltaTimeSecs)
{

	// Update Mozilla
	libMozilla::update();

	if(s_Widget)
		s_Widget->alter(deltaTimeSecs);
	
	bool isValidPage = Game::isSpace() ? (s_type == WBT_Space) : (s_type == WBT_Ground);

	if(s_Window && isValidPage)
	{
		// Since the Web browser is manually constructed, we need to make sure it's always in the current workspace.
		CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
		CuiWorkspace * const currentWorkspace = s_Window->getContainingWorkspace();
		
		if (workspace && workspace != currentWorkspace)
		{
			if(currentWorkspace)
				currentWorkspace->removeMediator(*s_Window);
			workspace->addMediator(*s_Window);
		}
	}

	if(!s_URL.empty() && s_timer.updateZero(deltaTimeSecs))
	{
		if(s_Widget)
		{
			if(s_postData.empty())
				s_Widget->setURL(s_URL);
			else
				s_Widget->setURL(s_URL, s_postData.c_str(), s_postDataLength);
		}

		s_URL.clear();
		s_postData.clear();
		s_postDataLength = 0;
	}
}

void SwgCuiWebBrowserManager::createWebBrowserPage(bool useHomePage)
{
	bool isValidPage = Game::isSpace() ? (s_type == WBT_Space) : (s_type == WBT_Ground);
	
	if(s_Window && isValidPage)
	{
		CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
		if (workspace)
		{
			s_Window->activate();
			s_Window->setEnabled(true);
			workspace->focusMediator(*s_Window, true);
		}

		return;
	}
	else if (s_Window && !isValidPage)
	{
		CuiWorkspace * const workspace = s_Window->getContainingWorkspace();
		if (workspace)
		{
			workspace->removeMediator(*s_Window);
		}

		s_Window->release();
		s_Window = NULL;
		s_Widget = NULL;
	}

	if(Game::isSpace())
		s_type = WBT_Space;
	else
		s_type = WBT_Ground;

	// Create a Web browser page.
	std::string templateName = "/PDA.WebBrowser";

	//get the template
	UIPage const * const prototype = NON_NULL(safe_cast<UIPage const *>(UIManager::gUIManager().GetObjectFromPath(templateName.c_str(), TUIPage)));

	if (!prototype)
	{
		WARNING (true, ("Bad browser page name <%s> given", templateName.c_str()));
		return;
	}

	//copy the "template" into the page that we'll use
	UIPage * const newPage = dynamic_cast<UIPage*>(NON_NULL(prototype->DuplicateObject()));
	DEBUG_FATAL(!newPage, ("Couldn't find base page in SwgCuiWebBrowser::createWebBrowserPage"));
	NOT_NULL(newPage);

	//create the mediator for the page
	SwgCuiWebBrowserWindow * browserWindow = NULL;

	browserWindow = new SwgCuiWebBrowserWindow(*newPage);

	s_Window = browserWindow;

	browserWindow->fetch();

	//add the page to the game  workspace (like a HUD)

	CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
	if (workspace)
	{
		workspace->addMediator(*browserWindow);
	}

	newPage->Link();

	UIPage * const browserWidgetParent = NON_NULL(safe_cast<UIPage *>(browserWindow->getPage().GetChild("webBrowserControl")));

	if (browserWidgetParent)
	{
		SwgCuiWebBrowserWidget * const browserWidget = new SwgCuiWebBrowserWidget;

		if(!s_homePage.empty())
			browserWidget->setHomePage(s_homePage);
		
		// This is very strange behavior from Mozilla.
		// If we want to instantly send the web browser somewhere(and the mozilla window has never been created before)
		// then we need to create it now instead of new frame - otherwise our setURL command will fail due to internal mozilla shenanigans.
		if(!useHomePage)
			browserWidget->createMozillaWindow();

		browserWidget->SetSize(browserWidgetParent->GetSize());
		browserWidget->SetVisible(true);
		browserWidget->SetName(UINarrowString("Browser"));

		browserWidgetParent->AddChild(browserWidget);
		browserWidget->SetProperty(UIWidget::PropertyName::PackSize, Unicode::narrowToWide("a,a"));
		browserWidget->SetAbsorbsInput(true);
		browserWidget->SetGetsInput(true);
		browserWidget->SetSelectable(true);

		s_Widget = browserWidget;

		UIImage* browserImage;
		browserWindow->getCodeDataObject (TUIImage, browserImage, "browserimage");

		browserWidget->setUIImage(browserImage);
		browserImage->SetSelectable(true);

		UIText* urlTextField;
		
		browserWindow->getCodeDataObject(TUIText, urlTextField, "urltext");
		urlTextField->SetSelectable(true);

		browserWidget->setUIText(urlTextField);

		browserWindow->SetBrowserWidget(browserWidget);
	}

	browserWindow->activate();
	browserWindow->setEnabled(true);
	workspace->focusMediator(*browserWindow, true);
	
}

void SwgCuiWebBrowserManager::debugOutput()
{
	if(s_Widget)
		s_Widget->debugOutput();
}

void SwgCuiWebBrowserManager::setURL(std::string url, bool clearCachedURL, char const * postData, int postDataLength)
{
	if(s_URL.empty() || clearCachedURL )
	{
		s_URL = url;

		if(postData)
		{
			s_postData = std::string(postData);
			s_postDataLength = postDataLength;
		}

		s_timer.reset();
	}

}

void SwgCuiWebBrowserManager::navigateForward()
{
	if(s_Widget)
		s_Widget->NavigateForward();
}

void SwgCuiWebBrowserManager::navigateBack()
{
	if(s_Widget)
		s_Widget->NavigateBack();
}

void SwgCuiWebBrowserManager::navigateStop()
{
	if(s_Widget)
		s_Widget->NavigateStop();
}

void SwgCuiWebBrowserManager::refreshPage()
{
	if(s_Widget)
		s_Widget->RefreshPage();
}

void SwgCuiWebBrowserManager::setHomePage(std::string const & home)
{
	s_homePage = home;
}