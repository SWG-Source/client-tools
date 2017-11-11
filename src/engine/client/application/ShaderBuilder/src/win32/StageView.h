#if !defined(AFX_STAGEVIEW_H__85DBB7E9_F028_4295_AE35_AD166766D063__INCLUDED_)
#define AFX_STAGEVIEW_H__85DBB7E9_F028_4295_AE35_AD166766D063__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StageView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStageView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include <vector>
#include <string>
#include "Node.h"
#include "TooltipComboBox.h"

class CStageView : public CFormView
{
protected:
	CStageView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CStageView)

// Form Data
public:
	//{{AFX_DATA(CStageView)
	enum { IDD = IDD_STAGE };
	CButton	m_colorArgument0AlphaReplicate;
	CButton	m_colorArgument2AlphaReplicate;
	CButton	m_colorArgument1AlphaReplicate;
	CButton	m_colorArgument0Complement;
	CButton	m_colorArgument2Complement;
	CButton	m_alphaArgument0Complement;
	CButton	m_alphaArgument2Complement;
	CButton	m_alphaArgument1Complement;
	CButton	m_colorArgument1Complement;
	CEdit	m_textureCoordinateSetTag;
	CEdit	m_textureTag;
	CComboBox	m_textureCoordinateGeneration;
	CComboBox	m_resultArgument;
	CTooltipComboBox	m_alphaArgument0;
	CTooltipComboBox	m_alphaArgument2;
	CTooltipComboBox	m_alphaOperation;
	CTooltipComboBox	m_alphaArgument1;
	CTooltipComboBox	m_colorArgument0;
	CTooltipComboBox	m_colorArgument2;
	CTooltipComboBox	m_colorOperation;
	CTooltipComboBox	m_colorArgument1;
	//}}AFX_DATA

// Attributes
public:

	CStageNode *stageNode;
	std::vector<std::string> m_operationTooltipText;

// Operations
public:

	void SetNode(CStageNode &node);
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
	//{{AFX_VIRTUAL(CStageView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CStageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CStageView)
	afx_msg void OnSelchangeColorargument1();
	afx_msg void OnSelchangeColoroperation();
	afx_msg void OnSelchangeColorargument2();
	afx_msg void OnSelchangeColorargument0();
	afx_msg void OnSelchangeAlphaargument1();
	afx_msg void OnSelchangeAlphaoperation();
	afx_msg void OnSelchangeAlphaargument2();
	afx_msg void OnSelchangeAlphaargument0();
	afx_msg void OnSelchangeResultargument();
	afx_msg void OnSelchangeTexturecoordinategeneration();
	afx_msg void OnChangeTexturetag();
	afx_msg void OnChangeTexturecoordinatesettag();
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnColorargument1complement();
	afx_msg void OnColorargument2complement();
	afx_msg void OnColorargument0complement();
	afx_msg void OnAlphaargument1complement();
	afx_msg void OnAlphaargument2complement();
	afx_msg void OnAlphaargument0complement();
	afx_msg void OnColorargument1alphareplicate();
	afx_msg void OnColorargument2alphareplicate();
	afx_msg void OnColorargument0alphareplicate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAGEVIEW_H__85DBB7E9_F028_4295_AE35_AD166766D063__INCLUDED_)
