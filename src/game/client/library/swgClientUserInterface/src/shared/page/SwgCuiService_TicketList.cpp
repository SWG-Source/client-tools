// ======================================================================
//
// SwgCuiService_TicketList.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService_TicketList.h"

#include "clientGame/CustomerServiceManager.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CustomerServiceComment.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiService_TicketList_AddComment.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIPage.h"
#include <vector>

// ======================================================================

SwgCuiService::TicketList::TicketList(UIPage &page, SwgCuiService &serviceMediator)
 : CuiMediator("SwgCuiService_TicketList", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_serviceMediator(serviceMediator)
 , m_deleteButton(NULL)
 , m_addCommentButton(NULL)
 , m_ticketTable(NULL)
 , m_ticketDetailsText(NULL)
 , m_rowToRemove(-1)
 , m_currentViewedTicketId(0)
 , m_ticketComments()
{
	m_callBack->connect(*this, &SwgCuiService::TicketList::onRequestTicketsResponse, static_cast<CustomerServiceManager::Messages::RequestTicketsResponse *>(0));
	m_callBack->connect(*this, &SwgCuiService::TicketList::onRequestCommentsResponseMessage, static_cast<CustomerServiceManager::Messages::RequestTicketCommentsResponse *>(0));

	getCodeDataObject(TUIButton, m_deleteButton, "buttonDelete");
	registerMediatorObject(*m_deleteButton, true);

	getCodeDataObject(TUIButton, m_addCommentButton, "buttonEdit");
	registerMediatorObject(*m_addCommentButton, true);

	getCodeDataObject(TUITable, m_ticketTable, "table");
	registerMediatorObject(*m_ticketTable, true);

	getCodeDataObject(TUIText, m_ticketDetailsText, "textDetails");
	registerMediatorObject(*m_ticketDetailsText, true);
	m_ticketDetailsText->Clear();
	m_ticketDetailsText->SetMaximumCharacters(50 * 1024);

	// Clear out any table data

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_ticketTable->GetTableModel());

	if (tableModel != NULL)
	{
		tableModel->ClearTable();
	}

	// Disable the comment button by default because a ticket needs
	// to be selected in order to add a comment

	m_addCommentButton->SetEnabled(false);
	m_deleteButton->SetEnabled(false);
}

//-----------------------------------------------------------------

