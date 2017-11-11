//
// IffTreeView.cpp
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#include "FirstViewIff.h"
#include "viewiff.h"

#include "IffDoc.h"
#include "ChildFrm.h"
#include "IffDataView.h"
#include "IffTreeView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(IffTreeView, CTreeView)

BEGIN_MESSAGE_MAP(IffTreeView, CTreeView)
	//{{AFX_MSG_MAP(IffTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

//-------------------------------------------------------------------

IffTreeView::IffTreeView()
{
}

IffTreeView::~IffTreeView()
{
}

//-------------------------------------------------------------------

BOOL IffTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP);

	return CTreeView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void IffTreeView::OnDraw(CDC* pDC)
{
	IffDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	UNREF (pDC);
}

//-------------------------------------------------------------------

BOOL IffTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void IffTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void IffTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

void IffTreeView::OnInitialUpdate()
{
//	imageList.Create (IDB_BITMAP_WORKSPACE, 16, 1, RGB (255,255,255));
//	GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL);

	CTreeView::OnInitialUpdate();
}

//-------------------------------------------------------------------

#ifdef _DEBUG

void IffTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void IffTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

IffDoc* IffTreeView::GetDocument (void)
{
	ASSERT (m_pDocument->IsKindOf (RUNTIME_CLASS (IffDoc)));
	return static_cast<IffDoc*> (m_pDocument);
}

const IffDoc* IffTreeView::GetDocument (void) const
{
	ASSERT (m_pDocument->IsKindOf (RUNTIME_CLASS (IffDoc)));
	return static_cast<const IffDoc*> (m_pDocument);
}

#endif //_DEBUG

//-------------------------------------------------------------------

void IffTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);	

	GetDocument ()->fillTreeCtrl (GetTreeCtrl ());
}

void IffTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	GetDocument ()->UpdateAllViews (this);
	
	*pResult = 0;
}

void IffTreeView::Dump(const char * fileName)
{
	GetDocument()->dump(fileName);
}
