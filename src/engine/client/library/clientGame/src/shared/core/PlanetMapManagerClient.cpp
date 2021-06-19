//======================================================================
//
// PlanetMapManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlanetMapManagerClient.h"

#include "StringId.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedMissionObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetMapLocationsMessage.h"
#include "sharedNetworkMessages/GetMapLocationsResponseMessage.h"
#include "sharedObject/Container.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>
#include <set>
#include <vector>

//======================================================================

namespace PlanetMapManagerClientNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const PlanetMapManagerClient::Messages::LocationsReceived::Payload &,   PlanetMapManagerClient::Messages::LocationsReceived >
			locationsReceived;
		MessageDispatch::Transceiver<const PlanetMapManagerClient::Messages::LocationsRequested::Payload &,   PlanetMapManagerClient::Messages::LocationsRequested >
			locationsRequested;
		MessageDispatch::Transceiver<const PlanetMapManagerClient::Messages::GcwEventLocationsRequested::Payload &, PlanetMapManagerClient::Messages::GcwEventLocationsRequested >
			gcwEventLocationsRequested;
		MessageDispatch::Transceiver<const PlanetMapManagerClient::Messages::GcwEventLocationsReceived::Payload &, PlanetMapManagerClient::Messages::GcwEventLocationsReceived > 
			gcwEventLocationsReceived;
	}

	bool              s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;
	}

	typedef stdset<std::string>::fwd DisabledPlanetaryMap;
	DisabledPlanetaryMap s_disabledPlanetaryMap;

	typedef stdmap<uint8, PlanetMapManagerClient::MapLocationVector>::fwd LocationTypeEntries;
	LocationTypeEntries s_locationTypeEntries;
	const PlanetMapManagerClient::MapLocationVector s_emptyMapLocations;

	PlanetMapManagerClient::MapLocationVector s_locsStatic;
	PlanetMapManagerClient::MapLocationVector s_locsDynamic;
	PlanetMapManagerClient::MapLocationVector s_locsPersist;

	// the game server has reserved version number 0 to be used to force an update
	int s_versionStatic  = 0;
	int s_versionDynamic = 0;
	int s_versionPersist = 0;
	
	typedef stdmap<uint8,       Unicode::String>::fwd TypeStringMap;
	TypeStringMap  s_typeStringMap;

	typedef stdmap<std::string, stdmap<std::string, std::pair<std::pair<float, float>, float> >::fwd >::fwd GalaxyGCWRegionMap;
	typedef stdmap<std::string, std::pair<std::pair<float, float>, float> >::fwd  PlanetGCWRegionMap;
	typedef stdmap<std::string, MapLocation>::fwd GcwEntriesMap;

	GcwEntriesMap ms_gcwEntriesMap;
	
	//----------------------------------------------------------------------

	class Listener :
	public MessageDispatch::Receiver
	{
	public:

		//----------------------------------------------------------------------

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (GetMapLocationsResponseMessage::MessageType);
			connectToMessage(Game::Messages::SCENE_CHANGED);
			connectToMessage ("GetSpecificMapLocationsResponseMessage");
		}

		//----------------------------------------------------------------------

		void receiveMessage (const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------

			if (message.isType (GetMapLocationsResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const GetMapLocationsResponseMessage msg (ri);
				PlanetMapManagerClient::receiveLocations (msg.getPlanetName (),
					msg.getMapLocationsStatic (),
					msg.getMapLocationsDynamic (),
					msg.getMapLocationsPersist (),
					msg.getVersionStatic (),
					msg.getVersionDynamic (),
					msg.getVersionPersist ());
			}
			else if(message.isType(Game::Messages::SCENE_CHANGED))
			{
				PlanetMapManagerClient::clear();
			}
			else if(message.isType("GetSpecificMapLocationsResponseMessage"))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream ().begin ();
				const GenericValueTypeMessage<std::vector<std::pair<std::string, MapLocation> > > msg(ri);
				const std::vector<std::pair<std::string, MapLocation> > & data = static_cast<std::vector<std::pair<std::string, MapLocation> > const &>(msg.getValue());
				PlanetMapManagerClient::receiveGcwEntries(data);
			}
		}
	};

	Listener * s_listener = 0;

	static const std::string s_testingCategories [] =
	{
		"bank",
		"cantina",
		"capitol",
		"garage",
		"guild",
		"hotel",
		"shuttleport",
		"starport",
		"themepark",
	};

	static const int s_numTestingCategories = static_cast<int>(sizeof (s_testingCategories) / sizeof (s_testingCategories [0]));

	void singlePlayerCallToPopulate (const void *)
	{
		float mapWidth = 4096.0f;

		const TerrainObject * const terrain = TerrainObject::getInstance ();
		if (terrain)
		{
			const ProceduralTerrainAppearance * const pta = dynamic_cast<const ProceduralTerrainAppearance *>(terrain->getAppearance ());
			if (pta)
				mapWidth = pta->getMapWidthInMeters ();
		}

		static PlanetMapManagerClient::CategoryVector sv;
		sv.clear ();
		PlanetMapManagerClient::getCategories (sv);

		if (std::find (sv.begin (), sv.end (), PlanetMapManager::getCityCategory ()) == sv.end ())
			sv.push_back (PlanetMapManager::getCityCategory ());

		static PlanetMapManagerClient::MapLocationVector mlv;
		mlv.clear ();

		for (PlanetMapManagerClient::CategoryVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const uint8 category = *it;

			int numEntries       = Random::random (10, 50);
			int numSubCategories = Random::random (numEntries / 4, numEntries / 2);

			for (int i = 0; i < numEntries; ++i)
			{
				MapLocation ml;
				ml.m_location  = Vector2d (Random::randomReal (-mapWidth, mapWidth) * 0.5f, Random::randomReal (-mapWidth, mapWidth) * 0.5f);
				ml.m_category  = category;

				char buf [64];

				ml.m_subCategory = static_cast<uint8>(i % numSubCategories);

				ml.m_locationName = Unicode::narrowToWide (buf);

				mlv.push_back (ml);
			}
		}

		PlanetMapManagerClient::receiveLocations (Game::getSceneId (), mlv, s_emptyMapLocations, s_emptyMapLocations, 0, 0, 0);
	}
}

