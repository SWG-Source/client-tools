// DialogFindObject.cpp : implementation file
//

#include "FirstWorldSnapshotViewer.h"
#include "DialogFindObject.h"

/////////////////////////////////////////////////////////////////////////////
// DialogFindObject dialog


DialogFindObject::DialogFindObject(CWnd* pParent /*=NULL*/)
	: CDialog(DialogFindObject::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogFindObject)
	m_objectId = _T("");
	//}}AFX_DATA_INIT
}


void DialogFindObject::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFindObject)
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Text(pDX, IDC_EDIT_OBJECTID, m_objectId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DialogFindObject, CDialog)
	//{{AFX_MSG_MAP(DialogFindObject)
	ON_EN_CHANGE(IDC_EDIT_OBJECTID, OnChangeEditObjectid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogFindObject message handlers

void DialogFindObject::OnOK() 
{
	UpdateData(true);
	
	CDialog::OnOK();
}

void DialogFindObject::OnChangeEditObjectid() 
{
	UpdateData(true);
	m_okButton.EnableWindow(!m_objectId.IsEmpty());
}

BOOL DialogFindObject::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_okButton.EnableWindow(false);
	
	return TRUE;  
	              
}
