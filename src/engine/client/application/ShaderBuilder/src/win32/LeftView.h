#if !defined(AFX_LEFTVIEW_H__60E551F4_3552_4AB2_AF6D_1368CCCBBFD0__INCLUDED_)
#define AFX_LEFTVIEW_H__60E551F4_3552_4AB2_AF6D_1368CCCBBFD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeftView.h : header file
//

#include "Node.h"

/////////////////////////////////////////////////////////////////////////////
// CLeftView view

class CMainFrame;
class CRenderView;

class CLeftView : public CTreeView
{
	friend class CMainFrame;
protected:
	CLeftView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:

	CMainFrame                *mainFrame;
	CRenderView               *renderView;
	CTemplateTreeNode         *templateTreeNode;
	CMenu                      rootMenu;
	CImageList                 imageList;
	HCURSOR                    cursorPointer;
	HCURSOR                    cursorPointerCopy;
	HCURSOR                    cursorNoDrag;
	HCURSOR                    cursorNotTarget;
	int                        contextMenu[CNode::Count];
	mutable const CNode       *renderNode;
	mutable HTREEITEM          renderItem;
	HTREEITEM                  dragItem;
	CNode                     *dragNode;
	bool                       dragging;
	bool                       targetValid;
	CImageList                *dragImageList;
	UINT                       timerId;

// Operations
public:

	void         ResetTree();
	void         ExpandSelectedItem();
	CNode       *GetSelectedNode();
	const CNode *GetSelectedNode() const;
	const CNode *GetRenderNode() const;
	CNode       &GetSelectedNode(CNode::Type type);
	void         DestroyNode(const CNode *node);
	void         CheckTarget();
	void         UpdateAllItemImages(HTREEITEM item);
	void         UpdateAllItemImages();

	void         ShowNode(CNode *node);

	BOOL         LoadTemplate(const char *fileName);
	BOOL         SaveTemplate(const char *fileName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLeftView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLeftView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEffectAddlocalimplementation();
	afx_msg void OnEffectLint();
	afx_msg void OnImplementationAddlocalpass();
	afx_msg void OnImplementationDelete();
	afx_msg void OnPassAddlocalstage();
	afx_msg void OnPassDelete();
	afx_msg void OnStageDelete();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSaveeffect();
	afx_msg void OnSaveeffectas();
	afx_msg void OnSaveImplementation();
	afx_msg void OnAddnamedimplementation();
	afx_msg void OnLocalizeeffect();
	afx_msg void OnLocalizeimplementation();
	afx_msg void OnLoadeffect();
	afx_msg void OnTemplateLoad();
	afx_msg void OnTemplateNew();
	afx_msg void OnTemplateSave();
	afx_msg void OnTemplateSaveas();
	afx_msg void OnUpdateAddlocalstage(CCmdUI* pCmdUI);
	afx_msg void OnAddlocalpixelshader();
	afx_msg void OnUpdateAddlocalpixelshader(CCmdUI* pCmdUI);
	afx_msg void OnPixelShaderDelete();
	afx_msg void OnAddfixedfunctionpipeline();
	afx_msg void OnUpdateAddfixedfunctionpipeline(CCmdUI* pCmdUI);
	afx_msg void OnAddvertexshader();
	afx_msg void OnUpdateAddvertexshader(CCmdUI* pCmdUI);
	afx_msg void OnDeleteFixedfunctionpipeline();
	afx_msg void OnDeleteVertexShader();
	afx_msg void OnCompileAllVertexShaderPrograms();
	afx_msg void OnOpenVertexShader();
	afx_msg void OnCompileallPixelShaders();
	afx_msg void OnOpenPixelShader();
	afx_msg void OnIncludeOpen();
	afx_msg void OnIncludeClose();
	afx_msg void OnVertexShaderProgramClose();
	afx_msg void OnPixelShaderProgramClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEFTVIEW_H__60E551F4_3552_4AB2_AF6D_1368CCCBBFD0__INCLUDED_)
