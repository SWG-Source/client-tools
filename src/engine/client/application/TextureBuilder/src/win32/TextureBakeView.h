// ======================================================================
//
// TextureBakeView.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_TEXTUREBAKEVIEW_H__13B55F9C_151C_4D14_98BB_25327A15166C__INCLUDED_)
#define AFX_TEXTUREBAKEVIEW_H__13B55F9C_151C_4D14_98BB_25327A15166C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

class  ObjectListCamera;
class  PackedArgb;
class  Shader;
class  StaticVertexBuffer;
class  DynamicVertexBuffer;
class  Texture;
class  TextureBuilderDoc;
class  Transform;

// ======================================================================

class TextureBakeView : public CView
{
public:

	struct Selection
	{
		float  m_x0;
		float  m_y0;
		float  m_x1;
		float  m_y1;
	};

public:

	void  notifyNewActiveSelection();
	void  notifyNoActiveSelection();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextureBakeView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:

	TextureBakeView();
	virtual ~TextureBakeView();

	DECLARE_DYNCREATE(TextureBakeView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(TextureBakeView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewToggleBackgroundColor();
	afx_msg void OnUpdateViewToggleBackgroundColor(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom100Percent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void               prepareBakedTextureGeometry(int textureWidth, int textureHeight);
	void               setupObjectPosition();

	TextureBuilderDoc       *GetDocument();
	const TextureBuilderDoc *GetDocument() const;

	void               calculateCameraPerScreenUnits(float depth, float &xCameraPerScreenUnits, float &yCameraPerScreenUnits) const;
	void               calculateScreenPerTextureUnits(float depth, float &xScreenPerTextureUnits, float &yScreenPerTextureUnits) const;
	void               setNoZoomEffectDistance(float depth);

	void               convertTextureToCameraSpace(const Selection &textureSpaceSelection, Selection &cameraSpaceSelection) const;
	void               fillActiveSelectionVb(const Selection &cameraSpaceSelection, const PackedArgb &color);

	void               dragActiveSelectionBorder(int windowX, int windowY);
	void               moveActiveSelection(int deltaWindowX, int deltaWindowY);

	bool               findSelectionModifyParameters(int screenX, int screenY, float **xModify, int &xModifyAdjustment, float **yModify, int &yModifyAdjustment);

private:

	int                    m_windowWidth;
	int                    m_windowHeight;

	float                  m_xCameraPerScreenUnits;
	float                  m_yCameraPerScreenUnits;

	Shader                *m_bakedTextureShader;
	ObjectListCamera      *m_camera;
	StaticVertexBuffer    *m_bakedTextureVertexBuffer;
	Transform             *m_objectToWorld;

	float                  m_distanceFromObject;
	float                  m_noZoomDistance;
	CPoint                 m_lastMousePosition;

	bool                   m_toggleBackgroundColor;

	int                    m_lastTextureWidth;
	int                    m_lastTextureHeight;

	bool                   m_hasActiveSelection;
	Selection              m_activeSelection;
	DynamicVertexBuffer   *m_activeSelectionVertexBuffer;

	bool                   m_isDraggingSelectionBorder;
	float                 *m_xModify;
	int                    m_xModifyAdjustment;
	float                 *m_yModify;
	int                    m_yModifyAdjustment;

	bool                   m_isMovingSelection;

private:
	// disabled
	TextureBakeView(const TextureBakeView&);
	TextureBakeView &operator =(const TextureBakeView&);
};

// ======================================================================

#ifndef _DEBUG

inline TextureBuilderDoc *TextureBakeView::GetDocument()
{
	return reinterpret_cast<TextureBuilderDoc*>(CView::GetDocument());
}

// ----------------------------------------------------------------------

inline const TextureBuilderDoc *TextureBakeView::GetDocument() const
{
	return reinterpret_cast<const TextureBuilderDoc*>(CView::GetDocument());
}

#endif

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
