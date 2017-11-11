// ======================================================================
//
// SwgCuiService_KnowledgeBase.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiService_KnowledgeBase.h"
 
#include "clientGame/ClientTextManager.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCSManagerListener.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CustomerServiceSearchResult.h"
#include "sharedNetworkMessages/GetArticleMessage.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseMessage.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIPage.h"
#include "UITextbox.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================
//
// KnowledgeBaseNamespace
//
// ======================================================================

namespace KnowledgeBaseNamespace
{
	time_t s_articleRequestWaitTime = 10;

	Unicode::String stripTitle(Unicode::String const &title);
};

//-----------------------------------------------------------------

Unicode::String KnowledgeBaseNamespace::stripTitle(Unicode::String const &title)
{
	// Pull the redundant "Star Wars Galaxies: " from all article titles

	Unicode::String articleTitlePrefix = Unicode::narrowToWide("Star Wars Galaxies: ");
	Unicode::String result;
	size_t startPosition = title.find(articleTitlePrefix);

	if (startPosition != std::string::npos)
	{
		result = title.substr(articleTitlePrefix.length());
	}
	else
	{
		result = title;
	}

	return result;
}

using namespace KnowledgeBaseNamespace;

// ======================================================================
//
// SwgCuiService::KnowledgeBase
//
// ======================================================================

SwgCuiService::KnowledgeBase::KnowledgeBase(UIPage &page, SwgCuiService &serviceMediator)
 : CuiMediator("SwgCuiService_KnowledgeBase", page)
 , m_callBack(new MessageDispatch::Callback)
 , UIEventCallback()
 , m_serviceMediator(serviceMediator)
 , m_searchArticleNumberButton(NULL)
 , m_searchArticleStringButton(NULL)
 , m_didNotHelpButton(NULL)
 , m_articleSearchStringTextBox(NULL)
 , m_articleSearchNumberTextBox(NULL)
 , m_articleTable(NULL)
 , m_articleSummaryText(NULL)
 , m_articleTitleText(NULL)
 , m_articleHeaderList(new ArticleHeaderList)
 , m_lastArticleRequestTime(0)
 , m_lastArticleSummaryRequestTime(0)
 , m_disablePanel(NULL)
 , m_textBoxComposite(NULL)
{

	m_callBack->connect(*this, &SwgCuiService::KnowledgeBase::onSearchKnowledgeBaseResponse, static_cast<CuiCSManagerListener::Messages::SearchKnowledgeBaseResponse *>(0));
	m_callBack->connect(*this, &SwgCuiService::KnowledgeBase::onGetArticleResponse, static_cast<CuiCSManagerListener::Messages::GetArticleResponse *>(0));


	getCodeDataObject(TUIButton, m_searchArticleNumberButton, "buttonArticleSearchNumber");
	registerMediatorObject(*m_searchArticleNumberButton, true);

	getCodeDataObject(TUIButton, m_searchArticleStringButton, "buttonArticleSearchString");
	registerMediatorObject(*m_searchArticleStringButton, true);

	getCodeDataObject(TUITextbox, m_articleSearchStringTextBox, "textBoxArticleSearchString");
	registerMediatorObject(*m_articleSearchStringTextBox, true);
	m_articleSearchStringTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUITextbox, m_articleSearchNumberTextBox, "textBoxArticleSearchNumber");
	registerMediatorObject(*m_articleSearchNumberTextBox, true);
	m_articleSearchNumberTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUITable, m_articleTable, "tableArticle");
	registerMediatorObject(*m_articleTable, true);

	getCodeDataObject(TUIText, m_articleSummaryText, "textArticleSummary");
	registerMediatorObject(*m_articleSummaryText, true);
	m_articleSummaryText->Clear();
	m_articleSummaryText->SetEditable(false);
	m_articleSummaryText->SetMaximumCharacters(50 * 1024);

	getCodeDataObject(TUIText, m_articleTitleText, "textArticleTitle");
	registerMediatorObject(*m_articleTitleText, true);
	m_articleTitleText->SetEditable(false);

	getCodeDataObject(TUIButton, m_didNotHelpButton, "buttonDidNotHelp");
	registerMediatorObject(*m_didNotHelpButton, true);
	m_didNotHelpButton->SetEnabled(false);

	getCodeDataObject(TUIPage, m_disablePanel, "disablePanel");
	registerMediatorObject(*m_disablePanel, true);
	m_disablePanel->SetVisible(true);

	getCodeDataObject(TUIPage, m_textBoxComposite, "textBoxComposite");
	registerMediatorObject(*m_textBoxComposite, true);

	clearTable();
}

//-----------------------------------------------------------------

