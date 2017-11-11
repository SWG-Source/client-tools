// ======================================================================
//
// Viewer.cpp
// Portions Copyright 1999, Bootprint Entertainment Inc.
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstViewer.h"
#include "viewer.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "viewerDoc.h"
#include "viewerView.h"

#include "PreferencesDialog.h"
#include "SkeletalAppearanceTemplateView.h"
#include "VariableSetView.h"
#include "ViewerPreferences.h"
#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/Audio.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/SetupClientGame.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorldCamera.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/DetailAppearanceTemplate.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/SetupClientObject.h"
#include "clientObject/ShadowManager.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/CompositeMesh.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientTerrain/SetupClientTerrain.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererManager.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedPathfinding/SetupSharedPathfinding.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedXml/SetupSharedXml.h"

#include <direct.h>

// ==================================================================
// constants
// ======================================================================

namespace ViewerNamespace
{
	const char *const ms_registryAnimationMapDialogSectionName            = "AnimationMapDialog";
	const char *const ms_registryAnimationFilenameEntry                   = "Filename";

	const char *const ms_registryGeneralSectionName                       = "General";
	const char *const ms_registryOpenFilenameEntry                        = "OpenFilename";
	const char *const ms_registryMeshGeneratorFilenameEntry               = "MeshGeneratorFilename";
	const char *const ms_registrySkeletonTemplateFilenameEntry            = "SkeletonTemplateFilename";
	const char *const ms_registrySkeletalAppearanceInstanceFilenameEntry  = "SkeletalAppearanceInstanceFilename";
	const char *const ms_registrySkeletalAppearanceTemplateFilenameEntry  = "SkeletalAppearanceTemplateFilename";
	const char *const ms_registrySkeletalAppearanceWorkspaceFilenameEntry = "SkeletalAppearanceWorkspaceFilename";

	Texture const * ms_defaultTexture = 0;
}

using namespace ViewerNamespace;

// ======================================================================

void CONSOLE_PRINT (const CString& newMessage)
{
	DEBUG_REPORT_LOG(true, ("%s", newMessage));
	CMainFrame* mainFrame = static_cast<CMainFrame*> (AfxGetMainWnd ());

	if (mainFrame)
		mainFrame->addMessage (newMessage);
}

// ======================================================================

CViewerApp      theApp;

static __int64  lastCounterTick;
static double   secondsPerCounterTick;

// ======================================================================

BEGIN_MESSAGE_MAP(CViewerApp, CWinApp)
	//{{AFX_MSG_MAP(CViewerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TOOLS_PREFERENCES, OnToolsPreferences)
	ON_COMMAND(ID_TOOLS_POPUPDEBUGMENU, OnToolsPopupDebugMenu)
	ON_COMMAND(ID_FILE_CLOSEALL, OnFileCloseall)
	ON_COMMAND(ID_FILE_NEW_SKELETAL_APPEARANCE_TEMPLATE, OnFileNewSkeletalAppearanceTemplate)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// ======================================================================

CViewerApp::CViewerApp()
:
	m_variableSetViewDocTemplate(0)
{
	strcpy(m_animationMapFilename, "");
	strcpy(m_openFilename, "");
	strcpy(m_lastMeshGeneratorFilename, "");
	strcpy(m_lastSkeletonTemplateFilename, "");

	strcpy(m_meshGeneratorReferenceDirectory, "appearance/mesh");
	strcpy(m_skeletonTemplateReferenceDirectory, "appearance/skeleton");
	strcpy(m_skeletalAppearanceTemplateReferenceDirectory, "appearance");
}

// ----------------------------------------------------------------------

CViewerApp::~CViewerApp()
{
}

// ----------------------------------------------------------------------

