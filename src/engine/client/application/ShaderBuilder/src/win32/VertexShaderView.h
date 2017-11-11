#if !defined(AFX_VERTEXSHADERVIEW_H__F18BBE40_2659_41C9_B525_A8C383FFA257__INCLUDED_)
#define AFX_VERTEXSHADERVIEW_H__F18BBE40_2659_41C9_B525_A8C383FFA257__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VertexShaderView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Node.h"

class CVertexShaderView : public CFormView
{
protected:
	CVertexShaderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CVertexShaderView)

// Form Data
public:
	//{{AFX_DATA(CVertexShaderView)
	enum { IDD = IDD_VERTEXSHADER };
	CEdit	m_fileName;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CVertexShaderNode & node);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVertexShaderView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CVertexShaderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CVertexShaderNode *m_vertexShaderNode;

	// Generated message map functions
	//{{AFX_MSG(CVertexShaderView)
	afx_msg void OnChangePixelshaderTextEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERTEXSHADERVIEW_H__F18BBE40_2659_41C9_B525_A8C383FFA257__INCLUDED_)
