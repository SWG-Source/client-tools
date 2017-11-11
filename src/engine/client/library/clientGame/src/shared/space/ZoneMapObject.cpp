//======================================================================
//
// ZoneMapObject.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ZoneMapObject.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ZoneMapSpaceStationTable.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/ExtentAppearance.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedGame/AsteroidGenerationManager.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedGame/NebulaManager.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Ray3d.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PulseDynamics.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace ZoneMapObjectNamespace
{
	float const cs_scaledZoneSize = 40.0f;

	char const * const cs_zoneAppearanceFileName = "appearance/pt_ui_zonemap_grid_01.prt";
	char const * const cs_hyperspacePointAppearanceFileName = "appearance/ui_hyperspace.apt";
	char const * const cs_spaceStationAppearanceFileName = "appearance/ui_spacestation.apt";
	char const * const cs_nebulaAppearanceFileName = "appearance/pt_nebula_display01.prt";
	char const * const cs_missionCriticalAppearanceFileName = "appearance/ui_mission_critical.apt";
	char const * const cs_focusAppearanceFileName = "appearance/ui_camera.apt";
	char const * const cs_shipAppearanceFileName = "appearance/ui_ship.apt";
	char const * const cs_selectionAppearanceFileName = "appearance/ui_select.apt";

	int const cs_numberOfAsteroidAppearanceFileNames = 3;
	char const * const cs_asteroidAppearanceFileNames[cs_numberOfAsteroidAppearanceFileNames] =
							{
								"appearance/ui_asteroid_big.apt",
								"appearance/ui_asteroid_med.apt",
								"appearance/ui_asteroid_sm.apt"
							};

	char const * const cs_waypointAppearanceFileNames[ZoneMapObject::WG_numberOfWaypointGroups] =
							{
								"appearance/ui_waypoint_active.apt",
								"appearance/ui_waypoint.apt",
								"appearance/ui_active_poi_waypoint.apt",
								"appearance/ui_poi_waypoint.apt"
							};

	char const * const cs_zoneMembrane1 = "/shared_owner/index_color_1";
	char const * const cs_zoneMembrane2 = "/shared_owner/index_color_2";

	Appearance * s_zoneAppearance = 0;
	Appearance * s_hyperspacePointAppearance = 0;
	Appearance * s_spaceStationAppearance = 0;
	Appearance * s_nebulaAppearance = 0;
	Appearance * s_missionCriticalAppearance = 0;
	Appearance * s_focusAppearance = 0;
	Appearance * s_asteroidAppearances0 = 0;
	Appearance * s_asteroidAppearances1 = 0;
	Appearance * s_asteroidAppearances2 = 0;
	Appearance * s_waypointAppearances0 = 0;
	Appearance * s_waypointAppearances1 = 0;
	Appearance * s_waypointAppearances2 = 0;
	Appearance * s_waypointAppearances3 = 0;
	Appearance * s_shipAppearance = 0;
	Appearance * s_selectionAppearance = 0;

	float const cs_minimumPulse = 0.9f;
	float const cs_maximumPulse = 1.0f;
	float const cs_pulseSpeed = 0.05f;

	int s_nextSimpleIdentifierKey = 0;
	int s_nextAsteroidAppearanceIndex = 0;

	typedef std::pair<int, int> MembraneCustomization;
	MembraneCustomization s_defaultMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_shipMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_selectMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_activeWaypointMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_inactiveWaypointMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_activePoiWaypointMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_inactivePoiWaypointMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_missionCriticalMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_spaceStationMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_hyperspaceMembrane(std::make_pair<int, int>(132, 122));
	MembraneCustomization s_asteroidMembrane(std::make_pair<int, int>(132, 122));

	void install();
	void remove();
	int createSimpleIdentifierKeyOnObjectData();

	Object * createObjectFromAppearanceTemplate(char const * templateName, MembraneCustomization const & membrane, Vector const & scale = Vector::xyz111);
	Object * createObjectFromAppearanceTemplate(char const * templateName, Vector const & scale = Vector::xyz111);
	bool testObjectForIntersection(Object const * object, Ray3d const & ray_p, float & length);

	typedef std::map<NetworkId, ClientWaypointObject const *> ClientWaypointObjectsById;
	void modifyWayPointsInGroup(ZoneMapObject * zone, ZoneMapObject::WaypointGroup const & group, ClientWaypointObjectsById const & currentWaypointsInGroup);

	void updateTextAndLocations(ZoneMapObject::DynamicZoneObjects const & zoneObjects, ZoneMapObject::TextAndLocationVector & textAndLocations);
	void updateTextAndLocations(ZoneMapObject::StaticZoneObjects const & zoneObjects, ZoneMapObject::TextAndLocationVector & textAndLocations);
	bool isObjectInGroup(Object const * object, ZoneMapObject::DynamicZoneObjects const & zoneObjects);
	bool isObjectInGroup(Object const * object, ZoneMapObject::StaticZoneObjects const & zoneObjects);
}

//======================================================================

void ZoneMapObjectNamespace::install()
{
	ZoneMapObjectNamespace::s_zoneAppearance = AppearanceTemplateList::createAppearance(cs_zoneAppearanceFileName);
	ZoneMapObjectNamespace::s_hyperspacePointAppearance = AppearanceTemplateList::createAppearance(cs_hyperspacePointAppearanceFileName);
	ZoneMapObjectNamespace::s_spaceStationAppearance = AppearanceTemplateList::createAppearance(cs_spaceStationAppearanceFileName);
	ZoneMapObjectNamespace::s_nebulaAppearance = AppearanceTemplateList::createAppearance(cs_nebulaAppearanceFileName);
	ZoneMapObjectNamespace::s_missionCriticalAppearance = AppearanceTemplateList::createAppearance(cs_missionCriticalAppearanceFileName);
	ZoneMapObjectNamespace::s_focusAppearance = AppearanceTemplateList::createAppearance(cs_focusAppearanceFileName);
	ZoneMapObjectNamespace::s_asteroidAppearances0 = AppearanceTemplateList::createAppearance(cs_asteroidAppearanceFileNames[0]);
	ZoneMapObjectNamespace::s_asteroidAppearances1 = AppearanceTemplateList::createAppearance(cs_asteroidAppearanceFileNames[1]);
	ZoneMapObjectNamespace::s_asteroidAppearances2 = AppearanceTemplateList::createAppearance(cs_asteroidAppearanceFileNames[2]);
	ZoneMapObjectNamespace::s_waypointAppearances0 = AppearanceTemplateList::createAppearance(cs_waypointAppearanceFileNames[0]);
	ZoneMapObjectNamespace::s_waypointAppearances1 = AppearanceTemplateList::createAppearance(cs_waypointAppearanceFileNames[1]);
	ZoneMapObjectNamespace::s_waypointAppearances2 = AppearanceTemplateList::createAppearance(cs_waypointAppearanceFileNames[2]);
	ZoneMapObjectNamespace::s_waypointAppearances3 = AppearanceTemplateList::createAppearance(cs_waypointAppearanceFileNames[3]);
	ZoneMapObjectNamespace::s_shipAppearance = AppearanceTemplateList::createAppearance(cs_shipAppearanceFileName);
	ZoneMapObjectNamespace::s_selectionAppearance = AppearanceTemplateList::createAppearance(cs_selectionAppearanceFileName);
}

//-------------------------------------------------------------------

void ZoneMapObjectNamespace::remove()
{
	delete s_zoneAppearance;
	delete s_hyperspacePointAppearance;
	delete s_spaceStationAppearance;
	delete s_nebulaAppearance;
	delete s_missionCriticalAppearance;
	delete s_focusAppearance;
	delete s_asteroidAppearances0;
	delete s_asteroidAppearances1;
	delete s_asteroidAppearances2;
	delete s_waypointAppearances0;
	delete s_waypointAppearances1;
	delete s_waypointAppearances2;
	delete s_waypointAppearances3;
	delete s_shipAppearance;
	delete s_selectionAppearance;
}

