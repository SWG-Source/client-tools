// UIFontBuilder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UIFontBuilder.h"
#include "UIFontBuilderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FB_KEY(a) _T("Software\\Sony Online Entertainment\\UIFontBuilder\\##a")
//#define KEY_PREFIX = ;

const TCHAR * const CUIFontBuilderApp::ms_dbFileKey        = _T ("Software\\Sony Online Entertainment\\UIFontBuilder\\db_filename");
const TCHAR * const CUIFontBuilderApp::ms_lastFontFaceKey  = _T ("Software\\Sony Online Entertainment\\UIFontBuilder\\lastFontFace");
const TCHAR * const CUIFontBuilderApp::ms_lastFontSizeKey  = _T ("Software\\Sony Online Entertainment\\UIFontBuilder\\lastFontSize");
const TCHAR * const CUIFontBuilderApp::ms_lastStyleFileKey = _T ("Software\\Sony Online Entertainment\\UIFontBuilder\\lastStyleFile");
const TCHAR * const CUIFontBuilderApp::ms_lastImageDirKey  = _T ("Software\\Sony Online Entertainment\\UIFontBuilder\\lastImageDir");

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderApp

BEGIN_MESSAGE_MAP(CUIFontBuilderApp, CWinApp)
	//{{AFX_MSG_MAP(CUIFontBuilderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderApp construction

CUIFontBuilderApp::CUIFontBuilderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUIFontBuilderApp object

CUIFontBuilderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderApp initialization

BOOL CUIFontBuilderApp::InitInstance()
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

	// first make sure the unicode database is available

	long len = _MAX_PATH - 1;
	TCHAR buf[_MAX_PATH];

	static HKEY                    ms_hkey;

	if (RegQueryValue (HKEY_CURRENT_USER, ms_dbFileKey, buf, &len) != ERROR_SUCCESS)
	{
		int mbRetval = MessageBox (0, _T("No Unicode data file specified in registry.\nPlease select your unicode data file.\n"), _T("Unicode file"), MB_OKCANCEL);

		if (mbRetval == IDCANCEL)
			return 0;

		for (;;)
		{

			CFileDialog fileDialog (
				true,
				_T ("txt"),
				_T ("UnicodeData.txt"),
				OFN_READONLY | OFN_PATHMUSTEXIST  |OFN_FILEMUSTEXIST);
			
			int retval = fileDialog.DoModal ();
			
			if (retval == IDOK)
			{
				CString pathName = fileDialog.GetPathName ();

				_snwprintf (buf, len, _T("%s"), (LPCTSTR)pathName);

				RegSetValue (HKEY_CURRENT_USER, ms_dbFileKey, REG_SZ, buf, wcslen (buf));
				break;
			}
			else
			{
				return 0;
			}
		}
	}

	CUIFontBuilderDlg dlg (buf);

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
	return 0;
}
