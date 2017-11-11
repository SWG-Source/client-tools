#if !defined(AFX_TEXTUREFACTORVIEW_H__B3C715AF_0DEF_4658_A56D_F9E5C714BFD4__INCLUDED_)
#define AFX_TEXTUREFACTORVIEW_H__B3C715AF_0DEF_4658_A56D_F9E5C714BFD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureFactorView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureFactorView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTextureFactorNode;
#include "colorbutton.h"

class CTextureFactorView : public CFormView
{
protected:
	CTextureFactorView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextureFactorView)

// Form Data
public:
	//{{AFX_DATA(CTextureFactorView)
	enum { IDD = IDD_TEXTUREFACTOR };
	CEdit	m_blue;
	CEdit	m_green;
	CEdit	m_red;
	CEdit	m_alpha;
	//}}AFX_DATA

	bool      m_once;
	CColorBtn m_colorButton;

// Attributes
public:

	CTextureFactorNode *textureFactorNode;

// Operations
public:

	void SetNode(CTextureFactorNode &node);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureFactorView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextureFactorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SetButtonColor();

	// Generated message map functions
	//{{AFX_MSG(CTextureFactorView)
	afx_msg void OnChangeAlpha();
	afx_msg void OnChangeRed();
	afx_msg void OnChangeGreen();
	afx_msg void OnChangeBlue();
	//}}AFX_MSG

	afx_msg LONG OnColorChange( UINT id, LONG value );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREFACTORVIEW_H__B3C715AF_0DEF_4658_A56D_F9E5C714BFD4__INCLUDED_)
