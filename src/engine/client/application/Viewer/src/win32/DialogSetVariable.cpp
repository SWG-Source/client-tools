// DialogSetVariable.cpp : implementation file
//

#include "FirstViewer.h"
#include "viewer.h"
#include "DialogSetVariable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogSetVariable dialog


DialogSetVariable::DialogSetVariable(CWnd* pParent /*=NULL*/)
	: CDialog(DialogSetVariable::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogSetVariable)
	m_valueString = _T("");
	m_variableName = _T("");
	m_valueType = -1;
	//}}AFX_DATA_INIT
}


void DialogSetVariable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogSetVariable)
	DDX_Text(pDX, IDC_EDIT_SETVAR_VALUE, m_valueString);
	DDX_Text(pDX, IDC_EDIT_SETVAR_VARNAME, m_variableName);
	DDX_Radio(pDX, IDC_RADIO_VALTYPE_REAL, m_valueType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DialogSetVariable, CDialog)
	//{{AFX_MSG_MAP(DialogSetVariable)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogSetVariable message handlers