BOOL CViewerApp::InitInstance()
{
	_getcwd(m_applicationDirectory, MAX_PATH);

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
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Verant Interactive\\Engine Viewer"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_VIEWERTYPE,
		RUNTIME_CLASS(CViewerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CViewerView));
	AddDocTemplate(pDocTemplate);

	//-- create the VariableSetView document template
	m_variableSetViewDocTemplate = new CMultiDocTemplate(
		IDR_VIEWERTYPE,
		RUNTIME_CLASS(CViewerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(VariableSetView));
	AddDocTemplate(m_variableSetViewDocTemplate);

	//-- create the VariableSetView document template
	m_skeletalAppearanceTemplateViewDocTemplate = new CMultiDocTemplate(
		IDR_VIEWERTYPE,
		RUNTIME_CLASS(CViewerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(SkeletalAppearanceTemplateView));
	AddDocTemplate(m_skeletalAppearanceTemplateViewDocTemplate);

	// note: we don't add this to doc template because we explicitly
	// create the windows ourselves.

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
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

	//-- setup shared
	{
		//-- thread
		SetupSharedThread::install();

		//-- debug
		SetupSharedDebug::install(4096);

		//-- foundation
		SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_mfc);
		setupFoundationData.useWindowHandle = true;
		setupFoundationData.windowHandle    = pMainFrame->m_hWnd;
		setupFoundationData.configFile      = "tools.cfg";
		setupFoundationData.verboseWarnings = true;
        setupFoundationData.writeMiniDumps  = ApplicationVersion::isBootlegBuild();
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
		SetupSharedObject::addSlotIdManagerData(setupObjectData, false);
		SetupSharedObject::install (setupObjectData);

		//-- pathfinding
		SetupSharedPathfinding::install();

		//-- terrain
		SetupSharedTerrain::Data setupSharedTerrainData;
		SetupSharedTerrain::setupGameData (setupSharedTerrainData);
		SetupSharedTerrain::install (setupSharedTerrainData);

		//-- xml
		SetupSharedXml::install ();

		//-- game
		SetupSharedGame::Data sharedGameData;
		SetupSharedGame::install(sharedGameData);

		SetupSharedLog::install ("Viewer");
	}

	//-- setup client
	{
		//-- audio
		SetupClientAudio::install ();

		//-- graphics
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultMFCData (setupGraphicsData);
		SetupClientGraphics::install (setupGraphicsData);

		ms_defaultTexture = TextureList::fetchDefaultTexture ();

		//-- object
		SetupClientObject::Data setupClientObjectData;
		SetupClientObject::setupToolData (setupClientObjectData);
		SetupClientObject::install (setupClientObjectData);

		DetailAppearance::setFadeInEnabled(false);
		MeshAppearance::setFadeInEnabled(false);

		ShadowManager::setMeshShadowsVolumetric (true);
		ShadowManager::setSkeletalShadowsVolumetric (true);

		//-- animation and skeletal animation
		SetupClientAnimation::install ();

		SetupClientSkeletalAnimation::Data  data;
		SetupClientSkeletalAnimation::setupViewerData(data);
		SetupClientSkeletalAnimation::install (data);

		//-- texture renderer
		SetupClientTextureRenderer::install ();

		//-- particle system
		SetupClientParticle::install ();

		//-- terrain system
		SetupClientTerrain::install();

		//-- game
		SetupClientGame::Data setupGameData;
		SetupClientGame::setupToolData (setupGameData);
		SetupClientGame::install (setupGameData);

		// make collision think we're running on the server so we load path graphs and such

		CollisionWorld::setServerSide(true);
	}
	
	// -qq- don't like these being hard-coded
	TreeFile::addSearchAbsolute(0);

	//-- install app systems
	ViewerPreferences::install ();
	CViewerDoc::install();

	// get clock frequency and last counter tick
	BOOL           result;
	__int64        frequency;

	result = QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));
	FATAL(!result, ("failed to get Windows timer frequency"));
	secondsPerCounterTick = 1.0 / static_cast<double>(frequency);

	result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&lastCounterTick));
	FATAL(!result, ("failed to get Windows timer tick"));

	loadPreferences ();

	//if given a command line file to open, do it
	if(cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
		OpenDocumentFile(cmdInfo.m_strFileName);

	return TRUE;
}

// ----------------------------------------------------------------------

int CViewerApp::ExitInstance() 
{
	//-- save our settings for next time
	savePreferences ();

	if (ms_defaultTexture)
	{
		ms_defaultTexture->release ();
		ms_defaultTexture = 0;
	}

	//-- close down app, game and engine systems
	CViewerDoc::remove ();
	ViewerPreferences::remove ();
	SetupSharedFoundation::remove();

	SetupSharedThread::remove();

	return CWinApp::ExitInstance();
}

// ----------------------------------------------------------------------
// alterDocuments calls alter() on all documents.

