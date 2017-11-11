// ======================================================================
//
// SwgCuiWebBrowserWindow.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWindow.h"
#include "SwgClientUserInterface/SwgCuiWebBrowserManager.h"

#include "clientDirectInput/DirectInput.h"

#include "clientGame/Game.h"

#include "clientUserInterface/CuiConsoleHelper.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"

#include "sharedIoWin/IoWinManager.h"
#include "sharedInputMap/InputMap.h"


#include "UIManager.h"
#include "UIMessage.h"

// ======================================================================

namespace browserWindow
{
	std::string s_currentURL;
}

using namespace browserWindow;

SwgCuiWebBrowserWindow::SwgCuiWebBrowserWindow(UIPage & page)
: CuiMediator("WebBrowser", page),
UIEventCallback(),
m_Back (NULL),
m_Forward (NULL),
m_Url (NULL),
m_Go (NULL),
m_Stop (NULL),
m_Refresh (NULL),
m_Close (NULL),
m_Widget (NULL)
{	
	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon(this);
	icon->SetName("SwgCuiWebBrowserWindow");
	icon->SetSize(UISize(32, 32));
	icon->SetBackgroundColor(UIColor(0, 0, 0, 50));
	icon->SetLocation(0, 64);

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));

	setIcon(icon);
	
	getCodeDataObject(TUIButton, m_Back, "back");
	getCodeDataObject(TUIButton, m_Forward, "forward");
	getCodeDataObject(TUIText, m_Url, "urltext");
	getCodeDataObject(TUIButton, m_Go, "gobutton");
	getCodeDataObject(TUIButton, m_Stop, "stop");
	getCodeDataObject(TUIButton, m_Refresh, "refresh");

	if(getButtonClose())
		m_Close = getButtonClose();

	registerMediatorObject(*m_Back, true);
	registerMediatorObject(*m_Forward, true);
	registerMediatorObject(*m_Url, true);
	registerMediatorObject(*m_Go, true);
	registerMediatorObject(*m_Stop, true);
	registerMediatorObject(*m_Refresh, true);

}

// ----------------------------------------------------------------------

SwgCuiWebBrowserWindow::~SwgCuiWebBrowserWindow()
{

}

// ----------------------------------------------------------------------

void SwgCuiWebBrowserWindow::performActivate()
{
	CuiManager::requestPointer(true);
	if(m_Widget && !s_currentURL.empty())
		SwgCuiWebBrowserManager::setURL(s_currentURL);
}

// ----------------------------------------------------------------------

void SwgCuiWebBrowserWindow::performDeactivate()
{
	CuiManager::requestPointer(false);
}

void SwgCuiWebBrowserWindow::OnButtonPressed(UIWidget *context)
{
	if(!m_Widget)
		return;
	if(context == m_Back)
	{
		m_Widget->NavigateBack();
	}
	else if(context == m_Forward)
	{
		m_Widget->NavigateForward();
	}
	else if(context == m_Go)
	{
		SwgCuiWebBrowserManager::setURL(Unicode::wideToNarrow(m_Url->GetLocalText()));
	}
	else if (context == m_Stop)
	{
		m_Widget->NavigateStop();
	}
	else if (context == m_Refresh)
	{
		m_Widget->RefreshPage();
	}
	else if (context == m_Close)
	{
		s_currentURL = m_Widget->getCurrentURL();
		SwgCuiWebBrowserManager::setURL("about:blank");
	}
}


bool SwgCuiWebBrowserWindow::OnMessage(UIWidget *context, const UIMessage & msg )
{
	UNREF(context);

	if(msg.Type == UIMessage::KeyDown)
	{
		if(msg.Keystroke == UIMessage::Enter)
		{
			UIWidget* focusWidget = getPage().GetFocusedLeafWidget();
			if(focusWidget && m_Url && focusWidget == m_Url)
			{
				SwgCuiWebBrowserManager::setURL(Unicode::wideToNarrow(m_Url->GetLocalText()));

				return false;
			}
		}
	}

	return true;
}

void SwgCuiWebBrowserWindow::SetBrowserWidget(SwgCuiWebBrowserWidget* widget)
{
	if(widget)
	{
		m_Widget = widget;
		registerMediatorObject(*m_Widget, true);
	}
}
// ======================================================================
