//
// FractalFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FractalFrame.h"

#include "FractalView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FractalFrame, CMDIChildWnd)

//-------------------------------------------------------------------

FractalFrame::FractalFrame() :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar ()
{
	m_windowName = "Fractals";
}

//-------------------------------------------------------------------

FractalFrame::~FractalFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FractalFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(FractalFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int FractalFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_FRACTALGROUPFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"FractalFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*1/2), (mainRect.bottom*1/2), (mainRect.right*1/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

void FractalFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	TerrainEditorDoc* const doc = dynamic_cast<TerrainEditorDoc*> (GetActiveDocument ());
	if (doc)
		doc->setFractalFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"FractalFrame");
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL FractalFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void FractalFrame::reset () const
{
	FractalView* fractalView = dynamic_cast<FractalView*> (GetActiveView ());
	if (fractalView)
		fractalView->reset ();
}

//-------------------------------------------------------------------
