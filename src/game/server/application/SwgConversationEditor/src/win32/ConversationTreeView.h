// ======================================================================
//
// ConversationTreeView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ConversationTreeView_H
#define INCLUDED_ConversationTreeView_H

// ======================================================================

#include "SwgConversationEditorDoc.h"

class ConversationBranch;
class ConversationResponse;

// ======================================================================

class ConversationTreeView : public CTreeView
{
protected:

	ConversationTreeView();
	DECLARE_DYNCREATE(ConversationTreeView)

public:

	//{{AFX_VIRTUAL(ConversationTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	virtual ~ConversationTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ConversationTreeView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBranchAddresponse();
	afx_msg void OnResponseAddbranch();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonDemote();
	afx_msg void OnButtonPromote();
	afx_msg void OnCollapseall();
	afx_msg void OnExpandall();
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBranchTest();
	afx_msg void OnBranchDelete();
	afx_msg void OnResponseDelete();
	afx_msg void OnUpdateBranchAddresponse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateResponseAddbranch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBranchTest(CCmdUI* pCmdUI);
	afx_msg void OnBranchGroupEchoRecursiveOn();
	afx_msg void OnBranchGroupEchoRecursiveOff();
	afx_msg void OnResponseGroupEchoRecursiveOn();
	afx_msg void OnResponseGroupEchoRecursiveOff();


	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void selectItem (ConversationItem const * conversationItem);
	SwgConversationEditorDoc::ItemData * getSelectedItemData ();

private:

	enum TreeViewIconType
	{
		TVIT_root,
		TVIT_branch,
		TVIT_branchNegated,
		TVIT_branchDebug,
		TVIT_branchNegatedDebug,
		TVIT_branchDefault,
		TVIT_branchNegatedDefault,
		TVIT_branchDebugDefault,
		TVIT_branchNegatedDebugDefault,
		TVIT_response,
		TVIT_responseNegated,
		TVIT_responseDebug,
		TVIT_responseNegatedDebug,
		TVIT_responseDefault,
		TVIT_responseNegatedDefault,
		TVIT_responseDebugDefault,
		TVIT_responseNegatedDebugDefault,
		TVIT_branchLabel,
		TVIT_branchNegatedLabel,
		TVIT_branchDebugLabel,
		TVIT_branchNegatedDebugLabel,
		TVIT_branchDefaultLabel,
		TVIT_branchNegatedDefaultLabel,
		TVIT_branchDebugDefaultLabel,
		TVIT_branchNegatedDebugDefaultLabel,
		TVIT_responseLabel,
		TVIT_responseNegatedLabel,
		TVIT_responseDebugLabel,
		TVIT_responseNegatedDebugLabel,
		TVIT_responseDefaultLabel,
		TVIT_responseNegatedDefaultLabel,
		TVIT_responseDebugDefaultLabel,
		TVIT_responseNegatedDebugDefaultLabel,
		TVIT_branchLink,
		TVIT_branchNegatedLink,
		TVIT_branchDebugLink,
		TVIT_branchNegatedDebugLink,
		TVIT_branchDefaultLink,
		TVIT_branchNegatedDefaultLink,
		TVIT_branchDebugDefaultLink,
		TVIT_branchNegatedDebugDefaultLink,
		TVIT_responseLink,
		TVIT_responseNegatedLink,
		TVIT_responseDebugLink,
		TVIT_responseNegatedDebugLink,
		TVIT_responseDefaultLink,
		TVIT_responseNegatedDefaultLink,
		TVIT_responseDebugDefaultLink,
		TVIT_responseNegatedDebugDefaultLink,
	};

	enum VerifyType
	{
		VT_none,
		VT_branch,
		VT_response
	};

	enum DragMode
	{
		DM_nothing,
		DM_move,
		DM_copy
	};

private:


	TreeViewIconType getIcon (SwgConversationEditorDoc::ItemData const * itemData) const;
	void refreshIcons (HTREEITEM treeItem);

	HTREEITEM find (HTREEITEM hItem, ConversationItem const * conversationItem);

	void addConversationToTree ();
	HTREEITEM addBranchToTree (HTREEITEM parentItem, HTREEITEM afterItem, ConversationBranch * branch) const;
	HTREEITEM addResponseToTree (HTREEITEM parentItem, HTREEITEM afterItem, ConversationResponse * response) const;
	void deleteBranch (HTREEITEM treeItem);
	void deleteBranch ();
	void deleteResponse (HTREEITEM treeItem);
	void deleteResponse ();

	bool isBranch (HTREEITEM treeItem) const;
	HTREEITEM getSelectedBranch () const;
	bool isBranchSelected () const;
	bool isResponse (HTREEITEM treeItem) const;
	HTREEITEM getSelectedResponse () const;
	bool isResponseSelected () const;

	SwgConversationEditorDoc::ItemData * getItemData (HTREEITEM treeItem, VerifyType verifyType = VT_none);
	SwgConversationEditorDoc::ItemData const * getItemData (HTREEITEM treeItem, VerifyType verifyType = VT_none) const;
	void setItemData (HTREEITEM treeItem, SwgConversationEditorDoc::ItemData * itemData) const;

	void promoteBranch ();
	void demoteBranch ();
	void promoteResponse ();
	void demoteResponse ();

	void saveExpandedState (HTREEITEM treeItem);
	void saveExpandedState ();

	void selectItem (HTREEITEM treeItem);

	void OnLButtonUpForMove ();
	void OnLButtonUpForCopy ();

	void copyBranch (HTREEITEM sourceTreeItem, HTREEITEM destinationTreeItem);
	void moveBranch (HTREEITEM sourceTreeItem, HTREEITEM destinationTreeItem);

	SwgConversationEditorDoc * getConversationEditorDoc();

private:

	bool m_imageListSet;
	CImageList m_imageList;

	HCURSOR m_copyCursor;
	HCURSOR m_moveCursor;
	DragMode m_dragMode;
	CImageList * m_pDragImage;
	HTREEITEM m_htiDrag;
	HTREEITEM m_htiDrop;
	HTREEITEM m_htiOldDrop;
	bool m_bLDragging;
	UINT m_idTimer;
    DWORD m_dwDragStart;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

