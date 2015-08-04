//
// View3dView.h
// aommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_View3dView_H
#define INCLUDED_View3dView_H

//-------------------------------------------------------------------

class GameCamera;
class ObjectList;
class TerrainObject;

//-------------------------------------------------------------------

class View3dView : public CView
{
private:

	GameCamera*    camera;
	TerrainObject* terrain;
	
	real           yaw;
	real           pitch;

	uint           timer;
	const uint     milliseconds;
	real           elapsedTime;
	bool           render;

protected:

	View3dView();           
	DECLARE_DYNCREATE(View3dView)

	//{{AFX_VIRTUAL(View3dView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:

	virtual ~View3dView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(View3dView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRefresh();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
