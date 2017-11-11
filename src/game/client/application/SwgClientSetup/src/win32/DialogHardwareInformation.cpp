// ======================================================================
//
// DialogHardwareInformation.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogHardwareInformation.h"

#include "swgclientsetup.h"

// ======================================================================

DialogHardwareInformation::DialogHardwareInformation(CWnd* pParent /*=NULL*/)
	: CDialog(DialogHardwareInformation::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogHardwareInformation)
	m_radio = -1;

	m_lblSendHardwareInfo = _T("");
	m_lblQuery = _T("");
	m_lblPleaseSend = _T("");
	m_lblPleaseDoNotSend = _T("");

	m_lblBtnCancel = _T("");
	m_lblBtnNext = _T("");

	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogHardwareInformation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogHardwareInformation)
	DDX_Radio(pDX, IDC_RADIO1, m_radio);

	DDX_Text(pDX, IDC_LBL_SEND_HARDWARE_INFO, m_lblSendHardwareInfo);
	DDX_Text(pDX, IDC_LBL_SEND_HARDWARE_QUERY, m_lblQuery);
	DDX_Text(pDX, IDC_RADIO1, m_lblPleaseSend);
	DDX_Text(pDX, IDC_RADIO2, m_lblPleaseDoNotSend);

	DDX_Text(pDX, ID_CANCEL, m_lblBtnCancel);
	DDX_Text(pDX, IDC_BUTTON_PROCEED, m_lblBtnNext);

	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogHardwareInformation, CDialog)
	//{{AFX_MSG_MAP(DialogHardwareInformation)
	ON_BN_CLICKED(IDC_BUTTON_PROCEED, OnButtonProceed)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogHardwareInformation::OnInitDialog() 
{
	CDialog::OnInitDialog();

	VERIFY(m_lblSendHardwareInfo.LoadString(IDS_SEND_HARDWARE_INFO));
	VERIFY(m_lblQuery.LoadString(IDS_SEND_HARDWARE_QUERY));
	VERIFY(m_lblPleaseSend.LoadString(IDS_SEND_HARWARE_PLEASE_SEND));
	VERIFY(m_lblPleaseDoNotSend.LoadString(IDS_HARDWARE_PLEASE_DO_NOT_SEND));
	VERIFY(m_lblBtnCancel.LoadString(IDS_BTN_CANCEL));
	VERIFY(m_lblBtnNext.LoadString(IDS_BTN_NEXT));
	
	m_radio = SwgClientSetupApp::getAutomaticallySendHardwareInformation () ? 0 : 1;

	UpdateData (false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogHardwareInformation::OnButtonProceed() 
{
	UpdateData (true);

	SwgClientSetupApp::setAutomaticallySendHardwareInformation (m_radio == 0);

	// TODO: Add your control notification handler code here
	CDialog::OnOK ();	
}

// ======================================================================
