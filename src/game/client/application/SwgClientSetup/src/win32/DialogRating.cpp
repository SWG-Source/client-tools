// ======================================================================
//
// DialogRating.cpp
// asommers
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogRating.h"

#include "SwgClientSetup.h"
#include "DialogRating.h"

// ======================================================================

DialogRating::DialogRating(CStdioFile & file, CString const & stationId, CString const & fromAddress)
	: CDialog(DialogRating::IDD, NULL), m_tabPoll(file), m_stationId(stationId), m_fromEmailAddress(fromAddress)
{
	//{{AFX_DATA_INIT(DialogRating)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogRating::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogRating)
	DDX_Control(pDX, IDC_BUTTON_COMPLETE, m_buttonFinished);
	DDX_Control(pDX, IDC_BUTTON_NOT_INTERESTED, m_buttonNotInterested);
	DDX_Control(pDX, IDC_STATIC_THANKYOU, m_description);
	DDX_Control(pDX, IDC_TAB_POLL, m_tabPoll);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogRating, CDialog)
	//{{AFX_MSG_MAP(DialogRating)
	ON_BN_CLICKED(IDC_BUTTON_PREV, OnButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_NOT_INTERESTED, OnButtonNotInterested)
	ON_BN_CLICKED(IDC_BUTTON_COMPLETE, OnButtonComplete)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_POLL, OnSelchangeTabPoll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ----------------------------------------------------------------------

void DialogRating::OnOK() 
{
	CDialog::OnOK();
}

// ----------------------------------------------------------------------

BOOL DialogRating::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_tabPoll.Init();
	m_tabPoll.SetRectangle();

	// Set the title.
	CString const & title = m_tabPoll.getTitle();
	if (title.GetLength()) 
	{
		SetWindowText(title);
	}

	// Set the description.
	CString const & description = m_tabPoll.getDescription();
	if (description.GetLength()) 
	{
		m_description.SetWindowText(description);
	}

	// Cancel button active?
	CString const & cancel = m_tabPoll.getCancelButton();
	if (cancel.IsEmpty()) 
	{
		m_buttonNotInterested.EnableWindow(FALSE);
		m_buttonNotInterested.ShowWindow(SW_HIDE);
	}
	else
	{
		m_buttonNotInterested.EnableWindow(TRUE);
		m_buttonNotInterested.ShowWindow(SW_SHOW);
		m_buttonNotInterested.SetWindowText(cancel);
	}

	// Submit button text.
	CString const & sumbit = m_tabPoll.getSubmitButtonText();
	if (!sumbit.IsEmpty()) 
	{
		m_buttonFinished.SetWindowText(sumbit);
	}

	updateExitButton();
	UpdateData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogRating::OnButtonPrev() 
{
	m_tabPoll.setPreviousTab();
	updateExitButton();
}

// ----------------------------------------------------------------------

void DialogRating::OnButtonNext() 
{
	m_tabPoll.setNextTab();	
	updateExitButton();
}

// ----------------------------------------------------------------------

void DialogRating::OnButtonNotInterested() 
{
	m_tabPoll.setCanceled(true);
	EndDialog(0);
}

// ----------------------------------------------------------------------

void DialogRating::OnSelchangeTabPoll(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tabPoll.setTabFromFocus();

	updateExitButton();

	*pResult = 0;
}

// ----------------------------------------------------------------------

void DialogRating::updateExitButton()
{
	m_buttonFinished.ShowWindow(m_tabPoll.isLastTab() ? SW_SHOW : SW_HIDE);
}

// ----------------------------------------------------------------------

void DialogRating::OnButtonComplete() 
{
	m_tabPoll.setCanceled(false);
	EndDialog(0);
}

// ----------------------------------------------------------------------

BOOL DialogRating::DestroyWindow() 
{
	CString const & subject = _T("TAG ") + m_tabPoll.getTag() + _T(" ID ") + m_stationId;

	std::vector<std::string> const attachments;

	std::string const emailTo(wideToNarrow(m_tabPoll.getEmailAddress()));
	std::string const emailFrom(wideToNarrow(m_fromEmailAddress));

	CString body;
	m_tabPoll.getMessageInformation(body);

	SwgClientSetupNamespace::sendMail(emailTo, emailFrom, wideToNarrow(subject), wideToNarrow(body), attachments);
	
	return CDialog::DestroyWindow();
}

// ----------------------------------------------------------------------

float DialogRating::getFrequency() const
{
	ASSERT(m_tabPoll.isHeaderParsed());

	return m_tabPoll.getFrequency();
}

// ----------------------------------------------------------------------

bool DialogRating::isValid() const
{
	return m_tabPoll.isHeaderParsed() && m_tabPoll.isPollActive();
}


