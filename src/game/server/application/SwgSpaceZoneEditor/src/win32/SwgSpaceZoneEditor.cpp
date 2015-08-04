// ======================================================================
//
// SwgSpaceZoneEditor.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "SwgSpaceZoneEditor.h"

#include "ChildFrame.h"
#include "Configuration.h"
#include "MainFrame.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "RecentDirectory.h"
#include "resource.h"
#include "SwgSpaceZoneEditorDoc.h"
#include "SpaceZoneTreeView.h"

// ======================================================================

BEGIN_MESSAGE_MAP(SwgSpaceZoneEditorApp, CWinApp)
	//{{AFX_MSG_MAP(SwgSpaceZoneEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgSpaceZoneEditorApp::SwgSpaceZoneEditorApp (void)
{
}

SwgSpaceZoneEditorApp::~SwgSpaceZoneEditorApp (void)
{
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorApp theApp;

// ----------------------------------------------------------------------

BOOL SwgSpaceZoneEditorApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

#if _MSC_VER < 1300

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

#endif

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register document templates

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_SWGSPACEZONEEDITORTYPE,
		RUNTIME_CLASS(SwgSpaceZoneEditorDoc),
		RUNTIME_CLASS(ChildFrame), // custom MDI child frame
		RUNTIME_CLASS(SpaceZoneTreeView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	MainFrame * pMainFrame = new MainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	//-- install the engine
	SetupSharedFoundation::Data setupSharedFoundationData(SetupSharedFoundation::Data::D_mfc);
	setupSharedFoundationData.useWindowHandle  = false;
	setupSharedFoundationData.configFile = "SwgSpaceZoneEditor.cfg";
	SetupSharedFoundation::install(setupSharedFoundationData);

	SetupSharedCompression::install();

	//-- file
	SetupSharedFile::install(false);

	SetupSharedUtility::Data sharedUtilityData;
	SetupSharedUtility::install(sharedUtilityData);

	TreeFile::addSearchAbsolute(0);

	{
		//-- Make sure we're running out of exe\win32
		char buffer[1024];
		if (GetCurrentDirectory(1024, buffer) != 0)
		{
			CString temp(buffer);
			temp.Remove('/');
			temp.Remove('\\');
			if (temp.Find("exewin32") == -1)
				AfxMessageBox ("SwgSpaceZoneEditor is not running from <branch>\\exe\\win32.  You may be running an older version.");
		}

		//-- Load ini file
		if (!Configuration::install())
			AfxMessageBox("SwgSpaceZoneEditor is not properly configured [SwgSpaceZoneEditor.ini or SwgSpaceZoneEditor.cfg not found].  Are you running the application in the correct directory?  Verify that SwgSpaceZoneEditor.cfg has all of the parameters found in template_SwgSpaceZoneEditor.cfg and that SwgSpaceZoneEditor.cfg is configured appropriately for your machine.");

		//-- Make sure the config file is pointing to the same directories as the exe is run from
		{
			CString const branch(Configuration::extractBranch(buffer));

			if (branch != Configuration::extractBranch(Configuration::getSpaceMobileDataTableFileName()))
				AfxMessageBox("SwgSpaceZoneEditor is running out of the '" + branch + "' branch which does not match the directories specified by SwgSpaceZoneEditor.cfg.  Make sure that SwgSpaceZoneEditor.cfg is configured appropriately for your machine.");
		}

		RecentDirectory::install("Software\\Sony Online Entertainment\\SwgSpaceZoneEditor\\Recent");
	}

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

int SwgSpaceZoneEditorApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	SetupSharedFoundation::remove ();
	SetupSharedThread::remove ();

	return CWinApp::ExitInstance();
}

// ----------------------------------------------------------------------

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ----------------------------------------------------------------------

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

// App command to run the dialog
void SwgSpaceZoneEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ======================================================================
