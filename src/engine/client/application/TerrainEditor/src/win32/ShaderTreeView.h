//
// ShaderTreeView.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ShaderTreeView_H
#define INCLUDED_ShaderTreeView_H

//-------------------------------------------------------------------

#include "sharedTerrain/ShaderGroup.h"

//-------------------------------------------------------------------

class ShaderTreeView : public CTreeView
{
private:

	ShaderGroup*    shaderGroup;

	bool            imageListSet;
	CImageList      imageList;

	bool            deletingUnused;

private:

	//-- drag and drop
	enum DragMode
	{
		DM_nothing,
		DM_move,
		DM_copy
	};

private:

	DragMode        m_dragMode;
	CImageList*     m_pDragImage;
	HTREEITEM       m_htiDrag, m_htiDrop, m_htiOldDrop;
	bool            m_bLDragging;
	UINT            m_idTimer;
	HCURSOR         m_moveCursor;
	HCURSOR         m_copyCursor;

private:

	void            OnLButtonUpForDrag (bool move);
	void            OnLButtonUpForMove ();
	void            OnLButtonUpForCopy ();

private:

	bool      childExistsInFamily (int familyId, const char* childName) const;
	bool      familyExists (const char* familyName) const;
	HTREEITEM findFamily (HTREEITEM hItem, int familyId) const;
	HTREEITEM insertChild (HTREEITEM familyItem, int familyId, const CString& childName);
	HTREEITEM insertFamily (int familyId, HTREEITEM& afterItem);

	const CString CreateUniqueChildName (int familyId);
	const CString CreateUniqueFamilyName (const CString* base=0);

private:

	void      expandBranch (HTREEITEM item);
	void      expandAll (void);

	bool      isFamily (HTREEITEM selection) const;
	HTREEITEM getSelectedFamily (void) const;
	bool      isFamilySelected (void) const;

	bool      isChild (HTREEITEM selection) const;
	HTREEITEM getSelectedChild (void) const;
	bool      isChildSelected (void) const;

	void      updateShaderView (void);

protected:

	ShaderTreeView();           
	DECLARE_DYNCREATE(ShaderTreeView)

public:

	void addFamily (const CString& familyName, const ArrayList<CString*>& children);
	void reset (void);

	//{{AFX_VIRTUAL(ShaderTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ShaderTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ShaderTreeView)
	afx_msg void OnNewshaderfamily();
	afx_msg void OnUpdateNewshaderfamily(CCmdUI* pCmdUI);
	afx_msg void OnNewshaderchild();
	afx_msg void OnUpdateNewshaderchild(CCmdUI* pCmdUI);
	afx_msg void OnPromoteshader();
	afx_msg void OnUpdatePromoteshader(CCmdUI* pCmdUI);
	afx_msg void OnDemoteshader();
	afx_msg void OnUpdateDemoteshader(CCmdUI* pCmdUI);
	afx_msg void OnDeleteshader();
	afx_msg void OnUpdateDeleteshader(CCmdUI* pCmdUI);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFindshader();
	afx_msg void OnUpdateFindshader(CCmdUI* pCmdUI);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonCalculatecolor();
	afx_msg void OnUpdateButtonCalculatecolor(CCmdUI* pCmdUI);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnGroupDelete();
	afx_msg void OnGroupRename();
	afx_msg void OnButtonDeleteunused();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGroupFindrulesusingthisfamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
