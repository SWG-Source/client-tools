//
// BookmarkView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BookmarkView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BookmarkView, CListView)

BookmarkView::BookmarkView() :
	CListView ()
{
}

//-------------------------------------------------------------------

BookmarkView::~BookmarkView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BookmarkView, CListView)
	//{{AFX_MSG_MAP(BookmarkView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void BookmarkView::OnDraw(CDC* pDC)
{
	UNREF(pDC);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void BookmarkView::AssertValid() const
{
	CListView::AssertValid();
}

void BookmarkView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void BookmarkView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	IGNORE_RETURN (GetListCtrl ().InsertColumn (0, "Label", LVCFMT_LEFT, 120, 0));
	IGNORE_RETURN (GetListCtrl ().InsertColumn (1, "X Coord", LVCFMT_LEFT, 120, 1));
	IGNORE_RETURN (GetListCtrl ().InsertColumn (2, "Z Coord", LVCFMT_LEFT, 120, 2));

	IGNORE_RETURN (GetListCtrl ().InsertItem (0, "#1"));
	IGNORE_RETURN (GetListCtrl ().InsertItem (1, "#2"));
	IGNORE_RETURN (GetListCtrl ().InsertItem (2, "#3"));

	IGNORE_RETURN (GetListCtrl ().SetItemText (0,1, "X1"));
	IGNORE_RETURN (GetListCtrl ().SetItemText (0,2, "Z1"));
	IGNORE_RETURN (GetListCtrl ().SetItemText (1,1, "X2"));
	IGNORE_RETURN (GetListCtrl ().SetItemText (1,2, "Z2"));
	IGNORE_RETURN (GetListCtrl ().SetItemText (2,1, "X3"));
	IGNORE_RETURN (GetListCtrl ().SetItemText (2,2, "Z3"));
}

//-------------------------------------------------------------------

void BookmarkView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	UNREF (pHint);
	UNREF(lHint);
	UNREF(pSender);
	
}

//-------------------------------------------------------------------

BOOL BookmarkView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = cs.style | LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

