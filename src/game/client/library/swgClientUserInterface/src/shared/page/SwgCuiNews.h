//======================================================================
//
// SwgCuiNews.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiNews_H
#define INCLUDED_SwgCuiNews_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIImage;
class UIText;

// ======================================================================

class SwgCuiNews :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiNews        (UIPage & page);

	virtual void             OnButtonPressed   (UIWidget *context);

//	virtual void             Notify            (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:
	virtual                 ~SwgCuiNews ();
	                         SwgCuiNews ();
	                         SwgCuiNews (const SwgCuiNews & rhs);
	SwgCuiNews &             operator=         (const SwgCuiNews & rhs);

private:

	UIButton *                    m_buttonBack;
	UIButton *                    m_buttonNext;
	UIButton *                    m_buttonIntro;

	UIText *                      m_text;
	UIImage *                     m_image;
};

//======================================================================

#endif
