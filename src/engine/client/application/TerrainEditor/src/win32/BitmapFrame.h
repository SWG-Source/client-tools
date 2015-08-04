//
// BitmapFrame.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BitmapFrame_H
#define INCLUDED_BitmapFrame_H

//-------------------------------------------------------------------

class BitmapFrame : public CMDIChildWnd
{
private:

	CString  m_windowName;
	CToolBar m_wndToolBar;

	DECLARE_DYNCREATE(BitmapFrame)

protected:

	BitmapFrame (void);
	virtual ~BitmapFrame (void);

	//{{AFX_MSG(BitmapFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	void reset () const;

	//{{AFX_VIRTUAL(BitmapFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
