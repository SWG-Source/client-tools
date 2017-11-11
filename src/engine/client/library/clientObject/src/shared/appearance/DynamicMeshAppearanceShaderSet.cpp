//======================================================================
//
// DynamicMeshAppearanceShaderSet.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/DynamicMeshAppearanceShaderSet.h"

#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/ObjectListCamera.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Triangle3d.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace DynamicMeshAppearanceShaderSetNamespace
{
	Vector s_randomization;

	bool s_installed = false;

	void install()
	{
		if (s_installed)
			return;
		
		s_installed = true;
	}

	//----------------------------------------------------------------------
	
	class VertexInfo
	{
	public:
		//lint -e1925 //public data members

		VertexInfo() :
		  pos(),
			normal(),
			randomized(false),
			textureCoordinateSets()
		  {
		  }
		  
		  Vector pos;
		  Vector normal;
		  bool randomized;
		  
		  typedef stdvector<float>::fwd FloatVector;
		  typedef stdvector<FloatVector>::fwd FloatVectorVector;
		  
		  FloatVectorVector textureCoordinateSets;
		  
		  void setup(VertexBufferReadIterator const & v0, VertexBufferReadIterator const & v1, float t);
		  void apply(VertexBufferReadWriteIterator & v) const;
	};
	
	//----------------------------------------------------------------------
	
	void VertexInfo::setup(VertexBufferReadIterator const & v0, VertexBufferReadIterator const & v1, float t)
	{		
		normal = Vector::linearInterpolate(v0.getNormal(), v1.getNormal(), t);
		IGNORE_RETURN(normal.normalize());
		
		textureCoordinateSets.clear();
		
		int const numberOfCoordinateSets = v0.getNumberOfTextureCoordinateSets();
		DEBUG_FATAL(numberOfCoordinateSets != v1.getNumberOfTextureCoordinateSets(), ("SplitMeshTest data mismatch.  Coordinate set count invalid"));
		
		textureCoordinateSets.resize(static_cast<size_t>(numberOfCoordinateSets));
		
		for (int i = 0; i < numberOfCoordinateSets; ++i)
		{
			FloatVector & textureCoordinates = textureCoordinateSets[static_cast<size_t>(i)];
			
			int const textureCoordinateSetDimension = v0.getTextureCoordinateSetDimension(i);
			DEBUG_FATAL(textureCoordinateSetDimension != v1.getTextureCoordinateSetDimension(i), ("SplitMeshTest data mismatch.  Coordinate set dimension invalid"));
			
			textureCoordinates.reserve(static_cast<size_t>(textureCoordinateSetDimension));
			
			for (int j = 0; j < textureCoordinateSetDimension; ++j)
			{
				float const tc0 = v0.getTextureCoordinate(i, j);
				float const tc1 = v1.getTextureCoordinate(i, j);
				
				textureCoordinates.push_back(linearInterpolate(tc0, tc1, t));
			}
		}		
		
	}
	
	//----------------------------------------------------------------------
	
	void VertexInfo::apply(VertexBufferReadWriteIterator & v) const 
	{
		
		v.setPosition(pos);
		v.setNormal(normal);
		
		int const numTextureCoordinateSets = static_cast<int>(textureCoordinateSets.size());
		
		DEBUG_FATAL(v.getNumberOfTextureCoordinateSets() != numTextureCoordinateSets, ("SplitMeshTest unable to reapply texture coordinates. bad count"));
		
		for (int i = 0; i < numTextureCoordinateSets; ++i)
		{
			FloatVector const & textureCoordinates = textureCoordinateSets[static_cast<size_t>(i)];
			int const textureCoordinateSetDimension = static_cast<int>(textureCoordinates.size());
			for (int j = 0; j < textureCoordinateSetDimension; ++j)
				v.setTextureCoordinate(i, j, textureCoordinates[static_cast<size_t>(j)]);
		}		
	}
	
	//----------------------------------------------------------------------
	
	typedef stdvector<VertexInfo>::fwd VertexInfoVector;				
	
	//----------------------------------------------------------------------
	
	
	//----------------------------------------------------------------------
	
	void getProperlyWoundTriangle(VertexInfo const & v0, VertexInfo const & v1, VertexInfo const & v2, uint16 const index0, uint16 const index1, uint16 const index2, DynamicMeshAppearanceShaderSet::IndexVector & indexVector)
	{
		indexVector.push_back(index0);
		
		Vector const & faceNormal = (v0.normal + v1.normal + v2.normal) / 3.0f;
		Vector const & v1relativePos = v1.pos - v0.pos;
		Vector const & v2relativePos = v2.pos - v0.pos;
		
		if (v2relativePos.cross((v1relativePos)).dot(faceNormal) > 0.0f)
		{
			indexVector.push_back(index2);
			indexVector.push_back(index1);
		}
		else
		{
			indexVector.push_back(index1);
			indexVector.push_back(index2);
		}
	}

	//----------------------------------------------------------------------

	void getIndexVector(StaticIndexBuffer const & staticIndexBuffer, DynamicMeshAppearanceShaderSet::IndexVector & result)
	{
		staticIndexBuffer.lockReadOnly();
		result.reserve(static_cast<size_t>(staticIndexBuffer.getNumberOfIndices()));
		result.insert(result.end(), staticIndexBuffer.beginReadOnly(), staticIndexBuffer.beginReadOnly() + staticIndexBuffer.getNumberOfIndices());
		staticIndexBuffer.unlock();
	}
	
}

