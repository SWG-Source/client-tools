//===================================================================
//
// ClientServerProceduralTerrainAppearance.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientServerProceduralTerrainAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainQuadTree.h"

#include "dpvsObject.hpp"

#include <map>
#include <stack>

//===================================================================
//
// ClientServerChunk - defines a chunk of terrain in this appearance
//
class ClientServerChunk : public ServerProceduralTerrainAppearance::ServerChunk
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();
	static void remove ();
	
public:

	class LocalShaderPrimitive;

public:

	explicit ClientServerChunk (ProceduralTerrainAppearance& appearance);
	virtual ~ClientServerChunk (void);

	virtual void render () const;

private:

	ClientServerChunk ();
	ClientServerChunk (const ClientServerChunk& rhs);             //lint -esym(754, ClientServerChunk::ClientServerChunk)
	ClientServerChunk& operator= (const ClientServerChunk& rhs);  //lint -esym(754, ClientServerChunk::operator=)

private:

	mutable LocalShaderPrimitive* m_localShaderPrimitive;
};

//===================================================================
//
// ClientServerChunk::LocalShaderPrimitive
//
class ClientServerChunk::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();
	static void remove ();

public:

	LocalShaderPrimitive (const Appearance& appearance);
	virtual ~LocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	void setIndexedTriangleList (const ArrayList<Vector>* vertexList, const ArrayList<int>* indexList, int m_excluded);

private:

	bool isExcluded (int index) const;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive& rhs);             //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive)
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive& rhs);  //lint -esym(754, LocalShaderPrimitive::operator=)

private:

	const Appearance&        m_appearance;
	const ArrayList<Vector>* m_vertexList;
	const ArrayList<int>*    m_indexList;
	int                      m_excluded;
};

//===================================================================

void ClientServerChunk::install ()
{
	installMemoryBlockManager ();
}

//-------------------------------------------------------------------

void ClientServerChunk::remove ()
{
	removeMemoryBlockManager ();
}

//===================================================================

ClientServerChunk::ClientServerChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance) :
	ServerChunk (proceduralTerrainAppearance),
	m_localShaderPrimitive (0)
{
	m_localShaderPrimitive = new LocalShaderPrimitive (proceduralTerrainAppearance);
}

//-------------------------------------------------------------------

ClientServerChunk::~ClientServerChunk (void)
{
	delete m_localShaderPrimitive;
	m_localShaderPrimitive = 0;
}

//-------------------------------------------------------------------

void ClientServerChunk::render () const
{
	m_localShaderPrimitive->setIndexedTriangleList (m_vertexList, ms_indexList, m_excluded);

	//-- umbra is not used here, so test sphere manually
	if (ShaderPrimitiveSorter::getCurrentCamera().testVisibility_w (getBoxExtent ().getSphere ()))
	{
		NOT_NULL (m_localShaderPrimitive);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive);
	}

	Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
	Graphics::drawExtent(&m_boxExtent, VectorArgb::solidRed);
}

//===================================================================

void ClientServerChunk::LocalShaderPrimitive::install ()
{
	installMemoryBlockManager ();
}

//-------------------------------------------------------------------

void ClientServerChunk::LocalShaderPrimitive::remove ()
{
	removeMemoryBlockManager ();
}

//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientServerChunk::LocalShaderPrimitive, true, 0, 0, 0);

//-------------------------------------------------------------------

ClientServerChunk::LocalShaderPrimitive::LocalShaderPrimitive (const Appearance& appearance) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_vertexList (0),  
	m_indexList (0)
{
}

//-------------------------------------------------------------------

ClientServerChunk::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

//-------------------------------------------------------------------

const Vector ClientServerChunk::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ClientServerChunk::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int ClientServerChunk::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ClientServerChunk::LocalShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ().prepareToView ();
}

//-------------------------------------------------------------------

void ClientServerChunk::LocalShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);

	//-- create m_vertexBuffer
	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();

		DynamicVertexBuffer vertexBuffer (format);
		vertexBuffer.lock (m_vertexList->getNumberOfElements ());

		VertexBufferWriteIterator v = vertexBuffer.begin ();

			int i;
			for (i = 0; i < m_vertexList->getNumberOfElements (); ++i, ++v)
			{
				v.setPosition ((*m_vertexList) [i]);
				v.setColor0 (PackedArgb::solidGray);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}

	//-- create m_indexBuffer
	{
		DynamicIndexBuffer indexBuffer;
		indexBuffer.lock (m_indexList->getNumberOfElements ());

			Index *ii = indexBuffer.begin();
			int i;
			for (i = 0; i < m_indexList->getNumberOfElements (); ++i)
				ii [i] = static_cast<Index> (isExcluded (i / 24) ? 0 : (*m_indexList) [i]);

		indexBuffer.unlock ();

		Graphics::setIndexBuffer (indexBuffer);
	}
}

