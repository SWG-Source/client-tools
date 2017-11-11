// ======================================================================
//
// RulerCornerView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "RulerCornerView.h"

#include "SpaceZoneMapView.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(CRulerCornerView, CView)

// ----------------------------------------------------------------------

CRulerCornerView::CRulerCornerView() :
	m_mapView(0)
{
}

// ----------------------------------------------------------------------

CRulerCornerView::~CRulerCornerView()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CRulerCornerView, CView)
	//{{AFX_MSG_MAP(CRulerCornerView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void CRulerCornerView::OnDraw(CDC* pDC)
{
	NOT_NULL(m_mapView);

	UNREF(pDC);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CRulerCornerView::AssertValid() const
{
	CView::AssertValid();
}

void CRulerCornerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL CRulerCornerView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void CRulerCornerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	GetParentFrame()->SetActiveView(const_cast<SpaceZoneMapView*>(m_mapView));
}

// ----------------------------------------------------------------------