using namespace DynamicMeshAppearanceShaderSetNamespace;

//----------------------------------------------------------------------

/*
DynamicMeshAppearanceShaderSet::DynamicMeshAppearanceShaderSet(DynamicMeshAppearanceShaderSet const & rhs) :
m_shader(NULL),
m_vertexBuffer(NULL),
m_indexBuffer(NULL)
{
	if (rhs.m_shader == NULL || rhs.m_vertexBuffer == NULL || rhs.m_indexBuffer == NULL)
		return;

	IndexVector iv;
	iv.reserve(rhs.m_indexBuffer->getNumberOfIndices());
	iv.insert(iv.end(), rhs.m_indexBuffer->beginReadOnly(), rhs.m_indexBuffer->beginReadOnly() + rhs.m_indexBuffer->getNumberOfIndices());

	set(*rhs.m_shader, *rhs.m_vertexBuffer, iv);
}
*/

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet::DynamicMeshAppearanceShaderSet() :
m_shader(NULL),
m_vertexBuffer(NULL),
m_indexBuffer(NULL)
{
}

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet::DynamicMeshAppearanceShaderSet(Shader const & shader, StaticVertexBuffer const & vertexBuffer, StaticIndexBuffer const & indexBuffer) :
m_shader(NULL),
m_vertexBuffer(NULL),
m_indexBuffer(NULL)
{
	IndexVector iv;
	getIndexVector(indexBuffer, iv);
	set(shader, vertexBuffer, iv);
}

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet & DynamicMeshAppearanceShaderSet::set(DynamicMeshAppearanceShaderSet const & rhs)
{
	if (rhs.m_shader == NULL || rhs.m_vertexBuffer == NULL || rhs.m_indexBuffer == NULL)
		return *this;

	IndexVector iv;
	getIndexVector(*rhs.m_indexBuffer, iv);
	set(*rhs.m_shader, *rhs.m_vertexBuffer, iv);
	return *this;
}

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet::DynamicMeshAppearanceShaderSet(Shader const & shader, StaticVertexBuffer const & vertexBuffer, IndexVector const & indexVector) :
m_shader(NULL),
m_vertexBuffer(NULL),
m_indexBuffer(NULL)
{
	set(shader, vertexBuffer, indexVector);
}

//----------------------------------------------------------------------

DynamicMeshAppearanceShaderSet::~DynamicMeshAppearanceShaderSet()
{
	if (m_shader != NULL)
	{
		m_shader->release();
		m_shader = NULL;
	}

	if (m_vertexBuffer != NULL)
	{
		delete m_vertexBuffer;
		m_vertexBuffer = NULL;
	}
	
	if (m_indexBuffer != NULL)
	{
		delete m_indexBuffer;
		m_indexBuffer = NULL;
	}
}

//----------------------------------------------------------------------

