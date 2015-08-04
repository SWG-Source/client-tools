// ======================================================================
//
// SwgCuiTicketSubmission_Instructions.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTicketSubmission_Instructions_H
#define INCLUDED_SwgCuiTicketSubmission_Instructions_H

#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission.h"
#include "UIEventCallback.h"



//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiTicketSubmission::Instructions : public CuiMediator
                                   , public UIEventCallback
{
public:

	Instructions(UIPage &page, SwgCuiTicketSubmission &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	virtual void OnButtonPressed(UIWidget *context);

private:
	

	MessageDispatch::Callback  *m_callBack;
	SwgCuiTicketSubmission     &m_ticketSubmissionMediator;
	UIButton                   *m_bugButton;
	UIButton                   *m_newTicketButton;

	// Disabled

	~Instructions();
	Instructions(Instructions const &rhs);
	Instructions &operator =(Instructions const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiTicketSubmission_Instructions_H
