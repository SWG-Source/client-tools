// ======================================================================
//
// SwgCuiTicketSubmission.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTicketSubmission_H
#define INCLUDED_SwgCuiTicketSubmission_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiTicketSubmission : public CuiMediator
                    , public UIEventCallback
{
public:

	explicit SwgCuiTicketSubmission(UIPage & page);

	virtual void          performActivate();
	virtual void          performDeactivate();

	void                  switchToInstructionsTab();
	void                  switchToNewTicketTab();


	static SwgCuiTicketSubmission *createInto(UIPage *parent);

	virtual void          OnTabbedPaneChanged(UIWidget *context);

	class Instructions;
	class NewTicket;

private:

	virtual void OnButtonPressed(UIWidget *context);

	enum Mediator
	{
		M_instructions,
		M_newTicket,
		M_count
	};

	CuiMediator                *m_mediators[M_count];
	UIButton                   *m_bugButton;
	UIButton                   *m_harassmentButton;
	UIButton                   *m_doneButton;
	Instructions               *m_instructionsMediator;
	NewTicket                  *m_newTicketMediator;
	
	UITabbedPane               *m_tabs;

	// Disabled

	~SwgCuiTicketSubmission();
	SwgCuiTicketSubmission(SwgCuiTicketSubmission const &rhs);
	SwgCuiTicketSubmission &operator =(SwgCuiTicketSubmission const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgTicketSubmission_H
