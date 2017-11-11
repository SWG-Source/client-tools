// ======================================================================
//
// DialogSpaceMobile.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogSpaceMobile.h"

#include "Configuration.h"

// ======================================================================

DialogSpaceMobile::DialogSpaceMobile(CString const & spaceMobile)
	: CDialog(DialogSpaceMobile::IDD, 0),
	m_spaceMobile(spaceMobile)
{
	//{{AFX_DATA_INIT(DialogSpaceMobile)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogSpaceMobile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSpaceMobile)
	DDX_Control(pDX, IDC_LIST_SPACEMOBILE, m_spaceMobileListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogSpaceMobile, CDialog)
	//{{AFX_MSG_MAP(DialogSpaceMobile)
	ON_LBN_DBLCLK(IDC_LIST_SPACEMOBILE, OnDblclkListSpacemobile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogSpaceMobile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int selection = 0;
	for (int i = 0; i < Configuration::getNumberOfSpaceMobiles(); ++i)
	{
		CString const & spaceMobile = Configuration::getSpaceMobile(i);
		m_spaceMobileListBox.AddString(spaceMobile);
		if (m_spaceMobile == spaceMobile)
			selection = i;
	}

	m_spaceMobileListBox.SetCurSel(selection);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

CString const & DialogSpaceMobile::getSpaceMobile() const
{
	return m_spaceMobile;
}

// ----------------------------------------------------------------------

void DialogSpaceMobile::OnOK()
{
	UpdateData(false);

	int const selection = m_spaceMobileListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_spaceMobileListBox.GetText(selection, m_spaceMobile);

		CDialog::OnOK();
	}
}

// ----------------------------------------------------------------------

void DialogSpaceMobile::OnDblclkListSpacemobile() 
{
	OnOK();
}

// ======================================================================
