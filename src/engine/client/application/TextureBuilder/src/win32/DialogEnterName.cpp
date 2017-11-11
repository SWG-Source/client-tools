// ======================================================================
//
// DialogEnterName.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "DialogEnterName.h"

#include "resource.h"

#include <string>

// ======================================================================

DialogEnterName::DialogEnterName(const std::string &name, CWnd* pParent)
	: CDialog(IDD_ENTER_NAME, pParent),
		m_name(name.c_str())
{
	/*
	//{{AFX_DATA_INIT(DialogEnterName)
	m_name = _T("");
	//}}AFX_DATA_INIT
	*/
}

// ----------------------------------------------------------------------

void DialogEnterName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogEnterName)
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogEnterName, CDialog)
	//{{AFX_MSG_MAP(DialogEnterName)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================
