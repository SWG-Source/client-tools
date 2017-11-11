// ======================================================================
//
// DialogSelectNpc.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "DialogSelectNpc.h"

#include "Configuration.h"

// ======================================================================

DialogSelectNpc::DialogSelectNpc(NpcType const npcType, CWnd* pParent /*=NULL*/)
	: CDialog(DialogSelectNpc::IDD, pParent),
	m_npcType (npcType)
{
	//{{AFX_DATA_INIT(DialogSelectNpc)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogSelectNpc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSelectNpc)
	DDX_Control(pDX, IDC_TREE_NPC, m_treeCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogSelectNpc, CDialog)
	//{{AFX_MSG_MAP(DialogSelectNpc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogSelectNpc::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (m_npcType == NT_giver)
		Configuration::populateNpcGiverEntries (m_treeCtrl);
	else
		Configuration::populateNpcTargetEntries (m_treeCtrl);

	m_treeCtrl.SelectItem (m_treeCtrl.GetRootItem ());
	m_treeCtrl.EnsureVisible (m_treeCtrl.GetRootItem ());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogSelectNpc::OnOK() 
{
	// TODO: Add extra validation here
	m_selection = m_treeCtrl.GetItemText (m_treeCtrl.GetSelectedItem ());
	
	CDialog::OnOK();
}

// ======================================================================

