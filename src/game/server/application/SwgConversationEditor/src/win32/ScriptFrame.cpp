// ======================================================================
//
// ScriptFrame.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptFrame.h"

#include "ScriptTreeView.h"
#include "ScriptView.h"
#include "SwgConversationEditor.h"
#include "SwgConversationEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ScriptFrame, CMDIChildWnd)

// ======================================================================

ScriptFrame::ScriptFrame() :
	m_windowName ("Script Editor"),
	m_wndToolBar (),
	m_splitter (),
	m_treeView (0),
	m_scriptView (0)
{
}

// ----------------------------------------------------------------------

ScriptFrame::~ScriptFrame()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ScriptFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ScriptFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

int ScriptFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_SCRIPTFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	CRect mainRect;
	AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
	mainRect.right  -= 4; // allow for frame
	mainRect.bottom -= 64; // allow for toolbars, etc.
	
//	IGNORE_RETURN (SetWindowPos (&wndTop, 0, mainRect.bottom / 2, mainRect.right, mainRect.bottom / 4, SWP_SHOWWINDOW));
	IGNORE_RETURN (SetWindowPos (&wndTop, 0, mainRect.bottom / 2, mainRect.right, mainRect.bottom / 2, SWP_SHOWWINDOW));
	
	return 0;
}

// ----------------------------------------------------------------------

BOOL ScriptFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext) 
{
	if (!m_splitter.CreateStatic (this, 1, 2))
		return FALSE;

	m_splitter.CreateView (0, 0, RUNTIME_CLASS (ScriptTreeView), CSize (200, 200), pContext);
	m_treeView = safe_cast<ScriptTreeView *> (m_splitter.GetPane (0, 0));

	m_splitter.CreateView (0, 1, RUNTIME_CLASS (ScriptView), CSize (200, 200), pContext);
	m_scriptView = safe_cast<ScriptView *> (m_splitter.GetPane (0, 1));

	return TRUE;
}

// ----------------------------------------------------------------------

void ScriptFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	//-- tell document we're being destroyed
	if (GetActiveDocument ())
		safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setScriptFrame (0);
}

// ----------------------------------------------------------------------

BOOL ScriptFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void ScriptFrame::editCondition (int const conditionFamilyId)
{
	m_scriptView->editCondition (conditionFamilyId);
}

// ----------------------------------------------------------------------

void ScriptFrame::editAction (int const actionFamilyId)
{
	m_scriptView->editAction (actionFamilyId);
}

// ----------------------------------------------------------------------

void ScriptFrame::editTokenTO (int const tokenTOFamilyId)
{
	m_scriptView->editTokenTO (tokenTOFamilyId);
}

// ----------------------------------------------------------------------

void ScriptFrame::editTokenDI (int const tokenDIFamilyId)
{
	m_scriptView->editTokenDI (tokenDIFamilyId);
}

// ----------------------------------------------------------------------

void ScriptFrame::editTokenDF (int const tokenDFFamilyId)
{
	m_scriptView->editTokenDF (tokenDFFamilyId);
}

// ----------------------------------------------------------------------

void ScriptFrame::editTrigger ()
{
	m_scriptView->editTrigger ();
}

// ----------------------------------------------------------------------

void ScriptFrame::editNothing ()
{
	m_scriptView->editNothing ();
}

// ----------------------------------------------------------------------

void ScriptFrame::ActivateFrame(int nCmdShow) 
{
	//-- tell the document about it
	safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setScriptFrame (this);

	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ======================================================================

