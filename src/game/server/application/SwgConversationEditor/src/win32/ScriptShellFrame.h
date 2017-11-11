// ======================================================================
//
// ScriptShellFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ScriptShellFrame_H
#define INCLUDED_ScriptShellFrame_H

// ======================================================================

#include "SwgConversationEditorDoc.h"

// ======================================================================

class ScriptShellFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ScriptShellFrame)

protected:

	ScriptShellFrame();           

public:

	//{{AFX_VIRTUAL(ScriptShellFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ScriptShellFrame();

	//{{AFX_MSG(ScriptShellFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_windowName;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

