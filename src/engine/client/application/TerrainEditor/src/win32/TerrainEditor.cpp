//
// TerrainEditor.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "TerrainEditor.h"

//-------------------------------------------------------------------

#include "BlendGroupFrame.h"
#include "BlendView.h"
#include "BitmapFrame.h"
#include "BitmapPreviewFrame.h"
#include "BitmapPreviewView.h"
#include "BitmapView.h"
#include "BookmarkFrame.h"
#include "BookmarkView.h"
#include "ConsoleFrame.h"
#include "ConsoleView.h"
#include "EditorTerrain.h"
#include "EnvironmentFrame.h"
#include "EnvironmentView.h"
#include "FindFrame.h"
#include "FindView.h"
#include "FloraGroupFrame.h"
#include "FloraMeshView.h"
#include "FractalFrame.h"
#include "FractalPreviewFrame.h"
#include "FractalPreviewView.h"
#include "FractalView.h"
#include "HelpFrame.h"
#include "HelpView.h"
#include "LayerFrame.h"
#include "LayerView.h"
#include "MainFrame.h"
#include "MapFrame.h"
#include "MapView.h"
#include "ProfileFrame.h"
#include "ProfileView.h"
#include "PropertyFrame.h"
#include "RadialGroupFrame.h"
#include "RadialTreeView.h"
#include "RadialView.h"
#include "RecentDirectory.h"
#include "ShaderGroupFrame.h"
#include "ShaderView.h"
#include "Splash.h"
#include "TerrainEditorDoc.h"
#include "TipDialog.h"
#include "View3dFrame.h"
#include "View3dView.h"
#include "WarningFrame.h"
#include "WarningView.h"
#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/SetupClientObject.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

//-------------------------------------------------------------------

void CONSOLE_PRINT (const CString& newMessage)
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		terrainDocument->addConsoleMessage (newMessage);
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(TerrainEditorApp, CWinApp)
	ON_COMMAND(CG_IDS_TIPOFTHEDAY, ShowTipOfTheDay)
	//{{AFX_MSG_MAP(TerrainEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_EDIT_FLORA, OnEditFlora)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FLORA, OnUpdateEditFlora)
	ON_COMMAND(ID_VIEWBOOKMARKS, OnViewbookmarks)
	ON_UPDATE_COMMAND_UI(ID_VIEWBOOKMARKS, OnUpdateViewbookmarks)
	ON_COMMAND(ID_EDIT_SHADERS, OnEditShaders)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SHADERS, OnUpdateEditShaders)
	ON_COMMAND(ID_VIEWCONSOLE, OnViewconsole)
	ON_UPDATE_COMMAND_UI(ID_VIEWCONSOLE, OnUpdateViewconsole)
	ON_COMMAND(ID_BUTTON_LAYERVIEW, OnButtonLayerview)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LAYERVIEW, OnUpdateButtonLayerview)
	ON_COMMAND(ID_VIEWPROPERTIES, OnViewproperties)
	ON_UPDATE_COMMAND_UI(ID_VIEWPROPERTIES, OnUpdateViewproperties)
	ON_COMMAND(ID_VIEWMAP, OnViewmap)
	ON_UPDATE_COMMAND_UI(ID_VIEWMAP, OnUpdateViewmap)
	ON_COMMAND(ID_3D_VIEW, On3dView)
	ON_UPDATE_COMMAND_UI(ID_3D_VIEW, OnUpdate3dView)
	ON_COMMAND(ID_EDIT_BLENDS, OnEditBlends)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BLENDS, OnUpdateEditBlends)
	ON_COMMAND(ID_FRACTAL_PREVIEW, OnFractalPreview)
	ON_UPDATE_COMMAND_UI(ID_FRACTAL_PREVIEW, OnUpdateFractalPreview)
	ON_COMMAND(ID_BITMAP_PREVIEW, OnBitmapPreview)
	ON_UPDATE_COMMAND_UI(ID_BITMAP_PREVIEW, OnUpdateBitmapPreview)
	ON_COMMAND(ID_EDIT_RADIAL, OnEditRadial)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RADIAL, OnUpdateEditRadial)
	ON_COMMAND(ID_VIEW_HELP, OnViewHelp)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HELP, OnUpdateViewHelp)
	ON_COMMAND(ID_VIEWPROFILE, OnViewProfile)
	ON_UPDATE_COMMAND_UI(ID_VIEWPROFILE, OnUpdateViewProfile)
	ON_COMMAND(ID_VIEWWARNING, OnViewWarning)
	ON_UPDATE_COMMAND_UI(ID_VIEWWARNING, OnUpdateViewWarning)
	ON_COMMAND(ID_VIEWFIND, OnViewFind)
	ON_UPDATE_COMMAND_UI(ID_VIEWFIND, OnUpdateViewFind)
	ON_COMMAND(ID_VIEWENVIRONMENT, OnViewEnvironment)
	ON_UPDATE_COMMAND_UI(ID_VIEWENVIRONMENT, OnUpdateViewEnvironment)
	ON_COMMAND(ID_EDIT_FRACTAL, OnEditFractal)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FRACTAL, OnUpdateEditFractal)
	ON_COMMAND(ID_EDIT_BITMAP, OnEditBitmap)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BITMAP, OnUpdateEditBitmap)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

