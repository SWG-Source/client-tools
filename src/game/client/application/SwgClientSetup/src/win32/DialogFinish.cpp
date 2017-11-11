// ======================================================================
//
// DialogFinish.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogFinish.h"

#include "SwgClientSetup.h"

// ======================================================================

DialogFinish::DialogFinish(CWnd* pParent /*=NULL*/)
	: CDialog(DialogFinish::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogFinish)
	m_information = _T("");

	m_lblBtnFinish = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogFinish::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFinish)
	DDX_Text(pDX, IDC_STATIC_INFORMATION, m_information);
	DDX_Text(pDX, IDC_BUTTON_PROCEED, m_lblBtnFinish);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogFinish, CDialog)
	//{{AFX_MSG_MAP(DialogFinish)
	ON_BN_CLICKED(IDC_BUTTON_PROCEED, OnButtonProceed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogFinish::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	VERIFY(m_information.LoadString(IDS_CONFIGCOMPLETE_INFO));
	VERIFY(m_lblBtnFinish.LoadString(IDS_BTN_FINISH));

	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogFinish::OnButtonProceed() 
{
	OnOK ();
}

// ======================================================================

