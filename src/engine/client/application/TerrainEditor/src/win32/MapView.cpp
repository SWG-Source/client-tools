//
// MapView.cpp
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "MapView.h"

#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/Boundary.h"
#include "DialogEditorPreferences.h"
#include "EditorTerrain.h"
#include "clientGraphics/Graphics.h"
#include "MainFrame.h"
#include "MapFrame.h"
#include "clientObject/ObjectListCamera.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "sharedDebug/Profiler.h"

#include <algorithm>

//-------------------------------------------------------------------

static inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

MapView::Grid::Grid (void) :
	mapView (0),
	gridDistance (0)
{
}

//-------------------------------------------------------------------

MapView::Grid::~Grid (void)
{
	mapView = 0;
}

//-------------------------------------------------------------------

void MapView::Grid::setMapView (const MapView* newMapView)
{
	mapView = newMapView;
}

//-------------------------------------------------------------------

void MapView::Grid::setGridDistance (int newGridDistance)
{
	gridDistance = abs (newGridDistance);
}

//-------------------------------------------------------------------

void MapView::Grid::draw (CDC* pDC) const
{
	UNREF (pDC);

	NOT_NULL (mapView);

	const int gridDistanceScreen = mapView->convertWorldToScreen (static_cast<real> (gridDistance));

	if (gridDistanceScreen > 0)
	{
		CRect rect;
		mapView->GetClientRect (&rect);

		Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

		//-- find where 0 is
		Vector2d zero;
		zero.makeZero ();

		CPoint centerScreen = mapView->convertWorldToScreen (zero);

		//-- draw vertical lines
		int x = rect.left / gridDistanceScreen;

		x += centerScreen.x % gridDistanceScreen;

		while (x < rect.right)
		{
			Graphics::drawLine (x, rect.top, x, rect.bottom, VectorArgb::solidRed);

			x += gridDistanceScreen;
		}

		//-- draw horizontal lines
		int y = rect.top / gridDistanceScreen;

		y += centerScreen.y % gridDistanceScreen;

		while (y < rect.bottom)
		{
			Graphics::drawLine (rect.left, y, rect.right, y, VectorArgb::solidRed);

			y += gridDistanceScreen;
		}
	}
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(MapView, CView)

//-------------------------------------------------------------------

MapView::MapView (void) :
	CView (),
	mapFrame (0),
	currentMode (MT_normal),
	centerOffset (0, 0),
	grid (),
	terrain (0),
	zoomLevel (1),
	zoomMin (RECIP (64)),
	zoomMax (CONST_REAL (32)),
	showBoundaries (true),
	showGrid (false),
	showLighting (true),
	showHeightMap (true),
	showColorMap (true),
	showShaderMap (true),
	showFloraMap (true),
	showRadialMap (true),
	showEnvironmentMap (false),
	showPassableMap(false),
	showWater (true),
	showProfile (false),
	showRoads (true),
	showRivers (true),
	showRibbonAffectors (true),
	key_panMap (),
	key_rotateBoundary (),
	key_translateBoundary (),
	key_scaleBoundary (),
	key_rotateBoundaryCentered ()
{
	{
		int i;
		for (i = 0; i < static_cast<int> (MT_count); ++i)
			modes [i] = 0;

		for (i = 0; i < static_cast<int> (CT_count); ++i)
			cursors [i] = 0;
	}

	modes [MT_normal]          = new ModeNormal (this);
	modes [MT_zoom]            = new ModeZoom (this);
	modes [MT_createCircle]    = new ModeCreateCircle (this);
	modes [MT_createRectangle] = new ModeCreateRectangle (this);
	modes [MT_createPolygon]   = new ModeCreatePolygon (this);
	modes [MT_modifyPolygon]   = new ModeModifyPolygon (this);
	modes [MT_createPolyline]  = new ModeCreatePolyline (this, &MapView::addBoundaryPolyline);
	modes [MT_modifyPolyline]  = new ModeModifyPolyline (this);
	modes [MT_createRoad]      = new ModeCreatePolyline (this, &MapView::addAffectorRoad);
	modes [MT_modifyRoad]      = new ModeModifyAffectorBoundaryPoly (this);
	modes [MT_createRiver]     = new ModeCreatePolyline (this, &MapView::addAffectorRiver);
	modes [MT_modifyRiver]     = new ModeModifyAffectorBoundaryPoly (this);
	modes [MT_createRibbonAffector] = new ModeCreatePolyline (this, &MapView::addAffectorRibbon);
	modes [MT_modifyRibbonAffector] = new ModeModifyAffectorBoundaryPoly (this);

	cursors [CT_pointer]   = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_MOVE));       //lint !e1924  //-- c-style cast
	cursors [CT_circle]    = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_CIRCLE));     //lint !e1924  //-- c-style cast
	cursors [CT_rectangle] = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_RECTANGLE));  //lint !e1924  //-- c-style cast
	cursors [CT_polygon]   = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_POLYGON));    //lint !e1924  //-- c-style cast
	cursors [CT_polyline]  = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_POLYLINE));   //lint !e1924  //-- c-style cast
	cursors [CT_zoomIn]    = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_IN));    //lint !e1924  //-- c-style cast
	cursors [CT_zoomOut]   = LoadCursor (AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_OUT));   //lint !e1924  //-- c-style cast

	{
		int i;
		for (i = 0; i < static_cast<int> (MT_count); ++i)
			NOT_NULL (modes [i]);

		for (i = 0; i < static_cast<int> (CT_count); ++i)
			NOT_NULL (cursors [i]);
	}

	grid.setMapView (this);

	key_panMap.bind                 (VK_MENU);
	key_rotateBoundary.bind         ('Z');
	key_translateBoundary.bind      ('M');
	key_scaleBoundary.bind          ('S');
	key_rotateBoundaryCentered.bind ('X');
}

//-------------------------------------------------------------------

MapView::~MapView()
{
	int i;
	for (i = 0; i < MT_count; i++)
	{
		delete modes [i];
		modes [i] = 0;
	}

	if (terrain)
	{
		delete terrain;
		terrain = 0;
	}

	mapFrame = 0;
}

//-------------------------------------------------------------------

void MapView::setMode (MapView::ModeType newMode)
{
	if (newMode != currentMode)
	{
		currentMode = newMode;
	}
}

//-------------------------------------------------------------------

void MapView::setCursor (CursorType newCursorType)
{
	SetCursor (cursors [newCursorType]);
}

//-------------------------------------------------------------------

void MapView::zoomIn()
{
	CRect rect;
	GetClientRect (&rect);

	const Vector2d center = convertScreenToWorld (rect.CenterPoint ());

	zoomIn (center);
}

//-------------------------------------------------------------------

void MapView::zoomIn (const Vector2d& center)
{
	if (zoomLevel < zoomMax)
	{
		zoomLevel = clamp (zoomMin, zoomLevel * CONST_REAL (2), zoomMax);

		GetMainFrame ()->setZoomLevel (zoomLevel);

		//-- update the ruler
		NOT_NULL (mapFrame);
		mapFrame->updateRulers ();

		recenter (center);

		//-- mark all chunks as dirty
		if (terrain)
			terrain->markDirty ();

		Invalidate (false);
	}
}

//-------------------------------------------------------------------

void MapView::zoomOut()
{
	//-- get the center
	CRect rect;
	GetClientRect (&rect);

	const Vector2d center = convertScreenToWorld (rect.CenterPoint ());

	zoomOut (center);
}

//-------------------------------------------------------------------

void MapView::zoomOut (const Vector2d& center)
{
	if (zoomLevel > zoomMin)
	{
		zoomLevel = clamp (zoomMin, zoomLevel * CONST_REAL (0.5), zoomMax);

		GetMainFrame ()->setZoomLevel (zoomLevel);

		//-- update the ruler
		NOT_NULL (mapFrame);
		mapFrame->updateRulers ();

		recenter (center);

		//-- mark all chunks as dirty
		if (terrain)
			terrain->markDirty ();

		Invalidate (false);
	}
}

//-------------------------------------------------------------------

