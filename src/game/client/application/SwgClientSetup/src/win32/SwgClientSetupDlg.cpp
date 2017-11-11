// ======================================================================
//
// SwgClientSetupDlg.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "SwgClientSetupDlg.h"

#include "SwgClientSetup.h"

#include <string>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

SwgClientSetupDlg::SwgClientSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SwgClientSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SwgClientSetupDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ----------------------------------------------------------------------

void SwgClientSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SwgClientSetupDlg)
	DDX_Control(pDX, IDC_BUTTON_DELETE, m_resetInGame);	
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(SwgClientSetupDlg, CDialog)
	//{{AFX_MSG_MAP(SwgClientSetupDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL SwgClientSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_propertySheet.AddPage (&m_pageInformation);
	m_propertySheet.AddPage (&m_pageGraphics);
	m_propertySheet.AddPage (&m_pageSound);
	m_propertySheet.AddPage (&m_pageGame);
	m_propertySheet.AddPage (&m_pageAdvanced);
	m_propertySheet.AddPage (&m_pageDebug);

	m_propertySheet.Create (this, WS_CHILD | WS_VISIBLE, 0);
	m_propertySheet.ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_propertySheet.ModifyStyle (0, WS_TABSTOP);

	CString strCaption;
	VERIFY(strCaption.LoadString(IDS_RESET_IN_GAME));
	m_resetInGame.SetWindowText(strCaption);
	TCITEMW tcItem;
	VERIFY(strCaption.LoadString(IDS_INFORMATION_CAPTION));	
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(0, &tcItem);
	VERIFY(strCaption.LoadString(IDS_GRAPHICS_CAPTION));
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(1, &tcItem);
	VERIFY(strCaption.LoadString(IDS_SOUND_CAPTION));
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(2, &tcItem);
	VERIFY(strCaption.LoadString(IDS_GAME_CAPTION));
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(3, &tcItem);
	VERIFY(strCaption.LoadString(IDS_ADVANCED_CAPTION));
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(4, &tcItem);
	VERIFY(strCaption.LoadString(IDS_DEBUG_CAPTION));
	tcItem.pszText = const_cast<LPWSTR>(static_cast<LPCTSTR>(strCaption));
	m_propertySheet.GetTabControl()->SetItem(5, &tcItem);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------

void SwgClientSetupDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ----------------------------------------------------------------------

HCURSOR SwgClientSetupDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ----------------------------------------------------------------------

void SwgClientSetupDlg::OnButtonDelete() 
{
	remove ("local_machine_options.iff");
	CString resetString;
	VERIFY(resetString.LoadString(IDS_RESET_IN_GAME_OPTIONS_MSG));
	MessageBox (resetString);
}

// ======================================================================

