//===================================================================
//
// ClientTerrainSorter.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientTerrainSorter.h"

#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SystemIndexBuffer.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Transform.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"

#include <bitset>
#include <algorithm>
#include <vector>
#include <map>

//===================================================================

static const int MAXIMUM_VERTEXBUFFER_SIZE = 256 * 1024;
static const int s_maximumNumberOfTextureCoordinates = 4 + 1;

//===================================================================

#ifdef _DEBUG
bool                                         ClientTerrainSorter::ms_installed;
bool                                         ClientTerrainSorter::ms_debugReport;
#endif

ClientTerrainSorter::FvfList*                ClientTerrainSorter::ms_fvfList;
ClientTerrainSorter::FvfMetricsList*         ClientTerrainSorter::ms_fvfMetricsList;
ClientTerrainSorter::PrimitiveListCache*     ClientTerrainSorter::ms_primitiveListCache;
ClientTerrainSorter::PrimitiveNodeListCache* ClientTerrainSorter::ms_primitiveNodeListCache;
ShaderPrimitiveSorter::LightBitSet           ClientTerrainSorter::ms_lightBitSet;

namespace ClientTerrainSorterNamespace
{
	int const VERTEX_INDIRECTION_ARRAY_SIZE = 9;

	struct VertexIndirectionData
	{
		int   x;
		int   z;
		float u;
		float v;
	};

	VertexIndirectionData const constructionIndirectionArray [VERTEX_INDIRECTION_ARRAY_SIZE] =
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

	VertexBufferFormat const getVertexBufferFormat(int const numberOfTextureCoordinateSets)
	{
		VertexBufferFormat format;
		format.setPosition();
		format.setNormal();
		format.setColor0();
		format.setNumberOfTextureCoordinateSets(numberOfTextureCoordinateSets);

		for (int i = 0; i < numberOfTextureCoordinateSets; ++i)
			format.setTextureCoordinateSetDimension(i, 2);

		return format;
	}

	DynamicIndexBuffer * ms_dynamicIndexBuffer;
}
using namespace ClientTerrainSorterNamespace;

//===================================================================

class ClientTerrainSorter::FvfMetrics
{
public:

	int numberOfShaders;           //lint !e1925  // public data member
	int numberOfVertices;          //lint !e1925  // public data member
	int numberOfTriangles;         //lint !e1925  // public data member
	int numberOfInputPrimitives;   //lint !e1925  // public data member
	int numberOfOutputPrimitives;  //lint !e1925  // public data member
	int vertexSize;                //lint !e1925  // public data member

public:

	FvfMetrics ();

	void reset ();
};

//-------------------------------------------------------------------

ClientTerrainSorter::FvfMetrics::FvfMetrics () :
	numberOfShaders (0),
	numberOfVertices (0),
	numberOfTriangles (0),
	numberOfInputPrimitives (0),
	numberOfOutputPrimitives (0),
	vertexSize (0)
{
}

//-------------------------------------------------------------------

void ClientTerrainSorter::FvfMetrics::reset ()
{
	numberOfShaders          = 0;
	numberOfVertices         = 0;
	numberOfTriangles        = 0;
	numberOfInputPrimitives  = 0;
	numberOfOutputPrimitives = 0;
	vertexSize               = 0;
}

//===================================================================

class ClientTerrainSorter::PrimitiveNode : public ShaderPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	PrimitiveNode (const Shader* shader, FvfMetrics* fvfMetrics);
	virtual ~PrimitiveNode ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

public:

	//-- accessed by the terrain sorter
	int                vertexBufferSize;
	VertexBufferFormat vertexBufferFormat;
	int                numberOfVertices;
	int                numberOfIndices;
	PrimitiveList*     primitiveList;

private:

	PrimitiveNode ();
	PrimitiveNode (const PrimitiveNode& rhs);             //lint -esym(754, PrimitiveNode::PrimitiveNode)
	PrimitiveNode& operator= (const PrimitiveNode& rhs);  //lint -esym(754, PrimitiveNode::operator=)

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:

	const Shader* m_shader;
	FvfMetrics*   m_fvfMetrics;
};

//-------------------------------------------------------------------

MemoryBlockManager* ClientTerrainSorter::PrimitiveNode::ms_memoryBlockManager;

//-------------------------------------------------------------------