//-------------------------------------------------------------------

int ZoneMapObjectNamespace::createSimpleIdentifierKeyOnObjectData()
{
	return ++s_nextSimpleIdentifierKey;
}

//----------------------------------------------------------------------

Object * ZoneMapObjectNamespace::createObjectFromAppearanceTemplate(char const * const templateName, MembraneCustomization const & membrane, Vector const & scale)
{
	Appearance * const appearance = AppearanceTemplateList::createAppearance(templateName);

	if (appearance != 0)
	{
		Object * const object = new Object;

		RenderWorld::addObjectNotifications(*object);

		object->setAppearance(appearance);

		//-- get or create the CustomizationDataProperty
		CustomizationDataProperty * property = safe_cast<CustomizationDataProperty*>(object->getProperty(CustomizationDataProperty::getClassPropertyId()));

		if (property == 0)
		{
			// create the CustomizationDataProperty
			property = new CustomizationDataProperty(*object);
			object->addProperty(*property);
		}

		//-- get the CustomizationData for the object
		NOT_NULL(property);

		CustomizationData * const customizationData = property->fetchCustomizationData();
		NOT_NULL(customizationData);

		//-- add variables influencing the appearance to the CustomizationData
		appearance->addCustomizationVariables(*customizationData);

		CustomizationVariable * variable1 = customizationData->findVariable(cs_zoneMembrane1);
		RangedIntCustomizationVariable * const variableInt1 = dynamic_cast<RangedIntCustomizationVariable*>(variable1);

		CustomizationVariable * variable2 = customizationData->findVariable(cs_zoneMembrane2);
		RangedIntCustomizationVariable * const variableInt2 = dynamic_cast<RangedIntCustomizationVariable*>(variable2);

		if (variableInt1 != 0)
		{
			variableInt1->setValue(membrane.first);
		}

		if (variableInt2 != 0)
		{
			variableInt2->setValue(membrane.second);
		}

		//-- set the appearance's CustomizationData
		appearance->setCustomizationData(customizationData);

		//-- release local reference
		customizationData->release();
		
		object->setRecursiveScale(scale);

		property = 0;//lint !e672 property is owned by object

		return object;
	}//lint !e672 property is owned by object

	return 0;
}


//----------------------------------------------------------------------

Object * ZoneMapObjectNamespace::createObjectFromAppearanceTemplate(char const * const templateName, Vector const & scale)
{
	Appearance * const appearance = AppearanceTemplateList::createAppearance(templateName);

	if (appearance != 0)
	{
		Object * const object = new Object;

		RenderWorld::addObjectNotifications(*object);

		object->setAppearance(appearance);
		object->setRecursiveScale(scale);

		return object;
	}//lint !e672 property is owned by object

	return 0;
}

//----------------------------------------------------------------------

