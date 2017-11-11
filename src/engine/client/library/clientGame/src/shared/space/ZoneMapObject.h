//======================================================================
//
// ZoneMapObject.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ZoneMapObject_H
#define INCLUDED_ZoneMapObject_H

//======================================================================

#include "sharedObject/Object.h"

#include <map>
#include <vector>

//======================================================================

class ShipObject;
class Ray3d;

//======================================================================

class ZoneMapObject : public Object
{
public:
	class ObjectData
	{
	public:
		enum DataType
		{
			DT_asteroid,
			DT_nebula,
			DT_hyperspacePoint,
			DT_spaceStation,
			DT_wayPoint,
			DT_missionCritical
		};

	public:
		ObjectData();
		ObjectData(DataType const type, Object * object, Transform const & transform_w);
		ObjectData(DataType const type, Object * object, Unicode::String const & localizedName, Transform const & transform_w);
		~ObjectData();

		DataType getDataType() const;
		Object * getObject() const;
		Unicode::String const & getLocalizedName() const;
		void setLocalizedName(Unicode::String const & localizedName);
		Transform const & getTransform_w() const;
		void setTransform_w(Transform const & transform_w);
		int getSimpleIdentifierKey() const;

	private:
		DataType m_type;
		Watcher<Object> m_object;
		Unicode::String m_localizedName;
		Transform m_transform_w;

		// this is used to help identify objects that do not have a networkId
		int m_simpleIdentifierKey;
	};

	// DynamicZoneObjects are objects that will continually be created
	// destroyed, renamed, and have their position updated
	typedef std::map<NetworkId, ObjectData> DynamicZoneObjects;
	// StaticZoneObjects are objects that will be created durring initial
	// population and will never be modified once populated
	typedef std::vector<ObjectData> StaticZoneObjects;
	typedef std::vector<Watcher<Object> > ObjectWatchList;

	// this is a set to allow compare*Ids to find referenced Ids more quickly
	typedef stdset<NetworkId>::fwd  NetworkIds;

	typedef std::pair<Unicode::String, Vector> TextAndLocation;
	typedef std::vector<TextAndLocation> TextAndLocationVector;

	enum WaypointGroup
	{
		WG_active = 0,
		WG_inactive,
		WG_activePoi,
		WG_inactivePoi,
		WG_numberOfWaypointGroups
	};

public:
	ZoneMapObject(std::string const & sceneId, ShipObject const * owningShip = 0);
	virtual ~ZoneMapObject();

	static void install();

	// colors and appearance
	static void setDefaultMembraneCustomizationFields(int const index1, int const index2);
	static void setShipMembraneCustomizationFields(int const index1, int const index2);
	static void setSelectMembraneCustomizationFields(int const index1, int const index2);
	static void setActiveWaypointMembraneCustomizationFields(int const index1, int const index2);
	static void setInactiveWaypointMembraneCustomizationFields(int const index1, int const index2);
	static void setActivePoiWaypointMembraneCustomizationFields(int const index1, int const index2);
	static void setInactivePoiWaypointMembraneCustomizationFields(int const index1, int const index2);
	static void setMissionCriticalMembraneCustomizationFields(int const index1, int const index2);
	static void setSpaceStationMembraneCustomizationFields(int const index1, int const index2);
	static void setHyperspaceMembraneCustomizationFields(int const index1, int const index2);
	static void setAsteroidMembraneCustomizationFields(int const index1, int const index2);
	static TextAndLocation makeTextAndLocation(ObjectData const & objectData); 	

	// zone static data
	static void createAsteroids(ZoneMapObject * zone);
	static void createNebulas(ZoneMapObject * zone);
	static void createHyperspacePoints(ZoneMapObject * zone);
	static void createSpaceStations(ZoneMapObject * zone);

	// zone player-based dynamic data
	// call when data is either added or removed
	static void modifyWaypoints(ZoneMapObject * zone);
	static void modifyMissionCriticals(ZoneMapObject * zone);
	static void updateMissionCriticalLocations(ZoneMapObject * zone);

	std::string const & getSceneId() const;

	// These are not static because these use m_actualZoneSize which
	// may not (in the future) be the same for different zones

	// transform world space to zone object space
	Transform worldTransformToZoneTransform(Transform const & worldTransform) const;
	Vector worldLocationToZoneLocation(Vector const & worldLocation) const;
	float worldScaleToZoneScale(float const worldScale) const;
	Vector worldScaleToZoneScale(Vector const & worldScale) const;

	// transform zone object space to world space
	Vector zoneLocationToWorldLocation(Vector const & zoneLocation) const;

	bool isPointInZone(Vector const & pointInZoneSpace) const;

	// add the static zone data
	void addAsteroid(Transform const & transform_w, float const scale_w = 1.0f);
	void addNebula(Transform const & transform_w, float const scale_w = 1.0f);
	void addHyperspacePoint(Unicode::String const & localizedName, Transform const & transform_w);
	void addSpaceStation(Unicode::String const & localizedName, Transform const & transform_w);

