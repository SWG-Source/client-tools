// ======================================================================
//
// SwgCuiService_TicketList_AddComment.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService_TicketList_AddComment.h"


#include "clientGame/ClientObject.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"

// ======================================================================
//
// SwgCuiService::TicketList::AddComment
//
// ======================================================================

namespace AddCommentNamespace
{
	unsigned int s_ticketId = 0;
};

using namespace AddCommentNamespace;

// ======================================================================
//
// SwgCuiService::TicketList::AddComment
//
// ======================================================================

//-----------------------------------------------------------------

SwgCuiService::TicketList::AddComment::AddComment(UIPage &page)
 : CuiMediator("SwgCuiService_TicketList_AddComment", page)
 , m_callBack(new MessageDispatch::Callback)
 , m_addButton(NULL)
 , m_cancelButton(NULL)
 , m_commentText(NULL)
{
	m_callBack->connect(*this, &SwgCuiService::TicketList::AddComment::onRequestTicketsResponse, static_cast<CustomerServiceManager::Messages::RequestTicketsResponse *>(0));

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));

	getCodeDataObject(TUIButton, m_addButton, "buttonAdd");
	registerMediatorObject(*m_addButton, true);

	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUIText, m_commentText, "textboxComments");
	registerMediatorObject(*m_commentText, true);

	getCodeDataObject(TUIText, m_ticketIdText, "textTicketNumber");
	registerMediatorObject(*m_ticketIdText, true);
}

//-----------------------------------------------------------------

SwgCuiService::TicketList::AddComment::~AddComment()
{
	delete m_callBack;
	m_callBack = NULL;

	m_addButton = NULL;
	m_cancelButton = NULL;
	m_commentText = NULL;
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::performActivate()
{
	// Set the ticket id

	char text[256];
	snprintf(text, sizeof(text), "%d", static_cast<int>(s_ticketId));
	m_ticketIdText->SetLocalText(Unicode::narrowToWide(text));

	m_commentText->SetFocus();
	//m_commentText->SelectAll();
	//m_commentText->MoveCaratToEndOfLine();
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::OnButtonPressed(UIWidget *context)
{
	if (context == m_addButton)
	{
		if (!m_commentText->GetLocalText().empty())
		{
			Unicode::String const &comment = m_commentText->GetLocalText();
			std::string playerName;
			Object const *object = Game::getConstPlayer();

			if (object != NULL)
			{
				ClientObject const *clientObject = object->asClientObject();

				if (clientObject != NULL)
				{
					playerName = Unicode::wideToNarrow(clientObject->getLocalizedName());
				}
			}

			// Make sure this ticket was not closed while we were typing a comment

			CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();

			CustomerServiceManager::TicketList::const_iterator iterTicketList = ticketList.begin();

			for (; iterTicketList != ticketList.end(); ++iterTicketList)
			{
				if (iterTicketList->getTicketId() == s_ticketId)
				{
					Unicode::String commentSubmittedString;
					CuiStringVariablesData data;
					data.digit_i = static_cast<int>(s_ticketId);

					if (iterTicketList->isClosed())
					{
						CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_add_fail_closed, data, commentSubmittedString);
					}
					else
					{
						CustomerServiceManager::requestAppendTicketComment(s_ticketId, playerName, comment);
						CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_add_submitted, data, commentSubmittedString);
					}

					CuiChatRoomManager::sendPrelocalizedChat(commentSubmittedString);

					break;
				}
			}

			m_commentText->Clear();

			s_ticketId = 0;

			deactivate();
		}
	}
	else if (context == m_cancelButton)
	{
		deactivate();
		s_ticketId = 0;
		m_commentText->Clear();
	}
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::setTicketId(unsigned int ticketId)
{
	s_ticketId = ticketId;
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &)
{
	if (s_ticketId != 0)
	{
		// Make sure the ticket the player is modifying still exists

		bool found = false;
		bool closed = false;
		CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();
		CustomerServiceManager::TicketList::const_iterator iterTicketList = ticketList.begin();

		for (; iterTicketList != ticketList.end(); ++iterTicketList)
		{
			unsigned int const ticketId = iterTicketList->getTicketId();

			if (ticketId == s_ticketId)
			{
				if (iterTicketList->m_closed)
				{
					closed = true;
					break;
				}
				else
				{
					found = true;
					break;
				}
			}
		}

		if (closed)
		{
			// Let the player know the ticket is now read-only

			CuiMessageBox *messageBox = CuiMessageBox::createInfoBox(CuiStringIdsCustomerService::ticket_pending_closed.localize());

			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiService::TicketList::AddComment::onDeleteTicket);
		}
		else if (!found)
		{
			// Let the player know the ticket is no longer available

			CuiMessageBox *messageBox = CuiMessageBox::createInfoBox(CuiStringIdsCustomerService::ticket_closed.localize());

			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiService::TicketList::AddComment::onDeleteTicket);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::AddComment::onDeleteTicket(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		OnButtonPressed(m_cancelButton);
	}
}

// ======================================================================
