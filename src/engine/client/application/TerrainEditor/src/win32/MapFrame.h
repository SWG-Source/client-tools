//
// MapFrame.cpp
// asommers 1-20-2001
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#ifndef MAPFRAME_H
#define MAPFRAME_H

//-------------------------------------------------------------------

#include "Ruler.h"

class MapView;

//-------------------------------------------------------------------

class MapFrame : public CMDIChildWnd
{
private:

	CString           windowName;
	CToolBar          m_wndToolBar1;
	CToolBar          m_wndToolBar2;
	CRulerSplitterWnd m_rulerSplitter;

protected:

	MapFrame();
	DECLARE_DYNCREATE(MapFrame)
	virtual ~MapFrame();

	//{{AFX_MSG(MapFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void updateRulers (void);
	void updateMousePosition (const CPoint& point);
	void recenter (const Vector2d& center_w);
	const Vector2d getCenter (void) const;

	void updateRiversAndRoads (void);
	void bakeTerrain ();

	MapView *getMapView();

	//{{AFX_VIRTUAL(MapFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
