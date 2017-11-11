//
// RadialGroupFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "RadialGroupFrame.h"

#include "terraineditor.h"
#include "RadialView.h"
#include "RadialTreeView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(RadialGroupFrame, CMDIChildWnd)

//-------------------------------------------------------------------

RadialGroupFrame::RadialGroupFrame (void) :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar (),
	splitter (),
	treeView (0),
	radialView (0)
{
	m_windowName = "Radial Families";
}

//-------------------------------------------------------------------
	
RadialGroupFrame::~RadialGroupFrame()
{
	treeView = 0;
	radialView = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(RadialGroupFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(RadialGroupFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int RadialGroupFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_RADIALGROUPFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

		TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"RadialGroupFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*1/4), (mainRect.bottom*3/4), (mainRect.right*1/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}
	return 0;
}

BOOL RadialGroupFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	UNREF (lpcs);

	if (!splitter.CreateStatic (this, 1, 2))
		return FALSE;

	if (!splitter.CreateView (0, 0, RUNTIME_CLASS (RadialTreeView), CSize (100, 100), pContext))
		return FALSE;

	if (!splitter.CreateView (0, 1, RUNTIME_CLASS (RadialView), CSize (100, 100), pContext))
		return FALSE;

	treeView   = safe_cast<RadialTreeView*> (splitter.GetPane (0, 0));
	radialView = safe_cast<RadialView*>     (splitter.GetPane (0, 1));

	//-- don't chain back
//	return CMDIChildWnd::OnCreateClient(lpcs, pContext);

	return TRUE;
}

void RadialGroupFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	safe_cast<TerrainEditorDoc*> (GetActiveDocument ())->setRadialGroupFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"RadialGroupFrame");

	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

BOOL RadialGroupFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void RadialGroupFrame::setSelectedShader (const char* name)
{
	//-- tell the mesh view about the newly selected object
	if (radialView)
		radialView->loadShader (name);
}

//-------------------------------------------------------------------

void RadialGroupFrame::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	if (treeView)
		treeView->addFamily (familyName, children);
}

//-------------------------------------------------------------------

void RadialGroupFrame::reset (void)
{
	if (treeView)
		treeView->reset ();
}

//-------------------------------------------------------------------
