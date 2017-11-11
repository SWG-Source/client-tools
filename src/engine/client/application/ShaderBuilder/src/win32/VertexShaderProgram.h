#if !defined(AFX_VERTEXSHADERPROGRAM_H__EB883FBC_44F2_401A_B47C_193B80ADADE3__INCLUDED_)
#define AFX_VERTEXSHADERPROGRAM_H__EB883FBC_44F2_401A_B47C_193B80ADADE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VertexShaderProgram.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderProgram form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CVertexShaderProgramNode;

class CVertexShaderProgram : public CFormView
{
protected:
	CVertexShaderProgram();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CVertexShaderProgram)

// Form Data
public:
	//{{AFX_DATA(CVertexShaderProgram)
	enum { IDD = IDD_VERTEXSHADERPROGRAM };
	CButton	m_create;
	CButton	m_compileButton;
	CStatic	m_staticText;
	CEdit	m_source;
	CEdit	m_fileName;
	CEdit	m_error;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CVertexShaderProgramNode &node);
	void SwitchAway();

	bool Compile();
	virtual BOOL PreTranslateMessage(MSG *);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVertexShaderProgram)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	virtual ~CVertexShaderProgram();
	bool CompileAsm();
	bool CompileHlsl();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CFont                    *m_font;
	CVertexShaderProgramNode *m_vertexShaderProgramNode;


	// Generated message map functions
	//{{AFX_MSG(CVertexShaderProgram)
	afx_msg void OnVertexshaderprogramCompile();
	afx_msg void OnChangeVertexshaderprogramSource();
	afx_msg void OnVertexshaderprogramCreate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERTEXSHADERPROGRAM_H__EB883FBC_44F2_401A_B47C_193B80ADADE3__INCLUDED_)
