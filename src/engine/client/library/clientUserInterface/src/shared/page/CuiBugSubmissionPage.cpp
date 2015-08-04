//======================================================================
//
// CuiBugSubmissionPage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiBugSubmissionPage.h"

#include "UIButton.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientBugReporting/ClientBugReporting.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiCSManagerListener.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSharedPageManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/StationId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"
#include "sharedNetworkMessages/ConnectPlayerMessage.h"
#include "sharedNetworkMessages/DisconnectPlayerMessage.h"
#include "sharedNetworkMessages/RequestCategoriesMessage.h"

//lint -esym(534, CuiMessageBox::createInfoBox)// ignore return
//lint -esym(534, _itoa)

//----------------------------------------------------------------------

namespace CuiBugSubmissionPageNamespace
{
	bool  s_createCSTicket       = true;
	bool  s_sendMail             = false;
	float s_maxTimeWait          = 3.0f;
}

using namespace CuiBugSubmissionPageNamespace;

//----------------------------------------------------------------------

CuiBugSubmissionPage::CuiBugSubmissionPage (UIPage & page) :
CuiMediator       ("CuiBugSubmissionPage", page),
UIEventCallback   (),
m_callBack(new MessageDispatch::Callback),
m_cancelButton    (0),
m_okButton        (0),
m_repeatableCombo (0),
m_bugTypeCombo    (0),
m_bugSubTypeCombo (0),
m_descriptionText (0),
m_sendMiniDump    (false),
m_addedBody       (),
m_timeoutTimer       (0.0f)
{
	m_callBack->connect(*this, &CuiBugSubmissionPage::onRequestTicketCategoriesResponse, static_cast<CustomerServiceManager::Messages::RequestTicketCategoriesResponse *>(0));

	getCodeDataObject (TUIButton,    m_okButton,        "buttonSend");
	getCodeDataObject (TUIButton,    m_cancelButton,    "buttonCancel");
	getCodeDataObject (TUIComboBox,  m_repeatableCombo, "comboRepeatable");
	getCodeDataObject (TUIComboBox,  m_bugTypeCombo,    "comboBugType");
	getCodeDataObject (TUIComboBox,  m_bugSubTypeCombo, "comboBugSubType");
	getCodeDataObject (TUIComboBox,  m_systemCombo,     "comboSystem");
	getCodeDataObject (TUIComboBox,  m_severityCombo,   "comboSeverity");
	getCodeDataObject (TUIText,      m_descriptionText, "descriptiontext");

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	setIsUpdating(true);

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	NOT_NULL (parent);
	CuiSharedPageManager::registerMediatorType (parent->GetName (), CuiMediatorTypes::BugSubmission);

	registerMediatorObject (*m_okButton,     true);
	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_bugTypeCombo, true);
	registerMediatorObject (*m_bugSubTypeCombo, true);

	m_timeoutTimer = s_maxTimeWait;

	m_repeatableCombo->SetSelectedIndex(-1);
	m_bugTypeCombo->SetSelectedIndex   (-1);
	m_bugSubTypeCombo->SetSelectedIndex(-1);
	
	m_bugTypeCombo->Clear   ();
	m_bugSubTypeCombo->Clear();

	m_systemCombo->SetSelectedIndex    (0);
	m_severityCombo->SetSelectedIndex  (0);

	if(!Game::getSinglePlayer() && s_createCSTicket)
	{
		m_okButton->SetEnabled(false);
	}
}

//----------------------------------------------------------------------

CuiBugSubmissionPage::~CuiBugSubmissionPage()
{
	delete m_callBack;
	m_callBack = NULL;
}

//----------------------------------------------------------------------

void CuiBugSubmissionPage::performActivate   ()
{
	CuiManager::requestPointer (true);

	if(m_sendMiniDump)
		m_bugTypeCombo->SetSelectedIndex (0);

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::BugSubmission, true);

	getPage ().Center ();

	m_bugTypeCombo->SetFocus ();

	// We only need to request connections, once connected we will leave
	// the connection open and the connection server will close the
	// connection when the player logs out

	CustomerServiceManager::requestConnection();

	if (m_bugTypeCombo->GetItemCount() <= 1)
	{
		populateCategories();
	}
}

//----------------------------------------------------------------------

