//
// MainFrame.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "MainFrame.h"

#include "TerrainEditorDoc.h"
#include "DialogControlPointEditor.h"
#include "DialogPalette.h"
#include "TerrainEditor.h"
#include "Splash.h"
#include "sharedRandom/Random.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
	ON_WM_INITMENU()
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_2D_MAP_POSITION, OnUpdate2dMapPosition)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ZOOM_LEVEL, OnUpdateZoomLevel)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SHADER, OnUpdateShaderFamilyName)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FLORASTATICCOLLIDABLE, OnUpdateFloraStaticCollidableFamilyName)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FLORASTATICNONCOLLIDABLE, OnUpdateFloraStaticNonCollidableFamilyName)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FLORADYNAMICNEAR, OnUpdateFloraDynamicNearFamilyName)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FLORADYNAMICFAR, OnUpdateFloraDynamicFarFamilyName)
	ON_COMMAND(ID_VIEW_CATALOG, OnViewCatalog)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CATALOG, OnUpdateViewCatalog)
	ON_COMMAND(ID_DEBUG_TEST, OnDebugTest)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_TEST, OnUpdateDebugTest)
	ON_COMMAND(ID_DEBUG_TESTPOINTEDITOR, OnDebugTestpointeditor)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_TESTPOINTEDITOR, OnUpdateDebugTestpointeditor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_2D_MAP_POSITION,
	ID_INDICATOR_ZOOM_LEVEL,
	ID_INDICATOR_SHADER,
	ID_INDICATOR_FLORASTATICCOLLIDABLE,
	ID_INDICATOR_FLORASTATICNONCOLLIDABLE,
	ID_INDICATOR_FLORADYNAMICNEAR,
	ID_INDICATOR_FLORADYNAMICFAR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

//-------------------------------------------------------------------

MainFrame::MainFrame (void) :
	CMDIFrameWnd (),
	m_wndStatusBar (),
	m_wndToolBar (),
	m_wndReBar (),
	catalogDialog (),
	catalogDialogBar (),
	mousePosition_w (),
	zoomLevel (1),
	shaderFamilyName (),
	floraStaticCollidableFamilyName (),
	floraStaticNonCollidableFamilyName (),
	floraDynamicNearFamilyName (),
	floraDynamicFarFamilyName ()
{
	mousePosition_w.makeZero ();
}

//-------------------------------------------------------------------
	
MainFrame::~MainFrame (void)
{
}

//-------------------------------------------------------------------

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// CG: The following line was added by the Splash Screen component.
	SplashScreen::ShowSplashScreen(this);

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

//	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
//		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
//	{
//		TRACE0("Failed to create dialogbar\n");
//		return -1;		// fail to create
//	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar))
//		!m_wndReBar.AddBar(&m_wndDlgBar))
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

	m_wndStatusBar.SetPaneInfo (1, ID_INDICATOR_2D_MAP_POSITION, SBPS_NORMAL, 300);

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	EnableDocking(CBRS_ALIGN_ANY); 

	CString temp ("Groups Catalog");
	if (!catalogDialogBar.Create(this, &catalogDialog, temp, IDD_DIALOG_CATALOG))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

    catalogDialogBar.SetBarStyle(catalogDialogBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	catalogDialogBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	ShowControlBar(&catalogDialogBar, FALSE, FALSE);
	DockControlBar(&catalogDialogBar, AFX_IDW_DOCKBAR_LEFT);

	return 0;
}

//-------------------------------------------------------------------

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	//-- start out maximized
	CRect rect;
	if (SystemParametersInfo (SPI_GETWORKAREA, 0, &rect, 0))
	{
		cs.x  = 0;
		cs.y  = 0;
		cs.cx = rect.Width ();
		cs.cy = rect.Height ();
	}

	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

//-------------------------------------------------------------------

