// ======================================================================
//
// ShaderPrimitiveSetTemplate.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderPrimitiveSetTemplate.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderPrimitiveSet.def"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include "clientGraphics/SwitchShaderTemplate.h"
#include "clientGraphics/SystemIndexBuffer.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/VertexBuffer.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane.h"
#include "sharedMath/PositionVertexIndexer.h"

#include <algorithm>
#include <string>
#include <vector>

#define PROFILE_COLLISION 0

// ======================================================================

namespace ShaderPrimitiveSetTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_CNT = TAG3(C,N,T);
	Tag const TAG_SPS = TAG3(S,P,S);
	Tag const TAG_INDX = TAG(I,N,D,X);
	Tag const TAG_SIDX = TAG(S,I,D,X);

	float const cms_verticalThreshold = sin(convertDegreesToRadians(30.f));

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::vector<uint16> ms_upwardIndices;
	std::vector<uint16> ms_indexLists[9];
	std::vector<Vector> ms_normalLists[9];

	bool  ms_allowUpwardIndicesCreation;
	bool  ms_buildingAsynchronousLoaderData;
	bool  ms_collideAgainstAllGeometry;
	bool  ms_createCollisionDataForNonCollidablePrimitives;
	bool  ms_reportCollision;
	bool  ms_logCollisionSplit;
