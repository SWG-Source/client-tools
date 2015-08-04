//
// ClientProceduralTerrainAppearance_Cache.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Cache.h"

#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

#include <vector>
#include <algorithm>

//-------------------------------------------------------------------

bool                                                              ClientProceduralTerrainAppearance::Cache::ms_installed;
bool                                                              ClientProceduralTerrainAppearance::Cache::ms_locked;
#ifdef _DEBUG
bool                                                              ClientProceduralTerrainAppearance::Cache::ms_debugReport;
#endif
Mutex                                                             ClientProceduralTerrainAppearance::Cache::ms_criticalSection;

ClientProceduralTerrainAppearance::Cache::ColorMapList            ClientProceduralTerrainAppearance::Cache::ms_colorMapList;
ClientProceduralTerrainAppearance::Cache::ShaderGroupList         ClientProceduralTerrainAppearance::Cache::ms_shaderMapList;
ClientProceduralTerrainAppearance::Cache::FloraGroupList          ClientProceduralTerrainAppearance::Cache::ms_floraMapList;
ClientProceduralTerrainAppearance::Cache::RadialGroupList         ClientProceduralTerrainAppearance::Cache::ms_radialMapList;
ClientProceduralTerrainAppearance::Cache::EnvironmentGroupList    ClientProceduralTerrainAppearance::Cache::ms_environmentMapList;
ClientProceduralTerrainAppearance::Cache::NormalMapList           ClientProceduralTerrainAppearance::Cache::ms_normalMapList;

//-------------------------------------------------------------------

