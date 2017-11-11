// ======================================================================
//
// DialogSpawnerList.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogSpawnerList.h"

#include "DialogSpawner.h"

// ======================================================================

DialogSpawnerList::DialogSpawnerList(CString const & spawnerList)
	: CDialog(DialogSpawnerList::IDD, 0),
	m_spawnerList(spawnerList)
{
	//{{AFX_DATA_INIT(DialogSpawnerList)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogSpawnerList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSpawnerList)
	DDX_Control(pDX, IDC_LIST_SPACEMOBILE, m_spawnerListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogSpawnerList, CDialog)
	//{{AFX_MSG_MAP(DialogSpawnerList)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveall)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

BOOL DialogSpawnerList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText("Spawner List");

	// TODO: Add extra initialization here
	while (!m_spawnerList.IsEmpty())
	{
		int index = m_spawnerList.Find('|');
		if (index == -1)
		{
			m_spawnerListBox.AddString(m_spawnerList);
			m_spawnerList.Empty();
		}
		else
		{
			m_spawnerListBox.AddString(m_spawnerList.Left(index));
			m_spawnerList = m_spawnerList.Right(m_spawnerList.GetLength() - index - 1);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

CString const & DialogSpawnerList::getSpawnerList() const
{
	return m_spawnerList;
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonAdd() 
{
	CString text;

	if (m_spawnerListBox.GetCurSel() != LB_ERR)
		m_spawnerListBox.GetText(m_spawnerListBox.GetCurSel(), text);

	DialogSpawner dlg(text);
	if (dlg.DoModal() == IDOK)
	{
		int const index = m_spawnerListBox.InsertString(m_spawnerListBox.GetCount(), dlg.getSpawner());
		m_spawnerListBox.SetCurSel(index);
	}
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonDown() 
{
	int const numberOfStrings = m_spawnerListBox.GetCount();
	int const selection = m_spawnerListBox.GetCurSel();
	if (selection != LB_ERR && selection < numberOfStrings - 1)
	{
		CString text;
		m_spawnerListBox.GetText(selection, text);
		m_spawnerListBox.DeleteString(selection);
		m_spawnerListBox.InsertString(selection + 1, text);
		m_spawnerListBox.SetCurSel(selection + 1);
	}
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonRemove() 
{
	int const selection = m_spawnerListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_spawnerListBox.DeleteString(selection);
		m_spawnerListBox.SetCurSel(selection == m_spawnerListBox.GetCount() ? selection - 1 : selection);
	}
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonRemoveall() 
{
	if (MessageBox("Are you sure you wish to remove all spawner entries?", "Remove All", MB_YESNO) == IDYES)
		m_spawnerListBox.ResetContent();
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonUp() 
{
	int const selection = m_spawnerListBox.GetCurSel();
	if (selection != LB_ERR && selection > 0)
	{
		CString text;
		m_spawnerListBox.GetText(selection, text);
		m_spawnerListBox.DeleteString(selection);
		m_spawnerListBox.InsertString(selection - 1, text);
		m_spawnerListBox.SetCurSel(selection - 1);
	}
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnOK()
{
	UpdateData(false);

	m_spawnerList.Empty();

	for (int i = 0; i < m_spawnerListBox.GetCount(); ++i)
	{
		if (i != 0)
			m_spawnerList += '|';

		CString text;
		m_spawnerListBox.GetText(i, text);

		m_spawnerList += text;
	}

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

void DialogSpawnerList::OnButtonEdit() 
{
	int const selection = m_spawnerListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		CString text;
		m_spawnerListBox.GetText(selection, text);

		DialogSpawner dlg(text);
		if (dlg.DoModal() == IDOK)
		{
			m_spawnerListBox.DeleteString(selection);
			m_spawnerListBox.InsertString(selection, dlg.getSpawner());
			m_spawnerListBox.SetCurSel(selection);
		}
	}
}

// ======================================================================
