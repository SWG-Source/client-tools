//
// FindView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FindView_H
#define INCLUDED_FindView_H

//-------------------------------------------------------------------

#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

class FindView : public CListView
{
private:

	bool        imageListSet;
	CImageList  imageList;

protected:

	FindView (void);
	virtual ~FindView (void);

	DECLARE_DYNCREATE(FindView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FindView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFindviewClear();
	afx_msg void OnFindviewFindbyname();
	afx_msg void OnFindviewFindbytype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void update (const TerrainGeneratorHelper::OutputData& outputData) const;
	void clear (void) const;

	//{{AFX_VIRTUAL(FindView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
