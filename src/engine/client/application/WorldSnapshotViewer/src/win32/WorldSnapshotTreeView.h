//===================================================================
//
// WorldSnapshotTreeView.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotTreeView_H
#define INCLUDED_WorldSnapshotTreeView_H

//===================================================================

#include "sharedMath/Vector.h"

//===================================================================

class WorldSnapshotTreeView : public CTreeView
{
protected:

	WorldSnapshotTreeView ();
	DECLARE_DYNCREATE(WorldSnapshotTreeView)

public:

	//{{AFX_VIRTUAL(WorldSnapshotTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~WorldSnapshotTreeView ();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(WorldSnapshotTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAppAbout();
	afx_msg void OnButtonFind();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	const Vector getPosition_p (HTREEITEM item) const;
	const Vector getPosition_w (HTREEITEM item) const;

private:

	HTREEITEM m_nameRoot;
	HTREEITEM m_nodeRoot;
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif
