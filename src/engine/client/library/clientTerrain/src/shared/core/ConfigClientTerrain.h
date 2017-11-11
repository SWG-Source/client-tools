//===================================================================
//
// ConfigClientTerrain.h
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ConfigClientTerrain_H
#define INCLUDED_ConfigClientTerrain_H

//===================================================================

class ConfigClientTerrain
{
public:

	static void  install ();

	static bool  getUseOcclusion ();
	static bool  getUseRealGeometryForOcclusion ();
	static bool  getUseClientServerProceduralTerrainAppearanceTemplate ();
	static float getHighLevelOfDetailThreshold ();

	static bool  getTerrainMultiThreaded ();

	static bool  getRadialFloraSortFrontToBack ();

	static float getThreshold ();
	static bool  getShowChunkExtents ();
	static bool  getHeightBiasDisabled ();
	static int   getHeightBiasMax ();
	static float getHeightBiasFactor ();

	static bool  getEnableFlora ();
	static bool  getPreloadGroups ();

	static bool  getDisableTerrainClouds ();
	static bool  getDisableTimeOfDay ();
	static float getEnvironmentStartTime ();
	static bool  getUseNormalizedTime ();
	static float getEnvironmentNormalizedStartTime ();

	static bool  getDisableTerrainBlending ();
	static bool  getShaderGroupUseFirstChildOnly ();
	static bool  getDisableClouds ();

	static bool  getEnableLightScaling ();

	static bool  getDynamicFarFloraEnabled          ();
	static bool  getDynamicNearFloraEnabled         ();
	static bool  getStaticNonCollidableFloraEnabled ();
	static float getDynamicNearFloraDistanceDefault ();
	static float getStaticNonCollidableFloraDistanceDefault ();

	static int   getMaximumNumberOfChunksAllowed ();

private:

	ConfigClientTerrain ();
	ConfigClientTerrain (const ConfigClientTerrain&);
	ConfigClientTerrain& operator= (const ConfigClientTerrain&);
};

//===================================================================

#endif
