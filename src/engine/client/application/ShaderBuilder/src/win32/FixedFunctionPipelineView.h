#if !defined(AFX_FIXEDFUNCTIONPIPELINEVIEW_H__D7CEEA66_5A4A_4AA2_9BC2_D993A4ADFF46__INCLUDED_)
#define AFX_FIXEDFUNCTIONPIPELINEVIEW_H__D7CEEA66_5A4A_4AA2_9BC2_D993A4ADFF46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FixedFunctionPipelineView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFixedFunctionPipelineView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Node.h"

class CFixedFunctionPipelineView : public CFormView
{
protected:
	CFixedFunctionPipelineView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFixedFunctionPipelineView)

// Form Data
public:
	//{{AFX_DATA(CFixedFunctionPipelineView)
	enum { IDD = IDD_FIXEDFUNCTIONPIPELINE };
	CComboBox	m_lightingEmissiveColorSource;
	CComboBox	m_lightingSpecularColorSource;
	CComboBox	m_lightingDiffuseColorSource;
	CComboBox	m_lightingAmbientColorSource;
	CButton	m_lightingColorVertex;
	CButton	m_lightingSpecularEnable;
	CButton	m_lighting;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	void SetNode(CFixedFunctionPipelineNode &node);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFixedFunctionPipelineView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFixedFunctionPipelineView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	template <class T> void UpdateTaggedTree(CTaggedTreeNode<T> &treeNode, CString &nodeValue, const CString &newValue)
	{
		if (nodeValue != newValue)
		{
			treeNode.Remove(nodeValue);
			nodeValue = newValue;
			treeNode.Add(newValue);
		}
	}

	CFixedFunctionPipelineNode *fixedFunctionPipelineNode;

	// Generated message map functions
	//{{AFX_MSG(CFixedFunctionPipelineView)
	afx_msg void OnLighting();
	afx_msg void OnSpecularenable();
	afx_msg void OnColorvertex();
	afx_msg void OnSelchangeAmbientmaterialsource();
	afx_msg void OnSelchangeDiffusematerialsource();
	afx_msg void OnSelchangeSpecularmaterialsource();
	afx_msg void OnSelchangeEmissivematerialsource();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIXEDFUNCTIONPIPELINEVIEW_H__D7CEEA66_5A4A_4AA2_9BC2_D993A4ADFF46__INCLUDED_)
