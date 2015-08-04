// ======================================================================
//
// DialogProperties.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "DialogProperties.h"

#include "Configuration.h"

// ======================================================================

DialogProperties::DialogProperties(CString const & originalObjVars, CString const & currentObjVars, CString const & scripts) :
	CDialog(DialogProperties::IDD, 0),
	m_originalObjVars(originalObjVars),
	m_currentObjVars(currentObjVars),
	m_scripts(scripts)
{
	//{{AFX_DATA_INIT(DialogProperties)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogProperties)
	DDX_Text(pDX, IDC_EDIT_SCRIPTS, m_scripts);
	DDX_Text(pDX, IDC_EDIT_CURRENT_OBJVARS, m_currentObjVars);
	DDX_Text(pDX, IDC_EDIT_ORIGINAL_OBJVARS, m_originalObjVars);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogProperties, CDialog)
	//{{AFX_MSG_MAP(DialogProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateData(false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogProperties::OnOK()
{
	UpdateData(true);

	CDialog::OnOK();
}

// ======================================================================
