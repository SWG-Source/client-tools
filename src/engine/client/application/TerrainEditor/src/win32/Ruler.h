//
// Ruler.h
// asommers 2001-01-18
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef RULER_H
#define RULER_H

//-------------------------------------------------------------------

class MapView;

#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

// ruler types
#define RT_VERTICAL					0x00000000
#define RT_HORIZONTAL				0x00000001

// hint information
#define VW_HSCROLL					0x00000001
#define VW_VSCROLL					0x00000002
#define VW_HPOSITION				0x00000003
#define VW_VPOSITION				0x00000004

//-------------------------------------------------------------------

class CRulerSplitterWnd : public CSplitterWnd
{
public:

	CRulerSplitterWnd();

	//{{AFX_VIRTUAL(CRulerSplitterWnd)
	//}}AFX_VIRTUAL

public:

	virtual ~CRulerSplitterWnd();

	int HitTest(CPoint pt) const;

protected:

	//{{AFX_MSG(CRulerSplitterWnd)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

class CRulerView : public CView
{
public:

	enum RulerType
	{
		RT_vertical,
		RT_horizontal
	};

private:

	const MapView* mapView;

	RulerType      rulerType;

	CPoint         mouseCursor;

protected:

	CRulerView();           
	virtual ~CRulerView();

	DECLARE_DYNCREATE(CRulerView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CRulerView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setRulerType (RulerType newRulerType=RT_horizontal);
	void setMapView (const MapView* newMapView);
	void setMouseCursor (const CPoint& newMouseCursor);

	//{{AFX_VIRTUAL(CRulerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

inline void CRulerView::setMapView (const MapView* newMapView)
{
	mapView = newMapView;
}

//-------------------------------------------------------------------

inline void CRulerView::setMouseCursor (const CPoint& newMouseCursor)
{
	mouseCursor = newMouseCursor;
}

//-------------------------------------------------------------------

class CRulerCornerView : public CView
{
private:

	const MapView* mapView;

protected:

	CRulerCornerView();
	DECLARE_DYNCREATE(CRulerCornerView)

public:

	void setMapView (const MapView* newMapView);

	//{{AFX_VIRTUAL(CRulerCornerView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

protected:

	virtual ~CRulerCornerView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(CRulerCornerView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

inline void CRulerCornerView::setMapView (const MapView* newMapView)
{
	mapView = newMapView;
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif 
