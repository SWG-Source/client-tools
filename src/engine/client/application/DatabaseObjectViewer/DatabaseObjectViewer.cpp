// ======================================================================
//
// DatabaseObjectViewer.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "DatabaseObjectViewer.h"

#include "MainFrame.h"
#include "ChildFrame.h"
#include "DatabaseObjectViewerDoc.h"
#include "DatabaseObjectViewerView.h"

#include <algorithm>

// ======================================================================

namespace DatabaseObjectViewerAppNamespace
{
	enum Section
	{
		S_unknown,
		S_planetList,
		S_hostNameList
	};
}

using namespace DatabaseObjectViewerAppNamespace;

// ======================================================================

BEGIN_MESSAGE_MAP(DatabaseObjectViewerApp, CWinApp)
	//{{AFX_MSG_MAP(DatabaseObjectViewerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

DatabaseObjectViewerApp::DatabaseObjectViewerApp() :
	m_planetNameSet (),
	m_hostNameClusterNameMap ()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// ----------------------------------------------------------------------

DatabaseObjectViewerApp theApp;

// ----------------------------------------------------------------------

BOOL DatabaseObjectViewerApp::InitInstance()
{
	//-- populate config options
	loadConfigFile ();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
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
		IDR_DATABATYPE,
		RUNTIME_CLASS(DatabaseObjectViewerDoc),
		RUNTIME_CLASS(ChildFrame), // custom MDI child frame
		RUNTIME_CLASS(DatabaseObjectViewerView));
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

	return TRUE;
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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DatabaseObjectViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ----------------------------------------------------------------------

bool DatabaseObjectViewerApp::isValidPlanetName (std::string const & planetName)
{
	DatabaseObjectViewerApp::PlanetNameSet const & planetNameSet = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getPlanetNameSet ();
	return planetNameSet.find (planetName) != planetNameSet.end ();
}

// ----------------------------------------------------------------------

bool DatabaseObjectViewerApp::isValidHostName (std::string const & hostName)
{
	DatabaseObjectViewerApp::HostNameClusterNameMap const & hostNameClusterNameMap = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getHostNameClusterNameMap ();
	return hostNameClusterNameMap.find (hostName) != hostNameClusterNameMap.end ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerApp::loadConfigFile ()
{
	//-- open the config file
	CStdioFile infile;
	if (!infile.Open ("DatabaseObjectViewer.cfg", CFile::modeRead | CFile::typeText))
	{
		MessageBox (0, "Could not open DatabaseObjectViewer.cfg.  You probably will not have any success opening any files.", 0, MB_OK);
		return;
	}

	Section section = S_unknown;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find ("#");
		if (index == 0)
			continue;

		//find section
		index = line.Find ("[");
		if (index == 0)
		{
			section = S_unknown;

			CString const sectionName = line.Mid (1, line.GetLength () - 2);

			if (sectionName == "PlanetList")
				section = S_planetList;
			else
				if (sectionName == "HostNameList")
					section = S_hostNameList;

			continue;
		}
		else
		{
			//-- find the =
			index = line.Find ("=");
			if (index == -1)
				continue;

			//-- left half goes in key, right half goes in value
			int const length = line.GetLength ();
			CString const left = line.Left (index);
			CString const right = line.Right (length - index - 1);

			switch (section)
			{
			case S_planetList:
				{
					if (left == "planet")
					{
						std::string planetName (right);
						if (m_planetNameSet.find (planetName) == m_planetNameSet.end ())
							m_planetNameSet.insert (planetName);
					}
				}
				break;

			case S_hostNameList:
				{
					std::string hostName (left);
					std::transform (hostName.begin (), hostName.end (), hostName.begin (), tolower);
					std::string clusterName (right);
					HostNameClusterNameMap::iterator iter = m_hostNameClusterNameMap.find (hostName);
					if (iter == m_hostNameClusterNameMap.end ())
						m_hostNameClusterNameMap.insert (std::make_pair (hostName, clusterName));
					else
						iter->second = clusterName;
				}
				break;

			default:
			case S_unknown:
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

DatabaseObjectViewerDoc* DatabaseObjectViewerApp::getDocument (void)
{
	//-- only display the new view if there is a document
	if (m_pMainWnd)
	{
		CMDIFrameWnd * const mdiFrameWnd = static_cast<CMDIFrameWnd *> (static_cast<MainFrame *> (m_pMainWnd)->GetActiveFrame ());

		if (mdiFrameWnd && mdiFrameWnd->GetActiveDocument ())
			return static_cast<DatabaseObjectViewerDoc *> (mdiFrameWnd->GetActiveDocument ());
	}

	return 0;
}

// ----------------------------------------------------------------------

DatabaseObjectViewerApp::PlanetNameSet const & DatabaseObjectViewerApp::getPlanetNameSet () const
{
	return m_planetNameSet;
}

// ----------------------------------------------------------------------

DatabaseObjectViewerApp::HostNameClusterNameMap const & DatabaseObjectViewerApp::getHostNameClusterNameMap () const
{
	return m_hostNameClusterNameMap;
}

// ======================================================================

