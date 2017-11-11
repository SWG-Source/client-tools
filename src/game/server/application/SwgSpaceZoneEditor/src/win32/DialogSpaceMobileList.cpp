// ======================================================================
//
// DialogSpaceMobileList.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "DialogSpaceMobileList.h"

#include "DialogSpaceMobile.h"

// ======================================================================

DialogSpaceMobileList::DialogSpaceMobileList(char const separator, CString const & spaceMobileList) : 
	CDialog(DialogSpaceMobileList::IDD, 0),
	m_separator(separator),
	m_spaceMobileList(spaceMobileList)
{
	//{{AFX_DATA_INIT(DialogSpaceMobileList)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSpaceMobileList)
	DDX_Control(pDX, IDC_LIST_SPACEMOBILE, m_spaceMobileListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogSpaceMobileList, CDialog)
	//{{AFX_MSG_MAP(DialogSpaceMobileList)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveall)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

BOOL DialogSpaceMobileList::OnInitDialog() 
{
	CDialog::OnInitDialog();

	while (!m_spaceMobileList.IsEmpty())
	{
		int index = m_spaceMobileList.Find(m_separator);
		if (index == -1)
		{
			m_spaceMobileListBox.AddString(m_spaceMobileList);
			m_spaceMobileList.Empty();
		}
		else
		{
			m_spaceMobileListBox.AddString(m_spaceMobileList.Left(index));
			m_spaceMobileList = m_spaceMobileList.Right(m_spaceMobileList.GetLength() - index - 1);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

CString const & DialogSpaceMobileList::getSpaceMobileList() const
{
	return m_spaceMobileList;
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonAdd() 
{
	CString text;

	if (m_spaceMobileListBox.GetCurSel() != LB_ERR)
		m_spaceMobileListBox.GetText(m_spaceMobileListBox.GetCurSel(), text);

	DialogSpaceMobile dlg(text);
	if (dlg.DoModal() == IDOK)
	{
		int const index = m_spaceMobileListBox.InsertString(m_spaceMobileListBox.GetCount(), dlg.getSpaceMobile());
		m_spaceMobileListBox.SetCurSel(index);
	}
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonDown() 
{
	int const numberOfStrings = m_spaceMobileListBox.GetCount();
	int const selection = m_spaceMobileListBox.GetCurSel();
	if (selection != LB_ERR && selection < numberOfStrings - 1)
	{
		CString text;
		m_spaceMobileListBox.GetText(selection, text);
		m_spaceMobileListBox.DeleteString(selection);
		m_spaceMobileListBox.InsertString(selection + 1, text);
		m_spaceMobileListBox.SetCurSel(selection + 1);
	}
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonRemove() 
{
	int const selection = m_spaceMobileListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_spaceMobileListBox.DeleteString(selection);
		m_spaceMobileListBox.SetCurSel(selection == m_spaceMobileListBox.GetCount() ? selection - 1 : selection);
	}
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonRemoveall() 
{
	if (MessageBox("Are you sure you wish to remove all space_mobile entries?", "Remove All", MB_YESNO) == IDYES)
		m_spaceMobileListBox.ResetContent();
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonUp() 
{
	int const selection = m_spaceMobileListBox.GetCurSel();
	if (selection != LB_ERR && selection > 0)
	{
		CString text;
		m_spaceMobileListBox.GetText(selection, text);
		m_spaceMobileListBox.DeleteString(selection);
		m_spaceMobileListBox.InsertString(selection - 1, text);
		m_spaceMobileListBox.SetCurSel(selection - 1);
	}
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnOK()
{
	UpdateData(false);

	m_spaceMobileList.Empty();

	for (int i = 0; i < m_spaceMobileListBox.GetCount(); ++i)
	{
		if (i != 0)
			m_spaceMobileList += m_separator;

		CString text;
		m_spaceMobileListBox.GetText(i, text);

		m_spaceMobileList += text;
	}

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

void DialogSpaceMobileList::OnButtonEdit() 
{
	int const selection = m_spaceMobileListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		CString text;
		m_spaceMobileListBox.GetText(selection, text);

		DialogSpaceMobile dlg(text);
		if (dlg.DoModal() == IDOK)
		{
			m_spaceMobileListBox.DeleteString(selection);
			m_spaceMobileListBox.InsertString(selection, dlg.getSpaceMobile());
			m_spaceMobileListBox.SetCurSel(selection);
		}
	}
}

// ======================================================================
