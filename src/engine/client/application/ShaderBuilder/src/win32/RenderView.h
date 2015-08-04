#if !defined(AFX_RENDERVIEW_H__271A40DC_3A6B_405B_8960_E705F72BEA2E__INCLUDED_)
#define AFX_RENDERVIEW_H__271A40DC_3A6B_405B_8960_E705F72BEA2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRenderView view

#include <d3d9.h>
#include <d3dx9.h>

class CLeftView;
class CImplementationNode;
class CPassNode;
class CStageNode;
class CTextureNode;

#include <map>

class CRenderView : public CView
{
protected:
	CRenderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRenderView)

// Attributes
public:

	const CLeftView                           *leftView;
	const char                                *errorMessage;
	const char                                *oldErrorMessage;
	HKEY                                       registryKey;

	IDirect3D9                                *direct3d;
	IDirect3DDevice9                          *device;
	bool                                       disabled;
	int                                        width;
	int                                        height;
	
	D3DXMATRIX                                 worldMatrix;
	IDirect3DVertexBuffer9                    *vertexBuffer;

	typedef std::map<CString, IDirect3DBaseTexture9 *>   TextureDataMap;
	TextureDataMap                             textureDataMap;

// Operations
public:

	void CreateDevice();
	void DestroyDevice();

	void SetupPass(const CPassNode &pass);

	void Draw();
	void BeginScene();
	void EndScene();

	void RenderTexture(const CTextureNode &textureNode);

	IDirect3DBaseTexture9 *GetTexture(const CString &fileName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRenderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void DrawBackground(const char *fileName);

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenderView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnReset();
	afx_msg void OnBackground();
	afx_msg void OnDisabled();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERVIEW_H__271A40DC_3A6B_405B_8960_E705F72BEA2E__INCLUDED_)
