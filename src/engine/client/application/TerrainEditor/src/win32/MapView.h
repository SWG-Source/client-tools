//
// MapView.h
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#ifndef MAPVIEW_H
#define MAPVIEW_H

//-------------------------------------------------------------------

class AffectorBoundaryPoly;
class BoundaryPolygon;
class BoundaryPolyline;
class EditorTerrain;
class MapFrame;
class TerrainEditorDoc;

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "EditorTerrain.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

#define MAPVIEW_REVERSED 1

//-------------------------------------------------------------------

class MapView : public CView
{
private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Grid
	{
	private:

		const MapView* mapView;
		int            gridDistance;

	public:

		Grid (void);
		~Grid (void);

		void setMapView (const MapView* newMapView);
		void setGridDistance (int newGridDistance);

		void draw (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class KeyState
	{
	private:

		uint16 key;

	public:

		bool   first;      //lint !e1925 //-- public data member
		CPoint lastPoint;  //lint !e1925 //-- public data member

	public:

		KeyState (void) :
			key (0),
			first (true),
			lastPoint (0, 0)
		{
		}

		void bind (uint16 newKey)
		{
			key = newKey;
		}

		bool isDown (void) const
		{
			return (GetKeyState (key) & 0x8000) != 0;
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef void (MapView:: *AddPolylineFunction) (const ArrayList<Vector2d>& newPointList);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Mode
	{
	protected:

		MapView* const mapView;
		bool           originated;

	private:

		Mode (void);

	public:

		explicit Mode (MapView* const newMapView);
		virtual ~Mode (void)=0;

		virtual void onCancel (void);
		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onRButtonDown (UINT flags, const CPoint& point);
		virtual void onRButtonUp   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeNormal : public Mode
	{
	private:

		ModeNormal (void);

	public:

		explicit ModeNormal (MapView* const newMapView);
		virtual ~ModeNormal (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeCreateCircle : public Mode
	{
	private:

		Vector2d start;
		Vector2d end;

		bool     drawing;

	private:

		ModeCreateCircle (void);

	public:

		explicit ModeCreateCircle (MapView* const newMapView);
		virtual ~ModeCreateCircle (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void render (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeCreateRectangle : public Mode
	{
	private:

		Vector2d start;
		Vector2d end;

		bool     drawing;

	private:

		ModeCreateRectangle (void);

	public:

		explicit ModeCreateRectangle (MapView* const newMapView);
		virtual ~ModeCreateRectangle (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void render (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeCreatePolygon : public Mode
	{
	private:

		ArrayList<Vector2d> pointList;
		Vector2d            end;

		bool                drawing;

	private:

		ModeCreatePolygon (void);

	public:

		explicit ModeCreatePolygon (MapView* const newMapView);
		virtual ~ModeCreatePolygon (void);

		virtual void onCancel (void);
		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeCreatePolyline : public Mode
	{
	private:

		ArrayList<Vector2d> pointList;
		Vector2d            end;

		bool                drawing;

		AddPolylineFunction m_addPolylineFunction;

	private:

		ModeCreatePolyline (void);

	public:

		explicit ModeCreatePolyline (MapView* const newMapView, AddPolylineFunction addPolylineFunction);
		virtual ~ModeCreatePolyline (void);

		virtual void onCancel (void);
		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeZoom : public Mode
	{
	private:

		ModeZoom (void);

	public:

		explicit ModeZoom (MapView* const newMapView);
		virtual ~ModeZoom (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeModifyPolygon : public Mode
	{
	private:

		BoundaryPolygon* boundaryPolygon;
		int              selectedIndex;

		bool             dragging;
		Vector2d         start;

	private:

		ModeModifyPolygon (void);

	public:

		explicit ModeModifyPolygon (MapView* const newMapView);
		virtual ~ModeModifyPolygon (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onRButtonDown (UINT flags, const CPoint& point);
		virtual void onRButtonUp   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;

		void                   setBoundaryPolygon (BoundaryPolygon* newBoundaryPolygon);
		const BoundaryPolygon* getBoundaryPolygon (void) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeModifyPolyline : public Mode
	{
	private:

		BoundaryPolyline* boundaryPolyline;
		int               selectedIndex;

		bool             dragging;
		Vector2d         start;

	private:

		ModeModifyPolyline (void);

	public:

		explicit ModeModifyPolyline (MapView* const newMapView);
		virtual ~ModeModifyPolyline (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onRButtonDown (UINT flags, const CPoint& point);
		virtual void onRButtonUp   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;

		void                   setBoundaryPolyline (BoundaryPolyline* newBoundaryPolyline);
		const BoundaryPolyline* getBoundaryPolyline (void) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ModeModifyAffectorBoundaryPoly : public Mode
	{
	private:

		AffectorBoundaryPoly* affectorBoundaryPoly;
		int                   selectedIndex;

		bool                  dragging;
		Vector2d              start;

	private:

		ModeModifyAffectorBoundaryPoly (void);

	public:

		explicit ModeModifyAffectorBoundaryPoly (MapView* const newMapView);
		virtual ~ModeModifyAffectorBoundaryPoly (void);

		virtual void onLButtonDown (UINT flags, const CPoint& point);
		virtual void onLButtonUp   (UINT flags, const CPoint& point);
		virtual void onMouseMove   (UINT flags, const CPoint& point);
		virtual void onRButtonDown (UINT flags, const CPoint& point);
		virtual void onRButtonUp   (UINT flags, const CPoint& point);
		virtual void onKeyDown     (UINT character, UINT repeatCount, UINT flags);
		virtual void render (CDC* pDC) const;

		void                        setAffectorBoundaryPoly (AffectorBoundaryPoly* newAffectorBoundaryPoly);
		const AffectorBoundaryPoly* getAffectorBoundaryPoly(void) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	enum ModeType
	{
		MT_normal,
		MT_zoom,
		MT_createCircle,
		MT_createRectangle,
		MT_createPolygon,
		MT_modifyPolygon,
		MT_createPolyline,
		MT_modifyPolyline,
		MT_createRoad,
		MT_modifyRoad,
		MT_createRiver,
		MT_modifyRiver,
		MT_createRibbonAffector,
		MT_modifyRibbonAffector,

		MT_count
	};

	enum CursorType
	{
		CT_pointer,
		CT_circle,
		CT_rectangle,
		CT_polygon,
		CT_polyline,
		CT_zoomIn,
		CT_zoomOut,

		CT_count
	};

private:

	MapFrame*      mapFrame;

	Mode*          modes [MT_count];
	ModeType       currentMode;

	HCURSOR        cursors [CT_count];
	CPoint         centerOffset;

	Grid           grid;

	EditorTerrain* terrain;

	real           zoomLevel;
	real           zoomMin;
	real           zoomMax;

	bool           showBoundaries;
	bool           showGrid;
	bool           showLighting;
	bool           showHeightMap;
	bool           showColorMap;
	bool           showShaderMap;
	bool           showFloraMap;
	bool           showRadialMap;
	bool           showWater;
	bool           showProfile;
	bool           showRoads;
	bool           showRivers;
	bool           showEnvironmentMap;
	bool           showPassableMap;
	bool           showRibbonAffectors;

	KeyState       key_panMap;
	KeyState       key_rotateBoundary;
	KeyState       key_translateBoundary;
	KeyState       key_scaleBoundary;
	KeyState       key_rotateBoundaryCentered;

private:

	void           rotateBoundaries (real angle) const;
	void           rotateLayer (real angle, TerrainGenerator::Layer* layer) const;
	void           rotateBoundary (real angle, TerrainGenerator::Boundary* boundary) const;
	void           rotateAffector (real angle, TerrainGenerator::Affector* affector) const;

	void           rotateCenteredBoundaries (real angle) const;
	void           rotateCenteredLayer (real angle, TerrainGenerator::Layer* layer, bool useCenter, const Vector2d& center) const;
	void           rotateCenteredBoundary (real angle, TerrainGenerator::Boundary* boundary, bool useCenter, const Vector2d& center) const;
	void           rotateCenteredAffector (real angle, TerrainGenerator::Affector* affector, bool useCenter, const Vector2d& center) const;

	void           translateBoundaries (const Vector2d& translation_w) const;
	void           translateLayer (const Vector2d& translation_w, TerrainGenerator::Layer* layer) const;
	void           translateBoundary (const Vector2d& translation_w, TerrainGenerator::Boundary* boundary) const;
	void           translateAffector (const Vector2d& translation_w, TerrainGenerator::Affector* affector) const;

	void           scaleBoundaries (real scalar_w) const;
	void           scaleLayer (real scalar_w, TerrainGenerator::Layer* layer) const;
	void           scaleBoundary (real scalar_w, TerrainGenerator::Boundary* boundary) const;
	void           scaleAffector (real scalar_w, TerrainGenerator::Affector* affector) const;

	void           drawSelectedBoundaries (CDC* pDC, const VectorArgb& color) const;
	void           drawBoundaries (CDC* pDC, const VectorArgb& color, bool forceDraw=false) const;
	void           drawLayer      (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw=false) const;
	void           drawBoundary   (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Boundary* boundary, bool forceDraw=false) const;
	void           drawSelectedRoads (CDC* pDC, const VectorArgb& color) const;
	void           drawRoad       (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw=false) const;
	void           drawRoads      (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw=false) const;
	void           drawRoads      (CDC* pDC, const VectorArgb& color, bool forceDraw=false) const;
	void           drawSelectedRivers (CDC* pDC, const VectorArgb& color) const;
	void           drawRiver      (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw=false) const;
	void           drawRivers     (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw=false) const;
	void           drawRivers     (CDC* pDC, const VectorArgb& color, bool forceDraw=false) const;
	void           drawSelectedRibbonAffectors (CDC* pDC, const VectorArgb& color) const;
	void           drawRibbonAffector       (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Affector* affector, bool forceDraw=false) const;
	void           drawRibbonAffectors      (CDC* pDC, const VectorArgb& color, const TerrainGenerator::Layer* layer, bool forceDraw=false) const;
	void           drawRibbonAffectors      (CDC* pDC, const VectorArgb& color, bool forceDraw=false) const;

	void           zoomIn (void);
	void           zoomOut (void);

private:

	MapView (const MapView&);
	MapView& operator= (const MapView&);

protected:

	MapView (void);
	DECLARE_DYNCREATE(MapView)
	virtual ~MapView (void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_VIRTUAL(MapView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(MapView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnCreaterectangle();
	afx_msg void OnUpdateCreaterectangle(CCmdUI* pCmdUI);
	afx_msg void OnCreatecircle();
	afx_msg void OnUpdateCreatecircle(CCmdUI* pCmdUI);
	afx_msg void OnCreatepolygon();
	afx_msg void OnUpdateCreatepolygon(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGrid();
	afx_msg void OnUpdateGrid(CCmdUI* pCmdUI);
	afx_msg void OnRecenter();
	afx_msg void OnButtonShowBoundaries();
	afx_msg void OnUpdateButtonShowBoundaries(CCmdUI* pCmdUI);
	afx_msg void OnViewColormap();
	afx_msg void OnUpdateViewColormap(CCmdUI* pCmdUI);
	afx_msg void OnViewFloramap();
	afx_msg void OnUpdateViewFloramap(CCmdUI* pCmdUI);
	afx_msg void OnViewHeightmap();
	afx_msg void OnUpdateViewHeightmap(CCmdUI* pCmdUI);
	afx_msg void OnViewShadermap();
	afx_msg void OnUpdateViewShadermap(CCmdUI* pCmdUI);
	afx_msg void OnRebuild();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLighting();
	afx_msg void OnUpdateLighting(CCmdUI* pCmdUI);
	afx_msg void OnZoom();
	afx_msg void OnUpdateZoom(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCreatePolyline();
	afx_msg void OnUpdateCreatePolyline(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewRadialmap();
	afx_msg void OnUpdateViewRadialmap(CCmdUI* pCmdUI);
	afx_msg void OnViewWatermap();
	afx_msg void OnUpdateViewWatermap(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowProfile();
	afx_msg void OnUpdateButtonShowProfile(CCmdUI* pCmdUI);
	afx_msg void OnModifypolygon();
	afx_msg void OnUpdateModifypolygon(CCmdUI* pCmdUI);
	afx_msg void OnModifypolyline();
	afx_msg void OnUpdateModifypolyline(CCmdUI* pCmdUI);
	afx_msg void OnCreateroad();
	afx_msg void OnUpdateCreateroad(CCmdUI* pCmdUI);
	afx_msg void OnCreateRibbonAffector();
	afx_msg void OnUpdateCreateRibbonAffector(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowRoads();
	afx_msg void OnUpdateButtonShowRoads(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowRibbonAffectors();
	afx_msg void OnUpdateButtonShowRibbonAffectors(CCmdUI* pCmdUI);
	afx_msg void OnCreateriver();
	afx_msg void OnUpdateCreateriver(CCmdUI* pCmdUI);
	afx_msg void OnButtonShowRivers();
	afx_msg void OnUpdateButtonShowRivers(CCmdUI* pCmdUI);
	afx_msg void OnModifyriver();
	afx_msg void OnUpdateModifyriver(CCmdUI* pCmdUI);
	afx_msg void OnModifyroad();
	afx_msg void OnUpdateModifyroad(CCmdUI* pCmdUI);
	afx_msg void OnModifyRibbonAffector();
	afx_msg void OnUpdateModifyRibbonAffector(CCmdUI* pCmdUI);
	afx_msg void OnViewEnvironmentmap();
	afx_msg void OnUpdateViewEnvironmentmap(CCmdUI* pCmdUI);
	afx_msg void OnViewPassableMap();
	afx_msg void OnUpdateViewPassableMap(CCmdUI* pCmdUI);
	afx_msg void OnButtonHighres();
	afx_msg void OnUpdateButtonHighres(CCmdUI* pCmdUI);
	afx_msg void OnButtonMedres();
	afx_msg void OnUpdateButtonMedres(CCmdUI* pCmdUI);
	afx_msg void OnButtonLowres();
	afx_msg void OnUpdateButtonLowres(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void           setMapFrame (MapFrame* newMapFrame);
	void           setMode (ModeType newMode);
	void           setCursor (CursorType newCursorType);

	const Vector2d convertScreenToWorld (const CPoint& point) const;
	real           convertScreenToWorld (int distance) const;
	const CPoint   convertWorldToScreen (const Vector2d& vector2d) const;
	int            convertWorldToScreen (real distance) const;

	void           drawCircle_w    (CDC* pDC, const Vector2d& center, real radius, const VectorArgb& color) const;
	void           drawRectangle_w (CDC* pDC, const Vector2d& start, const Vector2d& end, const VectorArgb& color) const;
	void           drawPolygon_w   (CDC* pDC, const ArrayList<Vector2d>& pointList, const VectorArgb& color) const;
	void           drawPolyline_w  (CDC* pDC, const ArrayList<Vector2d>& pointList, const float width, const VectorArgb& color) const;
	void           drawLine_w      (CDC* pDC, const Vector2d& start, const Vector2d& end, const VectorArgb& color) const;

	void           addBoundaryRectangle (const Rectangle2d& newRectangle);
	void           addBoundaryCircle    (const Vector2d& newCenter, real newRadius);
	void           addBoundaryPolygon   (const ArrayList<Vector2d>& newPointList);
	void           addBoundaryPolyline  (const ArrayList<Vector2d>& newPointList);
	void           addAffectorRoad      (const ArrayList<Vector2d>& newPointList);
	void           addAffectorRiver     (const ArrayList<Vector2d>& newPointList);
	void           addAffectorRibbon      (const ArrayList<Vector2d>& newPointList);

	void           recenter (const Vector2d& center_w);

	bool           getShowHeightMap (void) const;
	bool           getShowColorMap (void) const;
	bool           getShowShaderMap (void) const;
	bool           getShowFloraMap (void) const;
	bool           getShowRadialMap (void) const;
	bool           getShowEnvironmentMap (void) const;
	bool           getShowLighting (void) const;
	bool           getShowWater (void) const;
	bool           getShowProfile (void) const;
	bool           getShowPassableMap (void) const;

	real           getZoomLevel (void) const;

	void           setZoom (real newZoomLevel=1.f);
	void           zoomIn (const Vector2d& center);
	void           zoomOut (const Vector2d& center);

	void           updateRiversAndRoads (void);
	void           bakeTerrain ();

	const Vector2d getCenter (void) const;
};

//-------------------------------------------------------------------

inline void MapView::setMapFrame (MapFrame* newMapFrame)
{
	mapFrame = newMapFrame;
}

//-------------------------------------------------------------------

inline bool MapView::getShowHeightMap (void) const
{
	return showHeightMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowColorMap (void) const
{
	return showColorMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowShaderMap (void) const
{
	return showShaderMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowFloraMap (void) const
{
	return showFloraMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowRadialMap (void) const
{
	return showRadialMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowEnvironmentMap (void) const
{
	return showEnvironmentMap;
}

//-------------------------------------------------------------------

inline bool MapView::getShowLighting (void) const
{
	return showLighting;
}

//-------------------------------------------------------------------

inline bool MapView::getShowWater (void) const
{
	return showWater;
}

//-------------------------------------------------------------------

inline bool MapView::getShowProfile (void) const
{
	return showProfile;
}

//-------------------------------------------------------------------

inline real MapView::getZoomLevel (void) const
{
	return zoomLevel;
}

//----------------------------------------------------------------------

inline bool MapView::getShowPassableMap (void) const
{
	return showPassableMap;
}

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif
