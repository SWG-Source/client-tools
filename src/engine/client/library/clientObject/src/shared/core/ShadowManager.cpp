// ======================================================================
// 
// ShadowManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShadowManager.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/ShadowBlobManager.h"
#include "clientObject/ShadowVolume.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/LocalMachineOptionManager.h"

// ======================================================================

namespace ShadowManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum SkeletalShadowType
	{
		SST_none,
		SST_simple,
		SST_volumetric
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float cms_minimumVolumetricShadowDistance = 128.f;
	const float cms_maximumVolumetricShadowDistance = 512.f;
	const float cms_minimumSimpleShadowDistance     = 32.f;
	const float cms_maximumSimpleShadowDistance     = 256.f;
	const float cms_ignoreRatio                     = 10.f / 800.f;
	const float cms_fogNaturalBase					= 2.71828f;
	const float cms_fogCutOff						= 0.20f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  ms_enabled                  = false;
	bool ms_allowed = true;
	bool  ms_debugReport              = false;
	float ms_maximumVolumetricShadowDistance = cms_minimumVolumetricShadowDistance;
	float ms_volumetricShadowDistanceLevel = 0.f;
	float ms_shadowDetailLevel        = 0.f;
	float ms_volumetricShadowDistance = cms_minimumVolumetricShadowDistance;
	bool  ms_meshShadows              = false;
	float ms_simpleShadowDistance     = cms_minimumSimpleShadowDistance;
	int   ms_skeletalShadows          = SST_none;
	float ms_timeOfDay				  = 0.0f;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ()
	{
		DebugFlags::unregisterFlag (ms_debugReport);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void updateShadows ()
	{
		ShadowBlobManager::setEnabled (ms_skeletalShadows == SST_simple);
		ShadowVolume::setEnabled (ms_meshShadows || ms_skeletalShadows == SST_volumetric);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline bool shouldRender (Camera const & camera, Vector const & position_w, float const radius, float const maximumDistance)
	{
		const int ignoreSize = static_cast<int> (cms_ignoreRatio * Graphics::getCurrentRenderTargetWidth ());

		//-- check minimum screen size
		float screenRadius = 0.f;
		if (camera.computeRadiusInScreenSpace (camera.rotateTranslate_w2o (position_w), radius, screenRadius) && screenRadius < ignoreSize)
			return false;

		//-- check maximum distance
		if (camera.getPosition_w ().magnitudeBetweenSquared (position_w) > sqr (maximumDistance))
			return false;
		
		bool fogEnabled;
		float fogDensity;
		PackedArgb fogColor;
		Graphics::getFog(fogEnabled, fogDensity, fogColor);

		if(fogEnabled && fogDensity != 0.0f)
		{
			// Calculate our fog value
			float distance = camera.getPosition_w ().magnitudeBetween(position_w);
			float distanceDensitySqrd = pow(distance * fogDensity, 2);
			float baseDistanceDensity = pow(cms_fogNaturalBase, distanceDensitySqrd);
			float finalFogValue = 1.0f / baseDistanceDensity;

			if(finalFogValue < cms_fogCutOff)
				return false;
		}

		return true;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("-- ShadowManager\n"));
		DEBUG_REPORT_PRINT (true, ("                   enabled = %s\n", ms_enabled ? "yes" : "no"));
		DEBUG_REPORT_PRINT (true, ("       shadow detail level = %1.2f\n", ms_shadowDetailLevel));
		DEBUG_REPORT_PRINT (true, ("    simple shadow distance = %1.2f\n", ms_simpleShadowDistance));
		DEBUG_REPORT_PRINT (true, ("volumetric shadow distance = %1.2f\n", ms_volumetricShadowDistance));
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ShadowManagerNamespace;

// ======================================================================

void ShadowManager::install ()
{
	LocalMachineOptionManager::registerOption (ms_enabled, "ClientObject", "renderShadows");

	LocalMachineOptionManager::registerOption (ms_shadowDetailLevel, "ClientObject", "shadowDetailLevel");
	setShadowDetailLevel (ms_shadowDetailLevel);

	LocalMachineOptionManager::registerOption (ms_volumetricShadowDistanceLevel, "ClientObject", "volumetricShadowDistanceLevel");
	setVolumetricShadowDistanceLevel (ms_volumetricShadowDistanceLevel);

	LocalMachineOptionManager::registerOption (ms_meshShadows, "ClientObject", "meshShadows");
	LocalMachineOptionManager::registerOption (ms_skeletalShadows, "ClientObject", "skeletalShadows");

	updateShadows ();

	DebugFlags::registerFlag (ms_debugReport, "ClientObject", "reportShadowManager", debugReport);
	ExitChain::add (ShadowManagerNamespace::remove, "ShadowManagerNamespace::remove");
}

// ----------------------------------------------------------------------

bool ShadowManager::getEnabled ()
{
	return ms_enabled;
}

// ----------------------------------------------------------------------

void ShadowManager::setEnabled (bool const enabled)
{
	ms_enabled = enabled;
}

// ----------------------------------------------------------------------

bool ShadowManager::getEnabledDefault ()
{
	return true;
}

// ----------------------------------------------------------------------

bool ShadowManager::getAllowed()
{
	return ms_allowed;
}

// ----------------------------------------------------------------------

void ShadowManager::setAllowed(bool const allowed)
{
	ms_allowed = allowed;
}

// ----------------------------------------------------------------------

bool ShadowManager::getMeshShadowsNone ()
{
	return !ms_meshShadows;
}

// ----------------------------------------------------------------------

void ShadowManager::setMeshShadowsNone (bool const meshShadowsNone)
{
	if (meshShadowsNone)
		ms_meshShadows = false;

	updateShadows ();
}

// ----------------------------------------------------------------------

bool ShadowManager::getMeshShadowsNoneDefault () 
{ 
	return true; 
}

// ----------------------------------------------------------------------

bool ShadowManager::getMeshShadowsVolumetric ()
{
	return ms_meshShadows;
}

// ----------------------------------------------------------------------

void ShadowManager::setMeshShadowsVolumetric (bool const meshShadowsVolumetric)
{
	if (meshShadowsVolumetric)
		ms_meshShadows = true;

	updateShadows ();
}

// ----------------------------------------------------------------------

bool ShadowManager::getMeshShadowsVolumetricDefault () 
{ 
	return false; 
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsNone ()
{
	return ms_skeletalShadows == SST_none;
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsNoneDefault ()  
{ 
	return true; 
}

// ----------------------------------------------------------------------

void ShadowManager::setSkeletalShadowsNone (bool const skeletalShadowsNone)
{
	if (skeletalShadowsNone)
		ms_skeletalShadows = SST_none;

	updateShadows ();
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsSimple ()
{
	return ms_skeletalShadows == SST_simple;
}

// ----------------------------------------------------------------------

void ShadowManager::setSkeletalShadowsSimple (bool const skeletalShadowsSimple)
{
	if (skeletalShadowsSimple)
		ms_skeletalShadows = SST_simple;

	updateShadows ();
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsSimpleDefault ()  
{ 
	return true; 
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsVolumetric ()
{
	return ms_skeletalShadows == SST_volumetric;
}

// ----------------------------------------------------------------------

void ShadowManager::setSkeletalShadowsVolumetric (bool const skeletalShadowsVolumetric)
{
	if (skeletalShadowsVolumetric)
		ms_skeletalShadows = SST_volumetric;

	updateShadows ();
}

// ----------------------------------------------------------------------

bool ShadowManager::getSkeletalShadowsVolumetricDefault ()  
{ 
	return false; 
}

// ----------------------------------------------------------------------

void ShadowManager::setVolumetricShadowDistanceLevel (float const volumetricShadowDistanceLevel)
{
	ms_volumetricShadowDistanceLevel = clamp (0.f, volumetricShadowDistanceLevel, 1.f);
	ms_maximumVolumetricShadowDistance = linearInterpolate (cms_minimumVolumetricShadowDistance, cms_maximumVolumetricShadowDistance, ms_volumetricShadowDistanceLevel);

	setShadowDetailLevel (ms_shadowDetailLevel);
}

// ----------------------------------------------------------------------

float ShadowManager::getShadowDetailLevel ()
{
	return ms_shadowDetailLevel;
}

// ----------------------------------------------------------------------

void ShadowManager::setShadowDetailLevel (float const shadowDetailLevel)
{
	ms_shadowDetailLevel        = clamp (0.f, shadowDetailLevel, 1.f);
	ms_simpleShadowDistance     = linearInterpolate (cms_minimumSimpleShadowDistance, cms_maximumSimpleShadowDistance, ms_shadowDetailLevel);
	ms_volumetricShadowDistance = linearInterpolate (cms_minimumVolumetricShadowDistance, ms_maximumVolumetricShadowDistance, ms_shadowDetailLevel);
}

// ----------------------------------------------------------------------

float ShadowManager::getShadowDetailLevelDefault ()
{
	return 0.f;
}

// ----------------------------------------------------------------------

float ShadowManager::getSimpleShadowDistance ()
{
	return ms_simpleShadowDistance;
}

// ----------------------------------------------------------------------

bool ShadowManager::simpleShouldRender (Camera const & camera, Vector const & position_w, float const radius)
{
	return ms_enabled && shouldRender (camera, position_w, radius, ShadowManager::getSimpleShadowDistance ());
}

// ----------------------------------------------------------------------

float ShadowManager::getVolumetricShadowDistance ()
{
	return ms_volumetricShadowDistance;
}

// ----------------------------------------------------------------------

bool ShadowManager::volumetricShouldRender (Camera const & camera, Vector const & position_w, float const radius)
{
	return ms_enabled && shouldRender (camera, position_w, radius, ShadowManager::getVolumetricShadowDistance ());
}

// ----------------------------------------------------------------------

float ShadowManager::getTimeOfDay()
{
	return ms_timeOfDay;
}

// ----------------------------------------------------------------------

void ShadowManager::setTimeOfDay(float time)
{
	ms_timeOfDay = time;
}

// ======================================================================
