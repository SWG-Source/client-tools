// ======================================================================
//
// ChildFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ChildFrame_H
#define INCLUDED_ChildFrame_H

// ======================================================================

#include "AW_CMultiViewSplitter.h"

class DraftSchematicTreeView;
class FormAttributeView;
class FormEmptyView;
class FormPropertyView;
class FormSlotView;

// ======================================================================

class ChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ChildFrame)

public:

	ChildFrame();

	//{{AFX_VIRTUAL(ChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

public:

	virtual ~ChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(ChildFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setViewEmpty ();
	void setViewProperty ();
	void setViewSlots ();
	void setViewAttributes ();

private:

	bool m_initialized;

	AW_CMultiViewSplitter m_splitter;

	DraftSchematicTreeView * m_treeView;

	int m_emptyViewId;
	FormEmptyView * m_emptyView;

	int m_propertyViewId;
	FormPropertyView * m_propertyView;

	int m_slotViewId;
	FormSlotView * m_slotView;

	int m_attributeViewId;
	FormAttributeView * m_attributeView;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
