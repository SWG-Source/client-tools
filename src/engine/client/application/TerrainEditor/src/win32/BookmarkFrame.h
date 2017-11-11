//
// BookmarkFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef BOOKMARKFRAME_H
#define BOOKMARKFRAME_H

//-------------------------------------------------------------------

class BookmarkFrame : public CMDIChildWnd
{
private:

	CString  m_windowName;
	CToolBar m_wndToolBar;

	DECLARE_DYNCREATE(BookmarkFrame)

protected:

	BookmarkFrame (void);
	virtual ~BookmarkFrame (void);

	//{{AFX_MSG(BookmarkFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	//{{AFX_VIRTUAL(BookmarkFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
