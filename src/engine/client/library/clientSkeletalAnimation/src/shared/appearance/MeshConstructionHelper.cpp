// ==================================================================
//
// MeshConstructionHelper.cpp
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshConstructionHelper.h"

#include "clientGraphics/GraphicsDebugFlags.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#define MCH_SHOW_UNUSED_VERTICES 0

// ======================================================================

const int cs_maxSupportedTextureCoordinateSetCount = 4;

// ==================================================================
// lint supression

//lint -esym(754,  PerShaderData::PerShaderData) // Info -- local structure member not referenced
//lint -esym(754,  PerShaderData::operator=)     // Info -- local structure member not referenced

// ==================================================================

struct MeshConstructionHelper::VectorContainer
{
public:
	
	typedef std::vector<Vector> Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::TransformHeader
{
public:

	TransformHeader(void);

public:

	int m_transformCount;
	int m_firstTransformDataIndex;
};

// ------------------------------------------------------------------

inline MeshConstructionHelper::TransformHeader::TransformHeader(void)
:
	m_transformCount(0),
	m_firstTransformDataIndex(-1)
{
}

// ==================================================================

struct MeshConstructionHelper::TransformHeaderContainer
{
public:

	typedef std::vector<TransformHeader>  Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::TransformDataContainer
{
public:

	typedef std::vector<TransformData>  Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::VertexData
{
public:

	VertexData(void);

public:

	int         m_positionIndex;
	int         m_normalIndex;
	int         m_firstTextureCoordinateIndex;
	PackedArgb  m_diffuseColor;

	bool        m_isUsed;
	int         m_shaderVertexIndex;

};

// ==================================================================

struct MeshConstructionHelper::VertexDataContainer
{
public:

	typedef std::vector<VertexData> Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::TriStripHeader
{
public:

	TriStripHeader(void);
	TriStripHeader(int indexCount, int firstTriStripIndex, bool m_flipCullMode);

public:

	int  m_indexCount;
	int  m_firstTriStripIndex;
	bool m_flipCullMode;
};

// ==================================================================

struct MeshConstructionHelper::TriStripHeaderContainer
{
public:

	typedef std::vector<TriStripHeader>  Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::TriStripIndexContainer
{
public:

	typedef std::vector<size_t>  Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::PerShaderData
{
public:

	typedef std::vector<size_t>  IndexContainer;

public:

	explicit PerShaderData(const char *shaderTemplateName);

	void     calculateTextureCoordinateInfo();

public:

	uint                  m_vertexFormat;

	int                   m_textureCoordinateSetCount;
	int                   m_textureCoordinateDimensionality[cs_maxSupportedTextureCoordinateSetCount];

	int                   m_textureCoordinateCount;
	int                   m_textureCoordinateStartOffset[cs_maxSupportedTextureCoordinateSetCount];

	PersistentCrcString   m_shaderTemplateName;

	IndexContainer        m_vertexIndices;
	IndexContainer        m_triStripIndices;
	IndexContainer        m_triListIndices;

	// index of this perShaderData, assigned by prepareForRead()
	int                   m_usedIndex;  

	int                   m_usedVertexCount;
	int                   m_numberOfUsedTransformWeightPairs;
	int                   m_numberOfWeightedPrimitiveIndices;

private:

	// disabled
	PerShaderData(void);
	PerShaderData(const PerShaderData&);
	PerShaderData &operator =(const PerShaderData&);

};

// ==================================================================

struct MeshConstructionHelper::TriListHeader
{
	int     m_triangleCount;
	size_t  m_firstTriangleIndex;
};

// ==================================================================

struct MeshConstructionHelper::TriListHeaderContainer
{
public:

	typedef std::vector<TriListHeader>  Container;

public:

	Container  m_container;
};

// ==================================================================

struct MeshConstructionHelper::PerShaderDataContainer
{
public:

	typedef std::map<CrcString*, PerShaderData*, LessPointerComparator>  Container;

public:

	Container  m_container;
};

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct AffectedShaderData
	{
	public:

		AffectedShaderData(MeshConstructionHelper::PerShaderData *affectedShader, Tag shaderTextureTag);

	public:

		MeshConstructionHelper::PerShaderData *m_affectedShader;
		Tag                                    m_shaderTextureTag;     // tag of texture to set

		int                                    m_usedIndex;            // set by prepareForReading()

	private:
		// disabled
		AffectedShaderData();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline bool operator ==(const AffectedShaderData &lhs, const AffectedShaderData &rhs)
	{
		return (lhs.m_affectedShader == rhs.m_affectedShader) && (lhs.m_shaderTextureTag == rhs.m_shaderTextureTag);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<AffectedShaderData>                  ASDContainer;
	typedef std::vector<AffectedShaderData>::iterator        ASDIterator;
	typedef std::vector<AffectedShaderData>::const_iterator  ASDConstIterator;

}

// ======================================================================

struct MeshConstructionHelper::PerTextureRendererData
{
public:

	explicit PerTextureRendererData(const CrcLowerString &name);

public:

	CrcLowerString  m_name;
	ASDContainer    m_affectedShaders;

	int             m_usedIndex;      // set by prepareForReading()
	int             m_usedAffectedShaderCount;

private:
	// disabled
	PerTextureRendererData();
};

// ======================================================================
// struct AffectedShaderData
// ======================================================================

namespace
{
	AffectedShaderData::AffectedShaderData(MeshConstructionHelper::PerShaderData *affectedShader, Tag shaderTextureTag)
		:	m_affectedShader(affectedShader),
			m_shaderTextureTag(shaderTextureTag),
			m_usedIndex(-1)
	{
	}
}

// ======================================================================
// class MeshConstructionHelper::VertexData
// ======================================================================

MeshConstructionHelper::VertexData::VertexData(void)
:
	m_positionIndex(-1),
	m_normalIndex(-1),
	m_firstTextureCoordinateIndex(-1),
	m_diffuseColor(),
	m_isUsed(false),
	m_shaderVertexIndex(-1)
{
}

// ======================================================================
// class MeshConstructionHelper::TriStripHeader
// ======================================================================

MeshConstructionHelper::TriStripHeader::TriStripHeader(void)
:
	m_indexCount(0),
	m_firstTriStripIndex(-1),
	m_flipCullMode(false)
{
}

// ----------------------------------------------------------------------

MeshConstructionHelper::TriStripHeader::TriStripHeader(int indexCount, int firstTriStripIndex, bool flipCullMode)
:
	m_indexCount(indexCount),
	m_firstTriStripIndex(firstTriStripIndex),
	m_flipCullMode(flipCullMode)
{
}

// ======================================================================
// class MeshConstructionHelper::PerShaderData
// ======================================================================

MeshConstructionHelper::PerShaderData::PerShaderData(const char *shaderTemplateName) :
	m_vertexFormat(0),
	m_textureCoordinateSetCount(0),
	m_textureCoordinateCount(0),
	m_shaderTemplateName(shaderTemplateName, true),
	m_vertexIndices(),
	m_triStripIndices(),
	m_triListIndices(),
	m_usedIndex(0),
	m_usedVertexCount(0),
	m_numberOfUsedTransformWeightPairs(0),
	m_numberOfWeightedPrimitiveIndices(0)
{
	for (int i = 0; i < cs_maxSupportedTextureCoordinateSetCount; ++i)
	{
		m_textureCoordinateDimensionality[i] = 0;
		m_textureCoordinateStartOffset[i]    = 0;
	}
}

// ----------------------------------------------------------------------
/**
 * Calculate the m_textureCoordinateCount and m_textureCoordinateStartOffset vectors.
 */

void MeshConstructionHelper::PerShaderData::calculateTextureCoordinateInfo()
{
	m_textureCoordinateCount = 0;

	for (int i = 0; i < m_textureCoordinateSetCount; ++i)
	{
		m_textureCoordinateStartOffset[i]  = m_textureCoordinateCount;
		m_textureCoordinateCount          += m_textureCoordinateDimensionality[i];
	}
}

// ======================================================================
// struct MeshConstructionHelper::PerTextureRendererData
// ======================================================================

MeshConstructionHelper::PerTextureRendererData::PerTextureRendererData(const CrcLowerString &name)
:	m_name(name),
	m_affectedShaders(),
	m_usedIndex(-1),
	m_usedAffectedShaderCount(0)
{
}

// ======================================================================
// class MeshConstructionHelper
// ======================================================================

MeshConstructionHelper::MeshConstructionHelper(void) :
	m_positions(0),
	m_transformHeaders(0),
	m_transformData(0),
	m_normals(0),
	m_vertexData(0),
	m_textureCoordinates(new FloatVector()),
	m_triStripHeaders(0),
	m_triStripIndices(0),
	m_triListHeaders(0),
	m_indexedTriangles(0),
	m_usedPerShaderDataCount(0),
	m_perShaderData(0),
	m_usedPerTextureRendererDataCount(0),
	m_perTextureRendererData(new PTRDContainer())
{
	m_positions          = new VectorContainer();
	m_transformHeaders   = new TransformHeaderContainer();
	m_transformData      = new TransformDataContainer();
	m_normals            = new VectorContainer();

	m_vertexData         = new VertexDataContainer();
	m_triStripHeaders    = new TriStripHeaderContainer();
	m_triStripIndices    = new TriStripIndexContainer();
	m_triListHeaders     = new TriListHeaderContainer();
	m_indexedTriangles   = new IndexedTriangleContainer();
	m_perShaderData      = new PerShaderDataContainer();

	//-- Reserve reasonable space for vectors.
	const int reserveVertexCount = 2500;

	m_positions->m_container.reserve(reserveVertexCount);
	m_normals->m_container.reserve(reserveVertexCount);
	m_vertexData->m_container.reserve(reserveVertexCount);

	m_textureCoordinates->reserve(reserveVertexCount * 2);
}

// ----------------------------------------------------------------------

MeshConstructionHelper::~MeshConstructionHelper(void)
{
	std::for_each(m_perTextureRendererData->begin(), m_perTextureRendererData->end(), PointerDeleterPairSecond());
	delete m_perTextureRendererData;

	std::for_each(m_perShaderData->m_container.begin(), m_perShaderData->m_container.end(), PointerDeleterPairSecond());
	delete m_perShaderData;

	delete m_indexedTriangles;
	delete m_triListHeaders;
	delete m_triStripIndices;
	delete m_triStripHeaders;
	delete m_textureCoordinates;
	delete m_vertexData;

	delete m_normals;
	delete m_transformData;
	delete m_transformHeaders;
	delete m_positions;
}

// ----------------------------------------------------------------------

void MeshConstructionHelper::clearAllData(void)
{
	m_positions->m_container.clear();
	m_transformHeaders->m_container.clear();
	m_transformData->m_container.clear();
	m_normals->m_container.clear();

	m_vertexData->m_container.clear();
	m_textureCoordinates->clear();
	m_triStripHeaders->m_container.clear();
	m_triStripIndices->m_container.clear();
	m_triListHeaders->m_container.clear();
	m_indexedTriangles->clear();

	IGNORE_RETURN( std::for_each(m_perTextureRendererData->begin(), m_perTextureRendererData->end(), PointerDeleterPairSecond()) );
	m_perTextureRendererData->clear();
	m_usedPerTextureRendererDataCount = 0;

	IGNORE_RETURN( std::for_each(m_perShaderData->m_container.begin(), m_perShaderData->m_container.end(), PointerDeleterPairSecond()) );
	m_perShaderData->m_container.clear();
	m_usedPerShaderDataCount = 0;
}

// ----------------------------------------------------------------------
/**
 *
 * Call this function after adding all vertex information but
 * before reading any of it back.
 *
 * This function may only be called once after either
 * construction or a call to clearAllData().  Once this function
 * is called, more vertex data cannot be added to this object.
 * This function must be called before any reading is performed
 * on the object.
 */

void MeshConstructionHelper::prepareForReading(void)
{
	NOT_NULL(m_perShaderData);
	NOT_NULL(m_vertexData);
	NOT_NULL(m_transformHeaders);

	
	// What we do here is remove shader vertices that are unreferenced.
	// An unreferenced shader vertex is a vertex that does not appear in any
	// draw primitive for that shader.  The unreferenced vertices are masked
	// out by the read operations.

	// do this for all shaders
	{
		PerShaderDataContainer::Container::const_iterator itEnd = m_perShaderData->m_container.end();
		PerShaderDataContainer::Container::iterator       it    = m_perShaderData->m_container.begin();

		for (; it != itEnd; ++it)
		{
			PerShaderData *const psd = (*it).second;
			NOT_NULL(psd);

			// for each used shader vertex index, assign a "used index"
			psd->m_usedVertexCount        = 0;
			int nextUsedShaderVertexIndex = 0;

#if defined(_DEBUG) && MCH_SHOW_UNUSED_VERTICES
			int unusedVertexCount = 0;
#endif

			const size_t shaderVertexCount = psd->m_vertexIndices.size();
			for (size_t shaderVertexIndex = 0; shaderVertexIndex < shaderVertexCount; ++shaderVertexIndex)
			{
				// get the VertexData for this shader vertex
				const size_t vertexDataIndex = psd->m_vertexIndices[shaderVertexIndex];
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0U, vertexDataIndex, m_vertexData->m_container.size()); //lint !e1703 // arbitrarily chosen template function

				VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

				// if the vertex is used (already marked during addition of draw prim data),
				// assign it the next sequential usedVertex number
				if (vertexData.m_isUsed)
				{
					vertexData.m_shaderVertexIndex = nextUsedShaderVertexIndex;

					++nextUsedShaderVertexIndex;
					++psd->m_usedVertexCount;

					// keep track of the number of used transform weight pairs
					const TransformHeader &transformHeader   = m_transformHeaders->m_container[static_cast<size_t>(vertexData.m_positionIndex)];
					psd->m_numberOfUsedTransformWeightPairs += transformHeader.m_transformCount;
				}
#if defined(_DEBUG) && MCH_SHOW_UNUSED_VERTICES
				else
				{
					++unusedVertexCount;
				}
#endif
			}

#if defined(_DEBUG) && MCH_SHOW_UNUSED_VERTICES
			//-- print out # unused vertices for this shader
			if (unusedVertexCount)
				DEBUG_REPORT_ENGINE_LOG(characterSystem, ("MeshConstructionHelper: shader [%s] has [%d] unused vertices\n", psd->m_shaderTemplateName.c_str(), unusedVertexCount));
			else
				DEBUG_REPORT_ENGINE_LOG(characterSystem, ("MeshConstructionHelper: shader [%s] has no unused vertices\n", psd->m_shaderTemplateName.c_str(), unusedVertexCount));
#endif

		}
	}

#if defined(_DEBUG) && MCH_SHOW_UNUSED_VERTICES
	if (DEBUG_REPORT_ENGINE_FLAG(characterSystem))
	{
		//-- do a slow check to see if any vertices are unreferenced (independent of shaders)
		unsigned int globalUnusedVertices = 0;

		const VertexDataContainer::Container::const_iterator itEnd = m_vertexData->m_container.end();
		for (VertexDataContainer::Container::const_iterator it = m_vertexData->m_container.begin(); it != itEnd; ++it)
		{
			const VertexData &vertexData = *it;
			if (!vertexData.m_isUsed)
				++globalUnusedVertices;
		}

		if (globalUnusedVertices)
			DEBUG_REPORT_ENGINE_LOG(characterSystem, ("MeshConstructionHelper: mesh has [%d] unused vertices in total\n", globalUnusedVertices));
	}
#endif

	//-- assign index to each perShaderData that has used vertices
	m_usedPerShaderDataCount = 0;
	{
		PerShaderDataContainer::Container::iterator itEnd = m_perShaderData->m_container.end();
		for (PerShaderDataContainer::Container::iterator it = m_perShaderData->m_container.begin(); it != itEnd; ++it)
		{
			PerShaderData *const psd = it->second;
			if (psd->m_usedVertexCount)
				psd->m_usedIndex = m_usedPerShaderDataCount++;
			else
				psd->m_usedIndex = -1;
		}
	}

	//-- assign index to each perTextureRendererData that references a used ShaderTemplate
	m_usedPerTextureRendererDataCount = 0;
	{
		PTRDContainer::iterator itEnd = m_perTextureRendererData->end();
		for (PTRDContainer::iterator it = m_perTextureRendererData->begin(); it != itEnd; ++it)
		{
			//-- assign index to each affected shader that is 
			PerTextureRendererData *const ptrd = it->second;
			ptrd->m_usedAffectedShaderCount    = 0;

			ASDIterator affectedShaderItEnd = ptrd->m_affectedShaders.end();
			for (ASDIterator affectedShaderIt = ptrd->m_affectedShaders.begin(); affectedShaderIt != affectedShaderItEnd; ++affectedShaderIt)
			{
				AffectedShaderData &asd = *affectedShaderIt;
				if (asd.m_affectedShader->m_usedIndex >= 0)
				{
					// this shader is present in mesh
					asd.m_usedIndex = ptrd->m_usedAffectedShaderCount++;
				}
				else
				{
					// this shader is not used, so neither is this TextureRendererTemplate's affected shader data
					asd.m_usedIndex = -1;
				}
			}

			//-- if any assigned affected shaders, this texture renderer data stays
			if (ptrd->m_usedAffectedShaderCount)
				ptrd->m_usedIndex = m_usedPerTextureRendererDataCount++;
			else
				ptrd->m_usedIndex = -1;
		}
	}

}

// ----------------------------------------------------------------------

void MeshConstructionHelper::allocatePositionBuffer(int positionCount, int *firstIndexToUse, Vector **positionBuffer)
{
	NOT_NULL(firstIndexToUse);
	NOT_NULL(positionBuffer);
	DEBUG_FATAL(positionCount < 1, ("invalid positionCount arg [%d], must be >= 1", positionCount));

	NOT_NULL(m_positions);

	// returned position buffer added to end of existing position container
	*firstIndexToUse = static_cast<int>(m_positions->m_container.size());

	// make sure we've got enough capacity to store the specified vertex positions
	const size_t requiredSize = m_positions->m_container.size() + static_cast<size_t>(positionCount);

#ifdef _DEBUG
	if (requiredSize > m_positions->m_container.capacity())
		DEBUG_REPORT_LOG(GraphicsDebugFlags::logCharacterSystem, ("required position capacity exceeded [%u/%u], growing\n", requiredSize, m_positions->m_container.capacity()));
#endif
	m_positions->m_container.resize(requiredSize);

	// transform headers are 1-to-1 mapping with positions, so must be enlarged as well
	m_transformHeaders->m_container.resize(requiredSize);

	// return position buffer
	*positionBuffer = &( m_positions->m_container[ static_cast<size_t>(*firstIndexToUse) ] );
}

// ----------------------------------------------------------------------
/**
 *
 * Note: all transforms applied to a given position must be added
 * without any intervening calls to this function for any
 * other position.  e.g. you cannot add 1 weight for position 32,
 * then add a weight for position 33, then go back and add another
 * weight for position 32.
 */

void MeshConstructionHelper::addPositionWeight(int positionIndex, int transformIndex, real weight)
{
	NOT_NULL(m_transformHeaders);
	NOT_NULL(m_transformData);

	DEBUG_FATAL((positionIndex < 0) || (positionIndex >= static_cast<int>(m_transformHeaders->m_container.size())), ("positionIndex [%d] out of valid range [0..%u)", positionIndex, m_transformHeaders->m_container.size()));
	DEBUG_FATAL(transformIndex < 0, ("transformIndex [%d] must be >= 0", transformIndex));
	DEBUG_FATAL((weight < CONST_REAL(0)) || (weight > CONST_REAL(1.0)), ("suspicious weight [%f] outside range [0.0..1.0]", static_cast<float>(weight)));

	// get transform header for the position
	TransformHeader &transformHeader = m_transformHeaders->m_container[static_cast<size_t>(positionIndex)];
	if (transformHeader.m_transformCount == 0)
	{
		// set the first transform data index
		transformHeader.m_firstTransformDataIndex = static_cast<int>(m_transformData->m_container.size());
	}

#ifdef _DEBUG
	if (m_transformData->m_container.size() == m_transformData->m_container.capacity())
		DEBUG_REPORT_LOG(GraphicsDebugFlags::logCharacterSystem, ("new transform weight forcing resize [%u]\n", m_transformData->m_container.size()));
#endif

	// add transform weight
	m_transformData->m_container.push_back(TransformData(transformIndex, weight));
	++transformHeader.m_transformCount;

	// sanity check --- make sure caller has not added weights for other
	// positions in between adding weights for this position
#ifdef _DEBUG
	const int  expectedDataSize = transformHeader.m_firstTransformDataIndex + transformHeader.m_transformCount;
	const bool outOfOrder       = static_cast<int>(m_transformData->m_container.size()) != expectedDataSize;
	DEBUG_FATAL(outOfOrder, ("caller adding position weights out of order [%d/%u]", expectedDataSize, m_transformData->m_container.size()));
#endif
}

// ----------------------------------------------------------------------

void MeshConstructionHelper::allocateNormalBuffer(int normalCount, int *firstIndexToUse, Vector **normalBuffer)
{
	NOT_NULL(firstIndexToUse);
	NOT_NULL(normalBuffer);
	DEBUG_FATAL(normalCount < 1, ("invalid normalCount arg [%d], must be >= 1", normalCount));

	NOT_NULL(m_normals);

	// returned position buffer added to end of existing position container
	*firstIndexToUse = static_cast<int>(m_normals->m_container.size());

	// make sure we've got enough capacity to store the specified vertex positions
	const size_t requiredSize = m_normals->m_container.size() + static_cast<size_t>(normalCount);

#ifdef _DEBUG
	if (requiredSize > m_normals->m_container.capacity())
		DEBUG_REPORT_LOG(GraphicsDebugFlags::logCharacterSystem, ("required normal capacity exceeded [%u/%u], growing\n", requiredSize, m_normals->m_container.capacity()));
#endif
	m_normals->m_container.resize(requiredSize);

	// return normal buffer
	*normalBuffer = &( m_normals->m_container[ static_cast<size_t>(*firstIndexToUse) ] );
}

// ------------------------------------------------------------------

MeshConstructionHelper::PerShaderData *MeshConstructionHelper::addShaderTemplate(const char *shaderTemplateName)
{
	NOT_NULL(shaderTemplateName);
	NOT_NULL(m_perShaderData);

	// check if we already know about this shader
	TemporaryCrcString crcName(shaderTemplateName, true);

	PerShaderDataContainer::Container::iterator it = m_perShaderData->m_container.find(&crcName);
	if (it != m_perShaderData->m_container.end())
	{
		// found it
		return (*it).second;
	}

	// need to create a new one
	PerShaderData *const psd = new PerShaderData(shaderTemplateName);

	// add it to the map
	IGNORE_RETURN(m_perShaderData->m_container.insert(std::make_pair(&psd->m_shaderTemplateName, psd)));

	return psd;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setVertexFormat(PerShaderData *perShaderData, uint shaderVertexFormat)
{
	NOT_NULL(perShaderData);
	perShaderData->m_vertexFormat = shaderVertexFormat;
} //lint !e1762 // function could be made const // logically non-const.

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinateSetCount(PerShaderData *perShaderData, int setCount)
{
	NOT_NULL(perShaderData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setCount, cs_maxSupportedTextureCoordinateSetCount);

	//-- Remember the set count.
	perShaderData->m_textureCoordinateSetCount = setCount;

	//-- Initialize dimensionalities to two.
	for (int i = 0; i < setCount; ++i)
		perShaderData->m_textureCoordinateDimensionality[i] = 2;

	//-- Calculate texture coordinate information.
	perShaderData->calculateTextureCoordinateInfo();
} //lint !e1762 // function could be made const // logically non-const.

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinateDimensionality(PerShaderData *perShaderData, int setIndex, int dimensionality)
{
	NOT_NULL(perShaderData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData->m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, dimensionality, 5);

	//-- Set the texture coordinate dimensionality.
	perShaderData->m_textureCoordinateDimensionality[setIndex] = dimensionality;

	//-- Calculate texture coordinate information.
	perShaderData->calculateTextureCoordinateInfo();
} //lint !e1762 // function could be made const // logically non-const.

// ------------------------------------------------------------------

int MeshConstructionHelper::getNextNewVertexIndex(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);

	// get the shader vertex index for this new vertex.
	// shader vertex index = vertex offset from first full-fledged vertex associated with this shader.
	const int shaderVertexIndex = static_cast<int>(perShaderData->m_vertexIndices.size());
	return shaderVertexIndex;
}

// ----------------------------------------------------------------------

void MeshConstructionHelper::addVertexPosition(PerShaderData *perShaderData, int positionIndex)
{
	NOT_NULL(perShaderData);
	
	NOT_NULL(m_vertexData);

	// get the VertexData index for the new vertex.
	// vertex data index = offset of vertex data from beginning of vertex data array
	const size_t vertexDataIndex = m_vertexData->m_container.size();

	// associate the vertexDataIndex with the shaderVertexIndex
	perShaderData->m_vertexIndices.push_back(vertexDataIndex);

	//-- setup data for new vertex
	m_vertexData->m_container.resize(vertexDataIndex + 1);
	VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// setup position index
	vertexData.m_positionIndex = positionIndex;

	// set aside texture coordinate storage
	if (perShaderData->m_textureCoordinateSetCount > 0)
	{
		NOT_NULL(m_textureCoordinates);

		// Get our first texture coordinate index.
		vertexData.m_firstTextureCoordinateIndex = static_cast<int>(m_textureCoordinates->size());

		// Increase texture coordinate storage.
		// -TRF-: this code assumes we always add a vertex, then its texture coordinates, then a new vertex, then its texture coordinates.
		//        However, the interface does not imply this.  Consider changing interface so that you deal with an active vertex and
		//        can only add texture coordinates to the "current" vertex.
		const FloatVector::size_type newSize = static_cast<FloatVector::size_type>(vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateCount);
		m_textureCoordinates->resize(newSize);
	}
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setVertexNormal(PerShaderData *perShaderData, int shaderVertexIndex, int normalIndex)
{
	NOT_NULL(m_normals);
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);

	DEBUG_FATAL((normalIndex < 0) || (static_cast<size_t>(normalIndex) >= m_normals->m_container.size()), ("normalIndex arg [%d] out of valid range [0..%u]", normalIndex, m_normals->m_container.size()));

	// get vertexData index from shader vertex index
	DEBUG_FATAL((shaderVertexIndex < 0) || (static_cast<size_t>(shaderVertexIndex) >= perShaderData->m_vertexIndices.size()), ("shaderVertexIndex [%d] out of valid range [0..%u]", shaderVertexIndex, perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// get vertex data
	DEBUG_FATAL(vertexDataIndex >= m_vertexData->m_container.size(), ("vertexDataIndex [%u] out of valid range [0..%u]", m_vertexData->m_container.size()));
	VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// set data
	vertexData.m_normalIndex = normalIndex;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setVertexColor(PerShaderData *perShaderData, int shaderVertexIndex, const PackedArgb &color)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);

	// get vertexData index from shader vertex index
	DEBUG_FATAL((shaderVertexIndex < 0) || (static_cast<size_t>(shaderVertexIndex) >= perShaderData->m_vertexIndices.size()), ("shaderVertexIndex [%d] out of valid range [0..%u]", shaderVertexIndex, perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// get vertex data
	DEBUG_FATAL(vertexDataIndex >= m_vertexData->m_container.size(), ("vertexDataIndex [%u] out of valid range [0..%u]", m_vertexData->m_container.size()));
	VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// set data
	vertexData.m_diffuseColor = color;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinates(PerShaderData *perShaderData, int shaderVertexIndex, int tcSetIndex, float tc0)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, tcSetIndex, perShaderData->m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[tcSetIndex] != 1, ("tried to provide 1 texture coordinates for tc set index [%d] that accepts [%d].", tcSetIndex, perShaderData->m_textureCoordinateDimensionality[tcSetIndex]));

	// Get vertexData index from shader vertex index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, static_cast<int>(perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// Get vertex data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(vertexDataIndex), static_cast<int>(m_vertexData->m_container.size()));
	const VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// Get texture coordinate index.
	DEBUG_FATAL(vertexData.m_firstTextureCoordinateIndex < 0, ("invalid: m_firstTextureCoordinateIndex [%d] < 0", vertexData.m_firstTextureCoordinateIndex));
	const size_t tcDataIndex = static_cast<size_t>(vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[tcSetIndex]);

	// Set texture coordinate data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcDataIndex), static_cast<int>(m_textureCoordinates->size()));

	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 0)] = tc0;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinates(PerShaderData *perShaderData, int shaderVertexIndex, int tcSetIndex, float tc0, float tc1)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, tcSetIndex, perShaderData->m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[tcSetIndex] != 2, ("tried to provide 2 texture coordinates for tc set index [%d] that accepts [%d].", tcSetIndex, perShaderData->m_textureCoordinateDimensionality[tcSetIndex]));

	// Get vertexData index from shader vertex index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, static_cast<int>(perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// Get vertex data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(vertexDataIndex), static_cast<int>(m_vertexData->m_container.size()));
	const VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// Get texture coordinate index.
	DEBUG_FATAL(vertexData.m_firstTextureCoordinateIndex < 0, ("invalid: m_firstTextureCoordinateIndex [%d] < 0", vertexData.m_firstTextureCoordinateIndex));
	const size_t tcDataIndex = static_cast<size_t>(vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[tcSetIndex]);

	// Set texture coordinate data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcDataIndex + 1), static_cast<int>(m_textureCoordinates->size()));

	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 0)] = tc0;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 1)] = tc1;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinates(PerShaderData *perShaderData, int shaderVertexIndex, int tcSetIndex, float tc0, float tc1, float tc2)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, tcSetIndex, perShaderData->m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[tcSetIndex] != 3, ("tried to provide 3 texture coordinates for tc set index [%d] that accepts [%d].", tcSetIndex, perShaderData->m_textureCoordinateDimensionality[tcSetIndex]));

