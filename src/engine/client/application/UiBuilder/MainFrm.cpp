// MainFrm.cpp : implementation of the CMainFrame class
//

#include "FirstUiBuilder.h"
#include "DefaultObjectPropertiesDialog.h"
#include "EditUtils.h"
#include "ObjectEditor.h"
#include "ObjectFactory.h"
#include "ObjectBrowserDialog.h"
#include "ObjectPropertiesDialog.h"
#include "ObjectPropertiesTreeDialog.h"
#include "SerializedObjectBuffer.h"
#include "UserWindowsMessages.h"

#include "UiBuilder.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIScriptEngine.h"
#include "UIDirect3DPrimaryCanvas.h"
#include "UIBuilderHistory.h"
#include "UIBuilderLoader.h"
#include "UITextStyleManager.h"
#include "UIVersion.h"
#include "UISaver.h"

#include "MainFrm.h"
#include "FileLocator.h"

#include <algorithm>
#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>

/////////////////////////////////////////////////////////////////////////////

extern CUiBuilderApp theApp;

/////////////////////////////////////////////////////////////////////////////

const long               LineWidth             = 1;
const long               HandleSize            = 4;
const char               *gApplicationName            = "UIBuilder";
const char               *gDefaultExtension           = "ui";
UILowerString             gVisualEditLockPropertyName("VisualEditLock");
const char               *gFileFilter                 = "User Interface Files (*.ui)|ui_*.ui||";
const char               *gIncludeFileFilter          = "User Interface Include Files (.inc)\0ui_*.inc\0\0";

UIDirect3DPrimaryCanvas  *gPrimaryDisplay      = 0;

UINT CMainFrame::m_clipboardFormat=RegisterClipboardFormat("UiBuilderSerializedObjectTree");

UIColor CMainFrame::m_highlightOutlineColor          (  0, 255,   0, 255);
UIColor CMainFrame::m_selectionOutlineColor          (255,   0,   0, 255);
UIColor CMainFrame::m_highlightSelectionOutlineColor (255, 255,   0, 255);
UIColor CMainFrame::m_selectionFillColor             (255,   0,   0,   0);
UIColor CMainFrame::m_selectionBoxOutlineColor       (160, 160, 160, 160);
UIColor CMainFrame::m_selectionDragBoxOutlineColor   (  0, 255, 255, 255);

/////////////////////////////////////////////////////////////////////////////

void RecursiveSetProperty( UIBaseObject *root, const UILowerString & PropertyName, const char *OldValue, const char *NewValue )
{
	UIString Value;

	if( root->GetProperty( PropertyName, Value ) )
	{
		if( !_stricmp( UIUnicode::wideToNarrow (Value).c_str(), OldValue ) )
			root->SetProperty( PropertyName, UIUnicode::narrowToWide (NewValue) );

		UIBaseObject::UIObjectList Children;

		root->GetChildren( Children );

		for( UIBaseObject::UIObjectList::iterator i = Children.begin(); i != Children.end(); ++i )
		{
			RecursiveSetProperty( *i, PropertyName, OldValue, NewValue );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveas)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEAS, OnUpdateFileSaveas)
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_VIEW_DEFAULT_PROPERTIES, OnViewDefaultProperties)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEFAULT_PROPERTIES, OnUpdateViewDefaultProperties)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_COMMAND(ID_SELECTION_BURROW, OnSelectionBurrow)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_BURROW, OnUpdateSelectionBurrow)
	ON_COMMAND(ID_SELECTION_CLEARALL, OnSelectionClearall)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_CLEARALL, OnUpdateSelectionClearall)
	ON_COMMAND(ID_SELECTION_DESCENDANTS, OnSelectionDescendants)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_DESCENDANTS, OnUpdateSelectionDescendants)
	ON_COMMAND(ID_SELECTION_ANCESTORS, OnSelectionAncestors)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ANCESTORS, OnUpdateSelectionAncestors)
	ON_COMMAND(ID_EDIT_CANCELDRAG, OnEditCanceldrag)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CANCELDRAG, OnUpdateEditCanceldrag)
	ON_COMMAND(ID_SELECTION_DELETE, OnSelectionDelete)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_DELETE, OnUpdateSelectionDelete)
	ON_WM_ACTIVATEAPP()
	ON_COMMAND(ID_TOGGLE_GRID, OnToggleGrid)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_GRID, OnUpdateToggleGrid)
	ON_COMMAND(ID_SELECTION_ALIGNBOTTOM, OnSelectionAlignbottom)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNBOTTOM, OnUpdateSelectionAlignbottom)
	ON_COMMAND(ID_SELECTION_ALIGNLEFT, OnSelectionAlignleft)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNLEFT, OnUpdateSelectionAlignleft)
	ON_COMMAND(ID_SELECTION_ALIGNRIGHT, OnSelectionAlignright)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNRIGHT, OnUpdateSelectionAlignright)
	ON_COMMAND(ID_SELECTION_ALIGNTOP, OnSelectionAligntop)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNTOP, OnUpdateSelectionAligntop)
	ON_COMMAND(ID_SELECTION_ALIGNWIDTH, OnSelectionAlignwidth)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNWIDTH, OnUpdateSelectionAlignwidth)
	ON_COMMAND(ID_SELECTION_ALIGNHEIGHT, OnSelectionAlignheight)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNHEIGHT, OnUpdateSelectionAlignheight)
	ON_COMMAND(ID_SELECTION_ALIGNCENTERX, OnSelectionAligncenterx)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNCENTERX, OnUpdateSelectionAligncenterx)
	ON_COMMAND(ID_SELECTION_ALIGNCENTERY, OnSelectionAligncentery)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ALIGNCENTERY, OnUpdateSelectionAligncentery)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(IDC_CHECKOUT, OnCheckout)
	ON_UPDATE_COMMAND_UI(IDC_CHECKOUT, OnUpdateCheckout)
	ON_COMMAND(ID_VIEW_OBJECTBROWSER, OnViewObjectbrowser)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTBROWSER, OnUpdateViewObjectbrowser)
	ON_COMMAND(ID_VIEW_SELECTIONPROPERTIES, OnViewSelectionproperties)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTIONPROPERTIES, OnUpdateViewSelectionproperties)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_MESSAGE(WM_paintChild, OnPaintChild)
	ON_MESSAGE(WM_closePropertiesDialog, closePropertiesDialog)
	ON_MESSAGE(WM_closeObjectBrowserDialog, closeObjectBrowserDialog)
	ON_COMMAND_RANGE(ID_INSERT_NAMESPACE, ID_INSERT_DEFORMER_ROTATE, OnInsertObject)
	ON_UPDATE_COMMAND_UI_RANGE(ID_INSERT_NAMESPACE, ID_INSERT_DEFORMER_ROTATE, OnUpdateInsertObject)
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
:	m_editor(0),
	m_browserDialog(0),
	m_propertiesDialog(0),
	m_factory(0),
	m_defaultsManager(0),
	gVersionFilename(false),
	g_showShaders(false),
	m_inVisualEditingMode(true),
	gDrawCursor(true),
	gDrawGrid(false),
	m_drawSelectionRect(true),
	m_showActive(false),
	m_browserDialogOpen(false),
	m_propertiesDialogOpen(false),
	m_refreshTimer(0),
	gGridColor(255,255,255,32),
	gGridMajorTicks(10),
	gTriangleCount(0),
	gFlushCount(0),
	gFrameCount(0)
{
	m_cursors[CT_Normal]    = LoadCursor(0, IDC_ARROW);
	m_cursors[CT_Crosshair] = LoadCursor(0, IDC_CROSS);
	m_cursors[CT_Hand]      = LoadCursor(0, MAKEINTRESOURCE(IDC_GRABHAND));
	m_cursors[CT_SizeAll]   = LoadCursor(0, IDC_SIZEALL);
	m_cursors[CT_SizeNESW]  = LoadCursor(0, IDC_SIZENESW);
	m_cursors[CT_SizeNS]    = LoadCursor(0, IDC_SIZENS);
	m_cursors[CT_SizeNWSE]  = LoadCursor(0, IDC_SIZENWSE);
	m_cursors[CT_SizeWE]    = LoadCursor(0, IDC_SIZEWE);
	m_cursors[CT_Wait]      = LoadCursor(0, IDC_WAIT);
}

