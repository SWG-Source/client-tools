// ======================================================================
//
// MainFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "MainFrame.h"

#include "DatabaseObjectViewer.h"
#include "DatabaseObjectViewerDoc.h"

// ======================================================================

#define ID_DYNAMIC_COMMAND_START  48000 
#define ID_DYNAMIC_COMMAND_PLANET 48000 
#define ID_DYNAMIC_COMMAND_HOST   49000 
#define ID_DYNAMIC_COMMAND_END    49999

// ======================================================================

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENU()
	ON_COMMAND(ID_OPTIONS_OPTIMIZEFILEONLOAD, OnOptionsOptimizefileonload)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_OPTIMIZEFILEONLOAD, OnUpdateOptionsOptimizefileonload)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_2D_MAP_POSITION, OnUpdate2dMapPosition)
	ON_COMMAND_RANGE (ID_DYNAMIC_COMMAND_START, ID_DYNAMIC_COMMAND_END, OnExecuteDynamicMenu)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_2D_MAP_POSITION,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ----------------------------------------------------------------------

MainFrame::MainFrame() :
	m_planetMap (),
	m_hostMap (),
	m_mapPosition_w (0, 0),
	m_optimizeOnLoad (false)
{
	// TODO: add member initialization code here
	
}

// ----------------------------------------------------------------------

MainFrame::~MainFrame()
{
}

// ----------------------------------------------------------------------

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
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
		!m_wndReBar.AddBar(&m_wndToolBar))// ||
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

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndStatusBar.SetPaneInfo (1, ID_INDICATOR_2D_MAP_POSITION, SBPS_NORMAL, 100);

	return 0;
}

// ----------------------------------------------------------------------

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

// ----------------------------------------------------------------------

void MainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

void MainFrame::OnInitMenu(CMenu* pMenu) 
{
	DatabaseObjectViewerDoc * const document = dynamic_cast<DatabaseObjectViewerApp *> (AfxGetApp ())->getDocument ();
	if (document)
	{
		{
			UINT const planetOffset = 1;
			CString name;
			pMenu->GetMenuString (planetOffset, name, MF_BYPOSITION);
			if (name == "Planets")
			{
				m_planetMap.clear ();

				CMenu * const menu = pMenu->GetSubMenu (planetOffset);

				while (menu->GetMenuItemCount ())
					menu->RemoveMenu (0, MF_BYPOSITION);

				DatabaseObjectViewerApp::PlanetNameSet const & planetNameSet = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getPlanetNameSet ();
				DatabaseObjectViewerApp::PlanetNameSet::const_iterator end = planetNameSet.end ();
				UINT offset = 0;
				for (DatabaseObjectViewerApp::PlanetNameSet::const_iterator iter = planetNameSet.begin (); iter != end; ++iter, ++offset)
				{
					UINT flags = MF_STRING | MF_ENABLED;
					if (document->getSelectedPlanet () == *iter)
						flags |= MF_CHECKED;

					menu->AppendMenu (flags, ID_DYNAMIC_COMMAND_PLANET + offset, iter->c_str ());

					m_planetMap.insert (std::make_pair (ID_DYNAMIC_COMMAND_PLANET + offset, *iter));
				}
			}
		}

		{
			UINT const clusterOffset = 2;
			CString name;
			pMenu->GetMenuString (clusterOffset, name, MF_BYPOSITION);
			if (name == "Clusters")
			{
				m_hostMap.clear ();

				CMenu * const menu = pMenu->GetSubMenu (clusterOffset);

				while (menu->GetMenuItemCount ())
					menu->RemoveMenu (0, MF_BYPOSITION);

				DatabaseObjectViewerApp::HostNameClusterNameMap const & hostNameClusterNameMap = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getHostNameClusterNameMap ();

				DatabaseObjectViewerDoc::ClusterMap const & clusterMap = document->getClusterMap ();
				DatabaseObjectViewerDoc::ClusterMap::const_iterator end = clusterMap.end ();
				UINT offset = 0;
				for (DatabaseObjectViewerDoc::ClusterMap::const_iterator iter = clusterMap.begin (); iter != end; ++iter, ++offset)
				{
					UINT flags = MF_STRING | MF_ENABLED;
					if (document->getHostEnabled (iter->first))
						flags |= MF_CHECKED;

					CString name (iter->first.c_str ());

					DatabaseObjectViewerApp::HostNameClusterNameMap::const_iterator hostNameClusterNameMapIterator = hostNameClusterNameMap.find (iter->first);
					if (hostNameClusterNameMapIterator != hostNameClusterNameMap.end ())
					{
						name += " (";
						name += hostNameClusterNameMapIterator->second.c_str ();
						name += ")";
					}

					menu->AppendMenu (flags, ID_DYNAMIC_COMMAND_HOST + offset, name);

					m_hostMap.insert (std::make_pair (ID_DYNAMIC_COMMAND_HOST + offset, iter->first));
				}

				menu->AppendMenu (MF_SEPARATOR);
				menu->AppendMenu (MF_STRING | MF_ENABLED, ID_DYNAMIC_COMMAND_HOST + offset, "Check all");
				menu->AppendMenu (MF_STRING | MF_ENABLED, ID_DYNAMIC_COMMAND_HOST + offset + 1, "Clear all");
			}
		}
	}

	CMDIFrameWnd::OnInitMenu(pMenu);

	DrawMenuBar ();
}