	// Get vertexData index from shader vertex index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, static_cast<int>(perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// Get vertex data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(vertexDataIndex), static_cast<int>(m_vertexData->m_container.size()));
	const VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// Get texture coordinate index.
	DEBUG_FATAL(vertexData.m_firstTextureCoordinateIndex < 0, ("invalid: m_firstTextureCoordinateIndex [%d] < 0", vertexData.m_firstTextureCoordinateIndex));
	const size_t tcDataIndex = static_cast<size_t>(vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[tcSetIndex]);

	// Set texture coordinate data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcDataIndex + 2), static_cast<int>(m_textureCoordinates->size()));

	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 0)] = tc0;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 1)] = tc1;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 2)] = tc2;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::setTextureCoordinates(PerShaderData *perShaderData, int shaderVertexIndex, int tcSetIndex, float tc0, float tc1, float tc2, float tc3)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, tcSetIndex, perShaderData->m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[tcSetIndex] != 4, ("tried to provide 4 texture coordinates for tc set index [%d] that accepts [%d].", tcSetIndex, perShaderData->m_textureCoordinateDimensionality[tcSetIndex]));

	// Get vertexData index from shader vertex index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, static_cast<int>(perShaderData->m_vertexIndices.size()));
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	
	// Get vertex data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(vertexDataIndex), static_cast<int>(m_vertexData->m_container.size()));
	const VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];

	// Get texture coordinate index.
	DEBUG_FATAL(vertexData.m_firstTextureCoordinateIndex < 0, ("invalid: m_firstTextureCoordinateIndex [%d] < 0", vertexData.m_firstTextureCoordinateIndex));
	const size_t tcDataIndex = static_cast<size_t>(vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[tcSetIndex]);

	// Set texture coordinate data.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(tcDataIndex + 3), static_cast<int>(m_textureCoordinates->size()));

	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 0)] = tc0;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 1)] = tc1;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 2)] = tc2;
	(*m_textureCoordinates)[static_cast<FloatVector::size_type>(tcDataIndex + 3)] = tc3;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::addTriStrip(PerShaderData *perShaderData, bool flipCullMode)
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_triStripHeaders);
	NOT_NULL(m_triStripIndices);

	// get vertexTriStripIndex
	const int vertexTriStripIndex = static_cast<int>(perShaderData->m_triStripIndices.size());

	// get first available index in the triStripIndex container
	const int firstTriStripIndex = static_cast<int>(m_triStripIndices->m_container.size());

	// create TriStripHeader
	const size_t triStripHeaderIndex = m_triStripHeaders->m_container.size();
	m_triStripHeaders->m_container.push_back(TriStripHeader(0, firstTriStripIndex, flipCullMode));

	// associate TriStripHeader with PerShaderData (goes in vertexTriStripIndex slot)
	perShaderData->m_triStripIndices.push_back(triStripHeaderIndex);

	return vertexTriStripIndex;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::addTriStripVertex(PerShaderData *perShaderData, int triStripIndex, int shaderVertexIndex)
{
	NOT_NULL(m_triStripHeaders);
	NOT_NULL(m_triStripIndices);
	NOT_NULL(m_vertexData);

	NOT_NULL(perShaderData);
	DEBUG_FATAL((triStripIndex < 0) || (static_cast<size_t>(triStripIndex) >= perShaderData->m_triStripIndices.size()), ("triStripIndex [%d] out of valid range [0..%u)", triStripIndex, perShaderData->m_triStripIndices.size()));
	DEBUG_FATAL((shaderVertexIndex < 0) || (static_cast<size_t>(shaderVertexIndex) >= perShaderData->m_vertexIndices.size()), ("shaderVertexIndex [%d] out of valid range [0..%u)", shaderVertexIndex, perShaderData->m_vertexIndices.size()));

	// get the TriStripHeader index
	const size_t triStripHeaderIndex = perShaderData->m_triStripIndices[static_cast<size_t>(triStripIndex)];

	// get the TriStripHeader
	DEBUG_FATAL(triStripHeaderIndex >= m_triStripHeaders->m_container.size(), ("triStripHeaderIndex [%u] out of valid range [0..%u)", triStripHeaderIndex, m_triStripHeaders->m_container.size()));
	TriStripHeader &triStripHeader   = m_triStripHeaders->m_container[triStripHeaderIndex];

#ifdef _DEBUG
	// make sure caller hasn't been interleaving the strips to which vertices are added --- we don't support that
	const size_t expectedSize = static_cast<size_t>(triStripHeader.m_firstTriStripIndex + triStripHeader.m_indexCount);
	DEBUG_FATAL(m_triStripIndices->m_container.size() != expectedSize, ("unsupported: caller has been interleaving addTriStripVertex() calls among different strips [%u/%u]", m_triStripIndices->m_container.size(), expectedSize));
#endif

	// add shader vertex index to strip
	m_triStripIndices->m_container.push_back(static_cast<size_t>(shaderVertexIndex));
	++triStripHeader.m_indexCount;

	// mark this shader vertex data as used (referenced) so we do not throw it away
	const size_t vertexDataIndex = perShaderData->m_vertexIndices[static_cast<size_t>(shaderVertexIndex)];
	DEBUG_FATAL(vertexDataIndex >= m_vertexData->m_container.size(), ("vertexDataIndex [%u] out of valid range [0..%u)", vertexDataIndex, m_vertexData->m_container.size()));

	VertexData &vertexData = m_vertexData->m_container[vertexDataIndex];
	vertexData.m_isUsed    = true;

	// assume this primitive is weighted
	++perShaderData->m_numberOfWeightedPrimitiveIndices;
}

