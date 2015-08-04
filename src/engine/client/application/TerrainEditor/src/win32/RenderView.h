#if !defined(AFX_RENDERVIEW_H__3FE338D6_85CE_11D4_811C_F0174CC1668B__INCLUDED_)
#define AFX_RENDERVIEW_H__3FE338D6_85CE_11D4_811C_F0174CC1668B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RenderView view

class RenderView : public CView
{
protected:
	RenderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(RenderView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RenderView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~RenderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(RenderView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERVIEW_H__3FE338D6_85CE_11D4_811C_F0174CC1668B__INCLUDED_)
