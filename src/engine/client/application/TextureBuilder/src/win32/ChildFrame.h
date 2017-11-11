// ======================================================================
//
// ChildFrame.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_CHILDFRAME_H__EBE4D7B9_ACAE_4593_892A_3152A10AB5AF__INCLUDED_)
#define AFX_CHILDFRAME_H__EBE4D7B9_ACAE_4593_892A_3152A10AB5AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ======================================================================

class ChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ChildFrame)

public:

	ChildFrame();
	virtual ~ChildFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(ChildFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated message map functions
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_MSG(ChildFrame)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

#endif