// ------------------------------------------------------------------

MeshConstructionHelper::TriListHeader &MeshConstructionHelper::addTriList(PerShaderData *perShaderData)
{
	NOT_NULL(perShaderData);

	NOT_NULL(m_triListHeaders);
	NOT_NULL(m_indexedTriangles);

	// allocate the new tri list header
	const size_t triListHeaderIndex = m_triListHeaders->m_container.size();
	m_triListHeaders->m_container.resize(triListHeaderIndex+1);

	// associate this tri list header with the perShaderData
	perShaderData->m_triListIndices.push_back(triListHeaderIndex);

	// setup first indexed tri
	TriListHeader &triListHeader        = m_triListHeaders->m_container[triListHeaderIndex];
	triListHeader.m_firstTriangleIndex  = m_indexedTriangles->size() / 3;
	triListHeader.m_triangleCount       = 0;

	return triListHeader;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::addTriListTri(PerShaderData *perShaderData, TriListHeader &triList, int indexV0, int indexV1, int indexV2)
{
	NOT_NULL(perShaderData);

	// make sure caller isn't adding tris to multiple tri lists in an
	// interleaved fashion
	NOT_NULL(m_indexedTriangles);
	NOT_NULL(m_vertexData);

#ifdef _DEBUG
	const size_t expectedSize = triList.m_firstTriangleIndex + static_cast<size_t>(triList.m_triangleCount);
	DEBUG_FATAL((m_indexedTriangles->size() / 3) != expectedSize, ("caller has been adding triangles "));
#endif

	// "allocate" the triangle	
	++triList.m_triangleCount;
	m_indexedTriangles->push_back(indexV0);
	m_indexedTriangles->push_back(indexV1);
	m_indexedTriangles->push_back(indexV2);

	// mark shader vertices as "used"
	VertexData &vd0 = m_vertexData->m_container[ perShaderData->m_vertexIndices[static_cast<size_t>(indexV0)] ];
	vd0.m_isUsed    = true;

	VertexData &vd1 = m_vertexData->m_container[ perShaderData->m_vertexIndices[static_cast<size_t>(indexV1)] ];
	vd1.m_isUsed    = true;

	VertexData &vd2 = m_vertexData->m_container[ perShaderData->m_vertexIndices[static_cast<size_t>(indexV2)] ];
	vd2.m_isUsed    = true;

	// assume the tri is weighted
	perShaderData->m_numberOfWeightedPrimitiveIndices += 3;
}

// ------------------------------------------------------------------

const MeshConstructionHelper::PerShaderData *MeshConstructionHelper::getPerShaderData(int shaderIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, m_usedPerShaderDataCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, static_cast<int>(m_perShaderData->m_container.size()));

	//-- we've got a good guess as to where in the array this shader index exists, but we still need to
	//   hunt for it.  Any shader templates with no primitives are marked as unused.
	PerShaderDataContainer::Container::const_iterator itEnd = m_perShaderData->m_container.end();
	PerShaderDataContainer::Container::const_iterator it    = m_perShaderData->m_container.begin();
	std::advance(it, shaderIndex);

	for (; it != itEnd; ++it)
	{
		const PerShaderData *const psd = NON_NULL(it->second);
		if (psd->m_usedIndex == shaderIndex)
			return psd;
	}

	DEBUG_FATAL(true, ("failed to find used perShaderData %d", shaderIndex));
	return 0; //lint !e527 // unreachable // in debug
}

