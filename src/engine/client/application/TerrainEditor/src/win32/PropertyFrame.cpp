//
// PropertyFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "PropertyFrame.h"

#include "FormShaderFamily.h"
#include "terraineditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(PropertyFrame, CMDIChildWnd)

//-------------------------------------------------------------------

PropertyFrame::PropertyFrame() :
	CMDIChildWnd (),
	windowName ()	
{
	windowName = "Properties";
}

//-------------------------------------------------------------------

PropertyFrame::~PropertyFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(PropertyFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(PropertyFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void PropertyFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setPropertyFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"PropertyFrame");
	//-- chain to base 
	CMDIChildWnd::OnDestroy();	
}

//-----------------------------------------------------------------------------

void PropertyFrame::ApplyChanges ()
{
	// Grab any new data from current view.
	PropertyView* oldView = dynamic_cast<PropertyView*>(GetActiveView ());
	if (oldView)
		oldView->ApplyChanges ();
}

//-----------------------------------------------------------------------------

void PropertyFrame::RefreshView ()
{
	PropertyView* oldView = dynamic_cast<PropertyView*>(GetActiveView ());
	if (oldView)
		oldView->OnInitialUpdate ();
}

//-----------------------------------------------------------------------------

void PropertyFrame::ChangeView (CRuntimeClass* cls, PropertyView::ViewData* vd)
{
	CCreateContext ccc;
	ccc.m_pNewViewClass   = cls ? cls : RUNTIME_CLASS (PropertyView);
	ccc.m_pCurrentDoc     = GetActiveDocument();
	ccc.m_pCurrentFrame   = this;
	ccc.m_pLastView       = NULL;
	ccc.m_pNewDocTemplate = NULL;

	// Destroy current view.
	IGNORE_RETURN (GetActiveView ()->DestroyWindow ());

	// Load new view.

	PropertyView* newView = static_cast<PropertyView*> (CreateView (&ccc));

	newView->Initialize (vd);
	newView->OnInitialUpdate ();
	RecalcLayout();
	IGNORE_RETURN (newView->ShowWindow (SW_SHOW));
	newView->UpdateWindow ();
	SetActiveView (newView);
}

//-------------------------------------------------------------------

int PropertyFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
//	if (!m_wndToolBar.CreateEx(this) ||/
//		!m_wndToolBar.LoadToolBar(IDR_PROPERTYVIEW))
//	{
//		TRACE0("Failed to create toolbar\n");
//		return -1;      // fail to create
//	}

//	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
//		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"PropertyFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*3)/4, 0, mainRect.right*1/4, mainRect.bottom/2, SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

BOOL PropertyFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