static TerrainEditorApp theApp;

//-------------------------------------------------------------------

TerrainEditorApp::TerrainEditorApp (void) :
	CWinApp (),
	mapViewTemplate (0),
	floraGroupTemplate (0),
	radialGroupTemplate (0),
	bookmarkTemplate (0),
	shaderGroupTemplate (0),
	consoleTemplate (0),
	layerTemplate (0),
	propertyTemplate (0),
	blendGroupTemplate (0),
	fractalPreviewTemplate (0),
	bitmapPreviewTemplate (0),
	view3dTemplate (0),
	helpTemplate (0),
	profileTemplate (0),
	warningTemplate (0),
	findTemplate (0),
	environmentTemplate (0),
	fractalGroupTemplate (0),
	bitmapGroupTemplate (0)
{
}

//-------------------------------------------------------------------

TerrainEditorApp::~TerrainEditorApp (void)
{
	if (floraGroupTemplate)
	{
		delete floraGroupTemplate;
		floraGroupTemplate = 0;
	}

	if (radialGroupTemplate)
	{
		delete radialGroupTemplate;
		radialGroupTemplate = 0;
	}

	if (bookmarkTemplate)
	{
		delete bookmarkTemplate;
		bookmarkTemplate = 0;
	}

	if (shaderGroupTemplate)
	{
		delete shaderGroupTemplate;
		shaderGroupTemplate = 0;
	}

	if (consoleTemplate)
	{
		delete consoleTemplate;
		consoleTemplate = 0;
	}

	if (layerTemplate)
	{
		delete layerTemplate;
		layerTemplate = 0;
	}

	if (propertyTemplate)
	{
		delete propertyTemplate;
		propertyTemplate = 0;
	}

	if (blendGroupTemplate)
	{
		delete blendGroupTemplate;
		blendGroupTemplate = 0;
	}

	if (fractalGroupTemplate)
	{
		delete fractalGroupTemplate;
		fractalGroupTemplate = 0;
	}

	if (fractalPreviewTemplate)
	{
		delete fractalPreviewTemplate;
		fractalPreviewTemplate = 0;
	}

	if (bitmapGroupTemplate)
	{
		delete bitmapGroupTemplate;
		bitmapGroupTemplate = 0;
	}

	if (bitmapPreviewTemplate)
	{
		delete bitmapPreviewTemplate;
		bitmapPreviewTemplate = 0;
	}

	if (view3dTemplate)
	{
		delete view3dTemplate;
		view3dTemplate = 0;
	}

	if (helpTemplate)
	{
		delete helpTemplate;
		helpTemplate = 0;
	}

	if (profileTemplate)
	{
		delete profileTemplate;
		profileTemplate = 0;
	}

	if (warningTemplate)
	{
		delete warningTemplate;
		warningTemplate = 0;
	}

	if (findTemplate)
	{
		delete findTemplate;
		findTemplate = 0;
	}

	if (environmentTemplate)
	{
		delete environmentTemplate;
		environmentTemplate = 0;
	}

	//-- this is deleted by the document
//	if (mapViewTemplate)
//	{
//		delete mapViewTemplate;
//		mapViewTemplate = 0;
//	}

	mapViewTemplate = 0;
}

//-------------------------------------------------------------------