void ClientTerrainSorter::PrimitiveNode::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientTerrainSorter::PrimitiveNode::install\n"));
	DEBUG_FATAL (ms_memoryBlockManager, ("ClientTerrainSorter::PrimitiveNode already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("ClientTerrainSorter::PrimitiveNode::ms_memoryBlockManager", false, sizeof(PrimitiveNode), 200, 1, 0);
}

//-------------------------------------------------------------------

void ClientTerrainSorter::PrimitiveNode::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("ClientTerrainSorter::PrimitiveNode is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* ClientTerrainSorter::PrimitiveNode::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (PrimitiveNode), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void ClientTerrainSorter::PrimitiveNode::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

ClientTerrainSorter::PrimitiveNode::PrimitiveNode (const Shader* const shader, FvfMetrics* const fvfMetrics) :
	ShaderPrimitive (),
	vertexBufferSize (0),
	vertexBufferFormat (),
	numberOfVertices (0),
	numberOfIndices (0),
	primitiveList (0),
	m_shader (shader),
	m_fvfMetrics (fvfMetrics)
{
		m_shader->fetch();

		m_shader->prepareToView();
}

//-------------------------------------------------------------------
	
ClientTerrainSorter::PrimitiveNode::~PrimitiveNode ()
{
	primitiveList = 0;

	m_shader->release();
	m_shader = 0;

	primitiveList = 0;
	m_fvfMetrics = 0;
}

//-------------------------------------------------------------------

const Vector ClientTerrainSorter::PrimitiveNode::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ClientTerrainSorter::PrimitiveNode::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ClientTerrainSorter::PrimitiveNode::getVertexBufferSortKey() const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ClientTerrainSorter::PrimitiveNode::prepareToView () const
{
	return m_shader->prepareToView ();
}

//-------------------------------------------------------------------

void ClientTerrainSorter::PrimitiveNode::prepareToDraw () const
{
	DynamicVertexBuffer vertexBuffer (vertexBufferFormat);

	vertexBuffer.lock (numberOfVertices);
	ms_dynamicIndexBuffer->lock (numberOfIndices);

		Index *ii = ms_dynamicIndexBuffer->begin();

		{
			NOT_NULL (primitiveList);

			int vertexBufferOffset = 0;

			VertexBufferWriteIterator vi = vertexBuffer.begin();

			uint i;
			for (i = 0; i < primitiveList->size (); i++)
			{
				const ClientProceduralTerrainAppearance::ShaderSet::Primitive* const primitive = (*primitiveList) [i];

				//-- Copy vertex buffer data
				Vector const * const positionList = primitive->getPositionList();
				Vector const * const normalList = primitive->getNormalList();
				PackedArgb const * const colorList = primitive->getColorList();				
				float const uvScale = primitive->getBaseUvScale();

				{
					for (int j = 0; j < 9; ++j)
					{
						VertexIndirectionData const & vid = constructionIndirectionArray[j];

						vi.setPosition(positionList[j]);
						vi.setNormal(normalList[j]);
						vi.setColor0(colorList[j]);

						int uvset = 0;
						vi.setTextureCoordinates(uvset++, vid.u * uvScale, vid.v * uvScale);

						int const numberOfTextureCoordinateSets = vertexBufferFormat.getNumberOfTextureCoordinateSets();
						for (int k = 1; k < numberOfTextureCoordinateSets; ++k)
						{
							float u = 0.0f;
							float v = 0.0f;
							ClientProceduralTerrainAppearance::RotationType const rotationType = primitive->getRotationType()[k];
							primitive->getUv(rotationType, j, u, v);
							vi.setTextureCoordinates(uvset++, u, v);
						}

						++vi;
					}
				}

				//-- Copy index buffer data
				const SystemIndexBuffer* const primitiveIndexBuffer     = primitive->getIndexBuffer ();
				const Index*                   primitiveIndices         = primitiveIndexBuffer->beginReadOnly ();
				const int                      primitiveNumberOfIndices = primitiveIndexBuffer->getNumberOfIndices ();

				{
					int k;
					for (k = 0; k < primitiveNumberOfIndices; k++)
						*ii++ = static_cast<Index>(vertexBufferOffset + *primitiveIndices++);
				}
				
				//-- advance
				vertexBufferOffset += 9;
			}
		}

	ms_dynamicIndexBuffer->unlock();
	vertexBuffer.unlock ();

	//-- set the vb and ib once per primitive
	Graphics::setObjectToWorldTransformAndScale (TerrainObject::getConstInstance ()->getAppearance ()->getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (vertexBuffer);
	Graphics::setIndexBuffer(*ms_dynamicIndexBuffer);

#ifdef _DEBUG
	m_fvfMetrics->numberOfVertices        += numberOfVertices;
	m_fvfMetrics->numberOfTriangles       += numberOfIndices / 3;
	m_fvfMetrics->numberOfInputPrimitives += primitiveList->size ();
	m_fvfMetrics->vertexSize               = vertexBuffer.getVertexSize ();
#endif
}

//-------------------------------------------------------------------

void ClientTerrainSorter::PrimitiveNode::draw () const
{
#ifdef _DEBUG
	++m_fvfMetrics->numberOfOutputPrimitives;
#endif

	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientTerrainSorter::PrimitiveNode::draw");

	Graphics::drawIndexedTriangleList ();
}

//===================================================================

void ClientTerrainSorter::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientTerrainSorter::install\n"));

#if _DEBUG
	DEBUG_FATAL (ms_installed, ("ClientTerrainSorter::install - already installed"));
	ms_installed = true;
#endif

	//-- allocate memblocks
	PrimitiveNode::install ();

	//-- allocate the sorter
	{
		ms_fvfList = new FvfList;
		ms_fvfList->reserve (s_maximumNumberOfTextureCoordinates);

		ms_fvfMetricsList = new FvfMetricsList;
		ms_fvfMetricsList->reserve (s_maximumNumberOfTextureCoordinates);

		int i;
		for (i = 0; i < s_maximumNumberOfTextureCoordinates; ++i)
		{
			ms_fvfList->push_back (new ShaderMap ());
			ms_fvfMetricsList->push_back (new FvfMetrics);
		}
	}

	//-- allocate caches
	{
		ms_primitiveListCache = new PrimitiveListCache;

		int i;
		for (i = 0; i < 100; ++i)
			ms_primitiveListCache->push_back (new PrimitiveList);
	}

	{
		ms_primitiveNodeListCache = new PrimitiveNodeListCache;

		int i;
		for (i = 0; i < 100; ++i)
			ms_primitiveNodeListCache->push_back (new PrimitiveNodeList);
	}

	ms_dynamicIndexBuffer = new DynamicIndexBuffer();

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReport, "ClientTerrain", "reportClientTerrainSorter", debugReport);
#endif
}

//-------------------------------------------------------------------

void ClientTerrainSorter::remove ()
{
#if _DEBUG
	DEBUG_FATAL (!ms_installed, ("ClientTerrainSorter::remove - not installed"));
	ms_installed = false;
#endif

	delete ms_dynamicIndexBuffer;
	ms_dynamicIndexBuffer = 0;

	clear ();

	//-- remove caches
	{
		while (!ms_primitiveNodeListCache->empty ())
		{
			delete ms_primitiveNodeListCache->back ();
			ms_primitiveNodeListCache->pop_back ();
		}

		delete ms_primitiveNodeListCache;
		ms_primitiveNodeListCache = 0;
	}

	{
		while (!ms_primitiveListCache->empty ())
		{
			delete ms_primitiveListCache->back ();
			ms_primitiveListCache->pop_back ();
		}

		delete ms_primitiveListCache;
		ms_primitiveListCache = 0;
	}

	//-- remove the sorter
	{
		uint i;
		for (i = 0; i < ms_fvfList->size (); ++i)
		{
			//-- delete the primitive node lists
			for (ShaderMap::iterator iter = (*ms_fvfList) [i]->begin (); iter != (*ms_fvfList) [i]->end (); ++iter)
				delete (*iter).second;

			//-- delete the shader maps
			delete (*ms_fvfList) [i];
			delete (*ms_fvfMetricsList) [i];
		}

		//-- delete the fvf list
		delete ms_fvfList;
		ms_fvfList = 0;

		delete ms_fvfMetricsList;
		ms_fvfMetricsList = 0;
	}

	//-- remove memblocks
	PrimitiveNode::remove ();

#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_debugReport);
#endif
}

//-------------------------------------------------------------------

void ClientTerrainSorter::clear ()
{
	//-- go through the fvf list
	uint i;
	for (i = 0; i < ms_fvfList->size (); ++i)
	{
		//-- clear the shader map
		ShaderMap* const shaderMap = (*ms_fvfList) [i];

		if (shaderMap->size ())
		{
			//-- go through the list of primitives for each shader for each fvf
			for (ShaderMap::iterator iter = shaderMap->begin (); iter != shaderMap->end (); ++iter)
			{
				PrimitiveNodeList* const primitiveNodeList = (*iter).second;

				//-- build a huge vb from the primitivelist
				uint j;
				for (j = 0; j < primitiveNodeList->size (); j++)
				{
					PrimitiveNode* const primitiveNode = (*primitiveNodeList) [j];

					//-- cache the primitive list 
					ms_primitiveListCache->push_back (primitiveNode->primitiveList);

					delete primitiveNode;
				}

				//-- clear the list for the next frame; cache the node list
				ms_primitiveNodeListCache->push_back (primitiveNodeList);
			}
		}

		shaderMap->clear ();
	}

	ms_lightBitSet.reset();
}

//-------------------------------------------------------------------

void ClientTerrainSorter::queue (const Shader* shader, const ClientProceduralTerrainAppearance::ShaderSet::Primitive* primitive)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientTerrainSorter::queue");
	
	if (ShaderPrimitiveSorter::getUseClipRectangle())
	{
		Rectangle2d const & extent2d = primitive->getExtent2d();

		if (!ShaderPrimitiveSorter::getClipRectangle().intersects(extent2d))
			return;
	}

	ms_lightBitSet |= ShaderPrimitiveSorter::getLightsAffectingShadersWithoutPrecalculatedVertexLighting();

	const SystemIndexBuffer* const indexBuffer   = primitive->getIndexBuffer ();

	//-- find out the fvf (number of uv sets)
	const int index            = primitive->getNumberOfTextureCoordinateSets();
	VertexBufferFormat const vertexBufferFormat = getVertexBufferFormat(index);
	const int vertexSize       = SystemVertexBuffer::getDescriptorVertexSize(vertexBufferFormat);
	const int numberOfVertices = 9;
	const int vertexBufferSize = vertexSize * numberOfVertices;
	const int numberOfIndices  = indexBuffer->getNumberOfIndices ();

	//-- locate the shader and primitiveList
	PrimitiveNodeList* primitiveNodeList = 0;

	ShaderMap* const shaderMap = (*ms_fvfList) [static_cast<uint> (index)];

	ShaderMap::iterator iter = shaderMap->find (shader);
	if (iter != shaderMap->end ())
		primitiveNodeList = (*iter).second;
	else
	{
		//-- create new shader primitive list; see if one exists in the cache
		if (!ms_primitiveNodeListCache->empty ())
		{
			primitiveNodeList = ms_primitiveNodeListCache->back ();
			ms_primitiveNodeListCache->pop_back ();

			primitiveNodeList->clear ();
		}
		else
			primitiveNodeList = new PrimitiveNodeList;

		IGNORE_RETURN (shaderMap->insert (ShaderMap::value_type (shader, primitiveNodeList)));
	}

	//-- see if we can fit in any existing nodes
	uint i;
	for (i = 0; i < primitiveNodeList->size (); i++)
	{
		PrimitiveNode* const primitiveNode = (*primitiveNodeList) [i];

		if (primitiveNode->vertexBufferSize + vertexBufferSize < MAXIMUM_VERTEXBUFFER_SIZE)
		{
			primitiveNode->primitiveList->push_back (primitive);
			primitiveNode->vertexBufferSize += vertexBufferSize;
			primitiveNode->numberOfVertices += numberOfVertices;
			primitiveNode->numberOfIndices  += numberOfIndices;

			break;
		}
	}

	if (i == primitiveNodeList->size ())
	{
		PrimitiveNode* const primitiveNode = new PrimitiveNode (shader, (*ms_fvfMetricsList) [index]);

		//-- see if a primitiveList exists in the cache
		if (!ms_primitiveListCache->empty ())
		{
			primitiveNode->primitiveList = ms_primitiveListCache->back ();
			ms_primitiveListCache->pop_back ();

			primitiveNode->primitiveList->clear ();
		}
		else
			primitiveNode->primitiveList  = new PrimitiveList;

		primitiveNode->vertexBufferSize   = numberOfVertices * vertexSize;
		primitiveNode->vertexBufferFormat = vertexBufferFormat;
		primitiveNode->numberOfVertices   = numberOfVertices;
		primitiveNode->numberOfIndices    = numberOfIndices;

		primitiveNode->primitiveList->push_back (primitive);

		primitiveNodeList->push_back (primitiveNode);
	}
}  //lint !e429  //primitiveNodeList has not been freed or returned

