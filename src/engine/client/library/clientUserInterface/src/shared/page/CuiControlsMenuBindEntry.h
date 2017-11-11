// ======================================================================
//
// CuiControlsMenuBindEntryBindEntry.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiControlsMenuBindEntry_H
#define INCLUDED_CuiControlsMenuBindEntry_H

// ======================================================================

#include "sharedInputMap/InputMap.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Emitter.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIText; 
class CuiMessageBox;

//-----------------------------------------------------------------

/**
* The CuiControlsMenuBindEntry is the menu which captures all input and interprets
* it in a format suitable for binding the input sequence to a command.
* 
* It is used by the CuiControlsMenu.
*/

class CuiControlsMenuBindEntry :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver,
public MessageDispatch::Emitter
{
public:

	struct Messages
	{
		static const char * const UPDATE_BINDING;
	};

	struct BindWin;
	friend struct BindWin;

	explicit                     CuiControlsMenuBindEntry (UIPage & thePage);

	// Control specific notifications.
	virtual void                 OnButtonPressed    (UIWidget *);
	virtual bool                 OnMessage          (UIWidget *, const UIMessage &);


	virtual void                 performActivate   ();
	virtual void                 performDeactivate ();

	void                         setCommand (InputMap * imap, const InputMap::Command * cmd);

	virtual void                 receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

private:
	                            ~CuiControlsMenuBindEntry ();
	                             CuiControlsMenuBindEntry ();
	                             CuiControlsMenuBindEntry (const CuiControlsMenuBindEntry & rhs);
	CuiControlsMenuBindEntry &  operator=    (const CuiControlsMenuBindEntry & rhs);


	bool                         wantsInputAt (const UIPoint & pt) const;
	void                         updateBindResultText ();

	bool                         doAddBinding ();

private:

	UIButton *                m_cancelButton;
	UIButton *                m_okButton;
 
	UIText *                  m_bindText;
	UIText *                  m_bindResultText;

	const InputMap::Command * m_command;
	InputMap *                m_inputMap;
	

	InputMap::BindInfo        m_bindInfo;

	BindWin *                 m_bindWin;

	CuiMessageBox *           m_messageBox;

	bool                      m_modalWarning;
	bool                      m_overwriteWarning;

	bool                      m_inAddBindingConfirm;

	bool                      m_oldDiscardIdenticalJoystickMotionEvents;
};

// ======================================================================

#endif