SwgCuiService::TicketList::~TicketList()
{
	delete m_callBack;
	m_callBack = NULL;

	m_deleteButton = NULL;
	m_addCommentButton = NULL;
	m_ticketTable = NULL;
	m_ticketDetailsText = NULL;
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::performActivate()
{
	m_ticketComments.clear();

	setNoTicketSelectedComment();

	populateTable();
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::OnButtonPressed(UIWidget *context)
{
	if (context == m_deleteButton)
	{
		m_rowToRemove = -1;

		UITableModel *tableModel = NULL;

		for (unsigned int row = 0; row < static_cast<unsigned int>(m_ticketTable->GetRowCount()); ++row)
		{
			if (m_ticketTable->IsRowSelected(static_cast<long>(row)))
			{
				tableModel = m_ticketTable->GetTableModel();

				if (tableModel != NULL)
				{
					// Mark this row for removal

					m_rowToRemove = row;
					break;
				}
			}
		}

		CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();

		if (   (tableModel != NULL)
		    && (m_rowToRemove >= 0)
		    && (m_rowToRemove < static_cast<int>(ticketList.size())))
		{
			// Get the ticket number

			unsigned int const logicalRow = static_cast<unsigned int>(tableModel->GetLogicalDataRowIndex(static_cast<long>(m_rowToRemove)));
			DEBUG_FATAL((logicalRow < 0) || (logicalRow >= ticketList.size()), ("Invalid row in ticket list: %d", logicalRow));

			unsigned int const ticketId = ticketList[logicalRow].getTicketId();

			// Make sure the user wants to delete the ticket

			CuiStringVariablesData data;
			data.digit_i = static_cast<int>(ticketId);
			Unicode::String deleteTicketString;
			CuiStringVariablesManager::process(CuiStringIdsCustomerService::ticket_delete_confirmation, data, deleteTicketString);

			CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(deleteTicketString);

			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiService::TicketList::onDeleteTicket);
		}
	}
	else if (context == m_addCommentButton)
	{
		if (m_currentViewedTicketId != 0)
		{
			// Send the add comment dialog the selected ticket

			SwgCuiService::TicketList::AddComment::setTicketId(m_currentViewedTicketId);

			IGNORE_RETURN(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_TicketListAddComment));
		}
		else
		{
			DEBUG_WARNING(true, ("Trying to add a comment to a ticket with id 0."));
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_ticketTable)
	{
		if (m_ticketTable->GetLastSelectedRow() != -1 &&
			m_ticketTable->GetRowCount() > 0)
		{
			m_deleteButton->SetEnabled(true);
		}
		else
		{
			m_deleteButton->SetEnabled(false);
		}
	}
}

//-----------------------------------------------------------------

bool SwgCuiService::TicketList::OnMessage(UIWidget *context, const UIMessage &msg)
{
	bool result = true;

	if (context == m_ticketTable)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			int lastSelectedRow = m_ticketTable->GetLastSelectedRow();

			UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_ticketTable->GetTableModel());
			CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();

			if ((tableModel != NULL) &&
			    (lastSelectedRow >= 0) &&
			    (lastSelectedRow < m_ticketTable->GetRowCount()) &&
			    (lastSelectedRow < static_cast<int>(ticketList.size())))
			{
				unsigned int const logicalRow = static_cast<unsigned int>(tableModel->GetLogicalDataRowIndex(static_cast<long>(lastSelectedRow)));
				DEBUG_FATAL((logicalRow < 0) || (logicalRow >= ticketList.size()), ("Invalid row in ticket list: %d", logicalRow));

				CustomerServiceTicket const &customerServiceTicket = ticketList[logicalRow];
				
				m_currentViewedTicketId = customerServiceTicket.getTicketId();

				// Build a string of the default ticket comment

				m_ticketComments.clear();

				Unicode::String ticketIdString;
				CuiStringVariablesData data;
				data.digit_i = static_cast<int>(customerServiceTicket.getTicketId());

				CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_ticket_id, data, ticketIdString);

				m_ticketComments += ticketIdString;
				m_ticketComments += Unicode::narrowToWide("\n");
				m_ticketComments += Unicode::narrowToWide("\n");
				m_ticketComments += customerServiceTicket.getDetails();

				m_ticketDetailsText->SetLocalText(m_ticketComments);

				m_addCommentButton->SetEnabled(!customerServiceTicket.isClosed());
				m_deleteButton->SetEnabled(true);

				// Request any comments for this ticket

				CustomerServiceManager::requestTicketComments(customerServiceTicket.getTicketId());

				result = false;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &)
{
	populateTable();
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::onDeleteTicket(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_ticketTable->GetTableModel());
		CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();

		if ((tableModel != NULL) &&
		    (m_rowToRemove >= 0) &&
			(m_rowToRemove < static_cast<int>(ticketList.size())))
		{
			// Send a request to cancel the ticket

			unsigned int const logicalRow = static_cast<unsigned int>(tableModel->GetLogicalDataRowIndex(static_cast<long>(m_rowToRemove)));
			DEBUG_FATAL((logicalRow < 0) || (logicalRow >= ticketList.size()), ("Invalid row in ticket list: %d", logicalRow));

			unsigned int const ticketId = ticketList[logicalRow].getTicketId();

			CustomerServiceManager::requestTicketCancellation(ticketId);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::onRequestCommentsResponseMessage(CustomerServiceManager::Messages::RequestTicketCommentsResponse::Response const &response)
{
	// If there is any comments, append them

	CustomerServiceManager::TicketCommentsList const &ticketCommentsList = CustomerServiceManager::getTicketCommentsList();

	if (ticketCommentsList.size() > 0)
	{
		// Display the original ticket text

		m_ticketDetailsText->Clear();
		m_ticketDetailsText->AppendLocalText(m_ticketComments);

		bool const success = response;

		if (success)
		{
			CustomerServiceManager::TicketCommentsList::const_iterator iterTicketComments = ticketCommentsList.begin();

			for (; iterTicketComments != ticketCommentsList.end(); ++iterTicketComments)
			{
				CustomerServiceComment const &customerServiceComment = (*iterTicketComments);

				m_ticketDetailsText->AppendLocalText(Unicode::narrowToWide("\n"));
				m_ticketDetailsText->AppendLocalText(Unicode::narrowToWide("\n"));

				// Comment Id

				{
					Unicode::String commentIdString;
					CuiStringVariablesData data;
					data.digit_i = customerServiceComment.getCommentId();
					CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_id, data, commentIdString);

					m_ticketDetailsText->AppendLocalText(commentIdString);
					m_ticketDetailsText->AppendLocalText(Unicode::narrowToWide("\n"));
				}

				// From

				{
					Unicode::String commentFromString;
					CuiStringVariablesData data;
					data.targetName = Unicode::narrowToWide(customerServiceComment.getCommentorName());
					CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_from, data, commentFromString);

					m_ticketDetailsText->AppendLocalText(commentFromString);
					m_ticketDetailsText->AppendLocalText(Unicode::narrowToWide("\n"));
				}

				// Comment

				m_ticketDetailsText->AppendLocalText(customerServiceComment.getComment());
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::setNoTicketSelectedComment()
{
	Unicode::String text;

	if (CustomerServiceManager::getTicketList().empty())
	{
		text += CuiStringIdsCustomerService::ticket_none.localize();
	}
	else
	{
		text += CuiStringIdsCustomerService::ticket_click.localize();
	}

	text += Unicode::narrowToWide("\n\n");
	text += CuiStringIdsCustomerService::ticket_usage.localize();

	m_ticketDetailsText->SetLocalText(text);
}

//-----------------------------------------------------------------

void SwgCuiService::TicketList::populateTable()
{
	m_ticketTable->SelectRow(-1);
	m_deleteButton->SetEnabled(false);
	CustomerServiceManager::TicketList const &ticketList = CustomerServiceManager::getTicketList();

	bool currentTicketStillExists = false;

	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_ticketTable->GetTableModel());

	if (tableModel != NULL)
	{
		tableModel->ClearTable();

		std::vector<CustomerServiceTicket>::const_iterator iterTickets = ticketList.begin();

		for (; iterTickets != ticketList.end(); ++iterTickets)
		{
			if (iterTickets->getTicketId() == m_currentViewedTicketId)
			{
				m_addCommentButton->SetEnabled(!iterTickets->isClosed());
			}

			for (int column = 0; column < static_cast<int>(C_count); ++column)
			{
				switch (column)
				{
					case C_id:
						{
							if (!currentTicketStillExists &&
							    (m_currentViewedTicketId == iterTickets->getTicketId()))
							{
								currentTicketStillExists = true;
							}

							Unicode::String ticketIdString;

							if (iterTickets->isRead())
							{
								char text[256];
								snprintf(text, sizeof(text), "%d", iterTickets->getTicketId());
								ticketIdString = Unicode::narrowToWide(text);
							}
							else
							{
								CuiStringVariablesData data;
								data.digit_i = static_cast<int>(iterTickets->getTicketId());
								CuiStringVariablesManager::process(CuiStringIdsCustomerService::ticket_unread, data, ticketIdString);
							}

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, ticketIdString));
						}
						break;
					case C_date:
						{
							Unicode::String timeStamp;
							IGNORE_RETURN(CuiUtils::FormatDate(timeStamp, static_cast<size_t>(iterTickets->getModifiedDate())));

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, timeStamp));
						}
						break;
					case C_name:
						{
							IGNORE_RETURN(tableModel->AppendCell(column, NULL, Unicode::narrowToWide(iterTickets->getCharacterName().c_str())));
						}
						break;
					case C_category:
						{
							Unicode::String category;
							Unicode::String subCategory;

							if (!CustomerServiceManager::getCategoryText(iterTickets->getCategoryId(), category))
							{
								DEBUG_WARNING(true, ("Unable to find ticket category."));
							}

							if (!CustomerServiceManager::getSubCategoryText(iterTickets->getCategoryId(), iterTickets->getSubCategoryId(), subCategory))
							{
								DEBUG_WARNING(true, ("Unable to find ticket sub-category."));
							}

							Unicode::String label;

							label = category;
							label += Unicode::narrowToWide("->");
							label += subCategory;

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, label));
						}
						break;
					case C_status:
						{
							Unicode::String text;

							if (iterTickets->isClosed())
							{
								text = CuiStringIdsCustomerService::status_closed.localize();
							}
							else
							{
								text = CuiStringIdsCustomerService::status_open.localize();
							}

							IGNORE_RETURN(tableModel->AppendCell(column, NULL, text));
						}
						break;
					default:
						{
							DEBUG_FATAL(true, ("Invalid column specified."));
						}
						break;
				}
			}
		}
	}

	if (!currentTicketStillExists)
	{
		m_addCommentButton->SetEnabled(false);
		m_currentViewedTicketId = 0;
		setNoTicketSelectedComment();
	}
}

// ======================================================================
