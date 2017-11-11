// ProgressDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "ProgressDialog.h"

/////////////////////////////////////////////////////////////////////////////
// ProgressDialog dialog


ProgressDialog::ProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ProgressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(ProgressDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create(ProgressDialog::IDD, pParent);
}


void ProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ProgressDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ProgressDialog, CDialog)
	//{{AFX_MSG_MAP(ProgressDialog)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ProgressDialog message handlers

void ProgressDialog::PostNcDestroy() 
{
	delete this;
}

void ProgressDialog::OnClose() 
{
	DestroyWindow();
}
