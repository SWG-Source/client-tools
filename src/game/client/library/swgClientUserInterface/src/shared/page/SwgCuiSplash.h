// ======================================================================
//
// SwgCuiSplash.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiSplash_H
#define INCLUDED_SwgCuiSplash_H

class UIPage;
#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "UINotification.h"

// ======================================================================

class SwgCuiSplash :
public CuiMediator,
public UIEventCallback,
public UINotification
{
public:

	explicit                 SwgCuiSplash (UIPage & thePage);

	//- PS UI support
	virtual bool             OnMessage   (UIWidget *Context, const UIMessage & msg);
	virtual void             Notify      (UINotificationServer *notifyingObject, UIBaseObject *contextObject, SwgCuiSplash::Code notificationCode);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:
	virtual                 ~SwgCuiSplash ();
	                         SwgCuiSplash ();
	                         SwgCuiSplash (const SwgCuiSplash & rhs);
	SwgCuiSplash &           operator=    (const SwgCuiSplash & rhs);

	void                     proceed      ();

private:

	enum
	{
		m_numberOfPages = 4
	};

	UIPage *                 m_pages [m_numberOfPages];
	int                      m_currentPage;
	float                    m_timeout;
};

// ======================================================================

#endif
