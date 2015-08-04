// ======================================================================
//
// SwgCuiHarassmentReport.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHarassmentReport.h"

#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextbox.h"

// ======================================================================
//
// SwgCuiHarassmentReportNamespace
//
// ======================================================================

namespace SwgCuiHarassmentReportNamespace
{
	float s_verifyNameTimeoutTime = 30.0f * 1.0f;
	float s_dotTime = 0.5f;
	Unicode::String s_dot(Unicode::narrowToWide("."));
};

using namespace SwgCuiHarassmentReportNamespace;

// ======================================================================
//
// SwgCuiHarassmentReport
//
// ======================================================================

//-----------------------------------------------------------------
SwgCuiHarassmentReport::SwgCuiHarassmentReport(UIPage &page)
 : CuiMediator("SwgCuiHarassmentReport", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_submitButton(NULL)
 , m_cancelButton(NULL)
 , m_verifyNameButton(NULL)
 , m_playerNameTextBox(NULL)
 , m_commentText(NULL)
 , m_validPlayerName()
 , m_verifyingName(false)
 , m_dotTimer(0.0f)
 , m_dots()
 , m_verifyNameTimer(0.0f)
{
	m_callBack->connect(*this, &SwgCuiHarassmentReport::onVerifyPlayerNameResponse, static_cast<CommunityManager::Messages::RequestVerifyPlayerNameResponse *>(0));

	getCodeDataObject(TUIButton, m_submitButton, "buttonSubmit");
	registerMediatorObject(*m_submitButton, true);

	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUIButton, m_verifyNameButton, "buttonVerifyName");
	registerMediatorObject(*m_verifyNameButton, true);

	getCodeDataObject(TUIText, m_verifyNameLabel, "textVerifyName");
	registerMediatorObject(*m_verifyNameLabel, true);

	getCodeDataObject(TUITextbox, m_playerNameTextBox, "textBoxPlayerName");
	registerMediatorObject(*m_playerNameTextBox, true);
	m_playerNameTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUIText, m_commentText, "textComments");
	registerMediatorObject(*m_commentText, true);
	m_commentText->Clear();

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//-----------------------------------------------------------------
SwgCuiHarassmentReport::~SwgCuiHarassmentReport()
{
	delete m_callBack;
	m_callBack = NULL;

	m_submitButton = NULL;
	m_cancelButton = NULL;
	m_verifyNameButton = NULL;
	m_playerNameTextBox = NULL;
	m_commentText = NULL;
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::performActivate()
{
	m_playerNameTextBox->SetFocus();
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::performDeactivate()
{
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::OnButtonPressed(UIWidget *context)
{
	if (context == m_submitButton)
	{
		if (   !m_validPlayerName.empty()
		    && (m_validPlayerName == Unicode::getTrim(m_playerNameTextBox->GetLocalText())))
		{
			Object const *object = Game::getConstPlayer();

			if (object != NULL)
			{
				ClientObject const *clientObject = dynamic_cast<ClientObject const *>(object);

				if (clientObject != NULL)
				{
					std::string const playerName(Unicode::wideToNarrow(clientObject->getLocalizedName()));
					int category = 0;
					int subCategory = 0;
					getHarassmentCategory(category, subCategory);
					Unicode::String const details = Unicode::getTrim(m_commentText->GetLocalText());
					Unicode::String const harassingPlayerName = m_validPlayerName;

					CustomerServiceManager::requestTicketCreation(playerName, category, subCategory, details, harassingPlayerName);
					
					// Clear the text fields

					m_playerNameTextBox->SetLocalText(Unicode::emptyString);
					m_commentText->SetLocalText(Unicode::emptyString);
					m_validPlayerName = Unicode::emptyString;

					close();
				}
			}
		}
		else
		{
			CuiMessageBox::createInfoBox(CuiStringIdsCustomerService::harassment_verify_needed.localize());
		}
	}
	else if (context == m_verifyNameButton)
	{
		verifyName();
	}
	else if (context == m_cancelButton)
	{
		close();
	}
}

//-----------------------------------------------------------------

bool SwgCuiHarassmentReport::OnMessage(UIWidget *context, const UIMessage & msg)
{
	bool result = true;

	if (msg.Keystroke == UIMessage::Enter)
	{
		if (context == m_playerNameTextBox)
		{
			result = false;

			if (msg.Type == UIMessage::KeyUp)
			{
				verifyName();
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::onVerifyPlayerNameResponse(CommunityManager::Messages::RequestVerifyPlayerNameResponse::Response const &response)
{
	if (m_verifyingName)
	{
		setIsUpdating(false);
		m_verifyingName = false;

		if (response.first)
		{
			m_validPlayerName = response.second;
			m_verifyNameLabel->SetLocalText(CuiStringIdsCustomerService::harassment_verify_success.localize());
		}
		else
		{
			m_verifyNameLabel->SetLocalText(CuiStringIdsCustomerService::harassment_verify_failed.localize());
		}
	}
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::getHarassmentCategory(int &category, int &subCategory) const
{
	category = 0;
	subCategory = 0;

	CustomerServiceManager::TicketCategoryList const &ticketCategoryList = CustomerServiceManager::getServiceCategoryList();
	CustomerServiceManager::TicketCategoryList::const_iterator iterCategoryList = ticketCategoryList.begin();

	// Find the category

	for (; iterCategoryList != ticketCategoryList.end(); ++iterCategoryList)
	{
		CustomerServiceCategory const &customerServiceCategory = (*iterCategoryList);
		Unicode::String const & categoryName = customerServiceCategory.getCategoryName();

		if (categoryName == CuiStringIdsCustomerService::category_community_standard.localize())
		{
			category = customerServiceCategory.getCategoryId();

			// Find the sub-category

			std::vector<CustomerServiceCategory>::const_iterator iterSubCategoryList = customerServiceCategory.getSubCategories().begin();

			for (; iterSubCategoryList != customerServiceCategory.getSubCategories().end(); ++iterSubCategoryList)
			{
				CustomerServiceCategory const &customerServiceSubCategory = (*iterSubCategoryList);
				Unicode::String const & subCategoryName = customerServiceSubCategory.getCategoryName();

				if (subCategoryName == CuiStringIdsCustomerService::category_harassment.localize())
				{
					subCategory = customerServiceSubCategory.getCategoryId();
					break;
				}
			}

			break;
		}
	}

	DEBUG_FATAL((category == 0) || (subCategory == 0), ("The data changed for the categories and subcategories, change the strings above to match as needed."));
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::verifyName()
{
	m_verifyingName = true;
	m_verifyNameTimer = 0.0f;
	m_dotTimer = 0.0f;
	m_dots = Unicode::emptyString;
	CommunityManager::requestVerifyPlayerName(m_playerNameTextBox->GetLocalText());
	setIsUpdating(true);
}

//-----------------------------------------------------------------
void SwgCuiHarassmentReport::update(float deltaTimeSecs)
{
	m_verifyNameTimer += deltaTimeSecs;
	m_dotTimer -= deltaTimeSecs;

	if (m_dotTimer <= 0.0f)
	{
		m_dotTimer = s_dotTime;

		static Unicode::String dot(Unicode::narrowToWide("."));

		if (m_dots.length() >= 3)
		{
			m_dots = Unicode::emptyString;
		}
		else
		{
			m_dots.append(dot);
		}

		m_verifyNameLabel->SetLocalText(CuiStringIdsCustomerService::harassment_waiting_for_response.localize() + m_dots);
	}

	if (m_verifyNameTimer > s_verifyNameTimeoutTime)
	{
		onVerifyPlayerNameResponse(CommunityManager::Messages::RequestVerifyPlayerNameResponse::Response(std::make_pair(false, Unicode::emptyString)));
	}
}

// ======================================================================
