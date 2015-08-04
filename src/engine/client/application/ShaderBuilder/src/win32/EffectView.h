#if !defined(AFX_EFFECTVIEW_H__DB70F65E_1A11_45CA_BA29_6483EB4168A9__INCLUDED_)
#define AFX_EFFECTVIEW_H__DB70F65E_1A11_45CA_BA29_6483EB4168A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EffectView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEffectView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CEffectNode;

class CEffectView : public CFormView
{
protected:
	CEffectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEffectView)

// Form Data
public:
	//{{AFX_DATA(CEffectView)
	enum { IDD = IDD_EFFECT };
	CButton	m_effectPrelit;
	CStatic	m_name;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CEffectNode &effectNode);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEffectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CEffectNode *m_effectNode;

	// Generated message map functions
	//{{AFX_MSG(CEffectView)
	afx_msg void OnEffectPrelit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTVIEW_H__DB70F65E_1A11_45CA_BA29_6483EB4168A9__INCLUDED_)
