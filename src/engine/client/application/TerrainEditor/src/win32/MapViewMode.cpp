//
// MapViewMode.cpp
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "MapView.h"

#include "sharedTerrain/Affector.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/Boundary.h"
#include "clientGraphics/Graphics.h"
#include "LayerFrame.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"

#include <algorithm>

//-------------------------------------------------------------------

inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

static inline bool isCloseTo (const CPoint& source, int radius, const CPoint& point)
{
	CRect rect (source, CSize (radius * 2, radius * 2));
	rect.OffsetRect (-radius, -radius);

	return rect.PtInRect (point) != 0;
}

//-------------------------------------------------------------------

MapView::Mode::Mode (MapView* const newMapView) :
	mapView (newMapView),
	originated (false)
{
	NOT_NULL (mapView);
}

MapView::Mode::~Mode (void)
{
}

void MapView::Mode::onCancel (void)
{
	originated = false;

	mapView->setMode (MapView::MT_normal);
}

void MapView::Mode::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	originated = true;
}

void MapView::Mode::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated)
	{
	}
}

void MapView::Mode::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated)
	{
	}
}

void MapView::Mode::onRButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated)
	{
		onCancel ();
	}
}

void MapView::Mode::onRButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated)
	{
	}
}

void MapView::Mode::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (character);
	UNREF (repeatCount);
	UNREF (flags);

	if (originated)
	{
	}
}

void MapView::Mode::render (CDC* pDC) const
{
	UNREF (pDC);

	if (originated)
	{
	}
}

//-------------------------------------------------------------------

MapView::ModeNormal::ModeNormal (MapView* newMapView) :
	Mode (newMapView)
{
}

MapView::ModeNormal::~ModeNormal (void)
{
}

void MapView::ModeNormal::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (!keyDown(VK_SHIFT))
	{
		return;
	}

	const Vector2d point_w = mapView->convertScreenToWorld (point);
	TerrainEditorDoc* pDoc  = static_cast<TerrainEditorDoc*> (mapView->GetDocument ());
	if(!pDoc->getLayerFrame())
	{
		return;
	}

	ArrayList<TerrainGenerator::LayerItem*> itemList;
	{
		TerrainGeneratorHelper::findBoundaryAndAffectorBoundaryPoly (pDoc->getTerrainGenerator (), itemList,point_w,true);
	}

	if (itemList.getNumberOfElements () == 0)
		return;

	// see if we need to do a deep pick - meaning we're clicking on an already selected item that might be overlapping another
	//  item that we really want to select.  The deep pick will cycle through items on subsequent clicks.
	bool bDeepPick = false;
	if(pDoc->getSelectedItem() && (pDoc->getSelectedItem()->type == TerrainEditorDoc::Item::T_affector || pDoc->getSelectedItem()->type == TerrainEditorDoc::Item::T_boundary))
	{
		TerrainGenerator::Boundary* currentlySelectedBoundary = dynamic_cast <TerrainGenerator::Boundary*> (pDoc->getSelectedItem()->layerItem);

		if(currentlySelectedBoundary)
		{
			bDeepPick = true;
		}
		else
		{
			TerrainGenerator::Affector* currentlySelectedAffector = dynamic_cast <TerrainGenerator::Affector*> (pDoc->getSelectedItem()->layerItem);

			if(currentlySelectedAffector && (currentlySelectedAffector->getType() == TGAT_river || currentlySelectedAffector->getType() == TGAT_road || currentlySelectedAffector->getType() == TGAT_ribbon))
			{
				bDeepPick = true;
			}
		}
	}

	if(bDeepPick)
	{
		int currentlySelectedIndex = -1;
		for(int itemIndex = 0; itemIndex < itemList.getNumberOfElements(); ++itemIndex)
		{
			TerrainGenerator::LayerItem* layerItem = itemList[itemIndex];
			TerrainGenerator::LayerItem* currentlySelectedItem = pDoc->getSelectedItem()->layerItem;
			if(currentlySelectedItem == layerItem)
			{
				currentlySelectedIndex = itemIndex;
			}
		}

		int newItemIndex = (currentlySelectedIndex + 1) % itemList.getNumberOfElements();
		TerrainGenerator::LayerItem* newSelectedItem = itemList[newItemIndex];
		pDoc->getLayerFrame ()->selectLayerItem(newSelectedItem);
	}
	else
	{
		TerrainGenerator::LayerItem* layerItem = itemList [0];
		pDoc->getLayerFrame()->selectLayerItem(layerItem);
	}
}





