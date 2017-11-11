// ======================================================================
//
// ConstructionTreeView.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_CONSTRUCTIONTREEVIEW_H__005CD9FE_60BB_4074_AC8C_AB12DAC93417__INCLUDED_)
#define AFX_CONSTRUCTIONTREEVIEW_H__005CD9FE_60BB_4074_AC8C_AB12DAC93417__INCLUDED_

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

class Element;
class TextureBuilderDoc;

// ======================================================================

class ConstructionTreeView : public CTreeView
{
public:

	typedef stdvector<Element*>::fwd  ElementVector;

public:

	virtual ~ConstructionTreeView();

	TextureBuilderDoc* GetDocument();

	void     appendElementToParent(const Element *parent, Element *element, bool ensureVisible = true);
	void     updateElement(const Element *element, bool ensureVisible);

	Element *getSelectedElement();
	Element *getSelectedElementParent();

	int      getSelectedElementChildIndex() const;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(ConstructionTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

protected:

	// create from serialization only
	ConstructionTreeView();
	DECLARE_DYNCREATE(ConstructionTreeView)

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated message map functions
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_MSG(ConstructionTreeView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTextureGroupAddSingleTexture();
	afx_msg void OnCommandGroupAddDrawTextureCommand();
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void  appendTreeItem(HTREEITEM parentTreeItem, const Element &element, bool ensureVisible = true, HTREEITEM insertAfter = TVI_LAST);
	void  buildTree();

private:

	CImageList *m_treeImageList;

	CImageList *m_dragImageList;
	bool        m_leftDragging;
	Element    *m_dragElement;
	Element    *m_dropTargetElement;
	HCURSOR     m_dropCursor;
	HCURSOR     m_noDropCursor;

private:
	// disabled
	ConstructionTreeView(const ConstructionTreeView&);
	ConstructionTreeView &operator =(const ConstructionTreeView&);
};

// ======================================================================

#ifndef _DEBUG
inline TextureBuilderDoc* ConstructionTreeView::GetDocument()
{
	return reinterpret_cast<TextureBuilderDoc*>(m_pDocument);
}
#endif

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