namespace CacheNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//-- plus 1 because you can't have 0 texture coordinate sets; the 
	//   texture coordinate set count is used as an index into the cache
	const int maximumNumberOfTextureCoordinateSetsPlus1 = 4 + 1;

	const int preloadChunkSize             = 1024;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<PackedRgb>* localCreateColorMap (const int width, const int height)
	{
		Array2d<PackedRgb>* const map = new Array2d<PackedRgb>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<FloraGroup::Info>* localCreateFloraMap (const int width, const int height)
	{
		Array2d<FloraGroup::Info>* const map = new Array2d<FloraGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<ShaderGroup::Info>* localCreateShaderMap (const int width, const int height)
	{
		Array2d<ShaderGroup::Info>* const map = new Array2d<ShaderGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<RadialGroup::Info>* localCreateRadialMap (const int width, const int height)
	{
		Array2d<RadialGroup::Info>* const map = new Array2d<RadialGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<EnvironmentGroup::Info>* localCreateEnvironmentMap (const int width, const int height)
	{
		Array2d<EnvironmentGroup::Info>* const map = new Array2d<EnvironmentGroup::Info>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Array2d<Vector>* localCreateNormalMap (const int width, const int height)
	{
		Array2d<Vector>* const map = new Array2d<Vector>;
		map->allocate (width, height);

		return map;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace CacheNamespace;

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientProceduralTerrainAppearanceCache::install\n"));

	DEBUG_FATAL (ms_installed, ("already installed"));
	ms_installed = true;

	lock ();

	bool const warmCache = ConfigFile::getKeyBool ("ClientTerrain", "warmCache", true);

	//-- can't precreate maps because we don't know what size they will be
	if (warmCache)
	{
		ms_colorMapList.reserve (2 * preloadChunkSize);
		ms_shaderMapList.reserve (2 * preloadChunkSize);
		ms_floraMapList.reserve (2 * preloadChunkSize);
		ms_radialMapList.reserve (2 * preloadChunkSize);
		ms_environmentMapList.reserve (2 * preloadChunkSize);
		ms_normalMapList.reserve (2 * preloadChunkSize);
	}

	unlock ();

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReport, "ClientTerrain", "reportClientProceduralTerrainAppearanceCache", debugDump);
#endif
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	garbageCollect ();

	ms_installed = false;

#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_debugReport);
#endif
}

//-------------------------------------------------------------------

Array2d<PackedRgb>* ClientProceduralTerrainAppearance::Cache::createColorMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<PackedRgb>* map = 0;

	//-- just take one off the list
	if (!ms_colorMapList.empty ())
	{
		map = ms_colorMapList.back ();
		ms_colorMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateColorMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyColorMap (Array2d<PackedRgb>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_colorMapList.push_back (map);
}

//-------------------------------------------------------------------

Array2d<ShaderGroup::Info>* ClientProceduralTerrainAppearance::Cache::createShaderMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<ShaderGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_shaderMapList.empty ())
	{
		map = ms_shaderMapList.back ();
		ms_shaderMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateShaderMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyShaderMap (Array2d<ShaderGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_shaderMapList.push_back (map);
}

//-------------------------------------------------------------------

Array2d<FloraGroup::Info>* ClientProceduralTerrainAppearance::Cache::createFloraMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<FloraGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_floraMapList.empty ())
	{
		map = ms_floraMapList.back ();
		ms_floraMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateFloraMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyFloraMap (Array2d<FloraGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_floraMapList.push_back (map);
}

//-------------------------------------------------------------------

Array2d<RadialGroup::Info>* ClientProceduralTerrainAppearance::Cache::createRadialMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<RadialGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_radialMapList.empty ())
	{
		map = ms_radialMapList.back ();
		ms_radialMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateRadialMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyRadialMap (Array2d<RadialGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_radialMapList.push_back (map);
}

//-------------------------------------------------------------------

Array2d<EnvironmentGroup::Info>* ClientProceduralTerrainAppearance::Cache::createEnvironmentMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<EnvironmentGroup::Info>* map = 0;

	//-- just take one off the list
	if (!ms_environmentMapList.empty ())
	{
		map = ms_environmentMapList.back ();
		ms_environmentMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateEnvironmentMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyEnvironmentMap (Array2d<EnvironmentGroup::Info>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_environmentMapList.push_back (map);
}

//-------------------------------------------------------------------

Array2d<Vector>* ClientProceduralTerrainAppearance::Cache::createNormalMap (int width, int height)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	Array2d<Vector>* map = 0;

	//-- just take one off the list
	if (!ms_normalMapList.empty ())
	{
		map = ms_normalMapList.back ();
		ms_normalMapList.pop_back ();
	}
	else
	{
		//-- create one
		map = localCreateNormalMap (width, height);
	}
	
	NOT_NULL (map);
	DEBUG_FATAL (width != map->getWidth () && height != map->getHeight (), ("ClientProceduralTerrainAppearance::Cache - cached map of wrong size"));

	return map;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::destroyNormalMap (Array2d<Vector>* map)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!ms_locked, ("not locked"));

	if (map)
		ms_normalMapList.push_back (map);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::debugDump ()
{
#ifdef _DEBUG

	DEBUG_FATAL (!ms_installed, ("not installed"));

	DEBUG_REPORT_PRINT (ms_debugReport, ("-- ClientProceduralTerrainAppearance::Cache: list = size/capacity\n"));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  colorMapList size       = %4i/%4i\n", ms_colorMapList.size (), ms_colorMapList.capacity ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  shaderMapList size      = %4i/%4i\n", ms_shaderMapList.size (), ms_shaderMapList.capacity ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  floraMapList size       = %4i/%4i\n", ms_floraMapList.size (), ms_floraMapList.capacity ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  radialMapList size      = %4i/%4i\n", ms_radialMapList.size (), ms_radialMapList.capacity ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  environmentMapList size = %4i/%4i\n", ms_environmentMapList.size (), ms_environmentMapList.capacity ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("  normalMapList size      = %4i/%4i\n", ms_normalMapList.size (), ms_normalMapList.capacity ()));

#endif
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::lock ()
{
	ms_criticalSection.enter ();

	DEBUG_FATAL (ms_locked, ("already locked"));
	ms_locked = true;
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::unlock ()
{
	DEBUG_FATAL (!ms_locked, ("not locked"));
	ms_locked = false;

	ms_criticalSection.leave ();
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::Cache::garbageCollect ()
{
	if (!ms_installed)
		return;

	lock ();

		//-- clear non-array lists
#define CLEAR_SPECIFIC_LIST(a) \
		{ \
			std::for_each (ms_ ## a ## List.begin (), ms_ ## a ## List.end (), PointerDeleter ()); \
			ms_ ## a ## List.clear (); \
		}

		CLEAR_SPECIFIC_LIST (colorMap);
		CLEAR_SPECIFIC_LIST (shaderMap);
		CLEAR_SPECIFIC_LIST (floraMap);
		CLEAR_SPECIFIC_LIST (radialMap);
		CLEAR_SPECIFIC_LIST (environmentMap);
		CLEAR_SPECIFIC_LIST (normalMap);

	unlock ();
}

//=================================================================

