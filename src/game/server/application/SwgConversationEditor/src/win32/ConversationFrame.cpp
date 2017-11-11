// ======================================================================
//
// ConversationFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ConversationFrame.h"

#include "ConversationBranchView.h"
#include "ConversationEmptyView.h"
#include "ConversationResponseView.h"
#include "ConversationTreeView.h"
#include "SwgConversationEditor.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ConversationFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ConversationFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ConversationFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

ConversationFrame::ConversationFrame() :
	m_windowName ("Conversation Editor"),
	m_wndToolBar (),
	m_splitter (),
	m_treeView (0),
	m_emptyViewId (0),
	m_emptyView (0),
	m_responseViewId (0),
	m_responseView (0),
	m_branchViewId (0),
	m_branchView (0)
{
	m_splitter.setLocked (true);
}

// ----------------------------------------------------------------------

ConversationFrame::~ConversationFrame()
{
}

// ----------------------------------------------------------------------

BOOL ConversationFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ConversationFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void ConversationFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL ConversationFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext) 
{
	if (!m_splitter.CreateStatic (this, 1, 2))
		return FALSE;

	m_emptyViewId = m_splitter.AddView (0, 0, RUNTIME_CLASS (ConversationEmptyView), pContext);
	m_emptyView = safe_cast<ConversationEmptyView *> (m_splitter.GetView (m_emptyViewId));
	m_branchViewId = m_splitter.AddView (0, 0, RUNTIME_CLASS (ConversationBranchView), pContext);
	m_branchView = safe_cast<ConversationBranchView *> (m_splitter.GetView (m_branchViewId));
	m_responseViewId = m_splitter.AddView (0, 0, RUNTIME_CLASS (ConversationResponseView), pContext);
	m_responseView = safe_cast<ConversationResponseView *> (m_splitter.GetView (m_responseViewId));

	m_splitter.CreateView (0, 1, RUNTIME_CLASS (ConversationTreeView), CSize (0, 0), pContext);
	m_treeView = safe_cast<ConversationTreeView *> (m_splitter.GetPane (0, 1));

	int x = 0;
	x = max (m_emptyView->GetTotalSize ().cx, x);
	x = max (m_branchView->GetTotalSize ().cx, x);
	x = max (m_responseView->GetTotalSize ().cx, x);
	m_splitter.SetColumnInfo (0, x + 20, 0);

	return TRUE;
}

// ----------------------------------------------------------------------

int ConversationFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_CONVERSATIONFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	CRect mainRect;
	AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
	mainRect.right  -= 4; // allow for frame
	mainRect.bottom -= 64; // allow for toolbars, etc.
	
	IGNORE_RETURN (SetWindowPos (&wndTop, 0, 0, mainRect.right, mainRect.bottom / 2, SWP_SHOWWINDOW));

	return 0;
}

// ----------------------------------------------------------------------

void ConversationFrame::setView (SwgConversationEditorDoc::ItemData * const itemData, bool root)
{
	if (root)
	{
		m_splitter.ShowView (m_emptyViewId);
	}
	else
	{
		switch (itemData->m_type)
		{
		default:
		case SwgConversationEditorDoc::ItemData::T_unknown:
			m_splitter.ShowView (m_emptyViewId);
			break;

		case SwgConversationEditorDoc::ItemData::T_branch:
			m_splitter.ShowView (m_branchViewId);
			m_branchView->setItemData (itemData);
			break;

		case SwgConversationEditorDoc::ItemData::T_response:
			m_splitter.ShowView (m_responseViewId);
			m_responseView->setItemData (itemData);
			break;
		}
	}
}

// ----------------------------------------------------------------------

void ConversationFrame::setView (ConversationItem const * const conversationItem)
{
	if (conversationItem)
		m_treeView->selectItem (conversationItem);
}

// ----------------------------------------------------------------------

void ConversationFrame::updateView ()
{
	SwgConversationEditorDoc::ItemData * const itemData = m_treeView->getSelectedItemData ();
	if (itemData)
		setView (itemData, itemData->m_conversationItem == safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->getConversation ());
}

// ----------------------------------------------------------------------

void ConversationFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	//-- tell document we're being destroyed
	if (GetActiveDocument ())
		safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setConversationFrame (0);
}

// ----------------------------------------------------------------------

void ConversationFrame::ActivateFrame(int nCmdShow) 
{
	//-- tell the document about it
	safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setConversationFrame (this);

	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ======================================================================
