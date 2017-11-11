//
// FindFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FindFrame.h"

#include "FindView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FindFrame, CMDIChildWnd)

//-------------------------------------------------------------------

FindFrame::FindFrame() :
	CMDIChildWnd (),
	m_windowName ()
{
	m_windowName = _T("Find");
}

//-------------------------------------------------------------------
	
FindFrame::~FindFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FindFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(FindFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void FindFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setFindFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"FindFrame");
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL FindFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

int FindFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"FindFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, 0, (mainRect.bottom*1)/2, (mainRect.right*3/4), (mainRect.bottom*1/2), SWP_SHOWWINDOW));
	}
	return 0;
}

//-------------------------------------------------------------------

void FindFrame::update (const TerrainGeneratorHelper::OutputData& outputData)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<FindView*> (GetActiveView ())->update (outputData);
}

//-------------------------------------------------------------------

void FindFrame::clear (void)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<FindView*> (GetActiveView ())->clear ();
}

//-------------------------------------------------------------------

