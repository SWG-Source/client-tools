// ======================================================================
//
// RulerView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "RulerView.h"

#include "sharedMath/Vector2d.h"
#include "SpaceZoneMapView.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(CRulerView, CView)

// ----------------------------------------------------------------------

CRulerView::CRulerView() :
	m_rulerType(RT_horizontal),
	m_mapView(0)
{
}

// ----------------------------------------------------------------------

CRulerView::~CRulerView()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CRulerView, CView)
	//{{AFX_MSG_MAP(CRulerView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void CRulerView::OnDraw(CDC * pDC)
{
	NOT_NULL(m_mapView);

	CPoint const center = m_mapView->convertWorldToScreen(Vector::zero);

	int distance = 100;

	if (m_mapView->getZoomLevel() < CONST_REAL(0.0625))
		distance = 10000;
	else
		if (m_mapView->getZoomLevel() < CONST_REAL(0.5))
			distance = 1000;
		else
			if (m_mapView->getZoomLevel() > CONST_REAL(2))
				distance = 10;
			else
				distance = 100;

	distance = static_cast<int>(static_cast<real>(distance) * m_mapView->getZoomLevel());

	Vector const upperLeft3d = m_mapView->convertScreenToWorld(Vector::zero, CPoint(0, 0)) * m_mapView->getZoomLevel();
	Vector2d upperLeft;
	switch (m_mapView->getViewMode())
	{
	case SpaceZoneMapView::VM_xz:
		{
			upperLeft.x = upperLeft3d.x;
			upperLeft.y = upperLeft3d.z;
		}
		break;

	case SpaceZoneMapView::VM_xy:
		{
			upperLeft.x = upperLeft3d.x;
			upperLeft.y = upperLeft3d.y;
		}
		break;

	case SpaceZoneMapView::VM_zy:
		{
			upperLeft.x = upperLeft3d.z;
			upperLeft.y = upperLeft3d.y;
		}
		break;
	}

	//-- create resources
	CPen whitePen(PS_SOLID, 1, RGB(255, 255, 255));
	CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen bluePen(PS_SOLID, 1, RGB(0, 0, 255));
	CBrush whiteBrush(RGB(255, 255, 255));

	CFont hFont;
	hFont.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, "Arial");

	CFont vFont;
	vFont.CreateFont(12, 0, 900, 900, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, "Arial");

	//-- draw rectangle
	CPen * oldPen = pDC->SelectObject(&whitePen);
	CBrush * oldBrush = pDC->SelectObject(&whiteBrush);
	CFont * oldFont = pDC->SelectObject((m_rulerType == RT_horizontal) ? &hFont : &vFont);

	CRect rect;
	GetClientRect(&rect);
	pDC->Rectangle(rect);

	//-- draw tick marks and text
	int oldTextAlign = pDC->SetTextAlign((m_rulerType==RT_horizontal) ?(TA_RIGHT | TA_TOP) :(TA_LEFT | TA_TOP));
	int oldBkMode = pDC->SetBkMode(TRANSPARENT);

	pDC->SelectObject(blackPen);
	pDC->SetTextColor(RGB(0, 0, 0));

	//-- draw 10th tick marks
	if (m_rulerType == RT_horizontal)
	{
		int x = center.x % distance;

		while (x > 0)
			x -= distance / 10;

		while (x < rect.right)
		{
			pDC->MoveTo(x, rect.CenterPoint().y + rect.CenterPoint().y / 2);
			pDC->LineTo(x, rect.bottom);

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
			pDC->MoveTo(rect.CenterPoint().x + rect.CenterPoint().x / 2, y);
			pDC->LineTo(rect.right, y);

			y += distance / 10;
		}
	}

	//-- draw tick marks
	if (m_rulerType == RT_horizontal)
	{
		CString tick;

		int x = center.x % distance;
		int tickX = static_cast<int>(upperLeft.x) + x;

		while (x < rect.right)
		{
			tick.Format("%i", static_cast<int>(static_cast<real>(tickX) / m_mapView->getZoomLevel()));

			pDC->MoveTo(x, rect.top);
			pDC->LineTo(x, rect.bottom);
			pDC->TextOut(x, rect.top + 1, tick);

			x += distance;
			tickX += distance;
		}
	}
	else
	{
		CString tick;

		int y = center.y % distance;
		int tickY = static_cast<int>(upperLeft.y) - y;

		while (y < rect.bottom)
		{
			tick.Format("%i", static_cast<int>(static_cast<real>(tickY) / m_mapView->getZoomLevel()));

			pDC->MoveTo(rect.left, y);
			pDC->LineTo(rect.right, y);
			pDC->TextOut(rect.left, y - 1, tick);

			y += distance;
			tickY -= distance;
		}
	}

	pDC->SelectObject(bluePen);
	pDC->SetTextColor(RGB(0, 0, 255));

	Vector const mousePosition_w = safe_cast<SwgSpaceZoneEditorDoc const *>(GetDocument())->getMapPosition();
	CPoint mouseCursor = m_mapView->convertWorldToScreen(mousePosition_w);

	float x = 0;
	float y = 0;
	switch (m_mapView->getViewMode())
	{
	case SpaceZoneMapView::VM_xz:
		{
			x = mousePosition_w.x;
			y = mousePosition_w.z;
		}
		break;

	case SpaceZoneMapView::VM_xy:
		{
			x = mousePosition_w.x;
			y = mousePosition_w.y;
		}
		break;

	case SpaceZoneMapView::VM_zy:
		{
			x = mousePosition_w.z;
			y = mousePosition_w.y;
		}
		break;
	}

	//-- tick current position
	if (m_rulerType == RT_horizontal)
	{
		CString position;
		position.Format("%i", static_cast<int>(x));

		pDC->MoveTo(mouseCursor.x, rect.top);
		pDC->LineTo(mouseCursor.x, rect.bottom);
		pDC->TextOut(mouseCursor.x, rect.top + 1, position);
	}
	else
	{
		CString position;
		position.Format("%i", static_cast<int>(y));

		pDC->MoveTo(rect.left, mouseCursor.y);
		pDC->LineTo(rect.right, mouseCursor.y);
		pDC->TextOut(rect.left, mouseCursor.y - 1, position);
	}

	//-- reset old modes
	pDC->SelectObject(oldFont);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
	pDC->SetTextAlign(oldTextAlign);
	pDC->SetBkMode(oldBkMode);
}

// ----------------------------------------------------------------------

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

// ----------------------------------------------------------------------

BOOL CRulerView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void CRulerView::setRulerType(RulerType const rulerType)
{
	m_rulerType = rulerType;
}

// ----------------------------------------------------------------------

void CRulerView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}

// ----------------------------------------------------------------------

void CRulerView::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactiveView) 
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	// TODO: Add your specialized code here and/or call the base class
	GetParentFrame()->SetActiveView(const_cast<SpaceZoneMapView*>(m_mapView));
}

// ----------------------------------------------------------------------

BOOL CRulerView::OnEraseBkgnd(CDC * const /*pDC*/) 
{
	return true;
}

// ----------------------------------------------------------------------

void CRulerView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const pHint) 
{
	if (pSender != this && lHint == SwgSpaceZoneEditorDoc::H_updateRulers)
		Invalidate();

	CView::OnUpdate(pSender, lHint, pHint);
}

// ======================================================================