// ----------------------------------------------------------------------

void MainFrame::OnExecuteDynamicMenu(UINT nID)
{
	DatabaseObjectViewerDoc * const document = dynamic_cast<DatabaseObjectViewerApp *> (AfxGetApp ())->getDocument ();
	if (document)
	{
		if (nID >= ID_DYNAMIC_COMMAND_PLANET && nID < ID_DYNAMIC_COMMAND_HOST)
		{
			IdStringMap::const_iterator iter = m_planetMap.find (nID);
			if (iter != m_planetMap.end ())
				document->setSelectedPlanet (iter->second);
		}
		else
			if (nID >= ID_DYNAMIC_COMMAND_HOST && nID < ID_DYNAMIC_COMMAND_END)
			{
				int const numberOfHosts = m_hostMap.size ();
				if (nID < ID_DYNAMIC_COMMAND_HOST + m_hostMap.size ())
				{
					IdStringMap::const_iterator iter = m_hostMap.find (nID);
					if (iter != m_hostMap.end ())
						document->setHostEnabled (iter->second, !document->getHostEnabled (iter->second));
				}
				else
				{
					if (nID == ID_DYNAMIC_COMMAND_HOST + numberOfHosts)
						document->checkAllHosts ();
					else
						if (nID == ID_DYNAMIC_COMMAND_HOST + numberOfHosts + 1)
							document->clearAllHosts ();
				}
			}

		document->UpdateAllViews (0);
	}
}

// ----------------------------------------------------------------------

void MainFrame::OnUpdate2dMapPosition(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable (true); 

	CString info;
	info.Format ("<%i, %i>", static_cast<int> (m_mapPosition_w.x), static_cast<int> (m_mapPosition_w.y)); 
	pCmdUI->SetText (info);
}

// ----------------------------------------------------------------------

void MainFrame::setMapPosition (CPoint const & mapPosition_w)
{
	m_mapPosition_w = mapPosition_w;
}

// ----------------------------------------------------------------------

void MainFrame::OnOptionsOptimizefileonload() 
{
	m_optimizeOnLoad = !m_optimizeOnLoad;
}

// ----------------------------------------------------------------------

void MainFrame::OnUpdateOptionsOptimizefileonload(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_optimizeOnLoad);
}

// ----------------------------------------------------------------------

bool MainFrame::getOptimizeOnLoad () const
{
	return m_optimizeOnLoad;
}

// ======================================================================
