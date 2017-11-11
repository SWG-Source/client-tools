// ======================================================================
//
// MainFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_MainFrame_H
#define INCLUDED_MainFrame_H

// ======================================================================

class MainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(MainFrame)

public:

	MainFrame();

	//{{AFX_VIRTUAL(MainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:

	virtual ~MainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar  m_wndDlgBar;

protected:

	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

