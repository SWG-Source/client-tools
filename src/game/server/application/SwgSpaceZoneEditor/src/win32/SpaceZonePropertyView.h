// ======================================================================
//
// SpaceZonePropertyView.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceZonePropertyView_H
#define INCLUDED_SpaceZonePropertyView_H

// ======================================================================

#include "SwgSpaceZoneEditorDoc.h"

class PropertyListCtrl;

// ======================================================================

class SpaceZonePropertyView : public CView
{
protected: 

	SpaceZonePropertyView();
	DECLARE_DYNCREATE(SpaceZonePropertyView)

public:

	//{{AFX_VIRTUAL(SpaceZonePropertyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~SpaceZonePropertyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	afx_msg LONG OnPropertyChanged(UINT hiControlId_loItemChanged, LONG lPropertyType);

	//{{AFX_MSG(SpaceZonePropertyView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void createPropertyListCtrl();

private:

	bool m_initialized;
	PropertyListCtrl * m_properties;
	SwgSpaceZoneEditorDoc::Object * m_object;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