void MapView::setZoom (real newZoomLevel)
{
	zoomLevel = 1.f;

	GetMainFrame ()->setZoomLevel (newZoomLevel);

	//-- update the ruler
	NOT_NULL (mapFrame);
	mapFrame->updateRulers ();

	Vector2d zero;
	zero.makeZero ();
	recenter (zero);

	//-- mark all chunks as dirty
	if (terrain)
		terrain->markDirty ();

	Invalidate (false);
}

//-------------------------------------------------------------------

const Vector2d MapView::convertScreenToWorld (const CPoint& point) const
{
#if MAPVIEW_REVERSED
	return Vector2d (static_cast<real> (point.x - centerOffset.x) / zoomLevel, static_cast<real> (centerOffset.y - point.y) / zoomLevel);
#else
  	return Vector2d (static_cast<real> (point.x - centerOffset.x) / zoomLevel, static_cast<real> (point.y - centerOffset.y) / zoomLevel);
#endif
}

//-------------------------------------------------------------------

real MapView::convertScreenToWorld (int distance) const
{
	return static_cast<real> (distance / zoomLevel);
}

//-------------------------------------------------------------------

const CPoint MapView::convertWorldToScreen (const Vector2d& vector2d) const
{
#if MAPVIEW_REVERSED
	return CPoint (static_cast<int> (vector2d.x * zoomLevel + static_cast<real> (centerOffset.x)), static_cast<int> (static_cast<real> (centerOffset.y) - vector2d.y * zoomLevel));
#else
  	return CPoint (static_cast<int> (vector2d.x * zoomLevel + static_cast<real> (centerOffset.x)), static_cast<int> (vector2d.y * zoomLevel + static_cast<real> (centerOffset.y)));
#endif
}

//-------------------------------------------------------------------

int MapView::convertWorldToScreen (real distance) const
{
	return static_cast<int> (distance * zoomLevel);
}

//-------------------------------------------------------------------