void CuiBugSubmissionPage::performDeactivate ()
{
	//don't disconnect from the "disconnect" message until destruction time

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::BugSubmission, false);
	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void CuiBugSubmissionPage::OnButtonPressed (UIWidget *context)
{
	if (context == m_okButton)
	{
		Unicode::String description;
		m_descriptionText->GetLocalText (description);
		if(description.empty ())
		{
			CuiMessageBox::createInfoBox (CuiStringIds::bug_desc.localize());
			return;
		}

		Unicode::String bugType;
		IGNORE_RETURN (m_bugTypeCombo->GetProperty (UIComboBox::PropertyName::SelectedText, bugType));
		if(bugType.empty ())
		{
			CuiMessageBox::createInfoBox (CuiStringIds::bug_choosetype.localize());
			return;
		}

		Unicode::String repeatable;
		IGNORE_RETURN (m_repeatableCombo->GetProperty (UIComboBox::PropertyName::SelectedText, repeatable));
		if(repeatable.empty ())
		{
			CuiMessageBox::createInfoBox (CuiStringIds::bug_chooserepeatable.localize());
			return;
		}

		Unicode::String system;
		IGNORE_RETURN (m_systemCombo->GetProperty (UIComboBox::PropertyName::SelectedText, system));

		Unicode::String severity;
		IGNORE_RETURN (m_severityCombo->GetProperty (UIComboBox::PropertyName::SelectedText, severity));

		FormattedString<256> fs;

		Unicode::String mail_body;

		mail_body.append(Unicode::narrowToWide(fs.sprintf("StationId: %d\n", GameNetwork::getStationId())));

		mail_body.append(Unicode::narrowToWide("Bug Type: "));
		mail_body.append(bugType.c_str());
		mail_body.append(Unicode::narrowToWide("\n"));

		mail_body.append(Unicode::narrowToWide("Repeatable: "));
		mail_body.append(repeatable.c_str());
		mail_body.append(Unicode::narrowToWide("\n"));

		mail_body.append(Unicode::narrowToWide("Game System: "));
		mail_body.append(system.c_str());
		mail_body.append(Unicode::narrowToWide("\n"));

		mail_body.append(Unicode::narrowToWide("Severity: "));
		mail_body.append(severity.c_str());
		mail_body.append(Unicode::narrowToWide("\n"));

		//add in the data set by other systems (via the addLineToBody function)
		mail_body.append(Unicode::narrowToWide(m_addedBody));
		m_addedBody.clear ();

		mail_body.append(Unicode::narrowToWide("\n"));
		mail_body.append(description);

		bool result = true;
		//do we want to send a mail, or a cs ticket?
		if(s_sendMail)
		{
			//Build a short description to add in the subject
			std::string short_desc(Unicode::wideToNarrow(description.substr(0, 50)));
			if(short_desc.size() > 50)
				short_desc += "...";

			std::string const subject(fs.sprintf("%s Bug: %s", Unicode::wideToNarrow(bugType).c_str(), short_desc.c_str()));
			std::string const from(fs.sprintf("StationId_%d@swgbetatest.station.sony.com", GameNetwork::getStationId()));

			// NOTE: This mail will not correctly send Unicode data. If the entire mail bug report is non-unicode text, everything will work fine.

			std::vector<std::string> attachments;
			result = ClientBugReporting::sendMail("swgbetatestbugs@soe.sony.com", from.c_str(), subject, Unicode::wideToNarrow(mail_body), attachments, m_sendMiniDump);
		}

		//also create CS ticket
		if(!Game::getSinglePlayer() && s_createCSTicket)
		{
			if (CustomerServiceManager::getBugCategoryList().size() > 0)
			{
				std::string characterName(Unicode::wideToNarrow(Game::getClientPlayer()->getLocalizedName()));

				std::string categoryString;
				m_bugTypeCombo->GetSelectedIndexName(categoryString);
				unsigned int const category = static_cast<unsigned int>(atoi(categoryString.c_str()));
				
				std::string subCategoryString;
				m_bugSubTypeCombo->GetSelectedIndexName(subCategoryString);
				unsigned int const subCategory = static_cast<unsigned int>(atoi(subCategoryString.c_str()));

				GameNetwork::send(CreateTicketMessage(characterName, category, subCategory, mail_body, Unicode::emptyString, Unicode::emptyString, UIManager::gUIManager().GetLocaleString(), true), true);
			}
		}

		if(!result)
		{
			CuiMessageBox::createInfoBox (CuiStringIds::bug_failed.localize());
		}
		else
		{
			CuiMessageBox::createInfoBox (CuiStringIds::bug_success.localize());

			//clear bug description
			m_descriptionText->SetLocalText(UIString());
			m_repeatableCombo->SetSelectedIndex(-1);
			m_bugTypeCombo->SetSelectedIndex(-1);
			m_bugSubTypeCombo->SetSelectedIndex(-1);
			m_systemCombo->SetSelectedIndex(0);
			m_severityCombo->SetSelectedIndex(0);
			//if we sent a minidump, don't do so again this session
			m_sendMiniDump = false;
			deactivate();
		}
	}
	else if (context == m_cancelButton)
	{
		//if they cancel the startup-minidump mail sending, clear out the bug type (crash) entry
		if(m_sendMiniDump)
		{
			m_bugTypeCombo->SetSelectedIndex(-1);
			m_bugSubTypeCombo->SetSelectedIndex(-1);
		}
		//if we sent a minidump, don't do so again this session
		m_sendMiniDump = false;
		m_addedBody.clear ();

		deactivate();
	}
} //lint !e818 context could be const (don't change sig of overridden func)

