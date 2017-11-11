// ShaderBuilderView.h : interface of the CShaderBuilderView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHADERBUILDERVIEW_H__CD5DF44A_EDB4_49BB_8464_92947E20EFF7__INCLUDED_)
#define AFX_SHADERBUILDERVIEW_H__CD5DF44A_EDB4_49BB_8464_92947E20EFF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CShaderBuilderView : public CView
{
protected: // create from serialization only
	CShaderBuilderView();
	DECLARE_DYNCREATE(CShaderBuilderView)

// Attributes
public:
	CShaderBuilderDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShaderBuilderView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShaderBuilderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CShaderBuilderView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ShaderBuilderView.cpp
inline CShaderBuilderDoc* CShaderBuilderView::GetDocument()
   { return (CShaderBuilderDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADERBUILDERVIEW_H__CD5DF44A_EDB4_49BB_8464_92947E20EFF7__INCLUDED_)