void DynamicMeshAppearanceShaderSet::set(Shader const & shader, StaticVertexBuffer const & vertexBuffer, IndexVector const & indexVector)
{
	if (NULL != m_shader)
	{
		m_shader->release();
		m_shader = NULL;
	}

	m_shader = shader.getShaderTemplate().fetchShader();
		
	if (m_vertexBuffer != NULL)
	{
		delete m_vertexBuffer;
		m_vertexBuffer = NULL;
	}
	
	if (m_indexBuffer != NULL)
	{
		delete m_indexBuffer;
		m_indexBuffer = NULL;
	}

	m_indexBuffer = new StaticIndexBuffer(static_cast<int>(indexVector.size()));
	
	int const numberOfVertices = vertexBuffer.getNumberOfVertices();
	m_vertexBuffer = new StaticVertexBuffer(vertexBuffer.getFormat(), numberOfVertices);
	m_vertexBuffer->lock();
	vertexBuffer.lockReadOnly();
	VertexBufferReadIterator v_in = vertexBuffer.beginReadOnly();
	VertexBufferReadWriteIterator v_out = m_vertexBuffer->begin();
	v_out.copy(v_in, numberOfVertices);			
	m_vertexBuffer->unlock();
	vertexBuffer.unlock();
	
	{
		m_indexBuffer->lock();
		Index * index_out = m_indexBuffer->begin();
		
		{
			for (IndexVector::const_iterator it = indexVector.begin(); it != indexVector.end(); ++it)
			{
				*(index_out++) = *(it);
			}
		}
	}
	
	m_indexBuffer->unlock();
}

//----------------------------------------------------------------------