	// dynamic data is data that is driven by player preferences and state which
	// could cause them to be added and removed during the lifetime of this instance
	void compareWaypointIds(WaypointGroup const & group, NetworkIds const & currentWaypoints, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const;
	void addWaypoint(WaypointGroup const & group, NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w);
	void removeWaypoint(WaypointGroup const & group, NetworkId const & networkId);
	void updateWaypoint(WaypointGroup const & group, NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w);

	void compareMissionCriticalIds(NetworkIds const & currentMissionCriticals, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const;
	void addMissionCritical(NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w);
	void removeMissionCritical(NetworkId const & networkId);
	void updateMissionCritical(NetworkId const & networkId, Transform const & transform_w);

	StaticZoneObjects const & getAsteroids() const;
	StaticZoneObjects const & getNebulas() const;
	StaticZoneObjects const & getHyperspacePoints() const;
	StaticZoneObjects const & getSpaceStations() const;
	DynamicZoneObjects const & getActiveWaypoints() const;
	DynamicZoneObjects const & getInactiveWaypoints() const;
	DynamicZoneObjects const & getActivePoiWaypoints() const;
	DynamicZoneObjects const & getInactivePoiWaypoints() const;
	DynamicZoneObjects const & getMissionCriticals() const;
	TextAndLocationVector const & getAllTextAndLocations() const;

	void showAsteroid(bool show);
	void showNebula(bool show);
	void showHyperspacePoint(bool show);
	void showSpaceStations(bool show);
	void showWaypoint(bool show);
	void showMissionCritical(bool show);
	void showPlayerShip(bool show);
	void showLookAtFocus(bool show);

	void scaleContents(Vector const & scale);
	void highlightTarget(Object * target, bool setFocusTo = false);
	Vector getHighLightedTargetLocationInZoneSpace() const;
	Vector getLookAtTargetLocationInZoneSpace() const;
	void setLookAtTargetLocationInZoneSpace(Vector const & position_o);
	bool isFocusedOnHighLightedTarget() const;

	ObjectData * closestIntersectedObject(Ray3d const & ray_o) const;

	virtual float alter(float elapsedTime);

private:

	ZoneMapObject();
	ZoneMapObject(ZoneMapObject const &);
	ZoneMapObject & operator=(const ZoneMapObject &);

	void clearObjectsFromRoot(bool deleteTheObjects);
	void addObjectToRoot(Object * child, Transform const & transform_w);
	void addObjectsToRoot(DynamicZoneObjects const & objects);
	void addObjectsToRoot(StaticZoneObjects const & objects);
	void deleteObjects(DynamicZoneObjects & objects);
	void deleteObjects(StaticZoneObjects & objects);
	void scaleObjects(DynamicZoneObjects & objects, Vector const & scale);
	void scaleObjects(StaticZoneObjects & objects, Vector const & scale);
	ObjectData * closestIntersectedObject(DynamicZoneObjects const & objects, Ray3d const & ray_p, float & bestLength) const;
	ObjectData * closestIntersectedObject(StaticZoneObjects const & objects, Ray3d const & ray_p, float & bestLength) const;

	// currentIds is a set of network ids that are gathered from an authoritative outside
	//   source.  An example are the wayPoints that are dictated by the server.
	// toAdd are network Ids of objects not represented in this instance
	// toRemove are network Ids of objects that are not referenced in currentIds
	//   and should be removed from this instance
	// toUpdate are the network Ids of objects that are still referenced by this
	//   instance and possibly need updating, etc.
	void compareDynamicIds(DynamicZoneObjects const & dynamicZoneObjects, NetworkIds const & currentIds, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const;

	void addDynamic(DynamicZoneObjects & dynamicZoneObjects, ObjectData::DataType type, Object * child, NetworkId const & networkId, Unicode::String const & localizedName, const Transform & transform_w) const;
	void removeDynamic(DynamicZoneObjects & dynamicZoneObjects, ObjectWatchList & toDelete, NetworkId const & networkId) const;
	void updateDynamic(DynamicZoneObjects & dynamicZoneObjects, NetworkId const & networkId, Unicode::String const & localizedName, const Transform & transform_w) const;
	void updateDynamic(DynamicZoneObjects & dynamicZoneObjects, NetworkId const & networkId, const Transform & transform_w) const;

private:
	std::string m_sceneId;
	float m_actualZoneSize;

	ConstWatcher<Object> m_owningShip;
	Watcher<Object> m_player;
	Watcher<Object> m_focus;
	Watcher<Object> m_target; 
	Watcher<Object> m_targetSelection;
	StaticZoneObjects m_asteroids;
	StaticZoneObjects m_nebulas;
	StaticZoneObjects m_hyperspacePoints;
	StaticZoneObjects m_spaceStations;
	DynamicZoneObjects m_activeWaypoints;
	DynamicZoneObjects m_inactiveWaypoints;
	DynamicZoneObjects m_activePoiWaypoints;
	DynamicZoneObjects m_inactivePoiWaypoints;
	DynamicZoneObjects m_missionCriticals;

	bool m_dirty;
	bool m_renderAsteroids;
	bool m_renderNebulas;
	bool m_renderHyperspacePoints;
	bool m_renderSpaceStations;
	bool m_renderWaypoints;
	bool m_renderMissionCriticals;
	bool m_renderPlayerShip;
	bool m_renderFocus;
	bool m_focusCameraOnTarget;

	Vector m_scale;

	ObjectWatchList m_objectsToDeleteIfDirty;

	TextAndLocationVector m_textAndLocations;
};

//======================================================================

#endif