// ------------------------------------------------------------------

const CrcString &MeshConstructionHelper::getShaderTemplateName(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_shaderTemplateName;
}

// ------------------------------------------------------------------

uint MeshConstructionHelper::getVertexFormat(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_vertexFormat;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTextureCoordinateSets(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_textureCoordinateSetCount;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTextureCoordinateDimensionality(const PerShaderData *perShaderData, int setIndex) const
{
	NOT_NULL(perShaderData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData->m_textureCoordinateSetCount);

	return perShaderData->m_textureCoordinateDimensionality[setIndex];
}

// ----------------------------------------------------------------------

int MeshConstructionHelper::getNumberOfTransformWeightPairs(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_numberOfUsedTransformWeightPairs;
}

// ----------------------------------------------------------------------

int MeshConstructionHelper::getNumberOfWeightedPrimitiveIndices(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_numberOfWeightedPrimitiveIndices;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getVertexCount(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return perShaderData->m_usedVertexCount;
}

// ------------------------------------------------------------------

const MeshConstructionHelper::VertexData &MeshConstructionHelper::getVertexData(const PerShaderData *perShaderData, int shaderVertexIndex) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_vertexData);

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, perShaderData->m_usedVertexCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderVertexIndex, static_cast<int>(perShaderData->m_vertexIndices.size()));

	const size_t endIndex = perShaderData->m_vertexIndices.size();
	for (size_t searchIndex = static_cast<size_t>(shaderVertexIndex); searchIndex < endIndex; ++searchIndex)
	{
		const size_t  targetVertexDataIndex = perShaderData->m_vertexIndices[searchIndex];
		VertexData   &vertexData            = m_vertexData->m_container[targetVertexDataIndex];

		if (vertexData.m_shaderVertexIndex == shaderVertexIndex)
		{
			// found it
			return vertexData;
		}
	}

	FATAL(true, ("failed to find used shaderVertexIndex [%d]", shaderVertexIndex));
	return m_vertexData->m_container[0]; //lint !e527 // unreachable // yes
}

