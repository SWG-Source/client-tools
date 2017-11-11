// ======================================================================
//
// ChildFrame.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "ChildFrame.h"

#include "Resource.h"
#include "SwgSpaceZoneEditor.h"
#include "SpaceZonePropertyView.h"
#include "SpaceZoneTreeView.h"
#include "SpaceZoneMapView.h"
#include "RulerCornerView.h"
#include "RulerView.h"

// ======================================================================

namespace ChildFrameNamespace
{
	UINT const cms_indicators[] =
	{
		ID_SEPARATOR
	};
}

using namespace ChildFrameNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ChildFrame)
	ON_COMMAND(ID_BUTTON_VIEWMODE_XY, OnButtonViewmodeXy)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWMODE_XY, OnUpdateButtonViewmodeXy)
	ON_COMMAND(ID_BUTTON_VIEWMODE_XZ, OnButtonViewmodeXz)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWMODE_XZ, OnUpdateButtonViewmodeXz)
	ON_COMMAND(ID_BUTTON_VIEWMODE_ZY, OnButtonViewmodeZy)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWMODE_ZY, OnUpdateButtonViewmodeZy)
	ON_COMMAND(ID_BUTTON_HIDE_NAVPOINTS, OnButtonHideNavpoints)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIDE_NAVPOINTS, OnUpdateButtonHideNavpoints)
	ON_COMMAND(ID_BUTTON_HIDE_SPAWNERS, OnButtonHideSpawners)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIDE_SPAWNERS, OnUpdateButtonHideSpawners)
	ON_COMMAND(ID_BUTTON_HIDE_MISC, OnButtonHideMisc)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIDE_MISC, OnUpdateButtonHideMisc)
	ON_COMMAND(ID_BUTTON_INFORMATION, OnButtonInformation)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_INFORMATION, OnUpdateButtonInformation)
	ON_COMMAND(ID_BUTTON_HIDE_PATHS, OnButtonHidePaths)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIDE_PATHS, OnUpdateButtonHidePaths)
	ON_WM_SIZE()
	ON_COMMAND(ID_BUTTON_VALIDATE, OnButtonValidate)
	ON_COMMAND(ID_BUTTON_P4, OnButtonP4)
	ON_COMMAND(ID_BUTTON_HIDE_GRID, OnButtonHideGrid)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIDE_GRID, OnUpdateButtonHideGrid)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

ChildFrame::ChildFrame() :
	m_mainSplitterWnd(),
	m_splitterWnd1(),
	m_splitterWnd2(),
	m_splitterCreated(false),
	m_treeView(0),
	m_mapView(0)
{
}

ChildFrame::~ChildFrame()
{
}

// ----------------------------------------------------------------------

BOOL ChildFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext * pContext)
{
	m_mainSplitterWnd.CreateStatic(this, 1, 2);
	m_splitterWnd1.CreateStatic(&m_mainSplitterWnd, 2, 1, WS_CHILD | WS_VISIBLE, m_mainSplitterWnd.IdFromRowCol(0, 0));
	m_splitterWnd2.CreateStatic(&m_mainSplitterWnd, 2, 2, WS_CHILD | WS_VISIBLE, m_mainSplitterWnd.IdFromRowCol(0, 1));
	NOT_NULL(m_mainSplitterWnd.GetPane(0, 0));
	NOT_NULL(m_mainSplitterWnd.GetPane(0, 1));

	m_splitterWnd1.CreateView(0, 0, RUNTIME_CLASS(SpaceZoneTreeView), CSize(200,200), pContext);
	m_splitterWnd1.CreateView(1, 0, RUNTIME_CLASS(SpaceZonePropertyView), CSize(200,200), pContext);
	m_treeView = static_cast<SpaceZoneTreeView *>(m_splitterWnd1.GetPane(0, 0));
	NOT_NULL(m_treeView);
	m_propertyView = static_cast<SpaceZonePropertyView *>(m_splitterWnd1.GetPane(1, 0));
	NOT_NULL(m_propertyView);

	m_splitterWnd2.CreateView(0, 0, RUNTIME_CLASS(CRulerCornerView), CSize(200,200), pContext);
	m_splitterWnd2.CreateView(0, 1, RUNTIME_CLASS(CRulerView), CSize(200,200), pContext);
	m_splitterWnd2.CreateView(1, 0, RUNTIME_CLASS(CRulerView), CSize(200,200), pContext);
	m_splitterWnd2.CreateView(1, 1, RUNTIME_CLASS(SpaceZoneMapView), CSize(200,200), pContext);
	NOT_NULL(m_splitterWnd2.GetPane(0, 0));
	NOT_NULL(m_splitterWnd2.GetPane(0, 1));
	NOT_NULL(m_splitterWnd2.GetPane(1, 0));
	m_mapView = static_cast<SpaceZoneMapView *>(m_splitterWnd2.GetPane(1, 1));
	NOT_NULL(m_mapView);

	static_cast<CRulerCornerView *>(m_splitterWnd2.GetPane(0, 0))->setMapView(m_mapView);
	static_cast<CRulerView *>(m_splitterWnd2.GetPane(0, 1))->setRulerType(CRulerView::RT_horizontal);
	static_cast<CRulerView *>(m_splitterWnd2.GetPane(0, 1))->setMapView(m_mapView);
	static_cast<CRulerView *>(m_splitterWnd2.GetPane(1, 0))->setRulerType(CRulerView::RT_vertical);
	static_cast<CRulerView *>(m_splitterWnd2.GetPane(1, 0))->setMapView(m_mapView);

	resize();

	m_splitterCreated = true;

	return TRUE;
}

