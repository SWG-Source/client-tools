// SetBrightnessContrastGammaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetBrightnessContrastGamma.h"
#include "SetBrightnessContrastGammaDlg.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetBrightnessContrastGammaDlg dialog

CSetBrightnessContrastGammaDlg::CSetBrightnessContrastGammaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetBrightnessContrastGammaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetBrightnessContrastGammaDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSetBrightnessContrastGammaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetBrightnessContrastGammaDlg)
	DDX_Control(pDX, IDC_GAMMA, m_gamma);
	DDX_Control(pDX, IDC_CONTRAST, m_contrast);
	DDX_Control(pDX, IDC_BRIGHTNESS, m_brightness);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetBrightnessContrastGammaDlg, CDialog)
	//{{AFX_MSG_MAP(CSetBrightnessContrastGammaDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnApply)
	ON_BN_CLICKED(IDCANCEL, OnQuit)
	ON_BN_CLICKED(IDMINIMIZE, OnMinimize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetBrightnessContrastGammaDlg message handlers

BOOL CSetBrightnessContrastGammaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_brightness.SetWindowText("1.1");
	m_contrast.SetWindowText("1.3");
	m_gamma.SetWindowText("1.0");
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSetBrightnessContrastGammaDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSetBrightnessContrastGammaDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// BOOL WINAPI SetDeviceGammaRamp(HDC hDC, LPVOID lpRamp);

inline int clamp(int low, int value, int high)
{
	if (value < low)
		return low;
	if (value > high)
		return high;
	return value;
}

void CSetBrightnessContrastGammaDlg::buildRamp(float brightness, float contrast, float gamma)
{
	int i = 0;
	float f = 0.0f;
	float const step = 1.0 / 256.0f;
	float oneOverGamma = 1.0f / gamma;
	for ( ; i < 256; ++i, f += step)
	{
		float result = static_cast<float>(pow(0.5f + contrast * ((f * brightness) - 0.5f), oneOverGamma));
		WORD w = static_cast<WORD>(clamp(0, static_cast<int>(result * 65535.0f), 65535));
		m_ramp[i +   0] = w;
		m_ramp[i + 256] = w;
		m_ramp[i + 512] = w;
	}
}

void CSetBrightnessContrastGammaDlg::OnApply() 
{
	CString s;
	
	m_brightness.GetWindowText(s);
	float const brightness = static_cast<float>(atof(s));

	m_contrast.GetWindowText(s);
	float const contrast = static_cast<float>(atof(s));

	m_gamma.GetWindowText(s);
	float const gamma = static_cast<float>(atof(s));

	buildRamp(brightness, contrast, gamma);
	SetDeviceGammaRamp(::GetDC(::GetDesktopWindow()), m_ramp);
}

void CSetBrightnessContrastGammaDlg::OnQuit() 
{
	buildRamp(1.0f, 1.0f, 1.0f);
	SetDeviceGammaRamp(::GetDC(::GetDesktopWindow()), m_ramp);
	CDialog::OnCancel();	
}

void CSetBrightnessContrastGammaDlg::OnMinimize() 
{
	ShowWindow(SW_MINIMIZE);
}
