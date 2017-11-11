//
// ClientProceduralTerrainAppearance_Cache.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ClientProceduralTerrainAppearance_Cache_H
#define INCLUDED_ClientProceduralTerrainAppearance_Cache_H

//-------------------------------------------------------------------

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderSet.h"

//-------------------------------------------------------------------

class ClientProceduralTerrainAppearance::Cache
{
public:

	typedef stdvector<Array2d<PackedRgb>*>::fwd              ColorMapList;
	typedef stdvector<Array2d<ShaderGroup::Info>*>::fwd      ShaderGroupList;
	typedef stdvector<Array2d<FloraGroup::Info>*>::fwd       FloraGroupList;
	typedef stdvector<Array2d<RadialGroup::Info>*>::fwd      RadialGroupList;
	typedef stdvector<Array2d<EnvironmentGroup::Info>*>::fwd EnvironmentGroupList;
	typedef stdvector<Array2d<Vector>*>::fwd                 NormalMapList;

public:

	static void                             install ();
	static void                             remove ();

	static void                             lock ();
	static void                             unlock ();

	static Array2d<PackedRgb>*              createColorMap (int width, int height);
	static void                             destroyColorMap (Array2d<PackedRgb>* map);
	static Array2d<ShaderGroup::Info>*      createShaderMap (int width, int height);
	static void                             destroyShaderMap (Array2d<ShaderGroup::Info>* map);
	static Array2d<FloraGroup::Info>*       createFloraMap (int width, int height);
	static void                             destroyFloraMap (Array2d<FloraGroup::Info>* map);
	static Array2d<RadialGroup::Info>*      createRadialMap (int width, int height);
	static void                             destroyRadialMap (Array2d<RadialGroup::Info>* map);
	static Array2d<EnvironmentGroup::Info>* createEnvironmentMap (int width, int height);
	static void                             destroyEnvironmentMap (Array2d<EnvironmentGroup::Info>* map);
	static Array2d<Vector>*                 createNormalMap (int width, int height);
	static void                             destroyNormalMap (Array2d<Vector>* map);

	static void garbageCollect ();

private:

	static void debugDump ();

private:

	Cache ();
	~Cache ();
	Cache (const Cache&);
	Cache& operator= (const Cache&);

private:

	static bool                     ms_installed;
	static bool                     ms_locked;
#ifdef _DEBUG
	static bool                     ms_debugReport;
#endif
	static Mutex                    ms_criticalSection;

	static ColorMapList             ms_colorMapList;
	static ShaderGroupList          ms_shaderMapList;
	static FloraGroupList           ms_floraMapList;
	static RadialGroupList          ms_radialMapList;
	static EnvironmentGroupList     ms_environmentMapList;
	static NormalMapList            ms_normalMapList;
};

//-------------------------------------------------------------------

#endif