//-------------------------------------------------------------------

MapView::ModeCreateCircle::ModeCreateCircle (MapView* newMapView) :
	Mode (newMapView),
	start (),
	end (),
	drawing (false)
{
}

MapView::ModeCreateCircle::~ModeCreateCircle (void)
{
}

void MapView::ModeCreateCircle::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);

	originated  = true;
	drawing     = true;
	end = start = mapView->convertScreenToWorld (point);

	mapView->Invalidate (false);
}

void MapView::ModeCreateCircle::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		//-- not drawing anymore
		drawing = false;

		if (end.x > start.x && end.y < start.y)
		{
			//-- extract rect
			const real     radius = min (abs (start.x - end.x), abs (start.y - end.y)) / 2;

#if MAPVIEW_REVERSED
			const Vector2d center (start.x + radius, start.y - radius);
#else
			const Vector2d center (start.x + radius, start.y + radius);
#endif

			if (radius > CONST_REAL (0))
			{
				//-- send to system
				mapView->addBoundaryCircle (center, radius);
			}

			//-- reset for next rect
			end = start = mapView->convertScreenToWorld (point);
		}

		//--
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreateCircle::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		end = mapView->convertScreenToWorld (point);

		mapView->setCursor (MapView::CT_circle);
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreateCircle::render (CDC* pDC) const
{
	UNREF (pDC);

	if (originated && drawing)
	{
		CPoint startScreen  = mapView->convertWorldToScreen (start);
		CPoint endScreen    = mapView->convertWorldToScreen (end);

		if (endScreen.x > startScreen.x && endScreen.y > startScreen.y)
		{
			CRect rect (startScreen, endScreen);

			int length = min (rect.Width (), rect.Height ());
			int radius = length / 2;

			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
			Graphics::drawCircle (startScreen.x + radius, startScreen.y + radius, radius, VectorArgb::solidYellow);
		}
	}
}

//-------------------------------------------------------------------

MapView::ModeCreateRectangle::ModeCreateRectangle (MapView* newMapView) :
	Mode (newMapView),
	start (),
	end (),
	drawing (false)
{
}

MapView::ModeCreateRectangle::~ModeCreateRectangle (void)
{
}

void MapView::ModeCreateRectangle::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);

	originated  = true;
	drawing     = true;
	end = start = mapView->convertScreenToWorld (point);

	mapView->Invalidate (false);
}

void MapView::ModeCreateRectangle::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		//-- not drawing anymore
		drawing = false;

		//-- extract rect
		Rectangle2d rect;
		rect.x0 = start.x;
		rect.y0 = start.y;
		rect.x1 = end.x;
		rect.y1 = end.y;

		if (!rect.isVector2d () && rect.getWidth () != CONST_REAL (0) && rect.getHeight () != CONST_REAL (0))
		{
			//-- send to system
			mapView->addBoundaryRectangle (rect);
		}

		//-- reset for next rect
		end = start = mapView->convertScreenToWorld (point);

		//--
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreateRectangle::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		end = mapView->convertScreenToWorld (point);

		mapView->setCursor (MapView::CT_rectangle);
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreateRectangle::render (CDC* pDC) const
{
	UNREF (pDC);

	if (originated && drawing)
	{
		if (start != end)
			mapView->drawRectangle_w (pDC, start, end, VectorArgb::solidYellow);
	}
}

//-------------------------------------------------------------------

MapView::ModeCreatePolygon::ModeCreatePolygon (MapView* newMapView) :
	Mode (newMapView),
	pointList (),
	end (),
	drawing (false)
{
}

MapView::ModeCreatePolygon::~ModeCreatePolygon (void)
{
}

//-------------------------------------------------------------------

void MapView::ModeCreatePolygon::onCancel (void)
{
	pointList.clear ();

	Mode::onCancel ();
}

//-------------------------------------------------------------------

