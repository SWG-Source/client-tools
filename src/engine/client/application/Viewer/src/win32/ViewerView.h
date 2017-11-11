//===================================================================
//
// ViewerView.h
// copyright 1998, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ViewerView_H
#define INCLUDED_ViewerView_H

class CellProperty;
class CViewerDoc;
class Light;
class Object;
class ObjectList;
class RenderWorldCamera;
class Shader;
class ShaderTemplate;

#include "clientObject/LightManager.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

//===================================================================

class CViewerView : public CView
{
protected: 

	CViewerView();
	DECLARE_DYNCREATE(CViewerView)

public:

	CViewerDoc* GetDocument();
	void        recenter (void);

	const RenderWorldCamera *getCamera() const;

	void        alter (float elapsedTime);

	static void setNoRender (bool noRender);

	float getAlpha();

public:

	//{{AFX_VIRTUAL(CViewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

public:

	virtual ~CViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	Object*            cellObject;
	CellProperty*      cellProperty;
	RenderWorldCamera* camera;

	real               yaw;
	real               pitch;
	CPoint             lastPoint;
	real               distanceToPoint;

	int                badRenderCount;

	float              nPatchSegments;

	bool               m_showCollideTestRenderGeometry;
	Vector             m_collideTestRenderGeometryStart;
	Vector             m_collideTestRenderGeometryEnd;

	bool               m_showCollideTestCollisionGeometry;
	bool               m_rayTestCollisionGeometry;
	Vector             m_collideTestCollisionGeometryStart;
	Vector             m_collideTestCollisionGeometryEnd;

	bool               m_animateAlpha;
	float              m_animatedAlpha;

protected:

	//{{AFX_MSG(CViewerView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButtonAxis();
	afx_msg void OnButtonGrid();
	afx_msg void OnButtonReset();
	afx_msg void OnUpdateButtonGrid(CCmdUI* pCmdUI);
	afx_msg void OnButtonPolycount();
	afx_msg void OnButtonToggleWire();
	afx_msg void OnUpdateButtonToggleWire(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowextents();
	afx_msg void OnUpdateButtonShowextents(CCmdUI* pCmdUI);
	afx_msg void OnButtonFacenormals();
	afx_msg void OnUpdateButtonFacenormals(CCmdUI* pCmdUI);
	afx_msg void OnButtonVertexnormals();
	afx_msg void OnUpdateButtonVertexnormals(CCmdUI* pCmdUI);
	afx_msg void OnButtonToggleobject();
	afx_msg void OnUpdateButtonToggleobject(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateButtonAxis(CCmdUI* pCmdUI);
	afx_msg void OnButtonMipmapDecrease();
	afx_msg void OnUpdateButtonMipmapDecrease(CCmdUI* pCmdUI);
	afx_msg void OnButtonMipmapIncrease();
	afx_msg void OnUpdateButtonMipmapIncrease(CCmdUI* pCmdUI);
	afx_msg void OnButtonMipmapReset();
	afx_msg void OnUpdateButtonMipmapReset(CCmdUI* pCmdUI);
	afx_msg void OnButtonLodReset();
	afx_msg void OnButtonLodIncrease();
	afx_msg void OnButtonLodDecrease();
	afx_msg void OnUpdateButtonLodReset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonLodIncrease(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonLodDecrease(CCmdUI* pCmdUI);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnButtonLightSetup();
	afx_msg void OnButtonLightIcons();
	afx_msg void OnUpdateButtonLightIcons(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonLightSetup(CCmdUI* pCmdUI);
	afx_msg void OnView14ambientfulldirectional();
	afx_msg void OnUpdateView14ambientfulldirectional(CCmdUI* pCmdUI);
	afx_msg void OnViewFulllighting();
	afx_msg void OnUpdateViewFulllighting(CCmdUI* pCmdUI);
	afx_msg void OnViewLightinguserdefined();
	afx_msg void OnUpdateViewLightinguserdefined(CCmdUI* pCmdUI);
	afx_msg void OnViewLightingShowHemisphericColors();
	afx_msg void OnUpdateViewLightingShowHemisphericColors(CCmdUI* pCmdUI);
	afx_msg void OnViewRendersolid();
	afx_msg void OnUpdateViewRendersolid(CCmdUI* pCmdUI);
	afx_msg void OnViewRendertextured();
	afx_msg void OnUpdateViewRendertextured(CCmdUI* pCmdUI);
	afx_msg void OnViewRenderwire();
	afx_msg void OnUpdateViewRenderwire(CCmdUI* pCmdUI);
	afx_msg void OnViewAnimatealpha();
	afx_msg void OnUpdateViewAnimatealpha(CCmdUI* pCmdUI);
	afx_msg void OnViewLightingoff();
	afx_msg void OnUpdateViewLightingoff(CCmdUI* pCmdUI);
	afx_msg void OnViewGlgraphicslayermetrics();
	afx_msg void OnUpdateViewGlgraphicslayermetrics(CCmdUI* pCmdUI);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewShadowvolumes();
	afx_msg void OnUpdateViewShadowvolumes(CCmdUI* pCmdUI);
	afx_msg void OnViewRendernpatches();
	afx_msg void OnUpdateViewRendernpatches(CCmdUI* pCmdUI);
	afx_msg void OnButtonViewradarshape();
	afx_msg void OnUpdateButtonViewradarshape(CCmdUI* pCmdUI);
	afx_msg void OnButtonViewtestshape();
	afx_msg void OnUpdateButtonViewtestshape(CCmdUI* pCmdUI);
	afx_msg void OnButtonViewwriteshape();
	afx_msg void OnUpdateButtonViewwriteshape(CCmdUI* pCmdUI);
	afx_msg void OnViewShowshadowextrudededges();
	afx_msg void OnUpdateViewShowshadowextrudededges(CCmdUI* pCmdUI);
	afx_msg void OnViewShowshadownonmanifoldedges();
	afx_msg void OnUpdateViewShowshadownonmanifoldedges(CCmdUI* pCmdUI);
	afx_msg void OnViewAnimatelight();
	afx_msg void OnUpdateViewAnimatelight(CCmdUI* pCmdUI);
	afx_msg void OnEditUsetestshader();
	afx_msg void OnUpdateEditUsetestshader(CCmdUI* pCmdUI);
	afx_msg void OnViewHardpoints();
	afx_msg void OnUpdateViewHardpoints(CCmdUI* pCmdUI);
	afx_msg void OnCollisionInfoDrawCMeshes();
	afx_msg void OnUpdateCollisionInfoDrawCMeshes(CCmdUI* pCmdUI);
	afx_msg void OnCollisionInfoDrawExtents();
	afx_msg void OnUpdateCollisionInfoDrawExtents(CCmdUI* pCmdUI);
	afx_msg void OnCollisionInfoDrawFloors();
	afx_msg void OnUpdateCollisionInfoDrawFloors(CCmdUI* pCmdUI);
	afx_msg void OnCollisionInfoDrawPathNodes();
	afx_msg void OnUpdateCollisionInfoDrawPathNodes(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

#ifndef _DEBUG
inline CViewerDoc* CViewerView::GetDocument()
{
	return safe_cast<CViewerDoc*> (m_pDocument); 
}
#endif

// ======================================================================

inline const RenderWorldCamera *CViewerView::getCamera() const
{
	return camera;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

#endif 