CMainFrame::~CMainFrame()
{
	_destroyEditingObjects();
}

bool CMainFrame::openFile(const char *i_fileName)
{
	return _openWorkspaceFile(i_fileName);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rc;
	rc.left=0;
	rc.right=1024;
	rc.top=0;
	rc.bottom=768;
	AdjustWindowRectEx(&rc, lpCreateStruct->style, lpCreateStruct->hMenu!=0, lpCreateStruct->dwExStyle);
	lpCreateStruct->cx = rc.Width();
	lpCreateStruct->cy = rc.Height();

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	
//AFX_IDW_TOOLBAR
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

//AFX_IDW_DIALOGBAR
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndDlgBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

//AFX_IDW_STATUS_BAR
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// -------------------------------------------------

	_resize(1024, 768);

	// -------------------------------------------------

	if (!InitializeCanvasSystem(m_wndView.m_hWnd))
	{
		::MessageBox(NULL, "Could not initialize canvas system", NULL, MB_OK );
		return -1;
	}

	// Create the primary display before we invoke the loader so that the global gPrimaryDisplay
	// is set up - so that we can make the textures with the correct pixelformat.
	m_wndView.GetClientRect(&rc);
	gPrimaryDisplay = new UIDirect3DPrimaryCanvas( UISize( rc.right, rc.bottom ), m_wndView.m_hWnd, false );
	gPrimaryDisplay->ShowShaders (g_showShaders);
	gPrimaryDisplay->Attach( 0 );

	UIManager::gUIManager().SetScriptEngine( new UIScriptEngine );


	///////////////////////////////////////////////////////////////////

	char DirBuff[_MAX_PATH + 1];
	GetCurrentDirectory( sizeof( DirBuff ), DirBuff );
	gInitialDirectory = DirBuff;
	
	FILE *fp;

	/* TODO
	fp = fopen("defaults.cfg", "rb");
	if (fp)
	{
		m_defaultsManager.LoadFrom(fp);
		fclose(fp);
	}
	*/
	
	// searchpaths for FileLocator - jwatson
	
	FileLocator & loc = FileLocator::gFileLocator ();
	loc.addPath ("./");
	
	fp = fopen ("uibuilder_searchpaths.cfg", "rb");
	if (fp)
	{
		char buf[1024];
		
		while (fgets (buf, 1024, fp))
		{
			// strip newline
			char * ptr = strchr (buf, '\n');
			
			if (ptr)
				*ptr = 0;
			
			ptr = strchr (buf, '\r');
			
			if (ptr)
				*ptr = 0;
			
			loc.addPath (buf);
		}
		
		fclose (fp);
	}

	///////////////////////////////////////////////////////////////////

	m_refreshTimer = SetTimer(1, 100, 0);

	m_menuTipManager.Install(this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
	{
		return FALSE;
	}
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

//////////////////
// Calculate the size of the total frame, given a desired client (form) size.
// Start with client rect and add all the extra stuff.
//
void CMainFrame::CalcWindowRect(PRECT prcClient, UINT nAdjustType)
{
	const int desiredWidth = prcClient->right-prcClient->left;
	const int desiredHeight = prcClient->bottom-prcClient->top;

	CRect rect(0, 0, 32767, 32767);
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery,
		&rect, &rect, FALSE);

	CRect adjRect;
	adjRect.left=0;
	adjRect.top=0;
	adjRect.right=desiredWidth;
	adjRect.bottom=desiredHeight;
	::AdjustWindowRectEx(adjRect, GetStyle(), TRUE, GetExStyle());

	const int xAdjust = adjRect.Width()-desiredWidth;
	const int yAdjust = adjRect.Height()-desiredHeight;

	prcClient->right += xAdjust;
	prcClient->bottom += yAdjust + rect.Height();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::setCursor(CursorType showType)
{
	SetCursor(m_cursors[showType]);
}

// ====================================================

void CMainFrame::setCapture()
{
	m_wndView.SetCapture();
}

// ====================================================

void CMainFrame::releaseCapture()
{
	ReleaseCapture();
}

// ====================================================

void CMainFrame::redrawViews(bool synchronous)
{
	m_wndView.redrawView(synchronous);
}

// ====================================================

void CMainFrame::copyObjectBuffersToClipboard(const std::list<SerializedObjectBuffer> &i_buffers)
{
	if (!m_hWnd || !m_clipboardFormat)
	{
		return;
	}

	// Open the clipboard, and empty it. 
	if (!OpenClipboard())
	{
		return; 
	}
	if (!EmptyClipboard())
	{
		return;
	}

	std::list<SerializedObjectBuffer>::const_iterator bi;

	//------------------------------------------------------
	// compute data size
	int size=0;
	for (bi=i_buffers.begin();bi!=i_buffers.end();++bi)
	{
		size+=4; // file size.
		size+=bi->size();
	}
	//------------------------------------------------------

	if (size)
	{
		//------------------------------------------------------
		// allocate and fill buffer
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, size); 
		if (hglbCopy)
		{
			char *const copy = static_cast<char *>(GlobalLock(hglbCopy));

			char *copyIter=copy;
			for (bi=i_buffers.begin();bi!=i_buffers.end();++bi)
			{
				const SerializedObjectBuffer::Buffer &buffer = bi->getBuffer();
				const int bufferSize = buffer.size();

				*reinterpret_cast<int *>(copyIter) = bufferSize;
				copyIter+=4;

				memcpy(copyIter, &buffer[0], bufferSize);
				copyIter+=bufferSize;
			}

			assert(copyIter == copy + size);

			GlobalUnlock(hglbCopy); 
			
			// Place the handle on the clipboard. 
			SetClipboardData(m_clipboardFormat, hglbCopy); 
		}
		//------------------------------------------------------
	}

	CloseClipboard(); 
}

// ====================================================

bool CMainFrame::pasteObjectBuffersFromClipboard(std::list<SerializedObjectBuffer> &o_buffers)
{
	if (!m_hWnd || !m_clipboardFormat)
	{
		return false;
	}

	if (!IsClipboardFormatAvailable(m_clipboardFormat)) 
	{
		return false;
	}

	bool success=false;

	if (OpenClipboard())
	{
		HANDLE hData = GetClipboardData(m_clipboardFormat);
		if (hData)
		{
			DWORD size = GlobalSize(hData);
			if (size)
			{
				const char *const data = static_cast<char *>(GlobalLock(hData));
				const char *const dataEnd = data + size;
				if (data)
				{
					const char *dataIter = data;

					while (dataIter<dataEnd)
					{
						int bufferSize = *reinterpret_cast<const int *>(dataIter);
						dataIter+=4;

						if (dataIter + bufferSize>dataEnd)
						{
							break;
						}

						o_buffers.push_back();
						SerializedObjectBuffer &newBuffer = o_buffers.back();
						newBuffer.setBuffer(bufferSize, dataIter);
						dataIter+=bufferSize;
					}

					success = dataIter==dataEnd && !o_buffers.empty();
					GlobalUnlock(hData);
				}
			}
		}
		CloseClipboard();
	}

	if (!success)
	{
		o_buffers.clear();
	}

	return success;
}

// ====================================================

void CMainFrame::onRootSize(int width, int height)
{
	if (width<16)
	{
		width=16;
	}
	else if (width>2048)
	{
		width=2048;
	}
	if (height<16)
	{
		height=16;
	}
	else if (height>2048)
	{
		height=2048;
	}
	_resize(width, height);
}

// ====================================================

LRESULT CMainFrame::closePropertiesDialog(WPARAM, LPARAM)
{
	if (m_propertiesDialog)
	{
		m_propertiesDialogOpen=false;
		m_propertiesDialog->ShowWindow(SW_HIDE);
	}
	return 0;
}

// ====================================================

LRESULT CMainFrame::closeObjectBrowserDialog(WPARAM, LPARAM)
{
	if (m_browserDialog)
	{
		m_browserDialogOpen=false;
		m_browserDialog->ShowWindow(SW_HIDE);
	}
	return 0;
}

// ====================================================

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_resize(int desiredWidth, int desiredHeight)
{
	CRect screenRect;
	screenRect.left=0;
	screenRect.right=desiredWidth;
	screenRect.top=0;
	screenRect.bottom=desiredHeight;
	::AdjustWindowRectEx(&screenRect, m_wndView.GetStyle(), FALSE, m_wndView.GetExStyle());

	const int viewWidth = screenRect.Width();
	const int viewHeight = screenRect.Height();

	GetWindowRect(screenRect);
	screenRect.right = screenRect.left + viewWidth;
	screenRect.bottom = screenRect.top + viewHeight;
	CalcWindowRect(screenRect, adjustBorder);
	MoveWindow(&screenRect);
}

// ====================================================

void CMainFrame::_createEditingObjects()
{
	m_defaultsManager = new DefaultObjectPropertiesManager;
	m_factory = new ObjectFactory(*m_defaultsManager);
	m_editor = new ObjectEditor(*this, *m_factory);

	const bool isActive = GetActiveWindow()==this;

	// -------------------------------------------------------------------------

	m_browserDialog = new ObjectBrowserDialog(*m_editor, NULL, this);
	m_browserDialog->setAcceleratorTable(m_hAccelTable);
	m_browserDialog->ShowWindow(SW_SHOW);
	m_browserDialogOpen=true;


	// -------------------------------------------------------------------------

	m_propertiesDialog = new ObjectPropertiesDialog(*m_editor, "Selection Properties");
	m_propertiesDialog->Create(
		this, 
		-1,
		WS_EX_TOOLWINDOW
		);
	m_propertiesDialog->setAcceleratorTable(m_hAccelTable);
	m_propertiesDialog->ShowWindow(SW_SHOW);
	m_propertiesDialogOpen=true;

	// -------------------------------------------------------------------------

	_setActiveAppearance(isActive);
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_destroyEditingObjects()
{
	if (m_browserDialog)
	{
		m_browserDialog->DestroyWindow();
		m_browserDialogOpen=false;
		m_browserDialog=0;
	}

	if (m_propertiesDialog)
	{
		m_propertiesDialog->destroy();
		m_propertiesDialogOpen=false;
		m_propertiesDialog=0;
	}

	delete m_editor;
	m_editor=0;

	delete m_factory;
	m_factory=0;

	delete m_defaultsManager;
	m_defaultsManager=0;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_createNewWorkspace()
{
	// ------------------------------------------

	if (UIBuilderHistory::isInstalled())
	{
		UIBuilderHistory::remove();
	}
	UIBuilderHistory::install();

	// ------------------------------------------

	_unloadObjects();

	// ------------------------------------------

	UIPage *NewRoot = new UIPage;
	NewRoot->SetName( "root" );
	NewRoot->SetVisible( true );
	NewRoot->SetSize( UISize(800,600) );
	NewRoot->SetEnabled( true );
	UIManager::gUIManager().SetRootPage(NewRoot);

	_createEditingObjects();

	// TODO SizeWindowToCurrentPageSelection();

	m_fileName = "";
	_setMainWindowTitle();

	redrawViews(false);
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_openWorkspaceFile(const char *Filename)
{
	if (!Filename || !*Filename)
	{
		return false;
	}

	char res[_MAX_PATH+1] = "";

	if (!_closeWorkspaceFile())
	{
		return false;
	}

	strcpy( res, Filename );

	UIBuilderLoader             Loader;
	UIBaseObject::UIObjectList	 TopLevelObjects;

	char *pLastDelimitor = strrchr( res, '\\' );

	if( pLastDelimitor )
		*pLastDelimitor = '\0';

	SetCurrentDirectory( res );

	if( pLastDelimitor )
		*pLastDelimitor = '\\';

	UIBuilderLoader::s_fileTimes.clear ();

	if( !Loader.LoadFromResource( res, TopLevelObjects, true ) )
	{
		GetUIOutputStream()->flush ();
		MessageBox("The file could not be opened, check ui.log for more information", gApplicationName, MB_OK );
		return false;
	}

	if( TopLevelObjects.size() > 1 )
	{
		MessageBox("Error: The file contains more than one root level object", gApplicationName, MB_OK );
		return false;
	}
	
	UIBaseObject * const o = TopLevelObjects.front();

	UITextStyleManager::GetInstance()->Initialize(static_cast<UIPage *>(o), Loader);
	
	o->Link ();

	Loader.Lint ();

	if( o->IsA( TUIPage ) )
		UIManager::gUIManager().SetRootPage( static_cast< UIPage * >( o ) );
	else
	{
		MessageBox("Error: The root level object in the file is not a page", gApplicationName, MB_OK );
		return false;
	}

	UIBuilderHistory::install ();

//	if( Loader.ReadOnlyFilesInInput )
//		MessageBox( NULL, "Warning, one or more of the files making up this script are read only.", "UIBuilder", MB_OK );

	int version = 0;
	if (o->GetPropertyInteger (UIVersion::PropertyVersion, version) && version > UIVersion::ms_version)
	{		
		MessageBox("WARNING: the data you are editing is a newer version than this UIBuilder.", "UIBuilder", MB_OK | MB_ICONWARNING);
	}
	else
		o->SetPropertyInteger (UIVersion::PropertyVersion, UIVersion::ms_version);

	_createEditingObjects();
	
	m_fileName = Filename;
	theApp.AddToRecentFileList(Filename);
	_setMainWindowTitle();

	redrawViews(false);

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_saveWorkspaceFile(const char *fileName)
{
	if (!fileName || !*fileName)
	{
		return false;
	}

	char  res[_MAX_PATH+1];
	FILE *fp = 0;

	UIPage * const RootPage = UIManager::gUIManager().GetRootPage();

	if (RootPage)
	{
		int version = 0;
		if (  RootPage->GetPropertyInteger(UIVersion::PropertyVersion, version) 
			&& version > UIVersion::ms_version
			)
		{
			MessageBox("WARNING: the data you are editing is a newer version than this UIBuilder.", "UIBuilder", MB_OK  | MB_ICONWARNING);

			if (IDYES != MessageBox("Are you sure you want to overrite the newer data?", gApplicationName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING ))
			{
				return false;
			}
		}
	}

	strncpy( res, fileName, sizeof(res) );
	res[sizeof(res)-1] = '\0';

	if (RootPage)
	{
		//-- force packing
		RootPage->ForcePackChildren();

		UISaver			Saver;
		typedef std::map<UINarrowString, UINarrowString> NarrowStringMap;
		NarrowStringMap Output;

		RecursiveSetProperty( RootPage, UIBaseObject::PropertyName::SourceFile, m_fileName.c_str(), res );
		Saver.SaveToStringSet( Output, Output.end(), *RootPage );

		std::vector<std::string> messages;

		std::string TmpStr;
		UIBuilderLoader Loader;

		for (NarrowStringMap::iterator i = Output.begin(); i != Output.end();)
		{
			const std::string & FileName   = i->first.empty () ? res : i->first;
			const std::string & outputData = i->second;
			
			++i;

			{
				struct _stat statbuf;
				const int result = _stat (FileName.c_str (), &statbuf);
				if (!result)
				{
					const int t = statbuf.st_mtime;
					const int oldTime = UIBuilderLoader::s_fileTimes [FileName];
					if (t > oldTime)
					{
						char buf [1024];
						_snprintf (buf, sizeof (buf), "The file [%s] has changed on disk.", FileName.c_str ());
						MessageBox(buf, gApplicationName, MB_OK | MB_ICONWARNING);
						_snprintf (buf, sizeof (buf), "Are you sure you want to save [%s]?\nThis will almost certainly clobber someone else's changes.", FileName.c_str ());
						if (IDYES != MessageBox(buf, gApplicationName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING ))
						{
							continue;
						}
					}
				}
			}
			
			{
				TmpStr.clear ();
				Loader.LoadStringFromResource (FileName, TmpStr);
				
				//-- don't even attempt to write unmodified files
				if (TmpStr == outputData)
					continue;
			}

			FILE * fp = fopen( FileName.c_str (), "wb" );
			
			if( !fp )
			{
				messages.push_back (FileName);
			}
			else
			{
				fwrite( outputData.data (), outputData.size(), 1, fp );
				fclose( fp );
				
				struct _stat statbuf;
				const int result = _stat (FileName.c_str (), &statbuf);
				if (!result)
				{
					const int t = statbuf.st_mtime;
					UIBuilderLoader::s_fileTimes[FileName] = t;
				}
			}
		}

		if (!messages.empty ())
		{
			const std::string header ("Could not save objects to files:\n");
			int currentNumberOfFiles = 0;				

			std::string fileNames;

			for (size_t i = 0; i < messages.size ();)
			{
				const std::string & FileName = messages [i];

				++i;

				fileNames += FileName;
				fileNames.push_back ('\n');

				if (++currentNumberOfFiles >= 25 || i >= messages.size ())
				{
					const std::string msg = header + fileNames;
					MessageBox(msg.c_str (), gApplicationName, MB_OK ); 
					fileNames.clear ();
					currentNumberOfFiles = 0;
				}
			}
		}
	}	

	m_fileName = res;
	theApp.AddToRecentFileList(res);
	_setMainWindowTitle();	

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_closeWorkspaceFile()
{	
	if (UIManager::gUIManager().GetRootPage())
	{
		switch(MessageBox("Would you like to save your workspace before closing it?", gApplicationName, MB_YESNOCANCEL | MB_ICONWARNING ) )
		{
		case IDYES:
			OnFileSave();
			break;
			
		case IDNO:
			break;
			
		case IDCANCEL:
			return false;
		}
		
		UIBuilderHistory::remove();
	}
	
	_unloadObjects();
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_unloadObjects()
{
	_destroyEditingObjects();
	UIManager::gUIManager().SetRootPage(0);
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_getSaveFileName(CString &o_filename)
{
	CFileDialog openDialog(
		FALSE,
		gDefaultExtension,
		NULL,
		OFN_EXPLORER,
		gFileFilter,
		this
	);
	openDialog.m_ofn.hInstance			= GetModuleHandle(0);
	openDialog.m_ofn.lpstrInitialDir = ".";
	openDialog.m_ofn.lpstrTitle		= "Save As...";		

	if (openDialog.DoModal()==IDOK)
	{
		o_filename = openDialog.GetPathName();
		return true;
	}
	else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_setMainWindowTitle()
{
	char        buffer[_MAX_PATH+1];	
	const char *filename = 0;

	if (!m_fileName.empty())
	{
		filename = strrchr( m_fileName.c_str(), '\\' );
	}

	if (filename)
	{
		sprintf(buffer, "[%s]", filename + 1);
	}
	else
	{
		sprintf(buffer, "[Untitled]");
	}
		
	SetWindowText(buffer);
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_setActiveAppearance(bool i_showActive)
{
	// ----------------------------------------------------------------

	ObjectBrowserDialog *browser = _getObjectBrowserDialog();
	if (browser)
	{
		browser->setActiveAppearance(i_showActive);
	}

	// ----------------------------------------------------------------

	ObjectPropertiesEditor *properties = _getObjectPropertiesDialog();
	if (properties)
	{
		properties->setActiveAppearance(i_showActive);
	}

	// ----------------------------------------------------------------

	if (i_showActive!=m_showActive)
	{
		m_showActive=i_showActive;
		PostMessage(WM_NCACTIVATE, m_showActive);
	}
}

/////////////////////////////////////////////////////////////////////////////

ObjectBrowserDialog *CMainFrame::_getObjectBrowserDialog()
{
	if (!isEditing())
	{
		return 0;
	}
	return m_browserDialog;
}

/////////////////////////////////////////////////////////////////////////////

CMainFrame::PropertiesDialog *CMainFrame::_getObjectPropertiesDialog()
{
	if (!isEditing())
	{
		return 0;
	}
	return m_propertiesDialog;
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_objectBrowserIsFocus()
{
	ObjectBrowserDialog *browser = _getObjectBrowserDialog();

	for (CWnd *f = GetFocus();f;f = f->GetParent())
	{
		if (f==browser)
		{
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

bool CMainFrame::_objectPropertiesIsFocus()
{
	PropertiesDialog *properties = _getObjectPropertiesDialog();

	for (CWnd *f = GetFocus();f;f = f->GetParent())
	{
		if (f==properties)
		{
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_renderUI()
{
	if (!m_editor)
	{
		return;
	}

	UIManager &GUIManager = UIManager::gUIManager();

	if (!m_inVisualEditingMode)
	{
		GUIManager.SendHeartbeats();
	}

	UIPage *RootPage = GUIManager.GetRootPage();
	if (!RootPage)
	{
		return;
	}

	if (!gPrimaryDisplay->BeginRendering())
	{
		return;
	}

	UIPoint CanvasTranslation(0,0);
	
	gPrimaryDisplay->ClearTo(UIColor(0, 0, 0), UIRect(0, 0, gPrimaryDisplay->GetWidth(), gPrimaryDisplay->GetHeight()));
	gPrimaryDisplay->PushState();
	
	UIPage *currentlySelectedPage = m_editor->getCurrentlyVisiblePage();
	if (currentlySelectedPage)
	{
		CanvasTranslation = -currentlySelectedPage->GetLocation();
		gPrimaryDisplay->Translate(CanvasTranslation);
	}
	
	GUIManager.DrawCursor(gDrawCursor);
	GUIManager.Render(*gPrimaryDisplay);
	
	if (m_editor->getSnapToGrid())
	{
		int x;
		int y;
		int linecount;
		int width  = RootPage->GetWidth();
		int height = RootPage->GetHeight();
		
		float LineOpacity = gGridColor.a / 255.0f;
		float ThickLineOpacity = 2.0f * LineOpacity;
		
		gPrimaryDisplay->SetOpacity( LineOpacity );

		UIPoint gridSteps = m_editor->getGridSteps();
		
		for( x = 0, linecount = 0; x <= width; x += gridSteps.x, ++linecount )
		{
			if( (linecount % gGridMajorTicks) == 0 )
			{
				gPrimaryDisplay->SetOpacity( ThickLineOpacity );
				gPrimaryDisplay->ClearTo( gGridColor, UIRect( x, CanvasTranslation.y, x+1, height ) );
				gPrimaryDisplay->SetOpacity( LineOpacity );
			}
			else
				gPrimaryDisplay->ClearTo( gGridColor, UIRect( x, CanvasTranslation.y, x+1, height ) );
		}
		
		for( y = 0, linecount = 0; y <= height; y += gridSteps.y, ++linecount )
		{
			if( (linecount % gGridMajorTicks) == 0 )
			{
				gPrimaryDisplay->SetOpacity( ThickLineOpacity );
				gPrimaryDisplay->ClearTo( gGridColor, UIRect( CanvasTranslation.x, y, width, y+1 ) );
				gPrimaryDisplay->SetOpacity( LineOpacity );
			}
			else
				gPrimaryDisplay->ClearTo( gGridColor, UIRect( CanvasTranslation.x, y, width, y+1 ) );
		}
	}
	
	if (m_drawSelectionRect)
	{
		const UIObjectSet &sels = m_editor->getSelections();
		const UIObjectSet &hilt = m_editor->getHighlights();

		UIObjectSet::const_iterator osi;
		for (osi = sels.begin(); osi != sels.end(); ++osi)
		{
			UIBaseObject *const o = *osi;
			if (!o->IsA(TUIWidget))
			{
				continue;
			}
			const UIWidget * w = UI_ASOBJECT(UIWidget, o);

			UIRect WidgetRect;
			w->GetWorldRect(WidgetRect);

			const bool isSelectionHighlighted = hilt.contains(o);

			UIColor selectionFillColor    = m_selectionFillColor;
			UIColor selectionOutlineColor = (isSelectionHighlighted) ? m_highlightSelectionOutlineColor : m_selectionOutlineColor;

			// ------------------------------------------------------------------

			if (m_selectionFillColor.a>0)
			{
				gPrimaryDisplay->SetOpacity(selectionFillColor.a / 255.0f);
				gPrimaryDisplay->ClearTo(selectionFillColor, WidgetRect);
			}
			
			gPrimaryDisplay->SetOpacity(selectionOutlineColor.a / 255.0f);

			_drawBoxOutline(WidgetRect, selectionOutlineColor);

			// ------------------------------------------------------------------
		}

		for (osi = hilt.begin(); osi != hilt.end(); ++osi)
		{
			UIBaseObject *const o = *osi;
			if (!o->IsA(TUIWidget))
			{
				continue;
			}
			const UIWidget * w = UI_ASOBJECT(UIWidget, o);

			UIRect WidgetRect;
			w->GetWorldRect(WidgetRect);

			const bool isSelected = sels.contains(o);
			if (!isSelected)
			{
				gPrimaryDisplay->SetOpacity(m_highlightOutlineColor.a / 255.0f);
				_drawBoxOutline(WidgetRect, m_highlightOutlineColor);
			}
		}

		// ----------------------------------------------------
		UIRect selectionBox;
		if (m_editor->getSelectionBox(selectionBox))
		{
			gPrimaryDisplay->SetOpacity(m_selectionBoxOutlineColor.a / 255.0f);
			_drawBoxOutline(selectionBox, m_selectionBoxOutlineColor);
		}
		// ----------------------------------------------------

		UIRect selectionDragBox;
		if (m_editor->getSelectionDragBox(selectionDragBox))
		{
			const UIColor selectionDragBoxOutlineColor = m_selectionDragBoxOutlineColor;

			gPrimaryDisplay->SetOpacity(m_selectionDragBoxOutlineColor.a / 255.0f);

			_drawBoxOutline(selectionDragBox, m_selectionDragBoxOutlineColor);

			if (  m_inVisualEditingMode 
				&& selectionDragBox.Height() >= 8
				&& selectionDragBox.Width()  >= 8
				)
			{
				gPrimaryDisplay->ClearTo(
					selectionDragBoxOutlineColor, 
					UIRect(
						selectionDragBox.left + LineWidth,
						selectionDragBox.top  + LineWidth,
						selectionDragBox.left + HandleSize,
						selectionDragBox.top  + HandleSize
					) 
				);
				
				gPrimaryDisplay->ClearTo(
					selectionDragBoxOutlineColor,
					UIRect(
						selectionDragBox.right - HandleSize,
						selectionDragBox.top   + LineWidth,
						selectionDragBox.right - LineWidth,
						selectionDragBox.top   + HandleSize
					)
				);
				
				gPrimaryDisplay->ClearTo(
					selectionDragBoxOutlineColor,
					UIRect(
						selectionDragBox.left   + LineWidth,
						selectionDragBox.bottom - HandleSize,
						selectionDragBox.left   + HandleSize,
						selectionDragBox.bottom - LineWidth
					)
				);
				
				gPrimaryDisplay->ClearTo(
					selectionDragBoxOutlineColor,
					UIRect(
						selectionDragBox.right  - HandleSize,
						selectionDragBox.bottom - HandleSize,
						selectionDragBox.right  - LineWidth,
						selectionDragBox.bottom - LineWidth
					)
				);
				
				if (  selectionDragBox.Height()>=16 
					&& selectionDragBox.Width() >=16
					)
				{
					long HHandleLoc, VHandleLoc;
					
					HHandleLoc = (selectionDragBox.bottom + selectionDragBox.top - HandleSize) / 2;
					VHandleLoc = (selectionDragBox.right + selectionDragBox.left - HandleSize) / 2;
					
					gPrimaryDisplay->ClearTo(
						selectionDragBoxOutlineColor, 
						UIRect(
							selectionDragBox.left + LineWidth,
							HHandleLoc,
							selectionDragBox.left + HandleSize,
							HHandleLoc         + HandleSize
						)
					);
					
					gPrimaryDisplay->ClearTo(
						selectionDragBoxOutlineColor,
						UIRect(
							selectionDragBox.right - HandleSize,
							HHandleLoc,
							selectionDragBox.right - LineWidth,
							HHandleLoc          + HandleSize
						)
					);
					
					gPrimaryDisplay->ClearTo(
						selectionDragBoxOutlineColor,
						UIRect(
							VHandleLoc, 
							selectionDragBox.top + LineWidth,
							VHandleLoc        + HandleSize,
							selectionDragBox.top + HandleSize
						)
					);
					
					gPrimaryDisplay->ClearTo(
						selectionDragBoxOutlineColor,
						UIRect(
							VHandleLoc,
							selectionDragBox.bottom - HandleSize,
							VHandleLoc + HandleSize,
							selectionDragBox.bottom - LineWidth
						)
					);
				}
			}

		}
	}
	
	gPrimaryDisplay->PopState();
	gPrimaryDisplay->EndRendering();
	gPrimaryDisplay->Flip();
	
	gTriangleCount = gPrimaryDisplay->GetTriangleCount();
	gFlushCount = gPrimaryDisplay->GetFlushCount();
	
	++gFrameCount;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::_drawBoxOutline(UIRect box, UIColor color)
{
	gPrimaryDisplay->ClearTo(
		color,
		UIRect(
			box.left,
			box.top, 
			box.left + LineWidth, 
			box.bottom
		)
	);
	
	gPrimaryDisplay->ClearTo(
		color,
		UIRect(
			box.right - LineWidth,
			box.top,
			box.right,
			box.bottom
		)
	);
	
	gPrimaryDisplay->ClearTo(
		color,
		UIRect(
			box.left  + LineWidth,
			box.top,
			box.right - LineWidth,
			box.top   + LineWidth
		)
	);
	
	gPrimaryDisplay->ClearTo(
		color,
		UIRect(
			box.left   + LineWidth,
			box.bottom - LineWidth,
			box.right  - LineWidth,
			box.bottom
		)
	);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CMainFrame::OnDestroy() 
{
	if (m_refreshTimer)
	{
		KillTimer(m_refreshTimer);
		m_refreshTimer=0;
	}

	_destroyEditingObjects();

	if (gPrimaryDisplay)
	{
		// We better have closed all references to the display
//		if( gPrimaryDisplay->GetRefCount() != 1 )
//			MessageBox(0, "There is a handle leak: not all references to the primary display were closed.", gApplicationName, MB_OK );

		gPrimaryDisplay->Detach();
		gPrimaryDisplay = 0;
	}

	ShutdownCanvasSystem(m_wndView.m_hWnd);
	
	delete UIManager::gUIManager().GetScriptEngine();
	UIManager::gUIManager().SetScriptEngine(0);

	// --------------------------------------------

	SetCurrentDirectory( gInitialDirectory.c_str() );

	// --------------------------------------------

	CFrameWnd::OnDestroy();
}

void CMainFrame::OnFileNew() 
{
	_createNewWorkspace();
}

void CMainFrame::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}

LRESULT CMainFrame::OnPaintChild(WPARAM, LPARAM)
{
	_renderUI();

	return 0;
}

void CMainFrame::OnFileOpen() 
{
	CFileDialog openDialog(
		TRUE,
		gDefaultExtension,
		NULL,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		gFileFilter,
		this
	);
	openDialog.m_ofn.lpstrInitialDir = ".";
	openDialog.m_ofn.lpstrTitle		= "Select a User Interface Workspace to Open";

	if (openDialog.DoModal()==IDOK)
	{
		CString fileName = openDialog.GetPathName();
		_openWorkspaceFile(fileName);
	}
}

void CMainFrame::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}

void CMainFrame::OnFileClose() 
{
	_closeWorkspaceFile();
}

void CMainFrame::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	bool isLoaded = UIManager::gUIManager().GetRootPage()!=0;
	pCmdUI->Enable(isLoaded);
}

void CMainFrame::OnFileSave() 
{
	if (isEditing())
	{
		if (m_fileName.empty())
		{
			OnFileSaveas();
		}
		else
		{
			_saveWorkspaceFile(m_fileName.c_str());
		}
	}
}

void CMainFrame::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnFileSaveas() 
{
	if (isEditing())
	{
		CString fileName;

		if (_getSaveFileName(fileName))
		{
			_saveWorkspaceFile(fileName);
		}
	}
}

void CMainFrame::OnUpdateFileSaveas(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	if (gPrimaryDisplay)
	{
		gPrimaryDisplay->SetSize(UISize(cx, cy));
	}

	InvalidateRect(0, false);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	if (_closeWorkspaceFile())
	{
		char Filename[_MAX_PATH + 1];

		DragQueryFile(hDropInfo, 0, Filename, sizeof(Filename));
		_openWorkspaceFile(Filename);
	}
}

void CMainFrame::OnEditCopy() 
{
	if (m_editor)
	{
		m_editor->copySelectionsToClipboard(false);
	}
}

void CMainFrame::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	const bool canCopy=m_editor && m_editor->hasSelections();
	pCmdUI->Enable(canCopy);
}

void CMainFrame::OnEditCut() 
{
	if (m_editor)
	{
		m_editor->copySelectionsToClipboard(true);
	}
}

void CMainFrame::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	const bool canCopy=m_editor && m_editor->hasSelections();
	pCmdUI->Enable(canCopy);
}

void CMainFrame::OnEditPaste() 
{
	if (m_editor)
	{
		m_editor->pasteFromClipboard();
	}
}

void CMainFrame::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	const bool canPaste=m_editor && m_clipboardFormat && IsClipboardFormatAvailable(m_clipboardFormat)!=0;
	pCmdUI->Enable(canPaste);
}

void CMainFrame::OnInsertObject(UINT nId)
{
	if (isEditing())
	{
		m_editor->insertNewObject(nId);
	}
}

void CMainFrame::OnUpdateInsertObject(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnViewDefaultProperties() 
{
	// TODO: Add your command handler code here
	DefaultObjectPropertiesDialog dlg(*m_defaultsManager);
	dlg.DoModal();
}

void CMainFrame::OnUpdateViewDefaultProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_defaultsManager!=0);
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	if (  nIDEvent==m_refreshTimer
		&& isEditing()
		&& IsWindowVisible()
		&& !IsIconic()
		)
	{
		redrawViews(false);
	}
	
	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnClose() 
{
	if (m_editor)
	{
		if (m_browserDialog)
		{
			m_browserDialog->saveUserPreferences();
			m_browserDialog->DestroyWindow();
			m_browserDialog=0;
			m_browserDialogOpen=false;
		}

		if (m_propertiesDialog)
		{
			m_propertiesDialog->saveUserPreferences();
			m_propertiesDialog->DestroyWindow();
			m_propertiesDialog=0;
			m_propertiesDialogOpen=false;
		}
	}
	
	CFrameWnd::OnClose();
}

void CMainFrame::OnSelectionBurrow() 
{
	if (m_editor)
	{
		m_editor->burrow();
	}
}

void CMainFrame::OnUpdateSelectionBurrow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnSelectionClearall() 
{
	if (m_editor)
	{
		m_editor->clearSelections();
	}
}

void CMainFrame::OnUpdateSelectionClearall(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnSelectionDescendants() 
{
	if (m_editor)
	{
		m_editor->selectDescendants();
	}
}

void CMainFrame::OnUpdateSelectionDescendants(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnSelectionAncestors() 
{
	if (m_editor)
	{
		m_editor->selectAncestors();
	}
}

void CMainFrame::OnUpdateSelectionAncestors(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnEditCanceldrag() 
{
	if (m_editor)
	{
		m_editor->cancelDrag();
	}
}

void CMainFrame::OnUpdateEditCanceldrag(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
}

void CMainFrame::OnSelectionDelete() 
{
	if (m_editor)
	{
		UIBaseObject *nextSelection=0;
		if (  m_editor->getSelectionCount()==1
			&& _objectBrowserIsFocus()
			)
		{
			UIBaseObject *sel = m_editor->getSelections().front();
			UIBaseObject *selParent = sel->GetParent();
			if (selParent)
			{
				UIBaseObject::UIObjectList children;
				EditUtils::getChildren(children, *selParent);
				for (UIBaseObject::UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
				{
					if (*ci == sel)
					{
						UIBaseObject::UIObjectList::iterator ciNext=ci;
						++ciNext;
						if (ciNext!=children.end())
						{
							nextSelection=*ciNext;
						}
						else if (ci!=children.begin())
						{
							--ci;
							nextSelection=*ci;
						}
						else
						{
							nextSelection=selParent;
						}
						break;
					}
				}
			}
			assert(nextSelection!=sel);
		}

		if (m_editor->deleteSelections())
		{
			if (nextSelection)
			{
				m_editor->select(*nextSelection);
			}
		}
	}
}

void CMainFrame::OnUpdateSelectionDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->hasSelections());
}

void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CFrameWnd::OnActivateApp(bActive, hTask);

	_setActiveAppearance(bActive!=0);

	if (!bActive)
	{
		if (m_editor)
		{
			m_editor->onDeactivate();
		}
	}
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_NCACTIVATE:
			wParam=m_showActive;
			break;
	}
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

void CMainFrame::OnToggleGrid() 
{
	if (isEditing())
	{
		m_editor->setSnapToGrid(!m_editor->getSnapToGrid());
	}
}

void CMainFrame::OnUpdateToggleGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing());
	if (isEditing())
	{
		pCmdUI->SetCheck(m_editor->getSnapToGrid() ? 1 : 0);
	}
	else
	{
		pCmdUI->SetCheck(2);
	}
}

void CMainFrame::OnSelectionAlignbottom() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_BOTTOM);
	}
}

void CMainFrame::OnUpdateSelectionAlignbottom(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
	
}

void CMainFrame::OnSelectionAlignleft() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_LEFT);
	}
}

void CMainFrame::OnUpdateSelectionAlignleft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAlignright() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_RIGHT);
	}
}

void CMainFrame::OnUpdateSelectionAlignright(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAligntop() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_TOP);
	}
}

void CMainFrame::OnUpdateSelectionAligntop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAlignwidth() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_WIDTH);
	}
}

