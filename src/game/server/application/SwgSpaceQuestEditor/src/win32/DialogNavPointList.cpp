// ======================================================================
//
// DialogNavPointList.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogNavPointList.h"

#include "DialogNavPoint.h"

// ======================================================================

DialogNavPointList::DialogNavPointList(CString const & navPointList)
	: CDialog(DialogNavPointList::IDD, 0),
	m_navPointList(navPointList)
{
	//{{AFX_DATA_INIT(DialogNavPointList)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogNavPointList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogNavPointList)
	DDX_Control(pDX, IDC_LIST_SPACEMOBILE, m_navPointListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogNavPointList, CDialog)
	//{{AFX_MSG_MAP(DialogNavPointList)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveall)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

BOOL DialogNavPointList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText("Nav Point List");

	// TODO: Add extra initialization here
	while (!m_navPointList.IsEmpty())
	{
		int index = m_navPointList.Find('|');
		if (index == -1)
		{
			m_navPointListBox.AddString(m_navPointList);
			m_navPointList.Empty();
		}
		else
		{
			m_navPointListBox.AddString(m_navPointList.Left(index));
			m_navPointList = m_navPointList.Right(m_navPointList.GetLength() - index - 1);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

CString const & DialogNavPointList::getNavPointList() const
{
	return m_navPointList;
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonAdd() 
{
	CString text;

	if (m_navPointListBox.GetCurSel() != LB_ERR)
		m_navPointListBox.GetText(m_navPointListBox.GetCurSel(), text);

	DialogNavPoint dlg(text);
	if (dlg.DoModal() == IDOK)
	{
		int const index = m_navPointListBox.InsertString(m_navPointListBox.GetCount(), dlg.getNavPoint());
		m_navPointListBox.SetCurSel(index);
	}
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonDown() 
{
	int const numberOfStrings = m_navPointListBox.GetCount();
	int const selection = m_navPointListBox.GetCurSel();
	if (selection != LB_ERR && selection < numberOfStrings - 1)
	{
		CString text;
		m_navPointListBox.GetText(selection, text);
		m_navPointListBox.DeleteString(selection);
		m_navPointListBox.InsertString(selection + 1, text);
		m_navPointListBox.SetCurSel(selection + 1);
	}
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonRemove() 
{
	int const selection = m_navPointListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_navPointListBox.DeleteString(selection);
		m_navPointListBox.SetCurSel(selection == m_navPointListBox.GetCount() ? selection - 1 : selection);
	}
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonRemoveall() 
{
	if (MessageBox("Are you sure you wish to remove all nav point entries?", "Remove All", MB_YESNO) == IDYES)
		m_navPointListBox.ResetContent();
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonUp() 
{
	int const selection = m_navPointListBox.GetCurSel();
	if (selection != LB_ERR && selection > 0)
	{
		CString text;
		m_navPointListBox.GetText(selection, text);
		m_navPointListBox.DeleteString(selection);
		m_navPointListBox.InsertString(selection - 1, text);
		m_navPointListBox.SetCurSel(selection - 1);
	}
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnOK()
{
	UpdateData(false);

	m_navPointList.Empty();

	for (int i = 0; i < m_navPointListBox.GetCount(); ++i)
	{
		if (i != 0)
			m_navPointList += '|';

		CString text;
		m_navPointListBox.GetText(i, text);

		m_navPointList += text;
	}

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

void DialogNavPointList::OnButtonEdit() 
{
	int const selection = m_navPointListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		CString text;
		m_navPointListBox.GetText(selection, text);

		DialogNavPoint dlg(text);
		if (dlg.DoModal() == IDOK)
		{
			m_navPointListBox.DeleteString(selection);
			m_navPointListBox.InsertString(selection, dlg.getNavPoint());
			m_navPointListBox.SetCurSel(selection);
		}
	}
}

// ======================================================================