//-------------------------------------------------------------------

void ClientTerrainSorter::draw ()
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientTerrainSorter::draw");

	//-- go through the fvf list
	uint i;
	for (i = 0; i < ms_fvfList->size (); ++i)
	{
		ShaderMap* const shaderMap = (*ms_fvfList) [i];

		if (shaderMap->size ())
		{
			//-- go through the list of primitives for each shader for each fvf
			for (ShaderMap::iterator iter = shaderMap->begin (); iter != shaderMap->end (); ++iter)
			{
				PrimitiveNodeList* const primitiveNodeList = (*iter).second;

				//-- build a huge vb from the primitivelist
				uint j;
				for (j = 0; j < primitiveNodeList->size (); j++)
				{
					PrimitiveNode* const primitiveNode = (*primitiveNodeList) [j];

					ShaderPrimitiveSorter::add (*primitiveNode, ms_lightBitSet);
				}
			}
		}

#ifdef _DEBUG
		//-- set the size
		FvfMetrics* const metrics = (*ms_fvfMetricsList) [i];
		metrics->reset ();
		metrics->numberOfShaders = shaderMap->size ();
#endif
	}
}

//-------------------------------------------------------------------

void ClientTerrainSorter::debugReport ()
{
#ifdef _DEBUG
	//-- output statistics
	int totalNumberOfShaders          = 0;
	int totalNumberOfInputPrimitives  = 0;
	int totalNumberOfOutputPrimitives = 0;
	int totalGeometrySize             = 0;

	uint i;
	for (i = 0; i < ms_fvfMetricsList->size (); ++i)
	{
		if (i == 0)
			continue;

		FvfMetrics* const metrics = (*ms_fvfMetricsList) [i];

		DEBUG_REPORT_PRINT (ms_debugReport, ("[%i]  shaders=%3i  vs=%2i  gs=%6i  v=%6i  t=%6i  ip=%6i  op=%6i\n",
			i,
			metrics->numberOfShaders,
			metrics->vertexSize,
			metrics->vertexSize * metrics->numberOfVertices,
			metrics->numberOfVertices,
			metrics->numberOfTriangles,
			metrics->numberOfInputPrimitives,  
			metrics->numberOfOutputPrimitives));

		if (metrics->numberOfShaders)
		{
			totalNumberOfShaders          += metrics->numberOfShaders;
			totalNumberOfInputPrimitives  += metrics->numberOfInputPrimitives;
			totalNumberOfOutputPrimitives += metrics->numberOfOutputPrimitives;
			totalGeometrySize             += metrics->vertexSize * metrics->numberOfVertices;
		}

		metrics->reset ();
	}

	DEBUG_REPORT_PRINT (ms_debugReport, ("totalgs=%3iK  totals=%6i  totalip=%6i  totalop=%6i\n", 
		totalGeometrySize >> 10, 
		totalNumberOfShaders,
		totalNumberOfInputPrimitives, 
		totalNumberOfOutputPrimitives));

	static int s_maximumNumberOfShaders          = 0;
	static int s_maximumNumberOfInputPrimitives  = 0;
	static int s_maximumNumberOfOutputPrimitives = 0;
	static int s_maximumTotalGeometrySize        = 0;

	s_maximumNumberOfShaders          = std::max (s_maximumNumberOfShaders, totalNumberOfShaders);
	s_maximumNumberOfInputPrimitives  = std::max (s_maximumNumberOfInputPrimitives, totalNumberOfInputPrimitives);
	s_maximumNumberOfOutputPrimitives = std::max (s_maximumNumberOfOutputPrimitives, totalNumberOfOutputPrimitives);
	s_maximumTotalGeometrySize        = std::max (s_maximumTotalGeometrySize, totalGeometrySize);

	DEBUG_REPORT_PRINT (ms_debugReport, ("  maxgs=%3iK    maxs=%6i    maxip=%6i    maxop=%6i\n", 
		s_maximumTotalGeometrySize >> 10, 
		s_maximumNumberOfShaders,
		s_maximumNumberOfInputPrimitives, 
		s_maximumNumberOfOutputPrimitives));
#endif
}

//===================================================================