void CMainFrame::OnUpdateSelectionAlignwidth(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAlignheight() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_HEIGHT);
	}
}

void CMainFrame::OnUpdateSelectionAlignheight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAligncenterx() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_CENTERX);
	}
}

void CMainFrame::OnUpdateSelectionAligncenterx(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnSelectionAligncentery() 
{
	if (m_editor)
	{
		m_editor->alignSelections(ObjectEditor::AD_CENTERY);
	}
}

void CMainFrame::OnUpdateSelectionAligncentery(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>1);
}

void CMainFrame::OnEditUndo() 
{
	if (m_editor)
	{
		m_editor->undo();
	}
}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	const char *undoName=0;
	if (m_editor)
	{
		undoName = m_editor->getCurrentUndoOperation();
	}

	if (undoName)
	{
		char temp[256];
		sprintf(temp, "Undo %s \tCtrl-Z", undoName);

		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(temp);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Undo \tCtrl-Z");
	}
}

void CMainFrame::OnEditRedo() 
{
	if (m_editor)
	{
		m_editor->redo();
	}
}

void CMainFrame::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	const char *redoName=0;
	if (m_editor)
	{
		redoName = m_editor->getCurrentRedoOperation();
	}

	if (redoName)
	{
		char temp[256];
		sprintf(temp, "Redo %s \tCtrl-Y", redoName);

		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(temp);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Redo \tCtrl-Y");
	}
}

