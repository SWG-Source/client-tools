//===================================================================
//
// MainFrm.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstWorldSnapshotViewer.h"
#include "MainFrm.h"

#include "resource.h"

//===================================================================

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_2D_MAP_POSITION, OnUpdate2dMapPosition)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===================================================================

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_2D_MAP_POSITION,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

//-------------------------------------------------------------------

CMainFrame::CMainFrame() :
	m_mapPosition ()
{
}

CMainFrame::~CMainFrame()
{
}

//-------------------------------------------------------------------

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME,
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndDlgBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo (1, ID_INDICATOR_2D_MAP_POSITION, SBPS_NORMAL, 100);

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

//-------------------------------------------------------------------

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

//===================================================================

void CMainFrame::OnUpdate2dMapPosition(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable (true); 

	CString info;
	info.Format ("<%i, %i>", static_cast<int> (m_mapPosition.x), static_cast<int> (m_mapPosition.y)); 
	pCmdUI->SetText (info);
}

//===================================================================