void MapView::recenter (const Vector2d& center_w)
{
	CRect rect;
	GetClientRect (&rect);

	CPoint centerScreen = convertWorldToScreen (center_w);

	centerOffset.x += rect.CenterPoint ().x - centerScreen.x;
	centerOffset.y += rect.CenterPoint ().y - centerScreen.y;

	NOT_NULL (mapFrame);
	mapFrame->updateRulers ();

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::drawSelectedRoads (CDC* pDC, const VectorArgb& color) const
{
	const TerrainEditorDoc* pDoc = static_cast<const TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			drawRoads (pDC, color, static_cast<const TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			drawRoad (pDC, color, static_cast<const TerrainGenerator::Affector*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_boundary:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::drawRoad (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw) const
{
	if ((affector->isActive () || forceDraw) && affector->getType () == TGAT_road)
	{
		const AffectorRoad* const affectorRoad = static_cast<const AffectorRoad*> (affector);

		drawPolyline_w (pDC, affectorRoad->getPointList (), affectorRoad->getWidth ()/2.0f, color);
	}
}

//-------------------------------------------------------------------

void MapView::drawRoads (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw) const
{
	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			drawRoad (pDC, color, layer->getAffector (i), forceDraw);
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			drawRoads (pDC, color, layer->getLayer (i), forceDraw);
	}
}

//-------------------------------------------------------------------

void MapView::drawRoads (CDC* pDC, const VectorArgb& color, bool forceDraw) const
{
	const TerrainEditorDoc* pDoc      = static_cast<const TerrainEditorDoc*> (GetDocument());
	const TerrainGenerator* generator = pDoc->getTerrainGenerator ();

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		drawRoads (pDC, color, generator->getLayer (i), forceDraw);
}

//-------------------------------------------------------------------

void MapView::drawSelectedRivers (CDC* pDC, const VectorArgb& color) const
{
	const TerrainEditorDoc* pDoc = static_cast<const TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			drawRivers (pDC, color, static_cast<const TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			drawRiver (pDC, color, static_cast<const TerrainGenerator::Affector*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_boundary:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::drawRiver (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw) const
{
	if ((affector->isActive () || forceDraw) && affector->getType () == TGAT_river)
	{
		const AffectorRiver* const affectorRiver = static_cast<const AffectorRiver*> (affector);

		drawPolyline_w (pDC, affectorRiver->getPointList (), affectorRiver->getWidth (), color);
	}
}

//-------------------------------------------------------------------

void MapView::drawRivers (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw) const
{
	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			drawRiver (pDC, color, layer->getAffector (i), forceDraw);
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			drawRivers (pDC, color, layer->getLayer (i), forceDraw);
	}
}

//-------------------------------------------------------------------

void MapView::drawRivers (CDC* pDC, const VectorArgb& color, bool forceDraw) const
{
	const TerrainEditorDoc* pDoc      = static_cast<const TerrainEditorDoc*> (GetDocument());
	const TerrainGenerator* generator = pDoc->getTerrainGenerator ();

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		drawRivers (pDC, color, generator->getLayer (i), forceDraw);
}


//-------------------------------------------------------------------

void MapView::drawSelectedRibbonAffectors (CDC* pDC, const VectorArgb& color) const
{
	const TerrainEditorDoc* pDoc = static_cast<const TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			drawRibbonAffectors (pDC, color, static_cast<const TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			drawRibbonAffector (pDC, color, static_cast<const TerrainGenerator::Affector*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_boundary:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::drawRibbonAffector (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw) const
{
	if ((affector->isActive () || forceDraw) && affector->getType () == TGAT_ribbon)
	{
		const AffectorRibbon* const affectorRibbon = static_cast<const AffectorRibbon*> (affector);
		ArrayList<Vector2d> pointList = affectorRibbon->getPointList();
		const float halfWidth = affectorRibbon->getWidth ()/2.0f;
		const ArrayList<Vector2d>& endCapPointList = affectorRibbon->getEndCapPointList();
		if(endCapPointList.size() > 0)
		{
			drawPolygon_w(pDC, endCapPointList, color);
		}
		drawPolyline_w (pDC, pointList, halfWidth, color);
	}
}

//-------------------------------------------------------------------

void MapView::drawRibbonAffectors (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw) const
{
	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			drawRibbonAffector (pDC, color, layer->getAffector (i), forceDraw);
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			drawRibbonAffectors (pDC, color, layer->getLayer (i), forceDraw);
	}
}

//-------------------------------------------------------------------

void MapView::drawRibbonAffectors (CDC* pDC, const VectorArgb& color, bool forceDraw) const
{
	const TerrainEditorDoc* pDoc      = static_cast<const TerrainEditorDoc*> (GetDocument());
	const TerrainGenerator* generator = pDoc->getTerrainGenerator ();

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		drawRibbonAffectors (pDC, color, generator->getLayer (i), forceDraw);
}


//-------------------------------------------------------------------

void MapView::drawSelectedBoundaries (CDC* pDC, const VectorArgb& color) const
{
	const TerrainEditorDoc* pDoc = static_cast<const TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			drawLayer (pDC, color, static_cast<const TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			drawBoundary (pDC, color, static_cast<const TerrainGenerator::Boundary*> (pDoc->getSelectedItem ()->layerItem), true);
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	case TerrainEditorDoc::Item::T_affector:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::drawBoundaries (CDC* pDC, const VectorArgb& color, bool forceDraw) const
{
	const TerrainEditorDoc* pDoc = static_cast<const TerrainEditorDoc*> (GetDocument());
	const TerrainGenerator* generator = pDoc->getTerrainGenerator ();

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		drawLayer (pDC, color, generator->getLayer (i), forceDraw);
}

//-------------------------------------------------------------------

void MapView::drawLayer (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw) const
{
	int n = layer->getNumberOfBoundaries ();
	int i;
	for (i = 0; i < n; i++)
		drawBoundary (pDC, color, layer->getBoundary (i), forceDraw);

	n = layer->getNumberOfLayers ();
	for (i = 0; i < n; i++)
		drawLayer (pDC, color, layer->getLayer (i), forceDraw);
}

//-------------------------------------------------------------------

void MapView::drawBoundary (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Boundary* boundary, bool forceDraw) const
{
	if (!boundary->isActive () && !forceDraw)
		return;

	switch (boundary->getType ())
	{
	case TGBT_circle:
		{
			const BoundaryCircle* boundaryCircle = static_cast<const BoundaryCircle*> (boundary);

			drawCircle_w (pDC, Vector2d (boundaryCircle->getCenterX (), boundaryCircle->getCenterZ ()), boundaryCircle->getRadius (), color);
		}
		break;

	case TGBT_rectangle:
		{
			const BoundaryRectangle* boundaryRectangle = static_cast<const BoundaryRectangle*> (boundary);
			const Rectangle2d rect = boundaryRectangle->getRectangle ();

			drawRectangle_w (pDC, Vector2d (rect.x0, rect.y0), Vector2d (rect.x1, rect.y1), color);
		}
		break;

	case TGBT_polygon:
		{
			const BoundaryPolygon* boundaryPolygon = static_cast<const BoundaryPolygon*> (boundary);

			drawPolygon_w (pDC, boundaryPolygon->getPointList (), color);
		}
		break;

	case TGBT_polyline:
		{
			const BoundaryPolyline* boundaryPolyline = static_cast<const BoundaryPolyline*> (boundary);

			drawPolyline_w (pDC, boundaryPolyline->getPointList (), boundaryPolyline->getWidth (), color);
		}
		break;

	default:
		DEBUG_FATAL (true, ("invalid region type"));
		break;
	}
}

//-------------------------------------------------------------------

void MapView::drawCircle_w (CDC* pDC, const Vector2d& center, real radius, const VectorArgb& color) const
{
	UNREF (pDC);

	CRect rect;
	GetClientRect (&rect);

	CPoint centerScreen = convertWorldToScreen (center);
	int    radiusScreen = convertWorldToScreen (radius);

	//-- is the circle off screen?
	if (centerScreen.x + radiusScreen < rect.left ||
		centerScreen.x - radiusScreen > rect.right ||
		centerScreen.y + radiusScreen < rect.top ||
		centerScreen.y - radiusScreen > rect.bottom)
		return;

	//-- more optimizations here

	//-- draw the circle
	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
	Graphics::drawCircle (centerScreen.x, centerScreen.y, radiusScreen, color);
	Graphics::drawCircle (centerScreen.x, centerScreen.y, radiusScreen + 1, color);
}

//-------------------------------------------------------------------

void MapView::drawRectangle_w (CDC* pDC, const Vector2d& start, const Vector2d& end, const VectorArgb& color) const
{
	UNREF (pDC);

	CRect rect;
	GetClientRect (&rect);

	CPoint startScreen = convertWorldToScreen (start);
	CPoint endScreen   = convertWorldToScreen (end);

	//-- is the rectangle offscreen?
	int left   = min (startScreen.x, endScreen.x);
	int right  = max (startScreen.x, endScreen.x);
	int top    = min (startScreen.y, endScreen.y);
	int bottom = max (startScreen.y, endScreen.y);

	if (left   > rect.right ||
		right  < rect.left ||
		top    > rect.bottom ||
		bottom < rect.top)
		return;

	//-- draw the rectangle
	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
	Graphics::drawRectangle (startScreen.x, startScreen.y, endScreen.x, endScreen.y, color);
	Graphics::drawRectangle (startScreen.x + 1, startScreen.y + 1, endScreen.x - 1, endScreen.y - 1, color);
}

//-------------------------------------------------------------------

void MapView::drawLine_w (CDC* pDC, const Vector2d& start, const Vector2d& end, const VectorArgb& color) const
{
	UNREF (pDC);

	CRect rect;
	GetClientRect (&rect);

	CPoint startScreen = convertWorldToScreen (start);
	CPoint endScreen   = convertWorldToScreen (end);

	//-- is the rectangle offscreen?
	int left   = min (startScreen.x, endScreen.x);
	int right  = max (startScreen.x, endScreen.x);
	int top    = min (startScreen.y, endScreen.y);
	int bottom = max (startScreen.y, endScreen.y);

	if (left   > rect.right ||
		right  < rect.left ||
		top    > rect.bottom ||
		bottom < rect.top)
		return;

	//-- draw the line
	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
	Graphics::drawRectangle (startScreen.x - 1, startScreen.y - 1, startScreen.x + 1, startScreen.y + 1, VectorArgb::solidWhite);
	Graphics::drawRectangle (endScreen.x - 1, endScreen.y - 1, endScreen.x + 1, endScreen.y + 1, VectorArgb::solidWhite);
	Graphics::drawLine (startScreen.x, startScreen.y, endScreen.x, endScreen.y, color);
}

//-------------------------------------------------------------------

void MapView::drawPolygon_w (CDC* pDC, const ArrayList<Vector2d>& pointList, const VectorArgb& color) const
{
	UNREF (pDC);

	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

	const int n = pointList.getNumberOfElements ();

	int i;
	int j;
	for (i = 0, j = n - 1; i < n; j = i++)
	{
		CPoint screenPoint1 = convertWorldToScreen (pointList [i]);
		CPoint screenPoint2 = convertWorldToScreen (pointList [j]);

		Graphics::drawLine (screenPoint1.x, screenPoint1.y, screenPoint2.x, screenPoint2.y, color);
		Graphics::drawLine (screenPoint1.x+1, screenPoint1.y+1, screenPoint2.x+1, screenPoint2.y+1, color);
	}
}

//-------------------------------------------------------------------

void MapView::drawPolyline_w (CDC* pDC, const ArrayList<Vector2d>& pointList, const float width, const VectorArgb& color) const
{
	UNREF (pDC);

	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

	if (width > 0.f)
	{
		if (pointList.getNumberOfElements () == 0)
			return;

		int i;
		for (i = 0; i < pointList.getNumberOfElements () - 1; ++i)
		{
			const bool atStart   = (i == 0);
			const bool atEndNext = (i == (pointList.getNumberOfElements () - 2));

			const Vector2d& currentStart          = pointList [i];
			const Vector2d& currentEnd            = pointList [i + 1];
			const Vector2d  currentSegment        = currentEnd - currentStart;
			const Vector2d  currentDirection      = Vector2d::normalized (currentSegment);
			const Vector2d  currentNormal         = Vector2d::normal (currentDirection, false);
			const Vector2d  currentStartLeft      = currentStart - currentNormal * width;
			const Vector2d  currentStartRight     = currentStart + currentNormal * width;
			const Vector2d  currentEndLeft        = currentEnd   - currentNormal * width;
			const Vector2d  currentEndRight       = currentEnd   + currentNormal * width;

			const Vector2d& previousStart         = !atStart ? pointList [i - 1] : pointList [i];
			const Vector2d& previousEnd           = !atStart ? pointList [i]     : pointList [i];
			const Vector2d  previousSegment       = previousEnd - previousStart;
			const Vector2d  previousDirection     = !atStart ? Vector2d::normalized (previousSegment) : currentDirection;
			const Vector2d  previousNormal        = !atStart ? Vector2d::normal (previousDirection, false) : currentNormal;
			const Vector2d  previousStartLeft     = previousStart - previousNormal * width;
			const Vector2d  previousStartRight    = previousStart + previousNormal * width;
			const Vector2d  previousEndLeft       = previousEnd   - previousNormal * width;
			const Vector2d  previousEndRight      = previousEnd   + previousNormal * width;

			const Vector2d& nextStart             = !atEndNext ? pointList [i + 1] : pointList [i + 1];
			const Vector2d& nextEnd               = !atEndNext ? pointList [i + 2] : pointList [i + 1];
			const Vector2d  nextSegment           = nextEnd - nextStart;
			const Vector2d  nextDirection         = !atEndNext ? Vector2d::normalized (nextSegment) : currentDirection;
			const Vector2d  nextNormal            = !atEndNext ? Vector2d::normal (nextDirection, false) : currentNormal;
			const Vector2d  nextStartLeft         = nextStart - nextNormal * width;
			const Vector2d  nextStartRight        = nextStart + nextNormal * width;
			const Vector2d  nextEndLeft           = nextEnd   - nextNormal * width;
			const Vector2d  nextEndRight          = nextEnd   + nextNormal * width;

			const Vector2d  startLeft  = !atStart ? currentStart - (Vector2d::normalized (currentStart - (previousEndLeft + currentStartLeft) * 0.5f) * width) : currentStartLeft;
			const Vector2d  endLeft    = !atEndNext ? currentEnd - (Vector2d::normalized (currentEnd   - (currentEndLeft + nextStartLeft) * 0.5f) * width) : currentEndLeft;
			const Vector2d  startRight = !atStart ? currentStart - (Vector2d::normalized (currentStart - (previousEndRight + currentStartRight) * 0.5f) * width) : currentStartRight;
			const Vector2d  endRight   = !atEndNext ? currentEnd - (Vector2d::normalized (currentEnd   - (currentEndRight + nextStartRight) * 0.5f) * width) : currentEndRight;

			const CPoint    startLeftScreen  = convertWorldToScreen (startLeft);
			const CPoint    startRightScreen = convertWorldToScreen (startRight);
			const CPoint    endLeftScreen    = convertWorldToScreen (endLeft);
			const CPoint    endRightScreen   = convertWorldToScreen (endRight);

			if (atStart)
			{
				const CPoint p [6] =
				{
					convertWorldToScreen (currentStartLeft - (currentDirection * width)),
					convertWorldToScreen (currentStartRight - (currentDirection * width)),
					convertWorldToScreen (startLeft - (currentDirection * width)),
					convertWorldToScreen (startRight - (currentDirection * width)),
					convertWorldToScreen (endLeft + (nextDirection * width)),
					convertWorldToScreen (endRight + (nextDirection * width))
				};

				Graphics::drawLine (p [0].x, p [0].y, p [1].x, p [1].y, color);

				if (atEndNext)
				{
					Graphics::drawLine (p [2].x, p [2].y, p [4].x, p [4].y, color);
					Graphics::drawLine (p [3].x, p [3].y, p [5].x, p [5].y, color);
					Graphics::drawLine (p [4].x, p [4].y, p [5].x, p [5].y, color);
				}
				else
				{
					Graphics::drawLine (p [2].x, p [2].y, endLeftScreen.x, endLeftScreen.y, color);
					Graphics::drawLine (p [3].x, p [3].y, endRightScreen.x, endRightScreen.y, color);
				}
			}
			else
			{
				if (atEndNext)
				{
					const CPoint p [4] =
					{
						convertWorldToScreen (endLeft + (nextDirection * width)),
						convertWorldToScreen (endRight + (nextDirection * width)),
						convertWorldToScreen (nextEndLeft + (nextDirection * width)),
						convertWorldToScreen (nextEndRight + (nextDirection * width))
					};

					Graphics::drawLine (startLeftScreen.x, startLeftScreen.y, p [0].x, p [0].y, color);
					Graphics::drawLine (startRightScreen.x, startRightScreen.y, p [1].x, p [1].y, color);
					Graphics::drawLine (p [2].x, p [2].y, p [3].x, p [3].y, color);
				}
				else
				{
					Graphics::drawLine (startLeftScreen.x, startLeftScreen.y, endLeftScreen.x, endLeftScreen.y, color);
					Graphics::drawLine (startRightScreen.x, startRightScreen.y, endRightScreen.x, endRightScreen.y, color);
				}
			}
		}
	}
	else
	{
		const int n = pointList.getNumberOfElements ();

		int i;
		for (i = 0; i < n - 1; ++i)
		{
			CPoint screenPoint1 = convertWorldToScreen (pointList [i]);
			CPoint screenPoint2 = convertWorldToScreen (pointList [i + 1]);

			Graphics::drawLine (screenPoint1.x, screenPoint1.y, screenPoint2.x, screenPoint2.y, color);
			Graphics::drawLine (screenPoint1.x+1, screenPoint1.y+1, screenPoint2.x+1, screenPoint2.y+1, color);
		}
	}
}

//-------------------------------------------------------------------

void MapView::addBoundaryRectangle (const Rectangle2d& newRectangle)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addBoundaryRectangle (newRectangle);
}

//-------------------------------------------------------------------

void MapView::addBoundaryCircle (const Vector2d& newCenter, real newRadius)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addBoundaryCircle (newCenter, newRadius);
}

//-------------------------------------------------------------------

void MapView::addBoundaryPolygon (const ArrayList<Vector2d>& newPointList)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addBoundaryPolygon (newPointList);
}

//-------------------------------------------------------------------

void MapView::addBoundaryPolyline (const ArrayList<Vector2d>& newPointList)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addBoundaryPolyline (newPointList);
}

//-------------------------------------------------------------------

void MapView::addAffectorRoad (const ArrayList<Vector2d>& newPointList)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addAffectorRoad (newPointList);
}

//-------------------------------------------------------------------

void MapView::addAffectorRiver (const ArrayList<Vector2d>& newPointList)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addAffectorRiver (newPointList);
}

//-------------------------------------------------------------------

void MapView::addAffectorRibbon (const ArrayList<Vector2d>& newPointList)
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());
	NOT_NULL (pDoc);

	if (pDoc->canAddBoundary ())
		pDoc->addAffectorRibbon (newPointList);
}

//-------------------------------------------------------------------

void MapView::rotateBoundaries (real angle) const
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			rotateLayer (angle, static_cast<TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			rotateBoundary (angle, static_cast<TerrainGenerator::Boundary*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			rotateAffector (angle, static_cast<TerrainGenerator::Affector*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::rotateLayer (real angle, TerrainGenerator::Layer* layer) const
{
	{
		const int n = layer->getNumberOfBoundaries ();
		int i;
		for (i = 0; i < n; i++)
			rotateBoundary (angle, layer->getBoundary (i));
	}

	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			rotateAffector (angle, layer->getAffector (i));
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			rotateLayer (angle, layer->getLayer (i));
	}
}

//-------------------------------------------------------------------

void MapView::rotateBoundary (real angle, TerrainGenerator::Boundary* boundary) const
{
	boundary->rotate (angle);
}

//-------------------------------------------------------------------

void MapView::rotateAffector (real angle, TerrainGenerator::Affector* affector) const
{
	AffectorBoundaryPoly* affectorBoundaryPoly = dynamic_cast<AffectorBoundaryPoly*> (affector);

	if (affectorBoundaryPoly)
		affectorBoundaryPoly->rotate (angle);

	AffectorRibbon* affectorRibbon = dynamic_cast<AffectorRibbon*>(affectorBoundaryPoly);
	if(affectorRibbon)
	{
		affectorRibbon->generateEndCapPointList();
	}
}

//-------------------------------------------------------------------

void MapView::rotateCenteredBoundaries (real angle) const
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			TerrainGenerator::Layer* layer = static_cast<TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem);

			const Vector2d center = TerrainGeneratorHelper::computeCenter (layer);

			rotateCenteredLayer (angle, layer, true, center);
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			rotateCenteredBoundary (angle, static_cast<TerrainGenerator::Boundary*> (pDoc->getSelectedItem ()->layerItem), false, Vector2d (0.f, 0.f));
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			rotateCenteredAffector (angle, static_cast<TerrainGenerator::Affector*> (pDoc->getSelectedItem ()->layerItem), false, Vector2d (0.f, 0.f));
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::rotateCenteredLayer (real angle, TerrainGenerator::Layer* layer, bool useCenter, const Vector2d& center) const
{
	{
		const int n = layer->getNumberOfBoundaries ();
		int i;
		for (i = 0; i < n; i++)
			rotateCenteredBoundary (angle, layer->getBoundary (i), useCenter, center);
	}

	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			rotateCenteredAffector (angle, layer->getAffector (i), useCenter, center);
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			rotateCenteredLayer (angle, layer->getLayer (i), useCenter, center);
	}
}

//-------------------------------------------------------------------

void MapView::rotateCenteredBoundary (real angle, TerrainGenerator::Boundary* boundary, bool useCenter, const Vector2d& center) const
{
	if (useCenter)
		boundary->rotate (angle, center);
	else
		boundary->rotate (angle);
}

//-------------------------------------------------------------------

void MapView::rotateCenteredAffector (real angle, TerrainGenerator::Affector* affector, bool useCenter, const Vector2d& center) const
{
	AffectorBoundaryPoly* affectorBoundaryPoly = dynamic_cast<AffectorBoundaryPoly*> (affector);

	if (affectorBoundaryPoly)
	{
		if (useCenter)
			affectorBoundaryPoly->rotate (angle, center);
		else
			affectorBoundaryPoly->rotate (angle);

		AffectorRibbon* affectorRibbon = dynamic_cast<AffectorRibbon*>(affectorBoundaryPoly);
		if(affectorRibbon)
		{
			affectorRibbon->generateEndCapPointList();
		}
	}

	
}

//-------------------------------------------------------------------

void MapView::translateBoundaries (const Vector2d& translation_w) const
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			translateLayer (translation_w, static_cast<TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			translateBoundary (translation_w, static_cast<TerrainGenerator::Boundary*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			translateAffector (translation_w, static_cast<AffectorBoundaryPoly*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::translateLayer (const Vector2d& translation_w, TerrainGenerator::Layer* layer) const
{
	{
		const int n = layer->getNumberOfBoundaries ();
		int i;
		for (i = 0; i < n; i++)
			translateBoundary (translation_w, layer->getBoundary (i));
	}

	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			translateAffector (translation_w, layer->getAffector (i));
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			translateLayer (translation_w, layer->getLayer (i));
	}

}

//-------------------------------------------------------------------

void MapView::translateBoundary (const Vector2d& translation_w, TerrainGenerator::Boundary* boundary) const
{
	boundary->translate (translation_w);
}

//-------------------------------------------------------------------

void MapView::translateAffector (const Vector2d& translation_w, TerrainGenerator::Affector* affector) const
{
	AffectorBoundaryPoly* affectorBoundaryPoly = dynamic_cast<AffectorBoundaryPoly*> (affector);

	if (affectorBoundaryPoly)
		affectorBoundaryPoly->translate (translation_w);

	AffectorRibbon* affectorRibbon = dynamic_cast<AffectorRibbon*>(affectorBoundaryPoly);
	if(affectorRibbon)
	{
		affectorRibbon->generateEndCapPointList();
	}
}

//-------------------------------------------------------------------

void MapView::scaleBoundaries (real scalar) const
{
	TerrainEditorDoc* pDoc = static_cast<TerrainEditorDoc*> (GetDocument());

	if (!pDoc->getSelectedItem ())
		return;

	switch (pDoc->getSelectedItem ()->type)
	{
	case TerrainEditorDoc::Item::T_layer:
		{
			scaleLayer (scalar, static_cast<TerrainGenerator::Layer*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_boundary:
		{
			scaleBoundary (scalar, static_cast<TerrainGenerator::Boundary*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_affector:
		{
			scaleAffector (scalar, static_cast<AffectorBoundaryPoly*> (pDoc->getSelectedItem ()->layerItem));
		}
		break;

	case TerrainEditorDoc::Item::T_unknown:
	case TerrainEditorDoc::Item::T_filter:
	default:
		break;
	}
}

//-------------------------------------------------------------------

void MapView::scaleLayer (real scalar, TerrainGenerator::Layer* layer) const
{
	{
		const int n = layer->getNumberOfBoundaries ();
		int i;
		for (i = 0; i < n; i++)
			scaleBoundary (scalar, layer->getBoundary (i));
	}

	{
		const int n = layer->getNumberOfAffectors ();
		int i;
		for (i = 0; i < n; i++)
			scaleAffector (scalar, layer->getAffector (i));
	}

	{
		const int n = layer->getNumberOfLayers ();
		int i;
		for (i = 0; i < n; i++)
			scaleLayer (scalar, layer->getLayer (i));
	}
}

//-------------------------------------------------------------------

void MapView::scaleBoundary (real scalar, TerrainGenerator::Boundary* boundary) const
{
	boundary->scale (scalar);
}

//-------------------------------------------------------------------

void MapView::scaleAffector (real scalar, TerrainGenerator::Affector* affector) const
{
	AffectorBoundaryPoly* affectorBoundaryPoly = dynamic_cast<AffectorBoundaryPoly*> (affector);

	if (affectorBoundaryPoly)
		affectorBoundaryPoly->scale (scalar);

	AffectorRibbon* affectorRibbon = dynamic_cast<AffectorRibbon*>(affectorBoundaryPoly);
	if(affectorRibbon)
	{
		affectorRibbon->generateEndCapPointList();
	}
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(MapView, CView)
	//{{AFX_MSG_MAP(MapView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_COMMAND(ID_CREATERECTANGLE, OnCreaterectangle)
	ON_UPDATE_COMMAND_UI(ID_CREATERECTANGLE, OnUpdateCreaterectangle)
	ON_COMMAND(ID_CREATECIRCLE, OnCreatecircle)
	ON_UPDATE_COMMAND_UI(ID_CREATECIRCLE, OnUpdateCreatecircle)
	ON_COMMAND(ID_CREATEPOLYGON, OnCreatepolygon)
	ON_UPDATE_COMMAND_UI(ID_CREATEPOLYGON, OnUpdateCreatepolygon)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_GRID, OnGrid)
	ON_UPDATE_COMMAND_UI(ID_GRID, OnUpdateGrid)
	ON_COMMAND(ID_RECENTER, OnRecenter)
	ON_COMMAND(ID_BUTTON_SHOW_BOUNDARIES, OnButtonShowBoundaries)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_BOUNDARIES, OnUpdateButtonShowBoundaries)
	ON_COMMAND(ID_VIEW_COLORMAP, OnViewColormap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLORMAP, OnUpdateViewColormap)
	ON_COMMAND(ID_VIEW_FLORAMAP, OnViewFloramap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FLORAMAP, OnUpdateViewFloramap)
	ON_COMMAND(ID_VIEW_HEIGHTMAP, OnViewHeightmap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HEIGHTMAP, OnUpdateViewHeightmap)
	ON_COMMAND(ID_VIEW_SHADERMAP, OnViewShadermap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHADERMAP, OnUpdateViewShadermap)
	ON_COMMAND(ID_REBUILD, OnRebuild)
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_COMMAND(ID_LIGHTING, OnLighting)
	ON_UPDATE_COMMAND_UI(ID_LIGHTING, OnUpdateLighting)
	ON_COMMAND(ID_ZOOM, OnZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM, OnUpdateZoom)
	ON_WM_CHAR()
	ON_COMMAND(ID_CREATEPOLYLINE, OnCreatePolyline)
	ON_UPDATE_COMMAND_UI(ID_CREATEPOLYLINE, OnUpdateCreatePolyline)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_RADIALMAP, OnViewRadialmap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RADIALMAP, OnUpdateViewRadialmap)
	ON_COMMAND(ID_VIEW_WATERMAP, OnViewWatermap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WATERMAP, OnUpdateViewWatermap)
	ON_COMMAND(ID_BUTTON_SHOW_PROFILE, OnButtonShowProfile)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_PROFILE, OnUpdateButtonShowProfile)
	ON_COMMAND(ID_MODIFYPOLYGON, OnModifypolygon)
	ON_UPDATE_COMMAND_UI(ID_MODIFYPOLYGON, OnUpdateModifypolygon)
	ON_COMMAND(ID_MODIFYPOLYLINE, OnModifypolyline)
	ON_UPDATE_COMMAND_UI(ID_MODIFYPOLYLINE, OnUpdateModifypolyline)
	ON_COMMAND(ID_CREATEROAD, OnCreateroad)
	ON_UPDATE_COMMAND_UI(ID_CREATEROAD, OnUpdateCreateroad)
	ON_COMMAND(ID_CREATERIBBONAFFECTOR, OnCreateRibbonAffector)
	ON_UPDATE_COMMAND_UI(ID_CREATERIBBONAFFECTOR, OnUpdateCreateRibbonAffector)
	ON_COMMAND(ID_BUTTON_SHOW_ROADS, OnButtonShowRoads)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_ROADS, OnUpdateButtonShowRoads)
	ON_COMMAND(ID_BUTTON_SHOW_RIBBON, OnButtonShowRibbonAffectors)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_RIBBON, OnUpdateButtonShowRibbonAffectors)
	ON_COMMAND(ID_CREATERIVER, OnCreateriver)
	ON_UPDATE_COMMAND_UI(ID_CREATERIVER, OnUpdateCreateriver)
	ON_COMMAND(ID_BUTTON_SHOW_RIVERS, OnButtonShowRivers)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_RIVERS, OnUpdateButtonShowRivers)
	ON_COMMAND(ID_MODIFYRIVER, OnModifyriver)
	ON_UPDATE_COMMAND_UI(ID_MODIFYRIVER, OnUpdateModifyriver)
	ON_COMMAND(ID_MODIFYROAD, OnModifyroad)
	ON_UPDATE_COMMAND_UI(ID_MODIFYROAD, OnUpdateModifyroad)
	ON_COMMAND(ID_MODIFYRIBBONAFFECTOR, OnModifyRibbonAffector)
	ON_UPDATE_COMMAND_UI(ID_MODIFYRIBBONAFFECTOR, OnUpdateModifyRibbonAffector)
	ON_COMMAND(ID_VIEW_ENVIRONMENTMAP, OnViewEnvironmentmap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ENVIRONMENTMAP, OnUpdateViewEnvironmentmap)
	ON_COMMAND(ID_VIEW_PASSABLEMAP, OnViewPassableMap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PASSABLEMAP, OnUpdateViewPassableMap)
	ON_COMMAND(ID_BUTTON_HIGHRES, OnButtonHighres)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIGHRES, OnUpdateButtonHighres)
	ON_COMMAND(ID_BUTTON_MEDRES, OnButtonMedres)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MEDRES, OnUpdateButtonMedres)
	ON_COMMAND(ID_BUTTON_LOWRES, OnButtonLowres)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LOWRES, OnUpdateButtonLowres)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

