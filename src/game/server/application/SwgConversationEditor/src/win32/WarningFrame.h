// ======================================================================
//
// WarningFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_WarningFrame_H
#define INCLUDED_WarningFrame_H

// ======================================================================

#include "SwgConversationEditorDoc.h"

// ======================================================================

class WarningFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(WarningFrame)

protected:

	WarningFrame();           

public:

	//{{AFX_VIRTUAL(WarningFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~WarningFrame();

	//{{AFX_MSG(WarningFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void clear ();
	void add (int type, CString const & message, ConversationItem const * conversationItem);

private:

	CString m_windowName;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

