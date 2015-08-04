//===================================================================
//
// ClientRegionEffectManager.h
// mboudreaux
//
// copyright 2008, sony online entertainment
// 
//===================================================================

#ifndef INCLUDED_ClientRegionEffectManager_H
#define INCLUDED_ClientRegionEffectManager_H

//===================================================================

class ClientRegionEffectManager
{
public:
	static void install();
	static void remove();

	static bool isCurrentRegionPermanentDay(int32 const regionFlags);
	static bool isCurrentRegionPermanentNight(int32 const regionFlags);
	static bool isCurrentRegionSpookyZone(int32 const regionFlags);
};

#endif
//===================================================================