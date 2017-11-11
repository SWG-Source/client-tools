#if !defined(AFX_TEXTURECOORDINATESETVIEW_H__8B4F14E0_5B02_4B7D_BAFE_FE52D52C4E56__INCLUDED_)
#define AFX_TEXTURECOORDINATESETVIEW_H__8B4F14E0_5B02_4B7D_BAFE_FE52D52C4E56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureCoordinateSetView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureCoordinateSetView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTextureCoordinateSetNode;

class CTextureCoordinateSetView : public CFormView
{
protected:
	CTextureCoordinateSetView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextureCoordinateSetView)

// Form Data
public:
	//{{AFX_DATA(CTextureCoordinateSetView)
	enum { IDD = IDD_TEXTURECOORDINATESET };
	CComboBox	m_textureCoordinateSetIndex;
	//}}AFX_DATA

// Attributes
public:

	CTextureCoordinateSetNode	*textureCoordinateSetNode;

// Operations
public:

	void SetNode(CTextureCoordinateSetNode &node);
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureCoordinateSetView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextureCoordinateSetView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTextureCoordinateSetView)
	afx_msg void OnSelchangeTexturecoordinatesetindex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURECOORDINATESETVIEW_H__8B4F14E0_5B02_4B7D_BAFE_FE52D52C4E56__INCLUDED_)
