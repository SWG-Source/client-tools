// =====================================================================
//
// OverheadMap.h
// asommers
//
// copyright 2003, sony online entertainment
//
// =====================================================================

#ifndef INCLUDED_OverheadMap_H
#define INCLUDED_OverheadMap_H

// =====================================================================

#include "UIEventCallback.h"
#include "clientGame/PlanetMapManagerClient.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/Vector.h"

class Camera;
class CellProperty;
class CreatureObject;
class IndexedTriangleList;

// =====================================================================

namespace MessageDispatch
{
	class Callback;
}

// =====================================================================

class OverheadMap :
public UIEventCallback
{
public:

	typedef stdvector<Vector>::fwd PointList;

public:

	OverheadMap ();
	~OverheadMap ();

	bool getRenderMap () const;
	void toggle ();
	void zoom (bool in);
	void update (float elapsedTime);
	void setClientPath (PointList const & clientPath);
	void clearClientPath ();
	void render () const;

protected:

	void onLocationsReceived (const PlanetMapManagerClient::Messages::LocationsReceived::Payload & msg);

private:

	OverheadMap (const OverheadMap&);
	OverheadMap& operator= (const OverheadMap&);

	void refreshLocationList () const;
	void renderCreatureOnMap (const CreatureObject & creatureObject, const Vector & zoom_vector) const;

	void avoidScreenOverlap(Vector & location) const;

private:

	bool                    m_renderMap;
	Camera*                 m_radarCamera;
	int                     m_radarZoomSetting;
	float                   m_normalizedRadarZoom;
	float                   m_radarZoom;
	IndexedTriangleList*    m_creatureIndexedTriangleList;
	IndexedTriangleList*    m_playerIndexedTriangleList;
	IndexedTriangleList*    m_waypointIndexedTriangleList;
	IndexedTriangleList*	m_waypointEntranceIndexedTriangleList;

	typedef stdvector<const CellProperty*>::fwd CellPropertyList;
	CellPropertyList* const m_radarCellList;	
	mutable CellProperty const * m_cachedStartCell;

	PointList * const m_clientPath;

	class Location;
	typedef stdvector<Location*>::fwd LocationList;
	mutable LocationList* m_locationList;

	mutable Vector m_lastPlayerPosition_w;

	mutable Timer m_timer;

	PointList * const m_screenOverlapLocations;

	MessageDispatch::Callback * m_callback;
};

// =====================================================================

#endif
