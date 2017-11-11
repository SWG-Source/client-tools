// ======================================================================
//
// SwgCuiTicketSubmission.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission.h"

#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCsManagerListener.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission_Instructions.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission_NewTicket.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UITabbedPane.h"


// ======================================================================
//
// SwgCuiTicketSubmission
//
// ======================================================================

SwgCuiTicketSubmission::SwgCuiTicketSubmission(UIPage &page)
 : CuiMediator("SwgCuiTicketSubmission", page)
 , UIEventCallback()
 , m_bugButton(NULL)
 , m_harassmentButton(NULL)
 , m_doneButton(NULL)
 , m_instructionsMediator(NULL)
 , m_newTicketMediator(NULL)
 , m_tabs(NULL)
{
	UIPage *subPage = NULL;

	getCodeDataObject(TUIPage, subPage, "pageInstructions");
	m_instructionsMediator = new Instructions(*subPage, *this);
	m_instructionsMediator->fetch();
	m_mediators[M_instructions] = m_instructionsMediator;

	getCodeDataObject(TUIPage, subPage, "pageSubmitTicket");
	m_newTicketMediator = new NewTicket(*subPage, *this);
	m_newTicketMediator->fetch();
	m_mediators[M_newTicket] = m_newTicketMediator;

	getCodeDataObject(TUIButton, m_bugButton, "buttonBug");
	registerMediatorObject(*m_bugButton, true);
	
	getCodeDataObject(TUIButton, m_harassmentButton, "buttonHarassment");
	registerMediatorObject(*m_harassmentButton, true);

	getCodeDataObject(TUIButton, m_doneButton, "buttonDone");
	registerMediatorObject(*m_doneButton, true);

	getCodeDataObject (TUITabbedPane, m_tabs, "tabs");
	registerMediatorObject(*m_tabs, true);

	m_tabs->SetActiveTab(-1);
	m_tabs->SetActiveTab(M_instructions);

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//-----------------------------------------------------------------

SwgCuiTicketSubmission::~SwgCuiTicketSubmission()
{
	m_instructionsMediator->release();
	m_instructionsMediator = NULL;

	m_newTicketMediator->release();
	m_newTicketMediator = NULL;

	m_bugButton = NULL;
	m_harassmentButton = NULL;
	m_doneButton = NULL;
	m_tabs = NULL;
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::performActivate()
{
	OnTabbedPaneChanged(m_tabs);

	CuiManager::requestPointer(true);
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::performDeactivate()
{
	CuiManager::requestPointer(false);

	// Deactivate all the mediators
	m_instructionsMediator->deactivate();
	m_newTicketMediator->deactivate();
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::OnButtonPressed(UIWidget *context)
{

	
	if (context == m_doneButton)
	{
		deactivate();
	}
	else if(context == m_bugButton)
	{
		//open the BugSubmission page
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::bugReport, Unicode::emptyString));
	}
	else if(context == m_harassmentButton)
	{
		//open the Harassment Report page
		if (CustomerServiceManager::getServiceCategoryList().empty())
		{
			CuiMessageBox::createInfoBox(CuiStringIdsCustomerService::server_no_connection.localize());
			
		}
		else if (CustomerServiceManager::getTicketList().size() >= static_cast<unsigned int>(CustomerServiceManager::getMaximumNumberOfCustomerServiceTicketsAllowed()))
		{
			CuiMessageBox::createInfoBox(CuiStringIdsCustomerService::harassment_ticket_limit.localize());
		}
		else
		{

			IGNORE_RETURN(CuiActionManager::performAction (CuiActions::harassmentMessage, Unicode::emptyString));
		}
			
	}
	
}

//-----------------------------------------------------------------

SwgCuiTicketSubmission *SwgCuiTicketSubmission::createInto(UIPage *parent)
{
	NOT_NULL (parent);
	UIPage * const page = NON_NULL(safe_cast<UIPage *>(parent->GetObjectFromPath("/PDA.Service.ticketSubmission", TUIPage)));
	UIPage * const dupe = NON_NULL(safe_cast<UIPage *>(page->DuplicateObject()));
	IGNORE_RETURN(parent->AddChild(dupe));
	IGNORE_RETURN(parent->MoveChild(dupe, UIBaseObject::Top));
	dupe->Link();
	dupe->Center();
	
	SwgCuiTicketSubmission * const creation = new SwgCuiTicketSubmission(*dupe);
	return creation;
}

//----------------------------------------------------------------------

void SwgCuiTicketSubmission::OnTabbedPaneChanged(UIWidget *context)
{
	if (context == m_tabs)
	{
		if ((m_tabs->GetActiveTab() >= 0) &&
		    (m_tabs->GetActiveTab() < static_cast<int>(M_count)))
		{
			// Deactivate all the pages

			for (int i = 0; i < static_cast<int>(M_count); ++i)
			{
				if (i != m_tabs->GetActiveTab())
				{
					m_mediators[i]->deactivate();
				}
			}

			m_mediators[m_tabs->GetActiveTab()]->activate();
		}
	}
}



//------------------------------------------------------------------
void SwgCuiTicketSubmission::switchToInstructionsTab()
{
	if(m_tabs != NULL)
	{
		m_tabs->SetActiveTab(M_instructions);
	}
}

//-----------------------------------------------------------------
void SwgCuiTicketSubmission::switchToNewTicketTab()
{
	if (m_tabs != NULL)
	{
		m_tabs->SetActiveTab(M_newTicket);
	}
}






// ======================================================================
