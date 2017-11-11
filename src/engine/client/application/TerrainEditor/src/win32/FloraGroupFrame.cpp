//
// FloraGroupFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FloraGroupFrame.h"

#include "FloraMeshView.h"
#include "FloraTreeView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FloraGroupFrame, CMDIChildWnd)

//-------------------------------------------------------------------

FloraGroupFrame::FloraGroupFrame (void) :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar (),
	splitter (),
	meshView (0),
	treeView (0)
{
	m_windowName = "Flora Families";
}

//-------------------------------------------------------------------
	
FloraGroupFrame::~FloraGroupFrame (void)
{
	meshView = 0;
	treeView = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FloraGroupFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(FloraGroupFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int FloraGroupFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_FLORAGROUPFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"FloraGroupFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*1/4), (mainRect.bottom*1/2), (mainRect.right*1/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

BOOL FloraGroupFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	UNREF (lpcs);

	// TODO: Add your specialized code here and/or call the base class
	if (!splitter.CreateStatic (this, 1, 2))
		return FALSE;

	if (!splitter.CreateView (0, 0, RUNTIME_CLASS (FloraTreeView), CSize (100, 100), pContext))
		return FALSE;

	if (!splitter.CreateView (0, 1, RUNTIME_CLASS (FloraMeshView), CSize (100, 100), pContext))
		return FALSE;

	treeView = safe_cast<FloraTreeView*> (splitter.GetPane (0, 0));
	meshView = safe_cast<FloraMeshView*> (splitter.GetPane (0, 1));

	//-- don't chain back
//	return CMDIChildWnd::OnCreateClient(lpcs, pContext);

	return TRUE;
}

//-------------------------------------------------------------------

void FloraGroupFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	safe_cast<TerrainEditorDoc*> (GetActiveDocument ())->setFloraGroupFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"FloraGroupFrame");
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

void FloraGroupFrame::setSelectedObject (const char* name)
{
	//-- tell the mesh view about the newly selected object
	NOT_NULL (meshView);
	meshView->loadObject (name);
}

//-------------------------------------------------------------------

BOOL FloraGroupFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void FloraGroupFrame::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	if (treeView)
		treeView->addFamily (familyName, children);
}

//-------------------------------------------------------------------

void FloraGroupFrame::reset ()
{
	if (treeView)
		treeView->reset ();
}

//-------------------------------------------------------------------