void MapView::ModeCreatePolygon::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);

	originated  = true;

	end = mapView->convertScreenToWorld (point);

	if (pointList.getNumberOfElements () > 0 && isCloseTo (mapView->convertWorldToScreen (pointList [0]), 3, mapView->convertWorldToScreen (end)))
	{
		drawing = false;

		//-- send to system
		mapView->addBoundaryPolygon (pointList);

		pointList.clear ();
	}
	else
	{
		drawing = true;

		pointList.add (end);
	}

	mapView->Invalidate (false);
}

void MapView::ModeCreatePolygon::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		end = mapView->convertScreenToWorld (point);

		mapView->setCursor (MapView::CT_polygon);
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreatePolygon::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (repeatCount);
	UNREF (flags);

	if (originated && drawing)
	{
		switch (character)
		{
		case VK_DELETE:
			{
				if (pointList.getNumberOfElements () > 1)
				{
					pointList.deleteLast ();

					mapView->Invalidate (false);
				}
			}
			break;

		default:
			break;
		}
	}
}

void MapView::ModeCreatePolygon::render (CDC* pDC) const
{
	UNREF (pDC);

	if (originated && drawing)
	{
		if (pointList.getNumberOfElements () > 0)
		{
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

			mapView->drawPolygon_w (pDC, pointList, VectorArgb::solidYellow);

			int i;
			for (i = 0; i < pointList.getNumberOfElements (); i++)
			{
				CPoint screenPoint = mapView->convertWorldToScreen (pointList [i]);

				Graphics::drawRectangle (screenPoint.x - 2, screenPoint.y - 2, screenPoint.x + 2, screenPoint.y + 2, VectorArgb::solidWhite);
			}

			if (pointList.getNumberOfElements () > 0)
			{
				CPoint screenPoint1 = mapView->convertWorldToScreen (pointList [pointList.getNumberOfElements () - 1]);
				CPoint screenPoint2 = mapView->convertWorldToScreen (end);

				Graphics::drawLine (screenPoint1.x, screenPoint1.y, screenPoint2.x, screenPoint2.y, VectorArgb::solidYellow);
			}
		}
	}
}

//-------------------------------------------------------------------

MapView::ModeCreatePolyline::ModeCreatePolyline (MapView* newMapView, MapView::AddPolylineFunction addPolylineFunction) :
	Mode (newMapView),
	pointList (),
	end (),
	drawing (false),
	m_addPolylineFunction (addPolylineFunction)
{
}

MapView::ModeCreatePolyline::~ModeCreatePolyline (void)
{
}

//-------------------------------------------------------------------

void MapView::ModeCreatePolyline::onCancel (void)
{
	pointList.clear ();

	Mode::onCancel ();
}

//-------------------------------------------------------------------

void MapView::ModeCreatePolyline::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);

	originated  = true;

	end = mapView->convertScreenToWorld (point);

	if (pointList.getNumberOfElements () > 1 && isCloseTo (mapView->convertWorldToScreen (pointList [pointList.getNumberOfElements () - 1]), 3, mapView->convertWorldToScreen (end)))
	{
		drawing = false;

		//-- send to system
		(mapView->*m_addPolylineFunction) (pointList);

		pointList.clear ();
	}
	else
	{
		drawing = true;

		pointList.add (end);
	}

	mapView->Invalidate (false);
}

void MapView::ModeCreatePolyline::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (originated && drawing)
	{
		end = mapView->convertScreenToWorld (point);

		mapView->setCursor (MapView::CT_polyline);
		mapView->Invalidate (false);
	}
}

void MapView::ModeCreatePolyline::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (repeatCount);
	UNREF (flags);

	if (originated && drawing)
	{
		switch (character)
		{
		case VK_DELETE:
			{
				if (pointList.getNumberOfElements () > 1)
				{
					pointList.deleteLast ();

					mapView->Invalidate (false);
				}
			}
			break;

		default:
			break;
		}
	}
}

