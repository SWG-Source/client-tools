// ======================================================================
//
// DatabaseObjectViewerView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "stdafx.h"
#include "DatabaseObjectViewerView.h"

#include "DatabaseObjectViewer.h"
#include "DatabaseObjectViewerDoc.h"
#include "GdiHelper.h"
#include "MainFrame.h"

// ======================================================================

namespace
{
	const float s_mapWidthInMeters = 16384.f;
}

// ======================================================================

IMPLEMENT_DYNCREATE(DatabaseObjectViewerView, CView)

// ----------------------------------------------------------------------

DatabaseObjectViewerView::DatabaseObjectViewerView() :
	m_showStats (true),
	m_showGrid (true)
{
}

// ----------------------------------------------------------------------

DatabaseObjectViewerView::~DatabaseObjectViewerView()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DatabaseObjectViewerView, CView)
	//{{AFX_MSG_MAP(DatabaseObjectViewerView)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_BUTTON_SHOWSTATS, OnButtonShowstats)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOWSTATS, OnUpdateButtonShowstats)
	ON_COMMAND(ID_BUTTON_CHECKALL, OnButtonCheckall)
	ON_COMMAND(ID_BUTTON_CLEARALL, OnButtonClearall)
	ON_COMMAND(ID_BUTTON_GRID, OnButtonGrid)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_GRID, OnUpdateButtonGrid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

CPoint const convertScreenToWorld (CRect const & rect, CPoint const & point)
{
	float const x = s_mapWidthInMeters * (static_cast<float> (point.x - rect.CenterPoint ().x) / rect.Width ());
	float const y = -s_mapWidthInMeters * (static_cast<float> (point.y - rect.CenterPoint ().y) / rect.Height ());

	return CPoint (static_cast<int> (x), static_cast<int> (y));
}

//-------------------------------------------------------------------

CPoint const convertWorldToScreen (CRect const & rect, CPoint const & position_w)
{
	float const x = static_cast<float> (rect.CenterPoint ().x) + (position_w.x * rect.CenterPoint ().x) / (s_mapWidthInMeters * 0.5f);
	float const y = static_cast<float> (rect.CenterPoint ().y) - (position_w.y * rect.CenterPoint ().y) / (s_mapWidthInMeters * 0.5f);

	return CPoint (static_cast<int> (x), static_cast<int> (y));
}

//-------------------------------------------------------------------

void DatabaseObjectViewerView::OnDraw(CDC* pDC)
{
	DatabaseObjectViewerDoc const * const document = dynamic_cast<DatabaseObjectViewerDoc const *> (GetDocument ());
	DatabaseObjectViewerDoc::ClusterMap const & clusterMap = document->getClusterMap ();

	CRect rect;
	GetClientRect (&rect);

	GdiHelper helper (pDC);

	if (m_showGrid)
	{
		helper.blue1 ();
		{
			int const radius = rect.Width () / 64;
			int const x = rect.CenterPoint ().x;
			int const z = rect.CenterPoint ().y;

			pDC->Ellipse (static_cast<int> (x - radius), static_cast<int> (z - radius), static_cast<int> (x + radius), static_cast<int> (z + radius));
		}

		helper.blue1 ();
		{
			float const delta = 1.f / 8.f;

			{
				float current = 0.f;
				while (current <= 1.f)
				{
					pDC->MoveTo (static_cast<int> (current * rect.Width ()), 0);
					pDC->LineTo (static_cast<int> (current * rect.Width ()), rect.Height ());
					current += delta;
				}
			}

			{
				float current = 0.f;
				while (current <= 1.f)
				{
					pDC->MoveTo (0, static_cast<int> (current * rect.Height ()));
					pDC->LineTo (rect.Width (), static_cast<int> (current * rect.Height ()));
					current += delta;
				}
			}
		}
	}

	helper.black1 ();
	{
		std::string const planetName = document->getSelectedPlanet ();

		int offset = 0;
		int total = 0;
		DatabaseObjectViewerDoc::ClusterMap::const_iterator clusterMapEnd = clusterMap.end ();
		for (DatabaseObjectViewerDoc::ClusterMap::const_iterator clusterMapIter = clusterMap.begin (); clusterMapIter != clusterMapEnd; ++clusterMapIter)
		{
			if (document->getHostEnabled (clusterMapIter->first))
			{
				bool const snapshotObject = clusterMapIter->first == "worldsnapshot";

				DatabaseObjectViewerDoc::PlanetMap const & planetMap = clusterMapIter->second;
				DatabaseObjectViewerDoc::PlanetMap::const_iterator planetMapIter = planetMap.find (planetName);
				if (planetMapIter != planetMap.end ())
				{
					DatabaseObjectViewerDoc::NodeList const & nodeList = planetMapIter->second;

					for (size_t i = 0; i < nodeList.size (); ++i)
					{
						int const radius = 1;
						CPoint const point = convertWorldToScreen (rect, nodeList [i].m_point);

						if (snapshotObject)
							helper.red3 ();
						else
						{
							if (nodeList [i].m_installation)
								helper.blue3 ();
							else
								helper.black3 ();
						}

						pDC->Ellipse (point.x - radius, point.y - radius, point.x + radius, point.y + radius);
					}
				}

				if (m_showStats)
				{
					helper.red1 ();
					{
						DatabaseObjectViewerApp::HostNameClusterNameMap const & hostNameClusterNameMap = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getHostNameClusterNameMap ();

						CString name (clusterMapIter->first.c_str ());

						DatabaseObjectViewerApp::HostNameClusterNameMap::const_iterator hostNameClusterNameIter = hostNameClusterNameMap.find (clusterMapIter->first);
						if (hostNameClusterNameIter != hostNameClusterNameMap.end ())
						{
							name += " (";
							name += hostNameClusterNameIter->second.c_str ();
							name += ")";
						}

						CString buffer;
						buffer.Format ("%s (%i objects)", name, planetMapIter->second.size ());
						pDC->TextOut (0, offset, buffer);
						offset += 16;
					}
					helper.black1 ();
				}

				total += planetMapIter->second.size ();
			}
		}

		CString buffer;
		buffer.Format ("%i total objects", total);
		pDC->TextOut (0, offset, buffer);
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void DatabaseObjectViewerView::AssertValid() const
{
	CView::AssertValid();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect (&rect);

	dynamic_cast<MainFrame *> (AfxGetMainWnd ())->setMapPosition (convertScreenToWorld (rect, point));
	
	CView::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnButtonShowstats() 
{
	// TODO: Add your command handler code here
	m_showStats = !m_showStats;
	Invalidate ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnUpdateButtonShowstats(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (m_showStats);	
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnButtonCheckall() 
{
	// TODO: Add your command handler code here
	DatabaseObjectViewerDoc * const document = dynamic_cast<DatabaseObjectViewerDoc *> (GetDocument ());
	document->checkAllHosts ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnButtonClearall() 
{
	// TODO: Add your command handler code here
	DatabaseObjectViewerDoc * const document = dynamic_cast<DatabaseObjectViewerDoc *> (GetDocument ());
	document->clearAllHosts ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnButtonGrid() 
{
	m_showGrid = !m_showGrid;
	Invalidate ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnUpdateButtonGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_showGrid);	
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	DatabaseObjectViewerDoc * const document = dynamic_cast<DatabaseObjectViewerDoc *> (GetDocument ());
	SetWindowText (document->GetPathName () + " [" + document->getSelectedPlanet ().c_str () + "]");
}

// ======================================================================

