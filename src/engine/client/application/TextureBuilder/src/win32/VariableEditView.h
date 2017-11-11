// ======================================================================
//
// VariableEditView.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_VARIABLEEDITVIEW_H__F3BA14FA_C6B1_4755_B1FF_24B51C13C5A1__INCLUDED_)
#define AFX_VARIABLEEDITVIEW_H__F3BA14FA_C6B1_4755_B1FF_24B51C13C5A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

class CBrush;
class TextureBuilderDoc;
class VariableControl;
class VariableElement;

// ======================================================================

class VariableEditView : public CScrollView
{
public:

	typedef stdvector<VariableElement*>::fwd  VariableElementVector;
	typedef stdvector<VariableControl*>::fwd  VariableControlVector;

public:

	void replaceVariableElements(VariableElementVector &variableElements);

	//{{AFX_VIRTUAL(VariableEditView)
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:

	VariableEditView();
	virtual ~VariableEditView();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_DYNCREATE(VariableEditView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(VariableEditView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	TextureBuilderDoc *GetDocument();

	void               setupControlDimensions();

private:

	VariableControlVector *m_variableControls;

	int                    m_viewWidth;
	int                    m_viewHeight;

	HBRUSH                 m_eraseBrush;

private:
	// disabled
	VariableEditView(const VariableEditView&);
	VariableEditView &operator =(const VariableEditView&);
};

// ======================================================================

#ifndef _DEBUG
inline TextureBuilderDoc *VariableEditView::GetDocument()
{
	return reinterpret_cast<TextureBuilderDoc*>(CScrollView::GetDocument());
}
#endif

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
