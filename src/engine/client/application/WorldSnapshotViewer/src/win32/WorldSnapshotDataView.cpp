//===================================================================
//
// WorldSnapshotDataView.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstWorldSnapshotViewer.h"
#include "WorldSnapshotDataView.h"

#include "ChildFrm.h"
#include "GdiHelper.h"
#include "MainFrm.h"
#include "sharedMath/Transform.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "WorldSnapshotTreeView.h"
#include "WorldSnapshotViewerDoc.h"

#include <algorithm>

//===================================================================

namespace
{
	const float s_mapWidthInMeters = 16384.f;
}

//===================================================================

IMPLEMENT_DYNCREATE(WorldSnapshotDataView, CView)

BEGIN_MESSAGE_MAP(WorldSnapshotDataView, CView)
	//{{AFX_MSG_MAP(WorldSnapshotDataView)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

//===================================================================

WorldSnapshotDataView::WorldSnapshotDataView()
{
}

//-------------------------------------------------------------------

WorldSnapshotDataView::~WorldSnapshotDataView()
{
}

//-------------------------------------------------------------------

const Vector2d convertScreenToWorld (const CRect& rect, const CPoint& point)
{
	const float x = s_mapWidthInMeters * (static_cast<float> (point.x - rect.CenterPoint ().x) / rect.Width ());
	const float y = -s_mapWidthInMeters * (static_cast<float> (point.y - rect.CenterPoint ().y) / rect.Height ());

	return Vector2d (x, y);
}

//-------------------------------------------------------------------

const CPoint convertWorldToScreen (const CRect& rect, const Vector2d& position_w)
{
	const float x = static_cast<float> (rect.CenterPoint ().x) + (position_w.x * rect.CenterPoint ().x) / (s_mapWidthInMeters * 0.5f);
	const float y = static_cast<float> (rect.CenterPoint ().y) - (position_w.y * rect.CenterPoint ().y) / (s_mapWidthInMeters * 0.5f);

	return CPoint (static_cast<int> (x), static_cast<int> (y));
}

//-------------------------------------------------------------------

void WorldSnapshotDataView::OnDraw(CDC* pDC)
{
	const WorldSnapshotViewerDoc* document = safe_cast<const WorldSnapshotViewerDoc*> (GetDocument ());
	const WorldSnapshotReaderWriter* reader = document->getWorldSnapshotReaderWriter ();

	CRect rect;
	GetClientRect (&rect);

	GdiHelper helper (pDC);

	helper.blue1 ();
	{
		const int radius = rect.Width () / 64;
		const int x = rect.CenterPoint ().x;
		const int z = rect.CenterPoint ().y;

		pDC->Ellipse (static_cast<int> (x - radius), static_cast<int> (z - radius), static_cast<int> (x + radius), static_cast<int> (z + radius));
	}

	helper.red1 ();
	{
		const Vector& position_w = document->getSelectedPosition_w ();
		if (position_w != Vector::zero)
		{
			const int radius = rect.Width () / 32;
			const CPoint point = convertWorldToScreen (rect, Vector2d (position_w.x, position_w.z));
			pDC->Ellipse (point.x - radius, point.y - radius, point.x + radius, point.y + radius);

			CString buffer;
			buffer.Format ("%i root objects", document->getNumberOfNodes ());
			pDC->TextOut (0, 0, buffer);

			buffer.Format ("%i total objects", document->getTotalNumberOfNodes ());
			pDC->TextOut (0, 16, buffer);

			buffer.Format ("world %1.2f,  %1.2f,  %1.2f", position_w.x, position_w.y, position_w.z);
			pDC->TextOut (0, 32, buffer);

			const Vector position_p = document->getSelectedPosition_p ();

			buffer.Format ("cell  %1.2f,  %1.2f,  %1.2f", position_p.x, position_p.y, position_p.z);
			pDC->TextOut (0, 48, buffer);
		}
	}

	helper.blue1 ();
	{
		const float delta = RECIP (8.f);

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

	helper.black1 ();
	{
		int i;
		for (i = 0; i < reader->getNumberOfNodes (); ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = reader->getNode (i);

			const Transform& transform = node->getTransform_p ();
			const int radius = 1;
			const CPoint point = convertWorldToScreen (rect, Vector2d (transform.getPosition_p ().x, transform.getPosition_p ().z));
			pDC->Ellipse (point.x - radius, point.y - radius, point.x + radius, point.y + radius);
		}
	}
}

//-------------------------------------------------------------------

void WorldSnapshotDataView::OnInitialUpdate()
{
}

//-------------------------------------------------------------------

BOOL WorldSnapshotDataView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void WorldSnapshotDataView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void WorldSnapshotDataView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void WorldSnapshotDataView::AssertValid() const
{
	CView::AssertValid();
}

void WorldSnapshotDataView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

//===================================================================

void WorldSnapshotDataView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect (&rect);

	safe_cast<CMainFrame*> (AfxGetMainWnd ())->setMapPosition (convertScreenToWorld (rect, point));

	CView::OnMouseMove(nFlags, point);
}

//-------------------------------------------------------------------

void WorldSnapshotDataView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CView::OnLButtonDown(nFlags, point);
}

//-------------------------------------------------------------------

void WorldSnapshotDataView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CView::OnLButtonUp(nFlags, point);
}

//===================================================================