BOOL TerrainEditorApp::InitInstance()
{
	// CG: The following block was added by the Splash Screen component.
	{
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);
		SplashScreen::EnableSplashScreen(cmdInfo.m_bShowSplash);
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER < 1300

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	IGNORE_RETURN (Enable3dControlsStatic());	// Call this when linking to MFC statically
#endif

#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Software\\Sony Online Entertainment\\TerrainEditor"));
	RecentDirectory::install("Software\\Sony Online Entertainment\\TerrainEditor\\Recent");

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	mapViewTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(MapFrame), // custom MDI child frame
		RUNTIME_CLASS(MapView));
	AddDocTemplate(mapViewTemplate);

	floraGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(FloraGroupFrame), // custom MDI child frame
		RUNTIME_CLASS(FloraMeshView));

	radialGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(RadialGroupFrame), // custom MDI child frame
		RUNTIME_CLASS(RadialView));

	bookmarkTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(BookmarkFrame), // custom MDI child frame
		RUNTIME_CLASS(BookmarkView));

	shaderGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(ShaderGroupFrame), // custom MDI child frame
		RUNTIME_CLASS(ShaderView));

	consoleTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(ConsoleFrame), // custom MDI child frame
		RUNTIME_CLASS(ConsoleView));

	layerTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(LayerFrame), // custom MDI child frame
		RUNTIME_CLASS(LayerView));

	propertyTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(PropertyFrame), // custom MDI child frame
		RUNTIME_CLASS(PropertyView));

	blendGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(BlendGroupFrame), // custom MDI child frame
		RUNTIME_CLASS(BlendView));

	fractalGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(FractalFrame), // custom MDI child frame
		RUNTIME_CLASS(FractalView));

	fractalPreviewTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(FractalPreviewFrame), // custom MDI child frame
		RUNTIME_CLASS(FractalPreviewView));

	bitmapGroupTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(BitmapFrame), // custom MDI child frame
		RUNTIME_CLASS(BitmapView));

	bitmapPreviewTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(BitmapPreviewFrame), // custom MDI child frame
		RUNTIME_CLASS(BitmapPreviewView));

	view3dTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(View3dFrame), // custom MDI child frame
		RUNTIME_CLASS(View3dView));

	helpTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(HelpFrame), // custom MDI child frame
		RUNTIME_CLASS(HelpView));

	profileTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(ProfileFrame), // custom MDI child frame
		RUNTIME_CLASS(ProfileView));

	warningTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(WarningFrame), // custom MDI child frame
		RUNTIME_CLASS(WarningView));

	findTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(FindFrame), // custom MDI child frame
		RUNTIME_CLASS(FindView));

	environmentTemplate = new CMultiDocTemplate(
		IDR_TERRAITYPE,
		RUNTIME_CLASS(TerrainEditorDoc),
		RUNTIME_CLASS(EnvironmentFrame), // custom MDI child frame
		RUNTIME_CLASS(EnvironmentView));

	// create main MDI Frame window
	MainFrame* pMainFrame = new MainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	//Dispatch commands specified on the command line

	//-- don't open a file by default!
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	// The main window has been initialized, so show and update it.
	IGNORE_RETURN (pMainFrame->ShowWindow(m_nCmdShow));
	pMainFrame->UpdateWindow();

	//
	//-- install the engine
	//

	//-- setup shared
	{
		//-- thread
		SetupSharedThread::install();

		//-- debug
		SetupSharedDebug::install(4096);

		//-- foundation
		SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_mfc);
		setupFoundationData.useWindowHandle           = true;
		setupFoundationData.windowHandle              = pMainFrame->m_hWnd;
		setupFoundationData.configFile                = "tools.cfg";
        setupFoundationData.writeMiniDumps            = ApplicationVersion::isBootlegBuild();
	    SetupSharedFoundation::install(setupFoundationData);

	    SetupClientBugReporting::install();

        if (ApplicationVersion::isBootlegBuild())
        {
	        ToolBugReporting::startCrashReporter();
        }

		SetupSharedCompression::install();

		//-- file
		SetupSharedFile::install(false);

		//-- math
		SetupSharedMath::install();

		//-- utility
		SetupSharedUtility::Data setupUtilityData;
		SetupSharedUtility::setupGameData (setupUtilityData);
		SetupSharedUtility::install (setupUtilityData);

		//-- random
		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

		//-- image
		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData (setupImageData);
		SetupSharedImage::install (setupImageData);

		//-- object
		SetupSharedObject::Data setupObjectData;
		SetupSharedObject::setupDefaultMFCData (setupObjectData);
		SetupSharedObject::install (setupObjectData);

		//-- terrain
		SetupSharedTerrain::Data setupSharedTerrainData;
		SetupSharedTerrain::setupToolData (setupSharedTerrainData);
		SetupSharedTerrain::install (setupSharedTerrainData);
	}

	//-- setup client
	{
		//-- audio
		SetupClientAudio::install ();

		//-- graphics
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultMFCData (setupGraphicsData);
		SetupClientGraphics::install (setupGraphicsData);

		//-- animation 
		SetupClientAnimation::install ();

		//-- skeletal animation
		SetupClientSkeletalAnimation::Data setupSkeletalAnimationData;
		SetupClientSkeletalAnimation::setupToolData (setupSkeletalAnimationData);
		SetupClientSkeletalAnimation::install (setupSkeletalAnimationData);

		//-- texture renderer
		SetupClientTextureRenderer::install ();

		//-- particle system
		SetupClientParticle::install ();

		//-- object
		SetupClientObject::Data setupClientObjectData;
		SetupClientObject::setupToolData (setupClientObjectData);
		SetupClientObject::install (setupClientObjectData);
	}

	// -qq- don't like these being hard-coded
	TreeFile::addSearchAbsolute (0);

	// CG: This line inserted by 'Tip of the Day' component.
	ShowTipAtStartup();

	return TRUE;
}

