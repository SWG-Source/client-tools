// ======================================================================
//
// SwgCuiLoginScreen.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiLoginScreen_H
#define INCLUDED_SwgCuiLoginScreen_H

#include "sharedMessageDispatch/Receiver.h"
#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class UIButton;
class UIPage;
class UITextbox;
class CuiMessageBox;

namespace MessageDispatch
{
	class Callback;
};

// ======================================================================

class SwgCuiLoginScreen :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	explicit                SwgCuiLoginScreen   (UIPage & page);
	virtual void            OnButtonPressed     (UIWidget *Context );
	virtual void            receiveMessage      (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                    onAvatarListChanged (bool);
	void                    update              (float deltaTimeSecs);
	void                    ok                  ();

protected:
	virtual void            performActivate     ();
	virtual void            performDeactivate   ();

private:
	virtual                ~SwgCuiLoginScreen   ();
	                        SwgCuiLoginScreen   (const SwgCuiLoginScreen & rhs);
	SwgCuiLoginScreen &     operator=           (const SwgCuiLoginScreen & rhs);

	UIButton *                  m_cancelButton;
	UIButton *                  m_okButton;
	UIButton *                  m_devButton;

	UITextbox *                 m_usernameTextbox;
	UITextbox *                 m_passwordTextbox;

	CuiMessageBox *             m_messageBox;

	bool                        m_autoConnected;

	MessageDispatch::Callback * m_callback;

	bool                        m_proceed;

	UIPage *                    m_pageSession;
	UIPage *                    m_pageNormal;

	bool                        m_connecting;
};

// ======================================================================

#endif
