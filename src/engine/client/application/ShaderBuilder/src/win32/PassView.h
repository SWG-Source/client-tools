#if !defined(AFX_PASSVIEW_H__3CE1773E_78C0_4F5E_81A9_F13B4DFC8BA0__INCLUDED_)
#define AFX_PASSVIEW_H__3CE1773E_78C0_4F5E_81A9_F13B4DFC8BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PassView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPassView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "resource.h"
#include "Node.h"
#include "TooltipComboBox.h"
#include <vector>
#include <string>

class CPassView : public CFormView
{
protected:
	CPassView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPassView)

// Form Data
public:
	//{{AFX_DATA(CPassView)
	enum { IDD = IDD_PASS };
	CButton	m_heat;
	CComboBox	m_fogMode;
	CButton	m_writeMaskA;
	CButton	m_writeMaskB;
	CButton	m_writeMaskG;
	CButton	m_writeMaskR;
	CEdit	m_textureFactorTag;
	CEdit	m_textureFactor2Tag;
	CComboBox	m_shadeMode;
	CButton	m_ditherEnable;
	CEdit	m_stencilWriteMask;
	CEdit	m_stencilMask;
	CEdit	m_stencilReferenceTag;
	CComboBox	m_stencilCompareFunction;
	CComboBox	m_stencilPassOperation;
	CComboBox	m_stencilZFailOperation;
	CComboBox	m_stencilFailOperation;
	CComboBox	m_stencilCounterClockwiseCompareFunction;
	CComboBox	m_stencilCounterClockwisePassOperation;
	CComboBox	m_stencilCounterClockwiseZFailOperation;
	CComboBox	m_stencilCounterClockwiseFailOperation;
	CButton	m_stencilEnable;
	CButton	m_stencilTwoSidedMode;
	CComboBox	m_alphaTestFunction;
	CEdit	m_alphaTestTag;
	CButton	m_alphaTestEnable;
	CComboBox	m_alphaBlendOperation;
	CTooltipComboBox	m_alphaBlendDestination;
	CTooltipComboBox	m_alphaBlendSource;
	CButton	m_alphaBlendEnable;
	CComboBox	m_zCompare;
	CButton	m_zWrite;
	CButton	m_zEnable;
	CToolTipCtrl m_toolTipControl;
	CEdit m_materialTag;
	CEdit m_textureScrollTag;
	//}}AFX_DATA

// Attributes
public:

	CPassNode *passNode;
	std::vector<std::string> m_blendtypeTooltipItems;

// Operations
public:

	void SetNode(CPassNode &node);
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
	//{{AFX_VIRTUAL(CPassView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPassView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPassView)
	afx_msg void OnZenable();
	afx_msg void OnZwrite();
	afx_msg void OnAlphablendenable();
	afx_msg void OnAlphatestenable();
	afx_msg void OnDitherenable();
	afx_msg void OnHeat();
	afx_msg void OnStencilenable();
	afx_msg void OnStenciltwosidedmode();
	afx_msg void OnSelchangeZcomparefunction();
	afx_msg void OnSelchangeSourceblend();
	afx_msg void OnSelchangeDestinationblend();
	afx_msg void OnSelchangeBlendoperation();
	afx_msg void OnSelchangeAlphatestfunction();
	afx_msg void OnSelchangeShademode();
	afx_msg void OnSelchangeStencilfail();
	afx_msg void OnSelchangeStencilzfail();
	afx_msg void OnSelchangeStencilpass();
	afx_msg void OnSelchangeStencilfunction();
	afx_msg void OnSelchangeStencilCounterclockwisefail();
	afx_msg void OnSelchangeStencilCounterclockwisezfail();
	afx_msg void OnSelchangeStencilCounterclockwisepass();
	afx_msg void OnSelchangeStencilCounterclockwisefunction();
	afx_msg void OnChangeAlphareferencetag();
	afx_msg void OnChangeTexturefactortag();
	afx_msg void OnChangeTexturefactor2tag();
	afx_msg void OnChangeStencilreferencetag();
	afx_msg void OnChangeStencilmask();
	afx_msg void OnChangeStencilwritemask();
	afx_msg void OnChangeLightingmaterialtag();
	afx_msg void OnChangeTexturescrolltag();
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnKillfocusEdit7();
	afx_msg void OnKillfocusTexturescrolltag();
	afx_msg void OnCheckwriteenabler();
	afx_msg void OnCheckwriteenableg();
	afx_msg void OnCheckwriteenableb();
	afx_msg void OnCheckwriteenablea();
	afx_msg void OnSelchangeFogmode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PASSVIEW_H__3CE1773E_78C0_4F5E_81A9_F13B4DFC8BA0__INCLUDED_)
