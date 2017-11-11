//
// FloraGroupFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef FLORAGROUPFRAME_H
#define FLORAGROUPFRAME_H

//-------------------------------------------------------------------

class FloraMeshView;
class FloraTreeView;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"

//-------------------------------------------------------------------

class FloraGroupFrame : public CMDIChildWnd
{
private:

	DECLARE_DYNCREATE(FloraGroupFrame)

protected:

	CString        m_windowName;
	CToolBar       m_wndToolBar;
	CSplitterWnd   splitter;
	
	FloraMeshView* meshView;
	FloraTreeView* treeView;

protected:

	FloraGroupFrame (void);           // protected constructor used by dynamic creation
	virtual ~FloraGroupFrame();

public:

	void setSelectedObject (const char* name);
	void addFamily (const CString& familyName, const ArrayList<CString*>& children);
	void reset (void);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FloraGroupFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	// Generated message map functions
	//{{AFX_MSG(FloraGroupFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