SwgCuiService::KnowledgeBase::~KnowledgeBase()
{

	delete m_callBack;
	m_callBack = NULL;

	m_searchArticleNumberButton = NULL;
	m_searchArticleStringButton = NULL;
	m_didNotHelpButton = NULL;
	m_articleSearchStringTextBox = NULL;
	m_articleSearchNumberTextBox = NULL;
	m_articleTable = NULL;
	m_articleSummaryText = NULL;
	m_articleTitleText = NULL;
	m_disablePanel = NULL;
	m_textBoxComposite = NULL;
	delete m_articleHeaderList;
	m_articleHeaderList = NULL;
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::performActivate()
{
	m_articleSearchStringTextBox->SetFocus();

	// JU_TODO: this is a temp hack to get around the fixed article from the known issues tab being displayed here
	m_articleSummaryText->Clear();
	if(!m_articleSearchStringTextBox->GetLocalText().empty())
	{
		searchArticleString();
	}
	else if (!m_articleSearchNumberTextBox->GetLocalText().empty())
	{
		searchArticleNumber();
	}
	// JU_TODO: end temp


}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::performDeactivate()
{

}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::OnButtonPressed(UIWidget *context)
{
	if (context == m_searchArticleNumberButton)
	{
		m_disablePanel->SetVisible(false);
		searchArticleNumber();
	}
	else if (context == m_searchArticleStringButton)
	{
		m_disablePanel->SetVisible(false);
		searchArticleString();
	}
	else if (context == m_didNotHelpButton)
	{
		IGNORE_RETURN(CuiActionManager::performAction(CuiActions::ticketSubmission, Unicode::emptyString));
	}
}

//-----------------------------------------------------------------

bool SwgCuiService::KnowledgeBase::OnMessage(UIWidget *context, const UIMessage & msg)
{
	bool result = true;

	if (msg.Keystroke == UIMessage::Enter)
	{
		if (context == m_articleSearchNumberTextBox)
		{
			m_disablePanel->SetVisible(false);
			searchArticleNumber();

			result = false;
		}
		else if (context == m_articleSearchStringTextBox)
		{
			m_disablePanel->SetVisible(false);
			searchArticleString();

			result = false;
		}
	}
	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (context == m_articleTable)
		{
			time_t waitTime = CuiUtils::GetSystemSeconds() - m_lastArticleSummaryRequestTime;

			if (waitTime > 1)
			{
				int lastSelectedRow = m_articleTable->GetLastSelectedRow();

				if (lastSelectedRow < m_articleTable->GetRowCount())
				{
					CustomerServiceSearchResult const &customerServiveSearchResult = (*m_articleHeaderList)[static_cast<unsigned int>(lastSelectedRow)];

					m_articleTitle = Unicode::narrowToWide(customerServiveSearchResult.getId());

					// Mention to the user that no article is selected

					CuiStringVariablesData data;
					data.targetName = CuiStringIdsCustomerService::know_no_article_selected.localize();
					Unicode::String articleTitleString;
					CuiStringVariablesManager::process(CuiStringIdsCustomerService::know_article_id, data, articleTitleString);
					m_articleTitleText->SetLocalText(articleTitleString);					
					m_didNotHelpButton->SetEnabled(false);

					if (!customerServiveSearchResult.getId().empty())
					{
						// Let the user know we are retrieving the article

						m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_retrieve.localize());

						m_lastArticleSummaryRequestTime = CuiUtils::GetSystemSeconds();

						GameNetwork::send(GetArticleMessage(customerServiveSearchResult.getId(), std::string("en")), true);
					}
					else
					{
						// Error because the id is bad

						m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_response_error.localize());

					
					}

					m_textBoxComposite->Pack();

					result = false;
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::searchArticleNumber()
{
	time_t waitTime = CuiUtils::GetSystemSeconds() - m_lastArticleRequestTime;

 	if (!m_articleSearchNumberTextBox->GetLocalText().empty() &&
	    (waitTime > s_articleRequestWaitTime))
	{
		disableSearch();

		m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_retrieve.localize());

		m_textBoxComposite->Pack();

		m_lastArticleRequestTime = CuiUtils::GetSystemSeconds();

		clearTable();

		m_articleTitle = m_articleSearchNumberTextBox->GetLocalText();

		GameNetwork::send(GetArticleMessage(Unicode::wideToNarrow(m_articleSearchNumberTextBox->GetLocalText()), std::string("en")), true);
	}
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::searchArticleString()
{
	time_t waitTime = CuiUtils::GetSystemSeconds() - m_lastArticleRequestTime;

	if (!m_articleSearchStringTextBox->GetLocalText().empty() &&
	    (waitTime > s_articleRequestWaitTime))
	{
		disableSearch();

		m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_search.localize());

		m_textBoxComposite->Pack();

		m_lastArticleRequestTime = CuiUtils::GetSystemSeconds();

		clearTable();

		m_articleTitle = m_articleSearchStringTextBox->GetLocalText();

		GameNetwork::send(SearchKnowledgeBaseMessage(m_articleTitle, std::string("en")), true);
	}
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::onSearchKnowledgeBaseResponse(std::pair<bool, std::vector<CustomerServiceSearchResult> > const &result)
{
	DEBUG_REPORT_LOG(true, ("SwgCuiService::KnowledgeBase::onSearchKnowledgeBaseResponse() success(%s) entries(%d)\n", result.first ? "yes" : "no", static_cast<int>(result.second.size())));

	bool const success = result.first;

	*m_articleHeaderList = result.second;

	if (success)
	{
	    if (!result.second.empty())
		{
			UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_articleTable->GetTableModel());

			if (tableModel != NULL)
			{
				clearTable();
				int rank = 1;

				std::vector<CustomerServiceSearchResult>::const_iterator iterCustomerServiceSearchResult = m_articleHeaderList->begin();

				for (; iterCustomerServiceSearchResult != m_articleHeaderList->end(); ++iterCustomerServiceSearchResult)
				{
					for (int column = 0; column < static_cast<int>(C_count); ++column)
					{
						switch (column)
						{
							case C_articleId:
								{
									IGNORE_RETURN(tableModel->AppendCell(column, NULL, Unicode::narrowToWide(iterCustomerServiceSearchResult->getId())));
								}
								break;
							case C_rank:
								{
									char text[256];
									snprintf(text, sizeof(text), "%d", rank);
									text[sizeof(text) - 1] = '\0';

									IGNORE_RETURN(tableModel->AppendCell(column, NULL, Unicode::narrowToWide(text)));
								}
								break;
							case C_articleTitle:
								{
									IGNORE_RETURN(tableModel->AppendCell(column, NULL, stripTitle(iterCustomerServiceSearchResult->getTitle())));
								}
								break;
							default:
								{
									DEBUG_FATAL(true, ("Invalid column specified."));
								}
								break;
						}
					}

					++rank;
				}

				if (m_articleTable->GetRowCount() > 0)
				{
					m_articleTable->SelectRow(0);
				}
			}

			m_articleSummaryText->Clear();

		}
		else
		{
			m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_no_matches.localize());
		}
	}
	else
	{
		clearTable();
		m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_response_error.localize());
	}

	m_textBoxComposite->Pack();

	enableSearch();
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::onGetArticleResponse(std::pair<bool, Unicode::String> const &result)
{
	bool const success = result.first;

	if (success)
	{
		// Build the article summary

		Unicode::String articleSummary;

		NOT_NULL(m_articleHeaderList);

		int lastSelectedRow = m_articleTable->GetLastSelectedRow();

		if (   (lastSelectedRow >= 0)
			&& (m_articleTable->GetRowCount()))
		{
			CustomerServiceSearchResult const &customerServiveSearchResult = (*m_articleHeaderList)[static_cast<unsigned int>(lastSelectedRow)];

			articleSummary += ClientTextManager::getColorCode(PackedRgb::solidWhite);
			articleSummary += stripTitle(customerServiveSearchResult.getTitle());
			articleSummary += ClientTextManager::getResetTagCode();
			articleSummary += Unicode::narrowToWide("\n\n");
		}
		else
		{
			clearTable();
		}

		articleSummary += result.second;

		//DEBUG_REPORT_LOG(true, ("ArticleSummary: %s\n", Unicode::wideToNarrow(articleSummary).c_str()));

		m_articleSummaryText->SetLocalText(articleSummary);

		// Set the article title

		CuiStringVariablesData data;
		data.targetName = m_articleTitle;
		Unicode::String articleTitleString;
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::know_article_id, data, articleTitleString);
		m_articleTitleText->SetLocalText(articleTitleString);
		m_didNotHelpButton->SetEnabled(true);
	}
	else
	{
		m_articleSummaryText->SetLocalText(CuiStringIdsCustomerService::know_article_response_error.localize());
	}

	m_textBoxComposite->Pack();

	enableSearch();
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::disableSearch()
{
	m_articleSearchStringTextBox->SetEnabled(false);
	m_articleSearchNumberTextBox->SetEnabled(false);
	m_searchArticleNumberButton->SetEnabled(false);
	m_searchArticleStringButton->SetEnabled(false);
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::enableSearch()
{
	m_articleSearchStringTextBox->SetEnabled(true);
	m_articleSearchNumberTextBox->SetEnabled(true);
	m_searchArticleNumberButton->SetEnabled(true);
	m_searchArticleStringButton->SetEnabled(true);

	m_lastArticleSummaryRequestTime = 0;
	m_lastArticleRequestTime = 0;
}

//-----------------------------------------------------------------

void SwgCuiService::KnowledgeBase::clearTable()
{
	UITableModelDefault *tableModel = dynamic_cast<UITableModelDefault *>(m_articleTable->GetTableModel());

	if (tableModel != NULL)
	{
		tableModel->ClearTable();
	}

	if (m_articleTable != NULL)
	{
		m_articleTable->SelectRow(-1);
	}

	// Mention to the user that no article is selected

	CuiStringVariablesData data;
	data.targetName = CuiStringIdsCustomerService::know_no_article_selected.localize();
	Unicode::String articleTitleString;
	CuiStringVariablesManager::process(CuiStringIdsCustomerService::know_article_id, data, articleTitleString);
	m_articleTitleText->SetLocalText(articleTitleString);
	m_didNotHelpButton->SetEnabled(false);
}

// ======================================================================
