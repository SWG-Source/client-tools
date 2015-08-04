// DialogFind.cpp : implementation file
//

#include "FirstViewer.h"
#include "viewer.h"
#include "DialogFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogFind dialog


DialogFind::DialogFind(CWnd* pParent /*=NULL*/)
	: CDialog(DialogFind::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogFind)
	m_name = _T("");
	//}}AFX_DATA_INIT
}


void DialogFind::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFind)
	DDX_Text(pDX, IDC_EDIT1, m_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DialogFind, CDialog)
	//{{AFX_MSG_MAP(DialogFind)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogFind message handlers

