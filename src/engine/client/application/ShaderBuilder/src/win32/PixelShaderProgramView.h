#if !defined(AFX_PIXELSHADERPROGRAMVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D7__INCLUDED_)
#define AFX_PIXELSHADERPROGRAMVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PixelShaderProgramView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderProgramView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CPixelShaderProgramNode;

class CPixelShaderProgramView : public CFormView
{
protected:
	CPixelShaderProgramView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPixelShaderProgramView)

// Form Data
public:
	//{{AFX_DATA(CPixelShaderProgramView)
	enum { IDD = IDD_PIXELSHADERPROGRAM };
	CEdit	m_source;
	CEdit	m_fileName;
	CButton	m_create;
	CButton	m_compile;
	CEdit	m_error;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CPixelShaderProgramNode &node);
	void SwitchAway();

	bool Compile();
	virtual BOOL PreTranslateMessage(MSG *);

	CFont*                    m_font;
	CPixelShaderProgramNode  *m_pixelShaderProgramNode;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPixelShaderProgramView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPixelShaderProgramView();
	bool CompileAsm();
	bool CompileHlsl();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPixelShaderProgramView)
	afx_msg void OnPixelshaderprogramCompile();
	afx_msg void OnChangePixelshaderprogramSource();
	afx_msg void OnPixelshaderprogramCreate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PIXELSHADERPROGRAMVIEW_H__76C7B8B8_380A_4343_BA1B_BDA3EB6AF6D7__INCLUDED_)
