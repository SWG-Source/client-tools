// ======================================================================
//
// ClientProceduralTerrainAppearance_ClientChunk.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ClientChunk.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientObject/ShadowBlobManager.h"
#include "clientObject/ReticleManager.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Cache.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderCache.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderData.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/FastKeyList.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

#include <algorithm>
#include <map>

//===================================================================

namespace
{
	static const int VERTEX_INDIRECTION_ARRAY_SIZE = 9;

	struct VertexIndirectionData
	{
		int   x;
		int   z;
		float u;
		float v;
	};

	static const VertexIndirectionData constructionIndirectionArray [VERTEX_INDIRECTION_ARRAY_SIZE] =
	{
		{ 1,  1,  0.5f,  0.5f },
		{ 0,  1,  0.f,   0.5f },
		{ 0,  2,  0.f,   0.f  },
		{ 1,  2,  0.5f,  0.f  },
		{ 2,  2,  1.f,   0.f  },
		{ 2,  1,  1.f,   0.5f },
		{ 2,  0,  1.f,   1.f  },
		{ 1,  0,  0.5f,  1.f  },
		{ 0,  0,  0.f,   1.f  }
	};

	struct TriangleIndirectionData 
	{
		int offsetX0;
		int offsetZ0;
		int offsetX1;
		int offsetZ1;
		int offsetX2;
		int offsetZ2;
	};

	static const int TRIANGLE_INDIRECTION_ARRAY_SIZE = 8;

	static const TriangleIndirectionData constructionTriangleIndirectionArray [TRIANGLE_INDIRECTION_ARRAY_SIZE] =
	{
		// x1, z1, x2, z2, x3, z3
		{   1,  1,  0,  2,  1,  2 },
		{   1,  1,  1,  2,  2,  2 },
		{   1,  1,  2,  2,  2,  1 },
		{   1,  1,  2,  1,  2,  0 },
		{   1,  1,  2,  0,  1,  0 },
		{   1,  1,  1,  0,  0,  0 },
		{   1,  1,  0,  0,  0,  1 },
		{   1,  1,  0,  1,  0,  2 }
	};
}

//-------------------------------------------------------------------
//
// ClientProceduralTerrainAppearance::ClientChunk
//

MemoryBlockManager* ClientProceduralTerrainAppearance::ClientChunk::ms_memoryBlockManager;
const Shader*                    ClientProceduralTerrainAppearance::ClientChunk::ms_cloudShader;

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientProceduralTerrainAppearance::ClientChunk::install\n"));
	DEBUG_FATAL (ms_memoryBlockManager, ("ClientProceduralTerrainAppearance::ClientChunk already installed"));
	ms_memoryBlockManager = new MemoryBlockManager ("ClientProceduralTerrainAppearance::ClientChunk::ms_memoryBlockManager", false, sizeof(ClientChunk), 1000, 2, 0);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("ClientProceduralTerrainAppearance::ClientChunk is not installed"));
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* ClientProceduralTerrainAppearance::ClientChunk::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (ClientChunk), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();	
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::setTerrainCloudShader (const Shader* cloudShader)
{
	ms_cloudShader = cloudShader;
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ClientChunk::ClientChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance) :
	Chunk (proceduralTerrainAppearance),
	shaderCache (0),
	colorMap (0),
	floraStaticNonCollidableMap (0),
	floraDynamicNearMap (0),
	floraDynamicFarMap (0),
	environmentMap (0),
	vertexNormalMap (0),
	m_writeIndexedTriangleList (0),
	m_inWorld(false),
	m_dpvsObject (0),
	m_lotShader (0)
{
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ClientChunk::~ClientChunk ()
{
	{
		ShaderSetList::iterator ssi;
		for (ssi=m_shaderSetList.begin();ssi!=m_shaderSetList.end();++ssi)
		{
			delete *ssi;
		}
		m_shaderSetList.clear();
	}

	removeObjectFromWorld ();
	if (m_dpvsObject)
	{
		m_dpvsObject->release();
		m_dpvsObject = NULL;
	}

	setOwner(0);

	ClientProceduralTerrainAppearance::Cache::lock ();
		
		ClientProceduralTerrainAppearance::Cache::destroyColorMap (colorMap);
		colorMap = 0;

		ClientProceduralTerrainAppearance::Cache::destroyShaderMap (shaderMap);
		shaderMap = 0;

		if (m_floraStaticCollidableMap)
		{
			ClientProceduralTerrainAppearance::Cache::destroyFloraMap (m_floraStaticCollidableMap);
			m_floraStaticCollidableMap = 0;
		}

		ClientProceduralTerrainAppearance::Cache::destroyFloraMap (floraStaticNonCollidableMap);
		floraStaticNonCollidableMap = 0;

		ClientProceduralTerrainAppearance::Cache::destroyRadialMap (floraDynamicNearMap);
		floraDynamicNearMap = 0;

		ClientProceduralTerrainAppearance::Cache::destroyRadialMap (floraDynamicFarMap);
		floraDynamicFarMap = 0;

		ClientProceduralTerrainAppearance::Cache::destroyEnvironmentMap (environmentMap);
		environmentMap = 0;

		ClientProceduralTerrainAppearance::Cache::destroyNormalMap (vertexNormalMap);
		vertexNormalMap = 0;

	ClientProceduralTerrainAppearance::Cache::unlock ();

	if (m_writeIndexedTriangleList)
	{
		delete m_writeIndexedTriangleList;
		m_writeIndexedTriangleList = 0;
	}

	shaderCache = 0;

	m_lotShader = 0;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::collide (const Vector& start, const Vector& end, CollideParameters const & /*collideParameters*/, CollisionInfo& info) const
{
	bool found = false;

	//-- test the line against the extent
	if (getExtent ()->testSphereOnly (start, end))
	{
		//-- now test the fan sets
		const uint n = m_shaderSetList.size();
		uint i;
		for (i = 0; i < n; ++i)
			if (m_shaderSetList[i]->collide (start, end, info))
				found = true;
	}

	return found;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::getHeightAt (const Vector& pos, float * height) const
{
	const Vector vmin = m_boxExtent.getMin ();
	const Vector vmax = m_boxExtent.getMax ();
	const Vector start (pos.x, vmax.y + 0.1f, pos.z);
	const Vector end   (pos.x, vmin.y - 0.1f, pos.z);

	CollisionInfo info;
	info.setPoint(end);

	//-- test the line against the extent
	if (m_boxExtent.testSphereOnly (start, end))
	{
		//-- now test the fan sets, since we're testing height, exit on the first one we find
		const uint n = m_shaderSetList.size();
		uint i;
		for (i = 0; i < n; ++i)
			if (m_shaderSetList[i]->getHeightAt (start, end, info))
			{
				if (height)
					*height = info.getPoint().y;

				return true;
			}
	}

	return false;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::getHeightAt (const Vector& pos, float* height, Vector* normal) const
{
	const Vector vmin = m_boxExtent.getMin ();
	const Vector vmax = m_boxExtent.getMax ();
	const Vector start (pos.x, vmax.y + 0.1f, pos.z);
	const Vector end   (pos.x, vmin.y - 0.1f, pos.z);

	CollisionInfo info;
	info.setPoint(end);

	//-- test the line against the extent
	if (m_boxExtent.testSphereOnly (start, end))
	{
		//-- now test the fan sets, since we're testing height, exit on the first one we find
		const uint n = m_shaderSetList.size();
		uint i;
		for (i = 0; i < n; ++i)
			if (m_shaderSetList[i]->getHeightAt (start, end, info))
			{
				if (height)
					*height = info.getPoint().y;

				if (normal)
					*normal = info.getNormal();

				return true;
			}
	}

	return false;
}

//-------------------------------------------------------------------

struct PrioritizedFamily
{
	int key;
	int familyId;
};

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::createTileShader (ShaderData& shaderData, int x, int z, bool useFirstChild) const
{
	NOT_NULL (shaderCache);

	Zero (shaderData);

	//-- 1,1 is center
	ShaderGroup::Info sgi [3][3];

	{
		sgi [0][0] = shaderMap->getData (x - 2, z + 2);
		sgi [1][0] = shaderMap->getData (x    , z + 2);
		sgi [2][0] = shaderMap->getData (x + 2, z + 2);
		sgi [0][1] = shaderMap->getData (x - 2, z);
		sgi [1][1] = shaderMap->getData (x,     z);
		sgi [2][1] = shaderMap->getData (x + 2, z);
		sgi [0][2] = shaderMap->getData (x - 2, z - 2);
		sgi [1][2] = shaderMap->getData (x,     z - 2);
		sgi [2][2] = shaderMap->getData (x + 2, z - 2);
	}

	//-- find out how many unique families there are other than me
	FastKeyList<PrioritizedFamily, int, 3> familyList;

	if (!ConfigClientTerrain::getDisableTerrainBlending ())
	{
		int i;
		int j;
		for (i = 0; i < 3; ++i)
			for (j = 0; j < 3; ++j)
			{
				if (useFirstChild)
					sgi [i][j].setChildChoice (0.f);

				if (sgi [i][j].getPriority () > sgi [1][1].getPriority ())
				{
					PrioritizedFamily family;
					family.key      = sgi [i][j].getPriority ();
					family.familyId = sgi [i][j].getFamilyId ();
					familyList.insertIfNotExists (family);
				}
			}
	}

	//-- set up the base
	{
		shaderCache->getTextures (sgi [1][1], shaderData.inputShaders[0], shaderData.textures[0], shaderData.normals[0]);
		shaderData.numberOfTextures = 1 + familyList.getNumberOfElements ();
	}

	//-- find out how we blend. in the following examples, A is higher priority than B
	if (shaderData.numberOfTextures > 0)
	{
		const int upperLeft   = sgi [0][0].getFamilyId ();
		const int upper       = sgi [1][0].getFamilyId ();
		const int upperRight  = sgi [2][0].getFamilyId ();
		const int left        = sgi [0][1].getFamilyId ();
		const int right       = sgi [2][1].getFamilyId ();
		const int lowerLeft   = sgi [0][2].getFamilyId ();
		const int lower       = sgi [1][2].getFamilyId ();
		const int lowerRight  = sgi [2][2].getFamilyId ();

		//-- add unique shaders with a higher priority than me
		for (int i = 1; i < shaderData.numberOfTextures; ++i)
		{
			int familyIndex = i - 1;
 			const int familyId = familyList [familyIndex].familyId;

			ShaderGroup::Info next_sgi = sgi [1][1];

			const Shader* blendShader = 0;
			RotationType rotationType = RT_0;

			const bool    doUseFirstChild = useFirstChild || (familyIndex != familyList.getNumberOfElements() - 1);

			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			//-- check three quarter
			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

			// - A -
			// A B -
			// - - -
			if (left == familyId && upper == familyId)
			{
				//-- upper left
				next_sgi              = sgi [1][0];
				blendShader           = shaderCache->getBlendingThreeQuarterShader ();
				rotationType = RT_0;
			}
			else

			// - A -
			// - B A
			// - - -
			if (right == familyId && upper == familyId)
			{
				//-- upper right
				next_sgi              = sgi [1][0];
				blendShader           = shaderCache->getBlendingThreeQuarterShader ();
				rotationType = RT_270;
			}
			else

			// - - -
			// - B A
			// - A -
			if (right == familyId && lower == familyId)
			{
				//-- lower right
				next_sgi              = sgi [1][2];
				blendShader           = shaderCache->getBlendingThreeQuarterShader ();
				rotationType = RT_180;
			}
			else

			// - - -
			// A B -
			// - A -
			if (left == familyId && lower == familyId)
			{
				//-- lower left
				next_sgi              = sgi [1][2];
				blendShader           = shaderCache->getBlendingThreeQuarterShader ();
				rotationType = RT_90;
			}
			else

			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			//-- check one half
			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

			// - - -
			// A B -
			// - - -
			if (left == familyId)
			{
				//-- straight from left
				next_sgi              = sgi [0][1];
				blendShader           = shaderCache->getBlendingOneHalfShader ();
				rotationType = RT_0;
			}
			else

			// - A -
			// - B -
			// - - -
			if (upper == familyId)
			{
				//-- straight from top
				next_sgi              = sgi [1][0];
				blendShader           = shaderCache->getBlendingOneHalfShader ();
				rotationType = RT_270;
			}
			else

			// - - -
			// - B A
			// - - -
			if (right == familyId)
			{
				//-- straight from right
				next_sgi              = sgi [2][1];
				blendShader           = shaderCache->getBlendingOneHalfShader ();
				rotationType = RT_180;
			}
			else

			// - - -
			// - B -
			// - A -
			if (lower == familyId)
			{
				//-- straight from bottom
				next_sgi              = sgi [1][2];
				blendShader           = shaderCache->getBlendingOneHalfShader ();
				rotationType = RT_90;
			}
			else

			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			//-- check one quarter
			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

			// A - - 
			// - B -
			// - - -
			if (upperLeft == familyId)
			{
				//-- upper left
				next_sgi              = sgi [0][0];
				blendShader           = shaderCache->getBlendingOneQuarterShader ();
				rotationType = RT_0;
			}
			else

			// - - A
			// - B -
			// - - -
			if (upperRight == familyId)
			{
				//-- upper right
				next_sgi              = sgi [2][0];
				blendShader           = shaderCache->getBlendingOneQuarterShader ();
				rotationType = RT_270;
			}
			else

			// - - -
			// - B -
			// - - A
			if (lowerRight == familyId)
			{
				//-- lower right
				next_sgi              = sgi [2][2];
				blendShader           = shaderCache->getBlendingOneQuarterShader ();
				rotationType = RT_180;
			}
			else

			// - - -
			// - B -
			// A - -
			if (lowerLeft == familyId)
			{
				//-- lower left
				next_sgi              = sgi [0][2];
				blendShader           = shaderCache->getBlendingOneQuarterShader ();
				rotationType = RT_90;
			}

			DEBUG_FATAL (next_sgi.getPriority () == sgi [1][1].getPriority (), ("blend shader not selected"));

			//-- if we're not the top shader, always choose the first one in the list (no one will know the difference)
			if (doUseFirstChild)
				next_sgi.setChildChoice (0.f);

			//-- add to list

			NOT_NULL (blendShader);
			bool result = safe_cast<const StaticShader*> (blendShader)->getTexture (TAG (M,A,I,N), shaderData.alphas[i]);
			UNREF(result);
			DEBUG_FATAL (!result, ("couldn't get texture"));

			shaderCache->getTextures (next_sgi, shaderData.inputShaders[i], shaderData.textures[i], shaderData.normals[i]);

			shaderData.m_rotationTypes[i] = rotationType;
		}
	}

	shaderData.outputShader = shaderCache->createBlendedShader (shaderData);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::resetIndices(unsigned newHasLargerNeighborFlags)
{
	hasLargerNeighborFlags = static_cast<uint8>(newHasLargerNeighborFlags);

	int const numberOfTilesPerChunk = m_proceduralTerrainAppearance.getNumberOfTilesPerChunk();

	const uint n = m_shaderSetList.size();
	uint i;
	for (i = 0; i < n; ++i)
		m_shaderSetList[i]->chooseIndexBuffer(newHasLargerNeighborFlags, numberOfTilesPerChunk);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::create (const ClientCreateChunkData& createChunkData)
{
	createChunkData.validate ();

	chunkX                = createChunkData.chunkX;
	chunkZ                = createChunkData.chunkZ;
	chunkWidthInMeters    = createChunkData.chunkWidthInMeters;
	originOffset          = createChunkData.originOffset;
	numberOfPoles         = createChunkData.numberOfPoles;
	hasLargerNeighborFlags= createChunkData.hasLargerNeighborFlags;
	shaderCache           = createChunkData.shaderCache;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- cache variables from createChunkData 
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	int const numberOfTilesPerChunk = createChunkData.numberOfTilesPerChunk;
	const Array2d<PackedRgb>* const         ccd_colorMap   = &createChunkData.createChunkBuffer->colorMap;
	const Array2d<ShaderGroup::Info>* const ccd_shaderMap  = &createChunkData.createChunkBuffer->shaderMap;
	//const Array2d<FloraGroup::Info>* const  ccd_floraStaticCollidableMap    = &createChunkData.createChunkBuffer->floraStaticCollidableMap;
	const Array2d<FloraGroup::Info>* const  ccd_floraStaticNonCollidableMap = &createChunkData.createChunkBuffer->floraStaticNonCollidableMap;
	const Array2d<RadialGroup::Info>* const ccd_floraDynamicNearMap         = &createChunkData.createChunkBuffer->floraDynamicNearMap;
	const Array2d<RadialGroup::Info>* const ccd_floraDynamicFarMap          = &createChunkData.createChunkBuffer->floraDynamicFarMap;
	const Array2d<EnvironmentGroup::Info>* const ccd_environmentMap         = &createChunkData.createChunkBuffer->environmentMap;
	const Array2d<bool>* const              ccd_excludeMap = &createChunkData.createChunkBuffer->excludeMap;
	const Array2d<bool>* const              ccd_passableMap = &createChunkData.createChunkBuffer->passableMap;

	const Array2d<Vector>* const vertexPositionMap   = &createChunkData.createChunkBuffer->vertexPositionMap;
	const Array2d<Vector>* const ccd_vertexNormalMap = &createChunkData.createChunkBuffer->vertexNormalMap;

	const bool isLeaf = createChunkData.isLeaf;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- create and copy appropriate maps
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	ClientProceduralTerrainAppearance::Cache::lock ();
		vertexNormalMap             = ClientProceduralTerrainAppearance::Cache::createNormalMap (ccd_vertexNormalMap->getWidth (), ccd_vertexNormalMap->getHeight ());
		shaderMap                   = ClientProceduralTerrainAppearance::Cache::createShaderMap (ccd_shaderMap->getWidth (), ccd_shaderMap->getHeight ());
		//floraStaticCollidableMap    = ClientProceduralTerrainAppearance::Cache::createFloraMap (ccd_floraStaticCollidableMap->getWidth (), ccd_floraStaticCollidableMap->getHeight ());
		floraStaticNonCollidableMap = ClientProceduralTerrainAppearance::Cache::createFloraMap (ccd_floraStaticNonCollidableMap->getWidth (), ccd_floraStaticNonCollidableMap->getHeight ());
		floraDynamicNearMap         = ClientProceduralTerrainAppearance::Cache::createRadialMap (ccd_floraDynamicNearMap->getWidth (), ccd_floraDynamicNearMap->getHeight ());
		floraDynamicFarMap          = ClientProceduralTerrainAppearance::Cache::createRadialMap (ccd_floraDynamicFarMap->getWidth (), ccd_floraDynamicFarMap->getHeight ());
		environmentMap              = ClientProceduralTerrainAppearance::Cache::createEnvironmentMap (ccd_environmentMap->getWidth (), ccd_environmentMap->getHeight ());
		colorMap                    = ClientProceduralTerrainAppearance::Cache::createColorMap (ccd_colorMap->getWidth (), ccd_colorMap->getHeight ());
	ClientProceduralTerrainAppearance::Cache::unlock ();

	shaderMap->makeCopy (*ccd_shaderMap);
	vertexNormalMap->makeCopy (*ccd_vertexNormalMap);
	//floraStaticCollidableMap->makeCopy (*ccd_floraStaticCollidableMap);
	floraStaticNonCollidableMap->makeCopy (*ccd_floraStaticNonCollidableMap);
	floraDynamicNearMap->makeCopy (*ccd_floraDynamicNearMap);
	floraDynamicFarMap->makeCopy (*ccd_floraDynamicFarMap);
	environmentMap->makeCopy (*ccd_environmentMap);
	colorMap->makeCopy (*ccd_colorMap);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- initialization
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	m_boxExtent.setMin (Vector::maxXYZ);
	m_boxExtent.setMax (Vector::negativeMaxXYZ);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- build tiles which create shader fan sets
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	{
		BoxExtent tileExtent;
		Rectangle2d tileExtent2d;

		Vector vertexList[9];
		Vector normalList[9];
		PackedArgb colorList[9];

		//-- at the highest detail level, we should have a uv scale of 1 (2m tiles)
		float const baseUvScale = 0.25f * chunkWidthInMeters / numberOfTilesPerChunk;

		int x;
		int z;
		for (z = 0; z < numberOfTilesPerChunk; z++)
		{
			for (x = 0; x < numberOfTilesPerChunk; x++)
			{
				//-- find out where it is in the world
				const int indexX = x * 2 + originOffset;
				const int indexZ = z * 2 + originOffset;

				if (!ccd_passableMap->getData(indexX, indexZ))
				{
					setPassable(x, z, false);
				}

				//-- create the shader
				ShaderData shaderData;
				createTileShader (shaderData, indexX, indexZ, ConfigClientTerrain::getShaderGroupUseFirstChildOnly() ? true : createChunkData.chunkWidthInMeters >= 64);

				//-- fetch the shader set
				ShaderSet* shaderSet = 0;

				{
					const uint n = m_shaderSetList.size();
					uint i;
					for (i = 0; i < n; ++i)
						if (m_shaderSetList[i]->getShader () == shaderData.outputShader)
						{
							shaderSet = m_shaderSetList[i];

							break;
						}

					if (i == n)
					{
						shaderSet = new ShaderSet (shaderData.outputShader);
						m_shaderSetList.push_back(shaderSet);
					}
				}

				NOT_NULL (shaderSet);

				//-- number of texture coordinate sets = normal basis matrix + base + optional
				const int numberOfTextureCoordinateSets = shaderData.numberOfTextures;

				{
					tileExtent.setMin (Vector::maxXYZ);
					tileExtent.setMax (Vector::negativeMaxXYZ);

					for (int i = 0; i < VERTEX_INDIRECTION_ARRAY_SIZE; ++i)
					{
						const VertexIndirectionData& vid = constructionIndirectionArray [i];

						const int iX = indexX + vid.x;
						const int iZ = indexZ + vid.z;

						const Vector position = vertexPositionMap->getData (iX, iZ);
						vertexList[i] = position;
						normalList[i] = ccd_vertexNormalMap->getData(iX, iZ);
						colorList[i] = ccd_colorMap->getData(iX, iZ).convert();
#if PRODUCTION == 0
						if (ClientProceduralTerrainAppearance::isShowPassable())
						{
							PackedRgb prgb = ccd_colorMap->getData (iX, iZ);
							//-- exaggerate passable areas with color
							if (!ccd_passableMap->getData(iX, iZ))
								prgb = PackedRgb::linearInterpolate(prgb, PackedRgb::solidBlack, 0.6f);
							else
								prgb = PackedRgb::linearInterpolate(prgb, PackedRgb::solidWhite, 0.6f);
							colorList[i] = prgb.convert();
						}
#endif

						//-- extents
						m_boxExtent.updateMinAndMax (position);
						tileExtent.updateMinAndMax (position);
					}

					tileExtent.calculateCenterAndRadius ();
				}

				//-- add to shader set
				tileExtent2d.set (tileExtent.getMin ().x, tileExtent.getMin ().z, tileExtent.getMax ().x, tileExtent.getMax ().z);
				shaderSet->addPrimitive(x, z, vertexList, normalList, colorList, numberOfTextureCoordinateSets, baseUvScale, shaderData.m_rotationTypes, tileExtent.getSphere (), tileExtent2d, ccd_excludeMap->getData (indexX, indexZ) && isLeaf);

				//-- TODO: see if chooseIndexBuffer can be moved to ShaderSet::Primitive
				shaderSet->chooseIndexBuffer (hasLargerNeighborFlags, numberOfTilesPerChunk);
			}
		}
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- update extents
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	m_boxExtent.calculateCenterAndRadius();

	Sphere boundingSphere = m_boxExtent.getSphere();
	boundingSphere.setRadius(boundingSphere.getRadius() + 0.001f);
	m_boxExtent.setSphere(boundingSphere);

	//-- create write shape
	if (ConfigClientTerrain::getUseOcclusion ())
	{
		if (ConfigClientTerrain::getUseRealGeometryForOcclusion ())
		{
			const int actualNumberOfPoles = numberOfTilesPerChunk * 2 + 1;

			std::vector<Vector> vertices;
			vertices.reserve (static_cast<uint> (sqr (actualNumberOfPoles - originOffset)));

			{
				int x;
				int z;
				for (z = originOffset; z < originOffset + actualNumberOfPoles; z++)
					for (x = originOffset; x < originOffset + actualNumberOfPoles; x++)
						vertices.push_back (vertexPositionMap->getData (x, z));
			}

			//-- todo index list is the same for every chunk
			std::vector<int> indices;
			indices.reserve (static_cast<uint> (sqr (numberOfTilesPerChunk) * TRIANGLE_INDIRECTION_ARRAY_SIZE * 3));

			{
				int x;
				int z;
				for (z = 0; z < numberOfTilesPerChunk; z++)
				{
					for (x = 0; x < numberOfTilesPerChunk; x++)
					{
						const int tileOriginX = x * 2;
						const int tileOriginZ = z * 2;

						if (ccd_excludeMap->getData (tileOriginX + originOffset, tileOriginZ + originOffset))
							continue;

						int i;
						for (i = 0; i < TRIANGLE_INDIRECTION_ARRAY_SIZE; i++)
						{
							const int i0x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX0;
							const int i0z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ0;
							const int i0  = i0x + i0z * (actualNumberOfPoles);

							const int i1x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX1;
							const int i1z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ1;
							const int i1  = i1x + i1z * (actualNumberOfPoles);

							const int i2x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX2;
							const int i2z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ2;
							const int i2  = i2x + i2z * (actualNumberOfPoles);

							indices.push_back (i0);
							indices.push_back (i1);
							indices.push_back (i2);
						}
					}
				}
			}

			m_writeIndexedTriangleList = new IndexedTriangleList ();
			m_writeIndexedTriangleList->addIndexedTriangleList (&vertices [0], static_cast<int> (vertices.size ()), &indices [0], static_cast<int> (indices.size ()));
		}
		else
		{
			//   6---7
			//  /|  /|
			// 2-+-3 |
			// | 4-+-5  <----front (larger z)
			// |/  |/
			// 0---1  <----rear (smaller z)

			const Vector emin = m_boxExtent.getMin ();
			const Vector emax = m_boxExtent.getMax ();

			const Vector vertices [8] =
			{
				Vector (emin.x, -1000.f, emin.z),  // 0
				Vector (emax.x, -1000.f, emin.z),  // 1
				Vector (emin.x, emin.y, emin.z),  // 2
				Vector (emax.x, emin.y, emin.z),  // 3
				Vector (emin.x, -1000.f, emax.z),  // 4
				Vector (emax.x, -1000.f, emax.z),  // 5
				Vector (emin.x, emin.y, emax.z),  // 6
				Vector (emax.x, emin.y, emax.z)   // 7
			};

			const int indices [3 * 12] =
			{
				0, 2, 3, 0, 1, 3,   // rear
				1, 3, 7, 1, 7, 5,   // right
				2, 6, 3, 6, 7, 3,   // top
				0, 6, 2, 0, 4, 6,   // left
				0, 5, 4, 0, 1, 5,   // bottom
				6, 5, 7, 6, 4, 5    // front
			};

			m_writeIndexedTriangleList = new IndexedTriangleList ();
			m_writeIndexedTriangleList->addIndexedTriangleList (vertices, 8, indices, 36);
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::render () const
{
#ifdef _DEBUG
	if (ClientProceduralTerrainAppearance::getNoRenderTerrain())
	{
		return;
	}
#endif
	if (!ClientProceduralTerrainAppearance::getEnableRender())
	{
		return;
	}

	const ClientProceduralTerrainAppearance *owner = static_cast<const ClientProceduralTerrainAppearance *>(&m_proceduralTerrainAppearance);
	owner->_onChunkRendered();
	const Camera *camera = &ShaderPrimitiveSorter::getCurrentCamera();

	const ShaderSet *const*const shaderSetList = &m_shaderSetList[0];

	//-- render shader fan sets
	const uint n = m_shaderSetList.size();
	uint i;
	for (i = 0; i < n; ++i)
	{
		shaderSetList[i]->render (camera, m_lotShader, ms_cloudShader);
	}

	//-- render vertex normals
	if (getDrawVertexNormals ())
	{
		Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
		for (i = 0; i < n; ++i)
		{
			shaderSetList[i]->renderNormals();
		}
	}

	//-- render box extent
	if (getDrawExtent())
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader ());
		Graphics::drawExtent(&m_boxExtent, VectorArgb::solidWhite);
	}		

	//-- render shadow blobs
	ShadowBlobManager::renderShadowBlobs (*this);

	//-- render reticle
	ReticleManager::renderReticles (*this);

	m_lotShader = 0;
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::findStaticNonCollidableFlora (float positionX, float positionZ, StaticFloraData& data, bool& floraAllowed) const
{
	NOT_NULL (floraStaticNonCollidableMap);
	return _findStaticFlora (*floraStaticNonCollidableMap, positionX, positionZ, data, floraAllowed);
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::findDynamicFlora (const Array2d<RadialGroup::Info>& radialMap, 
																							  float positionX, 
																							  float positionZ, 
																							  DynamicFloraData& data, 
																							  bool& floraAllowed, 
																							  bool constrainToHighDetail
																							  ) const
{
	if ((!constrainToHighDetail && chunkWidthInMeters <= 128.f) || chunkWidthInMeters <= 16.f)
	{
		int tileX, tileZ;
		Vector pos3(positionX, 0, positionZ);

		_findTileXz(pos3, tileX, tileZ);
		if (isExcluded(tileX, tileZ))
			return false;

		floraAllowed = true;

		int x;
		int z;
		_findMapXz(pos3, x, z);

		const RadialGroup::Info rgi = radialMap.getData (x, z);

		if (rgi.getFamilyId())
		{
			RadialGroup const & radialGroup = m_proceduralTerrainAppearance.getRadialGroup();
			NOT_NULL (colorMap);

			//-- terrain data
			data.color           = colorMap->getData (x, z);

			//-- family data


			//-- family child data
			data.familyChildData = &radialGroup.createRadial(rgi);

			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::findDynamicNearFlora (float positionX, float positionZ, DynamicFloraData& data, bool& floraAllowed) const
{
	NOT_NULL (floraDynamicNearMap);
	return findDynamicFlora (*floraDynamicNearMap, positionX, positionZ, data, floraAllowed, true);
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::findDynamicFarFlora (float positionX, float positionZ, DynamicFloraData& data, bool& floraAllowed) const
{
	NOT_NULL (floraDynamicFarMap);
	return findDynamicFlora (*floraDynamicFarMap, positionX, positionZ, data, floraAllowed, false);
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::findEnvironment (const Vector& position, EnvironmentData& data) const
{
	int x;
	int z;
	_findMapXz (position, x, z);

	NOT_NULL (environmentMap);
	const EnvironmentGroup::Info egi = environmentMap->getData (x, z);

	int const familyId = egi.getFamilyId();
	if (familyId)
	{
		data.familyId = familyId;

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::addObjectToWorld ()
{
	if (!m_dpvsObject)
	{
		//-- create the DPVS object
		DEBUG_FATAL (m_dpvsObject, ("dpvs object already exists"));
		{
			DPVS::OBBModel *const testModel = RenderWorld::fetchBoxModel(m_boxExtent.getBox());
			m_dpvsObject = RenderWorld::createObject (this, testModel);
			testModel->release ();
		}

		if (m_writeIndexedTriangleList)
		{
			DPVS::MeshModel *const writeModel = RenderWorld::fetchMeshModel (*m_writeIndexedTriangleList);
			m_dpvsObject->setWriteModel(writeModel);
			writeModel->release();
		}

		m_inWorld=false;
	}
	if (!m_inWorld)
	{
		RenderWorld::addObjectToWorld(m_dpvsObject, getOwner());
		m_inWorld=true;
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::removeObjectFromWorld ()
{
	if (m_dpvsObject)
	{
		if (m_inWorld)
		{
			RenderWorld::removeObjectFromWorld(m_dpvsObject);
			m_inWorld=false;
		}
	}
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ClientChunk::isObjectInWorld()
{
	DEBUG_FATAL(m_inWorld!=RenderWorld::isObjectInWorld(m_dpvsObject), ("In-world flag out of sync with DPVS object."));
	return m_inWorld;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::setLotShader (const Shader* lotShader)
{
	m_lotShader = lotShader;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const
{
	const uint n = m_shaderSetList.size();
	uint i;
	for (i = 0; i < n; ++i)
		m_shaderSetList[i]->getPolygonSoup (extent2d, indexedTriangleList);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::renderShadowBlob (const Vector& position_o, const float radius) const
{
	const uint n = m_shaderSetList.size();
	uint i;
	for (i = 0; i < n; ++i)
		m_shaderSetList[i]->renderShadowBlob (position_o, radius);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientChunk::renderReticle (const Vector& position_o, const float radius) const
{
	const uint n = m_shaderSetList.size();
	uint i;
	for (i = 0; i < n; ++i)
		m_shaderSetList[i]->renderReticle (position_o, radius);
}
//===================================================================
