#if !defined(AFX_TEXTUREVIEW_H__11FEF177_6E13_4D1F_BBB6_580D1B375229__INCLUDED_)
#define AFX_TEXTUREVIEW_H__11FEF177_6E13_4D1F_BBB6_580D1B375229__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTextureNode;

class CTextureView : public CFormView
{
public:
	CTextureView();
	virtual ~CTextureView();

	DECLARE_DYNCREATE(CTextureView)

// Form Data
public:
	//{{AFX_DATA(CTextureView)
	enum { IDD = IDD_TEXTURE };
	CComboBox	m_anisotropy;
	CComboBox	m_filterMip;
	CComboBox	m_filterMin;
	CComboBox	m_filterMag;
	CComboBox	m_addressW;
	CComboBox	m_addressV;
	CComboBox	m_addressU;
	CButton	m_placeHolder;
	CEdit	m_textureName;
	//}}AFX_DATA

// Attributes
public:

	CTextureNode *textureNode;

// Operations
public:

	void SetNode(CTextureNode &node);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTextureView)
	afx_msg void OnChangeTexturefilename();
	afx_msg void OnCheckPlaceholder();
	afx_msg void OnTextureBrowse();
	afx_msg void OnKillfocusTexturefilename();
	afx_msg void OnChangeAddressU();
	afx_msg void OnChangeAddressV();
	afx_msg void OnChangeAddressW();
	afx_msg void OnChangeFilterMag();
	afx_msg void OnChangeFilterMin();
	afx_msg void OnChangeFilterMip();
	afx_msg void OnChangeAnisotropy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREVIEW_H__11FEF177_6E13_4D1F_BBB6_580D1B375229__INCLUDED_)
