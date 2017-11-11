// ======================================================================
//
// TabPoll.cpp
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "TabPoll.h"
#include "DialogPoll.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

namespace TabPollNamespace
{
	CString const TOKEN_HEADER_BEGIN(_T("Header"));
	CString const TOKEN_HEADER_END(_T("End"));
	CString const TOKEN_HEADER_TITLE(_T("Title"));
	CString const TOKEN_HEADER_DESCRIPTION(_T("Description"));
	CString const TOKEN_HEADER_TAG(_T("Tag"));
	CString const TOKEN_HEADER_FREQUENCY(_T("Frequency"));
	CString const TOKEN_HEADER_EMAIL(_T("Email"));
	CString const TOKEN_HEADER_CANCEL(_T("Cancel"));
	CString const TOKEN_HEADER_ACTIVE(_T("Active"));
	CString const TOKEN_HEADER_COMPLETE(_T("Complete"));
	
	CString const TOKEN_QUESTION_BEGIN(_T("Question"));
	CString const TOKEN_QUESTION_END(_T("End"));
	CString const TOKEN_QUESTION_TEXT(_T("Text"));
	CString const TOKEN_QUESTION_TAB_TITLE(_T("Title"));
	CString const TOKEN_QUESTION_BUTTON(_T("Button"));
	CString const TOKEN_QUESTION_INPUT(_T("Input"));
}

using namespace TabPollNamespace;

/////////////////////////////////////////////////////////////////////////////
// TabPoll

IMPLEMENT_DYNAMIC(TabPoll, CTabCtrl);

// ----------------------------------------------------------------------

TabPoll::TabPoll(CStdioFile & file) : m_pages(), m_currentTab(0), m_wasCanceled(true), m_file(file), m_headerParsed(false), m_pollActive(false)
{
	parseHeader();
}

// ----------------------------------------------------------------------

TabPoll::~TabPoll()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(TabPoll, CTabCtrl)
	//{{AFX_MSG_MAP(TabPoll)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TabPoll message handlers

