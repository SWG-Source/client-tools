// RenderView.cpp : implementation file
//

#include "FirstTerrainEditor.h"
#include "terrainEditor.h"
#include "RenderView.h"

/////////////////////////////////////////////////////////////////////////////
// RenderView

IMPLEMENT_DYNCREATE(RenderView, CView)

RenderView::RenderView()
{
}

RenderView::~RenderView()
{
}


BEGIN_MESSAGE_MAP(RenderView, CView)
	//{{AFX_MSG_MAP(RenderView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RenderView drawing

void RenderView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	UNREF (pDoc);

	pDC->TextOut (0, 0, "Render View");
}

/////////////////////////////////////////////////////////////////////////////
// RenderView diagnostics

#ifdef _DEBUG
void RenderView::AssertValid() const
{
	CView::AssertValid();
}

void RenderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// RenderView message handlers
