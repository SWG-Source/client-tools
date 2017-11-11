//
// ConsoleFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "ConsoleFrame.h"

#include "ConsoleView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ConsoleFrame, CMDIChildWnd)

//-------------------------------------------------------------------

ConsoleFrame::ConsoleFrame() :
	CMDIChildWnd ()
{
}

//-------------------------------------------------------------------

ConsoleFrame::~ConsoleFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ConsoleFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ConsoleFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void ConsoleFrame::OnDestroy() 
{
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"ConsoleFrame");

	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setConsoleFrame (0);
}

//-------------------------------------------------------------------

void ConsoleFrame::setConsoleMessage (const CString& newMessage)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<ConsoleView*> (GetActiveView ())->setConsoleMessage (newMessage);
}

//-------------------------------------------------------------------

BOOL ConsoleFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.lpszName = "Debug";
	cs.style &= ~static_cast<LONG> (FWS_ADDTOTITLE | FWS_PREFIXTITLE);

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

int ConsoleFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"ConsoleFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, 0, (mainRect.bottom*3)/4, (mainRect.right*3/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

