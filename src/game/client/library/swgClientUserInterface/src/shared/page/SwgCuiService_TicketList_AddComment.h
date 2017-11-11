// ======================================================================
//
// SwgCuiService_AddComment.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_AddComment_H
#define INCLUDED_SwgCuiService_AddComment_H

#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "swgClientUserInterface/SwgCuiService_TicketList.h"
#include "UIEventCallback.h"

class UIButton;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiService::TicketList::AddComment : public CuiMediator
                                            , public UIEventCallback
{
public:

	AddComment(UIPage &page);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);

	void onDeleteTicket(const CuiMessageBox &messageBox);
	void onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &response);

	static void setTicketId(unsigned int ticketId);

private:
	
	MessageDispatch::Callback *m_callBack;
	UIButton *                 m_addButton;
	UIButton *                 m_cancelButton;
	UIText *                   m_commentText;
	UIText *                   m_ticketIdText;

	// Disabled

	~AddComment();
	AddComment(AddComment const &rhs);
	AddComment &operator =(AddComment const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_AddComment_H
