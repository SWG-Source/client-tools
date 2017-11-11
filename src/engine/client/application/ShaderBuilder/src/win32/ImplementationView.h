#if !defined(AFX_IMPLEMENTATIONVIEW_H__D0066904_A77B_4EDD_9D85_D8E5E3A607BD__INCLUDED_)
#define AFX_IMPLEMENTATIONVIEW_H__D0066904_A77B_4EDD_9D85_D8E5E3A607BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImplementationView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImplementationView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CImplementationNode;

class CImplementationView : public CFormView
{
protected:
	CImplementationView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CImplementationView)

// Form Data
public:
	//{{AFX_DATA(CImplementationView)
	enum { IDD = IDD_IMPLEMENTATION };
	CButton	m_castsShadows;
	CButton	m_collidable;
	CEdit	m_optionTags;
	CEdit	m_shaderCapabilityCompatibility;
	CEdit	m_phaseTag;
	CStatic	m_name;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CImplementationNode &implementationNode);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImplementationView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CImplementationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CImplementationView)
	afx_msg void OnChangeEdit1();
	afx_msg void OnChangeEdit2();
	afx_msg void OnChangeEdit3();
	afx_msg void OnCheckCollidable();
	afx_msg void OnCheckCastsShadows();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CImplementationNode *m_implementationNode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPLEMENTATIONVIEW_H__D0066904_A77B_4EDD_9D85_D8E5E3A607BD__INCLUDED_)