void CMainFrame::OnCheckout() 
{
	if (!m_editor)
	{
		return;
	}

	std::set<std::string> fileNames;

	const UIObjectSet &sels = m_editor->getSelections();
	for (UIObjectSet::const_iterator oi = sels.begin();oi != sels.end(); ++oi)
	{
		UIBaseObject *const o = *oi;

		UIString sourcePath;
		if (UIManager::gUIManager().GetRootPage() == o) 
		{
			sourcePath = Unicode::narrowToWide("ui_root.ui");
		}
		else
		{
			o->GetProperty(UIBaseObject::PropertyName::SourceFile, sourcePath);
		}
		
		fileNames.insert(Unicode::wideToNarrow(sourcePath));
	}

	std::set<std::string>::iterator fi;
	for (fi=fileNames.begin();fi!=fileNames.end();++fi)
	{
		const std::string &fileName = *fi;;

		char command[1024];
		sprintf(command, "p4 edit %s\n\n", fileName.c_str());
		system(command);
	}
}

void CMainFrame::OnUpdateCheckout(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(isEditing() && m_editor->getSelectionCount()>0);
}

void CMainFrame::OnViewObjectbrowser() 
{
	if (m_browserDialog)
	{
		if (m_browserDialogOpen)
		{
			m_browserDialog->ShowWindow(SW_HIDE);
			m_browserDialogOpen=false;
		}
		else
		{
			m_browserDialog->ShowWindow(SW_SHOW);
			m_browserDialogOpen=true;
		}
	}
}

void CMainFrame::OnUpdateViewObjectbrowser(CCmdUI* pCmdUI) 
{
	if (m_browserDialog)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_browserDialogOpen ? 1 : 0);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnViewSelectionproperties() 
{
	if (m_propertiesDialog)
	{
		if (m_propertiesDialogOpen)
		{
			m_propertiesDialog->ShowWindow(SW_HIDE);
			m_propertiesDialogOpen=false;
		}
		else
		{
			m_propertiesDialog->ShowWindow(SW_SHOW);
			m_propertiesDialogOpen=true;
		}
	}
}

void CMainFrame::OnUpdateViewSelectionproperties(CCmdUI* pCmdUI) 
{
	if (m_propertiesDialog)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_propertiesDialogOpen ? 1 : 0);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(0);
	}
}
