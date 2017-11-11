//===================================================================
//
// ClientTerrainSorter.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientTerrainSorter_H
#define INCLUDED_ClientTerrainSorter_H

//===================================================================

#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"

class Shader;

//===================================================================

/*
 * Terrain sorter is used exclusively with ClientProceduralTerrainAppearance
 * to sort geometry by FVF, then by shader
 */

class ClientTerrainSorter
{
public:

	static void                    install ();
	static void                    remove ();

	static void                    clear ();
	static void                    queue (const Shader* shader, const ClientProceduralTerrainAppearance::ShaderSet::Primitive* primitive);
	static void                    draw ();

private:

	static void debugReport ();

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<const ClientProceduralTerrainAppearance::ShaderSet::Primitive*>::fwd PrimitiveList;
	class PrimitiveNode;
	typedef stdvector<PrimitiveNode*>::fwd                 PrimitiveNodeList;
	typedef stdmap<const Shader*, PrimitiveNodeList*>::fwd ShaderMap;
	typedef stdvector<ShaderMap*>::fwd                     FvfList;
	typedef stdvector<PrimitiveList*>::fwd                 PrimitiveListCache;
	typedef stdvector<PrimitiveNodeList*>::fwd             PrimitiveNodeListCache;
	class FvfMetrics;
	typedef stdvector<FvfMetrics*>::fwd                    FvfMetricsList;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

#ifdef _DEBUG
	static bool                               ms_installed;
	static bool                               ms_debugReport;
#endif

	static FvfList*                           ms_fvfList;
	static FvfMetricsList*                    ms_fvfMetricsList;
	static PrimitiveListCache*                ms_primitiveListCache;
	static PrimitiveNodeListCache*            ms_primitiveNodeListCache;
	static ShaderPrimitiveSorter::LightBitSet ms_lightBitSet;

private:

	ClientTerrainSorter ();
	~ClientTerrainSorter ();
};

//===================================================================

#endif
