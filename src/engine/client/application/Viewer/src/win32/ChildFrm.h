//===================================================================
//
// ChildFrm.h
// copyright 1998, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ChildFrm_H
#define INCLUDED_ChildFrm_H

//===================================================================

class CChildFrame : public CMDIChildWnd
{

	DECLARE_DYNCREATE(CChildFrame)

public:

	CChildFrame();
	virtual ~CChildFrame();

	//{{AFX_VIRTUAL(CChildFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(CChildFrame)
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
