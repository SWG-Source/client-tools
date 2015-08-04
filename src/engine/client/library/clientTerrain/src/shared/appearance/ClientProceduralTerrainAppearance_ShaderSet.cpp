// ======================================================================
//
// ClientProceduralTerrainAppearance_ShaderSet.cpp
// asommers
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderSet.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/SystemIndexBuffer.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientObject/ShadowBlobManager.h"
#include "clientObject/ReticleManager.h"
#include "clientTerrain/ClientTerrainSorter.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

#include <algorithm>

// ======================================================================

namespace ClientProceduralTerrainAppearanceShaderSetNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static int const TRIANGLE_INDIRECTION_ARRAY_SIZE = 8;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum IndexBufferPattern
	{
		IBP_center      = 0,
		IBP_north       = 1,
		IBP_northwest   = 2,
		IBP_west        = 3,
		IBP_southwest   = 4,
		IBP_south       = 5,
		IBP_southeast   = 6,
		IBP_east        = 7,
		IBP_northeast   = 8,
		IBP_count       = 9
	};

	int const constructionTileIndexArraySizes[IBP_count] =
	{
		(TRIANGLE_INDIRECTION_ARRAY_SIZE    ) * 3,		// center
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 1) * 3,		// north
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 2) * 3,		// northwest
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 1) * 3,		// west
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 2) * 3,		// southwest
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 1) * 3,		// south
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 2) * 3,		// southeast
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 1) * 3,		// east
		(TRIANGLE_INDIRECTION_ARRAY_SIZE - 2) * 3,		// northeast
	};

	Index const constructionTileIndexArrays[IBP_count][TRIANGLE_INDIRECTION_ARRAY_SIZE * 3] =
	{
		//-- center
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 1
		},

		//-- north
		{
			0, 1, 2,
			0, 2, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 1
			
			, 0, 0, 0
		},

		//-- northwest
		{
			0, 2, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 2

			, 0, 0, 0
			, 0, 0, 0
		},

		//-- west
		{
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 2

			, 0, 0, 0
		},

		//-- southwest
		{
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 8,
			0, 8, 2

			, 0, 0, 0
			, 0, 0, 0
		},

		//-- south
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 8,
			0, 8, 1

			, 0, 0, 0
		},

		//-- southeast
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 6,
			0, 6, 8,
			0, 8, 1

			, 0, 0, 0
			, 0, 0, 0
		},

		//-- east
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 1

			, 0, 0, 0
		},

		//-- northeast
		{
			0, 1, 2,
			0, 2, 4,
			0, 4, 6,
			0, 6, 7,
			0, 7, 8,
			0, 8, 1

			, 0, 0, 0
			, 0, 0, 0
		}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Uv
	{
		float m_u;
		float m_v;
	};

	Uv const ms_uvs[9 * 4] = 
	{
		// 0
		{ 0.5f, 0.5f },
		{ 0.f, 0.5f },
		{ 0.f, 0.f },
		{ 0.5f, 0.f },
		{ 1.f, 0.f },
		{ 1.f, 0.5f },
		{ 1.f, 1.f },
		{ 0.5f, 1.f },
		{ 0.f, 1.f },

		// PI_OVER_2
		{ 0.5f, 0.5f },
		{ 0.5f, 0.f },
		{ 1.f, 0.f },
		{ 1.f, 0.5f },
		{ 1.f, 1.f },
		{ 0.5f, 1.f },
		{ 0.f, 1.f },
		{ 0.f, 0.5f },
		{ 0.f, 0.f },

		//PI
		{ 0.5f, 0.5f },
		{ 1.f, 0.5f },
		{ 1.f, 1.f },
		{ 0.5f, 1.f },
		{ 0.f, 1.f },
		{ 0.f, 0.5f },
		{ 0.f, 0.f },
		{ 0.5f, 0.f },
		{ 1.f, 0.f },

		//-PI_OVER_2
		{ 0.5f, 0.5f },
		{ 0.5f, 1.f },
		{ 0.f, 1.f },
		{ 0.f, 0.5f },
		{ 0.f, 0.f },
		{ 0.5f, 0.f },
		{ 1.f, 0.f },
		{ 1.f, 0.5f },
		{ 1.f, 1.f }
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class DenormalizedLine2d
	{
	public:

		DenormalizedLine2d (const Vector2d& point0, const Vector2d& point1) :
			m_normal (-point1.y + point0.y, point1.x - point0.x),
			m_c (-m_normal.dot (point0))
		{
		}

		float computeDistanceTo (const Vector2d& point) const
		{
			return m_normal.dot (point) + m_c;
		}

	private:

		DenormalizedLine2d ();

	private:

		Vector2d   m_normal;
		float      m_c;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//-- there are only 9 possible index buffers, and these are all shared with all of the chunks
	typedef stdvector<SystemIndexBuffer*>::fwd IndexBufferList;
	IndexBufferList * ms_indexBufferList;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ClientProceduralTerrainAppearanceShaderSetNamespace;

// ======================================================================
// ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray
// ======================================================================

class ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();
	static void remove ();

public:

	Plane* getPlaneArray ();
	const Plane* getPlaneArray () const;

private:

	Plane m_planeArray [8];
};

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray::install ()
{
	installMemoryBlockManager ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray::remove ()
{
	removeMemoryBlockManager ();
}

//-------------------------------------------------------------------

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray, true, 0, 0, 0);

//-------------------------------------------------------------------

Plane* ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray::getPlaneArray ()
{
	return &m_planeArray [0];
}

//-------------------------------------------------------------------

const Plane* ClientProceduralTerrainAppearance::ShaderSet::Primitive::PlaneArray::getPlaneArray () const
{
	return &m_planeArray [0];
}

// ======================================================================
// ClientProceduralTerrainAppearance::ShaderSet
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientProceduralTerrainAppearance::ShaderSet, true, 0, 0, 0);

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::install ()
{
	installMemoryBlockManager();

	Primitive::install ();

	ms_indexBufferList = new IndexBufferList;

	//-- create index buffer data
	int i;
	for (i = 0; i < IBP_count; ++i)
	{
		SystemIndexBuffer* const indexBuffer = new SystemIndexBuffer (constructionTileIndexArraySizes [i]);

			int j;
			for (j = 0; j < constructionTileIndexArraySizes [i]; ++j)
				(*indexBuffer)[j] = constructionTileIndexArrays [i][j];

		// leave the index buffer locked so it can be used by multiple threads

		ms_indexBufferList->push_back (indexBuffer);
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::remove ()
{
	removeMemoryBlockManager();

	Primitive::remove ();

	//-- delete index buffer data
	uint i;
	for (i = 0; i < IBP_count; ++i)
	{
		delete (*ms_indexBufferList) [i];
		(*ms_indexBufferList) [i] = 0;
	}

	delete ms_indexBufferList;
	ms_indexBufferList = 0;
}

//===================================================================

ClientProceduralTerrainAppearance::ShaderSet::ShaderSet (const Shader* shader) :
	m_shader (shader)
{
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ShaderSet::~ShaderSet ()
{
	m_shader = 0;

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		Primitive* const primitive = m_primitiveList[i];

//			ClientProceduralTerrainAppearance::Cache::destroyVertexBuffer (NON_NULL (primitive->m_vertexBuffer));
		delete primitive->m_planeArray;

		delete primitive;
	}

	m_primitiveList.clear();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::addPrimitive(int const primitiveX, int const primitiveZ, Vector const * const primitivePositionList, Vector const * primitiveNormalList, PackedArgb const * primitiveColorList, int numberOfTextureCoordinateSets, float const baseUvScale, RotationType const * const rotationType, const Sphere& primitiveSphere, const Rectangle2d& primitiveExtent2d, const bool excluded)
{
	Primitive * const primitive = NON_NULL(new Primitive());
	primitive->m_excluded       = excluded;
	primitive->m_x              = primitiveX;
	primitive->m_z              = primitiveZ;
	memcpy(&primitive->m_positionList, primitivePositionList, sizeof(primitive->m_positionList));
	memcpy(&primitive->m_normalList, primitiveNormalList, sizeof(primitive->m_normalList));
	memcpy(&primitive->m_colorList, primitiveColorList, sizeof(primitive->m_colorList));
	primitive->m_numberOfTextureCoordinateSets = numberOfTextureCoordinateSets;
	primitive->m_baseUvScale = baseUvScale;
	memcpy(&primitive->m_rotationType, rotationType, sizeof(primitive->m_rotationType));
	primitive->m_indexBuffer    = 0;
	primitive->m_planeArray     = new Primitive::PlaneArray ();
	primitive->m_sphere         = primitiveSphere;
	primitive->m_extent2d       = primitiveExtent2d;

	m_primitiveList.push_back(primitive);
}

//-------------------------------------------------------------------

static inline int getTilePatternIndex (const int x, const int z, const int topTile, unsigned hasLargerNeighborFlags)
{
	if (  hasLargerNeighborFlags==0
		|| (  x != 0 
		   && z != 0 
			&& x != topTile 
			&& z != topTile
			)
		)
	{
		return IBP_center;
	}

	// check south row first
	if (z == 0 && (hasLargerNeighborFlags&TerrainQuadTree::Node::SOUTH_FLAG))
	{
		// southwest
		if (x == 0 && (hasLargerNeighborFlags&TerrainQuadTree::Node::WEST_FLAG))
			return IBP_southwest;
		// southeast
		else if (x == topTile && (hasLargerNeighborFlags&TerrainQuadTree::Node::EAST_FLAG))
			return IBP_southeast;
		// south
		else
			return IBP_south;
	}

	// check west column
	else if (x == 0 && (hasLargerNeighborFlags&TerrainQuadTree::Node::WEST_FLAG))
	{
		// northwest
		if (z == topTile && (hasLargerNeighborFlags&TerrainQuadTree::Node::NORTH_FLAG))
			return IBP_northwest;
		// west
		else
			return IBP_west;

		// southwest was already handled
	}

	// check north row
	else if (z == topTile && (hasLargerNeighborFlags&TerrainQuadTree::Node::NORTH_FLAG))
	{
		// northeast
		if (x == topTile && (hasLargerNeighborFlags&TerrainQuadTree::Node::EAST_FLAG))
			return IBP_northeast;
		// north
		else
			return IBP_north;
		// northwest was already handled
	}

	// (no check needed)
	// check east column
	else if (x == topTile && (hasLargerNeighborFlags&TerrainQuadTree::Node::EAST_FLAG))
	{
		return IBP_east;
		// northeast and southeast were already handled
	}

	else
		return IBP_center;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::chooseIndexBuffer(unsigned newHasLargerNeighborFlags, int numberOfTilesPerChunk)  
{  //lint !e578  // numberOfTilesPerChunk hides ProceduralTerrainAppearance::numberOfTilesPerChunk
	if (m_primitiveList.empty())
		return;

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		Primitive* const primitive = m_primitiveList[i];

		//-- select which index buffer based on neighbor information
		Vector const * const vertices = &primitive->m_positionList[0];
		int tilePatternIndex = getTilePatternIndex(primitive->m_x, primitive->m_z, numberOfTilesPerChunk - 1, newHasLargerNeighborFlags);
		const SystemIndexBuffer* const  indexBuffer  = (*ms_indexBufferList)[static_cast<uint> (tilePatternIndex)];
		Index const * indices = indexBuffer->beginReadOnly();
		Plane* const                    planeArray   = primitive->m_planeArray->getPlaneArray ();
		NOT_NULL (planeArray);

		//-- create plane array for collision
		const int numberOfPlanes = indexBuffer->getNumberOfIndices () / 3;
		DEBUG_FATAL (numberOfPlanes < 0 || numberOfPlanes > 8, ("numberOfPlanes out of range %i [0/7]", numberOfPlanes));

		for (int j = 0; j < numberOfPlanes; ++j)
		{
			Vector const & v0 = vertices[*indices++];
			Vector const & v1 = vertices[*indices++];
			Vector const & v2 = vertices[*indices++];
			planeArray[j].set (v0, v1, v2);
		}
				
		//-- set the index buffer
		primitive->m_indexBuffer = indexBuffer;
	}
}

//-------------------------------------------------------------------

const Shader* ClientProceduralTerrainAppearance::ShaderSet::getShader () const
{
	return m_shader;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::render (const Camera* camera, 
																			  const Shader* lotShader, 
																			  const Shader* i_cloudShader
																			  ) const
{
	UNREF (camera);

	if (m_primitiveList.empty())
		return;

	const bool dot3Terrain = Graphics::getShaderCapability () >= ShaderCapability(1,1) && ClientProceduralTerrainAppearance::getDot3Terrain();
	const Shader *const cloudShader = (dot3Terrain) ? i_cloudShader : (const Shader *)0;
	const Shader *shader = m_shader;
	const Vector cameraPosition = camera->getPosition_w ();
	const Primitive *const*const primitiveList = &m_primitiveList[0];
	const uint numPrimitives = m_primitiveList.size();

	uint i;
	for (i = 0; i < numPrimitives; ++i)
	{
		const Primitive* const primitive = primitiveList[i];
		if (primitive->m_excluded)
		{
			continue;
		}

		const Sphere& primitiveSphere = primitive->m_sphere;

		if (camera->testVisibility_w (primitiveSphere))
		{
#if 1
			bool tileIsBackfacing = true;

			//-- see if all of the normals point away from the camera
			{
				const Plane* const planeArray = primitive->m_planeArray->getPlaneArray ();

				int j;
				for (j = 0; j < 8; ++j)
				{
					if (planeArray [j].computeDistanceTo (cameraPosition) > 0.0f)
					{
						tileIsBackfacing = false;

						break;
					}
				}
			}

			if (tileIsBackfacing)
				continue;
#endif

			ClientTerrainSorter::queue (shader, primitive);

			if (cloudShader && dot3Terrain)
				ClientTerrainSorter::queue (cloudShader, primitive);

			if (lotShader)
				ClientTerrainSorter::queue (lotShader, primitive);
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::renderNormals () const
{
	return;

/*
	if (m_primitiveList.empty())
		return;

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		const SystemVertexBuffer* const vertexBuffer = primitive->m_vertexBuffer;
		Graphics::drawVertexNormals (vertexBuffer->beginReadOnly(), vertexBuffer->endReadOnly());
	}
*/
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ShaderSet::collide (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const
{
	if (m_primitiveList.empty())
		return false;

	bool found = false;

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		if (primitive->m_excluded)
			continue;

		Vector const * const vertices = &primitive->m_positionList[0];
		const SystemIndexBuffer* const  indexBuffer    = primitive->m_indexBuffer;  // indexBuffer is always locked
		const Plane* const              planeArray     = NON_NULL (primitive->m_planeArray->getPlaneArray ());
		const int                       numberOfPlanes = indexBuffer->getNumberOfIndices () / 3;

		if (planeArray)
		{
			Index const * indices = indexBuffer->beginReadOnly();
			const Vector       dir     = end_o - start_o;

			Vector intersection;
			Vector normal;

			int k;
			for (k = 0; k < numberOfPlanes; k++)
			{
				Vector const & v0 = vertices[*indices++];
				Vector const & v1 = vertices[*indices++];
				Vector const & v2 = vertices[*indices++];

				normal = planeArray [k].getNormal ();

				if ((dir.dot (normal) <= 0.f) && (planeArray [k].findIntersection (start_o, end_o, intersection)))
				{
					if (intersection.inPolygon (v0, v1, v2) && (start_o.magnitudeBetweenSquared (intersection) < start_o.magnitudeBetweenSquared (result.getPoint())))
					{
						found         = true;

						result.setPoint(intersection);
						result.setNormal(normal);
					}
				}
			}
		}
	}

	if (found)
	{
		// calculate time
		// according to the use in FreeChaseCamera the time that is
		// returned by result should __NOT__ be parametric
		result.setTime(start_o.magnitudeBetween(result.getPoint()));
	}

	return found;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::ShaderSet::getHeightAt (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const
{
	if (m_primitiveList.empty())
		return false;

	Vector   intersection;
	Vector2d extentMin;
	Vector2d extentMax;

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		if (primitive->m_excluded)
			continue;

		Vector const * const vertices = &primitive->m_positionList[0];
		const SystemIndexBuffer* const  indexBuffer    = primitive->m_indexBuffer;  // indexBuffer is always locked
		Index const * indices = indexBuffer->beginReadOnly();
		const Plane* const              planeArray     = NON_NULL (primitive->m_planeArray)->getPlaneArray ();
		const int                       numberOfPlanes = indexBuffer->getNumberOfIndices () / 3;

		if (planeArray)
		{
			int k;
			for (k = 0; k < numberOfPlanes; k++)
			{
				//-- see if vector hits the polygon in the xz plane
				Vector const & v0 = vertices[*indices++];
				Vector const & v1 = vertices[*indices++];
				Vector const & v2 = vertices[*indices++];

				if (planeArray [k].findIntersection (start_o, end_o, intersection))
				{
					extentMin.set (v0.x, v0.z);
					extentMax = extentMin;

					extentMin.x = std::min (v1.x, extentMin.x);
					extentMin.x = std::min (v2.x, extentMin.x);
					extentMin.y = std::min (v1.z, extentMin.y);
					extentMin.y = std::min (v2.z, extentMin.y);
					extentMax.x = std::max (v1.x, extentMax.x);
					extentMax.x = std::max (v2.x, extentMax.x);
					extentMax.y = std::max (v1.z, extentMax.y);
					extentMax.y = std::max (v2.z, extentMax.y);

					if (WithinRangeInclusiveInclusive (extentMin.x, start_o.x, extentMax.x) &&
						WithinRangeInclusiveInclusive (extentMin.y, start_o.z, extentMax.y))
					{
#if 0
						if (intersection.inPolygon (v0, v1, v2))
#else
						DenormalizedLine2d line01 (Vector2d (v0.x, v0.z), Vector2d (v1.x, v1.z));
						DenormalizedLine2d line12 (Vector2d (v1.x, v1.z), Vector2d (v2.x, v2.z));
						DenormalizedLine2d line20 (Vector2d (v2.x, v2.z), Vector2d (v0.x, v0.z));

						if (line01.computeDistanceTo (Vector2d (start_o.x, start_o.z)) <= 0 &&
							line12.computeDistanceTo (Vector2d (start_o.x, start_o.z)) <= 0 &&
							line20.computeDistanceTo (Vector2d (start_o.x, start_o.z)) <= 0)
#endif
						{
							result.setPoint(intersection);
							result.setNormal(planeArray [k].getNormal ());

							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const
{
	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		if (primitive->m_excluded)
			continue;

		if (extent2d.intersects (primitive->m_extent2d))
		{
			const int vertexBufferOffset = static_cast<int> (indexedTriangleList.getVertices ().size ());

			//-- copy vertex buffer contents
			{
				std::vector<Vector> & vertices = indexedTriangleList.getVertices();

				for (int j = 0; j < 9; ++j)
					vertices.push_back(primitive->m_positionList[j]);
			}

			//-- copy index buffer contents (with offset)
			{
				const SystemIndexBuffer* const indexBuffer = primitive->m_indexBuffer;
				const Index* const indexArray = indexBuffer->beginReadOnly ();
				std::vector<int>& indices = indexedTriangleList.getIndices ();

				int j;
				for (j = 0; j < indexBuffer->getNumberOfIndices (); ++j)
					indices.push_back (indexArray [j] + vertexBufferOffset);
			}
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::renderShadowBlob (const Vector& position_o, float radius) const
{
	if (radius < 0.25f)
		return;

	const float ooRadius = 1.0f / radius;
	const Rectangle2d extent2d (position_o.x - radius, position_o.z - radius, position_o.x + radius, position_o.z + radius);

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		if (primitive->m_excluded)
			continue;

		if (extent2d.intersects (primitive->m_extent2d))
		{
			Vector const * const positionList = &primitive->m_positionList[0];
			const SystemIndexBuffer* const indexBuffer = primitive->m_indexBuffer;
			Index const * indices = indexBuffer->beginReadOnly();

			for (int j = 0; j < indexBuffer->getNumberOfIndices (); ++j)
			{
				Vector const & position = positionList[*indices++];
				const float u = (position.x - position_o.x) * ooRadius + 0.5f;
				const float v = (position.z - position_o.z) * ooRadius + 0.5f;

				ShadowBlobManager::addVertex (position, u, v);
			}
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::renderReticle (const Vector& position_o, float radius) const
{
	if (radius < 0.25f)
		return;

	const float ooRadius = 1.0f / radius;
	const Rectangle2d extent2d (position_o.x - radius, position_o.z - radius, position_o.x + radius, position_o.z + radius);

	uint i;
	for (i = 0; i < m_primitiveList.size(); ++i)
	{
		const Primitive* const primitive = m_primitiveList[i];
		if (primitive->m_excluded)
			continue;

		if (extent2d.intersects (primitive->m_extent2d))
		{
			Vector const * const positionList = &primitive->m_positionList[0];
			const SystemIndexBuffer* const indexBuffer = primitive->m_indexBuffer;
			Index const * indices = indexBuffer->beginReadOnly();

			for (int j = 0; j < indexBuffer->getNumberOfIndices (); ++j)
			{
				Vector const & position = positionList[*indices++];
				const float u = (position.x - position_o.x) * ooRadius + 0.5f;
				const float v = (position.z - position_o.z) * ooRadius + 0.5f;

				ReticleManager::addVertex (position, u, v);
			}
		}
	}
}

//===================================================================
// ClientProceduralTerrainAppearance::ShaderSet::Primitive

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientProceduralTerrainAppearance::ShaderSet::Primitive, true, 0, 0, 0);

void ClientProceduralTerrainAppearance::ShaderSet::Primitive::install ()
{
	installMemoryBlockManager();

	PlaneArray::install ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::Primitive::remove ()
{
	removeMemoryBlockManager();

	PlaneArray::remove ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderSet::Primitive::getUv(RotationType const rotationType, int const index, float & u, float & v)
{
	DEBUG_FATAL(rotationType < 0 || rotationType > RT_270, (""));
	DEBUG_FATAL(index < 0 || index >= 9, (""));

	Uv const & uv = ms_uvs[rotationType * 9 + index];
	u = uv.m_u;
	v = uv.m_v;
}

// ======================================================================

ClientProceduralTerrainAppearance::ShaderSet::Primitive::Primitive () :
	m_excluded (false),
	m_x (0),
	m_z (0),
	m_positionList(),
	m_normalList(),
	m_colorList(),
	m_numberOfTextureCoordinateSets(0),
	m_baseUvScale(1.f),
	m_indexBuffer (0),
	m_planeArray (0),
	m_sphere (),
	m_extent2d ()
{
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ShaderSet::Primitive::~Primitive ()
{
	m_indexBuffer    = 0;
	m_planeArray     = 0;
}

// ======================================================================