// ------------------------------------------------------------------

const Vector &MeshConstructionHelper::getPosition(const VertexData &vertexData) const
{
	NOT_NULL(m_positions);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, vertexData.m_positionIndex, static_cast<int>(m_positions->m_container.size()) );
	
	return m_positions->m_container[static_cast<size_t>(vertexData.m_positionIndex)];
}

// ------------------------------------------------------------------

const Vector &MeshConstructionHelper::getNormal(const VertexData &vertexData) const
{
	NOT_NULL(m_normals);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, vertexData.m_normalIndex, static_cast<int>(m_normals->m_container.size()) );
	
	return m_normals->m_container[static_cast<size_t>(vertexData.m_normalIndex)];
}

// ------------------------------------------------------------------

const PackedArgb &MeshConstructionHelper::getDiffuseColor(const VertexData &vertexData) const
{
	return vertexData.m_diffuseColor;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::getTextureCoordinates(const PerShaderData *perShaderData, const VertexData &vertexData, int textureCoordinateSet, float &tc0) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_textureCoordinates);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, textureCoordinateSet, perShaderData->m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet], static_cast<int>(m_textureCoordinates->size()));
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[textureCoordinateSet] != 1, ("tc set %d is not 1d.", textureCoordinateSet));

	const int firstTextureCoordinateIndex = vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet];

	tc0 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 0)];
}

