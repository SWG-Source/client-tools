#if !defined(AFX_PIXELSHADERVIEW1_H__7032242B_1EC8_47BE_B334_4F331DB49FDD__INCLUDED_)
#define AFX_PIXELSHADERVIEW1_H__7032242B_1EC8_47BE_B334_4F331DB49FDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PixelShaderView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Node.h"

class CPixelShaderView : public CFormView
{
public:
	CPixelShaderView();
	virtual ~CPixelShaderView();
	DECLARE_DYNCREATE(CPixelShaderView)

	void SetNode(CPixelShaderNode &pixelShaderNode);

	CPixelShaderNode *m_pixelShaderNode;
// Form Data
public:
	//{{AFX_DATA(CPixelShaderView)
	enum { IDD = IDD_PIXELSHADER };
	CEdit	m_fileName;
	CEdit	m_t0;
	CEdit	m_t1;
	CEdit	m_t2;
	CEdit	m_t3;
	CEdit	m_t4;
	CEdit	m_t5;
	CEdit	m_t6;
	CEdit	m_t7;
	CEdit	m_t8;
	CEdit	m_t9;
	CEdit	m_t10;
	CEdit	m_t11;
	CEdit	m_t12;
	CEdit	m_t13;
	CEdit	m_t14;
	CEdit	m_t15;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	template <class T> void UpdateTaggedTree(CTaggedTreeNode<T> &treeNode, CString &nodeValue, const CString &newValue)
	{
		if (nodeValue != newValue)
		{
			treeNode.Remove(nodeValue);
			nodeValue = newValue;
			treeNode.Add(newValue);
		}
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPixelShaderView)
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
	//{{AFX_MSG(CPixelShaderView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEdit2();
	afx_msg void OnChangeT0();
	afx_msg void OnChangeT1();
	afx_msg void OnChangeT2();
	afx_msg void OnChangeT3();
	afx_msg void OnChangeT4();
	afx_msg void OnChangeT5();
	afx_msg void OnChangeT6();
	afx_msg void OnChangeT7();
	afx_msg void OnChangeT8();
	afx_msg void OnChangeT9();
	afx_msg void OnChangeT10();
	afx_msg void OnChangeT11();
	afx_msg void OnChangeT12();
	afx_msg void OnChangeT13();
	afx_msg void OnChangeT14();
	afx_msg void OnChangeT15();
	afx_msg void OnKillfocusT0();
	afx_msg void OnKillfocusT1();
	afx_msg void OnKillfocusT2();
	afx_msg void OnKillfocusT3();
	afx_msg void OnKillfocusT4();
	afx_msg void OnKillfocusT5();
	afx_msg void OnKillfocusT6();
	afx_msg void OnKillfocusT7();
	afx_msg void OnKillfocusT8();
	afx_msg void OnKillfocusT9();
	afx_msg void OnKillfocusT10();
	afx_msg void OnKillfocusT11();
	afx_msg void OnKillfocusT12();
	afx_msg void OnKillfocusT13();
	afx_msg void OnKillfocusT14();
	afx_msg void OnKillfocusT15();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PIXELSHADERVIEW1_H__7032242B_1EC8_47BE_B334_4F331DB49FDD__INCLUDED_)
