// ======================================================================
//
// ChildFrame.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ChildFrame_H
#define INCLUDED_ChildFrame_H

// ======================================================================

class ChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ChildFrame)

public:

	ChildFrame();
	void setToolTip(CString const & toolTip);

	//{{AFX_VIRTUAL(ChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

public:

	virtual ~ChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(ChildFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CStatusBar m_wndStatusBar;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