// ------------------------------------------------------------------

void MeshConstructionHelper::getTextureCoordinates(const PerShaderData *perShaderData, const VertexData &vertexData, int textureCoordinateSet, float &tc0, float &tc1) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_textureCoordinates);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, textureCoordinateSet, perShaderData->m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet] + 1, static_cast<int>(m_textureCoordinates->size()));
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[textureCoordinateSet] != 2, ("tc set %d is not 2d.", textureCoordinateSet));

	const int firstTextureCoordinateIndex = vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet];

	tc0 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 0)];
	tc1 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 1)];
}

// ------------------------------------------------------------------

void MeshConstructionHelper::getTextureCoordinates(const PerShaderData *perShaderData, const VertexData &vertexData, int textureCoordinateSet, float &tc0, float &tc1, float &tc2) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_textureCoordinates);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, textureCoordinateSet, perShaderData->m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet] + 2, static_cast<int>(m_textureCoordinates->size()));
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[textureCoordinateSet] != 3, ("tc set %d is not 3d.", textureCoordinateSet));

	const int firstTextureCoordinateIndex = vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet];

	tc0 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 0)];
	tc1 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 1)];
	tc2 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 2)];
}

// ------------------------------------------------------------------

void MeshConstructionHelper::getTextureCoordinates(const PerShaderData *perShaderData, const VertexData &vertexData, int textureCoordinateSet, float &tc0, float &tc1, float &tc2, float &tc3) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_textureCoordinates);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, textureCoordinateSet, perShaderData->m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet] + 3, static_cast<int>(m_textureCoordinates->size()));
	DEBUG_FATAL(perShaderData->m_textureCoordinateDimensionality[textureCoordinateSet] != 4, ("tc set %d is not 4d.", textureCoordinateSet));

	const int firstTextureCoordinateIndex = vertexData.m_firstTextureCoordinateIndex + perShaderData->m_textureCoordinateStartOffset[textureCoordinateSet];

	tc0 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 0)];
	tc1 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 1)];
	tc2 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 2)];
	tc3 = (*m_textureCoordinates)[static_cast<FloatVector::size_type>(firstTextureCoordinateIndex + 3)];
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getVertexTransformCount(const VertexData &vertexData) const
{
	NOT_NULL(m_transformHeaders);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, vertexData.m_positionIndex, static_cast<int>(m_transformHeaders->m_container.size()) );
	
	const TransformHeader &transformHeader = m_transformHeaders->m_container[static_cast<size_t>(vertexData.m_positionIndex)];
	return transformHeader.m_transformCount;
}

