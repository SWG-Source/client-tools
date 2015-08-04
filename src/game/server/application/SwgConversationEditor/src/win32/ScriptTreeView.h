// ======================================================================
//
// ScriptTreeView.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_ScriptTreeView_H
#define INCLUDED_ScriptTreeView_H

// ======================================================================

#include "SwgConversationEditorDoc.h"

class ScriptGroup;

// ======================================================================

class ScriptTreeView : public CTreeView
{
protected:

	ScriptTreeView();           
	DECLARE_DYNCREATE(ScriptTreeView)

	//{{AFX_VIRTUAL(ScriptTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ScriptTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ScriptTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnScriptAddaction();
	afx_msg void OnScriptAddcondition();
	afx_msg void OnScriptDelete();
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnScriptRename();
	afx_msg void OnUpdateScriptRename(CCmdUI* pCmdUI);
	afx_msg void OnScriptAddlibrary();
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonDeleteunused();
	afx_msg void OnButtonDelete();
	afx_msg void OnScriptDuplicate();
	afx_msg void OnUpdateScriptDuplicate(CCmdUI* pCmdUI);
	afx_msg void OnScriptAddlabel();
	afx_msg void OnScriptAddtokendf();
	afx_msg void OnScriptAddtokendi();
	afx_msg void OnScriptAddtokento();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	enum TreeViewIconType
	{
		TVIT_root,
		TVIT_condition,
		TVIT_conditionSelected,
		TVIT_action,
		TVIT_actionSelected,
		TVIT_tokenTO,
		TVIT_tokenTOSelected,
		TVIT_tokenDI,
		TVIT_tokenDISelected,
		TVIT_tokenDF,
		TVIT_tokenDFSelected,
		TVIT_label,
		TVIT_labelSelected,
		TVIT_library
	};

private:

	void selectItem (HTREEITEM treeItem);
	bool isCondition (HTREEITEM treeItem) const;
	HTREEITEM getSelectedCondition () const;
	bool isConditionSelected () const;
	bool isAction (HTREEITEM treeItem) const;
	HTREEITEM getSelectedAction () const;
	bool isActionSelected () const;
	bool isTokenTO (HTREEITEM treeItem) const;
	HTREEITEM getSelectedTokenTO () const;
	bool isTokenTOSelected () const;
	bool isTokenDI (HTREEITEM treeItem) const;
	HTREEITEM getSelectedTokenDI () const;
	bool isTokenDISelected () const;
	bool isTokenDF (HTREEITEM treeItem) const;
	HTREEITEM getSelectedTokenDF () const;
	bool isTokenDFSelected () const;
	bool isLibrary (HTREEITEM treeItem) const;
	HTREEITEM getSelectedLibrary () const;
	bool isLibrarySelected () const;
	bool isLabel (HTREEITEM treeItem) const;
	HTREEITEM getSelectedLabel () const;
	bool isLabelSelected () const;

	void addFamily (HTREEITEM rootItem, ScriptGroup * scriptGroup, CString const & rootName, TreeViewIconType icon, SwgConversationEditorDoc::CreateEmptyScriptFunction createEmptyScriptFunction);
	void duplicateFamily (HTREEITEM rootItem, ScriptGroup * scriptGroup, CString const & rootName, TreeViewIconType const icon, CString const & text);
	void deleteCondition (HTREEITEM treeItem);
	void deleteAction (HTREEITEM treeItem);
	void deleteTokenTO (HTREEITEM treeItem);
	void deleteTokenDI (HTREEITEM treeItem);
	void deleteTokenDF (HTREEITEM treeItem);
	void deleteLibrary (HTREEITEM treeItem);
	void deleteLabel (HTREEITEM treeItem);

private:

	bool m_imageListSet;
	CImageList m_imageList;

	HTREEITEM m_rootLibrary;
	HTREEITEM m_rootCondition;
	HTREEITEM m_rootAction;
	HTREEITEM m_rootTokenTO;
	HTREEITEM m_rootTokenDI;
	HTREEITEM m_rootTokenDF;
	HTREEITEM m_rootLabel;
	HTREEITEM m_rootTrigger;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
