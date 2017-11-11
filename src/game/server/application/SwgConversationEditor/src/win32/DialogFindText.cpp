// ======================================================================
//
// DialogFindText.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "DialogFindText.h"

// ======================================================================

DialogFindText::DialogFindText() : 
	CDialog(DialogFindText::IDD, NULL)
{
	//{{AFX_DATA_INIT(DialogFindText)
	m_matchCase = FALSE;
	m_wholeWord = FALSE;
	m_text = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogFindText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFindText)
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Check(pDX, IDC_EDIT_MATCH_CASE, m_matchCase);
	DDX_Text(pDX, IDC_EDIT_TEXT, m_text);
	DDX_Check(pDX, IDC_EDIT_WHOLE_WORD, m_wholeWord);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogFindText, CDialog)
	//{{AFX_MSG_MAP(DialogFindText)
	ON_EN_CHANGE(IDC_EDIT_TEXT, OnChangeEditText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogFindText::OnOK() 
{
	UpdateData (true);

	CDialog::OnOK ();
}

// ----------------------------------------------------------------------

void DialogFindText::OnChangeEditText() 
{
	UpdateData(true);

	m_okButton.EnableWindow(!m_text.IsEmpty());
}

// ----------------------------------------------------------------------

BOOL DialogFindText::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_okButton.EnableWindow(!m_text.IsEmpty());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ======================================================================

