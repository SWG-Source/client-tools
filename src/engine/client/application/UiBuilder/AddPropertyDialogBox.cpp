// AddPropertyDialogBox.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "AddPropertyDialogBox.h"
#include "UITypes.h"

/////////////////////////////////////////////////////////////////////////////
// AddPropertyDialogBox dialog
AddPropertyDialogBox::NarrowStringList AddPropertyDialogBox::s_oldPropertyNames;
AddPropertyDialogBox::StringList       AddPropertyDialogBox::s_oldPropertyValues;

AddPropertyDialogBox::AddPropertyDialogBox(CWnd* pParent /*=NULL*/)
	: CDialog(AddPropertyDialogBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddPropertyDialogBox)
	m_propertyName = _T("");
	m_propertyValue = _T("");
	//}}AFX_DATA_INIT
}


void AddPropertyDialogBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddPropertyDialogBox)
	DDX_CBString(pDX, IDC_PROPERTYNAME, m_propertyName);
	DDV_MaxChars(pDX, m_propertyName, 64);
	DDX_CBString(pDX, IDC_PROPERTYVALUE, m_propertyValue);
	DDV_MaxChars(pDX, m_propertyValue, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddPropertyDialogBox, CDialog)
	//{{AFX_MSG_MAP(AddPropertyDialogBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddPropertyDialogBox message handlers

BOOL AddPropertyDialogBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	{
		for (NarrowStringList::const_iterator i = s_oldPropertyNames.begin(); i != s_oldPropertyNames.end(); ++i)
		{
			SendDlgItemMessage(IDC_PROPERTYNAME, CB_ADDSTRING, 0, (LPARAM)i->c_str() );
		}
	}
	
	{
		for (StringList::const_iterator i = s_oldPropertyValues.begin(); i != s_oldPropertyValues.end(); ++i)
		{
			SendDlgItemMessage(IDC_PROPERTYVALUE, CB_ADDSTRING, 0, (LPARAM)i->c_str() );
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int AddPropertyDialogBox::DoModal() 
{
	int result = CDialog::DoModal();

	if (result==IDOK)
	{
		if (m_propertyName.IsEmpty())
		{
			result=IDCANCEL;
		}
		else
		{
			UINarrowString newPropertyName = static_cast<const char *>(m_propertyName);
			s_oldPropertyNames.push_front(newPropertyName);
			UIString newPropertyValue = UIUnicode::narrowToWide(static_cast<const char *>(m_propertyValue));
			s_oldPropertyValues.push_front(newPropertyValue);
		}
	}

	return result;
}
