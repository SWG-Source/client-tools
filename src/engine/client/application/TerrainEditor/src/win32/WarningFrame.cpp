//
// WarningFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "WarningFrame.h"

#include "WarningView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(WarningFrame, CMDIChildWnd)

//-------------------------------------------------------------------

WarningFrame::WarningFrame() :
	CMDIChildWnd (),
	m_windowName ()
{
	m_windowName = _T("Warning");
}

//-------------------------------------------------------------------
	
WarningFrame::~WarningFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(WarningFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(WarningFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void WarningFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setWarningFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"WarningFrame");
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL WarningFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

int WarningFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"WarningFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, 0, (mainRect.bottom*1)/2, (mainRect.right*3/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}
	return 0;
}

//-------------------------------------------------------------------

void WarningFrame::update (const TerrainGeneratorHelper::OutputData& outputData)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<WarningView*> (GetActiveView ())->update (outputData);
}

//-------------------------------------------------------------------

void WarningFrame::clear (void)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<WarningView*> (GetActiveView ())->clear ();
}

//-------------------------------------------------------------------

