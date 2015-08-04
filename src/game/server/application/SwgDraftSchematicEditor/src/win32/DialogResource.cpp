// ======================================================================
//
// DialogResource.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "DialogResource.h"

#include "Configuration.h"

// ======================================================================

DialogResource::DialogResource(CWnd* pParent /*=NULL*/)
	: CDialog(DialogResource::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogResource)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogResource::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogResource)
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_TREE_RESOURCE, m_treeCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogResource, CDialog)
	//{{AFX_MSG_MAP(DialogResource)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

CTreeCtrl & DialogResource::GetTreeCtrl ()
{
	return m_treeCtrl;
}

// ----------------------------------------------------------------------

BOOL DialogResource::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	Configuration::populateResourceTypes (m_treeCtrl);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ======================================================================

void DialogResource::OnOK() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	if (treeItem)
		m_selection = GetTreeCtrl ().GetItemText (treeItem);
	
	CDialog::OnOK();
}
