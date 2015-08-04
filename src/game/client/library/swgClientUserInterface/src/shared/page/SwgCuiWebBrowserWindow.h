// ======================================================================
//
// SwgCuiWebBrowserWindow.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiWebBrowserWindow_H
#define INCLUDED_SwgCuiWebBrowserWindow_H

#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWidget.h"
#include "UIButton.h"
#include "UITextbox.h"
#include "UIEventCallback.h"

class UIPage;

// ======================================================================

class SwgCuiWebBrowserWindow : public CuiMediator, public UIEventCallback
{
public:
	SwgCuiWebBrowserWindow(UIPage & page);
	virtual ~SwgCuiWebBrowserWindow();

	void         SetBrowserWidget      (SwgCuiWebBrowserWidget* widget);
	virtual bool OnMessage             (UIWidget *context, const UIMessage & msg );
protected:
	virtual void performActivate       ();
	virtual void performDeactivate     ();
	virtual void OnButtonPressed       ( UIWidget *context );


private: //disabled
	SwgCuiWebBrowserWindow();
	SwgCuiWebBrowserWindow(SwgCuiWebBrowserWindow const & rhs);
	SwgCuiWebBrowserWindow& operator= (SwgCuiWebBrowserWindow const & rhs);

private:

	UIButton*  m_Forward;
	UIButton*  m_Back;
	UIText*    m_Url;
	UIButton*  m_Go;
	UIButton*  m_Stop;
	UIButton*  m_Refresh;
	UIButton*  m_Close;

	SwgCuiWebBrowserWidget*  m_Widget;
};
// ======================================================================

#endif
