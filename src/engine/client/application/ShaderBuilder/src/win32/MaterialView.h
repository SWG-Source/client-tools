#if !defined(AFX_MATERIALVIEW_H__A1996024_D7E4_47C2_B55D_4300F3733000__INCLUDED_)
#define AFX_MATERIALVIEW_H__A1996024_D7E4_47C2_B55D_4300F3733000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MaterialView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMaterialView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CMaterialNode;

class CMaterialView : public CFormView
{
public:
	CMaterialView();
	virtual ~CMaterialView();
	DECLARE_DYNCREATE(CMaterialView)

// Form Data
public:
	//{{AFX_DATA(CMaterialView)
	enum { IDD = IDD_MATERIAL };
	CEdit	m_power;
	CEdit	m_emissiveBlue;
	CEdit	m_emissiveGreen;
	CEdit	m_emissiveRed;
	CEdit	m_emissiveAlpha;
	CEdit	m_specularBlue;
	CEdit	m_specularGreen;
	CEdit	m_specularRed;
	CEdit	m_specularAlpha;
	CEdit	m_ambientBlue;
	CEdit	m_ambientGreen;
	CEdit	m_ambientRed;
	CEdit	m_ambientAlpha;
	CEdit	m_diffuseBlue;
	CEdit	m_diffuseGreen;
	CEdit	m_diffuseRed;
	CEdit	m_diffuseAlpha;
	//}}AFX_DATA
// Attributes
public:

	CMaterialNode *materialNode;

// Operations
public:

	void SetNode(CMaterialNode &node);
	void ColorPick(CEdit &red, CEdit &green, CEdit &blue);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaterialView)
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
	//{{AFX_MSG(CMaterialView)
	afx_msg void OnChangeDiffusealpha();
	afx_msg void OnChangeDiffuseRed();
	afx_msg void OnChangeDiffuseGreen();
	afx_msg void OnChangeDiffuseblue();
	afx_msg void OnChangeAmbientalpha();
	afx_msg void OnChangeAmbientred();
	afx_msg void OnChangeAmbientgreen();
	afx_msg void OnChangeAmbientblue();
	afx_msg void OnChangeSpecularAlpha();
	afx_msg void OnChangeSpecularred();
	afx_msg void OnChangeSpeculargreen();
	afx_msg void OnChangeSpecularblue();
	afx_msg void OnChangeEmissivealpha();
	afx_msg void OnChangeEmissivered();
	afx_msg void OnChangeEmissivegreen();
	afx_msg void OnChangeEmissiveblue();
	afx_msg void OnChangePower();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MATERIALVIEW_H__A1996024_D7E4_47C2_B55D_4300F3733000__INCLUDED_)
