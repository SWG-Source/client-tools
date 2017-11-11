// ======================================================================
//
// SwgDraftSchematicEditor.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "SwgDraftSchematicEditor.h"

#include "Configuration.h"
#include "ChildFrame.h"
#include "MainFrame.h"
#include "RecentDirectory.h"
#include "Resource.h"
#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

BEGIN_MESSAGE_MAP(SwgDraftSchematicEditorApp, CWinApp)
	//{{AFX_MSG_MAP(SwgDraftSchematicEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_CLOSEALL, OnFileCloseall)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgDraftSchematicEditorApp::SwgDraftSchematicEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// ----------------------------------------------------------------------

SwgDraftSchematicEditorApp theApp;

// ----------------------------------------------------------------------

BOOL SwgDraftSchematicEditorApp::InitInstance()
{
	if (!Configuration::install ())
		AfxMessageBox ("SwgDraftSchematicEditor is not properly configured [or SwgDraftSchematicEditor.cfg not found].  Are you running the application in the correct directory?  Verify that SwgDraftSchematicEditor.cfg has all of the parameters found in template_SwgDraftSchematicEditor.cfg and that SwgDraftSchematicEditor.cfg is configured appropriately for your machine.");

	RecentDirectory::install ("Software\\Sony Online Entertainment\\SwgDraftSchematicEditor\\Recent");

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

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_SWGDRATYPE,
		RUNTIME_CLASS(SwgDraftSchematicEditorDoc),
		RUNTIME_CLASS(ChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CView));
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

	CONSOLE_PRINT ("------ configuration ------\n");
	CONSOLE_PRINT (Configuration::getConfiguration ());

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

void SwgDraftSchematicEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorApp::OnFileCloseall() 
{
	CloseAllDocuments (false);	
}

// ======================================================================

void * __cdecl operator new(unsigned int size, enum MemoryManagerNotALeak)
{
	return new unsigned char [size]; 
}

// ======================================================================
