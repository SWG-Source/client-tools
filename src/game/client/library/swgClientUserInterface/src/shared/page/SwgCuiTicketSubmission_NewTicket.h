// ======================================================================
//
// SwgCuiTicketSubmission_NewTicket.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTicketSubmission_NewTicket_H
#define INCLUDED_SwgCuiTicketSubmission_NewTicket_H

#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission.h"
#include "UIEventCallback.h"

class UIButton;
class UIComboBox;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiTicketSubmission::NewTicket : public CuiMediator
                               , public UIEventCallback
{
public:

	NewTicket(UIPage &page, SwgCuiTicketSubmission &ticketSubmissionMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnGenericSelectionChanged(UIWidget *context);
	virtual void OnButtonPressed(UIWidget *context);

	void         onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &response);
	void         onRequestTicketCategoriesResponse(CustomerServiceManager::Messages::RequestTicketCategoriesResponse::Response const &response);

	virtual void update(float deltaTimeSecs);

	static void           setTicketSubmissionEnabled(const bool enabled);
	static bool           isTicketSubmissionEnabled();

private:
	
	void setComboBoxItems(UIComboBox &comboBox, stdvector<CustomerServiceCategory>::fwd const &items) const;
	void populateCategories();
	void populateSubCategories();
	void setSubmitButtonStatus();

	MessageDispatch::Callback *m_callBack;
	SwgCuiTicketSubmission &            m_ticketSubmissionMediator;
	UIButton *                 m_submitButton;
	UIComboBox *               m_topicComboBox;
	UIComboBox *               m_subTopicComboBox;
	UIText *                   m_commentTextBox;
	float                      m_submitTimer;

	
	bool                       m_enableWidgets;

	bool isCommunityStandardsCategory(int category) const;

	// Disabled

	~NewTicket();
	NewTicket(NewTicket const &rhs);
	NewTicket &operator =(NewTicket const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiTicketSubmission_NewTicket_H
