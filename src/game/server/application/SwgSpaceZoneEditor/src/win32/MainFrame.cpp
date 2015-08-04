// ======================================================================
//
// MainFrame.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "MainFrame.h"

#include "resource.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_2D_MAP_POSITION, OnUpdate2dMapPosition)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_2D_MAP_POSITION,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ----------------------------------------------------------------------

MainFrame::MainFrame()
{
}

MainFrame::~MainFrame()
{
}

// ----------------------------------------------------------------------

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

/*
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME,
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
*/

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar))
//		|| !m_wndReBar.AddBar(&m_wndDlgBar))
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

	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_2D_MAP_POSITION, SBPS_NORMAL, 100);

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

// ----------------------------------------------------------------------

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWnd::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

void MainFrame::OnUpdate2dMapPosition(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(true); 

	Vector const mapPosition = GetActiveFrame() ?(GetActiveFrame()->GetActiveDocument() ? safe_cast<SwgSpaceZoneEditorDoc *>(GetActiveFrame()->GetActiveDocument())->getMapPosition() : Vector::zero) : Vector::zero;
	CString info;
	info.Format("<%i, %i, %i>", static_cast<int>(mapPosition.x), static_cast<int>(mapPosition.y), static_cast<int>(mapPosition.z)); 
	pCmdUI->SetText(info);
}

// ======================================================================
