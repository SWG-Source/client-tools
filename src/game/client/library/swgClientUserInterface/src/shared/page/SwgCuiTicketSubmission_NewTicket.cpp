// ======================================================================
//
// SwgCuiTicketSubmission_NewTicket.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTicketSubmission_NewTicket.h"

#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "UIButton.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIText.h"
#include <vector>

// ======================================================================
//
// SwgCuiTicketSubmission_NewTicketNamespace
//
// ======================================================================

namespace SwgCuiTicketSubmission_NewTicketNamespace
{
	bool s_ticketSubmissionEnabled = true;
};

using namespace SwgCuiTicketSubmission_NewTicketNamespace;

// ======================================================================
//
// SwgCuiTicketSubmission_NewTicket
//
// ======================================================================

//-----------------------------------------------------------------

SwgCuiTicketSubmission::NewTicket::NewTicket(UIPage &page, SwgCuiTicketSubmission &ticketSubmissionMediator)
 : CuiMediator("SwgCuiTicketSubmission_NewTicket", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_ticketSubmissionMediator(ticketSubmissionMediator)
 , m_submitButton(NULL)
 , m_topicComboBox(NULL)
 , m_subTopicComboBox(NULL)
 , m_commentTextBox(NULL)
 , m_submitTimer(0.0f)
 , m_enableWidgets(false)
{
	m_callBack->connect(*this, &SwgCuiTicketSubmission::NewTicket::onRequestTicketsResponse, static_cast<CustomerServiceManager::Messages::RequestTicketsResponse *>(0));
	m_callBack->connect(*this, &SwgCuiTicketSubmission::NewTicket::onRequestTicketCategoriesResponse, static_cast<CustomerServiceManager::Messages::RequestTicketCategoriesResponse *>(0));

	getCodeDataObject(TUIButton, m_submitButton, "buttonSubmit");
	registerMediatorObject(*m_submitButton, true);

	getCodeDataObject(TUIComboBox, m_topicComboBox, "comboTopic");
	registerMediatorObject(*m_topicComboBox, true);
	m_topicComboBox->Clear();
	m_topicComboBox->SetSelectedIndex(-1);

	getCodeDataObject(TUIComboBox, m_subTopicComboBox, "comboSubtopic");
	registerMediatorObject(*m_subTopicComboBox, true);
	m_subTopicComboBox->Clear();
	m_subTopicComboBox->SetSelectedIndex(-1);

	getCodeDataObject(TUIText, m_commentTextBox, "textboxComments");
	registerMediatorObject(*m_commentTextBox, true);
	m_commentTextBox->Clear();

	setIsUpdating(true);
}

//-----------------------------------------------------------------

SwgCuiTicketSubmission::NewTicket::~NewTicket()
{
	delete m_callBack;
	m_callBack = NULL;

	m_submitButton = NULL;
	m_topicComboBox = NULL;
	m_subTopicComboBox = NULL;
	m_commentTextBox = NULL;
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::performActivate()
{
	m_enableWidgets = false;

	m_commentTextBox->SetLocalText(Unicode::emptyString);



	if (CustomerServiceManager::getServiceCategoryList().empty())
	{

		m_topicComboBox->Clear();
		IGNORE_RETURN(m_topicComboBox->AddItem(CuiStringIdsCustomerService::querying_server.localize(), std::string("")));
		m_topicComboBox->SetSelectedIndex(0);
	}
	else if (CustomerServiceManager::getTicketList().size() >= static_cast<size_t>(CustomerServiceManager::getMaximumNumberOfCustomerServiceTicketsAllowed()))
	{

		m_enableWidgets = false;

		m_topicComboBox->Clear();
		IGNORE_RETURN(m_topicComboBox->AddItem(CuiStringIdsCustomerService::category_ticket_limit.localize(), std::string("")));
		m_commentTextBox->SetLocalText(CuiStringIdsCustomerService::ticket_limit_message.localize());
		m_topicComboBox->SetSelectedIndex(0);
	}

	else if (m_topicComboBox->GetItemCount() <= 1)
		{
			populateCategories();
		}

		// If there is some valid categories, allow ticket submission

		if (m_topicComboBox->GetItemCount() > 1)
		{
			m_enableWidgets = true;
		}


	m_topicComboBox->SetEnabled(m_enableWidgets);
	m_subTopicComboBox->SetEnabled(m_enableWidgets);
	m_commentTextBox->SetEnabled(m_enableWidgets);

	if (!m_enableWidgets)
	{
		m_subTopicComboBox->Clear();
		m_subTopicComboBox->SetSelectedIndex(-1);
	}
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::OnGenericSelectionChanged(UIWidget *context)
{

	if (context == m_topicComboBox)
	{
		populateSubCategories();
		std::string categoryString;
		m_topicComboBox->GetSelectedIndexName(categoryString);
		int const category = atoi(categoryString.c_str());
		
		setTicketSubmissionEnabled(true);
		if(isCommunityStandardsCategory(category))
		{
		
			setTicketSubmissionEnabled(false);
			IGNORE_RETURN(CuiActionManager::performAction (CuiActions::harassmentMessageFromKBSearch, Unicode::emptyString));
			
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::OnButtonPressed(UIWidget *context)
{
	if (context == m_submitButton)
	{
		if (   (CustomerServiceManager::getTicketList().size() < static_cast<size_t>(CustomerServiceManager::getMaximumNumberOfCustomerServiceTicketsAllowed()))
		    && !Unicode::getTrim(m_commentTextBox->GetLocalText()).empty())
		{
			Object const *object = Game::getConstPlayer();

			if (object != NULL)
			{
				ClientObject const *clientObject = dynamic_cast<ClientObject const *>(object);

				if (clientObject != NULL)
				{
					std::string playerName(Unicode::wideToNarrow(clientObject->getLocalizedName()));

					std::string categoryString;
					m_topicComboBox->GetSelectedIndexName(categoryString);
					unsigned int const category = static_cast<unsigned int>(atoi(categoryString.c_str()));

					std::string subCategoryString;
					m_subTopicComboBox->GetSelectedIndexName(subCategoryString);
					unsigned int const subCategory = static_cast<unsigned int>(atoi(subCategoryString.c_str()));
					
					Unicode::String details(Unicode::getTrim(m_commentTextBox->GetLocalText()));

					CustomerServiceManager::requestTicketCreation(playerName, category, subCategory, details, Unicode::emptyString);

					// Set the ticket wait timer

					m_submitTimer = 60.0f;

					// Let the user know the ticket was submitted

					CuiChatRoomManager::sendPrelocalizedChat(CuiStringIdsCustomerService::create_ticket_submitted.localize());

					// Clear the comments box so the player can't spam messages

					m_commentTextBox->Clear();
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::populateCategories()
{
	if (!CustomerServiceManager::getServiceCategoryList().empty())
	{
		// See if we need to populate the categories

		setComboBoxItems(*m_topicComboBox, CustomerServiceManager::getServiceCategoryList());

		m_commentTextBox->SetLocalText(Unicode::emptyString);

		// Set the proper category and sub-category items

		OnGenericSelectionChanged(m_topicComboBox);
	}
	else
	{
		m_topicComboBox->Clear();
		m_topicComboBox->AddItem(CuiStringIdsCustomerService::no_categories_found.localize(), "");
		m_topicComboBox->SetSelectedIndex(0);

		m_subTopicComboBox->Clear();
		m_subTopicComboBox->SetSelectedIndex(-1);

		m_commentTextBox->SetLocalText(CuiStringIdsCustomerService::server_no_connection.localize());
	}
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::populateSubCategories()
{
	if ((m_topicComboBox->GetItemCount() > 0) &&
		(m_topicComboBox->GetItemCount() == static_cast<int>(CustomerServiceManager::getServiceCategoryList().size())))
	{
		unsigned int const selectedIndex = static_cast<unsigned int>(m_topicComboBox->GetSelectedIndex());

		std::vector<CustomerServiceCategory> const &subCategories = CustomerServiceManager::getServiceCategoryList()[selectedIndex].getSubCategories();

		setComboBoxItems(*m_subTopicComboBox, subCategories);
	}
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::onRequestTicketsResponse(CustomerServiceManager::Messages::RequestTicketsResponse::Response const &)
{
	performActivate();
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::onRequestTicketCategoriesResponse(CustomerServiceManager::Messages::RequestTicketCategoriesResponse::Response const &)
{
	performActivate();
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::setComboBoxItems(UIComboBox &comboBox, stdvector<CustomerServiceCategory>::fwd const &items) const
{
	comboBox.Clear();

	std::vector<CustomerServiceCategory>::const_iterator iterCategoryList = items.begin();

	for (; iterCategoryList != items.end(); ++iterCategoryList)
	{
		Unicode::String const & categoryName = iterCategoryList->m_categoryName;

		char categoryId[256];
		snprintf(categoryId, sizeof(categoryId), "%d", iterCategoryList->m_categoryId);

		IGNORE_RETURN(comboBox.AddItem(categoryName, categoryId));
	}

	if (comboBox.GetItemCount() > 0)
	{
		comboBox.SetSelectedIndex(0);
	}
}
//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::update(float deltaTimeSecs)
{
	if (m_submitTimer > 0.0f)
	{
		m_submitTimer -= deltaTimeSecs;
	}

	setSubmitButtonStatus();
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::setSubmitButtonStatus()
{
	bool bTooManyTicketsAlready = !(CustomerServiceManager::getTicketList().size() < static_cast<size_t>(CustomerServiceManager::getMaximumNumberOfCustomerServiceTicketsAllowed()));

	
	if (isTicketSubmissionEnabled() 
		&& !bTooManyTicketsAlready
		&& m_enableWidgets
	    && (m_submitTimer <= 0.0f))
	{
		m_submitButton->SetEnabled(true);
	}
	else
	{
		m_submitButton->SetEnabled(false);
	}

}

//-----------------------------------------------------------------

bool SwgCuiTicketSubmission::NewTicket::isCommunityStandardsCategory(int category) const
{
	
	CustomerServiceManager::TicketCategoryList const &ticketCategoryList = CustomerServiceManager::getServiceCategoryList();
	CustomerServiceManager::TicketCategoryList::const_iterator iterCategoryList = ticketCategoryList.begin();


	for (; iterCategoryList != ticketCategoryList.end(); ++iterCategoryList)
	{
		CustomerServiceCategory const &customerServiceCategory = (*iterCategoryList);
		Unicode::String const & categoryName = customerServiceCategory.getCategoryName();

		if (categoryName == CuiStringIdsCustomerService::category_community_standard.localize())
		{
			if(category == customerServiceCategory.getCategoryId())
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------

void SwgCuiTicketSubmission::NewTicket::setTicketSubmissionEnabled(const bool enabled)
{
	s_ticketSubmissionEnabled = enabled;
}

//-----------------------------------------------------------------

bool SwgCuiTicketSubmission::NewTicket::isTicketSubmissionEnabled()
{
	return s_ticketSubmissionEnabled;
}

// ======================================================================
