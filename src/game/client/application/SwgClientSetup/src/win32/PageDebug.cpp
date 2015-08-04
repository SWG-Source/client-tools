// ======================================================================
//
// PageDebug.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageDebug.h"

#include "Options.h"
#include "SwgClientSetup.h"

#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(PageDebug, CPropertyPage)

PageDebug::PageDebug() : CPropertyPage(PageDebug::IDD)
{
	//{{AFX_DATA_INIT(PageDebug)
	m_contact = _T("");
	m_crashLog = _T("");
	m_hardwareInformation = _T("");
	m_stationId = _T("");

	m_lblCrashLogs = _T("");
	m_lblIncludeStationId = _T("");
	m_lblSendHardwareInformation = _T("");
	m_lblContact = _T("");

	m_lblBtnConfigure = _T("");

	//}}AFX_DATA_INIT
}

PageDebug::~PageDebug()
{
}

void PageDebug::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageDebug)
	DDX_Text(pDX, IDC_STATIC_CONTACT, m_contact);
	DDX_Text(pDX, IDC_STATIC_CRASHLOG, m_crashLog);
	DDX_Text(pDX, IDC_STATIC_HARDWAREINFORMATION, m_hardwareInformation);
	DDX_Text(pDX, IDC_STATIC_STATIONID, m_stationId);

	DDX_Text(pDX, IDC_LBL_DEBUG_SEND_CRASH_LOG, m_lblCrashLogs);
	DDX_Text(pDX, IDC_LBL_DEBUG_INCLUDE_STATION_ID, m_lblIncludeStationId);
	DDX_Text(pDX, IDC_LBL_DEBUG_SEND_HARDWARE_INFO, m_lblSendHardwareInformation);
	DDX_Text(pDX, IDC_LBL_DEBUG_ALLOW_SOE_CONTACT, m_lblContact);

	DDX_Text(pDX, IDC_BUTTON_CONFIGURE, m_lblBtnConfigure);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PageDebug, CPropertyPage)
	//{{AFX_MSG_MAP(PageDebug)
	ON_BN_CLICKED(IDC_BUTTON_CONFIGURE, OnButtonConfigure)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageDebug::initalizeDialog() 
{

	VERIFY(m_lblCrashLogs.LoadString(IDS_DEBUG_CRASH_LOGS));
	VERIFY(m_lblIncludeStationId.LoadString(IDS_DEBUG_INCLUDE_STATION_ID_IN_LOGS));
	VERIFY(m_lblSendHardwareInformation.LoadString(IDS_DEBUG_SEND_HARDWARE_INFORMATION));
	VERIFY(m_lblContact.LoadString(IDS_DEBUG_ALLOW_SOE_TO_CONTACT_ME));
	VERIFY(m_lblBtnConfigure.LoadString(IDS_BTN_CONFIGURE));
	
	// TODO: Add extra initialization here
	populate ();	
}

// ======================================================================

BOOL PageDebug::OnSetActive( )
{
	BOOL const result = CPropertyPage::OnSetActive();
	initalizeDialog();
	return result;
}

// ======================================================================

BOOL PageDebug::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();
	initalizeDialog();
	return result;
}

// ======================================================================

void PageDebug::OnButtonConfigure() 
{
	SwgClientSetupApp::configure ();

	populate ();
}

// ======================================================================

void PageDebug::populate ()
{
	if (SwgClientSetupApp::getSendMinidumps())
		VERIFY(m_crashLog.LoadString(IDS_DEBUG_ALWAYS_SEND));
	else
		VERIFY(m_crashLog.LoadString(IDS_DEBUG_NEVER_SEND));

	if (Options::getAllowCustomerContact ())
		VERIFY(m_contact.LoadString(IDS_YES));
	else
		VERIFY(m_contact.LoadString(IDS_NO));

	if (SwgClientSetupApp::getAutomaticallySendHardwareInformation ())
		VERIFY(m_hardwareInformation.LoadString(IDS_YES));
	else
		VERIFY(m_hardwareInformation.LoadString(IDS_NO));

	if (Options::getSendStationId())
		VERIFY(m_stationId.LoadString(IDS_YES));
	else
		VERIFY(m_stationId.LoadString(IDS_NO));

	UpdateData (false);
}

// ======================================================================

