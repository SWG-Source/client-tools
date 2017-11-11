// ======================================================================
//
// MainFrame.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "MainFrame.h"

#include "resource.h"
#include "TextureBuilderDoc.h"

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ======================================================================
// class MainFrame
// ======================================================================

MainFrame::MainFrame()
:	CMDIFrameWnd(),
	m_wndStatusBar(),
	m_wndToolBar()
{
}

// ----------------------------------------------------------------------

MainFrame::~MainFrame()
{
}

// ----------------------------------------------------------------------

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

// ----------------------------------------------------------------------

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif

// ----------------------------------------------------------------------

void MainFrame::getClientRect(RECT &rect) const
{
	//-- get the client rect for the frame
	GetClientRect(&rect);

	//-- adjust for the status bar height
	if (m_wndStatusBar.IsVisible())
	{
		RECT sbRect;
		m_wndStatusBar.GetWindowRect(&sbRect);
		rect.bottom -= (sbRect.bottom - sbRect.top);
	}

	//-- adjust for window toolbar height
	// -TRF- this assumes it is docked top or bottom, really need to check this
	if (m_wndToolBar.IsVisible())
	{
		RECT tbRect;
		m_wndToolBar.GetWindowRect(&tbRect);
		rect.bottom -= (tbRect.bottom - tbRect.top);
	}

	//-- hack adjust, figure out where this is coming from
	rect.right -= 4;
	rect.bottom -= 2;
}

// ----------------------------------------------------------------------

void MainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	//-- get active MDI frame
	CMDIChildWnd *activeChild = MDIGetActive();
	if (!activeChild)
		return;

	//-- tell the active document to resize itself to take up the whole main frame
	CDocument *genericDoc = activeChild->GetActiveDocument();
	if (genericDoc)
	{
		TextureBuilderDoc *const doc = dynamic_cast<TextureBuilderDoc*>(genericDoc);
		if (doc)
		{
			doc->layoutViews();
		}
	}
}

// ======================================================================