void MapView::ModeCreatePolyline::render (CDC* pDC) const
{
	UNREF (pDC);

	if (originated && drawing)
	{
		if (pointList.getNumberOfElements () > 0)
		{
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

			mapView->drawPolyline_w (pDC, pointList, 0.f, VectorArgb::solidYellow);

			int i;
			for (i = 0; i < pointList.getNumberOfElements (); i++)
			{
				const CPoint screenPoint = mapView->convertWorldToScreen (pointList [i]);

				if (i == pointList.getNumberOfElements () - 1)
				{
					const CPoint screenPointEnd = mapView->convertWorldToScreen (end);

					Graphics::drawLine (screenPoint.x, screenPoint.y, screenPointEnd.x, screenPointEnd.y, VectorArgb::solidYellow);
				}

				Graphics::drawRectangle (screenPoint.x - 2, screenPoint.y - 2, screenPoint.x + 2, screenPoint.y + 2, VectorArgb::solidWhite);
			}
		}
	}
}

//-------------------------------------------------------------------

MapView::ModeZoom::ModeZoom (MapView* newMapView) :
	Mode (newMapView)
{
}

MapView::ModeZoom::~ModeZoom (void)
{
}

//-------------------------------------------------------------------

void MapView::ModeZoom::onLButtonDown (UINT flags, const CPoint& point)
{
	if (flags & MK_CONTROL)
	{
		const Vector2d center_w = mapView->convertScreenToWorld (point);
		mapView->zoomOut (center_w);
	}
	else
	{
		const Vector2d center_w = mapView->convertScreenToWorld (point);
		mapView->zoomIn (center_w);
	}
}

//-------------------------------------------------------------------

void MapView::ModeZoom::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (point);

	if (flags & MK_CONTROL)
		mapView->setCursor (MapView::CT_zoomOut);
	else
		mapView->setCursor (MapView::CT_zoomIn);
}

//-------------------------------------------------------------------

MapView::ModeModifyPolygon::ModeModifyPolygon (MapView* newMapView) :
	Mode (newMapView),
	boundaryPolygon (0),
	selectedIndex (0),
	dragging (false),
	start ()
{
}

MapView::ModeModifyPolygon::~ModeModifyPolygon (void)
{
	boundaryPolygon = 0;
}

void MapView::ModeModifyPolygon::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;

	if (boundaryPolygon)
	{
		if (boundaryPolygon->getNumberOfPoints ())
		{
			int i;
			for (i = 0; i < boundaryPolygon->getNumberOfPoints (); i++)
			{
				if (isCloseTo (mapView->convertWorldToScreen (boundaryPolygon->getPoint (i)), 3, point))
				{
					selectedIndex = i;
					dragging      = true;

					mapView->Invalidate ();

					break;
				}
			}
		}
	}
}

void MapView::ModeModifyPolygon::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;
}

void MapView::ModeModifyPolygon::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (boundaryPolygon && dragging)
	{
		boundaryPolygon->replacePoint (selectedIndex, mapView->convertScreenToWorld (point));

		mapView->Invalidate ();
	}
}

void MapView::ModeModifyPolygon::onRButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	onCancel ();
}

void MapView::ModeModifyPolygon::onRButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);
}

void MapView::ModeModifyPolygon::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (character);
	UNREF (repeatCount);
	UNREF (flags);

	if (!boundaryPolygon)
		return;

	switch (character)
	{
	case VK_DELETE:
		{
			if (boundaryPolygon->getNumberOfPoints () > 3)
			{
				ArrayList<Vector2d> pointList = boundaryPolygon->getPointList ();

				pointList.removeIndexAndCompactList (selectedIndex);

				if (selectedIndex >= pointList.getNumberOfElements ())
					--selectedIndex;

				boundaryPolygon->copyPointList (pointList);

				mapView->Invalidate (false);
			}
		}
		break;

	case VK_INSERT:
		{
			ArrayList<Vector2d> pointList = boundaryPolygon->getPointList ();

			if (selectedIndex == pointList.getNumberOfElements () - 1)
			{
				const Vector2d midpoint ((pointList [0] + pointList [pointList.getNumberOfElements () - 1]) * 0.5f);

				pointList.add (midpoint);

				selectedIndex = pointList.getNumberOfElements () - 1;
			}
			else
			{
				const Vector2d midpoint ((pointList [selectedIndex] + pointList [selectedIndex + 1]) * 0.5f);

				pointList.insert (selectedIndex + 1, midpoint);
			}

			boundaryPolygon->copyPointList (pointList);

			mapView->Invalidate (false);
		}
		break;

	default:
		break;
	}
}