//-------------------------------------------------------------------

void ClientServerChunk::LocalShaderPrimitive::draw () const
{
	Graphics::drawIndexedTriangleList ();
}

//-------------------------------------------------------------------

void ClientServerChunk::LocalShaderPrimitive::setIndexedTriangleList (const ArrayList<Vector>* vertexList, const ArrayList<int>* indexList, const int excluded)
{
	m_vertexList = vertexList;
	m_indexList = indexList;
	m_excluded = excluded;
}

//-------------------------------------------------------------------

bool ClientServerChunk::LocalShaderPrimitive::isExcluded (const int index) const
{
	return (m_excluded & (1 << index)) != 0;
}

//===================================================================
//
// ClientServerProceduralTerrainAppearance
//
MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientServerChunk, true, 0, 0, 0);

//-------------------------------------------------------------------

void ClientServerProceduralTerrainAppearance::install ()
{
	ClientServerChunk::install ();
	ClientServerChunk::LocalShaderPrimitive::install ();

	ExitChain::add (remove, "ClientServerProceduralTerrainAppearance::remove");
}

//-------------------------------------------------------------------

void ClientServerProceduralTerrainAppearance::remove ()
{
	ClientServerChunk::LocalShaderPrimitive::remove ();
	ClientServerChunk::remove ();
}

//-------------------------------------------------------------------

ClientServerProceduralTerrainAppearance::ClientServerProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate) :
	ServerProceduralTerrainAppearance (appearanceTemplate),
	m_ambientLight (0),
	m_parallelLight (0),
	m_dpvsObject (0)

#if _DEBUG
	, m_noRenderTerrain (false)
#endif
{
	//-- create lights
	m_ambientLight = new Light (Light::T_ambient, VectorArgb::solidGray);
	m_ambientLight->setDebugName ("ambient light");
	m_ambientLight->addToWorld ();

	m_parallelLight = new Light (Light::T_parallel, VectorArgb::solidWhite);
	m_parallelLight->yaw_o (PI_OVER_4);
	m_parallelLight->pitch_o (PI_OVER_4);
	m_parallelLight->setDebugName ("parallel light");
	m_parallelLight->addToWorld ();

	m_dpvsObject = RenderWorld::createUnboundedObject (this);

#if _DEBUG
	DebugFlags::registerFlag (m_noRenderTerrain, "ClientTerrain", "noRenderTerrain");
#endif
}

//-------------------------------------------------------------------

ClientServerProceduralTerrainAppearance::~ClientServerProceduralTerrainAppearance (void)
{
	m_ambientLight->removeFromWorld ();
	delete m_ambientLight;
	m_ambientLight = 0;

	m_parallelLight->removeFromWorld ();
	delete m_parallelLight;
	m_parallelLight = 0;

	m_dpvsObject->release ();
	m_dpvsObject = 0;

#if _DEBUG
	DebugFlags::unregisterFlag (m_noRenderTerrain);
#endif
}

//-------------------------------------------------------------------

