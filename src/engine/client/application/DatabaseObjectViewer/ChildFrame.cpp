// ======================================================================
//
// ChildFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "ChildFrame.h"

#include "DatabaseObjectViewer.h"
#include "DatabaseObjectViewerDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ChildFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

ChildFrame::ChildFrame()
{
}

// ----------------------------------------------------------------------

ChildFrame::~ChildFrame()
{
}

// ----------------------------------------------------------------------

BOOL ChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

// ----------------------------------------------------------------------

void ChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL ChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}

// ----------------------------------------------------------------------

int ChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	CRect mainRect;
	AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
	mainRect.right  -= 2; // allow for frame
	mainRect.bottom -= 64; // allow for toolbars, etc.
	int const size = mainRect.right < mainRect.bottom ? mainRect.right : mainRect.bottom;
	SetWindowPos (&wndTop, 0, 0, size, size, SWP_SHOWWINDOW);

	return 0;
}

// ======================================================================
