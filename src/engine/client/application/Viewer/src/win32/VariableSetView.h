// ======================================================================
//
// VariableSetView.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_VARIABLESETVIEW_H__926B596F_BE0B_499C_BA61_2F1AB1469707__INCLUDED_)
#define AFX_VARIABLESETVIEW_H__926B596F_BE0B_499C_BA61_2F1AB1469707__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VariableSetView.h : header file
//

// ======================================================================
// forward declarations
// ======================================================================

class CompositeMesh;
class CustomizationData;
class CustomizationVariable;
class CViewerDoc;
class SkeletalAppearance2;

/////////////////////////////////////////////////////////////////////////////
// VariableSetView view

class VariableSetView : public CScrollView
{
protected:
	VariableSetView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(VariableSetView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VariableSetView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

public:

	static int ms_textHeight;
	static int ms_variableInfoHeight;

// Implementation
protected:
	virtual ~VariableSetView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:

	class VariableInfo;

	typedef stdvector<VariableInfo*>::fwd  VariableInfoVector;
	typedef stdset<std::string>::fwd       StringSet;

private:

	static void variableIteratorCallback(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

private:

	void addVariable(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable); 

	// disabled
	VariableSetView(const VariableSetView&);
	VariableSetView &operator =(const VariableSetView&);

private:

	CViewerDoc             *getViewerDocument();
	SkeletalAppearance2    *getDocumentAppearance();

	int                     buildVariableSetControlInfo(CustomizationData &customizationData);


	// Generated message map functions
	//{{AFX_MSG(VariableSetView)
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	VariableInfoVector *m_variableInfo;
	int                 m_viewWidth;
	int                 m_viewHeight;

	HBRUSH              m_eraseBrush;

	CustomizationData  *m_customizationData;
	int                 m_variableInfoTop;
	int                 m_controlId;

	StringSet          *m_variableNameSet;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VARIABLESETVIEW_H__926B596F_BE0B_499C_BA61_2F1AB1469707__INCLUDED_)
