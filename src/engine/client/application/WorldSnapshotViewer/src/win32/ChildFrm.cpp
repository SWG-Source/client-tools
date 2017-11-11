//===================================================================
//
// ChildFrm.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstWorldSnapshotViewer.h"
#include "ChildFrm.h"

#include "WorldSnapshotViewer.h"
#include "WorldSnapshotTreeView.h"
#include "WorldSnapshotDataView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

CChildFrame::CChildFrame()
{
}

CChildFrame::~CChildFrame()
{
}

//-------------------------------------------------------------------

BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(WorldSnapshotTreeView), CSize(200, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(WorldSnapshotDataView), CSize(200, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	treeView = dynamic_cast<WorldSnapshotTreeView*> (m_wndSplitter.GetPane (0, 0));
	dataView = dynamic_cast<WorldSnapshotDataView*> (m_wndSplitter.GetPane (0, 1));

	return TRUE;
}

//-------------------------------------------------------------------

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

//-------------------------------------------------------------------