// ------------------------------------------------------------------

const MeshConstructionHelper::TransformData &MeshConstructionHelper::getVertexTransformData(const VertexData &vertexData, int whichTransform) const
{
	NOT_NULL(m_transformData);
	NOT_NULL(m_transformHeaders);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, vertexData.m_positionIndex, static_cast<int>(m_transformHeaders->m_container.size()) );
	
	const TransformHeader &transformHeader = m_transformHeaders->m_container[static_cast<size_t>(vertexData.m_positionIndex)];
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, transformHeader.m_firstTransformDataIndex, static_cast<int>(m_transformData->m_container.size()) );
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, whichTransform, transformHeader.m_transformCount );

	const size_t transformDataIndex = static_cast<size_t>(transformHeader.m_firstTransformDataIndex + whichTransform);
	return m_transformData->m_container[transformDataIndex];
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTriStripCount(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);

	return static_cast<int>(perShaderData->m_triStripIndices.size());
}

// ------------------------------------------------------------------

const MeshConstructionHelper::TriStripHeader &MeshConstructionHelper::getTriStrip(const PerShaderData *perShaderData, int triStripIndex) const
{
	NOT_NULL(perShaderData);
	NOT_NULL(m_triStripHeaders);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0, triStripIndex, static_cast<int>(perShaderData->m_triStripIndices.size()) );

	const size_t triStripHeaderIndex = perShaderData->m_triStripIndices[static_cast<size_t>(triStripIndex)];
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE( 0U, triStripHeaderIndex, m_triStripHeaders->m_container.size() ); //lint !e1703

	return m_triStripHeaders->m_container[triStripHeaderIndex];
}

// ------------------------------------------------------------------

bool MeshConstructionHelper::getTriStripCullModeFlipped(const TriStripHeader &triStripHeader) const
{
	return triStripHeader.m_flipCullMode;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTriStripVertexCount(const TriStripHeader &triStripHeader) const
{
	return triStripHeader.m_indexCount;
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTriStripVertexIndex(const TriStripHeader &triStripHeader, int vertexIndex) const
{
	NOT_NULL(m_triStripIndices);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexIndex, triStripHeader.m_indexCount);

	const size_t indexIntoTriStripIndex = static_cast<size_t>(triStripHeader.m_firstTriStripIndex + vertexIndex);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0U, indexIntoTriStripIndex, m_triStripIndices->m_container.size() ); //lint !e1703

	return static_cast<int>(m_triStripIndices->m_container[indexIntoTriStripIndex]);
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTriListCount(const PerShaderData *perShaderData) const
{
	NOT_NULL(perShaderData);
	return static_cast<int>(perShaderData->m_triListIndices.size());
}

// ------------------------------------------------------------------

const MeshConstructionHelper::TriListHeader &MeshConstructionHelper::getTriList(const PerShaderData *perShaderData, int triListIndex) const
{
	NOT_NULL(m_triListHeaders);
	NOT_NULL(perShaderData);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, triListIndex, static_cast<int>(perShaderData->m_triListIndices.size()));

	const size_t triListHeaderIndex = perShaderData->m_triListIndices[static_cast<size_t>(triListIndex)];
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0U, triListHeaderIndex, m_triListHeaders->m_container.size()); //lint !e1703 // template arbitrarily selected

	return m_triListHeaders->m_container[triListHeaderIndex];
}

