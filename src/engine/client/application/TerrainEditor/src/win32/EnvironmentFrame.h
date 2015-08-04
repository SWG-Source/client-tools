//
// EnvironmentFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_EnvironmentFrame_H
#define INCLUDED_EnvironmentFrame_H

//-------------------------------------------------------------------

class EnvironmentFrame : public CMDIChildWnd
{
private:

	CString  m_windowName;
	CToolBar m_wndToolBar;

	DECLARE_DYNCREATE(EnvironmentFrame)

protected:

	EnvironmentFrame (void);
	virtual ~EnvironmentFrame (void);

	//{{AFX_MSG(EnvironmentFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	void reset () const;

	//{{AFX_VIRTUAL(EnvironmentFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
