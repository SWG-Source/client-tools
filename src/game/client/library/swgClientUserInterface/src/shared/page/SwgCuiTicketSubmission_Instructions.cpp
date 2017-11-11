// ======================================================================
//
// SwgCuiTicketSubmission_Instructions.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission_Instructions.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCsManagerListener.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIButton.h"
#include "UIMessage.h"
#include "unicodeArchive/UnicodeArchive.h"


// ======================================================================
//
// SwgCuiTicketSubmission::Instructions
//
// ======================================================================

SwgCuiTicketSubmission::Instructions::Instructions(UIPage &page, SwgCuiTicketSubmission &ticketSubmissionMediator)
 : CuiMediator("SwgCuiTicketSubmission_Instructions", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_ticketSubmissionMediator(ticketSubmissionMediator)
 , m_bugButton(NULL)
 , m_newTicketButton(NULL)
 
 
{

	getCodeDataObject(TUIButton, m_bugButton, "buttonBug");
	registerMediatorObject(*m_bugButton, true);
	
	getCodeDataObject(TUIButton, m_newTicketButton, "buttonTicket");
	registerMediatorObject(*m_newTicketButton, true);

}

//-----------------------------------------------------------------

SwgCuiTicketSubmission::Instructions::~Instructions()
{
	m_bugButton = NULL;
	m_newTicketButton = NULL;
	
	delete m_callBack;
	m_callBack = NULL;

}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::Instructions::performActivate()
{



}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::Instructions::performDeactivate()
{

}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::Instructions::OnButtonPressed(UIWidget *context)
{
	if(context == m_bugButton)
	{
		//open the BugSubmission page
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::bugReport, Unicode::emptyString));
	}
	
	else if(context == m_newTicketButton)
	{
		m_ticketSubmissionMediator.switchToNewTicketTab();
	}
	

}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::Instructions::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//-----------------------------------------------------------------


// ======================================================================
