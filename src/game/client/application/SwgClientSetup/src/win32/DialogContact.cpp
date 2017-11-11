// ======================================================================
//
// DialogContact.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogContact.h"

#include "Options.h"
#include "SwgClientSetup.h"


// ======================================================================

DialogContact::DialogContact(CWnd* pParent /*=NULL*/)
	: CDialog(DialogContact::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogContact)
	m_radio = -1;

	m_lblSOEContactInfo = _T("");
	m_lblSOEContactQuery = _T("");
	m_lblPleaseContact = _T("");
	m_lblPleaseDoNotContact = _T("");

	m_lblBtnCancel = _T("");
	m_lblBtnNext = _T("");

	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogContact::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogContact)
	DDX_Radio(pDX, IDC_RADIO1, m_radio);

	DDX_Text(pDX, IDC_LBL_SOE_CONTACT_INFO, m_lblSOEContactInfo);
	DDX_Text(pDX, IDC_LBL_SOE_CONTACT_QUERY, m_lblSOEContactQuery);
	DDX_Text(pDX, IDC_RADIO1, m_lblPleaseContact);
	DDX_Text(pDX, IDC_RADIO2, m_lblPleaseDoNotContact);
	
	DDX_Text(pDX, ID_CANCEL, m_lblBtnCancel);
	DDX_Text(pDX, IDC_BUTTON_PROCEED, m_lblBtnNext);

	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogContact, CDialog)
	//{{AFX_MSG_MAP(DialogContact)
	ON_BN_CLICKED(IDC_BUTTON_PROCEED, OnButtonProceed)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogContact::OnInitDialog() 
{
	CDialog::OnInitDialog();

	VERIFY(m_lblSOEContactInfo.LoadString(IDS_SOE_CONTACT_INFO));
	VERIFY(m_lblSOEContactQuery.LoadString(IDS_SOE_CONTACT_QUESTION));
	VERIFY(m_lblPleaseContact.LoadString(IDS_SOE_CONTACT_PLEASE_CONTACT));
	VERIFY(m_lblPleaseDoNotContact.LoadString(IDS_SOE_CONTACT_NO_PLEASE));
	VERIFY(m_lblBtnCancel.LoadString(IDS_BTN_CANCEL));
	VERIFY(m_lblBtnNext.LoadString(IDS_BTN_NEXT));
	
	m_radio = Options::getAllowCustomerContact () ? 0 : 1;

	UpdateData (false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogContact::OnButtonProceed() 
{
	UpdateData (true);

	Options::setAllowCustomerContact (m_radio == 0);

	// TODO: Add your control notification handler code here
	CDialog::OnOK ();	
}

// ======================================================================