#if PROFILE_COLLISION == 1
	int   ms_collide1Calls = 0;
	float ms_collide1Time = 0.f;
	int   ms_collide1Primitives = 0;
	int   ms_collide1Hits = 0;
	int   ms_collide2Calls = 0;
	float ms_collide2Time = 0.f;
	int   ms_collide2Primitives = 0;
	int   ms_collide2Hits = 0;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//	+Y (looking down)
	//      1 0
	//      2 3
	//
	//	-Y
	//      5 4
	//      6 7

	int hashPosition(Vector const & center, Vector const & position)
	{
		if (position.x < center.x)
		{
			if (position.y < center.y)
			{
				if (position.z < center.z)
					return 6;
				else
					return 5;
			}
			else
			{
				if (position.z < center.z)
					return 2;
				else
					return 1;
			}
		}
		else
		{
			if (position.y < center.y)
			{
				if (position.z < center.z)
					return 7;
				else
					return 4;
			}
			else
			{
				if (position.z < center.z)
					return 3;
				else
					return 0;
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int hashTriangle(Vector const & center, Vector const & v0, Vector const & v1, Vector const & v2)
	{
		int const i0 = hashPosition(center, v0);
		int const i1 = hashPosition(center, v1);
		int const i2 = hashPosition(center, v2);

		if (i0 == i1 && i1 == i2)
			return i0;

		return 8;
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Index findOrAdd(std::vector<Vector> & vertexList, Vector const & position)
	{
		uint const numberOfVertices = static_cast<int>(vertexList.size());

		for (uint i = 0; i < numberOfVertices; ++i)
			if (vertexList[i] == position)
				return static_cast<Index>(i);

		vertexList.push_back(position);

		return static_cast<Index>(numberOfVertices);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void debugReport()
	{
		DEBUG_REPORT_PRINT(true, ("-- ShaderPrimitiveSetTemplate\n1 = compacted, 2 = bucketed\n"));

#if PROFILE_COLLISION == 1
		DEBUG_REPORT_PRINT(true, ("     collide1Calls = %i\n", ms_collide1Calls));
		DEBUG_REPORT_PRINT(true, ("      collide1Time = %1.5f\n", ms_collide1Time));
		DEBUG_REPORT_PRINT(true, ("collide1Primitives = %i\n", ms_collide1Primitives));
		DEBUG_REPORT_PRINT(true, ("      collide1Hits = %i\n", ms_collide1Hits));
		DEBUG_REPORT_PRINT(true, ("     collide2Calls = %i\n", ms_collide2Calls));
		DEBUG_REPORT_PRINT(true, ("      collide2Time = %1.5f\n", ms_collide2Time));
		DEBUG_REPORT_PRINT(true, ("collide2Primitives = %i\n", ms_collide2Primitives));
		DEBUG_REPORT_PRINT(true, ("      collide2Hits = %i\n", ms_collide2Hits));

		ms_collide1Calls = 0;
		ms_collide1Time = 0.f;
		ms_collide1Primitives = 0;
		ms_collide1Hits = 0;
		ms_collide2Calls = 0;
		ms_collide2Time = 0.f;
		ms_collide2Primitives = 0;
		ms_collide2Hits = 0;
#endif
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ShaderPrimitiveSetTemplateNamespace;

// ======================================================================
// ShaderPrimitiveSetTemplateNamespace
// ======================================================================

void ShaderPrimitiveSetTemplateNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_reportCollision);
	DebugFlags::unregisterFlag(ms_logCollisionSplit);
}

// ======================================================================
// ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData
// ======================================================================

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::CollisionData(IndexedTriangleList * const indexedTriangleList) :
	m_upwardIndices(0),
	m_indexedTriangleList(indexedTriangleList),
	m_boxExtent()
{
	AxialBox box;
	box.add(indexedTriangleList->getVertices());
	m_boxExtent.setBox(box);
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::~CollisionData()
{
	delete m_indexedTriangleList;
	delete m_upwardIndices;
}

// ----------------------------------------------------------------------

IndexedTriangleList const * ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::getIndexedTriangleList() const
{
	return m_indexedTriangleList;
}

// ----------------------------------------------------------------------

Extent const & ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::getExtent() const
{
	return m_boxExtent;
}

// ----------------------------------------------------------------------

stdvector<uint16>::fwd const * ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::getUpwardIndices() const
{
	createUpwardIndices();

	return m_upwardIndices;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::CollisionData::clearUpwardIndices() const
{
	if (m_upwardIndices)
	{
		delete m_upwardIndices;
		m_upwardIndices = 0;
	}
}

// ======================================================================
// ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::SimpleCollisionData
// ======================================================================

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::SimpleCollisionData::SimpleCollisionData(IndexedTriangleList * const indexedTriangleList) :
	CollisionData(indexedTriangleList),
	m_faceNormals(0)
{
	int const numberOfIndices = static_cast<int>(indexedTriangleList->getIndices().size());
	int const numberOfFaces = numberOfIndices / 3;
	m_faceNormals = new Vector[numberOfFaces];

	Vector normal;
	Vector const * const vertices = &indexedTriangleList->getVertices()[0];
	int const * indices = &indexedTriangleList->getIndices()[0];
	for (int i = 0; i < numberOfFaces; i++)
	{
		uint16 const i0 = static_cast<uint16>(*indices++);
		uint16 const i1 = static_cast<uint16>(*indices++);
		uint16 const i2 = static_cast<uint16>(*indices++);

		Vector const & v0 = vertices[i0];
		Vector const & v1 = vertices[i1];
		Vector const & v2 = vertices[i2];

		//-- compute normal
		normal = (v0 - v2).cross(v1 - v0);
		normal.approximateNormalize();
		normal *= 100.f;
		m_faceNormals[i] = normal;
	}
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::SimpleCollisionData::~SimpleCollisionData()
{
	delete [] m_faceNormals;
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::SimpleCollisionData::collide(Vector const & start_o, Vector const & end_o, CollisionInfo & result) const
{
	IndexedTriangleList const * const indexedTriangleList = getIndexedTriangleList();
	NOT_NULL(indexedTriangleList);

#if PROFILE_COLLISION == 1
	PerformanceTimer timer;
	timer.start();

	++ms_collide1Calls;
#endif

	bool found = false;

	result.setPoint (end_o);

	if (getExtent().intersect(start_o, result.getPoint()))
	{
		Vector const * const vertices = &indexedTriangleList->getVertices()[0];
		int const * indices = &indexedTriangleList->getIndices()[0];
		int const numberOfFaces = static_cast<int>(indexedTriangleList->getIndices().size()) / 3;
		Vector const dir = end_o - start_o;

		Plane plane;
		Vector intersection;

		int i;
		for (i = 0; i < numberOfFaces; i++)
		{
#if PROFILE_COLLISION == 1
			++ms_collide1Primitives;
#endif

			Vector const & v0 = vertices[*indices++];
			Vector const & v1 = vertices[*indices++];
			Vector const & v2 = vertices[*indices++];
			Vector const & normal = m_faceNormals[i];

			//-- ignore backfaces
			if (dir.dot(normal) < 0.f)
			{
				//-- it doesn't matter that the normal is not normalized
				plane.set(normal, v0);

				//-- see if the end points intersect the plane the polygon lies on, lies within the polygon, and is closer to start than the previous point
				if ((plane.findIntersection(start_o, result.getPoint(), intersection)) &&
					(start_o.magnitudeBetweenSquared(intersection) < start_o.magnitudeBetweenSquared(result.getPoint())) &&
					(intersection.inPolygon(v0, v1, v2)))
				{
#if PROFILE_COLLISION == 1
					++ms_collide1Hits;
#endif

					found = true;

					Vector normalizedNormal(normal);
					normalizedNormal.normalize();

					result.setPoint(intersection);
					result.setNormal(normalizedNormal);
				}
			}
		}
	}

#if PROFILE_COLLISION == 1
	timer.stop();
	ms_collide1Time += timer.getElapsedTime();
#endif

	return found;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::SimpleCollisionData::createUpwardIndices() const
{
	if (m_upwardIndices)
		return;

	if (!ms_allowUpwardIndicesCreation)
		return;

	IndexedTriangleList const * const indexedTriangleList = getIndexedTriangleList();
	int const numberOfIndices = static_cast<int>(indexedTriangleList->getIndices().size());
	int const numberOfFaces = numberOfIndices / 3;

	ms_upwardIndices.reserve(static_cast<size_t>(numberOfIndices));

	Vector normal;
	Vector const * const vertices = &indexedTriangleList->getVertices()[0];
	int const * indices = &indexedTriangleList->getIndices()[0];
	for (int i = 0; i < numberOfFaces; i++)
	{
		uint16 const i0 = static_cast<uint16>(*indices++);
		uint16 const i1 = static_cast<uint16>(*indices++);
		uint16 const i2 = static_cast<uint16>(*indices++);

		Vector const & v0 = vertices[i0];
		Vector const & v1 = vertices[i1];
		Vector const & v2 = vertices[i2];

		//-- compute normal
		normal = (v0 - v2).cross(v1 - v0);
		normal.approximateNormalize();

		//-- add to upward indices
		if (normal.y > cms_verticalThreshold)
		{
			ms_upwardIndices.push_back(i0);
			ms_upwardIndices.push_back(i1);
			ms_upwardIndices.push_back(i2);
		}
	}

	//-- Shrink the upward indices list
	m_upwardIndices = new std::vector<uint16>;
	m_upwardIndices->reserve(ms_upwardIndices.size());
	m_upwardIndices->assign(ms_upwardIndices.begin(), ms_upwardIndices.end());
	ms_upwardIndices.clear();
}

// ======================================================================
// ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::BucketedCollisionData
// ======================================================================

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::BucketedCollisionData::BucketedCollisionData(IndexedTriangleList * const indexedTriangleList) :
	CollisionData(indexedTriangleList)
{
	for (int i = 0; i < 9; ++i)
	{
		m_bucketedIndices[i] = 0;
		m_bucketedFaceNormals[i] = 0;
	}

	{
		Vector const center = getExtent().getSphere().getCenter();

		Vector const * const vertices = &indexedTriangleList->getVertices()[0];
		int const * indices = &indexedTriangleList->getIndices()[0];
		int const numberOfIndices = static_cast<int>(indexedTriangleList->getIndices().size());
		int const numberOfFaces = numberOfIndices / 3;

		Vector normal;

		for (int i = 0; i < numberOfFaces; i++)
		{
			uint16 const i0 = static_cast<uint16>(*indices++);
			uint16 const i1 = static_cast<uint16>(*indices++);
			uint16 const i2 = static_cast<uint16>(*indices++);

			Vector const & v0 = vertices[i0];
			Vector const & v1 = vertices[i1];
			Vector const & v2 = vertices[i2];

			//-- compute normal
			normal = (v0 - v2).cross(v1 - v0);
			normal.approximateNormalize();
			normal *= 100.f;

			int const bucket = hashTriangle(center, v0, v1, v2);
			ms_indexLists[bucket].push_back(i0);
			ms_indexLists[bucket].push_back(i1);
			ms_indexLists[bucket].push_back(i2);
			ms_normalLists[bucket].push_back(normal);

			m_bucketedBoxExtent[bucket].updateMinAndMax(v0);
			m_bucketedBoxExtent[bucket].updateMinAndMax(v1);
			m_bucketedBoxExtent[bucket].updateMinAndMax(v2);
		}
	}

	//-- create system index buffers and box extents for each bucket
	{
		for (int i = 0; i < 9; ++i)
		{
			if (ms_indexLists[i].empty())
			{
				m_bucketedIndices[i] = 0;
				m_bucketedFaceNormals[i] = 0;
			}
			else
			{
				m_bucketedBoxExtent[i].calculateCenterAndRadius();

				//-- create bucketed index buffer
				m_bucketedIndices[i] = new std::vector<uint16>;
				m_bucketedIndices[i]->reserve(ms_indexLists[i].size());
				m_bucketedIndices[i]->assign(ms_indexLists[i].begin(), ms_indexLists[i].end());
				ms_indexLists[i].clear();

				//-- create bucketed face normals
				m_bucketedFaceNormals[i] = new Vector[ms_normalLists[i].size()];
				imemcpy(m_bucketedFaceNormals[i], &ms_normalLists[i][0], sizeof(Vector) * ms_normalLists[i].size());
				ms_normalLists[i].clear();
			}
		}
	}

	//-- Now that we're bucketed, we can destroy the indices of the original collision indexedTriangleList
	{
		std::vector<int>().swap(indexedTriangleList->getIndices());
	}
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::BucketedCollisionData::~BucketedCollisionData()
{
	for (int i = 0; i < 9; ++i)
	{
		delete m_bucketedIndices[i];
		delete [] m_bucketedFaceNormals[i];
	}
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::BucketedCollisionData::collide(Vector const & start_o, Vector const & end_o, CollisionInfo & result) const
{
	IndexedTriangleList const * const indexedTriangleList = getIndexedTriangleList();
	NOT_NULL(indexedTriangleList);

#if PROFILE_COLLISION == 1
	PerformanceTimer timer;
	timer.start();

	++ms_collide2Calls;
#endif

	bool found = false;

	result.setPoint (end_o);

	if (getExtent().intersect(start_o, result.getPoint()))
	{
		Vector const * const vertices = &indexedTriangleList->getVertices()[0];

		for (int j = 0; j < 9; ++j)
		{
			if (!m_bucketedIndices[j])
				continue;

			if (!m_bucketedBoxExtent[j].intersect(start_o, result.getPoint ()))
				continue;

			uint16 const * indices = &(*m_bucketedIndices[j])[0];
			int const numberOfFaces = static_cast<int>(m_bucketedIndices[j]->size()) / 3;
			Vector const dir = end_o - start_o;

			Plane plane;
			Vector intersection;

			int i;
			for (i = 0; i < numberOfFaces; i++)
			{
#if PROFILE_COLLISION == 1
				++ms_collide2Primitives;
#endif

				Vector const & v0 = vertices[*indices++];
				Vector const & v1 = vertices[*indices++];
				Vector const & v2 = vertices[*indices++];
				Vector const & normal = m_bucketedFaceNormals[j][i];

				//-- ignore backfaces
				if (dir.dot(normal) < 0.f)
				{
					//-- it doesn't matter that the normal is not normalized
					plane.set(normal, v0);

					//-- see if the end points intersect the plane the polygon lies on, lies within the polygon, and is closer to start than the previous point
					if ((plane.findIntersection(start_o, result.getPoint(), intersection)) &&
						(start_o.magnitudeBetweenSquared(intersection) < start_o.magnitudeBetweenSquared(result.getPoint())) &&
						(intersection.inPolygon(v0, v1, v2)))
					{
#if PROFILE_COLLISION == 1
						++ms_collide2Hits;
#endif

						found = true;

						Vector normalizedNormal(normal);
						normalizedNormal.normalize();

						result.setPoint(intersection);
						result.setNormal(normalizedNormal);
					}
				}
			}
		}
	}

#if PROFILE_COLLISION == 1
	timer.stop();
	ms_collide2Time += timer.getElapsedTime();
#endif

	return found;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::BucketedCollisionData::createUpwardIndices() const
{
	if (m_upwardIndices)
		return;

	if (!ms_allowUpwardIndicesCreation)
		return;

	IndexedTriangleList const * const indexedTriangleList = getIndexedTriangleList();
	size_t totalNumberOfIndices = 0;
	{
		for (int j = 0; j < 9; ++j)
			if (m_bucketedIndices[j])
				totalNumberOfIndices += m_bucketedIndices[j]->size();
	}
	ms_upwardIndices.reserve(totalNumberOfIndices);

	for (int j = 0; j < 9; ++j)
	{
		if (!m_bucketedIndices[j])
			continue;

		int const numberOfIndices = static_cast<int>(m_bucketedIndices[j]->size());
		int const numberOfFaces = numberOfIndices / 3;
		Vector const * const vertices = &indexedTriangleList->getVertices()[0];
		uint16 const * indices = &(*m_bucketedIndices[j])[0];

		Vector normal;
		for (int i = 0; i < numberOfFaces; i++)
		{
			uint16 const i0 = static_cast<uint16>(*indices++);
			uint16 const i1 = static_cast<uint16>(*indices++);
			uint16 const i2 = static_cast<uint16>(*indices++);

			Vector const & v0 = vertices[i0];
			Vector const & v1 = vertices[i1];
			Vector const & v2 = vertices[i2];

			//-- compute normal
			normal = (v0 - v2).cross(v1 - v0);
			normal.approximateNormalize();

			//-- add to upward indices
			if (normal.y > cms_verticalThreshold)
			{
				ms_upwardIndices.push_back(i0);
				ms_upwardIndices.push_back(i1);
				ms_upwardIndices.push_back(i2);
			}
		}
	}

	//-- Shrink the upward indices list
	m_upwardIndices = new std::vector<uint16>;
	m_upwardIndices->reserve(ms_upwardIndices.size());
	m_upwardIndices->assign(ms_upwardIndices.begin(), ms_upwardIndices.end());
	ms_upwardIndices.clear();
}

// ======================================================================
// ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate
// ======================================================================

static ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::DrawFunction s_drawFunctionLookupTable[] =
{
	Graphics::drawPointList,            // SPSPT_pointList,
	Graphics::drawLineList,             // SPSPT_lineList,
	Graphics::drawLineStrip,            // SPSPT_lineStrip,
	Graphics::drawTriangleList,         // SPSPT_triangleList,
	Graphics::drawTriangleStrip,        // SPSPT_triangleStrip,
	Graphics::drawTriangleFan,          // SPSPT_triangleFan,

	Graphics::drawIndexedPointList,     // SPSPT_indexedPointList,
	Graphics::drawIndexedLineList,      // SPSPT_indexedLineList,
	Graphics::drawIndexedLineStrip,     // SPSPT_indexedLineStrip,
	Graphics::drawIndexedTriangleList,  // SPSPT_indexedTriangleList,
	Graphics::drawIndexedTriangleStrip, // SPSPT_indexedTriangleStrip,
	Graphics::drawIndexedTriangleFan,   // SPSPT_indexedTriangleFan,
};

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::install()
{
	DebugFlags::registerFlag(ms_reportCollision, "ClientGraphics/ShaderPrimitiveSetTemplate", "reportCollision", debugReport);
	DebugFlags::registerFlag(ms_logCollisionSplit, "ClientGraphics/ShaderPrimitiveSetTemplate", "logCollisionSplit");

	ms_createCollisionDataForNonCollidablePrimitives = ConfigFile::getKeyBool("ClientGraphics/ShaderPrimitiveSetTemplate", "createCollisionDataForNonCollidablePrimitives", false);

	ExitChain::add(ShaderPrimitiveSetTemplateNamespace::remove, "ShaderPrimitiveSetTemplateNamespace::remove");
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::LocalShaderPrimitiveTemplate(const char *owner, const ShaderTemplate &shaderTemplate)
:
#ifdef _DEBUG
	m_owner(owner),
#endif
	m_shaderTemplate(&shaderTemplate),
	m_vertexBuffer(NULL),
	m_indexBuffer(NULL),
	m_sortedIndices(NULL),
	m_drawFunction(NULL),
	m_collisionData(0),
	m_approximateRadius(0.0f)
{
	UNREF(owner);
	m_shaderTemplate->fetch();
//	computeApproximateRadius();
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::~LocalShaderPrimitiveTemplate()
{
	//lint -esym(1740, LocalShaderPrimitiveTemplate::m_shaderTemplate) // not freed or zero'ed // it's okay, it's reference counted
	NOT_NULL(m_shaderTemplate);
	m_shaderTemplate->release();

	delete m_vertexBuffer;
	delete m_indexBuffer;

	if (m_sortedIndices)
	{
		std::for_each(m_sortedIndices->begin(), m_sortedIndices->end(), PointerDeleterPairSecond());
		delete m_sortedIndices;
	}

	delete m_collisionData;
}


//----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::computeApproximateRadius()
{
	m_approximateRadius = 0.0f;
	
	m_vertexBuffer->lockReadOnly();
	{
		
		VertexBufferReadIterator ivEnd = m_vertexBuffer->endReadOnly();
		for (VertexBufferReadIterator iv = m_vertexBuffer->beginReadOnly(); iv != ivEnd; ++iv)
		{
			Vector const & pos = iv.getPosition();
			m_approximateRadius = std::max(m_approximateRadius, pos.magnitudeSquared());
		}
	}

	m_vertexBuffer->unlock();
	m_approximateRadius = sqrt(m_approximateRadius);
}

// ----------------------------------------------------------------------
// Load an old version of the LSPT that doesn't have a version tag

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::load_old(Iff &iff)
{
	iff.enterForm();

		iff.enterChunk(TAG_INFO);

			// get the primitive type to figure out the draw function
			const int primitiveTypeInt = iff.read_int32();
			VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(static_cast<int>(SPSPT_pointList), primitiveTypeInt, static_cast<int>(SPSPT_indexedTriangleFan));
			//DEBUG_FATAL(primitiveTypeInt < static_cast<int>(SPSPT_pointList) || primitiveTypeInt > static_cast<int>(SPSPT_indexedTriangleFan), ("bad primitive type"));

		iff.exitChunk(TAG_INFO);

		m_drawFunction = s_drawFunctionLookupTable[primitiveTypeInt];

		m_vertexBuffer = new StaticVertexBuffer(iff);

		// see if there should be index data
		if (primitiveTypeInt >= static_cast<int>(SPSPT_indexedPointList) && primitiveTypeInt <= static_cast<int>(SPSPT_indexedTriangleFan))
		{
			// this is not done as a single array read because the size of the Index
			// type may change depending upon the platform, and we do not want the data
			// format to depend upon that.  so the data is stored on disk in int32 form,
			// and we'll read it in and convert it to the Index type as we go.

			iff.enterChunk(TAG_INDX);

				// figure out how many indices there are and allocate an array of them
				const int indexCount = iff.getChunkLengthTotal(sizeof(int32));
				m_indexBuffer = new StaticIndexBuffer(indexCount);

				m_indexBuffer->lock();

					// read the index data in
					const Index * const end = m_indexBuffer->end();
					for (Index * i = m_indexBuffer->begin(); i != end; ++i)
					{
						const Index index = static_cast<Index>(iff.read_int32());
						*i = index;
						DEBUG_FATAL(index >= m_vertexBuffer->getNumberOfVertices(), ("bad vertex index %d/%d", index, m_vertexBuffer->getNumberOfVertices()));
					}

				m_indexBuffer->unlock();

			iff.exitChunk(TAG_INDX);
		}

	iff.exitForm(true);

	//-- split into pieces for collision
	collisionSplit(iff.getFileName());

#ifdef _DEBUG
	if (DataLint::isEnabled())
		dataLint();
#endif
}

// ----------------------------------------------------------------------
// Load a versioned LSPT

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::load(Iff &iff)
{
	switch (iff.getCurrentName())
	{
	case TAG_0000:
		load_0000(iff);
		break;

	case TAG_0001:
		load_0001(iff);
		break;

	default:
		{
			char buffer[512];
			iff.formatLocation(buffer, sizeof(buffer));
			DEBUG_FATAL(true, ("Unsupported tag at %s", buffer));
		}
		break;
	}

	//-- split into pieces for collision
	collisionSplit(iff.getFileName());

	if (!m_collisionData)
		computeApproximateRadius();

#ifdef _DEBUG
	if (DataLint::isEnabled())
		dataLint();
#endif
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::collisionSplit(char const * const debugName)
{
	UNREF(debugName);

	//-- if we have no geometry, return
	if (m_indexBuffer->getNumberOfIndices() == 0)
		return;

	//-- If we're a non-collidable shader template, return
	if (!ms_createCollisionDataForNonCollidablePrimitives && !m_shaderTemplate->isCollidable() && !m_shaderTemplate->isOpaqueSolid())
		return;	

#if PROFILE_COLLISION == 1
	PerformanceTimer timer;
	timer.start();
#endif

	//-- Create collision indexed triangle list
	m_vertexBuffer->lockReadOnly();
	m_indexBuffer->lockReadOnly();

		VertexBufferReadIterator svi = m_vertexBuffer->beginReadOnly();
		Index const * sii = m_indexBuffer->beginReadOnly();
		int const numberOfIndices = m_indexBuffer->getNumberOfIndices();
		IndexedTriangleList * const indexedTriangleList = new IndexedTriangleList();

		//-- create index buffer
		std::vector<int> collisionIndices;
		collisionIndices.reserve(numberOfIndices);

		PositionVertexIndexer vertexIndexer;
		
		{
			Vector position;

			//-- create compacted index buffer
			for (int i = 0; i < numberOfIndices; ++i, ++sii)
			{
				position = (svi + *sii).getPosition();
				collisionIndices.push_back(vertexIndexer.addVertex(position));
			}
		}

		//-- copy compacted vertex array to vertex buffer
		indexedTriangleList->getVertices().swap(vertexIndexer.getVertices());

		std::vector<int> & indices = indexedTriangleList->getIndices();
		indices.reserve(collisionIndices.size());
		indices.assign(collisionIndices.begin(), collisionIndices.end());

	m_vertexBuffer->unlock();
	m_indexBuffer->unlock();

	int const numberOfCollisionIndices = static_cast<int>(indexedTriangleList->getIndices().size());
	int const numberOfCollisionFaces = numberOfCollisionIndices / 3;
	if (numberOfCollisionFaces > 64)
		m_collisionData = new BucketedCollisionData(indexedTriangleList);
	else
		m_collisionData = new SimpleCollisionData(indexedTriangleList);

#if PROFILE_COLLISION == 1
	timer.stop();
	DEBUG_REPORT_LOG(ms_logCollisionSplit, ("ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::collisionSplit [%s]: %1.5f seconds\n", debugName, timer.getElapsedTime()));
#endif
}

// ----------------------------------------------------------------------
// this is not done as a single array read because the size of the Index
// type may change depending upon the platform, and we do not want the data
// format to depend upon that.  so the data is stored on disk in int32 form,
// and we'll read it in and convert it to the Index type as we go.

StaticIndexBuffer *	ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::loadStaticIndexBuffer32(Iff &iff)
{
	const int nIndices = iff.read_int32();
	StaticIndexBuffer * pBuffer = new StaticIndexBuffer(nIndices);

	pBuffer->lock();

		Index * indices = pBuffer->begin();
		for(int i = 0; i < nIndices; i++)
		{
			indices[i] = static_cast<Index>(iff.read_int32());
		}

	pBuffer->unlock();

	return pBuffer;
}

// ----------------------------------------------------------------------

StaticIndexBuffer *	ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::loadStaticIndexBuffer16(Iff &iff)
{
	const int nIndices = iff.read_int32();
	StaticIndexBuffer * pBuffer = new StaticIndexBuffer(nIndices);

	pBuffer->lock();

		Index * indices = pBuffer->begin();
		for(int i = 0; i < nIndices; i++)
		{
			indices[i] = iff.read_uint16();
		}

	pBuffer->unlock();

	return pBuffer;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			// get the primitive type to figure out the draw function
			const int primitiveTypeInt = iff.read_int32();
			VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(static_cast<int>(SPSPT_pointList), primitiveTypeInt, static_cast<int>(SPSPT_indexedTriangleFan));
			m_drawFunction = s_drawFunctionLookupTable[primitiveTypeInt];

			const bool hasIndices = iff.read_bool8();
			const bool hasSortedIndices = iff.read_bool8();

		iff.exitChunk(TAG_INFO);

		// load the vertex buffer
		m_vertexBuffer = new StaticVertexBuffer(iff);

		// load the normal index buffer
		if (hasIndices)
		{
			iff.enterChunk(TAG_INDX);
				m_indexBuffer = loadStaticIndexBuffer32(iff);
			iff.exitChunk(TAG_INDX);
		}

		// load the directional index buffers
		if (hasSortedIndices)
		{
			//-- Only users with more than 256 MB of RAM will get sorted index buffers
			if (MemoryManager::getLimit() > 256)
			{
				m_sortedIndices = new SortedIndexBufferList();

				iff.enterChunk(TAG_SIDX);

					const int nArrays = iff.read_int32();
					for(int i = 0; i < nArrays; i++)
					{
						DirectionSortedIndexBuffer dsib;

						dsib.first = iff.read_floatVector();
						dsib.second = loadStaticIndexBuffer32(iff);

						m_sortedIndices->push_back(dsib);
					}

				iff.exitChunk(TAG_SIDX);
			}
			else
			{
				iff.enterChunk(TAG_SIDX);
				iff.exitChunk(TAG_SIDX, true);
			}
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_INFO);

			// get the primitive type to figure out the draw function
			const int primitiveTypeInt = iff.read_int32();
			VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(static_cast<int>(SPSPT_pointList), primitiveTypeInt, static_cast<int>(SPSPT_indexedTriangleFan));
			m_drawFunction = s_drawFunctionLookupTable[primitiveTypeInt];

			const bool hasIndices = iff.read_bool8();
			const bool hasSortedIndices = iff.read_bool8();

		iff.exitChunk(TAG_INFO);

		// load the vertex buffer
		m_vertexBuffer = new StaticVertexBuffer(iff);

		// load the normal index buffer
		if (hasIndices)
		{
			iff.enterChunk(TAG_INDX);
				m_indexBuffer = loadStaticIndexBuffer16(iff);
			iff.exitChunk(TAG_INDX);
		}

		// load the directional index buffers
		if (hasSortedIndices)
		{
			//-- Only users with more than 256 MB of RAM will get sorted index buffers
			if (MemoryManager::getLimit() > 256)
			{
				m_sortedIndices = new SortedIndexBufferList();

				iff.enterChunk(TAG_SIDX);

					const int nArrays = iff.read_int32();
					for(int i = 0; i < nArrays; i++)
					{
						DirectionSortedIndexBuffer dsib;

						dsib.first = iff.read_floatVector();
						dsib.second = loadStaticIndexBuffer16(iff);

						m_sortedIndices->push_back(dsib);
					}

				iff.exitChunk(TAG_SIDX);
			}
			else
			{
				iff.enterChunk(TAG_SIDX);
				iff.exitChunk(TAG_SIDX, true);
			}
		}

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::validateIndexBuffer(const StaticIndexBuffer *pIndexBuffer) const
{
	int nVertices = m_vertexBuffer->getNumberOfVertices();
	int nIndices = pIndexBuffer->getNumberOfIndices();

	pIndexBuffer->lockReadOnly();

	const Index *indices = pIndexBuffer->beginReadOnly();

	for(int i = 0; i < nIndices; i++)
		DEBUG_FATAL(indices[i] < 0 || indices[i] >= nVertices, ("bad vertex index %d/%d", indices[i], nVertices));

	pIndexBuffer->unlock();
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

static void formatBits(char *buffer, uint8 bits)
{
	for (int i = 0; i < 8; ++i)
		buffer[7-i] = ((bits & (1 << i)) != 0) ? '1' : '0';
	buffer[8] = '\0';
}

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::validateShaderTemplateWithVertexBuffer(const ShaderTemplate *shaderTemplate) const
{
	// validate static shader templates
	{
		const StaticShaderTemplate *sst = dynamic_cast<const StaticShaderTemplate *>(shaderTemplate);
		if (sst)
		{
			const char *name = sst->getName().getString();

			if (name)
				DataLint::pushAsset(name);

			if (sst->getShaderEffect().getActiveShaderImplementation())
				sst->getShaderEffect().getActiveShaderImplementation()->verifyCompatibility(m_vertexBuffer->getFormat(), *sst);

			uint8 shaderTextureCoordinateSets = sst->getTextureCoordinateSetUsageMask();
			uint8 vbTextureCoordinateSets = 0;
			for (int i = 0; i < m_vertexBuffer->getNumberOfTextureCoordinateSets(); ++i)
				vbTextureCoordinateSets |= (1 << i);

			if (shaderTextureCoordinateSets != vbTextureCoordinateSets)
			{
				char buffer1[16];
				char buffer2[16];
				formatBits(buffer1, shaderTextureCoordinateSets);
				formatBits(buffer2, vbTextureCoordinateSets);

				char const * const name = sst->getName().getString() ? sst->getName().getString() : "unknown";
				char const * const missing = (shaderTextureCoordinateSets & (~vbTextureCoordinateSets)) ? "missing " : "";
				char const * const extra = (vbTextureCoordinateSets & (~shaderTextureCoordinateSets)) ? "extra " : "";
				char const * const skip = (((shaderTextureCoordinateSets + 1) & shaderTextureCoordinateSets) != 0) ? "skip " : "";

				WARNING(true, ("shader %s needs different texture coordinates than vb %s%s%s(%s %s)", name, missing, extra, skip, buffer1, buffer2));
			}

			if (name)
				DataLint::popAsset();
		}
	}

	// validate children of switch shader templates
	{
		const SwitchShaderTemplate *sst = dynamic_cast<const SwitchShaderTemplate *>(shaderTemplate);
		if (sst)
		{
			const SwitchShaderTemplate::ShaderTemplates *sts = sst->getShaderTemplates();

			for (uint i = 0; i < sts->size(); ++i)
			{
				const ShaderTemplate *st = (*sts)[i];
				if (st)
					validateShaderTemplateWithVertexBuffer(st);
			}
		}
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::dataLint() const
{
	if (m_indexBuffer)
	{
		validateIndexBuffer(m_indexBuffer);
		WARNING(m_indexBuffer->getNumberOfIndices() == m_vertexBuffer->getNumberOfVertices(), ("No shared vertices %d/%d", m_indexBuffer->getNumberOfIndices(), m_vertexBuffer->getNumberOfVertices()));
	}

	if (m_sortedIndices)
	{
		for(uint i = 0; i < m_sortedIndices->size(); i++)
			validateIndexBuffer(m_sortedIndices->at(i).second);
	}

	validateShaderTemplateWithVertexBuffer(m_shaderTemplate);
}

#endif

// ----------------------------------------------------------------------

const Shader *ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::fetchShader() const
{
	NOT_NULL(m_shaderTemplate);
	return m_shaderTemplate->fetchShader();
}

// ----------------------------------------------------------------------

Shader *ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::fetchModifiableShader() const
{
	NOT_NULL(m_shaderTemplate);
	return m_shaderTemplate->fetchModifiableShader();
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getVertexBufferSortKey() const
{
	NOT_NULL(m_vertexBuffer);
	return m_vertexBuffer->getSortKey();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::prepareToDraw(const Transform &objectToWorldTransform) const
{
	NOT_NULL(m_vertexBuffer);
	Graphics::setVertexBuffer(*m_vertexBuffer);

	if (m_sortedIndices)
	{
		DEBUG_FATAL(m_sortedIndices->empty(),("ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::prepareToDraw - Tried to draw using sorted indices, but there aren't any."));

		const Vector cameraPosition = objectToWorldTransform.rotateTranslate_p2l(ShaderPrimitiveSorter::getCurrentCameraPosition());
		const int nArrays = m_sortedIndices->size();
		int minArray = 0;
		real minDot = m_sortedIndices->at(0).first.dot(cameraPosition);

		for (int i = 1; i < nArrays; ++i)
		{
			DirectionSortedIndexBuffer & D = m_sortedIndices->at(i);

			const real dot = D.first.dot(cameraPosition);

			if (dot < minDot)
			{
				minDot = dot;
				minArray = i;
			}
		}

		Graphics::setIndexBuffer(*(m_sortedIndices->at(minArray).second));
	}
	else
	{
		if (m_indexBuffer)
			Graphics::setIndexBuffer(*m_indexBuffer);
	}

#ifdef _DEBUG
	if (GraphicsDebugFlags::renderNormals)
	{
		ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new VertexNormalsDebugPrimitive(objectToWorldTransform, *m_vertexBuffer));
	}

	if (GraphicsDebugFlags::renderVertexMatrices)
		ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new VertexMatrixFramesDebugPrimitive(objectToWorldTransform, *m_vertexBuffer));
#endif
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::draw() const
{
	NOT_NULL(m_drawFunction);
	m_drawFunction();
}

// ----------------------------------------------------------------------

const ShaderTemplate* ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getShaderTemplate () const
{
	return m_shaderTemplate;
}

// ----------------------------------------------------------------------

const StaticVertexBuffer* ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getVertexBuffer () const
{
	return m_vertexBuffer;
}

// ----------------------------------------------------------------------

const StaticIndexBuffer* ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getIndexBuffer () const
{
	return m_indexBuffer;
}

// ----------------------------------------------------------------------

Extent const * ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getExtent() const
{
	if (m_collisionData)
		return &m_collisionData->getExtent();

	return 0;
}

// ----------------------------------------------------------------------

IndexedTriangleList const * ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getCollisionIndexedTriangleList() const
{
	if (m_collisionData)
		return m_collisionData->getIndexedTriangleList();

	return 0;
}

// ----------------------------------------------------------------------

stdvector<uint16>::fwd const * ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getUpwardIndices() const
{
	if (m_collisionData)
		return m_collisionData->getUpwardIndices();

	return 0;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::clearUpwardIndices() const
{
	if (m_collisionData)
		m_collisionData->clearUpwardIndices();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const
{
	// @todo make this aware of multi-pass effects being more expensive

	numberOfVertices += m_vertexBuffer->getNumberOfVertices();

	if (m_drawFunction == Graphics::drawTriangleList)
		numberOfTriangles += m_vertexBuffer->getNumberOfVertices() / 3;
	else
		if (m_drawFunction == Graphics::drawTriangleStrip)
			numberOfTriangles += m_vertexBuffer->getNumberOfVertices() - 2;
		else
			if (m_drawFunction == Graphics::drawTriangleFan)
				numberOfTriangles += m_vertexBuffer->getNumberOfVertices() - 2;
			else
				if (m_drawFunction == Graphics::drawIndexedTriangleList)
					numberOfTriangles += m_indexBuffer->getNumberOfIndices() / 3;
				else
					if (m_drawFunction == Graphics::drawIndexedTriangleStrip)
						numberOfTriangles += m_indexBuffer->getNumberOfIndices() - 2;
					else
						if (m_drawFunction == Graphics::drawIndexedTriangleFan)
							numberOfTriangles += m_indexBuffer->getNumberOfIndices() - 2;

	complexity += static_cast<float>(m_vertexBuffer->getNumberOfVertices());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::addToExtent(AxialBox &extent) const
{
	NOT_NULL(m_vertexBuffer);
	DEBUG_FATAL(m_vertexBuffer->getNumberOfVertices() == 0, ("VB with no vertices"));
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB has no position"));

	m_vertexBuffer->lockReadOnly();

		VertexBufferReadIterator iEnd = m_vertexBuffer->endReadOnly();
		for (VertexBufferReadIterator i = m_vertexBuffer->beginReadOnly(); i != iEnd; ++i)
			extent.add(i.getPosition());

	m_vertexBuffer->unlock();
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	if (m_collisionData && (ms_collideAgainstAllGeometry || collideParameters.typeShouldCollide(m_shaderTemplate->isCollidable(), m_shaderTemplate->isOpaqueSolid())))
		return m_collisionData->collide(start_o, end_o, result);

	return false;
}

// ======================================================================
// ShaderPrimitiveSetTemplate
// ======================================================================

void ShaderPrimitiveSetTemplate::setBuildingAsynchronousLoaderData(bool const buildingAsynchronousLoaderData)
{
	ms_buildingAsynchronousLoaderData = buildingAsynchronousLoaderData;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::setCollideAgainstAllGeometry(bool const collideAgainstAllGeometry)
{
	ms_collideAgainstAllGeometry = collideAgainstAllGeometry;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::setAllowUpwardIndicesCreation(bool const allowUpwardIndicesCreation)
{
	ms_allowUpwardIndicesCreation = allowUpwardIndicesCreation;
}

// ======================================================================

ShaderPrimitiveSetTemplate::ShaderPrimitiveSetTemplate(char const * const owner, Iff & iff) :
	m_localShaderPrimitiveTemplateList(new LocalShaderPrimitiveTemplateList)
{
	load(owner, iff);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::load(char const * const owner, Iff & iff)
{
	switch (iff.getCurrentName())
	{
	case TAG_SPS:
		load_sps(owner, iff);
		break;

	default:
		{
			char buffer[512];
			iff.formatLocation(buffer, sizeof(buffer));
			DEBUG_FATAL(true, ("Unsupported tag at %s", buffer));
		}
		break;
	}
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::load_sps(char const * const owner, Iff & iff)
{
	iff.enterForm(TAG_SPS);

	switch (iff.getCurrentName())
	{
	case TAG_0000:
		load_sps_0000(owner, iff);
		break;

	case TAG_0001:
		load_sps_0001(owner, iff);
		break;

	default:
		{
			char buffer[512];
			iff.formatLocation(buffer, sizeof(buffer));
			DEBUG_FATAL(true, ("Unsupported tag at %s", buffer));
		}
		break;
	}

	iff.exitForm(TAG_SPS);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::load_sps_0000(char const * const owner, Iff & iff)
{
	iff.enterForm(TAG_0000);

	// get the number of shaders
	iff.enterChunk(TAG_CNT);
		const int numberOfShaders = iff.read_int32();
	iff.exitChunk(TAG_CNT);

	// load the per-shader data
	for (int shaderIndex = 0; shaderIndex < numberOfShaders; ++shaderIndex)
	{
		iff.enterForm();

			// load the shader
			const ShaderTemplate *shaderTemplate = ShaderTemplateList::fetch(iff);

#ifdef _DEBUG
			if (ms_buildingAsynchronousLoaderData)
			{
				shaderTemplate->release();
				iff.exitForm(true);
				continue;
			}
#endif

			// get the number of primitives
			iff.enterChunk(TAG_INFO);
				const int numberOfPrimitives = iff.read_int32();
			iff.exitChunk(TAG_INFO);

			// load the per-primitive data
			for (int primitiveIndex = 0; primitiveIndex < numberOfPrimitives; ++primitiveIndex)
			{
				LocalShaderPrimitiveTemplate *primitive = new LocalShaderPrimitiveTemplate(owner, *shaderTemplate);
				primitive->load_old(iff);
				m_localShaderPrimitiveTemplateList->push_back(primitive);
			}

			shaderTemplate->release();

		iff.exitForm();
	}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSetTemplate::load_sps_0001(char const * const owner, Iff & iff)
{
	iff.enterForm(TAG_0001);

	// get the number of shaders
	iff.enterChunk(TAG_CNT);
		const int numberOfShaders = iff.read_int32();
	iff.exitChunk(TAG_CNT);

	// load the per-shader data
	for (int shaderIndex = 0; shaderIndex < numberOfShaders; ++shaderIndex)
	{
		iff.enterForm();

			// load the shader
			const ShaderTemplate *shaderTemplate = ShaderTemplateList::fetch(iff);

#ifdef _DEBUG
			if (ms_buildingAsynchronousLoaderData)
			{
				shaderTemplate->release();
				iff.exitForm(true);
				continue;
			}
#endif

			// get the number of primitives
			iff.enterChunk(TAG_INFO);
				const int numberOfPrimitives = iff.read_int32();
			iff.exitChunk(TAG_INFO);

			// load the per-primitive data
			for (int primitiveIndex = 0; primitiveIndex < numberOfPrimitives; ++primitiveIndex)
			{
				LocalShaderPrimitiveTemplate *primitive = new LocalShaderPrimitiveTemplate(owner, *shaderTemplate);
				primitive->load(iff);
				m_localShaderPrimitiveTemplateList->push_back(primitive);
			}

			shaderTemplate->release();

		iff.exitForm();
	}

	iff.exitForm(TAG_0001);
}


// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::~ShaderPrimitiveSetTemplate()
{
	std::for_each(m_localShaderPrimitiveTemplateList->begin(), m_localShaderPrimitiveTemplateList->end(), PointerDeleter());
	delete m_localShaderPrimitiveTemplateList;
}

// ----------------------------------------------------------------------

ShaderPrimitiveSet *ShaderPrimitiveSetTemplate::createShaderPrimitiveSet(const Appearance &owner) const
{
	return new ShaderPrimitiveSet(owner, *this);
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSetTemplate::getNumberOfShaderPrimitiveTemplates() const
{
	NOT_NULL (m_localShaderPrimitiveTemplateList);
	return static_cast<int> (m_localShaderPrimitiveTemplateList->size ());
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate const & ShaderPrimitiveSetTemplate::getShaderPrimitiveTemplate(int const shaderPrimitiveTemplateIndex) const
{
	NOT_NULL(m_localShaderPrimitiveTemplateList);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderPrimitiveTemplateIndex, getNumberOfShaderPrimitiveTemplates());

	return *(*m_localShaderPrimitiveTemplateList)[static_cast<size_t>(shaderPrimitiveTemplateIndex)];
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

AxialBox ShaderPrimitiveSetTemplate::getExtent() const
{
	DEBUG_FATAL(m_localShaderPrimitiveTemplateList->empty(), ("shader primitive set template is empty"));

	AxialBox result;
	result.clear();

	LocalShaderPrimitiveTemplateList::const_iterator const iEnd = m_localShaderPrimitiveTemplateList->end();
	for (LocalShaderPrimitiveTemplateList::const_iterator i = m_localShaderPrimitiveTemplateList->begin(); i != iEnd; ++i)
		(*i)->addToExtent(result);

	return result;
}

#endif

// ======================================================================