//-------------------------------------------------------------------

class CAboutDlg : public CDialog
{
public:

	CAboutDlg(void);

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void TerrainEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	IGNORE_RETURN (aboutDlg.DoModal());
}

//-------------------------------------------------------------------

TerrainEditorDoc* TerrainEditorApp::getDocument (void)
{
	//-- only display the new view if there is a document
	if (m_pMainWnd)
	{
		CMDIFrameWnd* mdiFrameWnd = static_cast<CMDIFrameWnd*> (static_cast<MainFrame*> (m_pMainWnd)->GetActiveFrame ());

		if (mdiFrameWnd && mdiFrameWnd->GetActiveDocument ())
			return static_cast<TerrainEditorDoc*> (mdiFrameWnd->GetActiveDocument ());
	}

	return 0;
}

//-------------------------------------------------------------------

void TerrainEditorApp::showFractalPreview (void)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewFractalPreview (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::showBitmapPreview (void)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
	{
		onViewBitmapPreview (terrainDocument);
	}
}

//-------------------------------------------------------------------
	
	
void TerrainEditorApp::showLayers (void)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewLayers (terrainDocument);
}

//-------------------------------------------------------------------
	
void TerrainEditorApp::showConsole (void)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewConsole (terrainDocument);
}

//-------------------------------------------------------------------
	
void TerrainEditorApp::showProfile (void)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewProfile (terrainDocument);
}

//-------------------------------------------------------------------
	
void TerrainEditorApp::showWarning (bool clear)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewWarning (terrainDocument, clear);
}

//-------------------------------------------------------------------
	
void TerrainEditorApp::showFind (bool clear)
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewFind (terrainDocument, clear);
}

//-------------------------------------------------------------------
	
