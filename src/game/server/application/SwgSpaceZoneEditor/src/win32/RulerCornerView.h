// ======================================================================
//
// RulerCornerView.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_RulerCornerView_H
#define INCLUDED_RulerCornerView_H

// ======================================================================

class SpaceZoneMapView;

// ======================================================================

class CRulerCornerView : public CView
{
private:

	SpaceZoneMapView const * m_mapView;

protected:

	CRulerCornerView();
	DECLARE_DYNCREATE(CRulerCornerView)

public:

	void setMapView (SpaceZoneMapView const * mapView);

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

// ----------------------------------------------------------------------

inline void CRulerCornerView::setMapView (SpaceZoneMapView const * const mapView)
{
	m_mapView = mapView;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

#endif 