void MapView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
	NOT_NULL (doc);

	//-- tell the document about our frame
	doc->setMapFrame (static_cast<MapFrame*> (GetParentFrame ()));

	//-- create the terrain
	terrain = new EditorTerrain (doc, this);

	//-- calculate initial center offset
	OnRecenter ();

	//-- set grid size
	grid.setGridDistance (32);
}

//-------------------------------------------------------------------------

BOOL MapView::OnEraseBkgnd(CDC* pDC)
{
	UNREF (pDC);

	return true;
}

//-------------------------------------------------------------------

void MapView::OnDraw(CDC* pDC)
{
	UNREF (pDC);

	if (!terrain)
		return;

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
	UNREF (doc);

	//--
	CRect rect;
	GetClientRect (&rect);

	//-- point the Gl at this window
	Graphics::setViewport (0, 0, min(Graphics::getCurrentRenderTargetWidth(), rect.Width ()), min(Graphics::getCurrentRenderTargetHeight(), rect.Height ()));

	Graphics::beginScene ();

			Graphics::clearViewport(true, 0xffff00ff, true, 1.0f, true, 0);

		//-- draw chunks
		terrain->draw ();

		//-- draw grid
		if (showGrid)
			grid.draw (pDC);

		//-- draw boundaries
		if (showBoundaries)
		{
			drawBoundaries (pDC, VectorArgb::solidGray);
			drawSelectedBoundaries (pDC, VectorArgb::solidGreen);
		}

		//-- draw roads
		if (showRoads)
		{
			drawRoads (pDC, VectorArgb::solidRed);
			drawSelectedRoads (pDC, VectorArgb::solidGreen);
		}

		//-- draw rivers
		if (showRivers)
		{
			drawRivers (pDC, VectorArgb::solidBlue);
			drawSelectedRivers (pDC, VectorArgb::solidGreen);
		}

		//-- draw ribbon affectors
		if (showRibbonAffectors)
		{
			drawRibbonAffectors (pDC, VectorArgb::solidBlue);
			drawSelectedRibbonAffectors (pDC, VectorArgb::solidGreen);
		}

		Vector2d zero;
		zero.makeZero ();
		drawCircle_w (pDC, zero, 2.f, VectorArgb::solidWhite);

		//-- draw mode
		if (currentMode != MT_normal)
			modes [currentMode]->render (pDC);

	Graphics::endScene ();
	Graphics::present(m_hWnd, rect.Width (), rect.Height ());
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void MapView::AssertValid() const
{
	CView::AssertValid();
}

void MapView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void MapView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);

	if (lHint == TerrainEditorDoc::H_layerViewApply)
	{
		//-- mark all chunks as dirty
		if (terrain)
			terrain->markDirty ();

		OnRebuild ();
	}

	{
		const TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		const TerrainEditorDoc::Item* const item = doc->getSelectedItem ();

		if (item && item->layerItem)
		{
			BoundaryPolygon* const boundaryPolygon = dynamic_cast<BoundaryPolygon*> (item->layerItem);

			static_cast<ModeModifyPolygon*> (modes [MT_modifyPolygon])->setBoundaryPolygon (boundaryPolygon);

			BoundaryPolyline* const boundaryPolyline = dynamic_cast<BoundaryPolyline*> (item->layerItem);

			static_cast<ModeModifyPolyline*> (modes [MT_modifyPolyline])->setBoundaryPolyline (boundaryPolyline);

			AffectorRiver* const affectorRiver = dynamic_cast<AffectorRiver*> (item->layerItem);

			static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRiver])->setAffectorBoundaryPoly (affectorRiver);

			AffectorRoad* const affectorRoad = dynamic_cast<AffectorRoad*> (item->layerItem);

			static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRoad])->setAffectorBoundaryPoly (affectorRoad);

			AffectorRibbon* const affectorRibbon = dynamic_cast<AffectorRibbon*> (item->layerItem);

			static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRibbonAffector])->setAffectorBoundaryPoly (affectorRibbon);
		}
	}

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnDestroy()
{
	//-- tell the document about our frame
	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	doc->setMapFrame (0);

	CView::OnDestroy();
}

