// ======================================================================
//
// SwgConversationEditor.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "SwgConversationEditor.h"

#include "Configuration.h"
#include "ConversationFrame.h"
#include "MainFrame.h"
#include "RecentDirectory.h"
#include "ScriptFrame.h"
#include "ScriptShellFrame.h"
#include "ScriptShellView.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "SwgConversationEditorDoc.h"
#include "WarningFrame.h"
#include "WarningView.h"

// ======================================================================

SwgConversationEditorApp theApp;

// ======================================================================

CString const SwgConversationEditorApp::getVersion()
{
	return "SwgConversationEditor 1.37";
}

// ======================================================================

BEGIN_MESSAGE_MAP(SwgConversationEditorApp, CWinApp)
	//{{AFX_MSG_MAP(SwgConversationEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TOOLS_RELOADDICTIONARY, OnToolsReloaddictionary)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

// ======================================================================

SwgConversationEditorApp::SwgConversationEditorApp() :
	m_currentDirectory (),
	m_conversationTemplate (0),
	m_scriptGroupTemplate (0),
	m_warningTemplate (0),
	m_shellTemplate (0),
	m_dictionary ()
{
}

// ----------------------------------------------------------------------

SwgConversationEditorApp::~SwgConversationEditorApp ()
{
	//-- deleted by the document
//	if (m_conversationTemplate)
//	{
//		delete m_conversationTemplate;
//		m_conversationTemplate = 0;
//	}

	if (m_scriptGroupTemplate)
	{
		delete m_scriptGroupTemplate;
		m_scriptGroupTemplate = 0;
	}

	if (m_warningTemplate)
	{
		delete m_warningTemplate;
		m_warningTemplate = 0;
	}

	if (m_shellTemplate)
	{
		delete m_shellTemplate;
		m_shellTemplate = 0;
	}
}

// ----------------------------------------------------------------------

BOOL SwgConversationEditorApp::InitInstance()
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

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_conversationTemplate = new CMultiDocTemplate(
		IDR_SWGCONTYPE,
		RUNTIME_CLASS(SwgConversationEditorDoc),
		RUNTIME_CLASS(ConversationFrame),
		RUNTIME_CLASS(CView));
	AddDocTemplate(m_conversationTemplate);

	m_scriptGroupTemplate = new CMultiDocTemplate(
		IDR_SWGCONTYPE,
		RUNTIME_CLASS(SwgConversationEditorDoc),
		RUNTIME_CLASS(ScriptFrame),
		RUNTIME_CLASS(CView));

	m_warningTemplate = new CMultiDocTemplate(
		IDR_SWGCONTYPE,
		RUNTIME_CLASS(SwgConversationEditorDoc),
		RUNTIME_CLASS(WarningFrame),
		RUNTIME_CLASS(WarningView));

	m_shellTemplate = new CMultiDocTemplate(
		IDR_SWGCONTYPE,
		RUNTIME_CLASS(SwgConversationEditorDoc),
		RUNTIME_CLASS(ScriptShellFrame),
		RUNTIME_CLASS(ScriptShellView));

	// create main MDI Frame window
	MainFrame* pMainFrame = new MainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	//-- thread
	SetupSharedThread::install ();

	//-- debug
	SetupSharedDebug::install (4096);

	//-- install the engine
	SetupSharedFoundation::Data setupFoundationData (SetupSharedFoundation::Data::D_mfc);
	setupFoundationData.useWindowHandle  = false;
	setupFoundationData.configFile = "SwgConversationEditor.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install ();

	//-- file
	SetupSharedFile::install (false);

	TreeFile::addSearchAbsolute (0);

	char buffer [1024];
	if (GetCurrentDirectory (1024, buffer) != 0)
		m_currentDirectory = buffer;

	{
		CString temp(buffer);
		temp.Remove('/');
		temp.Remove('\\');
		if (temp.Find("exewin32") == -1)
			AfxMessageBox ("SwgConversationEditor is not running from <branch>\\exe\\win32.  You may be running an older version.");
	}

	//-- Load ini file
	if (!Configuration::install ())
		AfxMessageBox ("SwgConversationEditor is not properly configured [SwgConversationEditor.ini not found].");

	RecentDirectory::install("Software\\Sony Online Entertainment\\SwgConversationEditor\\Recent");

	if (!ConfigFile::getKeyString ("SwgConversationEditor", "scriptPath", 0) || 
		!ConfigFile::getKeyString ("SwgConversationEditor", "stringPath", 0) ||
		!ConfigFile::getKeyString ("SwgConversationEditor", "mochaCommand", 0))
		AfxMessageBox ("SwgConversationEditor is not properly configured [or SwgConversationEditor.cfg not found].  Are you running the application in the correct directory?  Verify that SwgConversationEditor.cfg has all of the parameters found in template_SwgConversationEditor.cfg and that SwgConversationEditor.cfg is configured appropriately for your machine.");

	loadDictionary ();

	return TRUE;
}

// ----------------------------------------------------------------------

int SwgConversationEditorApp::ExitInstance() 
{
	SetupSharedFoundation::remove ();
	SetupSharedThread::remove ();
	
	return CWinApp::ExitInstance();
}

// ======================================================================

class CAboutDlg : public CDialog
{
public:

	CAboutDlg();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_version;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ----------------------------------------------------------------------

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_version = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_TEXT_VERSION, m_version);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_version = SwgConversationEditorApp::getVersion();

	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ======================================================================

void SwgConversationEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::onOpenDefaultViews (SwgConversationEditorDoc * const document)
{
	onViewScript (document);
//	onViewWarning (document);
//	onViewShell (document);
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::onViewConversation (SwgConversationEditorDoc * const document)
{
	if (document->getConversationFrame ())
	{
		//-- set as active
		document->getConversationFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (m_conversationTemplate);
		CFrameWnd * const conversationFrame = m_conversationTemplate->CreateNewFrame (document, NULL);

		//-- if it couldn't be created, just return
		if (!conversationFrame)
			return;

		//-- update the frame
		m_conversationTemplate->InitialUpdateFrame (conversationFrame, document);
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::onViewScript (SwgConversationEditorDoc * const document)
{
	if (document->getScriptFrame ())
	{
		//-- set as active
		document->getScriptFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (m_scriptGroupTemplate);
		CFrameWnd * const scriptFrame = m_scriptGroupTemplate->CreateNewFrame (document, NULL);

		//-- if it couldn't be created, just return
		if (!scriptFrame)
			return;

		//-- update the frame
		m_scriptGroupTemplate->InitialUpdateFrame (scriptFrame, document);
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::onViewWarning (SwgConversationEditorDoc * const document)
{
	if (document->getWarningFrame ())
	{
		//-- set as active
		document->getWarningFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (m_warningTemplate);
		CFrameWnd * const warningFrame = m_warningTemplate->CreateNewFrame (document, NULL);

		//-- if it couldn't be created, just return
		if (!warningFrame)
			return;

		//-- update the frame
		m_warningTemplate->InitialUpdateFrame (warningFrame, document);
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::onViewShell (SwgConversationEditorDoc * const document)
{
	if (document->getScriptShellFrame ())
	{
		//-- set as active
		document->getScriptShellFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (m_shellTemplate);
		CFrameWnd * const scriptShellFrame = m_shellTemplate->CreateNewFrame (document, NULL);

		//-- if it couldn't be created, just return
		if (!scriptShellFrame)
			return;

		//-- update the frame
		m_shellTemplate->InitialUpdateFrame (scriptShellFrame, document);
	}
}

// ----------------------------------------------------------------------

bool SwgConversationEditorApp::isValidWord (CString const & word) const
{
	//-- If it's in the dictionary, it's valid
	if (m_dictionary.find(word) != m_dictionary.end())
		return true;

	//-- If the word is a singular possessive (ends in 's), remove the ending quote and verify the word again
	if (word.GetLength() > 2 && word[word.GetLength() - 2] == '\'' && word[word.GetLength() - 1] == 's')
	{
		CString const subword = word.Left(word.GetLength() - 2);
		if (m_dictionary.find(subword) != m_dictionary.end())
			return true;
	}

	//-- If it's a quoted word, remove the front and back quotes and verify word again
	if (word.GetLength() >= 2 && word[0] == '\'' && word[word.GetLength() - 1] == '\'')
	{
		CString const subword = word.Mid(1, word.GetLength() - 2);
		if (m_dictionary.find(subword) != m_dictionary.end())
			return true;
	}

	//-- If it ends with a quote, remove the ending quote and verify the word again
	if (word.GetLength() > 0 && word[word.GetLength() - 1] == '\'')
	{
		CString const subword = word.Left(word.GetLength() - 1);
		if (m_dictionary.find(subword) != m_dictionary.end())
			return true;
	}

	//-- If it starts with a quote, remove the starting quote and verify the word again
	if (word.GetLength() > 0 && word[0] == '\'')
	{
		CString const subword = word.Right(word.GetLength() - 1);
		if (m_dictionary.find(subword) != m_dictionary.end())
			return true;
	}

	//-- word is not valid
	return false;
}

// ======================================================================

bool SwgConversationEditorApp::loadDictionary ()
{
	bool result = false;
	bool specified = false;
	char const * fileName = 0;
	int i = 0;
	do
	{
		fileName = ConfigFile::getKeyString ("SwgConversationEditor", "dictionary", i, 0);
		if (fileName)
		{
			specified = true;
			if (loadDictionary (fileName))
				result = true;
			else
			{
				CString buffer;
				buffer.Format ("Dictionary '%s' specified in SwgConversationEditor.cfg but the file was not found.", fileName);
				AfxMessageBox (buffer);
			}

			++i;
		}
	} 
	while (fileName);

	if (!result && !specified)
	{
		AfxMessageBox ("'dictionary' has not been defined in SwgConversationEditor.cfg.  Spell checker will not be available.");
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool SwgConversationEditorApp::loadDictionary (char const * const fileName)
{
	CStdioFile infile;
	if (!infile.Open (fileName, CFile::modeRead | CFile::typeText))
		return false;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- skip comments
		if (line [0] == '#')
			continue;

		//-- force lower case
		line.MakeLower ();

		m_dictionary.insert (line);
	}

	return !m_dictionary.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorApp::OnToolsReloaddictionary() 
{
	if (!m_currentDirectory.IsEmpty ())
		SetCurrentDirectory (m_currentDirectory);

	m_dictionary.clear ();
	if (loadDictionary ())
		AfxMessageBox ("Dictionary reloaded.", MB_ICONINFORMATION);
}

// ======================================================================
