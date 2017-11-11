// ======================================================================
//
// SwgCuiService.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService.h"

#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCsManagerListener.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "swgClientUserInterface/SwgCuiService_HowToSearch.h"
#include "swgClientUserInterface/SwgCuiService_KnowledgeBase.h"
#include "swgClientUserInterface/SwgCuiService_KnownIssues.h"
#include "swgClientUserInterface/SwgCuiService_TicketList.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UITabbedPane.h"

// ======================================================================
//
// SwgCuiService
//
// ======================================================================

SwgCuiService::SwgCuiService(UIPage &page)
 : CuiMediator("SwgCuiService", page)
 , UIEventCallback()
 , m_bugButton(NULL)
 , m_harassmentButton(NULL)
 , m_doneButton(NULL)
 , m_howToSearchMediator(NULL)
 , m_knowledgeBaseMediator(NULL)
 , m_knownIssuesMediator(NULL)
 , m_ticketListMediator(NULL)

 , m_tabs(NULL)
{
	UIPage *subPage = NULL;

	getCodeDataObject(TUIPage, subPage, "pageHowToSearch");
	m_howToSearchMediator = new HowToSearch(*subPage, *this);
	m_howToSearchMediator->fetch();
	m_mediators[M_howToSearch] = m_howToSearchMediator;

	getCodeDataObject(TUIPage, subPage, "pageKnow");
	m_knowledgeBaseMediator = new KnowledgeBase(*subPage, *this);
	m_knowledgeBaseMediator->fetch();
	m_mediators[M_knowledgeBase] = m_knowledgeBaseMediator;

	getCodeDataObject(TUIPage, subPage, "pageKnownIssues");
	m_knownIssuesMediator = new KnownIssues(*subPage, *this);
	m_knownIssuesMediator->fetch();
	m_mediators[M_knownIssues] = m_knownIssuesMediator;

	getCodeDataObject(TUIPage, subPage, "pageTickets");
	m_ticketListMediator = new TicketList(*subPage, *this);
	m_ticketListMediator->fetch();
	m_mediators[M_ticketList] = m_ticketListMediator;

	getCodeDataObject(TUIButton, m_bugButton, "buttonBug");
	registerMediatorObject(*m_bugButton, true);
	
	getCodeDataObject(TUIButton, m_harassmentButton, "buttonHarassment");
	registerMediatorObject(*m_harassmentButton, true);

	getCodeDataObject(TUIButton, m_doneButton, "buttonDone");
	registerMediatorObject(*m_doneButton, true);

	getCodeDataObject (TUITabbedPane, m_tabs, "tabs");
	registerMediatorObject(*m_tabs, true);

	m_tabs->SetActiveTab(-1);
	m_tabs->SetActiveTab(M_howToSearch);

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//-----------------------------------------------------------------

SwgCuiService::~SwgCuiService()
{
	m_howToSearchMediator->release();
	m_howToSearchMediator = NULL;

	m_knownIssuesMediator->release();
	m_knownIssuesMediator = NULL;

	m_knowledgeBaseMediator->release();
	m_knowledgeBaseMediator = NULL;

	m_ticketListMediator->release();
	m_ticketListMediator = NULL;

	m_bugButton = NULL;
	m_harassmentButton = NULL;
	m_doneButton = NULL;
	m_tabs = NULL;
}

//-----------------------------------------------------------------

void SwgCuiService::performActivate()
{
	OnTabbedPaneChanged(m_tabs);

	CuiManager::requestPointer(true);


	// We only need to request connections, once connected we will leave
	// the connection open and the connection server will close the
	// connection when the player logs out

	if (!CustomerServiceManager::isConnected())
	{
		CustomerServiceManager::requestConnection();
	}
}

//-----------------------------------------------------------------

void SwgCuiService::performDeactivate()
{
	CuiManager::requestPointer(false);


	// Deactivate all the mediators
	m_howToSearchMediator->deactivate();
	m_knowledgeBaseMediator->deactivate();
	m_knownIssuesMediator->deactivate();
	m_ticketListMediator->deactivate();
	
}

//-----------------------------------------------------------------

void SwgCuiService::OnButtonPressed(UIWidget *context)
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
			DEBUG_WARNING(true, ("\nat creatInfoBox"));
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

SwgCuiService *SwgCuiService::createInto(UIPage *parent)
{
	NOT_NULL (parent);
	UIPage * const page = NON_NULL(safe_cast<UIPage *>(parent->GetObjectFromPath("/PDA.Service.articleSearch", TUIPage)));
	UIPage * const dupe = NON_NULL(safe_cast<UIPage *>(page->DuplicateObject()));
	IGNORE_RETURN(parent->AddChild(dupe));
	IGNORE_RETURN(parent->MoveChild(dupe, UIBaseObject::Top));
	dupe->Link();
	dupe->Center();
	
	SwgCuiService * const creation = new SwgCuiService(*dupe);
	return creation;
}

//----------------------------------------------------------------------

void SwgCuiService::OnTabbedPaneChanged(UIWidget *context)
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
void SwgCuiService::switchToKnowledgeBaseTab()
{
	if(m_tabs != NULL)
	{
		m_tabs->SetActiveTab(M_knowledgeBase);
	}
}

//-------------------------------------------------------------------
void SwgCuiService::switchToKnownIssuesTab()
{
	if(m_tabs != NULL)
	{
		m_tabs->SetActiveTab(M_knownIssues);
	}
}


// ======================================================================
