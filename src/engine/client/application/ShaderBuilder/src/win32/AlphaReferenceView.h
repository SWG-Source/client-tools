#if !defined(AFX_ALPHAREFERENCEVIEW_H__0998B936_1C16_4A97_8DD5_5E52825A3ACF__INCLUDED_)
#define AFX_ALPHAREFERENCEVIEW_H__0998B936_1C16_4A97_8DD5_5E52825A3ACF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlphaReferenceView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAlphaReferenceView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CAlphaReferenceValueNode;

class CAlphaReferenceView : public CFormView
{
protected:
	CAlphaReferenceView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAlphaReferenceView)

// Form Data
public:
	//{{AFX_DATA(CAlphaReferenceView)
	enum { IDD = IDD_ALPHAREFERENCE };
	CSliderCtrl	m_slider;
	CEdit	m_alphaReferenceValue;
	//}}AFX_DATA

// Attributes
public:

	CAlphaReferenceValueNode *alphaReferenceValueNode;

// Operations
public:

	void SetNode(CAlphaReferenceValueNode &node);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlphaReferenceView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAlphaReferenceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CAlphaReferenceView)
	afx_msg void OnChangeAlphareferencevalue();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALPHAREFERENCEVIEW_H__0998B936_1C16_4A97_8DD5_5E52825A3ACF__INCLUDED_)