// ----------------------------------------------------------------------

BOOL ChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void ChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

void ChildFrame::OnButtonViewmodeXz() 
{
	m_mapView->setViewMode(SpaceZoneMapView::VM_xz);
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonViewmodeXz(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getViewMode() == SpaceZoneMapView::VM_xz);
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonViewmodeXy() 
{
	m_mapView->setViewMode(SpaceZoneMapView::VM_xy);
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonViewmodeXy(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getViewMode() == SpaceZoneMapView::VM_xy);
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonViewmodeZy() 
{
	m_mapView->setViewMode(SpaceZoneMapView::VM_zy);
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonViewmodeZy(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getViewMode() == SpaceZoneMapView::VM_zy);
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonHideNavpoints() 
{
	m_mapView->setHideNavPoints(!m_mapView->getHideNavPoints());
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonHideNavpoints(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getHideNavPoints());
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonHideSpawners() 
{
	m_mapView->setHideSpawners(!m_mapView->getHideSpawners());
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonHideSpawners(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getHideSpawners());
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonHideMisc() 
{
	m_mapView->setHideMisc(!m_mapView->getHideMisc());
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonHideMisc(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getHideMisc());
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonInformation() 
{
	m_treeView->showInformation();
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonInformation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_treeView->hasInformation());
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonHidePaths() 
{
	m_mapView->setHidePaths(!m_mapView->getHidePaths());
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonHidePaths(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getHidePaths());
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonHideGrid() 
{
	m_mapView->setHideGrid(!m_mapView->getHideGrid());
}

// ----------------------------------------------------------------------

void ChildFrame::OnUpdateButtonHideGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_mapView->getHideGrid());
}

// ----------------------------------------------------------------------

void ChildFrame::ActivateFrame(int nCmdShow) 
{
	if (nCmdShow == -1)
		nCmdShow = SW_SHOWMAXIMIZED;
	
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ----------------------------------------------------------------------

void ChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);

	if (m_splitterCreated)
		resize();
}

// ----------------------------------------------------------------------

void ChildFrame::resize()
{
	CRect rect;
	GetWindowRect(&rect);

	m_mainSplitterWnd.SetColumnInfo(0, rect.Width() / 3, 100);
	m_mainSplitterWnd.SetColumnInfo(1, rect.Width() * 2 / 3, 100);

	m_splitterWnd1.SetRowInfo(0, rect.Height() * 2 / 3, 100);

	m_splitterWnd2.SetColumnInfo(0, 16, 16);
	m_splitterWnd2.SetColumnInfo(1, rect.Width() - 16, 16);
	m_splitterWnd2.SetRowInfo(0, 16, 16);
	m_splitterWnd2.SetRowInfo(1, rect.Height() - 16, 200);
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonValidate() 
{
	m_treeView->validate();
}

// ----------------------------------------------------------------------

void ChildFrame::OnButtonP4()
{
	m_treeView->perforceEdit();
}

// ----------------------------------------------------------------------

int ChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(cms_indicators, sizeof(cms_indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	return 0;
}

// ----------------------------------------------------------------------

void ChildFrame::setToolTip(CString const & toolTip)
{
	m_wndStatusBar.SetWindowText(toolTip);
}

// ======================================================================

