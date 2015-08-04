// ======================================================================
//
// MainFrame.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "MainFrame.h"

#include "Resource.h"
#include "Configuration.h"

// ======================================================================

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_TOOLS_OPENALL, OnToolsOpenall)
	ON_COMMAND(ID_VIEW_CONSOLE, OnViewConsole)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CONSOLE, OnUpdateViewConsole)
	ON_COMMAND(ID_VIEW_DIRECTORY, OnViewDirectory)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DIRECTORY, OnUpdateViewDirectory)
	ON_COMMAND(ID_TOOLS_SCANALL, OnToolsScanall)
	ON_COMMAND(ID_TOOLS_OPENSAVEALL, OnToolsOpensaveall)
	ON_COMMAND(ID_TOOLS_OPENEDITALL, OnToolsOpenEditAll)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OPENEDITALL, OnUpdateToolsOpenEditAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ----------------------------------------------------------------------

MainFrame::MainFrame()
{
	// TODO: add member initialization code here
	
}

// ----------------------------------------------------------------------

MainFrame::~MainFrame()
{
}

// ----------------------------------------------------------------------

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- toolbar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	//-- status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	//-- directory bar
	CString title (Configuration::getSpaceQuestDirectory ());
	if (!m_directoryDialogBar.Create (this, &m_directoryDialog, title, IDD_DIALOG_DIRECTORY))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

	//-- console bar
	title = "Console";
	if (!m_consoleDialogBar.Create (this, &m_consoleDialog, title, IDD_DIALOG_CONSOLE))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

	//-- docking
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

    m_consoleDialogBar.SetBarStyle (m_consoleDialogBar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_consoleDialogBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	ShowControlBar (&m_consoleDialogBar, true, true);
	DockControlBar (&m_consoleDialogBar, AFX_IDW_DOCKBAR_BOTTOM);

    m_directoryDialogBar.SetBarStyle (m_directoryDialogBar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_directoryDialogBar.EnableDocking (CBRS_ALIGN_ANY);
	ShowControlBar (&m_directoryDialogBar, true, true);
	DockControlBar (&m_directoryDialogBar);
	m_directoryDialogBar.SetFocus ();

	return 0;
}

// ----------------------------------------------------------------------

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	//-- start out maximized
	CRect rect;
	if (SystemParametersInfo (SPI_GETWORKAREA, 0, &rect, 0))
	{
		cs.x  = 0;
		cs.y  = 0;
		cs.cx = rect.Width ();
		cs.cy = rect.Height ();
	}

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

void MainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

void MainFrame::consoleConfiguration(CString const & text)
{
	m_consoleDialog.addConfiguration(text);
}

// ----------------------------------------------------------------------

void MainFrame::consoleOutput(CString const & text)
{
	m_consoleDialog.addOutput(text);
}

// ----------------------------------------------------------------------

void MainFrame::consoleWarning(CString const & text)
{
	m_consoleDialog.addWarning(text);
}

// ----------------------------------------------------------------------

void MainFrame::consoleExecute (CString const & text)
{
	m_consoleDialog.execute (text);
}

// ----------------------------------------------------------------------

void MainFrame::OnToolsOpenall() 
{
	if (MessageBox ("Are you sure?", "Open all", MB_YESNO) == IDYES)
		m_directoryDialog.openAll ();
}

// ----------------------------------------------------------------------

void MainFrame::refreshDirectory ()
{
	m_directoryDialog.refresh ();
}

// ----------------------------------------------------------------------

void MainFrame::OnViewConsole() 
{
	ShowControlBar (&m_consoleDialogBar, !m_consoleDialogBar.IsVisible (), false);
}

// ----------------------------------------------------------------------

void MainFrame::OnUpdateViewConsole(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_consoleDialogBar.IsVisible ());
}

// ----------------------------------------------------------------------

void MainFrame::OnViewDirectory() 
{
	ShowControlBar (&m_directoryDialogBar, !m_directoryDialogBar.IsVisible (), false);
}

// ----------------------------------------------------------------------

void MainFrame::OnUpdateViewDirectory(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_directoryDialogBar.IsVisible ());
}

// ----------------------------------------------------------------------

void MainFrame::OnToolsScanall() 
{
	if (MessageBox ("Are you sure?", "Scan all", MB_YESNO) == IDYES)
		m_directoryDialog.scanAll ();
}

// ----------------------------------------------------------------------

void MainFrame::OnToolsOpensaveall() 
{
	if (MessageBox ("Make sure you open for edit all of the relevant quest directories.  Are you sure?", "Save all", MB_YESNO) == IDYES)
		m_directoryDialog.saveAll ();
}

// ----------------------------------------------------------------------

void MainFrame::OnToolsOpenEditAll() 
{
	/* FIXME: this hangs in the console exec ...
	if (MessageBox ("Are you sure?", "Edit all", MB_YESNO) == IDYES)
		m_directoryDialog.editAll ();
	*/
}

// ----------------------------------------------------------------------

void MainFrame::OnUpdateToolsOpenEditAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);
}

// ======================================================================

