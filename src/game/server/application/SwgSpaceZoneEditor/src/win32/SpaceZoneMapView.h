// ======================================================================
//
// SpaceZoneMapView.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceZoneMapView_H
#define INCLUDED_SpaceZoneMapView_H

// ======================================================================

class Vector;

#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

class SpaceZoneMapView : public CView
{
public:

	enum ViewMode
	{
		VM_xz,
		VM_xy,
		VM_zy
	};

public:

	//{{AFX_VIRTUAL(SpaceZoneMapView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~SpaceZoneMapView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void setViewMode(ViewMode viewMode);
	ViewMode getViewMode() const;

	void setHideNavPoints(bool hideNavPoints);
	bool getHideNavPoints() const;
	void setHideSpawners(bool hideSpawners);
	bool getHideSpawners() const;
	void setHideMisc(bool hideMisc);
	bool getHideMisc() const;
	void setHidePaths(bool hidePaths);
	bool getHidePaths() const;
	void setHideGrid(bool hideGrid);
	bool getHideGrid() const;

	Vector const convertScreenToWorld(Vector const & position_w, CPoint const & point) const;
	float convertScreenToWorld(int distance) const;
	CPoint const convertWorldToScreen(Vector const & position_w) const;
	int convertWorldToScreen(float distance) const;

	float getZoomLevel() const { return m_zoomLevel; }
	void setZoomLevel(float zoomLevel) { m_zoomLevel = zoomLevel; }

protected:

	SpaceZoneMapView();
	DECLARE_DYNCREATE(SpaceZoneMapView)

	//{{AFX_MSG(SpaceZoneMapView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	class Grid
	{
	public:

		Grid();
		~Grid();

		void setMapView(SpaceZoneMapView const * mapView);
		void setGridDistance(int gridDistance);

		void draw(CDC * pDC) const;

	private:

		SpaceZoneMapView const * m_mapView;
		int m_gridDistance;
	};

	class KeyState
	{
	public:

		KeyState() :
			key(0),
			first(true),
			lastPoint(0, 0)
		{
		}

		void bind(uint16 newKey)
		{
			key = newKey;
		}

		bool isDown() const
		{
			return (GetKeyState(key) & 0x8000) != 0;
		}

	public:

		bool   first;      //lint !e1925 //-- public data member
		CPoint lastPoint;  //lint !e1925 //-- public data member

	private:

		uint16 key;
	};

private:

	bool filterObject(SwgSpaceZoneEditorDoc::Object const * object) const;

	void zoomIn(Vector const & center);
	void zoomOut(Vector const & center);
	void recenter(Vector const & center, bool force=false);

private:

	ViewMode m_viewMode;
	bool m_dragging;
	bool m_draggingSelection;
	bool m_hideNavPoints;
	bool m_hideSpawners;
	bool m_hideMisc;
	bool m_hidePaths;
	bool m_hideGrid;
	float m_zoomLevel;
	float const m_minimumZoomLevel;
	float const m_maximumZoomLevel;
	CPoint m_centerOffset;

	KeyState m_keyStatePanMap;
	Grid m_grid;

	Vector m_upperLeft;
	Vector m_lowerRight;
	typedef std::vector<SwgSpaceZoneEditorDoc::Object *> SelectionList;
	SelectionList m_selectionList;
	Vector m_dragStart;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
