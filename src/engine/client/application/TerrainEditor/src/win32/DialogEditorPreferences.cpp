//
// DialogEditorPreferences.cpp
// asommers 
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "DialogEditorPreferences.h"

//-------------------------------------------------------------------

DialogEditorPreferences::DialogEditorPreferences(CWnd* pParent /*=NULL*/) : 
	CDialog(DialogEditorPreferences::IDD, pParent),
	m_tooltip (),

	//-- widgets
	m_maxHeight (0.f),
	m_minHeight (0.f)
{
	//{{AFX_DATA_INIT(DialogEditorPreferences)
	m_maxHeight = 0.0f;
	m_minHeight = 0.0f;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

void DialogEditorPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogEditorPreferences)
	DDX_Text(pDX, IDC_EDIT_MAXHEIGHT, m_maxHeight);
	DDX_Text(pDX, IDC_EDIT_MINHEIGHT, m_minHeight);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogEditorPreferences, CDialog)
	//{{AFX_MSG_MAP(DialogEditorPreferences)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

BOOL DialogEditorPreferences::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following block was added by the ToolTips component.
	{
		// Let the ToolTip process this message.
		m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);	// CG: This was added by the ToolTips component.
}

//-------------------------------------------------------------------

BOOL DialogEditorPreferences::OnInitDialog()
{
	CDialog::OnInitDialog();	// CG: This was added by the ToolTips component.
	// CG: The following block was added by the ToolTips component.
	{
		// Create the ToolTip control.
		IGNORE_RETURN (m_tooltip.Create(this));
		m_tooltip.Activate(TRUE);

		// TODO: Use one of the following forms to add controls:
		// m_tooltip.AddTool(GetDlgItem(IDC_<name>), <string-table-id>);
		// m_tooltip.AddTool(GetDlgItem(IDC_<name>), "<text>");
	}
	return TRUE;	// CG: This was added by the ToolTips component.
}

//-------------------------------------------------------------------

