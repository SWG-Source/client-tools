//===================================================================
//
// DialogControlPointEdit.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "DialogControlPointEdit.h"

//===================================================================

DialogControlPointEdit::DialogControlPointEdit (const Vector2d& point) : 
	CDialog(DialogControlPointEdit::IDD, 0),
	m_point (point),
	m_heightVal(0.0f),
	m_hasHeight(false),
	m_x (true),
	m_z (true),
	m_height(true)
{
	//{{AFX_DATA_INIT(DialogControlPointEdit)
	//}}AFX_DATA_INIT	
}

//-------------------------------------------------------------------

DialogControlPointEdit::DialogControlPointEdit (const Vector2d& point, const float height) : 
	CDialog(DialogControlPointEdit::IDD, 0),
	m_point (point),
	m_heightVal(height),
	m_hasHeight(true),
	m_x (true),
	m_z (true),
	m_height(true)
{
	//{{AFX_DATA_INIT(DialogControlPointEdit)
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

void DialogControlPointEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogControlPointEdit)
	DDX_Control(pDX, IDC_EDIT_Z, m_z);
	DDX_Control(pDX, IDC_EDIT_X, m_x);
	DDX_Control(pDX, IDC_EDIT_HEIGHTVAL, m_height);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogControlPointEdit, CDialog)
	//{{AFX_MSG_MAP(DialogControlPointEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

BOOL DialogControlPointEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_x = m_point.x;
	m_z = m_point.y;
	m_height = m_heightVal;
	m_height.EnableWindow(m_hasHeight);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void DialogControlPointEdit::OnOK() 
{
	m_point.x = m_x;
	m_point.y = m_z;
	m_heightVal = m_height;
	
	CDialog::OnOK();
}

//===================================================================