void MapView::ModeModifyPolygon::render (CDC* pDC) const
{
	UNREF (pDC);

	if (boundaryPolygon)
	{
		if (boundaryPolygon->getNumberOfPoints ())
		{
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

			int i;
			for (i = 0; i < boundaryPolygon->getNumberOfPoints (); i++)
			{
				CPoint screenPoint = mapView->convertWorldToScreen (boundaryPolygon->getPoint (i));

				Graphics::drawRectangle (screenPoint.x - 2, screenPoint.y - 2, screenPoint.x + 2, screenPoint.y + 2, i == selectedIndex ? VectorArgb::solidRed : VectorArgb::solidWhite);
			}
		}
	}
}

void MapView::ModeModifyPolygon::setBoundaryPolygon (BoundaryPolygon* newBoundaryPolygon)
{
	boundaryPolygon = newBoundaryPolygon;
	selectedIndex   = 0;
}

const BoundaryPolygon* MapView::ModeModifyPolygon::getBoundaryPolygon (void) const
{
	return boundaryPolygon;
}

//-------------------------------------------------------------------

MapView::ModeModifyPolyline::ModeModifyPolyline (MapView* newMapView) :
	Mode (newMapView),
	boundaryPolyline (0),
	selectedIndex (0),
	dragging (false),
	start ()
{
}

MapView::ModeModifyPolyline::~ModeModifyPolyline (void)
{
	boundaryPolyline = 0;
}

void MapView::ModeModifyPolyline::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;

	if (boundaryPolyline)
	{
		if (boundaryPolyline->getNumberOfPoints ())
		{
			int i;
			for (i = 0; i < boundaryPolyline->getNumberOfPoints (); i++)
			{
				if (isCloseTo (mapView->convertWorldToScreen (boundaryPolyline->getPoint (i)), 3, point))
				{
					selectedIndex = i;
					dragging      = true;

					mapView->Invalidate ();

					break;
				}
			}
		}
	}
}

void MapView::ModeModifyPolyline::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;
}

void MapView::ModeModifyPolyline::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (boundaryPolyline && dragging)
	{
		boundaryPolyline->replacePoint (selectedIndex, mapView->convertScreenToWorld (point));

		mapView->Invalidate ();
	}
}

void MapView::ModeModifyPolyline::onRButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	onCancel ();
}

void MapView::ModeModifyPolyline::onRButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);
}

void MapView::ModeModifyPolyline::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (character);
	UNREF (repeatCount);
	UNREF (flags);

	if (!boundaryPolyline)
		return;

	switch (character)
	{
	case VK_DELETE:
		{
			if (boundaryPolyline->getNumberOfPoints () > 2)
			{
				ArrayList<Vector2d> pointList = boundaryPolyline->getPointList ();

				pointList.removeIndexAndCompactList (selectedIndex);

				if (selectedIndex >= pointList.getNumberOfElements ())
					--selectedIndex;

				boundaryPolyline->copyPointList (pointList);

				mapView->Invalidate (false);
			}
		}
		break;

	case VK_INSERT:
		{
			ArrayList<Vector2d> pointList = boundaryPolyline->getPointList ();

			if (selectedIndex == pointList.getNumberOfElements () - 1)
			{
				const Vector2d lastPoint = pointList [pointList.getNumberOfElements () - 1];
				const Vector2d midpoint ((pointList [pointList.getNumberOfElements () - 2] + pointList [pointList.getNumberOfElements () - 1]) * 0.5f);

				pointList [pointList.getNumberOfElements () - 1] = midpoint;
				pointList.add (lastPoint);

				selectedIndex = pointList.getNumberOfElements () - 1;
			}
			else
			{
				const Vector2d midpoint ((pointList [selectedIndex] + pointList [selectedIndex + 1]) * 0.5f);

				pointList.insert (selectedIndex + 1, midpoint);
			}

			boundaryPolyline->copyPointList (pointList);

			mapView->Invalidate (false);
		}
		break;

	default:
		break;
	}
}