DPVS::Object* ClientServerProceduralTerrainAppearance::getDpvsObject () const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void ClientServerProceduralTerrainAppearance::createChunk (const int x, const int z, const int chunkSize, unsigned /*hasLargerNeighborFlags*/)
{
	//-- make sure indices are valid
	if (!areValidChunkIndices (x, z))
		return;

	//-- see if the chunk already exists
	if (findChunk (x, z, chunkSize) != 0)
		return;

	++m_chunksGeneratedThisFrame;

	const TerrainGenerator* terrainGenerator      = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();
	const int               numberOfTilesPerChunk = proceduralTerrainAppearanceTemplate->getNumberOfTilesPerChunk ();
	const float             chunkWidthInMeters    = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();
	const float             tileWidthInMeters     = proceduralTerrainAppearanceTemplate->getTileWidthInMeters ();

	//-- chunk does not exist -- it needs to be created. find out what map data i'll need to ask the generator for
	const float  distanceBetweenPoles =  tileWidthInMeters * 0.5f;

	const Vector start (
		static_cast<float> (x) * chunkWidthInMeters - static_cast<float>(originOffset) * distanceBetweenPoles, 
		0.0f,
		static_cast<float> (z) * chunkWidthInMeters - static_cast<float>(originOffset) * distanceBetweenPoles);

	ClientServerChunk* chunk = new ClientServerChunk (*this);

	//-- setup data needed to create a chunk
	ProceduralTerrainAppearance::CreateChunkData createChunkData (&createChunkBuffer);

	createChunkData.chunkX                     = x;
	createChunkData.chunkZ                     = z;
	createChunkData.start                      = start;
	createChunkData.numberOfTilesPerChunk      = numberOfTilesPerChunk;
	createChunkData.chunkWidthInMeters         = chunkWidthInMeters;
	createChunkData.tileWidthInMeters          = tileWidthInMeters;
	createChunkData.shaderGroup                = &terrainGenerator->getShaderGroup ();
	createChunkData.floraGroup                 = &terrainGenerator->getFloraGroup ();
	createChunkData.radialGroup                = &terrainGenerator->getRadialGroup ();
	createChunkData.environmentGroup           = &terrainGenerator->getEnvironmentGroup ();
	createChunkData.fractalGroup               = &terrainGenerator->getFractalGroup ();
	createChunkData.bitmapGroup                = &terrainGenerator->getBitmapGroup (); 
	createChunkData.originOffset               = originOffset;
	createChunkData.numberOfPoles              = numberOfPoles;

	//-- ask the generator to fill out this area
	TerrainGenerator::GeneratorChunkData generatorChunkData(proceduralTerrainAppearanceTemplate->getLegacyMode());

	generatorChunkData.heightMap            = &createChunkData.createChunkBuffer->heightMap;
	generatorChunkData.colorMap             = &createChunkData.createChunkBuffer->colorMap;
	generatorChunkData.shaderMap            = &createChunkData.createChunkBuffer->shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &createChunkData.createChunkBuffer->floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &createChunkData.createChunkBuffer->floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap  = &createChunkData.createChunkBuffer->floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap   = &createChunkData.createChunkBuffer->floraDynamicFarMap;
	generatorChunkData.environmentMap       = &createChunkData.createChunkBuffer->environmentMap;
	generatorChunkData.vertexPositionMap    = &createChunkData.createChunkBuffer->vertexPositionMap;
	generatorChunkData.vertexNormalMap      = &createChunkData.createChunkBuffer->vertexNormalMap;
	generatorChunkData.excludeMap           = &createChunkData.createChunkBuffer->excludeMap;
	generatorChunkData.start                = start;
	generatorChunkData.originOffset         = originOffset;
	generatorChunkData.numberOfPoles        = numberOfPoles;
	generatorChunkData.upperPad             = upperPad;
	generatorChunkData.distanceBetweenPoles = distanceBetweenPoles;
	generatorChunkData.shaderGroup          = &terrainGenerator->getShaderGroup ();
	generatorChunkData.floraGroup           = &terrainGenerator->getFloraGroup ();
	generatorChunkData.radialGroup          = &terrainGenerator->getRadialGroup ();
	generatorChunkData.environmentGroup     = &terrainGenerator->getEnvironmentGroup ();
	generatorChunkData.fractalGroup         = &terrainGenerator->getFractalGroup ();
	generatorChunkData.bitmapGroup          = &terrainGenerator->getBitmapGroup (); 

	terrainGenerator->generateChunk (generatorChunkData);

	//-- create the chunk using the data the generator created
	chunk->create (createChunkData);
	createFlora (chunk);

	addChunk (chunk, chunkSize);
}  //lint !e429  //chunk not freed or returned

//-------------------------------------------------------------------

void ClientServerProceduralTerrainAppearance::render () const
{
#if _DEBUG
	if (m_noRenderTerrain)
		return;
#endif

	const Camera *camera = &ShaderPrimitiveSorter::getCurrentCamera();

	ChunkMap::iterator iter = m_chunkMap->begin ();
	ChunkMap::iterator end = m_chunkMap->end ();
	for (; iter != end; ++iter)
	{
		const Chunk* const chunk = iter->second;

		if (chunk && camera->testVisibility_w (chunk->getSphere ()))
			chunk->render ();
	}
}

//===================================================================
