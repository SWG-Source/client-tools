//
// BlendView.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BlendView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BlendView, CView)

//-------------------------------------------------------------------

BlendView::BlendView() :
	CView ()
{
}

//-------------------------------------------------------------------

BlendView::~BlendView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BlendView, CView)
	//{{AFX_MSG_MAP(BlendView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void BlendView::OnDraw(CDC* pDC)
{
	UNREF (pDC);

	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	UNREF (pDoc);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void BlendView::AssertValid() const
{
	CView::AssertValid();
}

void BlendView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