// ------------------------------------------------------------------

int MeshConstructionHelper::getTriListTriangleCount(const TriListHeader &triListHeader) const
{
	return triListHeader.m_triangleCount;
}

// ------------------------------------------------------------------

void MeshConstructionHelper::getTriListTriangle(const PerShaderData *perShaderData, const TriListHeader &triListHeader, int triIndex, int &index0, int &index1, int &index2) const
{
	//-- retrieve triangle index
	NOT_NULL(m_indexedTriangles);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, triIndex, triListHeader.m_triangleCount);

	const size_t indexedTriangleIndex = triListHeader.m_firstTriangleIndex + static_cast<size_t>(triIndex);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0U, indexedTriangleIndex, m_indexedTriangles->size() / 3); //lint !e1703 // template arbitrarily selected

	//const IndexedTriangle &triangle = (*m_indexedTriangles)[indexedTriangleIndex];

	//-- return fixed-up indices (tri indices were submitted assuming nothing was dropped out)
	NOT_NULL(perShaderData);

	const size_t  vertexDataIndex0 = perShaderData->m_vertexIndices[static_cast<size_t>((*m_indexedTriangles)[indexedTriangleIndex * 3 + 0])];
	index0                         = m_vertexData->m_container[vertexDataIndex0].m_shaderVertexIndex;

	const size_t  vertexDataIndex1 = perShaderData->m_vertexIndices[static_cast<size_t>((*m_indexedTriangles)[indexedTriangleIndex * 3 + 1])];
	index1                         = m_vertexData->m_container[vertexDataIndex1].m_shaderVertexIndex;

	const size_t  vertexDataIndex2 = perShaderData->m_vertexIndices[static_cast<size_t>((*m_indexedTriangles)[indexedTriangleIndex * 3 + 2])];
	index2                         = m_vertexData->m_container[vertexDataIndex2].m_shaderVertexIndex;
}

// ----------------------------------------------------------------------

MeshConstructionHelper::PerTextureRendererData *MeshConstructionHelper::addTextureRendererTemplate(const CrcLowerString &textureRendererTemplateName)
{
	// -TRF- come back to this, implement as sorted vectors.  duplicates are fine at add time.  do sort and unique at prepareForReading() time.

	PTRDContainer::iterator it = m_perTextureRendererData->find(&textureRendererTemplateName);
	if (it != m_perTextureRendererData->end())
	{
		// found it
		return it->second;
	}

	//-- create it
	PerTextureRendererData *ptrd = new PerTextureRendererData(textureRendererTemplateName);

	//-- add to map
	const std::pair<PTRDContainer::iterator, bool> insertResult = m_perTextureRendererData->insert(PTRDContainer::value_type(&ptrd->m_name, ptrd));
	DEBUG_FATAL(!insertResult.second, ("find and insert failed for [%s]", textureRendererTemplateName.getString()));
	UNREF(insertResult);

	return ptrd;
}

// ----------------------------------------------------------------------

void MeshConstructionHelper::addAffectedShaderTemplate(PerTextureRendererData *perTextureRendererData, Tag shaderTextureTag, PerShaderData *perShaderData)
{
	NOT_NULL(perTextureRendererData);

	const AffectedShaderData affectedShaderData(perShaderData, shaderTextureTag);

	//-- make sure we don't duplicate the this entry
	ASDContainer &affectedShaders = perTextureRendererData->m_affectedShaders;

	ASDIterator it = std::find(affectedShaders.begin(), affectedShaders.end(), affectedShaderData);
	if (it == affectedShaders.end())
		affectedShaders.push_back(affectedShaderData);
} //lint !e1762 // function could be made const // logically non-const.

// ----------------------------------------------------------------------

const MeshConstructionHelper::PerTextureRendererData *MeshConstructionHelper::getPerTextureRendererData(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, m_usedPerTextureRendererDataCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_perTextureRendererData->size()));

	//-- we've got a good guess as to where in this PerTextureRendererData exists, but we still need to
	//   hunt for it.  Any PerTextureRendererData with all shader templates "removed" are themselves removed.
	PTRDContainer::const_iterator itEnd = m_perTextureRendererData->end();
	PTRDContainer::const_iterator it    = m_perTextureRendererData->begin();
	std::advance(it, index);

	for (; it != itEnd; ++it)
	{
		const PerTextureRendererData *const ptrd = NON_NULL(it->second);
		if (ptrd->m_usedIndex == index)
			return ptrd;
	}

	DEBUG_FATAL(true, ("failed to find used perTextureRendererData %d", index));
	return 0; //lint !e527 // unreachable // in debug
}

// ----------------------------------------------------------------------

const CrcLowerString &MeshConstructionHelper::getTextureRendererTemplateName(const PerTextureRendererData *perTextureRendererData) const
{
	NOT_NULL(perTextureRendererData);
	return perTextureRendererData->m_name;
}

// ----------------------------------------------------------------------

int MeshConstructionHelper::getAffectedShaderCount(const PerTextureRendererData *perTextureRendererData) const
{
	NOT_NULL(perTextureRendererData);
	return perTextureRendererData->m_usedAffectedShaderCount;
}

// ----------------------------------------------------------------------

void MeshConstructionHelper::getAffectedShaderData(const PerTextureRendererData *perTextureRendererData, int index, int *shaderIndex, Tag *shaderTextureTag) const
{
	NOT_NULL(perTextureRendererData);
	NOT_NULL(shaderIndex);
	NOT_NULL(shaderTextureTag);

	const ASDContainer &asdContainer = perTextureRendererData->m_affectedShaders;

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, perTextureRendererData->m_usedAffectedShaderCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(asdContainer.size()));

	ASDConstIterator itEnd = asdContainer.end();
	for (ASDConstIterator it = asdContainer.begin() + index; it != itEnd; ++it)
	{
		const AffectedShaderData &asd = *it;
		if (asd.m_usedIndex == index)
		{
			*shaderIndex      = asd.m_affectedShader->m_usedIndex;
			*shaderTextureTag = asd.m_shaderTextureTag;
		}
	}
}
// ==================================================================
