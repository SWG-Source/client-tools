// ======================================================================
//
// ChildFrame.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "ChildFrame.h"

#include "MainFrame.h"

// ======================================================================

namespace ChildFrameNamespace
{
	UINT const cms_indicators[] =
	{
		ID_SEPARATOR
	};
}

using namespace ChildFrameNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ChildFrame)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

ChildFrame::ChildFrame() :
	CMDIChildWnd ()
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
	cs.style |= WS_MAXIMIZE;

	if (!CMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void ChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL ChildFrame::DestroyWindow() 
{
	return CMDIChildWnd::DestroyWindow();
}

// ----------------------------------------------------------------------

int ChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(cms_indicators, sizeof(cms_indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	return 0;
}

// ----------------------------------------------------------------------

void ChildFrame::setToolTip(CString const & toolTip)
{
	m_wndStatusBar.SetWindowText(toolTip);
}

// ----------------------------------------------------------------------

void ChildFrame::ActivateFrame(int nCmdShow) 
{
	if (nCmdShow == -1)
		nCmdShow = SW_MAXIMIZE;
	
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ======================================================================

