// OutputDialog.cpp : implementation file
//

#include "FirstViewer.h"
#include "viewer.h"
#include "OutputDialog.h"

/////////////////////////////////////////////////////////////////////////////
// OutputDialog dialog


OutputDialog::OutputDialog(CWnd* pParent /*=NULL*/)
	: CDialog(OutputDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(OutputDialog)
	m_output = _T("");
	//}}AFX_DATA_INIT

	initialized = FALSE;
}


void OutputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OutputDialog)
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputWindow);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_output);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OutputDialog, CDialog)
	//{{AFX_MSG_MAP(OutputDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OutputDialog message handlers

BOOL OutputDialog::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL OutputDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);
	
	initialized = TRUE;

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void OutputDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if(!initialized)
		return;

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);

	m_outputWindow.MoveWindow(sz);
}

void OutputDialog::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}

void OutputDialog::clearMessage (void)
{
	m_output = "";
	UpdateData (false);
}

void OutputDialog::addMessage (const char* message)
{
	m_output += message;
	UpdateData (false);

	m_outputWindow.LineScroll (m_outputWindow.GetLineCount ());
}
