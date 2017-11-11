// ======================================================================
//
// ShadowManager.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShadowManager_H
#define INCLUDED_ShadowManager_H

// ======================================================================

class Camera;
class Object;
class Vector;

// ======================================================================

class ShadowManager
{
public:

	static void  install ();

	static bool  getEnabled ();
	static void  setEnabled (bool enabled);
	static bool  getEnabledDefault ();

	static bool getAllowed();
	static void setAllowed(bool allowed);

	static bool  getMeshShadowsNone ();
	static void  setMeshShadowsNone (bool meshShadowsNone);
	static bool  getMeshShadowsNoneDefault ();

	static bool  getMeshShadowsVolumetric ();
	static void  setMeshShadowsVolumetric (bool meshShadowsVolumetric);
	static bool  getMeshShadowsVolumetricDefault ();

	static bool  getSkeletalShadowsNone ();
	static void  setSkeletalShadowsNone (bool skeletalShadowsNone);
	static bool  getSkeletalShadowsNoneDefault ();

	static bool  getSkeletalShadowsSimple ();
	static void  setSkeletalShadowsSimple (bool skeletalShadowsSimple);
	static bool  getSkeletalShadowsSimpleDefault ();

	static bool  getSkeletalShadowsVolumetric ();
	static void  setSkeletalShadowsVolumetric (bool skeletalShadowsVolumetric);
	static bool  getSkeletalShadowsVolumetricDefault ();

	static void  setVolumetricShadowDistanceLevel (float volumetricShadowDistanceLevel);

	static float getShadowDetailLevel ();
	static void  setShadowDetailLevel (float shadowDetailLevel);
	static float getShadowDetailLevelDefault ();

	static float getSimpleShadowDistance ();
	static bool  simpleShouldRender (Camera const & camera, Vector const & position_w, float const radius);

	static float getVolumetricShadowDistance ();
	static bool  volumetricShouldRender (Camera const & camera, Vector const & position_w, float const radius);

	static float getTimeOfDay();
	static void  setTimeOfDay(float time);

private:

	ShadowManager ();
	ShadowManager (const ShadowManager&);
	ShadowManager& operator= (const ShadowManager&);
};

// ======================================================================

#endif
