// UiBuilder.cpp : Defines the class behaviors for the application.
//

#include "FirstUiBuilder.h"
#include <vector>
#include <string>
#include "UiBuilder.h"
#include "UICursor.h"
#include "UICursorInterface.h"
#include "UICursorSet.h"
#include "UIManager.h"
#include "UIIMEManager.h"
#include "UINullIMEManager.h"
#include "SetupUi.h"
#include "SimpleSoundCanvas.h"
#include "UiCanvasGenerator.h"
#include "DDSCanvasGenerator.h"
#include "FileLocator.h"
#include "UnicodeUtils.h"
#include "UIDirect3DTextureCanvas.h"

#include "MainFrm.h"

#include <afxadv.h>
#include "multimon.h" // HEADER for multi-monitor support on WINVER 4 compiles.

extern UIDirect3DPrimaryCanvas  *gPrimaryDisplay;

//----------------------------------------------------------------------
//-- this is a SWG hack to work with our modified STLPORT

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void * __cdecl operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}

//----------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp

BEGIN_MESSAGE_MAP(CUiBuilderApp, CWinApp)
	ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnRecentfile)
	//{{AFX_MSG_MAP(CUiBuilderApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp construction

CUiBuilderApp::CUiBuilderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUiBuilderApp object

CUiBuilderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp initialization
UIStandardCanvasFactory<DDSCanvasGenerator> *theDDSCanvasFactory;

BOOL CUiBuilderApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	SetRegistryKey(_T("Sony Online Entertainment"));

///////////////////////////////////////////////////

	LoadStdProfileSettings(6);

///////////////////////////////////////////////////
///////////////////////////////////////////////////

	//-- install the ui system
	SetupUi::Data setupUiData;
	SetupUi::install(setupUiData);

	theDDSCanvasFactory = new UIStandardCanvasFactory<DDSCanvasGenerator>;
	
	UIManager &theManager = UIManager::gUIManager();
	theManager.setUIIMEManager(new UINullIMEManager);
	UICursorInterfaceDefault* cursorInterface = new UICursorInterfaceDefault;
	theManager.SetCursorInterface (cursorInterface);

	FileLocator::gFileLocator();

	theManager.AddCanvasFactory(theDDSCanvasFactory);

	theManager.SetSoundCanvas(new SimpleSoundCanvas());

///////////////////////////////////////////////////
///////////////////////////////////////////////////

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);



	// The one and only window has been initialized, so show and update it.
	pFrame->DragAcceptFiles(TRUE);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

int CUiBuilderApp::ExitInstance() 
{
	UIManager &theManager = UIManager::gUIManager();

	UISoundCanvas * sc = UIManager::gUIManager ().GetSoundCanvas ();
	UIManager::gUIManager ().SetSoundCanvas (0);
	delete sc;

	UIManager::ExplicitDestroy();

	FileLocator::ExplicitDestroy ();

	delete theDDSCanvasFactory;
	theDDSCanvasFactory=0;

	//-- remove the ui system
	SetupUi::remove ();
	
	return CWinApp::ExitInstance();
}

void CUiBuilderApp::saveDialogPosition(const char *i_section, const CRect &windowRect)
{
	char buffer[256];
	sprintf(buffer, "%i %i %i %i", windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height());
	WriteProfileString(i_section, "Position", buffer);
}

bool CUiBuilderApp::getDialogPosition(CRect &o_windowRect, const char *i_section, bool clipToDesktop)
{
	CString rectString = GetProfileString(i_section, "Position");
	if (rectString.IsEmpty())
	{
		return false;
	}

	int left, top, width, height;
	if (sscanf(rectString, "%i %i %i %i", &left, &top, &width, &height)==4)
	{
		o_windowRect.left = left;
		o_windowRect.top = top;
		o_windowRect.right = left + width;
		o_windowRect.bottom = top + height;

		if (clipToDesktop)
		{
			CRect desktopRect;
			desktopRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
			desktopRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
			desktopRect.right = desktopRect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
			desktopRect.bottom = desktopRect.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

			if (o_windowRect.right<16)
			{
				o_windowRect.right=16;
				o_windowRect.left=o_windowRect.right - width;
			}
			if (o_windowRect.bottom<16)
			{
				o_windowRect.bottom=16;
				o_windowRect.top=o_windowRect.bottom - height;
			}

			if (o_windowRect.left>desktopRect.right-16)
			{
				o_windowRect.left=desktopRect.right-16;
				o_windowRect.right=o_windowRect.left + width;
			}
			if (o_windowRect.top>desktopRect.bottom-16)
			{
				o_windowRect.top=desktopRect.bottom-16;
				o_windowRect.bottom=o_windowRect.top + height;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp message handlers





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
	afx_msg void OnDestroy();
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
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CUiBuilderApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp message handlers


void CAboutDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CUiBuilderApp::OnRecentfile(UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);
	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	CMainFrame *mainFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	if (mainFrame)
	{
		if (!mainFrame->openFile((*m_pRecentFileList)[nIndex]))
		{
			m_pRecentFileList->Remove(nIndex);
		}
	}
}

bool CUiBuilderApp::getRecentFileName(CString &o_fileName, UINT index)
{
	if (index<(UINT)m_pRecentFileList->GetSize())
	{
		o_fileName = (*m_pRecentFileList)[index];
		return true; 
	}

	return false;
}
