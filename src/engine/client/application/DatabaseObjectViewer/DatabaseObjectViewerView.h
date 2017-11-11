// ======================================================================
//
// DatabaseObjectViewerView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseObjectViewerView_H
#define INCLUDED_DatabaseObjectViewerView_H

// ======================================================================

class DatabaseObjectViewerView : public CView
{
protected:

	DatabaseObjectViewerView();           
	DECLARE_DYNCREATE(DatabaseObjectViewerView)

public:

	//{{AFX_VIRTUAL(DatabaseObjectViewerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:

	virtual ~DatabaseObjectViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(DatabaseObjectViewerView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButtonShowstats();
	afx_msg void OnUpdateButtonShowstats(CCmdUI* pCmdUI);
	afx_msg void OnButtonCheckall();
	afx_msg void OnButtonClearall();
	afx_msg void OnButtonGrid();
	afx_msg void OnUpdateButtonGrid(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	
	bool m_showStats;
	bool m_showGrid;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

