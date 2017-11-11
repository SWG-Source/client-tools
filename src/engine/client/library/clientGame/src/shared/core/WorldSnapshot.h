//===================================================================
//
// WorldSnapshot.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshot_H
#define INCLUDED_WorldSnapshot_H

//===================================================================

#include <string>

class CellProperty;
class CrcString;
class NetworkId;
class Transform;
class Vector;
class Object;

//===================================================================

class WorldSnapshot
{
public:

	static void install ();


	static Object* addObject(
		int64 networkIdInt,
		int64 containerIdInt,
		CrcString const &sharedTemplate,
		Transform const &transform_p,
		float radius,
		uint32 portalLayoutCrc,
		int cellCount,
		std::string const &requiredEvent = "");


	static void load               (char const *sceneName);
	static void setExcludeArea     (char const *areaName);
	static void update             (CellProperty const * cellProperty, Vector const & position_w);
	static bool isClientCached     (int64 networkIdInt);
	static void moveObject         (int64 networkIdInt, Transform const &transform_p);
	static void removeObject       ( int64 networkIdInt);
	static void loadIfClientCached (NetworkId const &networkId);
	static void preloadSomeAssets  ();
	static int  getLoadingPercent  ();
	static bool donePreloading     ();
	static float getDetailLevelBias();
	static void setDetailLevelBias (float detailLevelBias);
	static void detailLevelChanged ();
	static NetworkId findClosestCellIdFromWorldPosition(Vector const & position_w);
	static void removeEventObjects (std::string const & eventName);
	static void addEventObjects    (std::string const & eventName);

private:

	static void remove ();
	static void unload ();

private:

	WorldSnapshot ();
	~WorldSnapshot ();
};


//===================================================================

#endif

