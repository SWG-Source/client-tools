// ======================================================================
//
// DialogDiff.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "DialogDiff.h"

// ======================================================================

DialogDiff::DialogDiff(CString const & stringId, CString const & stringTable1FileName, CString const & string1, CString const & stringTable2FileName, CString const & string2, CWnd* pParent /*=NULL*/) : 
	CDialog(DialogDiff::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogDiff)
	m_string1 = _T("");
	m_string2 = _T("");
	m_stringTable1 = _T("");
	m_stringTable2 = _T("");
	//}}AFX_DATA_INIT

	m_string1 = string1;
	m_stringTable1 = stringTable1FileName;
	m_string2 = string2;
	m_stringTable2 = stringTable2FileName;
	m_stringIdText = stringId;
}

// ----------------------------------------------------------------------

void DialogDiff::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDiff)
	DDX_Control(pDX, IDC_STATIC_STRINGID, m_stringId);
	DDX_Text(pDX, IDC_EDIT_STRINGTABLE1, m_string1);
	DDX_Text(pDX, IDC_EDIT_STRINGTABLE2, m_string2);
	DDX_Text(pDX, IDC_STATIC_STRINGTABLE1, m_stringTable1);
	DDX_Text(pDX, IDC_STATIC_STRINGTABLE2, m_stringTable2);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogDiff, CDialog)
	//{{AFX_MSG_MAP(DialogDiff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogDiff::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_stringId.SetWindowText(m_stringIdText);
	UpdateData(false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ======================================================================

