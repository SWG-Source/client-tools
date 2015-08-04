// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__0E8A733A_A8B9_4992_A762_BDF0901A407A__INCLUDED_)
#define AFX_MAINFRM_H__0E8A733A_A8B9_4992_A762_BDF0901A407A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"
#include "AW_CMultiViewSplitter.h"

class CLeftView;
class CBlankView;
class CEffectView;
class CImplementationView;
class CPassView;
class CStageView;
class CMaterialView;
class CTextureView;
class CAlphaReferenceView;
class CStencilReferenceView;
class CTextureCoordinateSetView;
class CTextureFactorView;
class CTextureScrollView;
class CPixelShaderView;
class CFixedFunctionPipelineView;
class CVertexShaderView;
class CVertexShaderProgram;
class CPixelShaderProgramView;
class CIncludeView;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

	int blankViewId;
	int implementationViewId;
	int effectViewId;
	int passViewId;
	int stageViewId;
	int materialViewId;
	int textureViewId;
	int alphaReferenceViewId;
	int stencilReferenceViewId;
	int textureCoordinateSetViewId;
	int textureFactorViewId;
	int textureScrollViewId;
	int pixelShaderViewId;
	int fixedFunctionPipelineViewId;
	int vertexShaderViewId;
	int vertexShaderProgramViewId;
	int pixelShaderProgramViewId;
	int includeViewId;

	CLeftView                 *leftView;

	CBlankView                 *blankView;
	CEffectView                *effectView;
	CImplementationView        *implementationView;
	CPassView                  *passView;
	CStageView                 *stageView;
	CMaterialView              *materialView;
	CTextureView               *textureView;
	CAlphaReferenceView        *alphaReferenceView;
	CStencilReferenceView      *stencilReferenceView;
	CTextureCoordinateSetView  *textureCoordinateSetView;
	CTextureFactorView         *textureFactorView;
	CTextureScrollView         *textureScrollView;
	CPixelShaderView           *pixelShaderView;
	CFixedFunctionPipelineView *fixedFunctionPipelineView;
	CVertexShaderView          *vertexShaderView;
	CVertexShaderProgram       *vertexShaderProgramView;
	CPixelShaderProgramView    *pixelShaderProgramView;
	CIncludeView               *includeView;

// Operations
public:

	void ShowNode(CNode &node);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar   m_wndStatusBar;
	AW_CMultiViewSplitter m_wndSplitter;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__0E8A733A_A8B9_4992_A762_BDF0901A407A__INCLUDED_)
