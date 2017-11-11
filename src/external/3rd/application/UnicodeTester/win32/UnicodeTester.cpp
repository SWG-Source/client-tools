// UnicodeTester.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UnicodeTester.h"
#include "UnicodeTesterDlg.h"

/*
#include "FirstFoundation.h"
#include "Misc.h"
#include "Fatal.h"
#include "SetupEngine.h"
#include "SetupFoundation.h"
*/
#include "UnicodeBlocks.h"
#include "UnicodeCharDataMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterApp

BEGIN_MESSAGE_MAP(CUnicodeTesterApp, CWinApp)
	//{{AFX_MSG_MAP(CUnicodeTesterApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterApp construction

CUnicodeTesterApp::CUnicodeTesterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CUnicodeTesterApp::~CUnicodeTesterApp()
{
	Unicode::Blocks::Mapping::explicitDestroy ();
	Unicode::CharDataMap::explicitDestroy ();
	/*
	SetupFoundation::remove();
	SetupEngine::remove();
	*/
}
/////////////////////////////////////////////////////////////////////////////
// The one and only CUnicodeTesterApp object

CUnicodeTesterApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterApp initialization

BOOL CUnicodeTesterApp::InitInstance()
{
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
	/*
	//-- install core
	SetupFoundation::Data setupFoundationData;
	SetupFoundation::setupDefaultMFCData (&setupFoundationData);
	SetupFoundation::install (&setupFoundationData);

	//-- install the object system
	SetupEngine::Data setupEngineData;
	SetupEngine::setupDefaultMFCData (&setupEngineData);
	setupEngineData.use3dSystem = false;
	SetupEngine::install (&setupEngineData);
*/
	CUnicodeTesterDlg dlg;
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

