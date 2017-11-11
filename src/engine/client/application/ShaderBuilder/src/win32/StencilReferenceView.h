#if !defined(AFX_STENCILREFERENCEVIEW_H__99C09D0C_1701_408D_A8BF_427F405905DA__INCLUDED_)
#define AFX_STENCILREFERENCEVIEW_H__99C09D0C_1701_408D_A8BF_427F405905DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StencilReferenceView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStencilReferenceView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CStencilReferenceValueNode;

class CStencilReferenceView : public CFormView
{
protected:
	CStencilReferenceView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CStencilReferenceView)

// Form Data
public:
	//{{AFX_DATA(CStencilReferenceView)
	enum { IDD = IDD_STENCILREFERENCE };
	CEdit	m_stencilReferenceValue;
	//}}AFX_DATA

// Attributes
public:

	CStencilReferenceValueNode *stencilReferenceValueNode;

// Operations
public:

	void SetNode(CStencilReferenceValueNode &node);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStencilReferenceView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CStencilReferenceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CStencilReferenceView)
	afx_msg void OnChangeStencilreferencevalue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STENCILREFERENCEVIEW_H__99C09D0C_1701_408D_A8BF_427F405905DA__INCLUDED_)
