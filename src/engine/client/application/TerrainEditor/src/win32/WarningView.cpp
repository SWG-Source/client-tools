//
// WarningView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "WarningView.h"

#include "LayerFrame.h"
#include "Resource.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(WarningView, CListView)

//-------------------------------------------------------------------

WarningView::WarningView() :
	CListView (),
	imageListSet (false),
	imageList ()
{
}

//-------------------------------------------------------------------

WarningView::~WarningView()
{
}

//-------------------------------------------------------------------

//lint -save -e1924 -e648

BEGIN_MESSAGE_MAP(WarningView, CListView)
	//{{AFX_MSG_MAP(WarningView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

void WarningView::OnDraw(CDC* pDC)
{
	UNREF(pDC);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void WarningView::AssertValid() const
{
	CListView::AssertValid();
}

void WarningView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void WarningView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_WARNING, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetListCtrl ().SetImageList (&imageList, LVSIL_SMALL));

		imageListSet = true;
	}

	IGNORE_RETURN (GetListCtrl ().InsertColumn (0, "Description", LVCFMT_LEFT, 600, 0));
}

//-------------------------------------------------------------------

void WarningView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	UNREF (pHint);
	UNREF(lHint);
	UNREF(pSender);
}

//-------------------------------------------------------------------

BOOL WarningView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = cs.style | LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void WarningView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	POSITION pos = GetListCtrl ().GetFirstSelectedItemPosition ();

	if (pos)
	{
		const int item = GetListCtrl ().GetNextSelectedItem (pos);

		const TerrainGenerator::LayerItem* layerItem = reinterpret_cast<const TerrainGenerator::LayerItem*> (GetListCtrl ().GetItemData (item));

		TerrainEditorDoc* doc  = static_cast<TerrainEditorDoc*> (GetDocument ());
		UNREF (doc);

		GetApp ()->showLayers ();

		if (doc->getLayerFrame ())
			doc->getLayerFrame ()->selectLayerItem (layerItem);
	}
	
	*pResult = 0;
}

//-------------------------------------------------------------------

void WarningView::update (const TerrainGeneratorHelper::OutputData& outputData)
{
	const int index = GetListCtrl ().GetItemCount ();

	IGNORE_RETURN (GetListCtrl ().InsertItem (index, *outputData.message, outputData.type));
	IGNORE_RETURN (GetListCtrl ().SetItemData (index, reinterpret_cast<DWORD> (outputData.layerItem)));
}

//-------------------------------------------------------------------

void WarningView::clear (void)
{
	IGNORE_RETURN (GetListCtrl ().DeleteAllItems ());
}

//-------------------------------------------------------------------

