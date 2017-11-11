//======================================================================
//
// PlanetMapManagerClient.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlanetMapManagerClient_H
#define INCLUDED_PlanetMapManagerClient_H

//======================================================================

class ClientWaypointObject;
class MapLocation;
class Vector;
class Vector2d;
class NetworkId;

template <typename T> class ConstWatcher;
template <typename T> class Watcher;

//----------------------------------------------------------------------

class PlanetMapManagerClient
{
public:
	class Messages
	{
   public:
		struct LocationsReceived
		{
			typedef std::pair<bool, std::pair<bool, bool> > Payload;
		};

		struct LocationsRequested
		{
			typedef bool Payload;
		};

		struct GcwEventLocationsRequested
		{
			typedef bool Payload;
		};

		struct GcwEventLocationsReceived
		{
			typedef bool Payload;
		};
	};

	struct PackedGCWRegion
	{
		std::string name;
		float x,z;
		float size;
	};

	typedef stdvector<MapLocation>::fwd     MapLocationVector;
	typedef stdvector<uint8>::fwd           CategoryVector;
	typedef stdvector<Watcher <ClientWaypointObject> >::fwd WaypointVector;
	typedef stdvector<ConstWatcher <ClientWaypointObject> >::fwd ConstWaypointVector;
	typedef stdvector<PackedGCWRegion>::fwd GCWRegionVector;
	typedef stdset< std::pair< std::string, NetworkId > >::fwd GcwEntriesSet;
	typedef stdvector<std::pair< std::string, MapLocation > >::fwd GcwEntriesResponseVector;

	static void                      install                     ();
	static void                      remove                      ();

	static void                      clear                       ();

	static const Unicode::String     getLocalizedEncoded         (const Unicode::String & category);
	static void                      receiveLocations            (const std::string & planet, const MapLocationVector & locsStatic, const MapLocationVector & locsDynamic, const MapLocationVector & locsPersist, int versionStatic, int versionDynamic, int versionPersist);

	static void                      getMapLocations             (uint8 category, MapLocationVector & mlv);
	static const MapLocationVector & getMapLocations             (uint8 category);
	static void                      getCategories               (CategoryVector & sv);
	static void                      requestEntries              ();


	static void                      getPlayerWaypoints          (WaypointVector & wv, const Vector2d & pos, const Unicode::String & name);
	static void                      getPlayerWaypoints          (WaypointVector & wv);
	static void                      getPoiWaypoints             (ConstWaypointVector & wv);

	static void						 getGCWRegions				 (GCWRegionVector & gcwv);

	static ClientWaypointObject *    findWaypoint                (const Vector2d & pos, const Unicode::String & name);

	static Unicode::String           localizeHeadingRange        (const Vector2d & pos, const Vector2d & target);

	static bool                      isRequestInProgress         ();
	static bool                      isGcwRequestInProgress      ();

	static bool                      isPlanetaryMapEnabled       (const std::string & planet, Vector const & pos_w);
	
	static const Unicode::String &   findCategoryString          (uint8 category);

	static void						 requestGcwEntries           (GcwEntriesSet const & values);
	static void                      receiveGcwEntries           (GcwEntriesResponseVector const & values);

	static bool                      getGcwEntryForPlanet        (const std::string & planet, MapLocation & loc);

private:

	static bool                  ms_requestInProgress;
	static bool					 ms_GcwRequestInProgress;
};

//----------------------------------------------------------------------

inline bool PlanetMapManagerClient::isRequestInProgress         ()
{
	return ms_requestInProgress;
}

inline bool PlanetMapManagerClient::isGcwRequestInProgress()
{
	return ms_GcwRequestInProgress;
}


//======================================================================

#endif
