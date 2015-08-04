//
// WarningView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_WarningView_H
#define INCLUDED_WarningView_H

//-------------------------------------------------------------------

#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

class WarningView : public CListView
{
private:

	bool        imageListSet;
	CImageList  imageList;

protected:

	WarningView (void);
	virtual ~WarningView (void);

	DECLARE_DYNCREATE(WarningView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(WarningView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void update (const TerrainGeneratorHelper::OutputData& outputData);
	void clear (void);

	//{{AFX_VIRTUAL(WarningView)
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
