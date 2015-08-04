//
// Ruler.cpp
// asommers 2001-01-18
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "Ruler.h"

#include "MapView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

CRulerSplitterWnd::CRulerSplitterWnd()
{
	m_cxSplitter=3;
	m_cySplitter=3;
	m_cxBorderShare=0;
	m_cyBorderShare=0;
	m_cxSplitterGap=3;
	m_cySplitterGap=3;
}

//-------------------------------------------------------------------

CRulerSplitterWnd::~CRulerSplitterWnd()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CRulerSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CRulerSplitterWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int CRulerSplitterWnd::HitTest(CPoint pt) const
{
	UNREF (pt);

	ASSERT_VALID(this);

	return 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CRulerView, CView)

//-------------------------------------------------------------------

CRulerView::CRulerView (void) :
	mapView (0),
	rulerType (RT_horizontal),
	mouseCursor (0, 0)
{
}

//-------------------------------------------------------------------

CRulerView::~CRulerView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CRulerView, CView)
	//{{AFX_MSG_MAP(CRulerView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void CRulerView::OnDraw(CDC* pDC)
{
	NOT_NULL (mapView);

	Vector2d zero;
	zero.makeZero ();

	const CPoint center   = mapView->convertWorldToScreen (zero);

	int distance = 100;

	if (mapView->getZoomLevel () < CONST_REAL (0.0625))
		distance = 10000;
	else
		if (mapView->getZoomLevel () < CONST_REAL (0.5))
			distance = 1000;
		else
			if (mapView->getZoomLevel () > CONST_REAL (2))
				distance = 10;
			else
				distance = 100;

	distance = static_cast<int> (static_cast<real> (distance) * mapView->getZoomLevel ());

	const Vector2d upperLeft = mapView->convertScreenToWorld (CPoint (0, 0)) * mapView->getZoomLevel ();

	//-- create resources
	CPen whitePen (PS_SOLID, 1, RGB (255, 255, 255));
	CPen blackPen (PS_SOLID, 1, RGB (0, 0, 0));
	CPen bluePen  (PS_SOLID, 1, RGB (0, 0, 255));
	CBrush whiteBrush (RGB (255, 255, 255));

	CFont hFont;
	hFont.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, "Arial");

	CFont vFont;
	vFont.CreateFont(12, 0, 900, 900, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, "Arial");

	//-- draw rectangle
	CPen*   oldPen       = pDC->SelectObject (&whitePen);
	CBrush* oldBrush     = pDC->SelectObject (&whiteBrush);
	CFont*  oldFont      = pDC->SelectObject ((rulerType == RT_horizontal) ? &hFont : &vFont);

	CRect rect;
	GetClientRect (&rect);
	pDC->Rectangle (rect);

	//-- draw tick marks and text
	int     oldTextAlign = pDC->SetTextAlign ((rulerType==RT_horizontal) ? (TA_RIGHT | TA_TOP) : (TA_LEFT | TA_TOP));
	int     oldBkMode    = pDC->SetBkMode (TRANSPARENT);

	pDC->SelectObject (blackPen);
	pDC->SetTextColor (RGB (0, 0, 0));

	//-- draw 10th tick marks
	if (rulerType == RT_horizontal)
	{
		int x = center.x % distance;

		while (x > 0)
			x -= distance / 10;

		while (x < rect.right)
		{
			pDC->MoveTo  (x, rect.CenterPoint ().y + rect.CenterPoint ().y / 2);
			pDC->LineTo  (x, rect.bottom);

			x += distance / 10;
		}
	}
	else
	{
		int y = center.y % distance;

		while (y > 0)
			y -= distance / 10;

		while (y < rect.bottom)
		{
			pDC->MoveTo  (rect.CenterPoint ().x + rect.CenterPoint ().x / 2,  y);
			pDC->LineTo  (rect.right, y);

			y += distance / 10;
		}
	}

	//-- draw tick marks
	if (rulerType == RT_horizontal)
	{
		CString tick;

		int x     = center.x % distance;
		int tickX = static_cast<int> (upperLeft.x) + x;

		while (x < rect.right)
		{
			tick.Format ("%i", static_cast<int> (static_cast<real> (tickX) / mapView->getZoomLevel ()));

			pDC->MoveTo  (x, rect.top);
			pDC->LineTo  (x, rect.bottom);
			pDC->TextOut (x, rect.top + 1, tick);

			x     += distance;
			tickX += distance;
		}
	}
	else
	{
		CString tick;

		int y     = center.y % distance;
#if MAPVIEW_REVERSED
		int tickY = static_cast<int> (upperLeft.y) - y;
#else
  		int tickY = static_cast<int> (upperLeft.y) + y;
#endif

		while (y < rect.bottom)
		{
			tick.Format ("%i", static_cast<int> (static_cast<real> (tickY) / mapView->getZoomLevel ()));

			pDC->MoveTo  (rect.left,  y);
			pDC->LineTo  (rect.right, y);
			pDC->TextOut (rect.left,  y - 1, tick);

			y     += distance;
#if MAPVIEW_REVERSED
			tickY -= distance;
#else
			tickY += distance;
#endif
		}
	}

	pDC->SelectObject (bluePen);
	pDC->SetTextColor (RGB (0, 0, 255));

	const Vector2d mousePosition_w = mapView->convertScreenToWorld (mouseCursor);

	//-- tick current position
	if (rulerType == RT_horizontal)
	{
		CString position;
		position.Format ("%i", static_cast<int> (mousePosition_w.x));

		pDC->MoveTo  (mouseCursor.x, rect.top);
		pDC->LineTo  (mouseCursor.x, rect.bottom);
		pDC->TextOut (mouseCursor.x, rect.top + 1, position);
	}
	else
	{
		CString position;
		position.Format ("%i", static_cast<int> (mousePosition_w.y));

		pDC->MoveTo  (rect.left,  mouseCursor.y);
		pDC->LineTo  (rect.right, mouseCursor.y);
		pDC->TextOut (rect.left,  mouseCursor.y - 1, position);
	}

	//-- reset old modes
	pDC->SelectObject (oldFont);
	pDC->SelectObject (oldBrush);
	pDC->SelectObject (oldPen);
	pDC->SetTextAlign (oldTextAlign);
	pDC->SetBkMode    (oldBkMode);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void CRulerView::AssertValid() const
{
	CView::AssertValid();
}

void CRulerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

BOOL CRulerView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void CRulerView::setRulerType (RulerType newRulerType)
{
	rulerType = newRulerType;
}

//-------------------------------------------------------------------

void CRulerView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CRulerCornerView, CView)

//-------------------------------------------------------------------

CRulerCornerView::CRulerCornerView() :
	mapView (0)
{
}

//-------------------------------------------------------------------

CRulerCornerView::~CRulerCornerView()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CRulerCornerView, CView)
	//{{AFX_MSG_MAP(CRulerCornerView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void CRulerCornerView::OnDraw(CDC* pDC)
{
	NOT_NULL (mapView);

	UNREF (pDC);
}

//-------------------------------------------------------------------

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

//-------------------------------------------------------------------

BOOL CRulerCornerView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void CRulerCornerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	GetParentFrame ()->SetActiveView (const_cast<MapView*> (mapView));
}

//-------------------------------------------------------------------

void CRulerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	// TODO: Add your specialized code here and/or call the base class
	GetParentFrame ()->SetActiveView (const_cast<MapView*> (mapView));
}

//-------------------------------------------------------------------

BOOL CRulerView::OnEraseBkgnd(CDC* pDC) 
{
	UNREF (pDC);

	return true;
}

//-------------------------------------------------------------------