void CViewerApp::alterDocuments(void)
{
	BOOL           result;
	__int64        counterTick;
	bool           audioUpdated = false;

	// calculate elapsed time since last idle
	result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counterTick));
	FATAL(!result, ("failed to get Windows timer tick"));

	real elapsedTime = static_cast<real>((counterTick - lastCounterTick) * secondsPerCounterTick);
	lastCounterTick  = counterTick;

	Os::update();

	// for each document template type, for each document attached, call the doc alter
	POSITION  docTemplatePos = GetFirstDocTemplatePosition();
	while (docTemplatePos)
	{
		CDocTemplate *const docTemplate = GetNextDocTemplate(docTemplatePos);
		DEBUG_FATAL(!docTemplate, ("null docTemplate, unexpected\n"));

		POSITION  docPos = docTemplate->GetFirstDocPosition();
		while (docPos)
		{
			CViewerDoc *const viewerDoc = dynamic_cast<CViewerDoc*>(docTemplate->GetNextDoc(docPos));
			if (viewerDoc)
			{
				//-- alter the doc
				viewerDoc->alter(elapsedTime);

				if (!audioUpdated)
				{
					//-- update the sound
					POSITION viewPos = viewerDoc->GetFirstViewPosition();
					while (viewPos && !audioUpdated)
					{
						CViewerView *const viewerView = dynamic_cast<CViewerView*>(viewerDoc->GetNextView(viewPos));
						if (!viewerView)
							continue;
						
						//-- found a view.  Update audio, get the camera and set it's position as the ear position.
						Audio::alter(elapsedTime, viewerView->getCamera());
						audioUpdated = true;
					}
				}
			}
		}
	}

	// perform other once-per-game-loop processing here
	// ideally we'd like to inject this between the alter and the render
	// so we don't have the one-frame of blackness.
	TextureRendererManager::alter(elapsedTime);
	ViewerPreferences::alter (elapsedTime);
	Graphics::update(elapsedTime);
}

// ----------------------------------------------------------------------

int CViewerApp::Run(void)
{
	ASSERT_VALID(this);
	LONG lIdleCount = 0;
    MSG msg;

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		// allow MFC to update our widgets
		while (OnIdle(lIdleCount++))
			{}

		// pump messages
		do
		{
			// pump message, but quit on WM_QUIT
			if (!PumpMessage())
				return ExitInstance();

			// reset the idle state so widgets properly update
			if (IsIdleMessage(&msg))
				lIdleCount = 0;

		} while (::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));

		// allow all documents to alter
		alterDocuments();
	}
}

// ======================================================================

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

// ======================================================================

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

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
// App command to run the dialog

void CViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ----------------------------------------------------------------------

void CViewerApp::OnToolsPreferences() 
{
	// TODO: Add your command handler code here
	PreferencesDialog dlg (0);
	dlg.DoModal ();
}

// ----------------------------------------------------------------------

void CViewerApp::OnToolsPopupDebugMenu() 
{
	Os::requestPopupDebugMenu();
}

// ----------------------------------------------------------------------

void CViewerApp::loadPreferences (void)
{
	FatalSetThrowExceptions (true);
	try
	{
		char nameBuffer [1000];
		sprintf (nameBuffer, "%s\\%s", m_applicationDirectory, "viewerPreferencesDoNotEdit.iff");

		ViewerPreferences::load (nameBuffer);
	}
	catch (FatalException& e)
	{
		MessageBox(0, e.getMessage(), "Error loading viewerPreferencesDoNotEdit.iff file", MB_ICONSTOP);
	}
	FatalSetThrowExceptions (false);

	//-- load registry-based settings
	// animation map filename
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_animationMapFilename), m_animationMapFilename));
	const CString animationMapFilename = GetProfileString(ms_registryAnimationMapDialogSectionName, ms_registryAnimationFilenameEntry, m_animationMapFilename);
	strcpy(m_animationMapFilename, animationMapFilename);

	// open filename directory
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_openFilename), m_openFilename));
	const CString openFilename = GetProfileString(ms_registryGeneralSectionName, ms_registryOpenFilenameEntry, m_openFilename);
	strcpy(m_openFilename, openFilename);

	// mesh generator filename
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_lastMeshGeneratorFilename), m_lastMeshGeneratorFilename));
	const CString meshGeneratorFilename = GetProfileString(ms_registryGeneralSectionName, ms_registryMeshGeneratorFilenameEntry, m_lastMeshGeneratorFilename);
	strcpy(m_lastMeshGeneratorFilename, meshGeneratorFilename);

	// skeletal appearance template filename
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_lastSkeletalAppearanceTemplateFilename), m_lastSkeletalAppearanceTemplateFilename));
	const CString skeletalAppearanceTemplateFilename = GetProfileString(ms_registryGeneralSectionName, ms_registrySkeletalAppearanceTemplateFilenameEntry, m_lastSkeletalAppearanceTemplateFilename);
	strcpy(m_lastSkeletalAppearanceTemplateFilename, skeletalAppearanceTemplateFilename);

	// skeletal appearance workspace filename
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_lastSkeletalAppearanceWorkspaceFilename), m_lastSkeletalAppearanceWorkspaceFilename));
	const CString skeletalAppearanceWorkspaceFilename = GetProfileString(ms_registryGeneralSectionName, ms_registrySkeletalAppearanceWorkspaceFilenameEntry, m_lastSkeletalAppearanceWorkspaceFilename);
	strcpy(m_lastSkeletalAppearanceWorkspaceFilename, skeletalAppearanceWorkspaceFilename);

	// skeleton template filename
	IGNORE_RETURN(GetCurrentDirectory(sizeof(m_lastSkeletonTemplateFilename), m_lastSkeletonTemplateFilename));
	const CString skeletonTemplateFilename = GetProfileString(ms_registryGeneralSectionName, ms_registrySkeletonTemplateFilenameEntry, m_lastSkeletonTemplateFilename);
	strcpy(m_lastSkeletonTemplateFilename, skeletonTemplateFilename);
}

