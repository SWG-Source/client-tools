// ======================================================================
//
// ScriptShellFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptShellFrame.h"

#include "SwgConversationEditorDoc.h"
#include "ShellView.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ScriptShellFrame, CMDIChildWnd)

ScriptShellFrame::ScriptShellFrame () :
	CMDIChildWnd (),
	m_windowName ("Shell")
{
}

ScriptShellFrame::~ScriptShellFrame()
{
}

// ======================================================================

BEGIN_MESSAGE_MAP(ScriptShellFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ScriptShellFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void ScriptShellFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	//-- tell document we're being destroyed
	if (GetActiveDocument ())
		safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setScriptShellFrame (0);
}

// ----------------------------------------------------------------------

BOOL ScriptShellFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

int ScriptShellFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect mainRect;
	AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
	mainRect.right  -= 4; // allow for frame
	mainRect.bottom -= 64; // allow for toolbars, etc.
	
	IGNORE_RETURN (SetWindowPos (&wndTop, 0, mainRect.bottom * 3 / 4, mainRect.right, mainRect.bottom / 4, SWP_SHOWWINDOW));
	
	return 0;
}

// ----------------------------------------------------------------------

void ScriptShellFrame::ActivateFrame(int nCmdShow) 
{
	//-- tell the document about it
	safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setScriptShellFrame (this);
	
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ======================================================================
