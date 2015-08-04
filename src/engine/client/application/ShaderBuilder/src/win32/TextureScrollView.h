#if !defined(AFX_TEXTURESCROLLVIEW_H__30BCB5CF_E002_4371_90B0_82F8C8558E9E__INCLUDED_)
#define AFX_TEXTURESCROLLVIEW_H__30BCB5CF_E002_4371_90B0_82F8C8558E9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureScrollView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureScrollView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTextureScrollNode;

class CTextureScrollView : public CFormView
{
protected:
	CTextureScrollView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextureScrollView)

// Form Data
public:
	//{{AFX_DATA(CTextureScrollView)
	enum { IDD = IDD_TEXTURESCROLL };
	CEdit m_textureScroll1U;
	CEdit m_textureScroll1V;
	CEdit m_textureScroll2U;
	CEdit m_textureScroll2V;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CTextureScrollNode &node);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureScrollView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextureScrollView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CTextureScrollNode  * m_scrollNode;

	// Generated message map functions
	//{{AFX_MSG(CTextureScrollView)
	afx_msg void OnChangeTextureScroll1U();
	afx_msg void OnChangeTextureScroll1V();
	afx_msg void OnChangeTextureScroll2U();
	afx_msg void OnChangeTextureScroll2V();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURESCROLLVIEW_H__30BCB5CF_E002_4371_90B0_82F8C8558E9E__INCLUDED_)