//----------------------------------------------------------------------

void CuiBugSubmissionPage::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if ((m_timeoutTimer > 0.0f) && (m_timeoutTimer - deltaTimeSecs) < 0.0f)
	{
		m_okButton->SetEnabled(true);
	}
	m_timeoutTimer -= deltaTimeSecs;
}

//----------------------------------------------------------------------

void CuiBugSubmissionPage::addLineToBody(const std::string& line)
{
	m_addedBody += line + "\n";
}

//----------------------------------------------------------------------

bool CuiBugSubmissionPage::getCSCategories(Unicode::String const & catName, Unicode::String const & subCatName, /*OUT*/unsigned int& category, /*OUT*/unsigned int& subCategory)
{
	CustomerServiceManager::TicketCategoryList const &ticketCategoryList = CustomerServiceManager::getBugCategoryList();

	for(CustomerServiceManager::TicketCategoryList::const_iterator i = ticketCategoryList.begin(); i != ticketCategoryList.end(); ++i)
	{
		if (i->getCategoryName() == catName)
		{
			const std::vector<CustomerServiceCategory>& subCats = i->getSubCategories();
			for(std::vector<CustomerServiceCategory>::const_iterator j = subCats.begin(); j != subCats.end(); ++j)
			{
				if(j->getCategoryName() == subCatName)
				{
					category = i->getCategoryId();
					subCategory = j->getCategoryId();
					return true;
				}
			}
		}
	}
	DEBUG_WARNING(true, ("Couldn't find the CSCategoryIds for %s, %s", catName.c_str(), subCatName.c_str()));
	return false;
}

//-----------------------------------------------------------------

void CuiBugSubmissionPage::setComboBoxItems(UIComboBox &comboBox, stdvector<CustomerServiceCategory>::fwd const &items) const
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
	else
	{
		comboBox.SetSelectedIndex(-1);
	}
}

//-----------------------------------------------------------------

void CuiBugSubmissionPage::OnGenericSelectionChanged(UIWidget *context)
{
	const std::vector<CustomerServiceCategory> & categories = CustomerServiceManager::getBugCategoryList();

	if (context == m_bugTypeCombo)
	{
		if ((m_bugTypeCombo->GetItemCount() > 0) && (m_bugTypeCombo->GetItemCount() == static_cast<int>(categories.size())))
		{
			const int selectedIndex = m_bugTypeCombo->GetSelectedIndex();
			if(selectedIndex >= 0 && selectedIndex < static_cast<int>(categories.size()))
			{
				const std::vector<CustomerServiceCategory> & subCategories = categories[static_cast<unsigned int>(selectedIndex)].getSubCategories();
				setComboBoxItems(*m_bugSubTypeCombo, subCategories);
			}
		}
	}
}

//-----------------------------------------------------------------

void CuiBugSubmissionPage::populateCategories()
{
	if (CustomerServiceManager::getBugCategoryList().size() > 0)
	{
		// See if we need to populate the categories

		setComboBoxItems(*m_bugTypeCombo, CustomerServiceManager::getBugCategoryList());

		// Set the proper category and sub-category items

		OnGenericSelectionChanged(m_bugTypeCombo);
		m_descriptionText->SetEditable(true);
		m_descriptionText->Clear();
		m_okButton->SetEnabled(true);
	}
	else
	{
		m_bugTypeCombo->Clear();
		m_bugTypeCombo->AddItem(CuiStringIdsCustomerService::no_categories_found.localize(), "");
		m_bugTypeCombo->SetSelectedIndex(0);

		m_bugSubTypeCombo->Clear();
		m_bugSubTypeCombo->SetSelectedIndex(0);

		m_descriptionText->SetEditable(false);
		m_descriptionText->SetLocalText(CuiStringIdsCustomerService::server_no_connection.localize());

		m_okButton->SetEnabled(false);
	}
}

//-----------------------------------------------------------------

void CuiBugSubmissionPage::onRequestTicketCategoriesResponse(CustomerServiceManager::Messages::RequestTicketCategoriesResponse::Response const &)
{
	populateCategories();
}

//======================================================================
