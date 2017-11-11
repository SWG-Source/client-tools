//
// ProfileView.cpp
// asommers 
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "ProfileView.h"

#include "LayerFrame.h"
#include "LayerView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ProfileView, CFormView)

BEGIN_MESSAGE_MAP(ProfileView, CFormView)
	//{{AFX_MSG_MAP(ProfileView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

//-------------------------------------------------------------------

ProfileView::ProfileView() : 
	CFormView(ProfileView::IDD),
	m_wndTreeList (),
	m_cImageList (),
	m_TLInitialized (FALSE),
	slowestTotalTime (0.f),
	slowestLayerTime (0.f),
	slowestOverheadTime (0.f),
	slowestBoundaryTime (0.f),
	slowestFilterTime (0.f),
	slowestAffectorTime (0.f),
	slowestSubLayerTime (0.f),
	slowestTotal (0),
	slowestLayer (0),
	slowestOverhead (0),
	slowestBoundary (0),
	slowestFilter (0),
	slowestAffector (0),
	slowestSubLayer (0)
{
	// TODO: add construction code here
	m_TLInitialized = FALSE;
}

//-------------------------------------------------------------------

ProfileView::~ProfileView()
{
	slowestTotal    = 0;
	slowestLayer    = 0;
	slowestOverhead = 0;
	slowestBoundary = 0;
	slowestFilter   = 0;
	slowestAffector = 0;
	slowestSubLayer = 0;
}

//-------------------------------------------------------------------

void ProfileView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BOOL ProfileView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

BOOL ProfileView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

//-------------------------------------------------------------------

void ProfileView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

void ProfileView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void ProfileView::AssertValid() const
{
	CView::AssertValid();
}

void ProfileView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void ProfileView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);
}

//-------------------------------------------------------------------

void ProfileView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	IGNORE_RETURN (m_wndTreeList.SubclassDlgItem(IDC_TREELIST_VIEW, this));

	IGNORE_RETURN (m_cImageList.Create(IDB_BITMAP_LAYER, 16, 10, RGB(255, 255, 255)));
	IGNORE_RETURN (m_wndTreeList.m_tree.SetImageList(&m_cImageList, TVSIL_NORMAL));

	CRect m_wndRect;
	GetClientRect(&m_wndRect);
	m_wndTreeList.MoveWindow(0, 0, m_wndRect.Width(), m_wndRect.Height());

	{
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (0, "Layer",      LVCFMT_LEFT,  200));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (1, "Total Time", LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (2, "Layer Time", LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (3, "Overhead",   LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (4, "Boundary",   LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (5, "Filter",     LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (6, "Affector",   LVCFMT_LEFT,  80));
		IGNORE_RETURN (m_wndTreeList.m_tree.InsertColumn (7, "SubLayer",   LVCFMT_RIGHT, 80));
	}

	m_TLInitialized = TRUE;
}

//-------------------------------------------------------------------

void ProfileView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	SetScrollRange(SB_HORZ, 0, 0);
	SetScrollRange(SB_VERT, 0, 0);

	if(m_TLInitialized)
		m_wndTreeList.MoveWindow(0, 0, cx, cy);
}

//-------------------------------------------------------------------

inline void updateTime (const real newTime, real& checkTime, const HTREEITEM newItem, HTREEITEM& checkItem)
{
	if (checkTime < newTime)
	{
		checkTime = newTime;
		checkItem = newItem;
	}
}

//-------------------------------------------------------------------

void ProfileView::updateTimes (const TerrainGenerator::Layer::ProfileData& profileData, HTREEITEM item)
{
	updateTime (profileData.getTotalTime (),  slowestTotalTime,    item, slowestTotal);
	updateTime (profileData.getLayerTime (),  slowestLayerTime,    item, slowestLayer);
	updateTime (profileData.timeInOverhead,   slowestOverheadTime, item, slowestOverhead);
	updateTime (profileData.timeInBoundaries, slowestBoundaryTime, item, slowestBoundary);
	updateTime (profileData.timeInFilters,    slowestFilterTime,   item, slowestFilter);
	updateTime (profileData.timeInAffectors,  slowestAffectorTime, item, slowestAffector);
	updateTime (profileData.timeInSubLayers,  slowestSubLayerTime, item, slowestSubLayer);
}

//-------------------------------------------------------------------

void ProfileView::update (const TerrainGenerator::Layer* layer, HTREEITEM root)
{
	const TerrainGenerator::Layer::ProfileData& profileData = layer->getProfileData ();

	if (!profileData.isWorthCounting ())
		return;

	CString buffer;

	const LayerView::LayerViewIconType icon = LayerView::getIcon (layer);

	HTREEITEM item = m_wndTreeList.m_tree.InsertItem (layer->getName (), icon, icon, root);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemData (item, reinterpret_cast<DWORD> (layer)));

	updateTimes (profileData, item);

	buffer.Format ("%1.3f", profileData.getTotalTime ());
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 1, buffer));

	buffer.Format ("%1.3f", profileData.getLayerTime ());
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 2, buffer));
	
	buffer.Format ("%1.3f", profileData.timeInOverhead);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 3, buffer));
	
	buffer.Format ("%1.3f", profileData.timeInBoundaries);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 4, buffer));
	
	buffer.Format ("%1.3f", profileData.timeInFilters);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 5, buffer));
	
	buffer.Format ("%1.3f", profileData.timeInAffectors);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 6, buffer));
	
	buffer.Format ("%1.3f", profileData.timeInSubLayers);
	IGNORE_RETURN (m_wndTreeList.m_tree.SetItemText (item, 7, buffer));

	int i;
	for (i = 0; i < layer->getNumberOfLayers (); ++i)
		if (layer->getLayer (i)->isActive ())
			update (layer->getLayer (i), root /*item*/);

	IGNORE_RETURN (m_wndTreeList.m_tree.Expand (item, TVE_EXPAND));
}

