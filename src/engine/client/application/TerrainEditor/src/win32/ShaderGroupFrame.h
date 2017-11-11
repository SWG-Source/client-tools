//
// ShaderGroupFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ShaderGroupFrame_H
#define INCLUDED_ShaderGroupFrame_H

//-------------------------------------------------------------------

class ShaderTreeView;
class ShaderView;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"

//-------------------------------------------------------------------

class ShaderGroupFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ShaderGroupFrame)

protected:

	CString      m_windowName;
	CToolBar     m_wndToolBar;
	CSplitterWnd splitter;

	ShaderTreeView* treeView;
	ShaderView*     shaderView;

protected:

	ShaderGroupFrame();           // protected constructor used by dynamic creation

public:

	void setSelectedShader (const char* name);
	void addFamily (const CString& familyName, const ArrayList<CString*>& children);
	void reset (void);

public:

	//{{AFX_VIRTUAL(ShaderGroupFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ShaderGroupFrame();

	//{{AFX_MSG(ShaderGroupFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
