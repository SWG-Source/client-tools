// ======================================================================
//
// SwgCuiService_TicketList.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_TicketList_H
#define INCLUDED_SwgCuiService_TicketList_H

#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "UIEventCallback.h"

class CuiMessageBox;
class CustomerServiceTicket;
class UIButton;
class UITable;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiService::TicketList : public CuiMediator
                                , public UIEventCallback
{
public:

	TicketList(UIPage &page, SwgCuiService &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage &msg);
	virtual void OnGenericSelectionChanged(UIWidget *context);

	void onDeleteTicket(const CuiMessageBox &messageBox);
	void onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &response);
	void onRequestCommentsResponseMessage(CustomerServiceManager::Messages::RequestTicketCommentsResponse::Response const &response);

	class AddComment;

private:

	enum Column
	{
		C_id,
		C_date,
		C_name,
		C_category,
		C_status,
		C_count
	};

	void sendTicketInformation();
	void setNoTicketSelectedComment();
	void populateTable();

	MessageDispatch::Callback * m_callBack;
	SwgCuiService &             m_serviceMediator;
	UIButton *                  m_deleteButton;
	UIButton *                  m_addCommentButton;
	UITable *                   m_ticketTable;
	UIText *                    m_ticketDetailsText;
	int                         m_rowToRemove;
	unsigned int                m_currentViewedTicketId;
	Unicode::String             m_ticketComments;

	// Disabled

	~TicketList();
	TicketList(TicketList const &rhs);
	TicketList &operator =(TicketList const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_TicketList_H
