// ======================================================================
//
// TextureBuilder.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureBuilder.h"

#include "ChildFrame.h"
#include "ConstructionTreeView.h"
#include "DialogAbout.h"
#include "DrawTextureCommandElement.h"
#include "ImageSlotElement.h"
#include "MainFrame.h"
#include "TextureBakeView.h"
#include "TextureBuilderDoc.h"
#include "VariableEditView.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/SetupClientObject.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

#include <string>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

TextureBuilderApp theApp;

// ======================================================================

namespace
{
	const char *const ms_registryPathnameSectionName = "Pathname";
	const char *const ms_registryTextureEntryName    = "Texture";
}

// ======================================================================
// class TextureBuilderApp
// ======================================================================

TextureBuilderApp::TextureBuilderApp()
:	CWinApp(),
	m_lastTexturePathname(new std::string()),
	m_textureBakeViewDocTemplate(0),
	m_constructionTreeViewDocTemplate(0),
	m_variableEditViewDocTemplate(0),
	m_mainFrame(0)
{
}

// ----------------------------------------------------------------------

TextureBuilderApp::~TextureBuilderApp()
{
	//-- deleted by MFC
	m_mainFrame                       = 0;
	m_variableEditViewDocTemplate     = 0;
	m_constructionTreeViewDocTemplate = 0;
	m_textureBakeViewDocTemplate      = 0;

	delete m_lastTexturePathname;
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(TextureBuilderApp, CWinApp)
	//{{AFX_MSG_MAP(TextureBuilderApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void TextureBuilderApp::saveSettings()
{
	IGNORE_RETURN(WriteProfileString(ms_registryPathnameSectionName, ms_registryTextureEntryName, m_lastTexturePathname->c_str()));
}

// ----------------------------------------------------------------------

void TextureBuilderApp::loadSettings()
{
	char defaultPathname[MAX_PATH];
	IGNORE_RETURN(GetCurrentDirectory(sizeof(defaultPathname), defaultPathname));

	*m_lastTexturePathname = GetProfileString(ms_registryPathnameSectionName, ms_registryTextureEntryName, defaultPathname);
}

// ----------------------------------------------------------------------

BOOL TextureBuilderApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER < 1300

#ifdef _AFXDLL
	IGNORE_RETURN(Enable3dControls());  // Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();           // Call this when linking to MFC statically
#endif

#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Verant Interactive\\Texture Builder"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	//-- register doc template for construction tree view
	m_constructionTreeViewDocTemplate = new CMultiDocTemplate(
		IDR_TEXTURTYPE,
		RUNTIME_CLASS(TextureBuilderDoc),
		RUNTIME_CLASS(ChildFrame),
		RUNTIME_CLASS(ConstructionTreeView));
	AddDocTemplate(m_constructionTreeViewDocTemplate);

	//-- register doc template for texture bake view
	m_textureBakeViewDocTemplate = new CMultiDocTemplate(
		IDR_TEXTURTYPE,
		RUNTIME_CLASS(TextureBuilderDoc),
		RUNTIME_CLASS(ChildFrame),
		RUNTIME_CLASS(TextureBakeView));
	AddDocTemplate(m_textureBakeViewDocTemplate);

	//-- register doc template for variable edit view
	m_variableEditViewDocTemplate = new CMultiDocTemplate(
		IDR_TEXTURTYPE,
		RUNTIME_CLASS(TextureBuilderDoc),
		RUNTIME_CLASS(ChildFrame),
		RUNTIME_CLASS(VariableEditView));
	AddDocTemplate(m_variableEditViewDocTemplate);

	// create main MDI Frame window
	m_mainFrame = new MainFrame;
	if (!m_mainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = m_mainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	//-- install thread
	SetupSharedThread::install ();

	//-- install debug
	SetupSharedDebug::install (4096);

	//-- install founation
	SetupSharedFoundation::Data setupFoundationData (SetupSharedFoundation::Data::D_mfc);
	setupFoundationData.useWindowHandle = true;
	setupFoundationData.windowHandle    = m_mainFrame->m_hWnd;
	setupFoundationData.configFile      = "tools.cfg";
	SetupSharedFoundation::install (setupFoundationData);

	SetupSharedCompression::install();

	//-- install file
	SetupSharedFile::install (false);

	//-- math
	SetupSharedMath::install ();

	//-- shared object system
	SetupSharedObject::Data setupObjectData;
	SetupSharedObject::setupDefaultMFCData (setupObjectData);
	SetupSharedObject::install (setupObjectData);

	//-- shared utility
	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	SetupSharedUtility::install (setupUtilityData);

	//-- install graphics
	SetupClientGraphics::Data setupClientGraphicsData;
	SetupClientGraphics::setupDefaultMFCData(setupClientGraphicsData);
	SetupClientGraphics::install(setupClientGraphicsData);

	//-- install client object system
//	SetupClientObject::install ();

	//ShaderTemplateList::preloadVertexColorShaderTemplates();

	//-- install texture renderer
	SetupClientTextureRenderer::install ();

	//-- install other systems
	DrawTextureCommandElement::install();
	ImageSlotElement::install();

#if 0
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
#endif

	// The main window has been initialized, so show and update it.
	IGNORE_RETURN(m_mainFrame->ShowWindow(m_nCmdShow));
	m_mainFrame->UpdateWindow();

	return TRUE;
}

// ----------------------------------------------------------------------

int TextureBuilderApp::ExitInstance() 
{
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return CWinApp::ExitInstance();
}

// ----------------------------------------------------------------------

// App command to run the dialog
void TextureBuilderApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	IGNORE_RETURN(aboutDlg.DoModal());
}

// ----------------------------------------------------------------------

void TextureBuilderApp::setLastTexturePathname(const char *pathname)
{
	*m_lastTexturePathname = pathname;
}

// ----------------------------------------------------------------------

void TextureBuilderApp::OnFileNew() 
{
	//-- we override this because we know exactly which document template
	//   to open.
	NOT_NULL(m_constructionTreeViewDocTemplate);
	IGNORE_RETURN(m_constructionTreeViewDocTemplate->OpenDocumentFile(NULL));
}

// ======================================================================
