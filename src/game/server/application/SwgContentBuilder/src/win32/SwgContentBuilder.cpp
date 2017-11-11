// ======================================================================
//
// SwgContentBuilder.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "SwgContentBuilder.h"

#include "Configuration.h"
#include "RecentDirectory.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "SwgContentBuilderDlg.h"

// ======================================================================

BEGIN_MESSAGE_MAP(SwgContentBuilderApp, CWinApp)
	//{{AFX_MSG_MAP(SwgContentBuilderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgContentBuilderApp::SwgContentBuilderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// ----------------------------------------------------------------------

SwgContentBuilderApp theApp;

// ----------------------------------------------------------------------

BOOL SwgContentBuilderApp::InitInstance()
{
	AfxEnableControlContainer();

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

	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	//-- install the engine
	SetupSharedFoundation::Data setupFoundationData (SetupSharedFoundation::Data::D_mfc);
	setupFoundationData.useWindowHandle = false;
	setupFoundationData.configFile = "SwgContentBuilder.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	//-- file
	SetupSharedFile::install(false);

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchAbsolute(0);

	RecentDirectory::install("Software\\Sony Online Entertainment\\SwgContentBuilder\\Recent");

	if (!Configuration::install ())
		AfxMessageBox ("SwgContentBuilder is not properly configured [SwgContentBuilder.cfg not found].  Are you running the application in the correct directory?");

	SwgContentBuilderDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

// ----------------------------------------------------------------------

int SwgContentBuilderApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	RecentDirectory::remove ();
	SetupSharedFoundation::remove ();
	SetupSharedThread::remove ();
	
	return CWinApp::ExitInstance();
}

//===================================================================
