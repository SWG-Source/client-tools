// ======================================================================
//
// DialogStringEdit.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "DialogStringEdit.h"

#include "Configuration.h"

// ======================================================================

DialogStringEdit::DialogStringEdit(CString const & stringId, CString const & string)
	: CDialog(DialogStringEdit::IDD, 0),
	m_stringId(stringId),
	m_string(string)
{
	//{{AFX_DATA_INIT(DialogStringEdit)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogStringEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogStringEdit)
	DDX_Control(pDX, IDC_BUTTON3, m_button3);
	DDX_Control(pDX, IDC_BUTTON2, m_button2);
	DDX_Control(pDX, IDC_BUTTON1, m_button1);
	DDX_Control(pDX, IDC_BUTTON0, m_button0);
	DDX_Control(pDX, IDC_EDIT1, m_stringCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogStringEdit, CDialog)
	//{{AFX_MSG_MAP(DialogStringEdit)
	ON_BN_CLICKED(IDC_BUTTON0, OnButton0)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

CString const & DialogStringEdit::getString() const
{
	return m_string;
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnButton(CButton const & button) 
{
	for (int i = 0; i < m_numberOfButtons; ++i)
		if (m_buttonList[i] == &button)
			m_stringCtrl.ReplaceSel(m_buttonData[i], true);
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnButton0() 
{
	OnButton(m_button0);
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnButton1() 
{
	OnButton(m_button1);
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnButton2() 
{
	OnButton(m_button2);
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnButton3() 
{
	OnButton(m_button3);
}

// ----------------------------------------------------------------------

BOOL DialogStringEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_stringId);
	m_stringCtrl.SetWindowText(m_string);

	m_buttonList[0] = &m_button0;
	m_buttonList[1] = &m_button1;
	m_buttonList[2] = &m_button2;
	m_buttonList[3] = &m_button3;

	m_numberOfButtons = std::min(4, 0);
	for (int i = 0; i < 4; ++i)
	{
//		if (i < m_numberOfButtons)
//		{
//			m_buttonList[i]->SetWindowText(Configuration::getStringButtonName(i));
//			m_buttonData[i] = Configuration::getStringButtonData(i);
//		}
//		else
		{
			m_buttonList[i]->EnableWindow(false);
			m_buttonList[i]->ShowWindow(SW_HIDE);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogStringEdit::OnOK()
{
	m_stringCtrl.GetWindowText(m_string);

	CDialog::OnOK();
}

// ======================================================================

