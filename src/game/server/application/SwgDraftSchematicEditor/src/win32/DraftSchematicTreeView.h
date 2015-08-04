// ======================================================================
//
// DraftSchematicTreeView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DraftSchematicTreeView_H
#define INCLUDED_DraftSchematicTreeView_H

// ======================================================================

#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

class DraftSchematicTreeView : public CTreeView
{
protected:

	DraftSchematicTreeView();           
	DECLARE_DYNCREATE(DraftSchematicTreeView)

	//{{AFX_VIRTUAL(DraftSchematicTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	virtual ~DraftSchematicTreeView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(DraftSchematicTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTreeAddattribute();
	afx_msg void OnTreeAddslot();
	afx_msg void OnTreeDelete();
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	SwgDraftSchematicEditorDoc::ItemData * getSelectedItemData ();

private:

	void addAttributeToTree (DraftSchematic::Attribute * const attribute);
	void addSlotToTree (DraftSchematic::Slot * const slot);

	void selectItem (HTREEITEM treeItem);
	SwgDraftSchematicEditorDoc::ItemData * getItemData (HTREEITEM treeItem);

	void refreshIcons (HTREEITEM treeItem);
	void refreshIcons ();

private:

	bool m_imageListSet;
	CImageList m_imageList;

	HTREEITEM m_rootProperty;
	HTREEITEM m_rootSlots;
	HTREEITEM m_rootAttributes;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
