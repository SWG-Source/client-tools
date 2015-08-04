// ======================================================================
//
// RulerView.h
// asommers 
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_RulerView_H
#define INCLUDED_RulerView_H

// ======================================================================

class SpaceZoneMapView;

class CRulerView : public CView
{
public:

	enum RulerType
	{
		RT_vertical,
		RT_horizontal
	};

private:

	RulerType m_rulerType;
	SpaceZoneMapView const * m_mapView;

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

	void setRulerType (RulerType newRulerType);
	void setMapView (SpaceZoneMapView * newMapView);

	//{{AFX_VIRTUAL(CRulerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
};

// ----------------------------------------------------------------------

inline void CRulerView::setMapView (SpaceZoneMapView * const mapView)
{
	m_mapView = mapView;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

#endif 