// ----------------------------------------------------------------------

void CViewerApp::savePreferences (void)
{
	FatalSetThrowExceptions (true);
	try
	{
		char nameBuffer [1000];
		sprintf (nameBuffer, "%s\\%s", m_applicationDirectory, "viewerPreferencesDoNotEdit.iff");

		ViewerPreferences::save (nameBuffer);
	}
	catch (FatalException& e)
	{
		MessageBox(0, e.getMessage(), "Error saving viewerPreferencesDoNotEdit.iff", MB_ICONSTOP);
	}
	FatalSetThrowExceptions (false);

	//-- save registry-based settings
	IGNORE_RETURN(WriteProfileString(ms_registryAnimationMapDialogSectionName, ms_registryAnimationFilenameEntry, m_animationMapFilename));
	IGNORE_RETURN(WriteProfileString(ms_registryGeneralSectionName, ms_registryOpenFilenameEntry, m_openFilename));
	IGNORE_RETURN(WriteProfileString(ms_registryGeneralSectionName, ms_registryMeshGeneratorFilenameEntry, m_lastMeshGeneratorFilename));
	IGNORE_RETURN(WriteProfileString(ms_registryGeneralSectionName, ms_registrySkeletalAppearanceTemplateFilenameEntry, m_lastSkeletalAppearanceTemplateFilename));
	IGNORE_RETURN(WriteProfileString(ms_registryGeneralSectionName, ms_registrySkeletalAppearanceWorkspaceFilenameEntry, m_lastSkeletalAppearanceWorkspaceFilename));
	IGNORE_RETURN(WriteProfileString(ms_registryGeneralSectionName, ms_registrySkeletonTemplateFilenameEntry, m_lastSkeletonTemplateFilename));
}

// ----------------------------------------------------------------------

void CViewerApp::OnFileCloseall() 
{
	// TODO: Add your command handler code here
	CloseAllDocuments (false);	
}

// ----------------------------------------------------------------------

void CViewerApp::OnFileNewSkeletalAppearanceTemplate() 
{
	NOT_NULL(m_skeletalAppearanceTemplateViewDocTemplate);

	//-- create a viewer doc with graphic viewer attached
	POSITION docTemplatePosition  = GetFirstDocTemplatePosition();
	CDocTemplate *const pTemplate = GetNextDocTemplate(docTemplatePosition);

	CViewerDoc *const viewerDoc = dynamic_cast<CViewerDoc*>(pTemplate->OpenDocumentFile(NULL));
	if (!viewerDoc)
	{
		DEBUG_REPORT_LOG(true, ("failed to create viewer doc\n"));
		return;
	}

	//-- create the SkeletalAppearanceTemplate2, give it to the document
	SkeletalAppearanceTemplate *const skeletalAppearanceTemplate = new SkeletalAppearanceTemplate();
	if (!skeletalAppearanceTemplate)
	{
		// -TRF- fix to bail out gracefully
		return;
	}
	viewerDoc->setSkeletalAppearanceTemplate(skeletalAppearanceTemplate);
	IGNORE_RETURN(AppearanceTemplateList::fetchNew(skeletalAppearanceTemplate));

	//-- create the SkeletalAppearanceTemplate view
	viewerDoc->OnViewSkeletalAppearanceTemplate();
}

// ----------------------------------------------------------------------

void CViewerApp::OnFileOpen() 
{
	//-- prompt the user
	CString filename(m_openFilename);

	if (!m_pDocManager->DoPromptFileName(filename, AFX_IDS_OPENFILE, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	//-- open the doc
	OpenDocumentFile(filename);

	//-- save the "open" filename
	strcpy(m_openFilename, filename);
}

//-------------------------------------------------------------------

CViewerDoc* CViewerApp::getDocument (void) const
{
	//-- only display the new view if there is a document
	if (m_pMainWnd)
	{
		CMDIFrameWnd* mdiFrameWnd = static_cast<CMDIFrameWnd*> (static_cast<CMainFrame*> (m_pMainWnd)->GetActiveFrame ());

		if (mdiFrameWnd && mdiFrameWnd->GetActiveDocument ())
			return static_cast<CViewerDoc*> (mdiFrameWnd->GetActiveDocument ());
	}

	return 0;
}

// ======================================================================
