//
// RadialGroupFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_RadialGroupFrame_H
#define INCLUDED_RadialGroupFrame_H

class RadialTreeView;
class RadialView;

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"

//-------------------------------------------------------------------

class RadialGroupFrame : public CMDIChildWnd
{
private:

	DECLARE_DYNCREATE(RadialGroupFrame)

protected:

	CString      m_windowName;
	CToolBar     m_wndToolBar;
	CSplitterWnd splitter;

	RadialTreeView* treeView;
	RadialView*     radialView;

protected:

	RadialGroupFrame();           

public:

	void setSelectedShader (const char* name);
	void addFamily (const CString& familyName, const ArrayList<CString*>& children);
	void reset (void);

public:

	//{{AFX_VIRTUAL(RadialGroupFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~RadialGroupFrame();

	//{{AFX_MSG(RadialGroupFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
