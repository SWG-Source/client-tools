// ShaderBuilder.cpp : Defines the class behaviors for the application.
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"

#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "LeftView.h"
#include "MainFrm.h"
#include "ShaderBuilderDoc.h"
#include "ShaderBuilderView.h"
#include "RecentDirectory.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderApp

BEGIN_MESSAGE_MAP(CShaderBuilderApp, CWinApp)
	//{{AFX_MSG_MAP(CShaderBuilderApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderApp construction

CShaderBuilderApp::CShaderBuilderApp()
{
	m_workingDirectory = new char[4096];
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CShaderBuilderApp::~CShaderBuilderApp()
{
	delete[] m_workingDirectory;
	RecentDirectory::remove();

	SetupSharedFoundation::remove();
	PerThreadData::remove();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CShaderBuilderApp object

CShaderBuilderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderApp initialization

BOOL CShaderBuilderApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER < 1300

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CShaderBuilderDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CShaderBuilderView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	SetupSharedThread::install ();
	SetupSharedDebug::install(4096);

	//-- install core
	SetupSharedFoundation::Data SetupSharedFoundationData (SetupSharedFoundation::Data::D_mfc);
	SetupSharedFoundationData.useWindowHandle  = true;
	SetupSharedFoundationData.windowHandle     = m_pMainWnd->m_hWnd;
	SetupSharedFoundationData.configFile       = "tools.cfg";
    SetupSharedFoundationData.writeMiniDumps   = ApplicationVersion::isBootlegBuild();
	SetupSharedFoundation::install(SetupSharedFoundationData);

	SetupClientBugReporting::install();

    if (ApplicationVersion::isBootlegBuild())
    {
	    ToolBugReporting::startCrashReporter();
    }

	// check for any config file entries
	if (ConfigFile::isEmpty())
		FATAL(true, ("tools.cfg not found! Are you running from the exe/win32 directory?"));

	SetupSharedCompression::install();

	SetupSharedFile::install(false);

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath (".", 0);

	RecentDirectory::install("Software\\Sony Online Entertainment\\ShaderBuilder\\Recent");

	GetCurrentDirectory(4096, m_workingDirectory);

	if (__argc > 1)
	{
		CShaderBuilderDoc::leftView->LoadTemplate(__argv[1]);
		if (__argc > 2)
			CShaderBuilderDoc::leftView->SaveTemplate(__argv[2]);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

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

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CShaderBuilderApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderApp message handlers

std::string CShaderBuilderApp::stripTreefileSearchPathFromFile(const std::string &sourceFileName)
{
	std::string result;
	if (!TreeFile::stripTreeFileSearchPathFromFile(sourceFileName, result))
		return sourceFileName;

	return result;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Given an edit box, ensure that its string value is exactly 4 characters long.  Trim or pad spaces if needed
 */
void Pad4(CEdit &edit)
{
	CString s;
	edit.GetWindowText(s);
	int length = s.GetLength();
	//trim if it's too long
	if(length > 4)
	{
		//remove characters above the fourth
		s.Delete(4, s.GetLength()-4);
		length = s.GetLength();
	}
	//pad if it's too short
	if (length != 0 && length != 4)
	{
		while (length != 4)
		{
			s += ' ';
			++length;
		}
	}
	edit.SetWindowText(s);
}

/////////////////////////////////////////////////////////////////////////////