using namespace PlanetMapManagerClientNamespace;

//----------------------------------------------------------------------

bool PlanetMapManagerClient::ms_requestInProgress = false;
bool PlanetMapManagerClient::ms_GcwRequestInProgress = false;

//----------------------------------------------------------------------

void PlanetMapManagerClient::install ()
{
	InstallTimer const installTimer("PlanetMapManagerClient::install");

	s_listener = new Listener;
	clear ();

	// build the list of planets where planetary map is disabled
	const std::string disabledPlanetaryMapIff("datatables/planetary_map/disabled_planetary_map.iff");
	DataTable * const t = DataTableManager::getTable(disabledPlanetaryMapIff, true);
	if (!t)
	{
		WARNING(true, ("PlanetMapManagerClient::install() - unable to load data table (%s)", disabledPlanetaryMapIff.c_str()));
		return;
	}

	for (int row = 0; row < t->getNumRows(); ++row)
		IGNORE_RETURN(s_disabledPlanetaryMap.insert(t->getStringValue("Planet", row)));

	DataTableManager::close(disabledPlanetaryMapIff);
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::remove ()
{
	delete s_listener;
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::clear()
{
	// Once the data is cleared, ensure we can repost the data.
	ms_requestInProgress = false;

	s_locationTypeEntries.clear ();

	// the game server has reserved version number 0 to be used to force an update
	s_versionStatic = s_versionDynamic = s_versionPersist = 0;

	s_locsStatic.clear  ();
	s_locsDynamic.clear ();
	s_locsPersist.clear ();

	ms_gcwEntriesMap.clear();
}

//----------------------------------------------------------------------

const Unicode::String PlanetMapManagerClient::getLocalizedEncoded (const Unicode::String & category)
{
	if (category.empty ())
		return Unicode::emptyString;

	if (category [0] == '@')
		return StringId (Unicode::wideToNarrow (category.c_str () + 1)).localize ();

	return category;
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::getCategories  (CategoryVector & sv)
{
	if (Game::getSinglePlayer ())
	{
		sv.push_back(PlanetMapManager::getPoiCategory());
//		for (int i = 0; i < s_numTestingCategories; ++i)
//		{
//			s_locationTypeEntries [static_cast<uint8>(i)];
//		}
	}

	sv.reserve (s_locationTypeEntries.size ());

	for (LocationTypeEntries::const_iterator it = s_locationTypeEntries.begin (); it != s_locationTypeEntries.end (); ++it)
	{
		sv.push_back ((*it).first);
	}
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::getMapLocations (uint8 category, MapLocationVector & mlv)
{
	MapLocationVector & tmp = s_locationTypeEntries [category];

	if (category == PlanetMapManager::getWaypointCategory ())
	{
		tmp.clear ();
		static WaypointVector wv;
		wv.clear ();
		getPlayerWaypoints (wv);

		for (WaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			const ClientWaypointObject * const waypoint = *it;
			MapLocation ml;

			if(waypoint->getColor() == "invisible")
				continue;

			ml.m_locationId        = waypoint->getNetworkId ();
			ml.m_location          = Vector2d (waypoint->getLocation ().x, waypoint->getLocation ().z);
			ml.m_locationName      = waypoint->getLocalizedName ();
			ml.m_category          = PlanetMapManager::getWaypointCategory ();
			tmp.push_back (ml);
		}
	}
	else if (category == PlanetMapManager::getPoiCategory ())
	{
		tmp.clear ();
		static ConstWaypointVector wv;
		wv.clear ();
		getPoiWaypoints (wv);

		for (ConstWaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			const ClientWaypointObject * const waypoint = *it;
			MapLocation ml;

			ml.m_locationId        = waypoint->getNetworkId ();
			ml.m_location          = Vector2d (waypoint->getLocation ().x, waypoint->getLocation ().z);
			ml.m_locationName      = waypoint->getLocalizedName ();
			ml.m_category          = PlanetMapManager::getPoiCategory ();
			tmp.push_back (ml);
		}
	}
	else if(category == PlanetMapManager::getGCWRegionCategory())
	{
		tmp.clear();
		GCWRegionVector gcwrv;
		gcwrv.clear();
		getGCWRegions(gcwrv);

		for(GCWRegionVector::const_iterator iter = gcwrv.begin(); iter != gcwrv.end(); ++iter)
		{
			const PackedGCWRegion * const region = &(*iter);
			MapLocation ml;
			ml.m_locationId   = NetworkId::cms_invalid;
			ml.m_location     = Vector2d(region->x, region->z);
			ml.m_locationName = Unicode::narrowToWide(region->name.c_str());
			ml.m_category     = PlanetMapManager::getGCWRegionCategory();
			ml.m_size		  = region->size;

			tmp.push_back(ml);
		}

	}

	mlv = tmp;
}

//----------------------------------------------------------------------

const PlanetMapManagerClient::MapLocationVector& PlanetMapManagerClient::getMapLocations (uint8 category)
{
	LocationTypeEntries::iterator iter = s_locationTypeEntries.find (category);
	if (iter != s_locationTypeEntries.end ())
		return iter->second;

	return s_emptyMapLocations;
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::requestEntries ()
{
	if (ms_requestInProgress)
		return;

	ms_requestInProgress = true;
	if (Game::getSinglePlayer ())
	{
//		GameScheduler::addCallback (singlePlayerCallToPopulate, 0, 1.0f);
	}
	else
	{
		const GetMapLocationsMessage msg (Game::getSceneId (), s_versionStatic, s_versionDynamic, s_versionPersist);
		GameNetwork::send (msg, true);
	}

	Transceivers::locationsRequested.emitMessage (true);
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::getPlayerWaypoints (WaypointVector & wv, const Vector2d & pos, const Unicode::String & name)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	ClientObject * const datapad = player->getDatapadObject ();
	if (!datapad)
		return;

	const Container * const container = ContainerInterface::getContainer (*datapad);

	if (!container)
		return;

	const std::string & sceneId = Game::getSceneId ();

	for (ContainerConstIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
	{
		const CachedNetworkId & id = *containerIterator;
		Object * const obj = id.getObject ();
		if (!obj)
			continue;

		if (obj->getObjectType () == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
		{
			ClientWaypointObject * const waypoint = safe_cast<ClientWaypointObject *>(obj);

			if (waypoint->isWaypointVisible ())
			{
				if (sceneId != waypoint->getPlanetName ())
					continue;

				if (!name.empty ())
				{
					if (name != waypoint->getLocalizedName ())
						continue;

					const Vector & waypointLoc = waypoint->getLocation ();
					if (static_cast<int>(pos.x) != static_cast<int>(waypointLoc.x) ||
						static_cast<int>(pos.y) != static_cast<int>(waypointLoc.z))
						continue;
					wv.push_back (Watcher<ClientWaypointObject>(waypoint));

					return;
				}

				wv.push_back (Watcher<ClientWaypointObject>(waypoint));
			}
		}

		else if (obj->getObjectType() == SharedMissionObjectTemplate::SharedMissionObjectTemplate_tag)
		{
			const ClientMissionObject * const mission = safe_cast<ClientMissionObject *>(obj);
			if (mission)
			{
				const Container * const c = ContainerInterface::getContainer(*mission);
				if(c)
				{
					for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
					{
						const Container::ContainedItem & item = *i;
						Object * const child = item.getObject();
						if (child && child->getObjectType () == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
						{
							ClientWaypointObject * const waypoint = safe_cast<ClientWaypointObject*>(child);

							if (waypoint && waypoint->isWaypointVisible())
							{
								if (sceneId != waypoint->getPlanetName ())
									continue;

								wv.push_back (Watcher<ClientWaypointObject>(waypoint));
							}
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::getPlayerWaypoints (WaypointVector & wv)
{
	getPlayerWaypoints (wv, Vector2d (0.0f, 0.0f), Unicode::emptyString);
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::getPoiWaypoints (ConstWaypointVector & wv)
{
	const ConstWaypointVector & pois = ClientWaypointObject::getPoiWaypoints();
	for(ConstWaypointVector::const_iterator it = pois.begin(); it != pois.end(); ++it)
	{
		const ClientWaypointObject* const wp = *it;
		if(wp)
		{
			const std::string & sceneId = Game::getSceneId ();
			if (wp->getPlanetName () == sceneId)
				wv.push_back(*it);
		}
	}
}

//----------------------------------------------------------------------

ClientWaypointObject * PlanetMapManagerClient::findWaypoint (const Vector2d & pos, const Unicode::String & name)
{
	WaypointVector wv;
	getPlayerWaypoints (wv, pos, name);
	if (wv.empty ())
		return 0;

	return wv.back().getPointer();
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::receiveLocations    (const std::string & planet,
												  const MapLocationVector & locsStatic,
												  const MapLocationVector & locsDynamic,
												  const MapLocationVector & locsPersist,
												  int versionStatic,
												  int versionDynamic,
												  int versionPersist)
{
	WARNING (!ms_requestInProgress, ("PlanetMapManagerClient received locations but no request was in progress"));

	ms_requestInProgress = false;

	const std::string & sceneId = Game::getSceneId ();

	if (planet != sceneId)
	{
		WARNING (true, ("PlanetMapManagerClient received planet locations for %s, but we are now on %s", planet.c_str (), sceneId.c_str ()));
		return;
	}

	bool versionStaticChanged = false;
	bool versionDynamicChanged = false;
	bool versionPersistChanged = false;

	if (versionStatic != s_versionStatic)
	{
		versionStaticChanged = true;
		s_versionStatic = versionStatic;
		s_locsStatic    = locsStatic;
	}

	if (versionDynamic != s_versionDynamic)
	{
		versionDynamicChanged = true;
		s_versionDynamic = versionDynamic;
		s_locsDynamic    = locsDynamic;
	}

	if (versionPersist != s_versionPersist)
	{
		versionPersistChanged = true;
		s_versionPersist = versionPersist;
		s_locsPersist    = locsPersist;
	}

	// don't need to do anything if we didn't get anything new
	if (!versionStaticChanged && !versionDynamicChanged && !versionPersistChanged)
	{
		Transceivers::locationsReceived.emitMessage (std::make_pair(false, std::make_pair(false, false)));
		return;
	}

	s_locationTypeEntries.clear ();

	{
		for (MapLocationVector::const_iterator it = s_locsStatic.begin (); it != s_locsStatic.end (); ++it)
		{
			const MapLocation & ml = *it;
			s_locationTypeEntries [ml.m_category].push_back (ml);
		}
	}
	{
		for (MapLocationVector::const_iterator it = s_locsDynamic.begin (); it != s_locsDynamic.end (); ++it)
		{
			const MapLocation & ml = *it;
			s_locationTypeEntries [ml.m_category].push_back (ml);
		}
	}
	{
		for (MapLocationVector::const_iterator it = s_locsPersist.begin (); it != s_locsPersist.end (); ++it)
		{
			const MapLocation & ml = *it;
			s_locationTypeEntries [ml.m_category].push_back (ml);
		}
	}

	{
		// Add the special POI waypoints

		ConstWaypointVector wv;
		getPoiWaypoints (wv);

		for (ConstWaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			const ClientWaypointObject * const waypoint = *it;
			MapLocation ml;

			ml.m_locationId        = waypoint->getNetworkId ();
			ml.m_location          = Vector2d (waypoint->getLocation ().x, waypoint->getLocation ().z);
			ml.m_locationName      = waypoint->getLocalizedName ();
			ml.m_category          = PlanetMapManager::getPoiCategory ();

			s_locationTypeEntries [ml.m_category].push_back (ml);
		}
	}

	Transceivers::locationsReceived.emitMessage (std::make_pair(versionStaticChanged, std::make_pair(versionDynamicChanged, versionPersistChanged)));
}

//----------------------------------------------------------------------

Unicode::String PlanetMapManagerClient::localizeHeadingRange        (const Vector2d & pos, const Vector2d & target)
{
	const Vector2d deltaVector = target - pos;
	const int mag       = static_cast<int>(deltaVector.magnitude ());
	const float theta   = deltaVector.theta ();

	static const char * abbrevs [] =
	{
		"S",
		"SW",
		"W",
		"NW",
		"N",
		"NE",
		"E",
		"SE"
	};

	static const int     num_abbrevs = static_cast<int>(sizeof (abbrevs) / sizeof (abbrevs [0]));
	static const float f_num_abbrevs = static_cast<float>(num_abbrevs);

	static const float   angle_increment = (2.0f * PI / f_num_abbrevs);

	//-- transformed so zero radians is north
	const float start_angle = -PI + angle_increment * 0.5f;

	int index = 0;
	for (int i = 0; i < num_abbrevs; ++i)
	{
		const float target_angle = start_angle + angle_increment * static_cast<float>(i);

		if (theta < target_angle)
		{
			index = i;
			break;
		}
	}

	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	_snprintf (buf, buf_size, "%5dm %3s", mag, abbrevs [index]);
	return Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

bool PlanetMapManagerClient::isPlanetaryMapEnabled(const std::string & planet, Vector const & pos_w)
{
	DisabledPlanetaryMap::const_iterator iter = s_disabledPlanetaryMap.find(planet);

	if (iter != s_disabledPlanetaryMap.end())
		return false;

	BuildoutArea const * const ba = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(planet, pos_w.x, pos_w.z, true);
	if (NULL != ba)
	{
		if (!ba->allowMap)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------

const Unicode::String & PlanetMapManagerClient::findCategoryString   (uint8 category)
{

	TypeStringMap::const_iterator const it = s_typeStringMap.find (category);
	if (it != s_typeStringMap.end ())
		return (*it).second;
	else
	{			
		Unicode::String const & result = PlanetMapManager::findCategoryStringId(category).localize();
		s_typeStringMap.insert (std::make_pair (category, result));
		TypeStringMap::const_iterator const it = s_typeStringMap.find (category);
		if (it != s_typeStringMap.end ())
			return (*it).second;
	}

	return Unicode::emptyString;
}

void PlanetMapManagerClient::getGCWRegions(GCWRegionVector & gcwv)
{
	GalaxyGCWRegionMap const & regionMap = GameNetwork::getGcwScoreCategoryRegions();

	std::string const & currentPlanet = Game::getSceneId();
	
	GalaxyGCWRegionMap::const_iterator planetRegions = regionMap.find(currentPlanet);
	if(planetRegions == regionMap.end())
	{
		//DEBUG_WARNING(true, ("PlanetMapManagerClient: Failed to find GCW Regions for scene[%s]", currentPlanet.c_str()));
		return;
	}

	PlanetGCWRegionMap const & planetRegionList = (*planetRegions).second;

	PlanetGCWRegionMap::const_iterator iter = planetRegionList.begin();
	for(; iter != planetRegionList.end(); ++iter)
	{
		PackedGCWRegion newRegion;

		newRegion.name = (*iter).first;
		newRegion.x    = (*iter).second.first.first;
		newRegion.z    = (*iter).second.first.second;
		newRegion.size = (*iter).second.second;

		gcwv.push_back(newRegion);
	}

}

//----------------------------------------------------------------------

void PlanetMapManagerClient::requestGcwEntries (std::set< std::pair< std::string, NetworkId > > const & values)
{
	if (ms_GcwRequestInProgress)
		return;

	ms_GcwRequestInProgress = true;
	if (Game::getSinglePlayer ())
	{
		//		GameScheduler::addCallback (singlePlayerCallToPopulate, 0, 1.0f);
	}
	else
	{
		const GenericValueTypeMessage< std::set< std::pair< std::string, NetworkId > > > msg("GetSpecificMapLocationsMessage", values); 
		GameNetwork::send (msg, true);
	}

	Transceivers::gcwEventLocationsRequested.emitMessage (true);
}

//----------------------------------------------------------------------

void PlanetMapManagerClient::receiveGcwEntries(std::vector< std::pair< std::string, MapLocation > > const & values)
{
	ms_gcwEntriesMap.clear();
	std::vector< std::pair< std::string, MapLocation > >::const_iterator iter = values.begin();
	for(; iter != values.end(); ++iter)
	{
		ms_gcwEntriesMap.insert(std::make_pair(iter->first, iter->second));
	}

	Transceivers::gcwEventLocationsReceived.emitMessage(true);

	ms_GcwRequestInProgress = false;
}


bool PlanetMapManagerClient::getGcwEntryForPlanet(const std::string & planet, MapLocation & loc)
{
	GcwEntriesMap::const_iterator iter = ms_gcwEntriesMap.find(planet);
	if(iter != ms_gcwEntriesMap.end())
	{
		loc = iter->second;
		return true;
	}

	return false;
}

//======================================================================
