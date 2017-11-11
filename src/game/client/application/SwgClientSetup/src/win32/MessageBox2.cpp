// MessageBox2.cpp : implementation file
//

#include "FirstSwgClientSetup.h"
#include "MessageBox2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <atlbase.h>

namespace MessageBox2Namespace
{
	TCHAR const * ms_registryFolder;
}
using namespace MessageBox2Namespace;

/////////////////////////////////////////////////////////////////////////////
// MessageBox2 dialog

void MessageBox2::install(TCHAR const * registryFolder)
{
	ms_registryFolder = registryFolder;
}

MessageBox2::MessageBox2(CString const & text, CWnd* pParent /*=NULL*/)
	: CDialog(MessageBox2::IDD, pParent)
{
	//{{AFX_DATA_INIT(MessageBox2)
	//}}AFX_DATA_INIT

	m_message = text;
	m_okayText = "Okay";
	m_cancelText = "Cancel";
}

void MessageBox2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MessageBox2)
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDOK, m_okay);
	DDX_Control(pDX, IDC_DONOTASKAGAIN, m_doNotAskAgain);
	DDX_Control(pDX, IDWEB, m_goToWebPage);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(MessageBox2, CDialog)
	//{{AFX_MSG_MAP(MessageBox2)
	ON_BN_CLICKED(IDWEB, OnWeb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MessageBox2 message handlers

void MessageBox2::setOkayButton(CString const & text)
{
	m_okayText = text;
}

void MessageBox2::setCancelButton(CString const & text)
{
	m_cancelText = text;
}

void MessageBox2::setWebButton(CString const & text, CString const & url)
{
	m_webText = text;
	m_url = url;
}

void MessageBox2::setDoNotShowAgainCheckBox(CString const & text, CString  const & registryKey)
{
	m_doNotShowText = text;
	m_registryKey = registryKey;
}

int MessageBox2::DoModal() 
{
	if (!m_registryKey.IsEmpty())
	{
		CRegKey regKey;
		regKey.Open (HKEY_CURRENT_USER, ms_registryFolder);

		DWORD value = 0;
#if _MSC_VER < 1300
		if (regKey.QueryValue (value, m_registryKey) == ERROR_SUCCESS)
			return IDOK;
#else
		if (regKey.QueryValue (value, m_registryKey) == ERROR_SUCCESS)
			return IDOK;
#endif
	}

	return CDialog::DoModal();
}

void MessageBox2::OnOK() 
{
	// TODO: Add extra validation here
	CDialog::OnOK();

	if (!m_registryKey.IsEmpty() && m_doNotAskAgain.GetCheck())
	{
		CRegKey regKey;
		regKey.Create (HKEY_CURRENT_USER, ms_registryFolder);
#if _MSC_VER < 1300
		regKey.SetValue(1, m_registryKey);
#else
		regKey.SetDWORDValue (m_registryKey, 1);
#endif
	}
}

void MessageBox2::OnCancel() 
{
	CDialog::OnCancel();
}

void MessageBox2::OnWeb() 
{
	if (!m_url.IsEmpty())
		ShellExecute(NULL, _T("open"), m_url, _T(""), _T("c:\\"), SW_SHOWNORMAL);
}

BOOL MessageBox2::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_okayText.IsEmpty())
		m_okay.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
	else
		m_okay.SetWindowText(m_okayText);
	
	if (m_cancelText.IsEmpty())
		m_cancel.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
	else
		m_cancel.SetWindowText(m_cancelText);
	
	if (m_webText.IsEmpty())
		m_goToWebPage.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
	else
		m_goToWebPage.SetWindowText(m_webText);
		
	if (m_doNotShowText.IsEmpty())
		m_doNotAskAgain.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
	else
		m_doNotAskAgain.SetWindowText(m_doNotShowText);

	return TRUE;
}
