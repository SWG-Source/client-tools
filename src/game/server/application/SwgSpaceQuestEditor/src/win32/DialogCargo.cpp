// ======================================================================
//
// DialogCargo.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogCargo.h"

#include "Configuration.h"

// ======================================================================

DialogCargo::DialogCargo(StringSet const & cargoSet, CString const & result)
	: CDialog(DialogCargo::IDD, 0),
	m_cargoSet(cargoSet),
	m_result(result)
{
	//{{AFX_DATA_INIT(DialogCargo)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogCargo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogCargo)
	DDX_Control(pDX, IDC_LIST_CARGO, m_cargoListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogCargo, CDialog)
	//{{AFX_MSG_MAP(DialogCargo)
	ON_LBN_DBLCLK(IDC_LIST_CARGO, OnDblclkListCargo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogCargo::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i = 0;
	for (StringSet::const_iterator iter = m_cargoSet.begin(); iter != m_cargoSet.end(); ++iter, ++i)
	{
		CString const & cargo = *iter;
		m_cargoListBox.AddString(cargo);
		
		if (cargo == m_result)
			m_cargoListBox.SetCurSel(i);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogCargo::OnOK()
{
	UpdateData(false);

	int const index = m_cargoListBox.GetCurSel();
	if (index != LB_ERR)
		m_cargoListBox.GetText(index, m_result);

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

CString const & DialogCargo::getResult() const
{
	return m_result;
}

// ----------------------------------------------------------------------

void DialogCargo::OnDblclkListCargo() 
{
	OnOK();
}

// ======================================================================

