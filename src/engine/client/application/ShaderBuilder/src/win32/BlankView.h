#if !defined(AFX_BLANKVIEW_H__E0B97AF0_1F5E_4135_BCCB_9BEF1BB4FD0F__INCLUDED_)
#define AFX_BLANKVIEW_H__E0B97AF0_1F5E_4135_BCCB_9BEF1BB4FD0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlankView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBlankView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CBlankView : public CFormView
{
public:
	CBlankView();
	virtual ~CBlankView();
	DECLARE_DYNCREATE(CBlankView)

// Form Data
public:
	//{{AFX_DATA(CBlankView)
	enum { IDD = IDD_BLANK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlankView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CBlankView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLANKVIEW_H__E0B97AF0_1F5E_4135_BCCB_9BEF1BB4FD0F__INCLUDED_)
