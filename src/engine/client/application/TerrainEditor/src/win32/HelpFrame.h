//
// HelpFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_HelpFrame_H
#define INCLUDED_HelpFrame_H

//-------------------------------------------------------------------

class HelpFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(HelpFrame)

protected:

	CString     m_windowName;
	CToolBar    m_wndToolBar;

protected:

	HelpFrame();           

public:

	//{{AFX_VIRTUAL(HelpFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~HelpFrame();

	//{{AFX_MSG(HelpFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
