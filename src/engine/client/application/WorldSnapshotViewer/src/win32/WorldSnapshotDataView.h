//===================================================================
//
// WorldSnapshotDataView.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotDataView_H
#define INCLUDED_WorldSnapshotDataView_H

//===================================================================

class WorldSnapshotViewerDoc;

//===================================================================

class WorldSnapshotDataView : public CView
{
protected:

	WorldSnapshotDataView();
	DECLARE_DYNCREATE(WorldSnapshotDataView)

public:

	//{{AFX_VIRTUAL(WorldSnapshotDataView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

public:

	virtual ~WorldSnapshotDataView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(WorldSnapshotDataView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif
