//===================================================================
//
// ConfigClientTerrain.cpp
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ConfigClientTerrain.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Production.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

//===================================================================

namespace
{
	bool  ms_useOcclusion;
	bool  ms_useRealGeometryForOcclusion;
	bool  ms_useClientServerProceduralTerrainAppearanceTemplate;
	float ms_highLevelOfDetailThreshold;

	bool  ms_terrainMultiThreaded;

	bool  ms_radialFloraSortFrontToBack;

	float ms_threshold;
	bool  ms_showChunkExtents;
	bool  ms_heightBiasDisabled;
	int   ms_heightBiasMax;
	float ms_heightBiasFactor;

	bool  ms_enableFlora;
	bool  ms_preloadGroups;

	bool  ms_disableTerrainClouds;

	float ms_environmentStartTime;
	float ms_environmentNormalizedStartTime;
	bool  ms_useNormalizedTime;
	bool  ms_disableTimeOfDay;

	bool  ms_disableTerrainBlending;
	bool  ms_shaderGroupUseFirstChildOnly;

	bool  ms_disableClouds;

	bool  ms_enableLightScaling;

	bool  ms_dynamicFarFloraEnabled;
	bool  ms_dynamicNearFloraEnabled;
	bool  ms_staticNonCollidableFloraEnabled;

	int   ms_maximumNumberOfChunksAllowed;
}

//===================================================================

bool ConfigClientTerrain::getUseOcclusion ()
{
	return ms_useOcclusion;
}

//-----------------------------------------------------------------

bool ConfigClientTerrain::getUseClientServerProceduralTerrainAppearanceTemplate ()
{
	return ms_useClientServerProceduralTerrainAppearanceTemplate;
}

//-----------------------------------------------------------------

float ConfigClientTerrain::getHighLevelOfDetailThreshold ()
{
	return ms_highLevelOfDetailThreshold;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getTerrainMultiThreaded ()
{
	return ms_terrainMultiThreaded;
}

// ----------------------------------------------------------------------

bool ConfigClientTerrain::getRadialFloraSortFrontToBack ()
{
	return ms_radialFloraSortFrontToBack;
}

//-------------------------------------------------------------------

float ConfigClientTerrain::getThreshold ()
{
	return ms_threshold;
}

//-----------------------------------------------------------------

bool ConfigClientTerrain::getShowChunkExtents ()
{
	return ms_showChunkExtents;
}

//-----------------------------------------------------------------

bool ConfigClientTerrain::getHeightBiasDisabled ()
{
	return ms_heightBiasDisabled;
}

//-----------------------------------------------------------------

int ConfigClientTerrain::getHeightBiasMax ()
{
	return ms_heightBiasMax;
}

//-----------------------------------------------------------------

float ConfigClientTerrain::getHeightBiasFactor ()
{
	return ms_heightBiasFactor;
}

//-----------------------------------------------------------------

bool ConfigClientTerrain::getEnableFlora ()
{
	return ms_enableFlora;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getPreloadGroups ()
{
	return ms_preloadGroups;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getUseRealGeometryForOcclusion ()
{
	return ms_useRealGeometryForOcclusion;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getDisableTerrainClouds ()
{
	return ms_disableTerrainClouds;
}

//-------------------------------------------------------------------

float ConfigClientTerrain::getEnvironmentStartTime ()
{
	return ms_environmentStartTime;
}

//-------------------------------------------------------------------

float ConfigClientTerrain::getEnvironmentNormalizedStartTime ()
{
	return ms_environmentNormalizedStartTime;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getUseNormalizedTime ()
{
	return ms_useNormalizedTime;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getDisableTimeOfDay ()
{
	return ms_disableTimeOfDay;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getDisableTerrainBlending ()
{
	return ms_disableTerrainBlending;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getShaderGroupUseFirstChildOnly ()
{
	return ms_shaderGroupUseFirstChildOnly;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getDisableClouds ()
{
	return ms_disableClouds;
}

//-------------------------------------------------------------------

bool ConfigClientTerrain::getEnableLightScaling ()
{
	return ms_enableLightScaling;
}

//----------------------------------------------------------------------

bool  ConfigClientTerrain::getDynamicFarFloraEnabled     ()
{
	return ms_dynamicFarFloraEnabled;
}

//----------------------------------------------------------------------

bool  ConfigClientTerrain::getDynamicNearFloraEnabled    ()
{
	return ms_dynamicNearFloraEnabled;
}

//----------------------------------------------------------------------

bool  ConfigClientTerrain::getStaticNonCollidableFloraEnabled ()
{
	return ms_staticNonCollidableFloraEnabled;
}

//-------------------------------------------------------------------

float ConfigClientTerrain::getDynamicNearFloraDistanceDefault ()
{
	return 32.f;
}

//-------------------------------------------------------------------

float ConfigClientTerrain::getStaticNonCollidableFloraDistanceDefault ()
{
	return 64.f;
}

//-------------------------------------------------------------------

int ConfigClientTerrain::getMaximumNumberOfChunksAllowed ()
{
	return ms_maximumNumberOfChunksAllowed;
}

//===================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientTerrain", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("ClientTerrain", #a, b))
#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientTerrain", #a, b))
//#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("ClientTerrain", #a, b))

//===================================================================

void ConfigClientTerrain::install()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ConfigClientTerrain::install\n"));

	KEY_BOOL   (useOcclusion,                        false);
	KEY_BOOL   (useClientServerProceduralTerrainAppearanceTemplate, false);
	KEY_BOOL   (useRealGeometryForOcclusion,         false);
	KEY_FLOAT  (highLevelOfDetailThreshold,          4.5f);

	KEY_BOOL   (terrainMultiThreaded,                true);

	KEY_BOOL   (radialFloraSortFrontToBack,          false);

	//-- testing minimums
	KEY_FLOAT  (threshold,                           4.f);
	KEY_BOOL   (showChunkExtents,                    false);
	KEY_BOOL   (heightBiasDisabled,                  true);
	KEY_INT    (heightBiasMax,                       50);
	KEY_FLOAT  (heightBiasFactor,                    8);
	KEY_BOOL   (enableFlora,                         true);
	KEY_BOOL   (preloadGroups,                       PRODUCTION ? true : false);

	KEY_BOOL   (disableTerrainClouds,                true);

	KEY_BOOL   (disableTimeOfDay,                    false);
	KEY_FLOAT  (environmentStartTime,                300.f);
	KEY_BOOL   (useNormalizedTime,                   false);
	KEY_FLOAT  (environmentNormalizedStartTime,      0.525f);

	KEY_BOOL   (disableTerrainBlending,              false);
	KEY_BOOL   (shaderGroupUseFirstChildOnly,        false);
	KEY_BOOL   (disableClouds,                       false);
	KEY_BOOL   (enableLightScaling,                  true);

	KEY_BOOL   (dynamicFarFloraEnabled,              true);
	KEY_BOOL   (dynamicNearFloraEnabled,             true);
	KEY_BOOL   (staticNonCollidableFloraEnabled,     true);
	KEY_INT    (maximumNumberOfChunksAllowed,        10 * 1024);
}

//===================================================================
