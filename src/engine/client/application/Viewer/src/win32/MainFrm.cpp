// MainFrm.cpp : implementation of the CMainFrame class
//

#include "FirstViewer.h"
#include "viewer.h"

#include "DialogFind.h"
#include "MainFrm.h"
#include "ViewerDoc.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "sharedObject/Object.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_DIRECTORYVIEW, OnViewDirectoryview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DIRECTORYVIEW, OnUpdateViewDirectoryview)
	ON_COMMAND(ID_VIEW_OUTPUTWINDOW, OnViewOutputwindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW, OnUpdateViewOutputwindow)
	ON_COMMAND(ID_BUTTON_CHECK_DUPES, OnButtonCheckDupes)
	ON_COMMAND(ID_VIEW_ANIMATIONEDITOR, OnViewAnimationeditor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATIONEDITOR, OnUpdateViewAnimationeditor)
	ON_COMMAND(ID_BUTTON_SHOWMIPMAPLEVELS, OnButtonShowmipmaplevels)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOWMIPMAPLEVELS, OnUpdateButtonShowmipmaplevels)
	ON_COMMAND(ID_BUTTON_FIND, OnButtonFind)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FIND, OnUpdateButtonFind)
	ON_COMMAND(ID_BUTTON_OPENALL, OnButtonOpenall)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_OPENALL, OnUpdateButtonOpenall)
	ON_COMMAND(ID_VIEW_GOT_DOT3, OnViewGotDot3)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GOT_DOT3, OnUpdateViewGotDot3)
	ON_COMMAND(ID_VIEW_GOT_ZHAK, OnViewGotZhak)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GOT_ZHAK, OnUpdateViewGotZhak)
	ON_COMMAND(ID_BUTTON_WRITE_CUSTOMIZATION_DATA, OnButtonWriteCustomizationData)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_WRITE_CUSTOMIZATION_DATA, OnUpdateButtonWriteCustomizationData)
	ON_COMMAND(ID_BUTTON_OPENCLOSEALL, OnButtonOpencloseall)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_OPENCLOSEALL, OnUpdateButtonOpencloseall)
	ON_COMMAND(ID_BUTTON_DEBUGDUMPALL, OnButtonDebugdumpall)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DEBUGDUMPALL, OnUpdateButtonDebugdumpall)
	ON_COMMAND(ID_BUTTON_BUILDASYNCLOADERDATA, OnButtonBuildasyncloaderdata)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_BUILDASYNCLOADERDATA, OnUpdateButtonBuildasyncloaderdata)
	ON_COMMAND(ID_BUTTON_FIND_LMGS, OnButtonFindLmgs)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FIND_LMGS, OnUpdateButtonFindLmgs)
	ON_COMMAND(ID_BUTTON_FIX_EXPORTED_SAT_FILES, OnButtonFixExportedSatFiles)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FIX_EXPORTED_SAT_FILES, OnUpdateButtonFixExportedSatFiles)
	ON_COMMAND(ID_VIEW_CLEAROUTPUT, OnViewClearoutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HARDPOINT_TREE, OnUpdateViewHardpointTree)
	ON_COMMAND(ID_VIEW_HARDPOINT_TREE, OnViewHardpointTree)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATION_TREE, OnUpdateViewAnimationTree)
	ON_COMMAND(ID_VIEW_ANIMATION_TREE, OnViewAnimationTree)
	ON_COMMAND(ID_HARDPOINT_ATTACH, OnHardpointAttach)
	ON_UPDATE_COMMAND_UI(ID_HARDPOINT_ATTACH, OnUpdateHardpointAttach)
	ON_COMMAND(ID_HARDPOINT_ATTACH_SELECTED, OnHardpointAttachSelected)
	ON_UPDATE_COMMAND_UI(ID_HARDPOINT_ATTACH_SELECTED, OnUpdateHardpointAttachSelected)
	ON_COMMAND(ID_REMOVE_ATTACHED_OBJECT, OnRemoveAttachedObject)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

const Tag TAG_DOT3 = TAG(D,O,T,3);
const Tag TAG_ZHAK = TAG(Z,H,A,K);

// ======================================================================

namespace CMainFrameNamespace
{
	Appearance const * getAppearance(const CView *view)
	{
		//-- get the appearance
		const Appearance *appearance = 0;
		
		if (view)
		{
			const CViewerDoc *viewerDoc = dynamic_cast<CViewerDoc*>(view->GetDocument());
			if (viewerDoc)
			{
				const Object *const object = viewerDoc->getObject();
				if (object)
					appearance = object->getAppearance();
			}
		}
		
		return appearance;
	}
	
