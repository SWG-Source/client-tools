// PropertyDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "PropertyDialog.h"

#include <afxadv.h>
#include "multimon.h" // HEADER for multi-monitor support on WINVER 4 compiles.

/////////////////////////////////////////////////////////////////////////////
// PropertyDialog dialog


PropertyDialog::PropertyDialog(int ID, CPoint anchorPosition, CWnd* pParent)
:	CDialog(ID, pParent),
	m_anchorPosition(anchorPosition)
{
	//{{AFX_DATA_INIT(PropertyDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void PropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PropertyDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PropertyDialog, CDialog)
	//{{AFX_MSG_MAP(PropertyDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PropertyDialog message handlers

void PropertyDialog::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
}

void PropertyDialog::OnOK()
{
	CDialog::OnOK();
}

void PropertyDialog::OnCancel()
{
	CDialog::OnCancel();
}

BOOL PropertyDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect startupRect;
	_getStartupRect(startupRect);
	const int width = startupRect.right;
	const int height = startupRect.bottom;
	startupRect.bottom = startupRect.top + height;
	startupRect.right = startupRect.left + width;

	CRect desktopRect;
	desktopRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	desktopRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	desktopRect.right = desktopRect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	desktopRect.bottom = desktopRect.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

	if (startupRect.right>desktopRect.right)
	{
		startupRect.right=desktopRect.right;
		startupRect.left=startupRect.right - width;
	}
	if (startupRect.bottom>desktopRect.bottom)
	{
		startupRect.bottom=desktopRect.bottom;
		startupRect.top=startupRect.bottom - height;
	}
	if (startupRect.left<0)
	{
		startupRect.left=0;
		startupRect.right=startupRect.left + width;
	}
	if (startupRect.top<0)
	{
		startupRect.top=0;
		startupRect.bottom=startupRect.top + height;
	}

	MoveWindow(startupRect.left, startupRect.top, startupRect.Width(), startupRect.Height());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
