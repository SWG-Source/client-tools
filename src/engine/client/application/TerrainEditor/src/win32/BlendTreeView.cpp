//
// BlendTreeView.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BlendTreeView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BlendTreeView, CTreeView)

//-------------------------------------------------------------------

BlendTreeView::BlendTreeView() :
	CTreeView ()
{
}

//-------------------------------------------------------------------

BlendTreeView::~BlendTreeView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BlendTreeView, CTreeView)
	//{{AFX_MSG_MAP(BlendTreeView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void BlendTreeView::OnDraw(CDC* pDC)
{
	UNREF (pDC);

	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	UNREF (pDoc);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void BlendTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void BlendTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