	Object const * getObject(const CView *view)
	{
		//-- get the object
		const Object *object = 0;
		
		if (view)
		{
			const CViewerDoc *viewerDoc = dynamic_cast<CViewerDoc*>(view->GetDocument());
			if (viewerDoc)
			{
				object = viewerDoc->getObject();
			}
		}
		
		return object;
	}
}

using namespace CMainFrameNamespace;

// ======================================================================

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	currentView = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarObject.CreateEx(this) ||
		!m_wndToolBarObject.LoadToolBar(IDR_OBJECT))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarFix.CreateEx(this) ||
		!m_wndToolBarFix.LoadToolBar(IDR_FIXBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndToolBarObject) ||
		!m_wndReBar.AddBar(&m_wndToolBarFix))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndToolBarObject.SetBarStyle(m_wndToolBarObject.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndToolBarFix.SetBarStyle(m_wndToolBarFix.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);

	EnableDocking(CBRS_ALIGN_ANY);

	CString temp ("Directory Tree");
	if (!directoryDialogBar.Create(this, &directoryDialog, temp, IDD_DIRECTORY_TREE))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

    directoryDialogBar.SetBarStyle(directoryDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	directoryDialogBar.EnableDocking(CBRS_ALIGN_ANY);
	ShowControlBar(&directoryDialogBar, FALSE, FALSE);
	DockControlBar(&directoryDialogBar);

	temp = "Animation";
	if (!animationDialogBar.Create(this, &animationDialog, temp, IDD_ANIMATION))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

    animationDialogBar.SetBarStyle(animationDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	animationDialogBar.EnableDocking(CBRS_ALIGN_ANY);
	ShowControlBar(&animationDialogBar, FALSE, FALSE);
	DockControlBar(&animationDialogBar, AFX_IDW_DOCKBAR_BOTTOM);

	temp = "Output";
	if (!outputDialogBar.Create(this, &outputDialog, temp, IDD_OUTPUT))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

    outputDialogBar.SetBarStyle(outputDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	outputDialogBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	ShowControlBar(&outputDialogBar, FALSE, FALSE);
	DockControlBar(&outputDialogBar, AFX_IDW_DOCKBAR_BOTTOM);

	temp = "Hardpoint";
	if (!hardpointDialogBar.Create(this, &hardpointDialog, temp, IDD_HARDPOINT_TREE))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

    hardpointDialogBar.SetBarStyle(hardpointDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	hardpointDialogBar.EnableDocking(CBRS_ALIGN_RIGHT | CBRS_ALIGN_TOP);
	ShowControlBar(&hardpointDialogBar, FALSE, FALSE);
	DockControlBar(&hardpointDialogBar, AFX_IDW_DOCKBAR_RIGHT);

	temp = "AnimationTree";
	if(!m_animationTreeDialogBar.Create(this,&m_animationTreeDialog, temp, IDD_ANIMATION_TREE))
	{
		TRACE0("Failed to create animation tree dialogbar\n");
		return -1;
	}
	m_animationTreeDialogBar.SetBarStyle(m_animationTreeDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	m_animationTreeDialogBar.EnableDocking(CBRS_ALIGN_RIGHT | CBRS_ALIGN_TOP);
	ShowControlBar(&m_animationTreeDialogBar, FALSE, FALSE);
	DockControlBar(&m_animationTreeDialogBar, AFX_IDW_DOCKBAR_RIGHT);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CMDIFrameWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

//-------------------------------------------------------------------

void CMainFrame::OnViewDirectoryview()
{
	// TODO: Add your command handler code here
	if(directoryDialogBar.IsVisible())
	{
		ShowControlBar(&directoryDialogBar, FALSE, FALSE);
	}
	else
	{
		directoryDialog.reset ();
		ShowControlBar(&directoryDialogBar, TRUE, FALSE);
	}
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateViewDirectoryview(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(directoryDialogBar.IsVisible())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//-------------------------------------------------------------------

void CMainFrame::OnViewOutputwindow()
{
	// TODO: Add your command handler code here
	if(outputDialogBar.IsVisible())
	{
		ShowControlBar(&outputDialogBar, FALSE, FALSE);
	}
	else
	{
		ShowControlBar(&outputDialogBar, TRUE, FALSE);
	}
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateViewOutputwindow(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(outputDialogBar.IsVisible())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}


//-------------------------------------------------------------------

void CMainFrame::updateStatusBar(const char* message)
{
	m_wndStatusBar.SetPaneText(0, message);
}

//-------------------------------------------------------------------

void CMainFrame::clearMessage (void)
{
	outputDialog.clearMessage ();
}

//-------------------------------------------------------------------

void CMainFrame::addMessage (const char* message)
{
	outputDialog.addMessage (message);
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonCheckDupes()
{
	// TODO: Add your command handler code here
	if(!outputDialogBar.IsVisible())
		ShowControlBar(&outputDialogBar, TRUE, FALSE);

	directoryDialog.OnButtonCheckDupes ();
}

//-------------------------------------------------------------------

void CMainFrame::OnViewAnimationeditor()
{
	// TODO: Add your command handler code here
	if(animationDialogBar.IsVisible())
	{
		ShowControlBar(&animationDialogBar, FALSE, FALSE);
	}
	else
	{
		ShowControlBar(&animationDialogBar, TRUE, FALSE);
	}
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateViewAnimationeditor(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (animationDialogBar.IsVisible ())
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

//-------------------------------------------------------------------

void CMainFrame::updateAnimationView (const CView* newView, const SkeletalAnimationKey::Map& skeletalAnimationKeyMap, const bool force)
{
	if (!newView || currentView != newView || force)
	{
		currentView = newView;
		animationDialog.updateAnimationView (getAppearance(newView), skeletalAnimationKeyMap);
	}
}

//-------------------------------------------------------------------

void CMainFrame::updateAnimationView (const CView* newView, const bool force)
{
	if (!newView || currentView != newView || force)
	{
		currentView = newView;
		animationDialog.updateAnimationView (getAppearance(newView));
	}
}

//-------------------------------------------------------------------

void CMainFrame::updateHardpointTree(CView const * const view)
{
	if (view)
		hardpointDialog.updateHardpointTree(getObject(view));
	else
		hardpointDialog.updateHardpointTree(0);
}

//-------------------------------------------------------------------

void CMainFrame::showOutputView (void)
{
	if(!outputDialogBar.IsVisible())
		ShowControlBar(&outputDialogBar, TRUE, FALSE);
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonShowmipmaplevels()
{
#ifdef _DEBUG
	const bool newValue = !Graphics::getShowMipmapLevels();
	Graphics::showMipmapLevels(newValue);
#endif
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonShowmipmaplevels(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
#ifdef _DEBUG

	pCmdUI->Enable (true);
	pCmdUI->SetCheck (Graphics::getShowMipmapLevels());

#else

	pCmdUI->Enable (false);

#endif
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonFind()
{
	DialogFind dlg;
	if (dlg.DoModal () == IDOK)
		directoryDialog.OnButtonFind(dlg.m_name);
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonFind(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonOpenall()
{
	directoryDialog.OnButtonOpenAll(false);
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonOpenall(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonOpencloseall()
{
	directoryDialog.OnButtonOpenAll(true);
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonOpencloseall(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonWriteCustomizationData()
{
	directoryDialog.OnButtonWriteCustomizationData();
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonWriteCustomizationData(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

//===================================================================

void CMainFrame::OnViewGotDot3()
{
	GraphicsOptionTags::toggle(TAG_DOT3);
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateViewGotDot3(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (GraphicsOptionTags::get(TAG_DOT3));
}

//-------------------------------------------------------------------

void CMainFrame::OnViewGotZhak()
{
	GraphicsOptionTags::toggle(TAG_ZHAK);
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateViewGotZhak(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (GraphicsOptionTags::get(TAG_ZHAK));
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonDebugdumpall()
{
	directoryDialog.OnButtonDebugDumpAll();
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonDebugdumpall(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

//-------------------------------------------------------------------

void CMainFrame::OnButtonBuildasyncloaderdata()
{
	directoryDialog.OnButtonBuildAsynchronousLoaderData();
}

//-------------------------------------------------------------------

void CMainFrame::OnUpdateButtonBuildasyncloaderdata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

// ----------------------------------------------------------------------

void CMainFrame::OnButtonFindLmgs()
{
	directoryDialog.OnButtonFindLmgs();
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateButtonFindLmgs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

// ----------------------------------------------------------------------

void CMainFrame::OnButtonFixExportedSatFiles()
{
	directoryDialog.OnButtonFixExportedSatFiles();
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateButtonFixExportedSatFiles(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (directoryDialogBar.IsVisible());
}

// ----------------------------------------------------------------------

void CMainFrame::OnViewClearoutput()
{
	clearMessage ();
}

// ----------------------------------------------------------------------

void CMainFrame::OnViewHardpointTree() 
{
	if (hardpointDialogBar.IsVisible())
	{
		ShowControlBar(&hardpointDialogBar, FALSE, FALSE);
	}
	else
	{
		ShowControlBar(&hardpointDialogBar, TRUE, FALSE);
	}
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateViewHardpointTree(CCmdUI* pCmdUI) 
{
	if (hardpointDialogBar.IsVisible ())
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

// ----------------------------------------------------------------------

void CMainFrame::OnHardpointAttach() 
{
	CString hardpoint = hardpointDialog.getSelectedHardpoint();

	if (hardpoint.IsEmpty())
		return;

	CFileDialog fileDialog(TRUE, "*.apt", 0, OFN_FILEMUSTEXIST, "Appearance Template *.apt|*.apt||");

	if (fileDialog.DoModal() == IDOK)
	{
		Object * parent = hardpointDialog.getParentObject();

		CViewerDoc * const viewerDoc = getViewerDoc();
		CFrameWnd * const childFrame =  GetActiveFrame();

		if (viewerDoc && childFrame)
		{
			viewerDoc->attachStandardAppearanceToHardpoint(parent, fileDialog.GetPathName(), hardpoint);
			updateHardpointTree(childFrame->GetActiveView());
		}
	}
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateHardpointAttach(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

// ----------------------------------------------------------------------

void CMainFrame::OnViewAnimationTree()
{
	if (m_animationTreeDialogBar.IsVisible())
	{
		ShowControlBar(&m_animationTreeDialogBar, FALSE, FALSE);
	}
	else
	{
		ShowControlBar(&m_animationTreeDialogBar, TRUE, TRUE);
	}
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateViewAnimationTree(CCmdUI* pCmdUI) 
{
	if (m_animationTreeDialogBar.IsVisible ())
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

// ----------------------------------------------------------------------

CViewerDoc * CMainFrame::getViewerDoc()
{
	CFrameWnd * const child =  GetActiveFrame();

	CViewerDoc * const viewerDoc = child ? dynamic_cast<CViewerDoc *>(child->GetActiveDocument()) : 0;

	WARNING(!viewerDoc, ("unable to find active document!"));

	return viewerDoc;
}

// ----------------------------------------------------------------------

void CMainFrame::OnHardpointAttachSelected() 
{
	CString filename = directoryDialog.getSelectedFilename();
	CString hardpoint = hardpointDialog.getSelectedHardpoint();

	if (filename.IsEmpty() || hardpoint.IsEmpty())
		return;

	Object * parent = hardpointDialog.getParentObject();

	DEBUG_REPORT_LOG(true, ("OnHardpointAttachSelected() [%s] [%s]\n", (char const *)filename, (char const *)hardpoint));

	// load the selected appearance and attach it to each checked hardpoint
	CViewerDoc * const viewerDoc = getViewerDoc();
	CFrameWnd * const childFrame =  GetActiveFrame();

	if (viewerDoc && childFrame)
	{
		viewerDoc->attachStandardAppearanceToHardpoint(parent, filename, hardpoint);
		updateHardpointTree(childFrame->GetActiveView());
	}
}

// ----------------------------------------------------------------------

void CMainFrame::OnUpdateHardpointAttachSelected(CCmdUI* pCmdUI) 
{
	UNREF(pCmdUI);
}

// ----------------------------------------------------------------------

void CMainFrame::OnRemoveAttachedObject() 
{
	CString hardpoint = hardpointDialog.getSelectedHardpoint();

	if (hardpoint.IsEmpty())
		return;

	Object * parent = hardpointDialog.getParentObject();

	CViewerDoc * const viewerDoc = getViewerDoc();
	CFrameWnd * const childFrame =  GetActiveFrame();

	if (viewerDoc && childFrame)
	{
		viewerDoc->removeAttachedObject(parent);
		updateHardpointTree(childFrame->GetActiveView());
	}
}

// ======================================================================
