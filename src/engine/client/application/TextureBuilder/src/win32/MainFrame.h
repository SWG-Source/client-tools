// ======================================================================
//
// MainFrame.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_MAINFRAME_H__2BF2CF53_B5D5_4BB5_B048_B3FA087840EB__INCLUDED_)
#define AFX_MAINFRAME_H__2BF2CF53_B5D5_4BB5_B048_B3FA087840EB__INCLUDED_

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ======================================================================

class MainFrame : public CMDIFrameWnd
{
public:

	MainFrame();
	virtual ~MainFrame();

	void         getClientRect(RECT &rect) const;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(MainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated message map functions
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	DECLARE_DYNAMIC(MainFrame)

private:

	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

};

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

#endif