void TerrainEditorApp::onOpenDefaultViews (TerrainEditorDoc* terrainDocument)
{
	onViewLayers      (terrainDocument);
	onViewFlora       (terrainDocument);
	onViewRadial      (terrainDocument);
	onViewShaders     (terrainDocument);
	onViewProperties  (terrainDocument);
	onViewFractals    (terrainDocument);
	onViewBitmaps     (terrainDocument);
	onViewEnvironment (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewFlora (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getFloraGroupFrame ())
	{
		//-- set as active
		terrainDocument->getFloraGroupFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (floraGroupTemplate);
		CFrameWnd* floraGroupFrame = floraGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!floraGroupFrame)
			return;

		//-- update the frame
		floraGroupTemplate->InitialUpdateFrame (floraGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setFloraGroupFrame (NON_NULL (dynamic_cast<FloraGroupFrame*> (floraGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditFlora() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewFlora (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditFlora(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getFloraGroupFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewRadial (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getRadialGroupFrame ())
	{
		//-- set as active
		terrainDocument->getRadialGroupFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (radialGroupTemplate);
		CFrameWnd* radialGroupFrame = radialGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!radialGroupFrame)
			return;

		//-- update the frame
		radialGroupTemplate->InitialUpdateFrame (radialGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setRadialGroupFrame (NON_NULL (dynamic_cast<RadialGroupFrame*> (radialGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditRadial () 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewRadial (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditRadial (CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getRadialGroupFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewBookmarks  (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getBookmarkFrame ())
	{
		//-- set as active
		terrainDocument->getBookmarkFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (bookmarkTemplate);
		CFrameWnd* bookmarkFrame = bookmarkTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!bookmarkFrame)
			return;

		//-- update the frame
		bookmarkTemplate->InitialUpdateFrame (bookmarkFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setBookmarkFrame (NON_NULL (dynamic_cast<BookmarkFrame*> (bookmarkFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewbookmarks() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewBookmarks (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewbookmarks(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getBookmarkFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewHelp  (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getHelpFrame ())
	{
		//-- set as active
		terrainDocument->getHelpFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (helpTemplate);
		CFrameWnd* helpFrame = helpTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!helpFrame)
			return;

		//-- update the frame
		helpTemplate->InitialUpdateFrame (helpFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setHelpFrame (NON_NULL (dynamic_cast<HelpFrame*> (helpFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewHelp() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewHelp (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewHelp(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getHelpFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewShaders (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getShaderGroupFrame ())
	{
		//-- set as active
		terrainDocument->getShaderGroupFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (shaderGroupTemplate);
		CFrameWnd* shaderGroupFrame = shaderGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!shaderGroupFrame)
			return;

		//-- update the frame
		shaderGroupTemplate->InitialUpdateFrame (shaderGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setShaderGroupFrame (NON_NULL (dynamic_cast<ShaderGroupFrame*> (shaderGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditShaders() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewShaders (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditShaders(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getShaderGroupFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewConsole (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getConsoleFrame ())
	{
		//-- set as active
		terrainDocument->getConsoleFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (consoleTemplate);
		CFrameWnd* consoleFrame = consoleTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!consoleFrame)
			return;

		//-- update the frame
		consoleTemplate->InitialUpdateFrame (consoleFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setConsoleFrame (NON_NULL (dynamic_cast<ConsoleFrame*> (consoleFrame)));
		terrainDocument->addConsoleMessage ("");
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewconsole() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewConsole (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewconsole(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getConsoleFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewLayers (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getLayerFrame ())
	{
		//-- set as active
		terrainDocument->getLayerFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (layerTemplate);
		CFrameWnd* layerFrame = layerTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!layerFrame)
			return;

		//-- update the frame
		layerTemplate->InitialUpdateFrame (layerFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setLayerFrame (NON_NULL (dynamic_cast<LayerFrame*> (layerFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnButtonLayerview() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewLayers (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateButtonLayerview(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getLayerFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewProperties (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getPropertyFrame ())
	{
		//-- set as active
		terrainDocument->getPropertyFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (propertyTemplate);
		CFrameWnd* propertyFrame = propertyTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!propertyFrame)
			return;

		//-- update the frame
		propertyTemplate->InitialUpdateFrame (propertyFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setPropertyFrame (NON_NULL (dynamic_cast<PropertyFrame*>(propertyFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewproperties() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewProperties (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewproperties(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getPropertyFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewMap (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getMapFrame ())
	{
		//-- set as active
		terrainDocument->getMapFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (mapViewTemplate);
		CFrameWnd* mapFrame = mapViewTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!mapFrame)
			return;

		//-- update the frame
		mapViewTemplate->InitialUpdateFrame (mapFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setMapFrame (NON_NULL (dynamic_cast<MapFrame*> (mapFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewmap() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewMap (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewmap(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getMapFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

BOOL TerrainEditorApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (SplashScreen::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinApp::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------

void TerrainEditorApp::ShowTipAtStartup(void)
{
	// CG: This function added by 'Tip of the Day' component.

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bShowSplash)
	{
		CTipDialog dlg;
		if (dlg.m_bStartup)
			IGNORE_RETURN (dlg.DoModal());
	}

}

//-------------------------------------------------------------------

void TerrainEditorApp::ShowTipOfTheDay(void)
{
	CTipDialog dlg;
	IGNORE_RETURN (dlg.DoModal());
}

//-------------------------------------------------------------------

void TerrainEditorApp::onView3d (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getView3dFrame ())
	{
		//-- set as active
		terrainDocument->getView3dFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (view3dTemplate);
		CFrameWnd* view3dFrame = view3dTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!view3dFrame)
			return;

		//-- update the frame
		view3dTemplate->InitialUpdateFrame (view3dFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setView3dFrame (NON_NULL (dynamic_cast<View3dFrame*> (view3dFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::On3dView() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onView3d (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdate3dView(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getView3dFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewBlends (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getBlendGroupFrame ())
	{
		//-- set as active
		terrainDocument->getBlendGroupFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (blendGroupTemplate);
		CFrameWnd* blendGroupFrame = blendGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!blendGroupFrame)
			return;

		//-- update the frame
		blendGroupTemplate->InitialUpdateFrame (blendGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setBlendGroupFrame (NON_NULL (dynamic_cast<BlendGroupFrame*> (blendGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditBlends() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewBlends (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditBlends(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getBlendGroupFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewFractalPreview (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getFractalPreviewFrame ())
	{
		//-- set as active
		terrainDocument->getFractalPreviewFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (fractalPreviewTemplate);
		CFrameWnd* fractalPreviewFrame = fractalPreviewTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!fractalPreviewFrame)
			return;

		//-- update the frame
		fractalPreviewTemplate->InitialUpdateFrame (fractalPreviewFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setFractalPreviewFrame (NON_NULL (dynamic_cast<FractalPreviewFrame*> (fractalPreviewFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewBitmapPreview (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getBitmapPreviewFrame ())
	{
		//-- set as active
		terrainDocument->getBitmapPreviewFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (bitmapPreviewTemplate);
		CFrameWnd* bitmapPreviewFrame = bitmapPreviewTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!bitmapPreviewFrame)
			return;

		//-- update the frame
		bitmapPreviewTemplate->InitialUpdateFrame (bitmapPreviewFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setBitmapPreviewFrame (NON_NULL (dynamic_cast<BitmapPreviewFrame*> (bitmapPreviewFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnFractalPreview() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewFractalPreview (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateFractalPreview(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getFractalPreviewFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnBitmapPreview() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
	{
		onViewBitmapPreview (terrainDocument);
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateBitmapPreview(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
	{
		pCmdUI->SetCheck (terrainDocument->getBitmapPreviewFrame () != 0 ? TRUE : FALSE);
	}
}

//-------------------------------------------------------------------

int TerrainEditorApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
	RecentDirectory::remove ();

	return CWinApp::ExitInstance();
}

//-----------------------------------------------------------------

void TerrainEditorApp::onViewProfile (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getProfileFrame ())
	{
		//-- set as active
		terrainDocument->getProfileFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (profileTemplate);
		CFrameWnd* profileFrame = profileTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!profileFrame)
			return;

		//-- update the frame
		profileTemplate->InitialUpdateFrame (profileFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setProfileFrame (NON_NULL (dynamic_cast<ProfileFrame*> (profileFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewProfile() 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	if (terrainDocument)
		onViewProfile (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewProfile(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getProfileFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewWarning (TerrainEditorDoc* terrainDocument, bool clear)
{
	if (terrainDocument->getWarningFrame ())
	{
		//-- set as active
		terrainDocument->getWarningFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (warningTemplate);
		CFrameWnd* warningFrame = warningTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!warningFrame)
			return;

		//-- update the frame
		warningTemplate->InitialUpdateFrame (warningFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setWarningFrame (NON_NULL (dynamic_cast<WarningFrame*> (warningFrame)));
	}

	if (clear)
		terrainDocument->getWarningFrame ()->clear ();
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewWarning() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewWarning (terrainDocument, false);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewWarning(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getWarningFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewFind (TerrainEditorDoc* terrainDocument, bool clear)
{
	if (terrainDocument->getFindFrame ())
	{
		//-- set as active
		terrainDocument->getFindFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (findTemplate);
		CFrameWnd* findFrame = findTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!findFrame)
			return;

		//-- update the frame
		findTemplate->InitialUpdateFrame (findFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setFindFrame (NON_NULL (dynamic_cast<FindFrame*> (findFrame)));
	}

	if (clear)
		terrainDocument->getFindFrame ()->clear ();
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewFind() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewFind (terrainDocument, false);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewFind(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getFindFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewEnvironment (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getEnvironmentFrame ())
	{
		//-- set as active
		terrainDocument->getEnvironmentFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (environmentTemplate);
		CFrameWnd* environmentFrame = environmentTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!environmentFrame)
			return;

		//-- update the frame
		environmentTemplate->InitialUpdateFrame (environmentFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setEnvironmentFrame (NON_NULL (dynamic_cast<EnvironmentFrame*> (environmentFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnViewEnvironment() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewEnvironment (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateViewEnvironment(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getEnvironmentFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewFractals (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getFractalFrame ())
	{
		//-- set as active
		terrainDocument->getFractalFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (fractalGroupTemplate);
		CFrameWnd* fractalGroupFrame = fractalGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!fractalGroupFrame)
			return;

		//-- update the frame
		fractalGroupTemplate->InitialUpdateFrame (fractalGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setFractalFrame (NON_NULL (dynamic_cast<FractalFrame*> (fractalGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditFractal() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewFractals (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditFractal(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getFractalFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void TerrainEditorApp::onViewBitmaps (TerrainEditorDoc* terrainDocument)
{
	if (terrainDocument->getBitmapFrame ())
	{
		//-- set as active
		terrainDocument->getBitmapFrame ()->ActivateFrame ();
	}
	else
	{
		//-- create frame [and views] and attach to document
		NOT_NULL (bitmapGroupTemplate);
		CFrameWnd* bitmapGroupFrame = bitmapGroupTemplate->CreateNewFrame (terrainDocument, NULL);

		//-- if it couldn't be created, just return
		if (!bitmapGroupFrame)
			return;

		//-- update the frame
		bitmapGroupTemplate->InitialUpdateFrame (bitmapGroupFrame, terrainDocument);

		//-- tell the document about it
		terrainDocument->setBitmapFrame (NON_NULL (dynamic_cast<BitmapFrame*> (bitmapGroupFrame)));
	}
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnEditBitmap() 
{
	TerrainEditorDoc* terrainDocument = GetApp ()->getDocument ();

	if (terrainDocument)
		onViewBitmaps (terrainDocument);
}

//-------------------------------------------------------------------

void TerrainEditorApp::OnUpdateEditBitmap(CCmdUI* pCmdUI) 
{
	TerrainEditorDoc* const terrainDocument = getDocument ();

	pCmdUI->Enable (terrainDocument != 0 ? TRUE : FALSE);

	if (terrainDocument)
		pCmdUI->SetCheck (terrainDocument->getBitmapFrame () != 0 ? TRUE : FALSE);
}

//-------------------------------------------------------------------

CDocument* TerrainEditorApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	FILE* const infile = fopen (lpszFileName, "rb");
	if (infile)
	{
		fclose (infile);
		
		return CWinApp::OpenDocumentFile(lpszFileName);
	}

	return 0;
}

//-------------------------------------------------------------------

void TerrainEditorApp::SaveWindowPosition(const CWnd* cwnd, const char* name)
{
	WINDOWPLACEMENT wp;
	cwnd->GetWindowPlacement(&wp);
	WriteProfileInt(name,"flags",wp.flags);
	WriteProfileInt(name,"showCmd",wp.showCmd);
	WriteProfileInt(name,"x1",wp.rcNormalPosition.left);
	WriteProfileInt(name,"y1",wp.rcNormalPosition.top);
	WriteProfileInt(name,"x2",wp.rcNormalPosition.right);
	WriteProfileInt(name,"y2",wp.rcNormalPosition.bottom);
}

bool TerrainEditorApp::RestoreWindowPosition(CWnd* cwnd, const char* name)
{
	WINDOWPLACEMENT wp;
	cwnd->GetWindowPlacement(&wp);
	if( (-1 != (wp.flags = GetProfileInt(name,"flags",-1)))
		&& (-1 != (wp.showCmd = GetProfileInt(name,"showCmd",-1)))
		&& (-1 != (wp.rcNormalPosition.left = GetProfileInt(name,"x1",-1)))
		&& (-1 != (wp.rcNormalPosition.top = GetProfileInt(name,"y1",-1)))
		&& (-1 != (wp.rcNormalPosition.right = GetProfileInt(name,"x2",-1)))
		&& (-1 != (wp.rcNormalPosition.bottom = GetProfileInt(name,"y2",-1)))
	)
	{
		cwnd->SetWindowPlacement(&wp);
		return true;
	}
	return false;
}