//-------------------------------------------------------------------

inline void colorItem (CNewTreeListCtrl& tree, HTREEITEM item, DWORD color)
{
	if (item)
		IGNORE_RETURN (tree.SetItemColor (item, color));
}

void ProfileView::update (const TerrainGenerator* generator)
{
	IGNORE_RETURN (m_wndTreeList.m_tree.DeleteAllItems ());

	slowestTotalTime    = 0;
	slowestLayerTime    = 0;
	slowestOverheadTime = 0;
	slowestBoundaryTime = 0;
	slowestFilterTime   = 0;
	slowestAffectorTime = 0;
	slowestSubLayerTime = 0;

	slowestTotal    = 0;
	slowestLayer    = 0;
	slowestOverhead = 0;
	slowestBoundary = 0;
	slowestFilter   = 0;
	slowestAffector = 0;
	slowestSubLayer = 0;

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); ++i)
		if (generator->getLayer (i)->isActive ())
			update (generator->getLayer (i), TVI_ROOT);

	const DWORD color = RGB (255, 0, 0);

	colorItem (m_wndTreeList.m_tree, slowestTotal, color);
	colorItem (m_wndTreeList.m_tree, slowestLayer, color);
	colorItem (m_wndTreeList.m_tree, slowestOverhead, color);
	colorItem (m_wndTreeList.m_tree, slowestBoundary, color);
	colorItem (m_wndTreeList.m_tree, slowestFilter, color);
	colorItem (m_wndTreeList.m_tree, slowestAffector, color);
	colorItem (m_wndTreeList.m_tree, slowestSubLayer, color);

	Invalidate ();
}

//-------------------------------------------------------------------

void ProfileView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UNREF (nFlags);
	UNREF (point);

	const HTREEITEM selection = m_wndTreeList.m_tree.GetSelectedItem ();

	if (selection)
	{
		const TerrainGenerator::Layer* layer = reinterpret_cast<const TerrainGenerator::Layer*> (m_wndTreeList.m_tree.GetItemData (selection));

		TerrainEditorDoc* doc  = static_cast<TerrainEditorDoc*> (GetDocument ());
		UNREF (doc);

		GetApp ()->showLayers ();

		if (doc->getLayerFrame ())
			doc->getLayerFrame ()->selectLayerItem (layer);
	}
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

