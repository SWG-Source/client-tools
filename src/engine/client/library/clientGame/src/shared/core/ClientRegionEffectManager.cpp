// ======================================================================
//
// ClientRegionEffectManager.cpp
// mboudreaux
// 
// copyright 2008, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientRegionEffectManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Fatal.h"
#include "sharedFoundation/TemporaryCrcString.h"

#include <map>

// ======================================================================

typedef std::map< TemporaryCrcString, int32 > RegionEffectMap;


namespace ClientRegionEffectManagerNamespace
{
	bool            s_installed = false;

	RegionEffectMap s_regionEffectMap;

	const char *    s_regionEffects[] = { "permanentDay", "permanentNight", "spookyZone" }; // This declaration must match EXACTLY the region_environment_flags.tab/.iff

	bool lookUpKey(TemporaryCrcString const & key, int32 const flags)
	{
		RegionEffectMap::iterator iter = s_regionEffectMap.find(key);
		if(iter != s_regionEffectMap.end())
			return (flags & iter->second) != 0;
		
		return false;
	}
}

using namespace ClientRegionEffectManagerNamespace;

#define REGION_EFFECTS_SIZE (sizeof(s_regionEffects)/sizeof(s_regionEffects[0]))

void ClientRegionEffectManager::install()
{
	DEBUG_FATAL(s_installed, ("ClientRegionManager already installed."));

	for(int i = 0; i < REGION_EFFECTS_SIZE; ++i)
	{
		s_regionEffectMap.insert( std::make_pair<TemporaryCrcString, int32 >( TemporaryCrcString(s_regionEffects[i], true), 1 << i ) );
	}

	s_installed = true;
}

void ClientRegionEffectManager::remove()
{
	DEBUG_FATAL(!s_installed, ("ClientRegionManager was not installed."));

	s_installed = false;

	if(!s_regionEffectMap.empty())
		s_regionEffectMap.clear();
}

bool ClientRegionEffectManager::isCurrentRegionPermanentDay(int32 const regionFlags)
{
	if(s_regionEffectMap.empty())
		return false;
	
	static TemporaryCrcString permDay = TemporaryCrcString("permanentDay", true);

	return lookUpKey(permDay, regionFlags);
}

bool ClientRegionEffectManager::isCurrentRegionPermanentNight(int32 const regionFlags)
{
	if(s_regionEffectMap.empty())
		return false;

	static TemporaryCrcString permNight = TemporaryCrcString("permanentNight", true);

	return lookUpKey(permNight, regionFlags);
}

bool ClientRegionEffectManager::isCurrentRegionSpookyZone(int32 const regionFlags)
{
	if(s_regionEffectMap.empty())
		return false;

	static TemporaryCrcString spookyZone = TemporaryCrcString("spookyZone", true);

	return lookUpKey(spookyZone, regionFlags);
}