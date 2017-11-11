// ======================================================================
//
// DialogMinidump.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogMinidump.h"

#include "swgclientsetup.h"

#include <atlbase.h>

// ======================================================================

DialogMinidump::DialogMinidump(CWnd* pParent /*=NULL*/)
	: CDialog(DialogMinidump::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogMinidump)
	m_radio = -1;

	m_lblInfo = _T("");
	m_lblQuery = _T("");
	m_lblPleaseSend = _T("");
	m_lblNeverSend = _T("");

	m_lblBtnCancel = _T("");
	m_lblBtnNext = _T("");

	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogMinidump::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogMinidump)
	DDX_Radio(pDX, IDC_RADIO1, m_radio);

	DDX_Text(pDX, IDC_LBL_SENDCRASH_INFO, m_lblInfo);
	DDX_Text(pDX, IDC_LBL_SENDCRASH_QUERY, m_lblQuery);
	DDX_Text(pDX, IDC_RADIO1, m_lblPleaseSend);
	DDX_Text(pDX, IDC_RADIO2, m_lblNeverSend);

	DDX_Text(pDX, ID_CANCEL, m_lblBtnCancel);
	DDX_Text(pDX, IDC_BUTTON_PROCEED, m_lblBtnNext);

	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogMinidump, CDialog)
	//{{AFX_MSG_MAP(DialogMinidump)
	ON_BN_CLICKED(IDC_BUTTON_PROCEED, OnButtonProceed)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogMinidump::OnInitDialog() 
{
	CDialog::OnInitDialog();

	VERIFY(m_lblInfo.LoadString(IDS_SENDCRASH_INFO));
	VERIFY(m_lblQuery.LoadString(IDS_SENDCRASH_QUESTION));
	VERIFY(m_lblPleaseSend.LoadString(IDS_SENDCRASH_RADIO_PLEASE_SEND));
	VERIFY(m_lblNeverSend.LoadString(IDS_SENDCRASH_RADIO_NEVER_SEND));
	VERIFY(m_lblBtnCancel.LoadString(IDS_BTN_CANCEL));
	VERIFY(m_lblBtnNext.LoadString(IDS_BTN_NEXT));
	
	m_radio = SwgClientSetupApp::getSendMinidumps () ? 0 : 1;

	UpdateData (false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogMinidump::OnButtonProceed() 
{
	UpdateData (true);

	SwgClientSetupApp::setSendMinidumps (m_radio == 0);

	// TODO: Add your control notification handler code here
	CDialog::OnOK ();	
}

// ======================================================================

