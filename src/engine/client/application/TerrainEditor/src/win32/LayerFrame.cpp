//
// LayerFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "LayerFrame.h"

#include "LayerView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(LayerFrame, CMDIChildWnd)

//-------------------------------------------------------------------

LayerFrame::LayerFrame() :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar ()
{
	m_windowName = "Construction Layers";
}

//-------------------------------------------------------------------
	
LayerFrame::~LayerFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(LayerFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(LayerFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int LayerFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_LAYERVIEW))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"LayerFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		IGNORE_RETURN (SetWindowPos (&wndTop, (mainRect.right*3)/4, mainRect.bottom/2, mainRect.right*1/4, mainRect.bottom/2, SWP_SHOWWINDOW));
	}
	
	

	return 0;
}

//-------------------------------------------------------------------

void LayerFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setLayerFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"LayerFrame");

	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL LayerFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void LayerFrame::selectLayerItem (const TerrainGenerator::LayerItem* layerItem)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<LayerView*> (GetActiveView ())->selectLayerItem (layerItem);
}

//-------------------------------------------------------------------

