// ======================================================================
//
// SkeletalAppearanceTemplateView.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_SKELETALAPPEARANCETEMPLATEVIEW_H__75860CC0_B6D7_4AC9_A17F_547B7A44218B__INCLUDED_)
#define AFX_SKELETALAPPEARANCETEMPLATEVIEW_H__75860CC0_B6D7_4AC9_A17F_547B7A44218B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ======================================================================

class CImageList;
class CViewerApp;
class CViewerDoc;
class SkeletalAppearanceInstanceData;
class SkeletalAppearanceTemplate;

// ======================================================================

class SkeletalAppearanceTemplateView : public CTreeView
{
protected:
	SkeletalAppearanceTemplateView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(SkeletalAppearanceTemplateView)

// Attributes
public:

// Operations
public:

	void  addAvailableMeshGenerator(const char *meshGeneratorName, int index);
	void  addAvailableSkeletonTemplate(const char *skeletonTemplateName, const char *attachmentTransformName, int index);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SkeletalAppearanceTemplateView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~SkeletalAppearanceTemplateView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(SkeletalAppearanceTemplateView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSkeletalAppearanceWorkspaceSave();
	afx_msg void OnSkeletalAppearanceWorkspaceSaveAs();
	afx_msg void OnSkeletalAppearanceWorkspaceOpen();
	afx_msg void OnSkeletalAppearanceTemplateSave();
	afx_msg void OnSkeletonAppearanceTemplateSaveAs();
	afx_msg void OnDeleteSkeletonTemplate();
	afx_msg void OnDeleteMeshGenerator();
	afx_msg void OnRenderSkeletalAppearanceTemplate();
	afx_msg void OnAnimStateGraphBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	enum ItemType
	{
		IT_none,
		IT_appearanceTemplate,
		IT_availableMeshGeneratorGroup,
		IT_availableMeshGenerator,
		IT_availableSkeletonTemplateGroup,
		IT_availableSkeletonTemplate,
		IT_workspace,
		IT_animationStateGraph
	};

private:

	void rebuildTreeControl();

	CViewerApp                     *getOurApp() const;
	CViewerDoc                     *getOurDocument() const;
	SkeletalAppearanceTemplate     *getSkeletalAppearanceTemplate() const;

	static CString                  formatFilenameText(const char *pathName);
	static CString                  formatSkeletalAppearanceTemplateText(const char *pathName);
	static CString                  formatSkeletalAppearanceWorkspaceText(const char *pathName);
	static CString                  formatAnimationStateGraphText(const char *pathName);

	ItemType                        getItemType(HTREEITEM hItem) const;

private:

	static const std::string  mcs_animationStateGraphDir;

private:

	HTREEITEM  m_workspaceTreeItem;

	HTREEITEM  m_appearanceTemplateTreeItem;
	HTREEITEM  m_meshGeneratorGroupTreeItem;
	HTREEITEM  m_skeletonTemplateGroupTreeItem;

	HTREEITEM  m_animationStateGraphTreeItem;

	ItemType   m_selectedItemType;
	int        m_selectedIndex;

	CImageList *m_treeImageList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

// ======================================================================

#endif // !defined(AFX_SKELETALAPPEARANCETEMPLATEVIEW_H__75860CC0_B6D7_4AC9_A17F_547B7A44218B__INCLUDED_)
