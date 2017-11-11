#if !defined(AFX_INCLUDEVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D8__INCLUDED_)
#define AFX_INCLUDEVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IncludeView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIncludeView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CIncludeNode;

class CIncludeView : public CFormView
{
protected:
	CIncludeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CIncludeView)

// Form Data
public:
	//{{AFX_DATA(CIncludeView)
	enum { IDD = IDD_INCLUDE };
	CEdit	m_source;
	CEdit	m_fileName;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CIncludeNode &node);
	void SwitchAway();

	virtual BOOL PreTranslateMessage(MSG *);

	CFont*         m_font;
	CIncludeNode  *m_includeNode;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIncludeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CIncludeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CIncludeView)
	afx_msg void OnChangeSource();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INCLUDEVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D8__INCLUDED_)
