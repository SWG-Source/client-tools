// ======================================================================
//
// ClientProceduralTerrainAppearance_ClientChunk.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_ClientChunk_H
#define INCLUDED_ClientProceduralTerrainAppearance_ClientChunk_H

// ======================================================================

#include "clientTerrain/ClientProceduralTerrainAppearance.h"

#include <vector>

class MemoryBlockManager;
namespace DPVS
{
	class Object;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Chunk defines a chunk of terrain in the terrain system
//
class ClientProceduralTerrainAppearance::ClientChunk : public ProceduralTerrainAppearance::Chunk
{
private:
	
	static MemoryBlockManager* ms_memoryBlockManager;
	static const Shader*                    ms_cloudShader;

public:
	
	static void  install (void);
	static void  remove (void);
	
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

	static void  setTerrainCloudShader (const Shader* cloudShader);

private:
	
	//-- list of all fans organized by shader
	typedef std::vector<ShaderSet*> ShaderSetList;
	ShaderSetList              m_shaderSetList;
	
	//-- 
	const ShaderCache*         shaderCache;
	
	//--
	Array2d<PackedRgb>*        colorMap;
	
	//--
	Array2d<FloraGroup::Info>* floraStaticNonCollidableMap;
	
	//--
	Array2d<RadialGroup::Info>* floraDynamicNearMap;
	Array2d<RadialGroup::Info>* floraDynamicFarMap;

	//-- 
	Array2d<EnvironmentGroup::Info>* environmentMap;

	//--
	Array2d<Vector>*           vertexNormalMap;

	//-- used for occlusion
	IndexedTriangleList*       m_writeIndexedTriangleList;

	bool                       m_inWorld;
	DPVS::Object*              m_dpvsObject;
	mutable const Shader*      m_lotShader;

private:
	
	void createTileShader (ShaderData& shaderData, int x, int z, bool useFirstChild) const;

	bool findDynamicFlora(const Array2d<RadialGroup::Info>& radialMap, float positionX, float positionZ, DynamicFloraData& data, bool& floraAllowed, bool constrainToHighDetail) const;

private:

	ClientChunk ();
	ClientChunk (const ClientChunk& rhs);
	ClientChunk& operator= (const ClientChunk& rhs);
	
public:
	
	explicit ClientChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance);
	virtual ~ClientChunk (void);
	
	const PackedRgb&      getColorAt  (int x, int z) const;
	bool                  getHeightAt (const Vector& pos, float * height) const;
	const Vector&         getNormalAt (int x, int z) const;
	bool                  getHeightAt (const Vector& pos, float * height, Vector * normal) const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;

	virtual void          render () const;
	
	void                  create (const ClientCreateChunkData& newCreateChunkData);
	
	bool                  findStaticNonCollidableFlora (float positionX, float positionZ, ClientProceduralTerrainAppearance::StaticFloraData& data, bool& floraAllowed) const;
	bool                  findDynamicNearFlora         (float positionX, float positionZ, ClientProceduralTerrainAppearance::DynamicFloraData& data, bool& floraAllowed) const;
	bool                  findDynamicFarFlora          (float positionX, float positionZ, ClientProceduralTerrainAppearance::DynamicFloraData& data, bool& floraAllowed) const;
	bool                  findEnvironment (const Vector& position, ClientProceduralTerrainAppearance::EnvironmentData& data) const;
	
	void                  resetIndices(unsigned newHasLargerNeighborFlags);

	void                  addObjectToWorld ();
	void                  removeObjectFromWorld ();
	bool                  isObjectInWorld();

	void                  setLotShader (const Shader* lotShader);

	void getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const;
	virtual void renderShadowBlob (const Vector& position_o, float radius) const;
	virtual void renderReticle (const Vector& position_o, float radius) const;
};

//-----------------------------------------------------------------

inline const PackedRgb& ClientProceduralTerrainAppearance::ClientChunk::getColorAt  (int x, int z) const
{
	return colorMap->getData (x, z);
}

//----------------------------------------------------------------------

inline const Vector& ClientProceduralTerrainAppearance::ClientChunk::getNormalAt  (int x, int z) const
{
	return vertexNormalMap->getData (x, z);
}

// ======================================================================

#endif