void DynamicMeshAppearanceShaderSet::split(Vector const & randomization, Plane const & plane, Shader const & shader, StaticVertexBuffer const & inputVertexBuffer, StaticIndexBuffer const & inputIndexBuffer, DynamicMeshAppearanceShaderSet *& outputFront, DynamicMeshAppearanceShaderSet *& outputBack, EdgeVector & edges)
{
	IndexVector indexVectorFront;
	IndexVector indexVectorBack;
	
	VertexInfoVector extraVerts;
	
	int const numberOfIndices = inputIndexBuffer.getNumberOfIndices();
	int const numberOfVertices = inputVertexBuffer.getNumberOfVertices();
	
	indexVectorFront.reserve(static_cast<size_t>(numberOfIndices));
	indexVectorBack.reserve(static_cast<size_t>(numberOfIndices));
	
	{			
		inputVertexBuffer.lockReadOnly();
		VertexBufferReadIterator const inputVertexIteratorBegin = inputVertexBuffer.beginReadOnly();
		
		inputIndexBuffer.lockReadOnly();
		Index const * inputIndexIteratorBegin = inputIndexBuffer.beginReadOnly();
		
		for (int i = 0; i < numberOfIndices;)
		{
			uint16 const triangleCornerIndices[3] = {
				*(inputIndexIteratorBegin + (i++)),
					*(inputIndexIteratorBegin + (i++)),
					*(inputIndexIteratorBegin + (i++))
			};
			
			VertexBufferReadIterator const vertexBufferIterators[3] =
			{
				inputVertexIteratorBegin + triangleCornerIndices[0],
					inputVertexIteratorBegin + triangleCornerIndices[1],
					inputVertexIteratorBegin + triangleCornerIndices[2],
			};
			
			Vector const corners[3] =
			{
				vertexBufferIterators[0].getPosition(),
					vertexBufferIterators[1].getPosition(),
					vertexBufferIterators[2].getPosition()
			};
			
			Vector const normals[3] =
			{
				vertexBufferIterators[0].getNormal(),
					vertexBufferIterators[1].getNormal(),
					vertexBufferIterators[2].getNormal()
			};
			
			float const triangleVertexDistance[3] =
			{
				plane.computeDistanceTo(corners[0]),
					plane.computeDistanceTo(corners[1]),
					plane.computeDistanceTo(corners[2])
			};
			
			float const epsilon = 0.001f;
			
			int const numVerticesFront = 
				((triangleVertexDistance[0] > epsilon) ? 1 : 0) + 
				((triangleVertexDistance[1] > epsilon) ? 1 : 0) + 
				((triangleVertexDistance[2] > epsilon) ? 1 : 0);
			
			int const numVerticesBack = 
				((triangleVertexDistance[0] < -epsilon) ? 1 : 0) + 
				((triangleVertexDistance[1] < -epsilon) ? 1 : 0) + 
				((triangleVertexDistance[2] < -epsilon) ? 1 : 0);
			
			int numVerticesPlanar = (3 - numVerticesFront) - numVerticesBack;
			
			//-- keep the edge for later, so we can cap the hole
			if (numVerticesPlanar == 2)
			{
				Edge edge;
				int edgeVectorIndex = 0;
				if (WithinEpsilonInclusive(0.0f, triangleVertexDistance[0], epsilon))
					edge.v[edgeVectorIndex++] = corners[0];
				if (WithinEpsilonInclusive(0.0f, triangleVertexDistance[1], epsilon))
					edge.v[edgeVectorIndex++] = corners[1];
				if (WithinEpsilonInclusive(0.0f, triangleVertexDistance[2], epsilon))
				{
					DEBUG_FATAL(edgeVectorIndex >= 2, ("DynamicMeshAppearanceShaderSet out of bounds"));
					edge.v[edgeVectorIndex++] = corners[2];
				}

				DEBUG_FATAL(edgeVectorIndex != 2, ("SplitMeshTest generate edges failed"));
				
				edges.push_back(edge); //lint !e645 //edge init
			}
			
			if ((numVerticesFront + numVerticesPlanar) == 3)
			{
				indexVectorFront.push_back(triangleCornerIndices[0]);
				indexVectorFront.push_back(triangleCornerIndices[1]);
				indexVectorFront.push_back(triangleCornerIndices[2]);
			}
			else if ((numVerticesBack + numVerticesPlanar) == 3)
			{
				indexVectorBack.push_back(triangleCornerIndices[0]);
				indexVectorBack.push_back(triangleCornerIndices[1]);
				indexVectorBack.push_back(triangleCornerIndices[2]);
			}
			else
			{
				VertexInfo intersections[3];
				int numIntersections = 0;
				int oppositeCorners[3] = {-1, -1, -1};
				int frontCorner = 0;
				
				{
					float frontCornerDistance = 0.0f;
					for (int j = 0; j < 3; ++j)
					{
						if (triangleVertexDistance[j] > frontCornerDistance)
						{
							frontCornerDistance = triangleVertexDistance[j];
							frontCorner = j;
						}
					}
				}
				
				{
					//-- each element is corner0, corner1, opposite corner
					int const cornersToCheck[3][3] =
					{
						{0, 1, 2},
						{1, 2, 0},
						{2, 0, 1}
					};
					
					for (int j = 0; j < 3; ++j)
					{
						int const corner0 = cornersToCheck[j][0];
						int const corner1 = cornersToCheck[j][1];
						int const oppositeCorner = cornersToCheck[j][2];
						
						float intersectionT = 0.0f;
						VertexInfo & vertexInfo = intersections[numIntersections];

						if (!WithinEpsilonInclusive(0.0f, triangleVertexDistance[corner0], epsilon) &&
							!WithinEpsilonInclusive(0.0f, triangleVertexDistance[corner1], epsilon))
						{
							//-- only test segment/plane intersections when it is known that neither corner lies on the plane
							if (plane.findIntersection(corners[corner0], corners[corner1], vertexInfo.pos, intersectionT))
							{
								oppositeCorners[numIntersections] = oppositeCorner;
								vertexInfo.setup(vertexBufferIterators[corner0], vertexBufferIterators[corner1], intersectionT);
								++numIntersections;
							}							
						}
					}
				}
				
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(1, numIntersections, 3);//, ("SplitMeshTest numIntersections bad [%d]", numIntersections));
				
				int const oldExtraVertsSize = static_cast<int>(extraVerts.size());
				
				{
					for (int j = 0; j < numIntersections; ++j)
					{
						extraVerts.push_back(intersections[j]);
					}
				}
				
				uint16 const newIndices[2] =
				{
					static_cast<uint16>(numberOfVertices + oldExtraVertsSize),
						static_cast<uint16>(numberOfVertices + oldExtraVertsSize + 1),
				};
				
				VertexInfo oldVerts[3];
				oldVerts[0].pos = corners[0];
				oldVerts[0].normal = normals[0];
				oldVerts[1].pos = corners[1];
				oldVerts[1].normal = normals[1];
				oldVerts[2].pos = corners[2];
				oldVerts[2].normal = normals[2];
				
				if (numIntersections == 1)
				{
					int backCorner = 0;
					if (frontCorner != 0 && oppositeCorners[0] != 0)
						backCorner = 0;
					else if (frontCorner != 1 && oppositeCorners[0] != 1)
						backCorner = 1;
					else if (frontCorner != 2 && oppositeCorners[0] != 2)
						backCorner = 2;
					
					DEBUG_FATAL((oppositeCorners[0] == frontCorner || oppositeCorners[0] == backCorner), ("SplitMeshTest failed with 1 intersection.  The corner opposite the intersection must not be the front or back corner"));
					
					getProperlyWoundTriangle(oldVerts[oppositeCorners[0]], oldVerts[frontCorner], intersections[0],
						triangleCornerIndices[oppositeCorners[0]], triangleCornerIndices[frontCorner], newIndices[0], 
						indexVectorFront);
					
					getProperlyWoundTriangle(oldVerts[oppositeCorners[0]], oldVerts[backCorner], intersections[0],
						triangleCornerIndices[oppositeCorners[0]], triangleCornerIndices[backCorner], newIndices[0],
						indexVectorBack);
					
					//-- keep the edge for later, so we can cap the hole
					Edge edge;
					edge.v[0] = oldVerts[oppositeCorners[0]].pos;
					edge.v[1] = intersections[0].pos;
					edges.push_back(edge);
				}
				else if (numIntersections == 2)
				{
					IndexVector * indexVectorSingle = NULL;
					IndexVector * indexVectorDouble = NULL;
					
					int singleCorner = 0;
					
					if (oppositeCorners[0] != 0 && oppositeCorners[1] != 0)
						singleCorner = 0;
					else if (oppositeCorners[0] != 1 && oppositeCorners[1] != 1)
						singleCorner = 1;
					else
						singleCorner = 2;
					
					//-- the double-triangle side of the split is the front, add 2 new triangles to the front mesh, 1 to the back
					if (frontCorner == oppositeCorners[0] || frontCorner == oppositeCorners[1])
					{
						indexVectorSingle = &indexVectorBack;
						indexVectorDouble = &indexVectorFront;
					}
					//-- the single-triangle side of the split is in the front, add 2 new triangles to the front mesh, 1 to the back
					else
					{
						indexVectorSingle = &indexVectorFront;
						indexVectorDouble = &indexVectorBack;
					}
					
					getProperlyWoundTriangle(oldVerts[oppositeCorners[0]], intersections[0], intersections[1],
						triangleCornerIndices[oppositeCorners[0]], newIndices[0], newIndices[1],
						*indexVectorDouble);
					
					getProperlyWoundTriangle(oldVerts[oppositeCorners[0]], intersections[0], oldVerts[oppositeCorners[1]],
						triangleCornerIndices[oppositeCorners[0]], newIndices[0], triangleCornerIndices[oppositeCorners[1]], 
						*indexVectorDouble);
					
					getProperlyWoundTriangle(oldVerts[singleCorner], intersections[0], intersections[1],
						triangleCornerIndices[singleCorner], newIndices[0], newIndices[1],
						*indexVectorSingle);
					
					//-- keep the edge for later, so we can cap the hole
					Edge edge;
					edge.v[0] = intersections[0].pos;
					edge.v[1] = intersections[1].pos;
					edges.push_back(edge);
				}
			}
		}
		
		inputIndexBuffer.unlock();
		inputVertexBuffer.unlock();
	}
	
	//-- randomize verts
	//-- be careful to keep everything welded together
	{
		if (randomization != Vector::zero)
		{
			float const epsilon = 0.001f;
			
			for (VertexInfoVector::const_iterator it = extraVerts.begin(); it != extraVerts.end(); ++it)
			{
				VertexInfo const & oldVertexInfo = *it;
				
				if (oldVertexInfo.randomized)
					continue;
				
				Vector const randomVector (Random::randomReal(-randomization.x, randomization.x), Random::randomReal(-randomization.y, randomization.y), Random::randomReal(-randomization.z, randomization.z));
				Vector const oldPos = oldVertexInfo.pos;
				
				for (VertexInfoVector::iterator check_it = extraVerts.begin(); check_it != extraVerts.end(); ++check_it)
				{
					VertexInfo & checkVertexInfo = *check_it;
					Vector & posToTest = checkVertexInfo.pos;
					if (posToTest.magnitudeBetweenSquared(oldPos) < epsilon)
					{
						posToTest += randomVector;
						checkVertexInfo.randomized = true;
					}
				}
			}
		}
	}

	IndexVector indexVectorFrontSortedUnique = indexVectorFront;
	std::sort(indexVectorFrontSortedUnique.begin(), indexVectorFrontSortedUnique.end());
	IGNORE_RETURN(indexVectorFrontSortedUnique.erase(std::unique(indexVectorFrontSortedUnique.begin(), indexVectorFrontSortedUnique.end()), indexVectorFrontSortedUnique.end()));
	
	IndexVector indexVectorBackSortedUnique = indexVectorBack;
	std::sort(indexVectorBackSortedUnique.begin(), indexVectorBackSortedUnique.end());
	IGNORE_RETURN(indexVectorBackSortedUnique.erase(std::unique(indexVectorBackSortedUnique.begin(), indexVectorBackSortedUnique.end()), indexVectorBackSortedUnique.end()));
	
	StaticVertexBuffer * const newInputVertexBufferFront = indexVectorFrontSortedUnique.empty() ? NULL : new StaticVertexBuffer(inputVertexBuffer.getFormat(), static_cast<int>(indexVectorFrontSortedUnique.size()));
	StaticVertexBuffer * const newInputVertexBufferBack = indexVectorBackSortedUnique.empty() ? NULL : new StaticVertexBuffer(inputVertexBuffer.getFormat(), static_cast<int>(indexVectorBackSortedUnique.size()));

	typedef stdmap<uint16, uint16>::fwd IndexMapping;

	IndexMapping indexMappingFront;
	IndexMapping indexMappingBack;

	DEBUG_FATAL(newInputVertexBufferFront == NULL && newInputVertexBufferBack == NULL, ("DynamicMeshAppearanceShaderSet neither front nor back vertex buffer has nonzero size"));

	{
		if (newInputVertexBufferFront != NULL)
			newInputVertexBufferFront->lock();
		if (newInputVertexBufferBack != NULL)
			newInputVertexBufferBack->lock();

		inputVertexBuffer.lockReadOnly();
		
		//-- we must initialize these iterators even if the underlying vertex buffer is null.
		//-- we won't use the invalid ones

		VertexBufferReadWriteIterator v_out_front = newInputVertexBufferFront != NULL ? newInputVertexBufferFront->begin() : newInputVertexBufferBack->begin(); //lint !e613 //not possible null deref
		VertexBufferReadWriteIterator v_out_back = newInputVertexBufferBack != NULL ? newInputVertexBufferBack->begin() : newInputVertexBufferFront->begin(); //lint !e613 //not possible null deref
		
		uint16 inputVertexIndex = 0;
		uint16 outputVertexIndexFront = 0;
		uint16 outputVertexIndexBack = 0;
			
		int const numberOfInputVertices = inputVertexBuffer.getNumberOfVertices();
		for (VertexBufferReadIterator v_in = inputVertexBuffer.beginReadOnly(); inputVertexIndex < numberOfInputVertices; ++inputVertexIndex, ++v_in)
		{
			if (newInputVertexBufferFront != NULL && std::binary_search(indexVectorFrontSortedUnique.begin(), indexVectorFrontSortedUnique.end(), inputVertexIndex))
			{
				v_out_front.copy(v_in);
				++v_out_front;
				IGNORE_RETURN(indexMappingFront.insert(std::make_pair(inputVertexIndex, outputVertexIndexFront++)));
			}

			if (newInputVertexBufferBack != NULL && std::binary_search(indexVectorBackSortedUnique.begin(), indexVectorBackSortedUnique.end(), inputVertexIndex))
			{
				v_out_back.copy(v_in);
				++v_out_back;
				IGNORE_RETURN(indexMappingBack.insert(std::make_pair(inputVertexIndex, outputVertexIndexBack++)));
			}
		}

		//MemoryManager::verify(true, true);
		
		bool const hasColor0 = inputVertexBuffer.hasColor0();
		bool const hasColor1 = inputVertexBuffer.hasColor1();

		for (VertexInfoVector::const_iterator it = extraVerts.begin(); it != extraVerts.end(); ++it, ++inputVertexIndex)
		{
			if (newInputVertexBufferFront != NULL && std::binary_search(indexVectorFrontSortedUnique.begin(), indexVectorFrontSortedUnique.end(), inputVertexIndex))
			{
				VertexInfo const & vertexInfo = *it;
				vertexInfo.apply(v_out_front);
				
				if (hasColor0)
					v_out_front.setColor0(PackedArgb::solidRed);
				if (hasColor1)
					v_out_front.setColor1(PackedArgb::solidRed);
				
				++v_out_front;
				IGNORE_RETURN(indexMappingFront.insert(std::make_pair(inputVertexIndex, outputVertexIndexFront++)));
			}
			if (newInputVertexBufferBack != NULL && std::binary_search(indexVectorBackSortedUnique.begin(), indexVectorBackSortedUnique.end(), inputVertexIndex))
			{
				VertexInfo const & vertexInfo = *it;
				vertexInfo.apply(v_out_back);
				
				if (hasColor0)
					v_out_back.setColor0(PackedArgb::solidRed);
				if (hasColor1)
					v_out_back.setColor1(PackedArgb::solidRed);
				
				++v_out_back;
				IGNORE_RETURN(indexMappingBack.insert(std::make_pair(inputVertexIndex, outputVertexIndexBack++)));
			}
		}
		

		inputVertexBuffer.unlock();

		if (newInputVertexBufferFront != NULL)
			newInputVertexBufferFront->unlock();
		if (newInputVertexBufferBack != NULL)
			newInputVertexBufferBack->unlock();
	}
	
	if (!indexVectorFront.empty())
	{
		for (IndexVector::iterator it = indexVectorFront.begin(); it != indexVectorFront.end(); ++it)
		{
			uint16 & index = *it;
			IndexMapping::const_iterator map_it = indexMappingFront.find(index);
			DEBUG_FATAL(map_it == indexMappingFront.end(), ("DynamicMeshAppearanceShaderSet index mapping failed for front"));
			index = (*map_it).second;
		}
		
		outputFront = new DynamicMeshAppearanceShaderSet(shader, *NON_NULL(newInputVertexBufferFront), indexVectorFront);
	}
	
	if (!indexVectorBack.empty())
	{
		for (IndexVector::iterator it = indexVectorBack.begin(); it != indexVectorBack.end(); ++it)
		{
			uint16 & index = *it;
			IndexMapping::const_iterator map_it = indexMappingBack.find(index);
			DEBUG_FATAL(map_it == indexMappingBack.end(), ("DynamicMeshAppearanceShaderSet index mapping failed for back"));
			index = (*map_it).second;
		}
		
		outputBack = new DynamicMeshAppearanceShaderSet(shader, *NON_NULL(newInputVertexBufferBack), indexVectorBack);
	}
	
	//MemoryManager::verify(true, true);

	delete newInputVertexBufferFront;
	delete newInputVertexBufferBack;
}

//----------------------------------------------------------------------

AxialBox DynamicMeshAppearanceShaderSet::calculateAxialBox() const
{
	NOT_NULL(m_vertexBuffer);

	AxialBox box;

	m_vertexBuffer->lockReadOnly();
	for (VertexBufferReadIterator v_it = m_vertexBuffer->beginReadOnly(); v_it != m_vertexBuffer->end(); ++v_it)
	{
		Vector const & pos = v_it.getPosition();

		box.add(pos);
	}

	m_vertexBuffer->unlock();

	return box;
}


//======================================================================
