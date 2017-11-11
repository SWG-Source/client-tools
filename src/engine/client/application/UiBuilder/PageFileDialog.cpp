// PageFileDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "PageFileDialog.h"

#include "UIPage.h"

/////////////////////////////////////////////////////////////////////////////
// PageFileDialog dialog


PageFileDialog::PageFileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(PageFileDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(PageFileDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void PageFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageFileDialog)
	DDX_Control(pDX, IDC_PAGEFILE, m_pageFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PageFileDialog, CDialog)
	//{{AFX_MSG_MAP(PageFileDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_NEWFILE, OnNewfile)
	ON_BN_CLICKED(IDC_PARENT, OnParent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PageFileDialog message handlers

BOOL PageFileDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CheckRadioButton(IDC_PARENT, IDC_NEWFILE, IDC_PARENT);
	m_pageFile.EnableWindow(FALSE);
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PageFileDialog::OnClose() 
{
	// Equivalent to pressing cancel
	EndDialog(0);
}

void PageFileDialog::OnOK() 
{
	UIPage *thePage = new UIPage;

	if (IsDlgButtonChecked(IDC_NEWFILE))
	{
		CString pageFile;
		m_pageFile.GetWindowText(pageFile);
		thePage->SetProperty( UIBaseObject::PropertyName::SourceFile, UIUnicode::narrowToWide(static_cast<const char *>(pageFile)));
	}
	EndDialog((long)thePage);
}

void PageFileDialog::OnCancel() 
{
	EndDialog(0);
}

void PageFileDialog::OnNewfile() 
{
	m_pageFile.EnableWindow(TRUE);
}

void PageFileDialog::OnParent() 
{
	m_pageFile.EnableWindow(FALSE);
}
