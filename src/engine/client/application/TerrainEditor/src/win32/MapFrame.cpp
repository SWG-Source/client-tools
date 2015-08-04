//
// MapFrame.cpp
// asommers 1-20-2001
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "MapFrame.h"

#include "MapView.h"
#include "Ruler.h"
#include "terraineditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(MapFrame, CMDIChildWnd)

//-------------------------------------------------------------------

MapFrame::MapFrame() : 
	CMDIChildWnd (),
	windowName (),
	m_wndToolBar1 (),
	m_wndToolBar2 (),
	m_rulerSplitter ()
{
	windowName = "2D Map";
}

//-------------------------------------------------------------------
	
MapFrame::~MapFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(MapFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(MapFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int MapFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar1.CreateEx(this) ||
		!m_wndToolBar1.LoadToolBar(IDR_MAPVIEW))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar1.SetBarStyle(m_wndToolBar1.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	//-- create tool bar
	if (!m_wndToolBar2.CreateEx(this) ||
		!m_wndToolBar2.LoadToolBar(IDR_MAPVIEW2))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar2.SetBarStyle(m_wndToolBar2.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

		TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"MapFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		IGNORE_RETURN (SetWindowPos (&wndTop, 0, 0, (mainRect.right*3)/4, mainRect.bottom/2, SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

void MapFrame::OnDestroy() 
{
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"MapFrame");
	
	CMDIChildWnd::OnDestroy();
	
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setMapFrame (0);
}

//-------------------------------------------------------------------

BOOL MapFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

BOOL MapFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	UNREF (lpcs);

	IGNORE_RETURN (m_rulerSplitter.CreateStatic(this, 2, 2, WS_CHILD | WS_VISIBLE | WS_BORDER));
	IGNORE_RETURN (m_rulerSplitter.CreateView(0, 0, RUNTIME_CLASS (CRulerCornerView), CSize(0,0), pContext));
	IGNORE_RETURN (m_rulerSplitter.CreateView(0, 1, RUNTIME_CLASS (CRulerView), CSize(0,0), pContext));
	IGNORE_RETURN (m_rulerSplitter.CreateView(1, 0, RUNTIME_CLASS (CRulerView), CSize(0,0), pContext));
	IGNORE_RETURN (m_rulerSplitter.CreateView(1, 1, RUNTIME_CLASS (MapView), CSize(0,0), pContext));
	m_rulerSplitter.SetColumnInfo (0, 16, 0);
	m_rulerSplitter.SetRowInfo (0, 16, 0);

	MapView* mapView = static_cast<MapView*> (m_rulerSplitter.GetPane (1, 1));
	mapView->setMapFrame (this);

	static_cast<CRulerView*> (m_rulerSplitter.GetPane (0, 1))->setRulerType (CRulerView::RT_horizontal);
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (1, 0))->setRulerType (CRulerView::RT_vertical);
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (0, 1))->setMapView (mapView);
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (1, 0))->setMapView (mapView);
	static_cast<CRulerCornerView*> (m_rulerSplitter.GetPane (0, 0))->setMapView (mapView);
	
	return true;
}

//-------------------------------------------------------------------

void MapFrame::updateRulers (void)
{
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (1, 0))->Invalidate ();
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (0, 1))->Invalidate ();
	static_cast<CRulerCornerView*> (m_rulerSplitter.GetPane (0, 0))->Invalidate ();
}

//-------------------------------------------------------------------

void MapFrame::updateMousePosition (const CPoint& point)
{
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (1, 0))->setMouseCursor (point);
	static_cast<CRulerView*> (m_rulerSplitter.GetPane (0, 1))->setMouseCursor (point);
}

//-------------------------------------------------------------------

void MapFrame::recenter (const Vector2d& center_w)
{
	static_cast<MapView*> (m_rulerSplitter.GetPane (1, 1))->recenter (center_w);
}

//-------------------------------------------------------------------

void MapFrame::updateRiversAndRoads (void)
{
	static_cast<MapView*> (m_rulerSplitter.GetPane (1, 1))->updateRiversAndRoads ();
}

//-------------------------------------------------------------------

const Vector2d MapFrame::getCenter (void) const
{
	return static_cast<const MapView*> (m_rulerSplitter.GetPane (1, 1))->getCenter ();
}

//-------------------------------------------------------------------

void MapFrame::bakeTerrain ()
{
	static_cast<MapView*> (m_rulerSplitter.GetPane (1, 1))->bakeTerrain ();
}

//-------------------------------------------------------------------

MapView *MapFrame::getMapView()
{
	return static_cast<MapView*>(m_rulerSplitter.GetPane (1, 1));
}

