//
// BookmarkView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef BOOKMARKVIEW_H
#define BOOKMARKVIEW_H

//-------------------------------------------------------------------

class BookmarkView : public CListView
{
protected:

	BookmarkView (void);
	DECLARE_DYNCREATE(BookmarkView)
	virtual ~BookmarkView (void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(BookmarkView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//{{AFX_VIRTUAL(BookmarkView)
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