void MapView::ModeModifyPolyline::render (CDC* pDC) const
{
	UNREF (pDC);

	if (boundaryPolyline)
	{
		if (boundaryPolyline->getNumberOfPoints ())
		{
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

			int i;
			for (i = 0; i < boundaryPolyline->getNumberOfPoints (); i++)
			{
				CPoint screenPoint = mapView->convertWorldToScreen (boundaryPolyline->getPoint (i));

				Graphics::drawRectangle (screenPoint.x - 2, screenPoint.y - 2, screenPoint.x + 2, screenPoint.y + 2, i == selectedIndex ? VectorArgb::solidRed : VectorArgb::solidWhite);
			}
		}
	}
}

void MapView::ModeModifyPolyline::setBoundaryPolyline (BoundaryPolyline* newBoundaryPolyline)
{
	boundaryPolyline = newBoundaryPolyline;
	selectedIndex   = 0;
}

const BoundaryPolyline* MapView::ModeModifyPolyline::getBoundaryPolyline (void) const
{
	return boundaryPolyline;
}

//-------------------------------------------------------------------

MapView::ModeModifyAffectorBoundaryPoly::ModeModifyAffectorBoundaryPoly (MapView* newMapView) :
	Mode (newMapView),
	affectorBoundaryPoly (0),
	selectedIndex (0),
	dragging (false),
	start ()
{
}

MapView::ModeModifyAffectorBoundaryPoly::~ModeModifyAffectorBoundaryPoly (void)
{
	affectorBoundaryPoly = 0;
}

void MapView::ModeModifyAffectorBoundaryPoly::onLButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;

	if (affectorBoundaryPoly)
	{
		if (affectorBoundaryPoly->getNumberOfPoints ())
		{
			int i;
			for (i = 0; i < affectorBoundaryPoly->getNumberOfPoints (); i++)
			{
				if (isCloseTo (mapView->convertWorldToScreen (affectorBoundaryPoly->getPoint (i)), 3, point))
				{
					selectedIndex = i;
					dragging      = true;

					mapView->Invalidate ();

					break;
				}
			}
		}
	}
}

void MapView::ModeModifyAffectorBoundaryPoly::onLButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	dragging = false;
}

void MapView::ModeModifyAffectorBoundaryPoly::onMouseMove (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	if (affectorBoundaryPoly && dragging)
	{
		affectorBoundaryPoly->replacePoint (selectedIndex, mapView->convertScreenToWorld (point));

		if(affectorBoundaryPoly->getType() == TGAT_ribbon)
		{
			AffectorRibbon* affectorRibbon  = static_cast<AffectorRibbon*> (affectorBoundaryPoly);
			affectorRibbon->generateEndCapPointList();
		}
		
		mapView->Invalidate ();
	}
}

void MapView::ModeModifyAffectorBoundaryPoly::onRButtonDown (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);

	onCancel ();
}

void MapView::ModeModifyAffectorBoundaryPoly::onRButtonUp (UINT flags, const CPoint& point)
{
	UNREF (flags);
	UNREF (point);
}

