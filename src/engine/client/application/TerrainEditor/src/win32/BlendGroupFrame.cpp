//
// BlendGroupFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BlendGroupFrame.h"

#include "BlendTreeView.h"
#include "BlendView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BlendGroupFrame, CMDIChildWnd)

//-------------------------------------------------------------------

BlendGroupFrame::BlendGroupFrame() :
	CMDIChildWnd (),
	m_windowName (),
	splitter (),
	treeView (0),
	blendView (0)
{
	m_windowName = "Blend Families";
}

//-------------------------------------------------------------------
	
BlendGroupFrame::~BlendGroupFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BlendGroupFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(BlendGroupFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

BOOL BlendGroupFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

int BlendGroupFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"BlendGroupFrame"))
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

void BlendGroupFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setBlendGroupFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"BlendGroupFrame");

	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL BlendGroupFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	UNREF (lpcs);

	if (!splitter.CreateStatic (this, 1, 2))
		return FALSE;

	if (!splitter.CreateView (0, 0, RUNTIME_CLASS (BlendTreeView), CSize (100, 100), pContext))
		return FALSE;

	if (!splitter.CreateView (0, 1, RUNTIME_CLASS (BlendView), CSize (100, 100), pContext))
		return FALSE;

	treeView  = static_cast<BlendTreeView*> (splitter.GetPane (0, 0));
	blendView = static_cast<BlendView*>     (splitter.GetPane (0, 1));

	//-- don't chain back
//	return CMDIChildWnd::OnCreateClient(lpcs, pContext);

	return TRUE;
}

//-------------------------------------------------------------------

