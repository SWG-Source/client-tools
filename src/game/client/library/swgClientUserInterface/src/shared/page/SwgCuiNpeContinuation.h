//======================================================================
//
// SwgCuiNpeContinuation.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiNpeContinuation_H
#define INCLUDED_SwgCuiNpeContinuation_H

//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNpeContinuation.h"


#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "sharedMessageDispatch/Receiver.h"

class ClientNotificationBoxMessage;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiNpeContinuation :
public CuiMediator,
public UIEventCallback
{

public:
	explicit            SwgCuiNpeContinuation (UIPage & page);

	void                OnButtonPressed         (UIWidget *context);
	
protected:
	
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();
	
private:

	                    SwgCuiNpeContinuation          ();
	                    SwgCuiNpeContinuation          (const SwgCuiNpeContinuation &);
	SwgCuiNpeContinuation &operator=                   (const SwgCuiNpeContinuation &);

	                   ~SwgCuiNpeContinuation          ();
	
	UIButton *m_previewButton;
	UIButton *m_continueButton;
	UIButton *m_returnButton;
	UIButton *m_closeButton;
};
//======================================================================

#endif