//-------------------------------------------------------------------

void MapView::OnLButtonDown(UINT nFlags, CPoint point)
{
	//-- is the user panning?
	if (!keyDown (VK_MENU))
		modes [currentMode]->onLButtonDown (nFlags, point);

	CView::OnLButtonDown(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void MapView::OnLButtonUp(UINT nFlags, CPoint point)
{
	modes [currentMode]->onLButtonUp (nFlags, point);

	CView::OnLButtonUp(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void MapView::OnMouseMove(UINT nFlags, CPoint point)
{
	SetFocus ();

	modes [currentMode]->onMouseMove (nFlags, point);

	//-- handle panning the map
	{
		if (key_panMap.isDown ())
		{
			SetCapture ();

			if (nFlags & MK_LBUTTON)
			{
				//-- panning
				if (key_panMap.first)
				{
					key_panMap.first     = false;
					key_panMap.lastPoint = point;
				}
				else
				{
					centerOffset += point - key_panMap.lastPoint;
				}

				Invalidate (false);
			}
		}
		else
		{
			key_panMap.first = true;

			if (key_translateBoundary.isDown ())
			{
				setMode (MT_normal);

				SetCapture ();

				if (nFlags & MK_LBUTTON)
				{
					//-- panning
					if (key_translateBoundary.first)
					{
						key_translateBoundary.first     = false;
						key_translateBoundary.lastPoint = point;
					}
					else
					{
						const CPoint translation = point - key_translateBoundary.lastPoint;

						Vector2d translation_w;
						translation_w.x = convertScreenToWorld (translation.x);
#if MAPVIEW_REVERSED
						translation_w.y = -convertScreenToWorld (translation.y);
#else
						translation_w.y = convertScreenToWorld (translation.y);
#endif
						translateBoundaries (translation_w);

						TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
						UNREF (doc);

						doc->UpdateAllViews (this);
					}

					Invalidate (false);
				}
			}
			else
			{
				key_translateBoundary.first = true;

				if (key_scaleBoundary.isDown ())
				{
					setMode (MT_normal);

					SetCapture ();

					if (nFlags & MK_LBUTTON)
					{
						//-- panning
						if (key_scaleBoundary.first)
						{
							key_scaleBoundary.first     = false;
							key_scaleBoundary.lastPoint = point;
						}
						else
						{
							const CPoint translation = point - key_scaleBoundary.lastPoint;

							const real scale = CONST_REAL (1) + (CONST_REAL (0.01) * translation.x);

							scaleBoundaries (scale);

							TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
							UNREF (doc);

							doc->UpdateAllViews (this);
						}

						Invalidate (false);
					}
				}
				else
				{
					key_scaleBoundary.first = true;

					if (key_rotateBoundary.isDown ())
					{
						setMode (MT_normal);

						SetCapture ();

						if (nFlags & MK_LBUTTON)
						{
							//-- panning
							if (key_rotateBoundary.first)
							{
								key_rotateBoundary.first     = false;
								key_rotateBoundary.lastPoint = point;
							}
							else
							{
								const CPoint translation = point - key_rotateBoundary.lastPoint;

								const real rotation = CONST_REAL (0.01) * translation.x;

								rotateBoundaries (rotation);

								TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
								UNREF (doc);

								doc->UpdateAllViews (this);
							}

							Invalidate (false);
						}
					}
					else
					{
						key_rotateBoundary.first = true;

						if (key_rotateBoundaryCentered.isDown ())
						{
							setMode (MT_normal);

							SetCapture ();

							if (nFlags & MK_LBUTTON)
							{
								//-- panning
								if (key_rotateBoundaryCentered.first)
								{
									key_rotateBoundaryCentered.first     = false;
									key_rotateBoundaryCentered.lastPoint = point;
								}
								else
								{
									const CPoint translation = point - key_rotateBoundaryCentered.lastPoint;

									const real rotation = CONST_REAL (0.01) * translation.x;

									rotateCenteredBoundaries (rotation);

									TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
									UNREF (doc);

									doc->UpdateAllViews (this);
								}

								Invalidate (false);
							}
						}
						else
						{
							key_rotateBoundaryCentered.first = true;

							ReleaseCapture ();
						}
					}
				}
			}
		}

		key_panMap.lastPoint                 = point;
		key_translateBoundary.lastPoint      = point;
		key_scaleBoundary.lastPoint          = point;
		key_rotateBoundary.lastPoint         = point;
		key_rotateBoundaryCentered.lastPoint = point;
	}

	//-- handle updating the status bar
	const Vector2d mousePosition_w = convertScreenToWorld (point);

	NOT_NULL (terrain);

	EditorTerrain::Data data;
	IGNORE_RETURN (terrain->getDataAt (mousePosition_w, data));

	GetMainFrame ()->setMousePosition_w (Vector (mousePosition_w.x, data.height, mousePosition_w.y));
	GetMainFrame ()->setShaderFamilyName (data.shaderFamilyName);
	GetMainFrame ()->setFloraStaticCollidableFamilyName  (data.floraStaticCollidableFamilyName);
	GetMainFrame ()->setFloraStaticNonCollidableFamilyName  (data.floraStaticNonCollidableFamilyName);
	GetMainFrame ()->setFloraDynamicNearFamilyName (data.floraDynamicNearFamilyName);
	GetMainFrame ()->setFloraDynamicFarFamilyName (data.floraDynamicFarFamilyName);

	//-- update the ruler
	NOT_NULL (mapFrame);
	mapFrame->updateMousePosition (point);
	mapFrame->updateRulers ();

	CView::OnMouseMove(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void MapView::OnRButtonDown(UINT nFlags, CPoint point)
{
	modes [currentMode]->onRButtonDown (nFlags, point);

	CView::OnRButtonDown(nFlags, point);

	Invalidate ();
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void MapView::OnRButtonUp(UINT nFlags, CPoint point)
{
	modes [currentMode]->onRButtonUp (nFlags, point);

	CView::OnRButtonUp(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

struct BookMark
{
	bool     set;
	Vector2d center_w;
	real     zoom;

	BookMark (void) :
		set (false),
		center_w (0.f, 0.f),
		zoom (1.f)
	{
	}
};

//-------------------------------------------------------------------

void MapView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	modes [currentMode]->onKeyDown (nChar, nRepCnt, nFlags);

	//-- handle bookmarks
	if (nChar >= '0' && nChar <= '9')
	{
		uint index = nChar - '0';

		DEBUG_FATAL (index >= 10, ("Index out of range"));

		static BookMark bookMarks [10];

		if (keyDown (VK_CONTROL))
		{
			CRect rect;
			GetClientRect (&rect);

			bookMarks [index].set      = true;
			bookMarks [index].center_w = convertScreenToWorld (rect.CenterPoint ());
			bookMarks [index].zoom     = zoomLevel;
		}
		else
		{
			if (bookMarks [index].set)
			{
				zoomLevel = bookMarks [index].zoom;
				recenter (bookMarks [index].center_w);

				Invalidate ();
			}
		}
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

//-------------------------------------------------------------------

void MapView::OnCreaterectangle()
{
	setMode (MT_createRectangle);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreaterectangle(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createRectangle ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnCreatecircle()
{
	setMode (MT_createCircle);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreatecircle(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createCircle ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnCreatepolygon()
{
	setMode (MT_createPolygon);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreatepolygon(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createPolygon ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnModifypolygon()
{
	setMode (MT_modifyPolygon);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateModifypolygon(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((static_cast<ModeModifyPolygon*> (modes [MT_modifyPolygon])->getBoundaryPolygon () != 0) ? TRUE : FALSE);
	pCmdUI->SetCheck (currentMode == MT_modifyPolygon);
}

//-------------------------------------------------------------------

void MapView::OnModifypolyline()
{
	setMode (MT_modifyPolyline);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateModifypolyline(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((static_cast<ModeModifyPolyline*> (modes [MT_modifyPolyline])->getBoundaryPolyline () != 0) ? TRUE : FALSE);
	pCmdUI->SetCheck (currentMode == MT_modifyPolyline);
}

//-------------------------------------------------------------------

void MapView::OnCreatePolyline()
{
	setMode (MT_createPolyline);
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreatePolyline(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createPolyline ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnGrid()
{
	showGrid = !showGrid;

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnUpdateGrid(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showGrid);
}

//-------------------------------------------------------------------

void MapView::OnRecenter()
{
	Vector2d zero;
	zero.makeZero ();

	recenter (zero);
}

//-------------------------------------------------------------------

void MapView::OnRebuild()
{
	//-- tell the terrain system to build all visible chunks
	if (terrain)
		terrain->update (true);
}

//-------------------------------------------------------------------

void MapView::OnButtonShowBoundaries()
{
	showBoundaries = !showBoundaries;

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonShowBoundaries(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showBoundaries);
}

//-------------------------------------------------------------------

void MapView::OnViewColormap()
{
	showColorMap = !showColorMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewColormap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showColorMap);
}

//-------------------------------------------------------------------

void MapView::OnViewFloramap()
{
	showFloraMap = !showFloraMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewFloramap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showFloraMap);
}

//-------------------------------------------------------------------

void MapView::OnViewHeightmap()
{
	showHeightMap = !showHeightMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewHeightmap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showHeightMap);
}

//-------------------------------------------------------------------

void MapView::OnViewShadermap()
{
	showShaderMap = !showShaderMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewShadermap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showShaderMap);
}

//-------------------------------------------------------------------

void MapView::OnViewRadialmap()
{
	showRadialMap = !showRadialMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewRadialmap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showRadialMap);
}

//-------------------------------------------------------------------

void MapView::OnViewEnvironmentmap()
{
	showEnvironmentMap = !showEnvironmentMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewEnvironmentmap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showEnvironmentMap);
}

//-------------------------------------------------------------------

void MapView::OnViewPassableMap()
{
	showPassableMap = !showPassableMap;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewPassableMap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showPassableMap);
}

//-------------------------------------------------------------------

BOOL MapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect (&rect);

	CPoint point = pt;
	ScreenToClient (&point);

	const Vector2d center = convertScreenToWorld (point);

	if (zDelta > 0)
		zoomIn (center);
	else
		zoomOut (center);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void MapView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnLighting()
{
	showLighting = !showLighting;
}

//-------------------------------------------------------------------

void MapView::OnUpdateLighting(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showLighting);
}

//-------------------------------------------------------------------

void MapView::OnZoom()
{
	setMode (MT_zoom);
}

//-------------------------------------------------------------------

void MapView::OnUpdateZoom(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (currentMode == MT_zoom ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nFlags & MK_CONTROL && nChar == '-')
		zoomOut ();

	if (nFlags & MK_CONTROL && nChar == '+')
		zoomIn ();

	CView::OnChar(nChar, nRepCnt, nFlags);
}

//-------------------------------------------------------------------

void MapView::OnViewWatermap()
{
	showWater = !showWater;
}

//-------------------------------------------------------------------

void MapView::OnUpdateViewWatermap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showWater);
}

//-------------------------------------------------------------------

void MapView::OnButtonShowProfile()
{
	showProfile = !showProfile;

	//-- clear all chunk profile data
	NOT_NULL (terrain);

	terrain->showProfile (showProfile);

	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonShowProfile(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showProfile);
}

//-------------------------------------------------------------------

void MapView::OnCreateroad()
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	if (doc->getTerrainGenerator ()->getShaderGroup ().getNumberOfFamilies () != 0)
	{
		setMode (MT_createRoad);
		Invalidate ();
	}
	else
	{
		MessageBox ("Please first define at least one shader family.");
	}
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreateroad(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createRoad ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnCreateRibbonAffector()
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	if (doc->getTerrainGenerator ()->getShaderGroup ().getNumberOfFamilies () != 0)
	{
		setMode (MT_createRibbonAffector);
		Invalidate ();
	}
	else
	{
		MessageBox ("Please first define at least one shader family.");
	}

}

//-------------------------------------------------------------------

void MapView::OnUpdateCreateRibbonAffector(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createRibbonAffector ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnButtonShowRoads()
{
	showRoads = !showRoads;

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonShowRoads(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showRoads);
}

//-------------------------------------------------------------------

void MapView::OnButtonShowRibbonAffectors()
{
	showRibbonAffectors = !showRibbonAffectors;

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonShowRibbonAffectors(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showRibbonAffectors);
}

//-------------------------------------------------------------------

void MapView::updateRiversAndRoads (void)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	//
	//-- scan though the affectors and generate a list of chunks to build which possibly intersect rivers/roads
	//
	ArrayList<AffectorBoundaryPoly*> affectorList;
	{
		TerrainGeneratorHelper::findAffectorBoundaryPoly (doc->getTerrainGenerator (), affectorList);
	}

	//-- make sure we have rivers and/or roads
	if (affectorList.getNumberOfElements () == 0)
		return;

	ArrayList<EditorTerrain::MetaData> metaDataList;
	metaDataList.preallocate(4096); // preallocate so it doesn't grow - causes corruption
	{
		int i;
		for (i = 0; i < affectorList.getNumberOfElements (); ++i)
		{
			AffectorBoundaryPoly* affector = affectorList [i];

			if (affector->isActive ())
			{
				EditorTerrain::MetaData metaData;
				metaData.extent = affector->getExtent ();
				metaData.name   = affector->getName ();
				metaData.width  = affector->getWidth ();

				int j;
				for (j = 0; j < affector->getNumberOfPoints () - 1; ++j)
				{
					const Vector2d start = affector->getPoint (j);
					const Vector2d end   = affector->getPoint (j + 1);
					const Vector   v0 (start.x, 0.f,  start.y);
					const Vector   v1 (start.x, 10.f, start.y);
					const Vector   v2 (end.x,   0.f,  end.y);

					Plane plane (v0, v1, v2);
					metaData.planeList.add (plane);
				}

				metaDataList.add (metaData);
			}
		}
	}

	//-- build the terrain for those roads
	const real oldZoom = zoomLevel;
	setZoom (1.f);

	if (terrain->buildHeightData (metaDataList))
	{
		//
		//-- run through the rivers generating the meta data for the rivers
		//
		{
			const real desiredLength = 8.f;

			int i;
			for (i = 0; i < affectorList.getNumberOfElements (); ++i)
			{
				AffectorBoundaryPoly* affector = affectorList [i];

				if(affector->getType () == TGAT_ribbon) // ribbons don't need height data
				{
					continue;
				}

				affector->clearHeightData ();

				int j;
				for (j = 0; j < affector->getNumberOfPoints () - 1; ++j)
				{
					//-- add a segment
					affector->addSegmentHeightData ();

					//-- walk the segment adding samples
					const Vector2d segmentStart     = affector->getPoint (j);
					const Vector2d segmentEnd       = affector->getPoint (j + 1);
					float          segmentLength    = segmentEnd.magnitudeBetween (segmentStart);
					const int      numberOfSegments = 1 + static_cast<int> (segmentLength / desiredLength);
					float          stepLength       = segmentLength / static_cast<float> (numberOfSegments);

					const Vector2d toEnd   = Vector2d::normalized (segmentEnd - segmentStart) * stepLength;

					Vector2d current = segmentStart;

					int i = 0;
					for (i = 0; i < numberOfSegments - 1; ++i)
					{
						affector->addPointHeightData (Vector (current.x, terrain->getHeightDataHeightAt (current), current.y));

						current += toEnd;
						segmentLength -= stepLength;
					}

					current = segmentEnd;
					affector->addPointHeightData (Vector (current.x, terrain->getHeightDataHeightAt (current), current.y));
				}

				affector->createHeightData ();
			}
		}
	}

	setZoom (oldZoom);
}

//-------------------------------------------------------------------

void MapView::OnCreateriver()
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	if (doc->getTerrainGenerator ()->getShaderGroup ().getNumberOfFamilies () != 0)
	{
		setMode (MT_createRiver);
		Invalidate ();
	}
	else
	{
		MessageBox ("Please first define at least one shader family.");
	}
}

//-------------------------------------------------------------------

void MapView::OnUpdateCreateriver(CCmdUI* pCmdUI)
{
	const TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	pCmdUI->Enable (doc->canAddBoundary ());
	pCmdUI->SetCheck (currentMode == MT_createRiver ? TRUE : FALSE);
}

//-------------------------------------------------------------------

void MapView::OnButtonShowRivers()
{
	showRivers = !showRivers;

	Invalidate (false);
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonShowRivers(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (showRivers);
}

//-------------------------------------------------------------------

const Vector2d MapView::getCenter (void) const
{
	CRect rect;
	GetClientRect (&rect);

	return convertScreenToWorld (rect.CenterPoint ());
}

//-------------------------------------------------------------------

void MapView::OnModifyriver()
{
	setMode (MT_modifyRiver);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateModifyriver(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRiver])->getAffectorBoundaryPoly () != 0) ? TRUE : FALSE);
	pCmdUI->SetCheck (currentMode == MT_modifyRiver);
}

//-------------------------------------------------------------------

void MapView::OnModifyroad()
{
	setMode (MT_modifyRoad);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateModifyroad(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRoad])->getAffectorBoundaryPoly () != 0) ? TRUE : FALSE);
	pCmdUI->SetCheck (currentMode == MT_modifyRoad);
}

//-------------------------------------------------------------------

void MapView::OnModifyRibbonAffector()
{
	setMode (MT_modifyRibbonAffector);
	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateModifyRibbonAffector(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ((static_cast<ModeModifyAffectorBoundaryPoly*> (modes [MT_modifyRibbonAffector])->getAffectorBoundaryPoly () != 0) ? TRUE : FALSE);
	pCmdUI->SetCheck (currentMode == MT_modifyRibbonAffector);
}

//-------------------------------------------------------------------

void MapView::OnButtonHighres()
{
	terrain->setResolutionType (EditorTerrain::RT_high);

	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonHighres(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (terrain->getResolutionType () == EditorTerrain::RT_high);
}

//-------------------------------------------------------------------

void MapView::OnButtonMedres()
{
	terrain->setResolutionType (EditorTerrain::RT_medium);

	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonMedres(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (terrain->getResolutionType () == EditorTerrain::RT_medium);
}

//-------------------------------------------------------------------

void MapView::OnButtonLowres()
{
	terrain->setResolutionType (EditorTerrain::RT_low);

	Invalidate ();
}

//-------------------------------------------------------------------

void MapView::OnUpdateButtonLowres(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (terrain->getResolutionType () == EditorTerrain::RT_low);
}

//-------------------------------------------------------------------

void MapView::bakeTerrain ()
{
	terrain->bakeTerrain ();
}

//-------------------------------------------------------------------

