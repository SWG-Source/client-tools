//
// EnvironmentFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "EnvironmentFrame.h"

#include "EnvironmentView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(EnvironmentFrame, CMDIChildWnd)

//-------------------------------------------------------------------

EnvironmentFrame::EnvironmentFrame() :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar ()
{
	m_windowName = "Environments";
}

//-------------------------------------------------------------------

EnvironmentFrame::~EnvironmentFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(EnvironmentFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(EnvironmentFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int EnvironmentFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_ENVIRONMENTFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"EnvironmentFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*1/2), (mainRect.bottom*3/4), (mainRect.right*1/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

void EnvironmentFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	TerrainEditorDoc* const doc = dynamic_cast<TerrainEditorDoc*> (GetActiveDocument ());
	if (doc)
		doc->setEnvironmentFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"EnvironmentFrame");

	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL EnvironmentFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void EnvironmentFrame::reset () const
{
	EnvironmentView* environmentView = dynamic_cast<EnvironmentView*> (GetActiveView ());
	if (environmentView)
		environmentView->reset ();
}

//-------------------------------------------------------------------
