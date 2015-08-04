// ======================================================================
//
// DialogSpawner.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogSpawner.h"

#include "Configuration.h"

// ======================================================================

namespace DialogSpawnerNamespace
{
	HTREEITEM find(CTreeCtrl & treeCtrl, HTREEITEM hItem, CString const & text) 
	{
		HTREEITEM theItem = NULL;

		if (hItem == NULL) 
			return NULL;

		if ((theItem = treeCtrl.GetNextSiblingItem(hItem)) != NULL) 
		{
			theItem = find(treeCtrl, theItem, text);

			if (theItem != NULL) 
				return theItem;
		}

		if ((theItem = treeCtrl.GetChildItem(hItem)) != NULL) 
		{		
			theItem = find(treeCtrl, theItem, text);

			if (theItem != NULL) 
				return theItem;
		}

		if (treeCtrl.GetItemText(hItem) == text)
			return hItem;

		return theItem;
	}
}

using namespace DialogSpawnerNamespace;

// ======================================================================

DialogSpawner::DialogSpawner(CString const & spawner)
	: CDialog(DialogSpawner::IDD, 0),
	m_spawner(spawner)
{
	//{{AFX_DATA_INIT(DialogSpawner)
	m_custom = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogSpawner::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSpawner)
	DDX_Control(pDX, IDC_TREE_NAVPOINT, m_treeCtrl);
	DDX_Text(pDX, IDC_EDIT_CUSTOM, m_custom);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogSpawner, CDialog)
	//{{AFX_MSG_MAP(DialogSpawner)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_NAVPOINT, OnDblclkTreeNavpoint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogSpawner::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText("Choose spawner");

	Configuration::populateSpawners(m_treeCtrl);

	StringList stringList;
	Configuration::unpackString(m_spawner, stringList, ':');
	if (stringList.size() == 2)
	{
		m_treeCtrl.SelectItem(find(m_treeCtrl, m_treeCtrl.GetRootItem(), stringList[1]));
		m_treeCtrl.SetFocus();
	}

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

CString const & DialogSpawner::getSpawner() const
{
	return !m_custom.IsEmpty() ? m_custom : m_spawner;
}

// ----------------------------------------------------------------------

void DialogSpawner::OnOK()
{
	UpdateData(true);

	if (!m_custom.IsEmpty())
		CDialog::OnOK();

	HTREEITEM treeItem = m_treeCtrl.GetSelectedItem();
	if (treeItem)
	{
		HTREEITEM parentTreeItem = m_treeCtrl.GetParentItem(treeItem);
		if (parentTreeItem)
		{
			m_spawner = m_treeCtrl.GetItemText(parentTreeItem) + ':' + m_treeCtrl.GetItemText(treeItem);
	
			CDialog::OnOK();
		}
	}
}

// ----------------------------------------------------------------------

void DialogSpawner::OnDblclkTreeNavpoint(NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	// TODO: Add your control notification handler code here
	OnOK();
	
	*pResult = 0;
}

// ======================================================================