bool ZoneMapObjectNamespace::testObjectForIntersection(Object const * const object, Ray3d const & ray_p, float & length)
{
	if (object != 0)
	{
		Appearance const * const appearance = object->getAppearance();

		if (appearance != 0)
		{
			Ray3d const ray_o(object->rotateTranslate_p2o(ray_p.getPoint()), object->rotate_p2o(ray_p.getNormal()));
			Sphere const & sphere = appearance->getSphere();

			Intersect3d::ResultData data(false);

			if (Intersect3d::intersectRaySphereWithData(ray_o, sphere, &data))
			{
				length = data.m_length;
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

void ZoneMapObjectNamespace::modifyWayPointsInGroup(ZoneMapObject * const zone, ZoneMapObject::WaypointGroup const & group, ClientWaypointObjectsById const & currentWaypointsInGroup)
{
	NOT_NULL(zone);

	ZoneMapObject::NetworkIds waypointIds;

	{
		// build a set of all waypointIds for compareWaypointIds
		ClientWaypointObjectsById::const_iterator ii = currentWaypointsInGroup.begin();
		ClientWaypointObjectsById::const_iterator iiEnd = currentWaypointsInGroup.end();

		for (; ii != iiEnd; ++ii)
		{
			IGNORE_RETURN(waypointIds.insert(ii->first));
		}
	}

	ZoneMapObject::NetworkIds waypointsToAdd;
	ZoneMapObject::NetworkIds waypointsToRemove;
	ZoneMapObject::NetworkIds waypointsToUpdate;
	zone->compareWaypointIds(group, waypointIds, waypointsToAdd, waypointsToRemove, waypointsToUpdate);

	{
		ZoneMapObject::NetworkIds::const_iterator ii = waypointsToAdd.begin();
		ZoneMapObject::NetworkIds::const_iterator iiEnd = waypointsToAdd.end();

		for (; ii != iiEnd; ++ii)
		{
			ClientWaypointObjectsById::const_iterator jj = currentWaypointsInGroup.find(*ii);
			ClientWaypointObject const * const clientWaypointObject = (jj != currentWaypointsInGroup.end()) ? jj->second : 0;
			NOT_NULL(clientWaypointObject);

			Unicode::String const & localizedName = clientWaypointObject->getLocalizedName();
			Transform transform_w;
			transform_w.setPosition_p(clientWaypointObject->getLocation());
			zone->addWaypoint(group, *ii, localizedName, transform_w);
		}
	}

	{
		ZoneMapObject::NetworkIds::const_iterator ii = waypointsToRemove.begin();
		ZoneMapObject::NetworkIds::const_iterator iiEnd = waypointsToRemove.end();

		for (; ii != iiEnd; ++ii)
		{
			zone->removeWaypoint(group, *ii);
		}
	}

	{
		ZoneMapObject::NetworkIds::const_iterator ii = waypointsToUpdate.begin();
		ZoneMapObject::NetworkIds::const_iterator iiEnd = waypointsToUpdate.end();

		for (; ii != iiEnd; ++ii)
		{
			ClientWaypointObjectsById::const_iterator jj = currentWaypointsInGroup.find(*ii);
			ClientWaypointObject const * const clientWaypointObject = (jj != currentWaypointsInGroup.end()) ? jj->second : 0;
			NOT_NULL(clientWaypointObject);

			Unicode::String const & localizedName = clientWaypointObject->getLocalizedName();
			Transform transform_w;
			transform_w.setPosition_p(clientWaypointObject->getLocation());
			zone->updateWaypoint(group, *ii, localizedName, transform_w);
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObjectNamespace::updateTextAndLocations(ZoneMapObject::DynamicZoneObjects const & zoneObjects, ZoneMapObject::TextAndLocationVector & textAndLocations)
{
	ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
	ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ZoneMapObject::ObjectData const & objectData = ii->second;
		textAndLocations.push_back(ZoneMapObject::makeTextAndLocation(objectData));
	}
}

//----------------------------------------------------------------------

void ZoneMapObjectNamespace::updateTextAndLocations(ZoneMapObject::StaticZoneObjects const & zoneObjects, ZoneMapObject::TextAndLocationVector & textAndLocations)
{
	ZoneMapObject::StaticZoneObjects::const_iterator ii = zoneObjects.begin();
	ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ZoneMapObject::ObjectData const & objectData = *ii;
		textAndLocations.push_back(ZoneMapObject::makeTextAndLocation(objectData));
	}
}

//----------------------------------------------------------------------

bool ZoneMapObjectNamespace::isObjectInGroup(Object const * const object, ZoneMapObject::DynamicZoneObjects const & zoneObjects)
{
	NOT_NULL(object);
	ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
	ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ZoneMapObject::ObjectData const & objectData = ii->second;
		if (objectData.getObject() == object)
		{
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool ZoneMapObjectNamespace::isObjectInGroup(Object const * const object, ZoneMapObject::StaticZoneObjects const & zoneObjects)
{
	NOT_NULL(object);
	ZoneMapObject::StaticZoneObjects::const_iterator ii = zoneObjects.begin();
	ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ZoneMapObject::ObjectData const & objectData = *ii;
		if (objectData.getObject() == object)
		{
			return true;
		}
	}
	return false;
}

//======================================================================

using namespace ZoneMapObjectNamespace;

//======================================================================

ZoneMapObject::ObjectData::ObjectData()
: m_type()
, m_object(0)
, m_localizedName()
, m_transform_w()
, m_simpleIdentifierKey()
{
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData::ObjectData(DataType const type, Object * object, Transform const & transform_w)
: m_type(type)
, m_object(object)
, m_localizedName()
, m_transform_w(transform_w)
, m_simpleIdentifierKey(createSimpleIdentifierKeyOnObjectData())
{
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData::ObjectData(DataType const type, Object * object, Unicode::String const & localizedName, Transform const & transform_w)
: m_type(type)
, m_object(object)
, m_localizedName(localizedName)
, m_transform_w(transform_w)
, m_simpleIdentifierKey(createSimpleIdentifierKeyOnObjectData())
{
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData::~ObjectData()
{
	m_object = 0;
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData::DataType ZoneMapObject::ObjectData::getDataType() const
{
	return m_type;
}

//----------------------------------------------------------------------

Object * ZoneMapObject::ObjectData::getObject() const
{
	return m_object;
}

//----------------------------------------------------------------------

Unicode::String const & ZoneMapObject::ObjectData::getLocalizedName() const
{
	return m_localizedName;
}

//----------------------------------------------------------------------

void ZoneMapObject::ObjectData::setLocalizedName(Unicode::String const & localizedName)
{
	m_localizedName = localizedName;
}

//----------------------------------------------------------------------

Transform const & ZoneMapObject::ObjectData::getTransform_w() const
{
	return m_transform_w;
}

//----------------------------------------------------------------------

void ZoneMapObject::ObjectData::setTransform_w(Transform const & transform_w)
{
	m_transform_w = transform_w;
}

//----------------------------------------------------------------------

int ZoneMapObject::ObjectData::getSimpleIdentifierKey() const
{
	return m_simpleIdentifierKey;
}

//======================================================================

ZoneMapObject::ZoneMapObject(std::string const & sceneId, ShipObject const * const owningShip)
: Object()
, m_sceneId(sceneId)
, m_actualZoneSize(0.0f)
, m_owningShip(owningShip)
, m_player(0)
, m_focus(0)
, m_target(0)
, m_targetSelection(0)
, m_asteroids()
, m_nebulas()
, m_hyperspacePoints()
, m_spaceStations()
, m_activeWaypoints()
, m_inactiveWaypoints()
, m_activePoiWaypoints()
, m_inactivePoiWaypoints()
, m_missionCriticals()
, m_dirty(false)
, m_renderAsteroids(false)
, m_renderNebulas(false)
, m_renderHyperspacePoints(false)
, m_renderSpaceStations(false)
, m_renderWaypoints(false)
, m_renderMissionCriticals(false)
, m_renderPlayerShip(false)
, m_renderFocus(false)
, m_focusCameraOnTarget(false)
, m_scale(Vector::xyz111)
, m_objectsToDeleteIfDirty()
, m_textAndLocations()
{
	// This class assumes that all space zones are the same dimension.
	// i.e. space_naboo and space_tatooine are the same size
	TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
	if (terrainObject != 0)
	{
		m_actualZoneSize = terrainObject->getMapWidthInMeters();
	}

	Appearance * const appearance = AppearanceTemplateList::createAppearance(cs_zoneAppearanceFileName);

	NOT_NULL(appearance);
	setAppearance(appearance);

	if (m_owningShip != 0)
	{
		m_player = createObjectFromAppearanceTemplate(cs_shipAppearanceFileName, s_shipMembrane);
	}

	m_focus = createObjectFromAppearanceTemplate(cs_focusAppearanceFileName, s_defaultMembrane);

	m_targetSelection = createObjectFromAppearanceTemplate(cs_selectionAppearanceFileName, s_selectMembrane);
}//lint !e429 extentAppearance now owned by m_target

//----------------------------------------------------------------------

ZoneMapObject::~ZoneMapObject()
{
	clearObjectsFromRoot(true);

	if (m_player != 0)
	{
		Object * const object = m_player.getPointer();
		delete object;
	}

	if (m_focus != 0)
	{
		Object * const object = m_focus.getPointer();
		delete object;
	}

	if (m_target != 0)
	{
		Object * const object = m_target.getPointer();
		delete object;
	}

	if (m_targetSelection != 0)
	{
		Object * const object = m_targetSelection.getPointer();
		delete object;
	}

	// root deletes all that are visible... watchers
	deleteObjects(m_asteroids);
	deleteObjects(m_nebulas);
	deleteObjects(m_hyperspacePoints);
	deleteObjects(m_spaceStations);
	deleteObjects(m_activeWaypoints);
	deleteObjects(m_inactiveWaypoints);
	deleteObjects(m_activePoiWaypoints);
	deleteObjects(m_inactivePoiWaypoints);
}

//----------------------------------------------------------------------

void ZoneMapObject::install()
{
	InstallTimer const installTimer("ZoneMapObject::install");

	ZoneMapObjectNamespace::install();
	ExitChain::add(remove, "ZoneMapObjectNamespace::remove");
}

//----------------------------------------------------------------------

void ZoneMapObject::setDefaultMembraneCustomizationFields(int const index1, int const index2)
{
	s_defaultMembrane.first = index1;
	s_defaultMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setShipMembraneCustomizationFields(int const index1, int const index2)
{
	s_shipMembrane.first = index1;
	s_shipMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setSelectMembraneCustomizationFields(int const index1, int const index2)
{
	s_selectMembrane.first = index1;
	s_selectMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setActiveWaypointMembraneCustomizationFields(int const index1, int const index2)
{
	s_activeWaypointMembrane.first = index1;
	s_activeWaypointMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setInactiveWaypointMembraneCustomizationFields(int const index1, int const index2)
{
	s_inactiveWaypointMembrane.first = index1;
	s_inactiveWaypointMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setActivePoiWaypointMembraneCustomizationFields(int const index1, int const index2)
{
	s_activePoiWaypointMembrane.first = index1;
	s_activePoiWaypointMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setInactivePoiWaypointMembraneCustomizationFields(int const index1, int const index2)
{
	s_inactivePoiWaypointMembrane.first = index1;
	s_inactivePoiWaypointMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setMissionCriticalMembraneCustomizationFields(int const index1, int const index2)
{
	s_missionCriticalMembrane.first = index1;
	s_missionCriticalMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setSpaceStationMembraneCustomizationFields(int const index1, int const index2)
{
	s_spaceStationMembrane.first = index1;
	s_spaceStationMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setHyperspaceMembraneCustomizationFields(int const index1, int const index2)
{
	s_hyperspaceMembrane.first = index1;
	s_hyperspaceMembrane.second = index2;
}

//----------------------------------------------------------------------

void ZoneMapObject::setAsteroidMembraneCustomizationFields(int const index1, int const index2)
{
	s_asteroidMembrane.first = index1;
	s_asteroidMembrane.second = index2;
}

//----------------------------------------------------------------------

ZoneMapObject::TextAndLocation ZoneMapObject::makeTextAndLocation(ObjectData const & objectData)
{
	Object const * const object = objectData.getObject();

	if (object != 0)
	{
		Unicode::String const & text = objectData.getLocalizedName();
		Vector location(object->getPosition_w());
		location.y += object->getAppearanceSphereRadius();
		return std::make_pair(text, location);
	}
	return TextAndLocation();
}

//----------------------------------------------------------------------

void ZoneMapObject::createAsteroids(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	typedef std::vector<AsteroidGenerationManager::AsteroidFieldData> Asteroids;

	std::string const & sceneId = zone->getSceneId();

	if (!AsteroidGenerationManager::hasDataForScene(sceneId))
	{
		AsteroidGenerationManager::loadStaticFieldDataForScene(sceneId, false);
	}

	Asteroids const & asteroids = AsteroidGenerationManager::getDataForScene(sceneId);

	Asteroids::const_iterator ii = asteroids.begin();
	Asteroids::const_iterator iiEnd = asteroids.end();

	for (; ii != iiEnd; ++ii)
	{
		AsteroidGenerationManager::AsteroidFieldData const & data = *ii;

		Transform transform_w;
		transform_w.setPosition_p(data.centerPosition);
		zone->addAsteroid(transform_w, data.radius);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::createNebulas(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	std::string const & sceneId = zone->getSceneId();

	if (!NebulaManager::hasSceneData(sceneId))
	{
		NebulaManager::loadSceneData(sceneId);
	}

	NebulaManager::NebulaVector const & nebulas = NebulaManager::getNebulaVector(sceneId);

	NebulaManager::NebulaVector::const_iterator ii = nebulas.begin();
	NebulaManager::NebulaVector::const_iterator iiEnd = nebulas.end();

	for (; ii != iiEnd; ++ii)
	{
		Nebula const * const nebula = *ii;

		if (nebula != 0)
		{
			Sphere const & sphere = nebula->getSphere(); 

			Transform transform_w;
			transform_w.setPosition_p(sphere.getCenter());
			zone->addNebula(transform_w, sphere.getRadius());
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::createHyperspacePoints(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	CreatureObject const * const player = Game::getPlayerCreature();
	std::string const & sceneId = zone->getSceneId();

	typedef std::vector<HyperspaceManager::HyperspaceLocation> Locations;
	Locations locations;

	if (HyperspaceManager::getHyperspacePoints(sceneId, locations))
	{
		Locations::const_iterator ii = locations.begin();
		Locations::const_iterator iiEnd = locations.end();

		for (; ii != iiEnd; ++ii)
		{
			HyperspaceManager::HyperspaceLocation const & hyperspaceLocation = *ii;

			// TODO: need to get a system to be aware of granted hyperspace points

			if ((hyperspaceLocation.requiredCommand.empty()) || (player->hasCommand(hyperspaceLocation.requiredCommand)))
			{
				Transform transform_w;
				transform_w.setPosition_p(hyperspaceLocation.location);
				Unicode::String const localizedName(StringId("hyperspace_points_n", hyperspaceLocation.name).localize());
				zone->addHyperspacePoint(localizedName, transform_w);
			}
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::createSpaceStations(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	std::string const & sceneId = zone->getSceneId();

	ZoneMapSpaceStationTable::loadForScene(sceneId);

	ZoneMapSpaceStationTable::DataVector const & spaceStations = ZoneMapSpaceStationTable::getData();
	ZoneMapSpaceStationTable::DataVector::const_iterator ii = spaceStations.begin();
	ZoneMapSpaceStationTable::DataVector::const_iterator iiEnd = spaceStations.end();

	for (; ii != iiEnd; ++ii)
	{
		ZoneMapSpaceStationTable::Data const & data = *ii;

		Transform transform_w;
		transform_w.setPosition_p(data.m_location);
		Unicode::String const localizedName(data.m_localizedName);
		zone->addSpaceStation(localizedName, transform_w);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::modifyWaypoints(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	ClientWaypointObjectsById currentActiveWaypoints;
	ClientWaypointObjectsById currentInactiveWaypoints;
	ClientWaypointObjectsById currentActivePoiWaypoints;
	ClientWaypointObjectsById currentInactivePoiWaypoints;

	{
		ClientWaypointObject::WaypointVector const & waypoints = ClientWaypointObject::getActiveWaypoints();

		ClientWaypointObject::WaypointVector::const_iterator ii = waypoints.begin ();
		ClientWaypointObject::WaypointVector::const_iterator iiEnd = waypoints.end ();

		for (; ii != iiEnd; ++ii)
		{
			ClientWaypointObject const * const clientWaypointObject = *ii;

			if (clientWaypointObject != 0)
			{
				if (clientWaypointObject->getPlanetName() == zone->getSceneId())
				{
					NetworkId const & networkId = clientWaypointObject->getNetworkId();
					if (clientWaypointObject->isWaypointActive())
					{
						currentActiveWaypoints[networkId] = clientWaypointObject;
					}
					else
					{
						currentInactiveWaypoints[networkId] = clientWaypointObject;
					}
				}
			}
		}
	}

	{
		ClientWaypointObject::ConstWaypointVector const & waypoints = ClientWaypointObject::getRegularWaypoints();

		ClientWaypointObject::ConstWaypointVector::const_iterator ii = waypoints.begin ();
		ClientWaypointObject::ConstWaypointVector::const_iterator iiEnd = waypoints.end ();

		for (; ii != iiEnd; ++ii)
		{
			ClientWaypointObject const * const clientWaypointObject = *ii;

			if (clientWaypointObject != 0)
			{
				if (clientWaypointObject->getPlanetName() == zone->getSceneId())
				{
					NetworkId const & networkId = clientWaypointObject->getNetworkId();
					if (clientWaypointObject->isWaypointActive())
					{
						currentActiveWaypoints[networkId] = clientWaypointObject;
					}
					else
					{
						currentInactiveWaypoints[networkId] = clientWaypointObject;
					}
				}
			}
		}
	}

	{
		ClientWaypointObject::ConstWaypointVector const & waypoints = ClientWaypointObject::getPoiWaypoints();

		ClientWaypointObject::ConstWaypointVector::const_iterator ii = waypoints.begin ();
		ClientWaypointObject::ConstWaypointVector::const_iterator iiEnd = waypoints.end ();

		for (; ii != iiEnd; ++ii)
		{
			ClientWaypointObject const * const clientWaypointObject = *ii;

			if (clientWaypointObject != 0)
			{
				if (clientWaypointObject->getPlanetName() == zone->getSceneId())
				{
					NetworkId const & networkId = clientWaypointObject->getNetworkId();

					if (clientWaypointObject->isWaypointActive())
					{
						currentActivePoiWaypoints[networkId] = clientWaypointObject;
					}
					else
					{
						currentInactivePoiWaypoints[networkId] = clientWaypointObject;
					}
				}
			}
		}
	}

	modifyWayPointsInGroup(zone, WG_active, currentActiveWaypoints);
	modifyWayPointsInGroup(zone, WG_inactive, currentInactiveWaypoints);
	modifyWayPointsInGroup(zone, WG_activePoi, currentActivePoiWaypoints);
	modifyWayPointsInGroup(zone, WG_inactivePoi, currentInactivePoiWaypoints);
}

//----------------------------------------------------------------------

void ZoneMapObject::modifyMissionCriticals(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	ZoneMapObject::NetworkIds currentMissionCriticals;

	CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject != 0)
	{
		CreatureObject::GroupMissionCriticalObjectSet::const_iterator ii = playerCreatureObject->getGroupMissionCriticalObjects().begin();
		CreatureObject::GroupMissionCriticalObjectSet::const_iterator iiEnd = playerCreatureObject->getGroupMissionCriticalObjects().end();

		for (; ii != iiEnd; ++ii)
		{
			IGNORE_RETURN(currentMissionCriticals.insert(ii->second));
		}
	}

	ZoneMapObject::NetworkIds currentMissionCriticalsToAdd;
	ZoneMapObject::NetworkIds currentMissionCriticalsToRemove;
	ZoneMapObject::NetworkIds currentMissionCriticalsToUpdate;

	zone->compareMissionCriticalIds(currentMissionCriticals, currentMissionCriticalsToAdd, currentMissionCriticalsToRemove, currentMissionCriticalsToUpdate);

	{
		ZoneMapObject::NetworkIds::const_iterator ii = currentMissionCriticalsToAdd.begin();
		ZoneMapObject::NetworkIds::const_iterator iiEnd = currentMissionCriticalsToAdd.end();

		for (; ii != iiEnd; ++ii)
		{
			Object const * const object = NetworkIdManager::getObjectById(*ii);
			ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
			if (clientObject != 0)
			{
				Unicode::String const & localizedName = clientObject->getLocalizedName();
				Transform const transform_w = clientObject->getTransform_o2w();
				zone->addMissionCritical(*ii, localizedName, transform_w);
			}
		}
	}

	{
		ZoneMapObject::NetworkIds::const_iterator ii = currentMissionCriticalsToRemove.begin();
		ZoneMapObject::NetworkIds::const_iterator iiEnd = currentMissionCriticalsToRemove.end();

		for (; ii != iiEnd; ++ii)
		{
			zone->removeMissionCritical(*ii);
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::updateMissionCriticalLocations(ZoneMapObject * const zone)
{
	NOT_NULL(zone);

	ZoneMapObject::NetworkIds currentMissionCriticals;

	CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject != 0)
	{
		CreatureObject::GroupMissionCriticalObjectSet::const_iterator ii = playerCreatureObject->getGroupMissionCriticalObjects().begin();
		CreatureObject::GroupMissionCriticalObjectSet::const_iterator iiEnd = playerCreatureObject->getGroupMissionCriticalObjects().end();

		for (; ii != iiEnd; ++ii)
		{
			Object const * const object = NetworkIdManager::getObjectById(ii->second);
			if (object != 0)
			{
				Transform const transform_w = object->getTransform_o2w();
				zone->updateMissionCritical(ii->second, transform_w);
			}
		}
	}
}

//----------------------------------------------------------------------

std::string const & ZoneMapObject::getSceneId() const
{
	return m_sceneId;
}

//----------------------------------------------------------------------

Vector ZoneMapObject::worldLocationToZoneLocation(Vector const & worldLocation) const
{
	float const zoneLocationScale = cs_scaledZoneSize / m_actualZoneSize;
	return worldLocation * zoneLocationScale;
}

//----------------------------------------------------------------------

Transform ZoneMapObject::worldTransformToZoneTransform(Transform const & worldTransform) const
{
	Vector const worldLocation_w = worldTransform.getPosition_p();
	Transform transform(worldTransform);
	transform.setPosition_p(worldLocationToZoneLocation(worldLocation_w));

	return transform;
}

//----------------------------------------------------------------------

float ZoneMapObject::worldScaleToZoneScale(float const worldScale) const
{
	return worldScale * cs_scaledZoneSize / m_actualZoneSize;
}

//----------------------------------------------------------------------

Vector ZoneMapObject::worldScaleToZoneScale(Vector const & worldScale) const
{
	return worldScale * cs_scaledZoneSize / m_actualZoneSize;
}

//----------------------------------------------------------------------

Vector ZoneMapObject::zoneLocationToWorldLocation(Vector const & zoneLocation) const
{
	float const zoneToWorldLocationScale = m_actualZoneSize / cs_scaledZoneSize;
	return zoneLocation * zoneToWorldLocationScale;
}

//----------------------------------------------------------------------


bool ZoneMapObject::isPointInZone(Vector const & pointInZoneSpace) const
{
	float const halfSize = cs_scaledZoneSize * 0.5f;
	if (pointInZoneSpace.z < -halfSize) return false;
	if (pointInZoneSpace.x < -halfSize) return false;
	if (pointInZoneSpace.y < -halfSize) return false;
	if (pointInZoneSpace.z >  halfSize) return false;
	if (pointInZoneSpace.x >  halfSize) return false;
	if (pointInZoneSpace.y >  halfSize) return false;
	return true;
}

//----------------------------------------------------------------------

float ZoneMapObject::alter(float elapsedTime)
{
	float const result = Object::alter(elapsedTime);

	if (m_dirty)
	{
		clearObjectsFromRoot(false);
		m_textAndLocations.clear();
	
		if (m_renderAsteroids)
		{
			addObjectsToRoot(m_asteroids);
		}

		if (m_renderNebulas)
		{
			addObjectsToRoot(m_nebulas);
		}

		if (m_renderHyperspacePoints)
		{
			addObjectsToRoot(m_hyperspacePoints);
			updateTextAndLocations(m_hyperspacePoints, m_textAndLocations);
		}

		if (m_renderSpaceStations)
		{
			addObjectsToRoot(m_spaceStations);
			if (!m_renderWaypoints)
			{
				updateTextAndLocations(m_spaceStations, m_textAndLocations);
			}
		}

		if (m_renderWaypoints)
		{
			addObjectsToRoot(m_activeWaypoints);
			addObjectsToRoot(m_inactiveWaypoints);
			addObjectsToRoot(m_activePoiWaypoints);
			addObjectsToRoot(m_inactivePoiWaypoints);
			updateTextAndLocations(m_activeWaypoints, m_textAndLocations);
			updateTextAndLocations(m_inactiveWaypoints, m_textAndLocations);
			updateTextAndLocations(m_activePoiWaypoints, m_textAndLocations);
			updateTextAndLocations(m_inactivePoiWaypoints, m_textAndLocations);
		}

		if (m_renderMissionCriticals)
		{
			addObjectsToRoot(m_missionCriticals);
			updateTextAndLocations(m_missionCriticals, m_textAndLocations);
		}

		if (m_renderPlayerShip)
		{
			if (m_player != 0)
			{
				addChildObject_o(m_player);
			}
		}

		if (m_renderFocus)
		{
			if (m_focus != 0)
			{
				addChildObject_o(m_focus);
			}
		}

		if ((m_targetSelection != 0) && (m_target != m_player))
		{
			// don't draw a focus on the player ship
			addChildObject_o(m_targetSelection);
		}

		ZoneMapObject::ObjectWatchList::const_iterator ii = m_objectsToDeleteIfDirty.begin();
		ZoneMapObject::ObjectWatchList::const_iterator iiEnd = m_objectsToDeleteIfDirty.end();

		for (; ii != iiEnd; ++ii)
		{
			Object * const object = *ii;
			if (object != 0)
			{
				delete object;
			}
		}

		m_objectsToDeleteIfDirty.clear();
		m_dirty = false;
	}

	// update the player ship to represent current game location
	if (m_owningShip != 0)
	{
		Transform transform(m_owningShip->getTransform_o2w());

		transform.setPosition_p(worldLocationToZoneLocation(transform.getPosition_p()));

		if (m_player != 0)
		{
			m_player->setTransform_o2w(transform);
		}
	}

	if ((m_target != 0) && (m_targetSelection != 0))
	{
		m_targetSelection->setRecursiveScale(m_target->getScale() * m_target->getAppearanceSphereRadius());
		m_targetSelection->setTransform_o2p(m_target->getTransform_o2p());
	}

	return result;
}

//----------------------------------------------------------------------

void ZoneMapObject::addAsteroid(Transform const & transform_w, float const scale_w)
{
	char const * const appearance = cs_asteroidAppearanceFileNames[s_nextAsteroidAppearanceIndex];

	++s_nextAsteroidAppearanceIndex;
	if (s_nextAsteroidAppearanceIndex >= cs_numberOfAsteroidAppearanceFileNames)
	{
		s_nextAsteroidAppearanceIndex = 0;
	}

	float const scale = worldScaleToZoneScale(scale_w);

	Object * const asteroid = createObjectFromAppearanceTemplate(appearance, s_asteroidMembrane, Vector::xyz111 * scale);

	if (asteroid != 0)
	{
		m_asteroids.push_back(ObjectData(ObjectData::DT_asteroid, asteroid, transform_w));
		m_dirty = true;
	}

	NOT_NULL(asteroid);
}

//----------------------------------------------------------------------

void ZoneMapObject::addNebula(Transform const & transform_w, float const scale_w)
{
	float const scale = std::max(worldScaleToZoneScale(scale_w), 1.0f);

	Object * const nebula = createObjectFromAppearanceTemplate(cs_nebulaAppearanceFileName, Vector::xyz111 * scale);

	if (nebula != 0)
	{
		m_nebulas.push_back(ObjectData(ObjectData::DT_nebula, nebula, transform_w));
		m_dirty = true;
	}

	NOT_NULL(nebula);
}

//----------------------------------------------------------------------

void ZoneMapObject::addHyperspacePoint(Unicode::String const & localizedName, Transform const & transform_w)
{
	Object * const hyperspacePoint = createObjectFromAppearanceTemplate(cs_hyperspacePointAppearanceFileName, s_hyperspaceMembrane);

	if (hyperspacePoint != 0)
	{
		m_hyperspacePoints.push_back(ObjectData(ObjectData::DT_hyperspacePoint, hyperspacePoint, localizedName, transform_w));
		m_dirty = true;
	}

	NOT_NULL(hyperspacePoint);
}

//----------------------------------------------------------------------

void ZoneMapObject::addSpaceStation(Unicode::String const & localizedName, Transform const & transform_w)
{
	Object * const spaceStation = createObjectFromAppearanceTemplate(cs_spaceStationAppearanceFileName, s_spaceStationMembrane);

	if (spaceStation != 0)
	{
		m_spaceStations.push_back(ObjectData(ObjectData::DT_spaceStation, spaceStation, localizedName, transform_w));
		m_dirty = true;
	}

	NOT_NULL(spaceStation);
}

//----------------------------------------------------------------------

void ZoneMapObject::compareDynamicIds(DynamicZoneObjects const & dynamicZoneObjects, NetworkIds const & currentIds, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const
{
	{
		NetworkIds::const_iterator ii = currentIds.begin();
		NetworkIds::const_iterator iiEnd = currentIds.end();

		for (; ii != iiEnd; ++ ii)
		{
			NetworkId const & networkIdToTestAgainst = *ii;
			DynamicZoneObjects::const_iterator jj = dynamicZoneObjects.find(networkIdToTestAgainst);

			if (jj == dynamicZoneObjects.end())
			{
				// doesn't exit so add
				IGNORE_RETURN(toAdd.insert(*ii));
			}
			else
			{
				// exists so update it
				IGNORE_RETURN(toUpdate.insert(*ii));
			}
		}
	}


	{
		DynamicZoneObjects::const_iterator ii = dynamicZoneObjects.begin();
		DynamicZoneObjects::const_iterator iiEnd = dynamicZoneObjects.end();

		for (; ii != iiEnd; ++ ii)
		{
			NetworkId const & networkIdToTestAgainst = ii->first;

			NetworkIds::const_iterator jj = currentIds.find(networkIdToTestAgainst);

			if (jj == currentIds.end())
			{
				// doesn't exist so remove
				IGNORE_RETURN(toRemove.insert(ii->first));
			}
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::addDynamic(DynamicZoneObjects & dynamicZoneObjects, ObjectData::DataType type, Object * const child, NetworkId const & networkId, Unicode::String const & localizedName, const Transform & transform_w) const
{
	DynamicZoneObjects::const_iterator ii = dynamicZoneObjects.find(networkId);

	WARNING_DEBUG_FATAL(ii != dynamicZoneObjects.end(), ("ZoneMapObject::addDynamic: object already exists."));

	if (ii == dynamicZoneObjects.end())
	{
		ObjectData data(type, child, localizedName, transform_w);
		dynamicZoneObjects[networkId] = data;
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::removeDynamic(DynamicZoneObjects & dynamicZoneObjects, ObjectWatchList & toDelete, NetworkId const & networkId) const
{
	DynamicZoneObjects::iterator ii = dynamicZoneObjects.find(networkId);

	WARNING_DEBUG_FATAL(ii == dynamicZoneObjects.end(), ("ZoneMapObject::removeDynamic: object doesn\'t exist."));

	if (ii != dynamicZoneObjects.end())
	{
		ObjectData const & objectData = ii->second;
		Object * object = objectData.getObject();
		if (object != 0)
		{
			toDelete.push_back(Watcher<Object>(object));
		}

		dynamicZoneObjects.erase(ii);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::updateDynamic(DynamicZoneObjects & dynamicZoneObjects, NetworkId const & networkId, Unicode::String const & localizedName, const Transform & transform_w) const
{
	DynamicZoneObjects::iterator ii = dynamicZoneObjects.find(networkId);

	WARNING_DEBUG_FATAL(ii == dynamicZoneObjects.end(), ("ZoneMapObject::updateDynamic: object doesn\'t exist."));

	if (ii != dynamicZoneObjects.end())
	{
		ObjectData & data = ii->second;

		if (!localizedName.empty())
		{
			data.setLocalizedName(localizedName);
		}

		data.setTransform_w(transform_w);
		data.getObject()->setTransform_o2p(worldTransformToZoneTransform(transform_w));
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::updateDynamic(DynamicZoneObjects & dynamicZoneObjects, NetworkId const & networkId, Transform const & transform_w) const
{
	updateDynamic(dynamicZoneObjects, networkId, Unicode::emptyString, transform_w);
}

//----------------------------------------------------------------------

void ZoneMapObject::compareWaypointIds(ZoneMapObject::WaypointGroup const & group, NetworkIds const & currentWaypoints, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const
{
	switch(group)
	{
		case WG_active:
			compareDynamicIds(m_activeWaypoints, currentWaypoints, toAdd, toRemove, toUpdate);
			break;
		case WG_inactive:
			compareDynamicIds(m_inactiveWaypoints, currentWaypoints, toAdd, toRemove, toUpdate);
			break;
		case WG_activePoi:
			compareDynamicIds(m_activePoiWaypoints, currentWaypoints, toAdd, toRemove, toUpdate);
			break;
		case WG_inactivePoi:
			compareDynamicIds(m_inactivePoiWaypoints, currentWaypoints, toAdd, toRemove, toUpdate);
			break;
		case WG_numberOfWaypointGroups:
		default:
			break;
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::addWaypoint(ZoneMapObject::WaypointGroup const & group, NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w)
{
	WARNING_DEBUG_FATAL(group >= WG_numberOfWaypointGroups, ("ZoneMapObject::addWaypoint: group >= WG_numberOfWaypointGroups."));

	Object * waypoint = 0;

	switch(group)
	{
		case WG_active:
			waypoint = createObjectFromAppearanceTemplate(cs_waypointAppearanceFileNames[group], s_activeWaypointMembrane);
			addDynamic(m_activeWaypoints, ObjectData::DT_wayPoint, waypoint, networkId, localizedName, transform_w);
			break;
		case WG_inactive:
			waypoint = createObjectFromAppearanceTemplate(cs_waypointAppearanceFileNames[group], s_inactiveWaypointMembrane);
			addDynamic(m_inactiveWaypoints, ObjectData::DT_wayPoint, waypoint, networkId, localizedName, transform_w);
			break;
		case WG_activePoi:
			waypoint = createObjectFromAppearanceTemplate(cs_waypointAppearanceFileNames[group], s_activePoiWaypointMembrane);
			addDynamic(m_activePoiWaypoints, ObjectData::DT_wayPoint, waypoint, networkId, localizedName, transform_w);
			break;
		case WG_inactivePoi:
			waypoint = createObjectFromAppearanceTemplate(cs_waypointAppearanceFileNames[group], s_inactivePoiWaypointMembrane);
			addDynamic(m_inactivePoiWaypoints, ObjectData::DT_wayPoint, waypoint, networkId, localizedName, transform_w);
			break;
		case WG_numberOfWaypointGroups:
		default:
			break;
	}
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::removeWaypoint(ZoneMapObject::WaypointGroup const & group, NetworkId const & networkId)
{
	switch(group)
	{
		case WG_active:
			removeDynamic(m_activeWaypoints, m_objectsToDeleteIfDirty, networkId);
			break;
		case WG_inactive:
			removeDynamic(m_inactiveWaypoints, m_objectsToDeleteIfDirty, networkId);
			break;
		case WG_activePoi:
			removeDynamic(m_activePoiWaypoints, m_objectsToDeleteIfDirty, networkId);
			break;
		case WG_inactivePoi:
			removeDynamic(m_inactivePoiWaypoints, m_objectsToDeleteIfDirty, networkId);
			break;
		case WG_numberOfWaypointGroups:
		default:
			break;
	}
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::updateWaypoint(ZoneMapObject::WaypointGroup const & group, NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w)
{
	switch(group)
	{
		case WG_active:
			updateDynamic(m_activeWaypoints, networkId, localizedName, transform_w);
			break;
		case WG_inactive:
			updateDynamic(m_inactiveWaypoints, networkId, localizedName, transform_w);
			break;
		case WG_activePoi:
			updateDynamic(m_activePoiWaypoints, networkId, localizedName, transform_w);
			break;
		case WG_inactivePoi:
			updateDynamic(m_inactivePoiWaypoints, networkId, localizedName, transform_w);
			break;
		case WG_numberOfWaypointGroups:
		default:
			break;
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::compareMissionCriticalIds(NetworkIds const & currentMissionCriticals, NetworkIds & toAdd, NetworkIds & toRemove, NetworkIds & toUpdate) const
{
	compareDynamicIds(m_missionCriticals, currentMissionCriticals, toAdd, toRemove, toUpdate);
}

//----------------------------------------------------------------------

void ZoneMapObject::addMissionCritical(NetworkId const & networkId, Unicode::String const & localizedName, Transform const & transform_w)
{
	Object * const missionCritical = createObjectFromAppearanceTemplate(cs_missionCriticalAppearanceFileName, s_missionCriticalMembrane);

	if (missionCritical != 0)
	{
		addDynamic(m_missionCriticals, ObjectData::DT_missionCritical, missionCritical, networkId, localizedName, transform_w);
		m_dirty = true;
	}

	NOT_NULL(missionCritical);
}

//----------------------------------------------------------------------

void ZoneMapObject::removeMissionCritical(NetworkId const & networkId)
{
	removeDynamic(m_missionCriticals, m_objectsToDeleteIfDirty, networkId);
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::updateMissionCritical(NetworkId const & networkId, Transform const & transform_w)
{
	updateDynamic(m_missionCriticals, networkId, transform_w);
}

//----------------------------------------------------------------------

ZoneMapObject::StaticZoneObjects const & ZoneMapObject::getAsteroids() const
{
	return m_asteroids;
}

//----------------------------------------------------------------------

ZoneMapObject::StaticZoneObjects const & ZoneMapObject::getNebulas() const
{
	return m_nebulas;
}

//----------------------------------------------------------------------

ZoneMapObject::StaticZoneObjects const & ZoneMapObject::getHyperspacePoints() const
{
	return m_hyperspacePoints;
}

//----------------------------------------------------------------------

ZoneMapObject::StaticZoneObjects const & ZoneMapObject::getSpaceStations() const
{
	return m_spaceStations;
}

//----------------------------------------------------------------------

ZoneMapObject::DynamicZoneObjects const & ZoneMapObject::getActiveWaypoints() const
{
	return m_activeWaypoints;
}

//----------------------------------------------------------------------

ZoneMapObject::DynamicZoneObjects const & ZoneMapObject::getInactiveWaypoints() const
{
	return m_inactiveWaypoints;
}

//----------------------------------------------------------------------

ZoneMapObject::DynamicZoneObjects const & ZoneMapObject::getActivePoiWaypoints() const
{
	return m_activePoiWaypoints;
}

//----------------------------------------------------------------------

ZoneMapObject::DynamicZoneObjects const & ZoneMapObject::getInactivePoiWaypoints() const
{
	return m_inactivePoiWaypoints;
}

//----------------------------------------------------------------------

ZoneMapObject::DynamicZoneObjects const & ZoneMapObject::getMissionCriticals() const
{
	return m_missionCriticals;
}

//----------------------------------------------------------------------

ZoneMapObject::TextAndLocationVector const & ZoneMapObject::getAllTextAndLocations() const
{
	return m_textAndLocations;
}

//----------------------------------------------------------------------

void ZoneMapObject::showAsteroid(bool show)
{
	m_renderAsteroids = show;
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::showNebula(bool show)
{
	m_renderNebulas = show;
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::showHyperspacePoint(bool show)
{
	m_renderHyperspacePoints = show;
	m_dirty = true;

	if (show)
	{
		return;
	}

	if ((m_target != 0) && (isObjectInGroup(m_target, m_hyperspacePoints)))
	{
		highlightTarget(0);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::showSpaceStations(bool show)
{
	m_renderSpaceStations = show;
	m_dirty = true;

	if (show)
	{
		return;
	}

	if ((m_target != 0) && (isObjectInGroup(m_target, m_spaceStations)))
	{
		highlightTarget(0);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::showWaypoint(bool show)
{
	m_renderWaypoints = show;
	m_dirty = true;

	if (show)
	{
		return;
	}

	if (m_target != 0)
	{
		if (isObjectInGroup(m_target, m_activeWaypoints))
		{
			highlightTarget(0);
		}
		else if (isObjectInGroup(m_target, m_inactiveWaypoints))
		{
			highlightTarget(0);
		}
		else if (isObjectInGroup(m_target, m_activePoiWaypoints))
		{
			highlightTarget(0);
		}
		else if (isObjectInGroup(m_target, m_inactivePoiWaypoints))
		{
			highlightTarget(0);
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::showMissionCritical(bool show)
{
	m_renderMissionCriticals = show;
	m_dirty = true;

	if (show)
	{
		return;
	}

	if ((m_target != 0) && (isObjectInGroup(m_target, m_missionCriticals)))
	{
		highlightTarget(0);
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::showPlayerShip(bool show)
{
	m_renderPlayerShip = show;
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::showLookAtFocus(bool show)
{
	m_renderFocus = show;
	m_dirty = true;
}

//----------------------------------------------------------------------

void ZoneMapObject::scaleContents(Vector const & scale)
{
	m_scale = scale;
	m_dirty = true;

	setScale(scale);

	scaleObjects(m_asteroids, scale);
	scaleObjects(m_nebulas, scale);
	scaleObjects(m_hyperspacePoints, scale);
	scaleObjects(m_spaceStations, scale);
	scaleObjects(m_activeWaypoints, scale);
	scaleObjects(m_inactiveWaypoints, scale);
	scaleObjects(m_activePoiWaypoints, scale);
	scaleObjects(m_inactivePoiWaypoints, scale);
	scaleObjects(m_missionCriticals, scale);

	if (m_player != 0)
	{
		m_player->setRecursiveScale(scale);
	}
}

//----------------------------------------------------------------------

Vector ZoneMapObject::getHighLightedTargetLocationInZoneSpace() const
{
	if (m_target != 0)
	{
		return m_target->getPosition_p();
	}

	return Vector::zero;
}

//----------------------------------------------------------------------

Vector ZoneMapObject::getLookAtTargetLocationInZoneSpace() const
{
	if (m_focus != 0)
	{
		return m_focus->getPosition_p();
	}

	return Vector::zero;
}

//----------------------------------------------------------------------

void ZoneMapObject::setLookAtTargetLocationInZoneSpace(Vector const & position_o)
{
	if (m_focus != 0)
	{
		m_focus->setPosition_p(position_o);
	}
}//lint !e1762 not const because setting state on members

//----------------------------------------------------------------------

void ZoneMapObject::highlightTarget(Object * const target, bool setFocusTo)
{
	m_focusCameraOnTarget = setFocusTo;

	if ((m_target != 0) && (m_target != m_player))
	{
		m_target->setDynamics(0);
	}

	m_target = target;
	m_dirty = true;

	if (m_target == 0)
	{
		m_target = m_player;
	}

	if ((m_target != 0) && (m_target != m_player))
	{
		PulseDynamics * const pulse = new PulseDynamics(m_target, cs_minimumPulse, cs_maximumPulse, cs_pulseSpeed);
		m_target->setDynamics(pulse);
	}//lint !e429 pulse now owned by m_target

	if ((m_focusCameraOnTarget) && (m_target != 0) && (m_focus != 0))
	{
		m_focus->setPosition_p(m_target->getPosition_p());
	}
}

//----------------------------------------------------------------------

bool ZoneMapObject::isFocusedOnHighLightedTarget() const
{
	return (m_focusCameraOnTarget) && (!m_renderFocus);
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData * ZoneMapObject::closestIntersectedObject(Ray3d const & ray_o) const
{
	ObjectData * bestObject = 0;
	float bestLength = REAL_MAX;

	ObjectData * object = 0;
	float length = REAL_MAX;

	if (m_renderHyperspacePoints)
	{
		object = closestIntersectedObject(m_hyperspacePoints, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}
	}

	if (m_renderSpaceStations)
	{
		object = closestIntersectedObject(m_spaceStations, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}
	}

	if (m_renderWaypoints)
	{
		object = closestIntersectedObject(m_activeWaypoints, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}

		object = closestIntersectedObject(m_inactiveWaypoints, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}

		object = closestIntersectedObject(m_activePoiWaypoints, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}

		object = closestIntersectedObject(m_inactivePoiWaypoints, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}
	}

	if (m_renderMissionCriticals)
	{
		object = closestIntersectedObject(m_missionCriticals, ray_o, length);
		if ((object != 0) && (length < bestLength))
		{
			bestLength = length;
			bestObject = object;
		}
	}

	return bestObject;
}

//----------------------------------------------------------------------

void ZoneMapObject::clearObjectsFromRoot(bool deleteTheObjects)
{
	typedef std::vector<Object *> ObjectsToRemove;
	ObjectsToRemove objectsToRemove;

	int const numberOfChildObjects = getNumberOfChildObjects();
	objectsToRemove.reserve(static_cast<unsigned int>(numberOfChildObjects));

	for (int i = 0; i < numberOfChildObjects; ++i)
	{
		Object * const child = getChildObject(i);
		objectsToRemove.push_back(child);
	}

	ObjectsToRemove::const_iterator ii = objectsToRemove.begin();
	ObjectsToRemove::const_iterator iiEnd = objectsToRemove.end();

	for (; ii != iiEnd; ++ii)
	{
		Object * const object = *ii;

		if (deleteTheObjects)
		{
			delete object;
		}
		else
		{
			removeChildObject(object, Object::DF_none);
		}
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::addObjectToRoot(Object * const child, Transform const & transform_w)
{
	Vector const worldLocation_w = transform_w.getPosition_p();
	Vector const zoneLocation = worldLocationToZoneLocation(worldLocation_w);
	Vector scaledLocation_z;

	float const x = zoneLocation.x * m_scale.x;
	float const y = zoneLocation.y * m_scale.y;
	float const z = zoneLocation.z * m_scale.z;

	Vector const scaledLocation(x, y, z);

	child->setPosition_p(scaledLocation);
	addChildObject_o(child);
}

//----------------------------------------------------------------------

void ZoneMapObject::addObjectsToRoot(DynamicZoneObjects const & zoneObjects)
{
	DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
	DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = ii->second;
		addObjectToRoot(objectData.getObject(), objectData.getTransform_w());
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::addObjectsToRoot(StaticZoneObjects const & zoneObjects)
{
	StaticZoneObjects::const_iterator ii = zoneObjects.begin();
	StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();
	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = (*ii);
		addObjectToRoot(objectData.getObject(), objectData.getTransform_w());
	}
}

//----------------------------------------------------------------------

void ZoneMapObject::deleteObjects(DynamicZoneObjects & zoneObjects)
{
	DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
	DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = ii->second;
		Object * const object = objectData.getObject();
		if (object != 0)
		{
			delete object;
		}
	}
	zoneObjects.clear();
}//lint !e1762 not const for consistency with other helper functions

//----------------------------------------------------------------------

void ZoneMapObject::deleteObjects(StaticZoneObjects & zoneObjects)
{
	StaticZoneObjects::const_iterator ii = zoneObjects.begin();
	StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = (*ii);
		Object * const object = objectData.getObject();
		if (object != 0)
		{
			delete object;
		}
	}
	zoneObjects.clear();
}//lint !e1762 logically not const

//----------------------------------------------------------------------

void ZoneMapObject::scaleObjects(StaticZoneObjects & objects, Vector const & scale)
{
	StaticZoneObjects::const_iterator ii = objects.begin();
	StaticZoneObjects::const_iterator iiEnd = objects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = *ii;
		Object * const object = objectData.getObject();
		if (object != 0)
		{
			Vector relativeScale(object->getScale());
			relativeScale.x *= scale.x;
			relativeScale.y *= scale.y;
			relativeScale.z *= scale.z;
			object->setRecursiveScale(relativeScale);
		}
	}
}//lint !e1762 logically not const

//----------------------------------------------------------------------

void ZoneMapObject::scaleObjects(DynamicZoneObjects & objects, Vector const & scale)
{
	DynamicZoneObjects::const_iterator ii = objects.begin();
	DynamicZoneObjects::const_iterator iiEnd = objects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = ii->second;
		Object * const object = objectData.getObject();
		if (object != 0)
		{
			Vector relativeScale(object->getScale());
			relativeScale.x *= scale.x;
			relativeScale.y *= scale.y;
			relativeScale.z *= scale.z;
			object->setRecursiveScale(relativeScale);
		}
	}
}//lint !e1762 logically not const

//----------------------------------------------------------------------

ZoneMapObject::ObjectData * ZoneMapObject::closestIntersectedObject(StaticZoneObjects const & objects, Ray3d const & ray_p, float & bestLength) const
{
	ObjectData * bestObject = 0;
	bestLength = REAL_MAX;

	StaticZoneObjects::const_iterator ii = objects.begin();
	StaticZoneObjects::const_iterator iiEnd = objects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = *ii;
		Object const * const object = objectData.getObject();

		float length = REAL_MAX;

		if ((testObjectForIntersection(object, ray_p, length)) && (length < bestLength))
		{
			bestObject = const_cast<ObjectData *>(&objectData);
			bestLength = length;
		}
	}

	return bestObject;
}

//----------------------------------------------------------------------

ZoneMapObject::ObjectData * ZoneMapObject::closestIntersectedObject(DynamicZoneObjects const & objects, Ray3d const & ray_p, float & bestLength) const
{
	ObjectData * bestObject = 0;
	bestLength = REAL_MAX;

	DynamicZoneObjects::const_iterator ii = objects.begin();
	DynamicZoneObjects::const_iterator iiEnd = objects.end();

	for(; ii != iiEnd; ++ii)
	{
		ObjectData const & objectData = ii->second;
		Object const * const object = objectData.getObject();

		float length = REAL_MAX;

		if ((testObjectForIntersection(object, ray_p, length)) && (length < bestLength))
		{
			bestObject = const_cast<ObjectData *>(&objectData);
			bestLength = length;
		}
	}

	return bestObject;
}

//======================================================================
