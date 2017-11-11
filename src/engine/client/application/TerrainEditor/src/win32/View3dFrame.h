//
// View3dFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_View3dFrame_H
#define INCLUDED_View3dFrame_H

//-------------------------------------------------------------------

class View3dFrame : public CMDIChildWnd
{
private:

	CString     m_windowName;
	CToolBar    m_wndToolBar;

private:

	DECLARE_DYNCREATE(View3dFrame)

protected:

	View3dFrame();           

public:

	//{{AFX_VIRTUAL(View3dFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~View3dFrame();

	//{{AFX_MSG(View3dFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
