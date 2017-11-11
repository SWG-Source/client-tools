//
// BookmarkFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BookmarkFrame.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BookmarkFrame, CMDIChildWnd)

//-------------------------------------------------------------------

BookmarkFrame::BookmarkFrame() :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar ()
{
	m_windowName = "Bookmarks";
}

//-------------------------------------------------------------------

BookmarkFrame::~BookmarkFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BookmarkFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(BookmarkFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int BookmarkFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_BOOKMARKFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"BookmarkFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*2/3), (mainRect.bottom*3)/4, (mainRect.right/3), (mainRect.bottom/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

void BookmarkFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setBookmarkFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"BookmarkFrame");

	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL BookmarkFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