void TabPoll::Init()
{
	deleteTabs();

	// Parse the header.
	if (m_headerParsed)
	{
		// Now parse questions.
		Parser questionParser;

		// Add tokens.
		questionParser.addToken(TOKEN_QUESTION_BEGIN, Parser::tt_begin);
		questionParser.addToken(TOKEN_QUESTION_END, Parser::tt_end);
		questionParser.addToken(TOKEN_QUESTION_TEXT, Parser::tt_store);
		questionParser.addToken(TOKEN_QUESTION_TAB_TITLE, Parser::tt_store);
		questionParser.addToken(TOKEN_QUESTION_BUTTON, Parser::tt_store);
		questionParser.addToken(TOKEN_QUESTION_INPUT, Parser::tt_store);

		// Parse the entire file.  If you get errors, display using internal error code.
	 	while (!questionParser.eof()) 
		{
			if (questionParser.parse(m_file))
			{
				// Set the text for tab the button.
				CString const & question = questionParser.getTokenAsString(TOKEN_QUESTION_TAB_TITLE);
				DialogPoll * dlg = createTab(question.GetLength() ? question : _T("Question"));

				if (dlg)
				{
					// Add user input text.
					CString const & questionText = questionParser.getTokenAsString(TOKEN_QUESTION_TEXT);
					dlg->setQuestionText(questionText);

					// Add buttons to the bottom of the page.
					Parser::Nodes buttonNodes;
					questionParser.getNodes(TOKEN_QUESTION_BUTTON, buttonNodes);

					for(Parser::Nodes::const_iterator itNode = buttonNodes.begin(); itNode != buttonNodes.end(); ++itNode)
					{
						Parser::Node const & node = itNode->second;
						dlg->addButton(node.first, node.second == Parser::nf_default);
					}

					// Add user input controls.
					if (questionParser.hasToken(TOKEN_QUESTION_INPUT))
					{
						CString const & inputText = questionParser.getTokenAsString(TOKEN_QUESTION_INPUT);
						dlg->setInputText(inputText, true);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void TabPoll::SetRectangle()
{
	CRect tabRect, itemRect;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);
	
	int nX = itemRect.left;
	int nY = itemRect.bottom + 1;
	int nXc = tabRect.right - itemRect.left - 1;
	int nYc = tabRect.bottom - nY- 1;

	for (TabPages::iterator itTab = m_pages.begin(); itTab != m_pages.end(); ++itTab) 
	{
		CDialog * const dlg = *itTab;
		if (dlg) 
		{
			dlg->SetWindowPos(&wndTop, nX, nY, nXc, nYc, itTab == m_pages.begin() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
		}
	}
}

// ----------------------------------------------------------------------

void TabPoll::deleteTabs()
{
	for (TabPages::iterator itTab = m_pages.begin(); itTab != m_pages.end(); ++itTab) 
	{
		CDialog * const dlg = *itTab;
		
		dlg->DestroyWindow();

		delete dlg;
	}

	m_pages.clear();

	refreshItems();
}

// ----------------------------------------------------------------------

DialogPoll * TabPoll::createTab(CString const & tabName)
{
	DialogPoll * const newDlg = new DialogPoll;

	newDlg->Create(IDD_DIALOG_POLL, this);

	newDlg->SetWindowText(tabName);

	m_pages.push_back(newDlg);

	refreshItems();

	setCurrentTab(0);

	return newDlg;
}

// ----------------------------------------------------------------------

void TabPoll::refreshItems()
{
	DeleteAllItems();

	for (TabPages::iterator itTab = m_pages.begin(); itTab != m_pages.end(); ++itTab) 
	{
		CDialog * const dlg = *itTab;
		
		int const index = itTab - m_pages.begin();

		CString windowText;
		dlg->GetWindowText(windowText);

		InsertItem(index, windowText);
	}
}

// ----------------------------------------------------------------------

void TabPoll::OnDestroy() 
{
	CTabCtrl::OnDestroy();

	deleteTabs();
}

// ----------------------------------------------------------------------

void TabPoll::setInfoFromHeader(Parser const & headerParser)
{
	CString const & title = headerParser.getTokenAsString(TOKEN_HEADER_TITLE);
	setTitle(title);

	CString const & description = headerParser.getTokenAsString(TOKEN_HEADER_DESCRIPTION);
	setDescription(description);

	CString const & tag = headerParser.getTokenAsString(TOKEN_HEADER_TAG);
	setTag(tag);

	float const frequency = headerParser.getTokenAsFloat(TOKEN_HEADER_FREQUENCY);
	setFrequency(frequency);

	CString const & cancelButton = headerParser.getTokenAsString(TOKEN_HEADER_CANCEL);
	setCancelButton(cancelButton);

	CString const & email = headerParser.getTokenAsString(TOKEN_HEADER_EMAIL);
	setEmailAddress(email);

	bool const active = headerParser.getTokenAsBool(TOKEN_HEADER_ACTIVE);
	setPollActive(active);

	CString const & complete = headerParser.getTokenAsString(TOKEN_HEADER_COMPLETE);
	setSubmitButtonText(complete);
}

// ----------------------------------------------------------------------

void TabPoll::setTitle(CString const & title)
{
	m_title = title;	
}

// ----------------------------------------------------------------------

void TabPoll::setDescription(CString const & description)
{
	m_description = description;
}

// ----------------------------------------------------------------------

void TabPoll::setTag(CString const & tag)
{
	m_tag = tag;
}

// ----------------------------------------------------------------------

void TabPoll::setFrequency(float frequency)
{
	m_frequency = frequency;
}

// ----------------------------------------------------------------------

CString const & TabPoll::getTitle() const
{
	return m_title;	
}

// ----------------------------------------------------------------------

CString const & TabPoll::getDescription() const
{
	return m_description;
}

// ----------------------------------------------------------------------

CString const & TabPoll::getTag() const
{
	return m_tag;
}

// ---------------------------------------------------------------------

float TabPoll::getFrequency() const
{
	return m_frequency;
}

// ---------------------------------------------------------------------

void TabPoll::setCancelButton(CString const & cancel)
{
	m_cancel = cancel;
}

// ---------------------------------------------------------------------

void TabPoll::setPollActive(bool const active)
{
	m_pollActive = active;
}

// ---------------------------------------------------------------------

bool TabPoll::isPollActive() const
{
	return m_pollActive;
}

// ---------------------------------------------------------------------

CString const & TabPoll::getCancelButton() const
{
	return m_cancel;
}

// ---------------------------------------------------------------------

void TabPoll::setEmailAddress(CString const & email)
{
	m_email = email;
}

// ---------------------------------------------------------------------

CString const & TabPoll::getEmailAddress() const
{
	return m_email;
}

// ---------------------------------------------------------------------

void TabPoll::setSubmitButtonText(CString const & submit)
{
	m_submit = submit;
}

// ---------------------------------------------------------------------

CString const & TabPoll::getSubmitButtonText() const
{
	return m_submit;
}

// ----------------------------------------------------------------------

void TabPoll::setCurrentTab(int tabIndex)
{
	if (m_pages.empty())
	{
		m_currentTab = -1;
	}
	else
	{
		tabIndex = clamp(static_cast<int>(0), tabIndex, static_cast<int>(m_pages.size() - 1));
		m_currentTab = tabIndex;

		SetCurSel(m_currentTab);

		for (TabPages::iterator itTab = m_pages.begin(); itTab != m_pages.end(); ++itTab) 
		{
			CDialog * const dlg = *itTab;
			
			int const index = itTab - m_pages.begin();

			dlg->ShowWindow(SW_HIDE);

			if (index == tabIndex) 
			{
				dlg->ShowWindow(SW_SHOW);
				dlg->SetFocus();
			}
		}
	}
}

// ----------------------------------------------------------------------

void TabPoll::setTabFromFocus()
{
	setCurrentTab(GetCurFocus());
}

// ----------------------------------------------------------------------

int TabPoll::getCurrentTab() const
{
	return m_currentTab;
}

// ----------------------------------------------------------------------

void TabPoll::setPreviousTab()
{
	setCurrentTab(getCurrentTab() - 1);
}

// ----------------------------------------------------------------------

void TabPoll::setNextTab()
{
	setCurrentTab(getCurrentTab() + 1);
}

// ----------------------------------------------------------------------

bool TabPoll::isLastTab() const
{
	return m_currentTab == (m_pages.size() - 1);
}

// ----------------------------------------------------------------------

void TabPoll::setCanceled(bool cancel)
{
	m_wasCanceled = cancel;
}

// ----------------------------------------------------------------------

void TabPoll::getMessageInformation(CString & msgInfo)
{
	for (TabPages::iterator itTab = m_pages.begin(); itTab != m_pages.end(); ++itTab) 
	{
		DialogPoll * const dlg = static_cast<DialogPoll*>(*itTab);
		dlg->getMessageInformation(msgInfo);
		msgInfo += _T("\n");
	}
}

// ----------------------------------------------------------------------

bool TabPoll::parseHeader()
{
	Parser headerParser;
	
	headerParser.addToken(TOKEN_HEADER_BEGIN, Parser::tt_begin);
	headerParser.addToken(TOKEN_HEADER_END, Parser::tt_end);
	headerParser.addToken(TOKEN_HEADER_ACTIVE, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_TITLE, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_DESCRIPTION, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_TAG, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_FREQUENCY, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_EMAIL, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_CANCEL, Parser::tt_store);
	headerParser.addToken(TOKEN_HEADER_COMPLETE, Parser::tt_store);

	m_headerParsed = headerParser.parse(m_file);

	if (m_headerParsed) 
	{
		setInfoFromHeader(headerParser);
	}

	return m_headerParsed;
}

// ----------------------------------------------------------------------

bool TabPoll::isHeaderParsed() const
{
	return m_headerParsed;
}


