// ======================================================================
//
// ConversationFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ConversationFrame_H
#define INCLUDED_ConversationFrame_H

// ======================================================================

#include "AW_CMultiViewSplitter.h"
#include "SwgConversationEditorDoc.h"

class ConversationBranchView;
class ConversationEmptyView;
class ConversationResponseView;
class ConversationTreeView;

// ======================================================================

class ConversationFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ConversationFrame)

public:

	ConversationFrame();

	//{{AFX_VIRTUAL(ConversationFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

public:

	virtual ~ConversationFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ConversationFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	void updateView ();
	void setView (ConversationItem const * const conversationItem);

private:

	void setView (SwgConversationEditorDoc::ItemData * itemData, bool root);

private:

	CString const m_windowName;
	CToolBar m_wndToolBar;
	AW_CMultiViewSplitter m_splitter;

	ConversationTreeView * m_treeView;

	int m_emptyViewId;
	ConversationEmptyView * m_emptyView;

	int m_responseViewId;
	ConversationResponseView * m_responseView;

	int m_branchViewId;
	ConversationBranchView * m_branchView;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