void MapView::ModeModifyAffectorBoundaryPoly::onKeyDown (UINT character, UINT repeatCount, UINT flags)
{
	UNREF (character);
	UNREF (repeatCount);
	UNREF (flags);

	if (!affectorBoundaryPoly)
		return;

	switch (character)
	{
	case VK_DELETE:
		{
			if (affectorBoundaryPoly->getNumberOfPoints () > 2)
			{
				ArrayList<Vector2d> pointList = affectorBoundaryPoly->getPointList ();

				pointList.removeIndexAndCompactList (selectedIndex);
				affectorBoundaryPoly->copyPointList (pointList);

				if(affectorBoundaryPoly->getType() == TGAT_ribbon)
				{
					AffectorRibbon* affectorRibbon  = static_cast<AffectorRibbon*> (affectorBoundaryPoly);
					ArrayList<float> heightList = affectorRibbon->getHeightList();
					heightList.removeIndexAndCompactList (selectedIndex);
					affectorRibbon->copyHeightList (heightList);
					affectorRibbon->generateEndCapPointList();
				}
				else if(affectorBoundaryPoly->getType() == TGAT_road)
				{
					AffectorRoad* affectorRoad  = static_cast<AffectorRoad*> (affectorBoundaryPoly);
					if(affectorRoad->getHasFixedHeights())
					{
						ArrayList<float> heightList = affectorRoad->getHeightList();
						heightList.removeIndexAndCompactList (selectedIndex);
						affectorRoad->copyHeightList(heightList);
					}
				}

				if (selectedIndex >= pointList.getNumberOfElements ())
					--selectedIndex;

				mapView->Invalidate (false);
			}
		}
		break;

	case VK_INSERT:
		{
			ArrayList<Vector2d> pointList = affectorBoundaryPoly->getPointList ();

			if (selectedIndex == pointList.getNumberOfElements () - 1)
			{
				const Vector2d lastPoint = pointList [pointList.getNumberOfElements () - 1];
				const Vector2d midpoint ((pointList [pointList.getNumberOfElements () - 2] + pointList [pointList.getNumberOfElements () - 1]) * 0.5f);

				pointList [pointList.getNumberOfElements () - 1] = midpoint;
				pointList.add (lastPoint);
				affectorBoundaryPoly->copyPointList (pointList);

				if(affectorBoundaryPoly->getType() == TGAT_ribbon)
				{
					AffectorRibbon* affectorRibbon  = static_cast<AffectorRibbon*> (affectorBoundaryPoly);
					ArrayList<float> heightList = affectorRibbon->getHeightList();
					heightList.add(0.0f);
					affectorRibbon->copyHeightList(heightList);
					affectorRibbon->generateEndCapPointList();
				}
				else if(affectorBoundaryPoly->getType() == TGAT_road)
				{
					AffectorRoad* affectorRoad  = static_cast<AffectorRoad*> (affectorBoundaryPoly);
					if(affectorRoad->getHasFixedHeights())
					{
						ArrayList<float> heightList = affectorRoad->getHeightList();
						heightList.add(0.0f);
						affectorRoad->copyHeightList(heightList);
					}
				}

				selectedIndex = pointList.getNumberOfElements () - 1;
			}
			else
			{
				const Vector2d midpoint ((pointList [selectedIndex] + pointList [selectedIndex + 1]) * 0.5f);

				pointList.insert (selectedIndex + 1, midpoint);
				affectorBoundaryPoly->copyPointList (pointList);

				if(affectorBoundaryPoly->getType() == TGAT_ribbon)
				{
					AffectorRibbon* affectorRibbon  = static_cast<AffectorRibbon*> (affectorBoundaryPoly);
					ArrayList<float> heightList = affectorRibbon->getHeightList();
					heightList.insert (selectedIndex + 1, 0.0f);
					affectorRibbon->copyHeightList(heightList);
					affectorRibbon->generateEndCapPointList();
				}
				else if(affectorBoundaryPoly->getType() == TGAT_road)
				{
					AffectorRoad* affectorRoad  = static_cast<AffectorRoad*> (affectorBoundaryPoly);
					if(affectorRoad->getHasFixedHeights())
					{
						ArrayList<float> heightList = affectorRoad->getHeightList();
						heightList.insert (selectedIndex + 1, 0.0f);
						affectorRoad->copyHeightList(heightList);
					}
				}
			}

			mapView->Invalidate (false);
		}
		break;

	default:
		break;
	}
}

void MapView::ModeModifyAffectorBoundaryPoly::render (CDC* pDC) const
{
	UNREF (pDC);

	if (affectorBoundaryPoly)
	{
		if (affectorBoundaryPoly->getNumberOfPoints ())
		{
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

			int i;
			for (i = 0; i < affectorBoundaryPoly->getNumberOfPoints (); i++)
			{
				CPoint screenPoint = mapView->convertWorldToScreen (affectorBoundaryPoly->getPoint (i));

				Graphics::drawRectangle (screenPoint.x - 2, screenPoint.y - 2, screenPoint.x + 2, screenPoint.y + 2, i == selectedIndex ? VectorArgb::solidRed : VectorArgb::solidWhite);
			}
		}
	}
}

void MapView::ModeModifyAffectorBoundaryPoly::setAffectorBoundaryPoly (AffectorBoundaryPoly* newBoundaryPoly)
{
	affectorBoundaryPoly = newBoundaryPoly;
	selectedIndex   = 0;
}

const AffectorBoundaryPoly* MapView::ModeModifyAffectorBoundaryPoly::getAffectorBoundaryPoly (void) const
{
	return affectorBoundaryPoly;
}

//-------------------------------------------------------------------