void MainFrame::OnUpdate2dMapPosition(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable (true); 

	CString info;

	info.Format ("<%f, %f, %f>",  (mousePosition_w.x),  (mousePosition_w.y),  (mousePosition_w.z)); 

	pCmdUI->SetText (info);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateZoomLevel (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 

	CString info;

	if (zoomLevel != CONST_REAL (0))
		info.Format ("%i%%", static_cast<int> (CONST_REAL (100) * zoomLevel)); 

	pCmdUI->SetText (info);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateShaderFamilyName (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 
	pCmdUI->SetText (shaderFamilyName);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateFloraStaticCollidableFamilyName (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 
	pCmdUI->SetText (floraStaticCollidableFamilyName);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateFloraStaticNonCollidableFamilyName (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 
	pCmdUI->SetText (floraStaticNonCollidableFamilyName);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateFloraDynamicNearFamilyName (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 
	pCmdUI->SetText (floraDynamicNearFamilyName);
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateFloraDynamicFarFamilyName (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (true); 
	pCmdUI->SetText (floraDynamicFarFamilyName);
}

//-------------------------------------------------------------------

void MainFrame::OnInitMenu(CMenu* pMenu)
{
   CMDIFrameWnd::OnInitMenu(pMenu);
  
	// CG: This block added by 'Tip of the Day' component.
	{
		// TODO: This code adds the "Tip of the Day" menu item
		// on the fly.  It may be removed after adding the menu
		// item to all applicable menu items using the resource
		// editor.

		// Add Tip of the Day menu item on the fly!
		static CMenu* pSubMenu = NULL;

		CString strHelp; 
		IGNORE_RETURN (strHelp.LoadString(CG_IDS_TIPOFTHEDAYHELP));
		CString strMenu;
		int nMenuCount = pMenu->GetMenuItemCount();
		BOOL bFound = FALSE;
		for (int i=0; i < nMenuCount; i++) 
		{
			pMenu->GetMenuString(i, strMenu, MF_BYPOSITION);
			if (strMenu == strHelp)
			{ 
				pSubMenu = pMenu->GetSubMenu(i);
				bFound = TRUE;
				ASSERT(pSubMenu != NULL);
			}
		}

		CString strTipMenu;
		strTipMenu.LoadString(CG_IDS_TIPOFTHEDAYMENU);
		if (!bFound)
		{
			// Help menu is not available. Please add it!
			if (pSubMenu == NULL) 
			{
				// The same pop-up menu is shared between mainfrm and frame 
				// with the doc.
				static CMenu popUpMenu;
				pSubMenu = &popUpMenu;
				pSubMenu->CreatePopupMenu();
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			} 
			pMenu->AppendMenu(MF_STRING|MF_BYPOSITION|MF_ENABLED|MF_POPUP, 
				(UINT)pSubMenu->m_hMenu, strHelp);
			DrawMenuBar();
		} 
		else
		{      
			// Check to see if the Tip of the Day menu has already been added.
			pSubMenu->GetMenuString(0, strMenu, MF_BYPOSITION);

			if (strMenu != strTipMenu) 
			{
				// Tip of the Day submenu has not been added to the 
				// first position, so add it.
				pSubMenu->InsertMenu(0, MF_BYPOSITION);  // Separator
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			}
		}
	}

}

//-------------------------------------------------------------------

void MainFrame::OnViewCatalog() 
{
	// TODO: Add your command handler code here
	if(catalogDialogBar.IsVisible())
	{
		ShowControlBar(&catalogDialogBar, FALSE, FALSE);
	}
	else
	{
		catalogDialog.reset ();
		ShowControlBar(&catalogDialogBar, TRUE, FALSE);
	}
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateViewCatalog(CCmdUI* pCmdUI) 
{
	if (catalogDialogBar.IsVisible ())
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

//-------------------------------------------------------------------

void MainFrame::OnDebugTest() 
{
	CDialogPalette dlg;
	dlg.DoModal();
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateDebugTest(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------

void MainFrame::OnDebugTestpointeditor() 
{
	CONSOLE_PRINT ("-----------------\r\n");

	ArrayList<Vector2d> pointList;
	int i;
	for (i = 0; i < Random::random (5, 10); ++i)
	{
		pointList.add (Vector2d (Random::randomReal (-10.f, 10.f), Random::randomReal (-10.f, 10.f)));

		CString temp;
		temp.Format ("%1.2f, %1.2f\r\n", pointList [i].x, pointList [i].y);
		CONSOLE_PRINT (temp);
	}

	CONSOLE_PRINT ("-----------------\r\n");

	DialogControlPointEditor dlg (pointList);
	dlg.DoModal ();

	pointList = dlg.getPointList ();

	for (i = 0; i < pointList.size (); ++i)
	{
		CString temp;
		temp.Format ("%1.2f, %1.2f\r\n", pointList [i].x, pointList [i].y);
		CONSOLE_PRINT (temp);
	}

	CONSOLE_PRINT ("-----------------\r\n");
}

//-------------------------------------------------------------------

void MainFrame::OnUpdateDebugTestpointeditor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
}

//-------------------------------------------------------------------
