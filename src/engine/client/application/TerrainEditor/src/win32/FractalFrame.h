//
// FractalFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FractalFrame_H
#define INCLUDED_FractalFrame_H

//-------------------------------------------------------------------

class FractalFrame : public CMDIChildWnd
{
private:

	CString  m_windowName;
	CToolBar m_wndToolBar;

	DECLARE_DYNCREATE(FractalFrame)

protected:

	FractalFrame (void);
	virtual ~FractalFrame (void);

	//{{AFX_MSG(FractalFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	void reset () const;

	//{{AFX_VIRTUAL(FractalFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
