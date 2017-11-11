// ======================================================================
//
// SwgSpaceQuestEditor.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "SwgSpaceQuestEditor.h"

#include "Configuration.h"
#include "ChildFrame.h"
#include "MainFrame.h"
#include "RecentDirectory.h"
#include "Resource.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "SwgSpaceQuestEditorDoc.h"
#include "SwgSpaceQuestEditorView.h"

// ======================================================================

namespace SwgSpaceQuestEditorAppNamespace
{
	CString const extractBranch(CString const & path)
	{
		CString buffer(path);
		buffer.Replace('\\', '/');
		buffer = buffer.Right(buffer.GetLength() - buffer.Find("swg") - 4);
		int const index = buffer.Find('/');
		buffer = buffer.Left(index);
		return buffer;
	}
}

using namespace SwgSpaceQuestEditorAppNamespace;

// ======================================================================

BEGIN_MESSAGE_MAP(SwgSpaceQuestEditorApp, CWinApp)
	//{{AFX_MSG_MAP(SwgSpaceQuestEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_CLOSEALL, OnFileCloseall)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgSpaceQuestEditorApp::SwgSpaceQuestEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// ----------------------------------------------------------------------

SwgSpaceQuestEditorApp theApp;

// ----------------------------------------------------------------------

BOOL SwgSpaceQuestEditorApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options(including MRU)

	{
		//-- thread
		SetupSharedThread::install();

		//-- debug
		SetupSharedDebug::install(4096);

		//-- install the engine
		SetupSharedFoundation::Data setupSharedFoundationData(SetupSharedFoundation::Data::D_mfc);
		setupSharedFoundationData.useWindowHandle  = false;
		setupSharedFoundationData.configFile = "SwgSpaceQuestEditor.cfg";
		SetupSharedFoundation::install(setupSharedFoundationData);

		SetupSharedCompression::install();

		//-- file
		SetupSharedFile::install(false);

		//-- utility
		SetupSharedUtility::Data setupSharedUtilityData;
		SetupSharedUtility::setupToolData(setupSharedUtilityData);
		SetupSharedUtility::install(setupSharedUtilityData);

		TreeFile::addSearchAbsolute(0);

		//-- Make sure we're running out of exe\win32
		char buffer[1024];
		if (GetCurrentDirectory(1024, buffer) != 0)
		{
			CString temp(buffer);
			temp.Remove('/');
			temp.Remove('\\');
			if (temp.Find("exewin32") == -1)
				AfxMessageBox ("SwgSpaceQuestEditor is not running from <branch>\\exe\\win32.  You may be running an older version.");
		}

		//-- Load ini file
		if (!Configuration::install())
			AfxMessageBox("SwgSpaceQuestEditor is not properly configured [SwgSpaceQuestEditor.ini or SwgSpaceQuestEditor.cfg not found].  Are you running the application in the correct directory?  Verify that SwgSpaceQuestEditor.cfg has all of the parameters found in template_SwgSpaceQuestEditor.cfg and that SwgSpaceQuestEditor.cfg is configured appropriately for your machine.");

		//-- Make sure the config file is pointing to the same directories as the exe is run from
		{
			CString const branch(extractBranch(buffer));

			if (branch != extractBranch(Configuration::getServerMissionDataTablePath()) || 
				branch != extractBranch(Configuration::getSharedStringFilePath()) ||
				branch != extractBranch(Configuration::getSharedQuestListDataTablePath()) ||
				branch != extractBranch(Configuration::getSharedQuestTaskDataTablePath()))
				AfxMessageBox("SwgSpaceQuestEditor is running out of the '" + branch + "' branch which does not match the directories specified by SwgSpaceQuestEditor.cfg.  Make sure that SwgSpaceQuestEditor.cfg is configured appropriately for your machine.");
		}

		RecentDirectory::install("Software\\Sony Online Entertainment\\SwgSpaceQuestEditor\\Recent");
	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_SWGDRATYPE,
		RUNTIME_CLASS(SwgSpaceQuestEditorDoc),
		RUNTIME_CLASS(ChildFrame), // custom MDI child frame
		RUNTIME_CLASS(SwgSpaceQuestEditorView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	MainFrame* pMainFrame = new MainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	CONSOLE_CONFIGURATION("------ configuration ------\n");
	CONSOLE_CONFIGURATION(Configuration::getConfiguration());

	return TRUE;
}

// ======================================================================

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ----------------------------------------------------------------------

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorApp::OnFileCloseall() 
{
	CloseAllDocuments(false);	
}

// ----------------------------------------------------------------------

int SwgSpaceQuestEditorApp::ExitInstance() 
{
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
	
	return CWinApp::ExitInstance();
}

// ----------------------------------------------------------------------

CDocument * SwgSpaceQuestEditorApp::GetActiveDocument()
{
	if (m_pMainWnd)
	{
		CMDIFrameWnd * const mdiFrameWnd = static_cast<CMDIFrameWnd *>(static_cast<MainFrame *>(m_pMainWnd)->GetActiveFrame());

		if (mdiFrameWnd && mdiFrameWnd->GetActiveDocument())
			return mdiFrameWnd->GetActiveDocument();
	}

	return 0;
}

// ======================================================================
