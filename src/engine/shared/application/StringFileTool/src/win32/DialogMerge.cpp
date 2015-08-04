// ======================================================================
//
// DialogMerge.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "DialogMerge.h"

// ======================================================================

DialogMerge::DialogMerge(CString const & stringId, CString const & original, CString const & theirs, CString const & yours, CWnd* pParent /*=NULL*/)
	: CDialog(DialogMerge::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogMerge)
	m_original = _T("");
	m_theirs = _T("");
	m_yours = _T("");
	m_merged = _T("");
	//}}AFX_DATA_INIT

	m_stringIdText = stringId;
	m_original = original;
	m_theirs = theirs;
	m_yours = yours;
	m_merged = yours;
}

// ----------------------------------------------------------------------

void DialogMerge::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogMerge)
	DDX_Control(pDX, IDC_STATIC_STRINGID, m_stringId);
	DDX_Text(pDX, IDC_EDIT_ORIGINAL, m_original);
	DDX_Text(pDX, IDC_EDIT_THEIRS, m_theirs);
	DDX_Text(pDX, IDC_EDIT_YOURS, m_yours);
	DDX_Text(pDX, IDC_EDIT_MERGED, m_merged);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogMerge, CDialog)
	//{{AFX_MSG_MAP(DialogMerge)
	ON_BN_CLICKED(IDC_BUTTON_ORIGINAL, OnButtonOriginal)
	ON_BN_CLICKED(IDC_BUTTON_THEIRS, OnButtonTheirs)
	ON_BN_CLICKED(IDC_BUTTON_YOURS, OnButtonYours)
	ON_BN_CLICKED(IDC_BUTTON_MERGED, OnButtonMerged)
	ON_EN_CHANGE(IDC_EDIT_MERGED, OnChangeEditMerged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogMerge::OnButtonOriginal() 
{
	m_merged = m_original;
	UpdateData(false);
	OnOK();
}

// ----------------------------------------------------------------------

void DialogMerge::OnButtonTheirs() 
{
	m_merged = m_theirs;
	UpdateData(false);
	OnOK();
}

// ----------------------------------------------------------------------

void DialogMerge::OnButtonYours() 
{
	m_merged = m_yours;
	UpdateData(false);
	OnOK();
}

// ----------------------------------------------------------------------

void DialogMerge::OnButtonMerged() 
{
	UpdateData(true);
	OnOK();
}

// ----------------------------------------------------------------------

void DialogMerge::OnChangeEditMerged() 
{
	UpdateData(true);
}

// ----------------------------------------------------------------------

BOOL DialogMerge::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_stringId.SetWindowText(m_stringIdText);
	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogMerge::OnOK() 
{
	CDialog::OnOK();
}

// ======================================================================

