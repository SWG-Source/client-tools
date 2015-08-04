// ======================================================================
//
// StaticMeshBuilder.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "StaticMeshBuilder.h"

#include "ExporterLog.h"
#include "MayaUtility.h"
#include "NormalMapUtility.h"
#include "Messenger.h"
#include "ShaderPrimitiveSetWriter.h"
#include "VertexIndexer.h"
#include "TriangleRasterizer.h"

#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Quaternion.h"
#include "sharedObject/Hardpoint.h"
#include "sharedImage/Image.h"
#include "sharedImage/TargaFormat.h"

#include "maya/MDagPath.h"
#include "maya/MGlobal.h"
#include "maya/MMatrix.h"
#include "maya/MObject.h"
#include "maya/MSelectionList.h"

#include "nvtristrip.h"

#include <map>
#include <io.h>

// ======================================================================

#define VERBOSE_GEOMETRY_LOGGING 0
#define GENERATE_STRIPS 1
#define REMAP_INDICES 0

static const Tag TAG_MESH = TAG (M,E,S,H);

extern Messenger * messenger;

// ======================================================================

StaticMeshBuilder::ShaderData::ShaderData(const char *newName, bool i_hasDot3, int i_normalMapTextureCoordinateIndex)
:	next(0),
	shaderTemplateName(newName),
	faces(),
	m_wrapU(false),
	m_wrapV(false),
	hasDot3TextureCoordinate(i_hasDot3),
	dot3TextureCoordinateIndex(-1),
	normalMapTextureCoordinateIndex(i_normalMapTextureCoordinateIndex),
	m_heightMapScale(1)
{
	MESSENGER_INDENT;
}

// ----------------------------------------------------------------------

StaticMeshBuilder::ShaderData::~ShaderData(void)
{
	MESSENGER_INDENT;

	next = 0;
}

// ----------------------------------------------------------------------

void StaticMeshBuilder::ShaderData::setShaderNormalsInfo(
	const char   *i_normalsBaseFileName, 
	CompressType  i_normalsCompressType,
	bool          wrapU,
	bool          wrapV,
	float         i_heightMapScale
)
{
	m_normalsCompressType=i_normalsCompressType;
	m_wrapU=wrapU;
	m_wrapV=wrapV;
	m_heightMapScale=i_heightMapScale;
	if (i_normalsBaseFileName)
	{
		normalsBaseFileName=i_normalsBaseFileName;
	}
	else
	{
		normalsBaseFileName.clear();
	}
}

// ----------------------------------------------------------------------

void StaticMeshBuilder::ShaderData::getNormalsFileNames(
	std::string *o_tangentNormalsFileName,
	std::string *o_worldNormalsFileName,
	std::string *o_scratchMapFileName
) const
{
	int baseLength = normalsBaseFileName.size();

	if (o_tangentNormalsFileName)
	{
		o_tangentNormalsFileName->clear();
		if (baseLength)
		{
			o_tangentNormalsFileName->reserve(baseLength + 7);
			(*o_tangentNormalsFileName)=normalsBaseFileName;
			if (m_normalsCompressType==CT_normalMap)
			{
				o_tangentNormalsFileName->append("_cn.tga");
			}
			else
			{
				o_tangentNormalsFileName->append("_n.tga");
			}
		}
	}

	if (o_worldNormalsFileName)
	{
		o_worldNormalsFileName->clear();
		if (baseLength)
		{
			o_worldNormalsFileName->reserve(baseLength + 7);
			(*o_worldNormalsFileName)=normalsBaseFileName;
			o_worldNormalsFileName->append("_wn.tga");
		}
	}

	if (o_scratchMapFileName)
	{
		o_scratchMapFileName->clear();
		if (baseLength)
		{
			o_scratchMapFileName->reserve(baseLength + 7);
			(*o_scratchMapFileName)=normalsBaseFileName;
			o_scratchMapFileName->append("_hm.tga");
		}
	}
}

// ----------------------------------------------------------------------
#define SKIP_SKIRT_AND_FILL      0
#define SKIP_TANGENT_SPACE_XFORM 0
#define IGNORE_WORLD_NORMALS     0
#define IGNORE_HEIGHT_MAP        0
#define MIX_FLOAT_NORMALS        0

namespace StaticMeshBuilderNamespace
{
	typedef std::vector<const StaticMeshBuilder::ShaderData *> ShaderDataList;
	typedef std::map<std::string, ShaderDataList> File2Shaders;

	typedef std::vector<int> IntList;
	typedef std::map<std::string, IntList> Shader2Primitive;

	struct BlendedDot3Transform
	{
		// --------------
		// matching components
		Vector position;
		Vector normal;
		bool   flipped;
		// --------------
		// blended components
		DoubleVector du;
		DoubleVector dv;
		int    count;

		DoubleVector flippedDu;
		DoubleVector flippedDv;
		int    flippedCount;
	};

	typedef std::vector<BlendedDot3Transform> BlendedDot3TransformList;

	struct NormalTexelMixer
	{
	#if MIX_FLOAT_NORMALS
		float x, y, z;
	#else
		short x, y, z;
	#endif
		uint8 alpha;
		uint8 wasSet;
	};

	static float _bestSlope(
		float x1, float y1, 
		float x2, float y2, 
		float x3, float y3
	);
}
using namespace StaticMeshBuilderNamespace;

static DoubleVector _mirrorVector(const DoubleVector &i_vector, const DoubleVector &i_mirrorNormal)
{
	double d = i_mirrorNormal.dot(i_vector);
	return i_vector - 2.0f * d * i_mirrorNormal;
}

#pragma warning (disable: 4189)
bool StaticMeshBuilder::ShaderData::build(ShaderPrimitiveSetWriter *writer, int priority)
{
	MESSENGER_INDENT;
	MESSENGER_REJECT(!writer, ("null writer arg\n"));
	{
		REPORT_LOG_PRINT(true, ("*************************\n"));
		REPORT_LOG_PRINT(true, ("Shader %s\n", shaderTemplateName.c_str()));
		MESSENGER_LOG(("Shader %s\n", shaderTemplateName.c_str()));
		MESSENGER_INDENT;

		if (!faces.empty())
		{
			const int    indexCount = 3 * faces.size();
			MESSENGER_REJECT(indexCount > 65535, ("too many polygons for shader %s. %i is the limit.\n", shaderTemplateName.c_str(), 65536 / 3));

			Index *const indexArray = new Index[static_cast<size_t>(indexCount)];
			MESSENGER_REJECT(!indexArray, ("null indexArray\n"));

			unsigned int index;
			VertexIndexer          vi;
			VertexIndexer::Vertex  viVertex;

			vi.reserve((faces.size() * 3) / 2);

			BlendedDot3TransformList blendedDot3TransformList;

			// build up a list of all the dot3 transforms affecting each vertex
			for (index = 0; index < faces.size(); ++index)
			{
				MeshBuilder::Face const & face = *faces[index];

				if (!face.dot3TransformMatrixValid)
				{
					continue;
				}

				const bool flipped = face.dot3TransformMatrixFlipped;

				for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
				{
					const DoubleVector  vertexPosition = face.positionArray[vertexIndex];
					const DoubleVector  vertexNormal   = face.vertexNormalArray[vertexIndex];

					// project du and dv onto the vertex's tangent plane.
					DoubleVector du, dv;

					du = face.du - (vertexNormal * face.du.dot(vertexNormal));
					du.normalize();

					dv = face.dv - (vertexNormal * face.dv.dot(vertexNormal));
					dv.normalize();

					BlendedDot3TransformList::iterator i = blendedDot3TransformList.begin();
					BlendedDot3TransformList::iterator iEnd = blendedDot3TransformList.end();
					for ( ; i != iEnd; ++i)
					{
						BlendedDot3Transform &t = *i;

						if (  face.positionArray[vertexIndex] == t.position 
							&& face.vertexNormalArray[vertexIndex] == t.normal
							)
						{
							if (flipped)
							{
								t.flippedDu += face.du;
								t.flippedDv += face.dv;
								t.flippedCount += 1;
							}
							else
							{
								t.du += face.du;
								t.dv += face.dv;
								t.count += 1;
							}
							break;
						}
					}

					if (i == iEnd)
					{
						BlendedDot3Transform t;
						t.position = vertexPosition;
						t.normal = vertexNormal;

						if (flipped)
						{
							t.du = DoubleVector::zero;
							t.dv = DoubleVector::zero;
							t.count = 0;

							t.flippedDu = du;
							t.flippedDv = dv;
							t.flippedCount = 1;
						}
						else
						{
							t.du = du;
							t.dv = dv;
							t.count = 1;

							t.flippedDu = DoubleVector::zero;
							t.flippedDv = DoubleVector::zero;
							t.flippedCount = 0;
						}

						blendedDot3TransformList.push_back(t);
					}
				}
			}

			const int numDot3Transforms = blendedDot3TransformList.size();
			REPORT_LOG_PRINT(true, ("Created %i dot3 transforms.\n", numDot3Transforms));

			// average the dot3 transforms at each vertex
			{
				BlendedDot3TransformList::iterator iEnd = blendedDot3TransformList.end();
				for (BlendedDot3TransformList::iterator i = blendedDot3TransformList.begin(); i != iEnd; ++i)
				{
					BlendedDot3Transform &t = *i;
					DoubleVector ueqv, ueqvFlipped;

					const DoubleVector tNormal=t.normal;

					if (t.count)
					{
						ueqv = t.du + t.dv;
						ueqv.normalize();
					}

					if (t.flippedCount)
					{
						ueqvFlipped = t.flippedDu + t.flippedDv;
						ueqvFlipped.normalize();
					}

					if (t.count && t.flippedCount)
					{
						DoubleVector mirror;
						mirror = ueqv - ueqvFlipped;
						double mag = mirror.magnitude();
						if (mag>(1.0f/1024.0f))
						{
							mirror *= 1.0f / mag;
							DoubleVector du2 = _mirrorVector(t.flippedDu, mirror);
							DoubleVector dv2 = _mirrorVector(t.flippedDv, mirror);
							DoubleVector ueqv2 = t.du + du2 + t.dv + dv2;
							ueqv2.normalize();

							ueqv = ueqv2;
							ueqvFlipped = _mirrorVector(ueqv2, mirror);
						}
					}

					if (t.count)
					{
						DoubleVector midt90 = ueqv.cross(tNormal);
						midt90.normalize();

						DoubleVector du = ueqv + midt90;
						du.normalize();

						DoubleVector dv = ueqv - midt90;
						dv.normalize();

						// project du and dv onto the vertex's tangent plane.
						du = du - (tNormal * du.dot(tNormal));
						du.normalize();

						dv = dv - (tNormal * dv.dot(tNormal));
						dv.normalize();

						t.du=du;
						t.dv=dv;
					}

					if (t.flippedCount)
					{
						DoubleVector midt90 = ueqvFlipped.cross(tNormal);
						midt90.normalize();
						midt90 = -midt90;

						DoubleVector flippedDu = ueqvFlipped + midt90;
						flippedDu.normalize();

						DoubleVector flippedDv = ueqvFlipped - midt90;
						flippedDv.normalize();

						// project flippedDu and flippedDv onto the vertex's tangent plane.
						flippedDu = flippedDu - (tNormal * flippedDu.dot(tNormal));
						flippedDu.normalize();

						flippedDv = flippedDv - (tNormal * flippedDv.dot(tNormal));
						flippedDv.normalize();

						t.flippedDu=flippedDu;
						t.flippedDv=flippedDv;
					}
				}
			}

			// figure out the vertex buffer format to write out
			bool hasDiffuse = true;
			bool hasDot3 = true;
			int numberOfTextureCoordinates = VertexIndexer::MAX_TEXTURE_COORDINATE_SETS;
			for (index = 0; index < faces.size(); ++index)
			{
				MeshBuilder::Face const & face = *faces[index];

				if (!face.diffuseColorsValid)
					hasDiffuse = false;
				
				if (!face.dot3TransformMatrixValid)
					hasDot3 = false;

				if (face.uvCount < numberOfTextureCoordinates)
					numberOfTextureCoordinates = face.uvCount;
			}

			// let the vertex indexer know what elements to compare
			vi.addPosition();
			vi.addNormal();
			if (hasDiffuse)
			{
				vi.addColor0();
			}
			for (int i = 0; i < numberOfTextureCoordinates; ++i)
			{
				vi.addTextureCoordinateSet(2);
			}
			if (hasDot3)
			{
				vi.addTextureCoordinateSet(4);

				// Keep track of this fact so we can properly set the
				// DOT3 texture set tag in the ShaderTemplate.
				hasDot3TextureCoordinate   = true;
				dot3TextureCoordinateIndex = vi.getNumberOfTextureCoordinateSets() - 1;
			}
					
			// let the user know what format we will be writing out
			{
				std::string first;
				vi.getVertexBufferFormat().formatFormat(first);
				MESSENGER_LOG(("Using vertex buffer format: %s\n", first.c_str()));
			}

			bool colorWarned = false;
			bool textureCoordinateSetWarned = false;
			bool dot3Warned = false;
			Index                  *currentIndex;
			for (index = 0, currentIndex = indexArray; index < faces.size(); ++index, currentIndex += 3)
			{
				MeshBuilder::Face const & face = *faces[index];

				for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
				{
					Zero(viVertex);

					viVertex.position = face.positionArray[vertexIndex];
					viVertex.normal = face.vertexNormalArray[vertexIndex];

					if (hasDiffuse)
					{
						viVertex.color0 = face.vertexDiffuseColorArray[vertexIndex];
					}
					else
						if (!colorWarned && face.diffuseColorsValid)
						{
							colorWarned = true;
							MESSENGER_LOG_WARNING(("Some, but not all, vertices have color in %s <%1.2f, %1.2f, %1.2f>\n", shaderTemplateName.c_str(), -viVertex.position.x, viVertex.position.y, viVertex.position.z));
						}

					for (int i = 0; i < numberOfTextureCoordinates; i++)
					{
						viVertex.textureCoordinateSets[i].coordinate[0] = face.uArray[i][vertexIndex];
						viVertex.textureCoordinateSets[i].coordinate[1] = face.vArray[i][vertexIndex];
					}

					if (!textureCoordinateSetWarned && face.uvCount > numberOfTextureCoordinates)
					{
						textureCoordinateSetWarned = true;
						MESSENGER_LOG_WARNING(("Some, but not all, vertices have extra texture coordinate sets %d vs %d in %s <%1.2f, %1.2f, %1.2f>\n", face.uvCount, numberOfTextureCoordinates, shaderTemplateName.c_str(), -viVertex.position.x, viVertex.position.y, viVertex.position.z));
					}

					if (hasDot3)
					{
						BlendedDot3TransformList::iterator i = blendedDot3TransformList.begin();
						BlendedDot3TransformList::iterator iEnd = blendedDot3TransformList.end();
						for ( ; i != iEnd; ++i)
						{
							BlendedDot3Transform &t = *i;

							if (	face.positionArray[vertexIndex]     == t.position 
								&& face.vertexNormalArray[vertexIndex] == t.normal
								)
							{
								if (face.dot3TransformMatrixFlipped)
								{
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[0] = float(t.flippedDu.x);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[1] = float(t.flippedDu.y);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[2] = float(t.flippedDu.z);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[3] = -1.0f;
								}
								else
								{
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[0] = float(t.du.x);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[1] = float(t.du.y);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[2] = float(t.du.z);
									viVertex.textureCoordinateSets[numberOfTextureCoordinates].coordinate[3] = 1.0f;
								}

								break;
							}
						}

						if (i == iEnd)
						{
							MESSENGER_REJECT(true, ("pb: could not find correct transform\n"));
						}
					}
					else
					{
						if (!dot3Warned && face.dot3TransformMatrixValid)
						{
							dot3Warned = true;
							MESSENGER_LOG_WARNING(("Some, but not all, vertices have dot3 transform matrices in %s <%1.2f, %1.2f, %1.2f>\n", shaderTemplateName.c_str(), -viVertex.position.x, viVertex.position.y, viVertex.position.z));
						}
					}

				#if VERBOSE_GEOMETRY_LOGGING
					REPORT_LOG_PRINT(true, ("Vertex:%+1.3f, %+1.3f, %+1.3f Normal:%+1.3f, %+1.3f, %+1.3f\n", double(viVertex.position.x), double(viVertex.position.y), double(viVertex.position.z), double(viVertex.normal.x), double(viVertex.normal.y), double(viVertex.normal.z)));
				#endif
					currentIndex[vertexIndex] = static_cast<Index>(vi.addVertex(viVertex));
				}
			}

			const int numberOfVertices = vi.getNumberOfVertices();
			REPORT_LOG_PRINT(true, ("Created %i vertices.\n", numberOfVertices));

#if GENERATE_STRIPS

			// reorder the indices for better cache coherency
			SetCacheSize(CACHESIZE_GEFORCE1_2);
			SetListsOnly(true);
			PrimitiveGroup *primitiveGroups = 0;
			unsigned short numberOfPrimitiveGroups = 0;
			GenerateStrips(indexArray, indexCount, &primitiveGroups, &numberOfPrimitiveGroups);
			MESSENGER_REJECT(numberOfPrimitiveGroups != 1, ("pb: optimization produced multiple primitive groups\n"));

	#if REMAP_INDICES
			// build the remapping array
			PrimitiveGroup *reorderedPrimitiveGroups = 0;
			RemapIndices(primitiveGroups, numberOfPrimitiveGroups, static_cast<ushort>(numberOfVertices), &reorderedPrimitiveGroups);

			int *vertexRemapArray = new int[numberOfVertices];
			{
				for (int i = 0; i < numberOfVertices; ++i)
					vertexRemapArray[i] = -1;
			}
	#endif

			{
				for (uint i = 0; i < primitiveGroups[0].numIndices; ++i)
				{
	#if REMAP_INDICES
					indexArray[i] = reorderedPrimitiveGroups[0].indices[i];
					vertexRemapArray[reorderedPrimitiveGroups[0].indices[i]] = primitiveGroups[0].indices[i];
	#else
					indexArray[i] = primitiveGroups[0].indices[i];
	#endif

				}
			}

	#if REMAP_INDICES
			{
				for (int i = 0; i < numberOfVertices; ++i)
					MESSENGER_REJECT(vertexRemapArray[i] < 0, ("vertex %d did not get remapped", i));
			}
	#endif

			// create new vertex buffer and fill it in
			SystemVertexBuffer *reorderedSystemVertexBuffer = new SystemVertexBuffer(vi.getVertexBufferFormat(), numberOfVertices);
			{
			#if VERBOSE_GEOMETRY_LOGGING
				REPORT_LOG_PRINT(true, ("Writing vertex buffer:\n"));
			#endif

				VertexBufferWriteIterator viIterator = reorderedSystemVertexBuffer->begin();
				for (int i = 0; i < numberOfVertices; ++i, ++viIterator)
				{
				#if REMAP_INDICES
					int vertexIndex=vertexRemapArray[i];
				#else
					int vertexIndex=i;
				#endif

				#if VERBOSE_GEOMETRY_LOGGING
					const VertexIndexer::Vertex &viVertex = vi.getVertex(vertexIndex);
					REPORT_LOG_PRINT(true, ("V:%+1.5f,%+1.5f,%+1.5f  ", double(viVertex.position.x), double(viVertex.position.y), double(viVertex.position.z)));
					REPORT_LOG_PRINT(true, ("N:%+1.3f,%+1.3f,%+1.3f  ", double(viVertex.normal.x), double(viVertex.normal.y), double(viVertex.normal.z)));
					const float *dot3tc = viVertex.textureCoordinateSets[dot3TextureCoordinateIndex].coordinate;
					REPORT_LOG_PRINT(true, ("D:%+1.5f,%+1.5f,%+1.5f,%+1.1f  ", double(dot3tc[0]), double(dot3tc[1]), double(dot3tc[2]), double(dot3tc[3])));
					REPORT_LOG_PRINT(true, ("\n"));
				#endif

					vi.writeVertex(vertexIndex, viIterator);
				}
			}

			// don't need any of this stuff anymore
			delete [] primitiveGroups;

	#if REMAP_INDICES
			delete [] reorderedPrimitiveGroups;
			delete [] vertexRemapArray;
	#endif

#else // !GENERATE_STRIPS
			// create new vertex buffer and fill it in
			SystemVertexBuffer *reorderedSystemVertexBuffer = new SystemVertexBuffer(vi.getVertexBufferFormat(), numberOfVertices);
			{
				VertexBufferWriteIterator viIterator = reorderedSystemVertexBuffer->begin();
				for (int i = 0; i < numberOfVertices; ++i, ++viIterator)
				{
					vi.writeVertex(i, viIterator);
				}
			}
#endif
			writer->add(shaderTemplateName.c_str(), SPSPT_indexedTriangleList, reorderedSystemVertexBuffer, indexCount, indexArray, priority);

			MESSENGER_LOG(("%d triangles, %d merged vertices\n", faces.size(), numberOfVertices, shaderTemplateName.c_str()));
		}
		else
		{
			MESSENGER_LOG(("No geometry found\n"));
		}
	}

	return true;
}

// ======================================================================

StaticMeshBuilder::StaticMeshBuilder(MObject const & rootMayaObject) :
	MeshBuilder(rootMayaObject),
	m_setWriter(new ShaderPrimitiveSetWriter(true)),
	m_shaderTemplateCount(0),
	m_firstShaderData(0),
	m_lastShaderData(0),
	m_hasAlpha(false)
{
	MESSENGER_INDENT;
}

// ----------------------------------------------------------------------

StaticMeshBuilder::~StaticMeshBuilder(void)
{
	MESSENGER_INDENT;

	while (m_firstShaderData)
	{
		ShaderData *const deadShaderData = m_firstShaderData;
		m_firstShaderData = m_firstShaderData->next;
		delete deadShaderData;
	}

	delete m_setWriter;
	m_lastShaderData = NULL;

	delete m_extent;
	m_extent = NULL;
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::hasFrames (void) const
{
	return false;
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::setShaderTemplate(
	const char *shaderTemplateName, 
	int        *shaderTemplateIndex, 
	bool        dot3, 
	int         normalMapTextureCoordinateIndex
	)
{
	MeshBuilder::setShaderTemplate(shaderTemplateName, shaderTemplateIndex, dot3, normalMapTextureCoordinateIndex);

	MESSENGER_INDENT;
	MESSENGER_REJECT(!shaderTemplateIndex, ("null shaderTemplateIndex arg\n"));
	*shaderTemplateIndex = m_shaderTemplateCount;

	++m_shaderTemplateCount;
	ShaderData *newShaderData = new ShaderData(shaderTemplateName, dot3, normalMapTextureCoordinateIndex);

	// link to list
	if (m_lastShaderData)
		m_lastShaderData->next = newShaderData;
	else
		m_firstShaderData = newShaderData;
	m_lastShaderData = newShaderData;

	return true;
}

// ----------------------------------------------------------------------

void StaticMeshBuilder::setShaderNormalsInfo(
	const char   *i_normalsBaseFileName, 
	CompressType  i_normalsCompressType,
	bool          wrapU,
	bool          wrapV,
	float         heightMapScale
)
{
	if (m_lastShaderData)
	{
		m_lastShaderData->setShaderNormalsInfo(
			i_normalsBaseFileName,
			i_normalsCompressType,
			wrapU,
			wrapV,
			heightMapScale
			);
	}
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::addTriangleFace(const Face &face)
{
	MESSENGER_INDENT;
	MESSENGER_REJECT(!m_lastShaderData, ("null m_lastShaderData, unexpected\n"));

	m_lastShaderData->faces.push_back(new Face(face));

#if VERBOSE_GEOMETRY_LOGGING
	REPORT_LOG_PRINT(true, ("Face:\n"));
	REPORT_LOG_PRINT(true, (" P0: %+1.7f,%+1.7f,%+1.7f\n", double(face.positionArray[0].x), double(face.positionArray[0].y), double(face.positionArray[0].z)));
	REPORT_LOG_PRINT(true, (" P1: %+1.7f,%+1.7f,%+1.7f\n", double(face.positionArray[1].x), double(face.positionArray[1].y), double(face.positionArray[1].z)));
	REPORT_LOG_PRINT(true, (" P2: %+1.7f,%+1.7f,%+1.7f\n", double(face.positionArray[2].x), double(face.positionArray[2].y), double(face.positionArray[2].z)));
	REPORT_LOG_PRINT(true, ("UV0: %+1.7f,%+1.7f\n", double(face.uArray[0][0]), double(face.vArray[0][0])));
	REPORT_LOG_PRINT(true, ("UV1: %+1.7f,%+1.7f\n", double(face.uArray[0][1]), double(face.vArray[0][1])));
	REPORT_LOG_PRINT(true, ("UV2: %+1.7f,%+1.7f\n", double(face.uArray[0][2]), double(face.vArray[0][2])));
	REPORT_LOG_PRINT(true, ("DU:%+1.6f,%+1.6f,%+1.6f DV:%+1.6f,%+1.6f,%+1.6f\n", double(face.du.x), double(face.du.y), double(face.du.z), double(face.dv.x), double(face.dv.y), double(face.dv.z)));
#endif

	return true;
}

// ----------------------------------------------------------------------
/**
 * -TRF- this is Maya-specific code.  I did this for speed of implementation,
 * but the MeshBuilder interface should really expose an interface for adding
 * hardpoints and specifying their position and orientation.  The MeshBuilder
 * class is not supposed to have Maya-specific code in it (that way this class could
 * be used in conjunction with, say, a 3DS MAX MeshReader without change).
 */

// ----------------------------------------------------------------------

void StaticMeshBuilder::setHasAlpha( bool bHasAlpha )
{
	m_hasAlpha = bHasAlpha;
	m_setWriter->setHasAlpha(bHasAlpha);
}

//====================================================================================

static void _makeDot3Vectors(
	Vector o_dot3Matrix[3],
	const TriangleRasterizer::Vertex &sample, 
	const float flip
)
{
	//------------------------------------------
	// construct dot3 matrix at this texel.
	o_dot3Matrix[0] = sample.dot3;
	o_dot3Matrix[0].normalize();

	o_dot3Matrix[2] = sample.normal;
	o_dot3Matrix[2].normalize();

	o_dot3Matrix[1] = o_dot3Matrix[2].cross(o_dot3Matrix[0]) * flip;
	o_dot3Matrix[1].normalize();
	//------------------------------------------
}

//------------------------------------------------------------------------------------

static void _applyScratchMapNormal(Vector &o_newTangentNormal, const Vector &i_scratchNormal, const Vector &i_tangentNormal)
{
	Vector newNormal;
	const float magSqr = i_scratchNormal.y*i_scratchNormal.y + i_scratchNormal.x*i_scratchNormal.x;
	if (magSqr)
	{
		const float mag        = sqrt(magSqr);
		const float cosAngle   = sqrt(1.0f - magSqr);
		const float omCosAngle = 1.0f - cosAngle;

		const float rotX =  i_scratchNormal.y / mag;
		const float rotY = -i_scratchNormal.x / mag;

		const Vector axisRotX(
			  rotX*rotX*omCosAngle + cosAngle
			, rotX*rotY*omCosAngle
			, i_scratchNormal.x
		);

		const Vector axisRotY(
			  rotY*rotX*omCosAngle
			, rotY*rotY*omCosAngle + cosAngle
			, i_scratchNormal.y
		);

		const Vector axisRotZ(-i_scratchNormal.x, -i_scratchNormal.y, cosAngle);

		Vector newNormal(axisRotX.dot(i_tangentNormal), axisRotY.dot(i_tangentNormal), axisRotZ.dot(i_tangentNormal));
		newNormal.normalize();

		o_newTangentNormal = newNormal;
	}
	else
	{
		o_newTangentNormal = i_tangentNormal;
	}
}

//------------------------------------------------------------------------------------

bool StaticMeshBuilder::_transformWorldSpaceNormals(
	NormalTexelMixer *mixBuffer, 
	const Image *worldNormalsImage, 
	const Image *tangentNormalsImage, 
	const Image *scratchNormalsImage, 
	const ShaderDataList &shaders
	)
{
	//-----------------------------------------------------
	if (!worldNormalsImage && !scratchNormalsImage)
	{
		DEBUG_WARNING(true, ("No source images supplied!\n"));
		return false;
	}
	//-----------------------------------------------------

	//-----------------------------------------------------
	const uint8 *worldNormalsPixels=0;
	if (worldNormalsImage)
	{
		worldNormalsPixels = worldNormalsImage->lockReadOnly();
		if (!worldNormalsPixels)
		{
			DEBUG_FATAL(true, ("Failed to lock source image pixels.\n"));
			return false;
		}
	}
	Image::UnlockGuard unlockWorldImage(worldNormalsImage);
	//-----------------------------------------------------

	//-----------------------------------------------------
	const uint8 *tangentNormalsPixels=0;
	if (tangentNormalsImage)
	{
		tangentNormalsPixels = tangentNormalsImage->lockReadOnly();
		if (!tangentNormalsPixels)
		{
			DEBUG_FATAL(true, ("Failed to lock source image pixels.\n"));
			return false;
		}
	}
	Image::UnlockGuard unlockTangentImage(tangentNormalsImage);
	//-----------------------------------------------------

	//-----------------------------------------------------
	const uint8 *scratchNormalsPixels = 0;
	if (scratchNormalsImage)
	{
		scratchNormalsPixels = scratchNormalsImage->lockReadOnly();
		if (!scratchNormalsPixels)
		{
			DEBUG_FATAL(true, ("Failed to lock source image pixels.\n"));
			return false;
		}
	}
	Image::UnlockGuard unlockScratchImage(scratchNormalsImage);
	//-----------------------------------------------------

	//-----------------------------------------------------
	const Image *mainImage=0;
	if (worldNormalsImage)
	{
		mainImage=worldNormalsImage;
	}
	else if (tangentNormalsImage)
	{
		mainImage=tangentNormalsImage;
	}
	else if (scratchNormalsImage)
	{
		mainImage=scratchNormalsImage;
	}
	//-----------------------------------------------------

	const ShaderPrimitiveSetWriter &psw = getShaderPrimitiveSetWriter();

	//-----------------------------------------------------
	Shader2Primitive primitives;
	int count;
	count=psw.getNumberOfPrimitives();
	for (int i=0;i<count;i++)
	{
		primitives[std::string(psw.getShaderTemplateName(i))].push_back(i);
	}
	//-----------------------------------------------------

	const int imageBytesPerPixel = 
		  (worldNormalsImage)   ? worldNormalsImage->getBytesPerPixel() 
		: (tangentNormalsImage) ? tangentNormalsImage->getBytesPerPixel()
		: 0
	;
	const int imageWidth         = mainImage->getWidth();
	const int imageHeight        = mainImage->getHeight();

	TriangleRasterizer triRaster (imageWidth, imageHeight);

	//-----------------------------------------------------
	for (int fillMode=0;fillMode<2;fillMode++)
	{
		switch (fillMode)
		{
		case 0: triRaster.setFillMode(TriangleRasterizer::MODE_POLY); break;
		case 1: triRaster.setFillMode(TriangleRasterizer::MODE_EDGE); break;
		}

		ShaderDataList::const_iterator si;
		for (si=shaders.begin();si!=shaders.end();++si)
		{
			const ShaderData &sd = *(*si);

			int normalMapTextureCoordinateIndex = sd.normalMapTextureCoordinateIndex;
			int dot3TextureCoordinateIndex = sd.dot3TextureCoordinateIndex;

			const IntList &shaderPrimitives=primitives[sd.shaderTemplateName];
			IntList::const_iterator ii;
			for (ii=shaderPrimitives.begin();ii!=shaderPrimitives.end();++ii)
			{
				int primitiveIndex = *ii;
				ShaderPrimitiveSetPrimitiveType  primitiveType = psw.getPrimitiveType(primitiveIndex);
				const SystemVertexBuffer        *vertexBuffer  = psw.getVertexBuffer(primitiveIndex);
				int                              indexCount    = psw.getIndexCount(primitiveIndex);
				const Index                     *indexArray    = psw.getIndexArray(primitiveIndex);

				const Index *indexStop = indexArray + indexCount;
				VertexBufferReadIterator vertexZero = vertexBuffer->beginReadOnly();

				if (primitiveType==SPSPT_indexedTriangleList)
				{
					float flip=1;
					for (;indexArray!=indexStop;indexArray+=3)
					{
						float tc0, tc1;
						TriangleRasterizer::Vertex triangle[3];

						//-----------------------------------------
						for (int i=0;i<3;i++)
						{
							VertexBufferReadIterator v = vertexZero + static_cast<int>(indexArray[i]);
							v.getTextureCoordinates(normalMapTextureCoordinateIndex, tc0, tc1);
							triangle[i].coords.x = tc0 * float(imageWidth);
							triangle[i].coords.y = tc1 * float(imageHeight);
							triangle[i].normal=v.getNormal();
							v.getTextureCoordinates(dot3TextureCoordinateIndex, triangle[i].dot3.x, triangle[i].dot3.y, triangle[i].dot3.z, flip);
						}
						//-----------------------------------------

						if (flip<0)
						{
							continue;
						}

						//-----------------------------------------
						triRaster.begin(triangle);
						while (triRaster.advance())
						{
							int pixelX, pixelY;
							const TriangleRasterizer::Vertex &sample = triRaster.sample(pixelX, pixelY);

							bool processPixel=false;
							int pixelOffset = pixelY*imageWidth + pixelX;
							NormalTexelMixer *destPixel = mixBuffer + pixelOffset;

							switch (triRaster.getFillMode())
							{
							case TriangleRasterizer::MODE_POLY:
								processPixel=true;
								break;

							case TriangleRasterizer::MODE_EDGE:
								if (destPixel->wasSet)
								{
									if (triRaster.isSampleLeft())
									{
										if (pixelX>0 && !destPixel[-1].wasSet)
										{
											destPixel--;
											pixelX--;
											pixelOffset--;
										}
										else if (pixelY>0 && !destPixel[-imageWidth].wasSet)
										{
											destPixel  -=imageWidth;
											pixelX     -=imageWidth;
											pixelOffset-=imageWidth;
										}
									}
									else
									{
										if (pixelX+1<imageWidth && !destPixel[1].wasSet)
										{
											destPixel++;
											pixelX++;
											pixelOffset++;
										}
										else if (pixelY+1<imageHeight && !destPixel[imageWidth].wasSet)
										{
											destPixel  +=imageWidth;
											pixelX     +=imageWidth;
											pixelOffset+=imageWidth;
										}
									}
								}
								processPixel=!destPixel->wasSet;
								break;
							}

							if (processPixel)
							{
							#if !IGNORE_WORLD_NORMALS
								const uint8 *const worldPixel = (worldNormalsPixels) ? worldNormalsPixels + pixelOffset*imageBytesPerPixel : (const uint8 *)0;
							#else
								const uint8 *const worldPixel = 0;
							#endif

								const uint8 *const tangentPixel = (tangentNormalsPixels) ? tangentNormalsPixels + pixelOffset*imageBytesPerPixel : (const uint8 *)0;
								const uint8 *const scratchPixel = (scratchNormalsPixels) ? scratchNormalsPixels + pixelOffset*3                  : (const uint8 *)0;

								//------------------------------------------
								// construct dot3 matrix at this texel.
								Vector dot3Matrix[3];

								_makeDot3Vectors(dot3Matrix, sample, flip);
								//------------------------------------------

								//-------------------------------------------------
								// Get our source world-normal.
								Vector worldNormal;

								if (worldPixel)
								{
									Vector tempNormal;
									NormalMapUtility::unpackBgrNormal(tempNormal, worldPixel);

									worldNormal.x=-tempNormal.x;
									worldNormal.y= tempNormal.z;
									worldNormal.z=-tempNormal.y;
								}
								else
								{
									worldNormal=dot3Matrix[2];
								}
								//-------------------------------------------------

								//-------------------------------------------------
								// Get our tangent-space normal
								Vector tangentSpaceNormal;
								if (tangentPixel)
								{
									NormalMapUtility::unpackBgrNormal(tangentSpaceNormal, tangentPixel);
								}
								else
								{
								#if SKIP_TANGENT_SPACE_XFORM
									tangentSpaceNormal=worldNormal;
								#else
									tangentSpaceNormal.x = worldNormal.dot(dot3Matrix[0]);
									tangentSpaceNormal.y = worldNormal.dot(dot3Matrix[1]);
									tangentSpaceNormal.z = worldNormal.dot(dot3Matrix[2]);
									tangentSpaceNormal.normalize();
								#endif
								}
								//-------------------------------------------------

								//-------------------------------------------------
								// if there is a scratch-map, apply that rotation 
								// to the current tangent-space normal.
							#if !IGNORE_HEIGHT_MAP && !SKIP_TANGENT_SPACE_XFORM
								if (scratchPixel)
								{
									Vector scratchNormal;

									NormalMapUtility::unpackBgrNormal(scratchNormal, scratchPixel);
									_applyScratchMapNormal(tangentSpaceNormal, scratchNormal, tangentSpaceNormal);
								}
							#endif
								//------------------------------------------

							#if MIX_FLOAT_NORMALS
								destPixel->x = tangentSpaceNormal.x;
								destPixel->y = tangentSpaceNormal.y;
								destPixel->z = tangentSpaceNormal.z;
							#else
								destPixel->x = static_cast<short>(tangentSpaceNormal.x*256.0f);
								destPixel->y = static_cast<short>(tangentSpaceNormal.y*256.0f);
								destPixel->z = static_cast<short>(tangentSpaceNormal.z*256.0f);
							#endif

								if (imageBytesPerPixel==4)
								{
									if (worldPixel)
									{
										destPixel->alpha = worldPixel[3];
									}
									else if (tangentPixel)
									{
										destPixel->alpha = tangentPixel[3];
									}
								}

								destPixel->wasSet=true;
							}
						}
						//-----------------------------------------
					}
				}
			}
		}
	}
	//-----------------------------------------------------

	return true;
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::_transformWorldSpaceNormals(const ShaderDataList &i_shaders)
{
	ShaderDataList::const_iterator si;

	int mixWidth=-1, mixHeight=-1;
	NormalTexelMixer *mixBuffer=0;
	bool mixAlpha=false;

	//--------------------------------------------------
	//-- Prepare destination file.
	si = i_shaders.begin();
	const ShaderData &s = *(*si);

	std::string worldNormalsFileName;
	std::string tangentNormalsFileName;
	std::string scratchMapFileName;

	s.getNormalsFileNames(
		&tangentNormalsFileName,
		&worldNormalsFileName,
		&scratchMapFileName
	);

	const float heightMapScale = s.m_heightMapScale;

	const bool wrapU = s.m_wrapU;
	const bool wrapV = s.m_wrapV;

	//-------------------------------------------
	Image *worldNormalsImage;
	if (!TargaFormat().loadImage(worldNormalsFileName.c_str(), &worldNormalsImage))
	{
		worldNormalsImage=0;
	}
	//-------------------------------------------

	//-------------------------------------------
	Image *tangentNormalsImage=0;
	/*
	if (!worldNormalsImage)
	{
		if (!TargaFormat().loadImage(tangentNormalsFileName.c_str(), &tangentNormalsImage))
		{
			tangentNormalsImage=0;
		}
	}
	*/
	//-------------------------------------------

	//-------------------------------------------
	// see if we can find a scratch map.
	Image *scratchNormalsImage=0;
	{
		Image *scratchMapImage=0;
		if (TargaFormat().loadImageReformat(scratchMapFileName.c_str(), &scratchMapImage, Image::PF_w_8))
		{
			scratchNormalsImage=NormalMapUtility::createScratchNormalMap(*scratchMapImage, heightMapScale, wrapU, wrapV);
			delete scratchMapImage;
		}
	}
	//-------------------------------------------

	//-------------------------------------------
	const Image *mainImage=0;
	if (worldNormalsImage)
	{
		mainImage=worldNormalsImage;
	}
	else if (tangentNormalsImage)
	{
		mainImage=tangentNormalsImage;
	}
	else if (scratchNormalsImage)
	{
		mainImage=scratchNormalsImage;
	}
	//-------------------------------------------

	//-------------------------------------------
	if (mainImage)
	{
		Image::PixelFormat pixelFormat  = mainImage->getPixelFormat();
		const int          imageWidth   = mainImage->getWidth();
		const int          imageHeight  = mainImage->getHeight();
		const int          numPixels    = imageWidth*imageHeight;

		if (
				pixelFormat==Image::PF_bgra_8888 
			|| pixelFormat==Image::PF_abgr_8888 
			|| pixelFormat==Image::PF_rgba_8888 
			|| pixelFormat==Image::PF_argb_8888
		)
		{
			mixAlpha=true;
		}

		//-------------------------------------------
		if (worldNormalsImage)
		{
			uint8 *worldNormalsBuffer = worldNormalsImage->lock();
			NormalMapUtility::skirtNormalMap(worldNormalsBuffer, imageWidth, imageHeight, worldNormalsImage->getBytesPerPixel());
			worldNormalsImage->unlock();
		}
		//-------------------------------------------

		//-------------------------------------------
		if (!mixBuffer)
		{
			mixWidth=imageWidth;
			mixHeight=imageHeight;
			mixBuffer = new NormalTexelMixer[numPixels];
			NormalTexelMixer *niter=mixBuffer;
			NormalTexelMixer *nstop=mixBuffer + numPixels;
			while (niter!=nstop)
			{
				niter->x=0;
				niter->y=0;
				niter->z=0;
				niter->alpha=255;
				niter->wasSet=false;
				niter++;
			}
		}
		//-------------------------------------------

		_transformWorldSpaceNormals(
			mixBuffer, 
			worldNormalsImage,
			tangentNormalsImage,
			scratchNormalsImage, 
			i_shaders
			);

		//-------------------------------------------
		delete worldNormalsImage;
		worldNormalsImage=0;

		delete tangentNormalsImage;
		tangentNormalsImage=0;

		delete scratchNormalsImage;
		scratchNormalsImage=0;
		//-------------------------------------------
	}
	//--------------------------------------------------

	//--------------------------------------------------
	if (mixBuffer)
	{
		Image::PixelFormat destFormat;
		int destBytesPerPixel;

		mixAlpha=true;
		if (mixAlpha)
		{
			destFormat=Image::PF_bgra_8888;
			destBytesPerPixel=4;
		}
		else
		{
			destFormat=Image::PF_bgr_888;
			destBytesPerPixel=3;
		}

		const int destBufferSize = mixWidth*mixHeight*destBytesPerPixel;
		uint8 *destImageBuffer = new uint8[destBufferSize];
		memset(destImageBuffer, 0, destBufferSize);

		const NormalTexelMixer *niter=mixBuffer;
		const NormalTexelMixer *const nstop = mixBuffer + mixWidth * mixHeight;
		uint8 *diter = destImageBuffer;
		while (niter!=nstop)
		{
			if (niter->wasSet)
			{
				Vector normal;

				normal.x = float(niter->x);
				normal.y = float(niter->y);
				normal.z = float(niter->z);
				normal.normalize();

				NormalMapUtility::packBgrNormal(diter, normal);
			}
			if (destBytesPerPixel==4)
			{
				diter[3]=niter->alpha;
			}
			diter+=destBytesPerPixel;
			niter++;
		}

		delete [] mixBuffer;
		mixBuffer=0;

	#if !SKIP_SKIRT_AND_FILL
		NormalMapUtility::skirtNormalMap(destImageBuffer, mixWidth, mixHeight, destBytesPerPixel);
		NormalMapUtility::skirtNormalMap(destImageBuffer, mixWidth, mixHeight, destBytesPerPixel);
		NormalMapUtility::skirtNormalMap(destImageBuffer, mixWidth, mixHeight, destBytesPerPixel);
		NormalMapUtility::skirtNormalMap(destImageBuffer, mixWidth, mixHeight, destBytesPerPixel);
		NormalMapUtility::fillNormalMapEmptySpace(destImageBuffer, mixWidth, mixHeight, destBytesPerPixel);
	#endif

		const uint alphaMask = (mixAlpha) ? 0xff000000 : 0;
		const uint redMask   = 0x00ff0000;
		const uint greenMask = 0x0000ff00;
		const uint blueMask  = 0x000000ff;

		{
			Image destImage(
				destImageBuffer, 
				destBufferSize,
				mixWidth,
				mixHeight,
				destBytesPerPixel*8,
				destBytesPerPixel,
				mixWidth * destBytesPerPixel,
				redMask,
				greenMask,
				blueMask,
				alphaMask
			);

			DEBUG_FATAL(destImage.getPixelFormat()!=destFormat, ("Destination image pixel format is incorrect.\n"));

			TargaFormat().saveImage(destImage, tangentNormalsFileName.c_str());
		}

		delete [] destImageBuffer;
	}
	//--------------------------------------------------

	delete [] mixBuffer;
	mixBuffer=0;

	return true;
}

// ----------------------------------------------------------------------

void StaticMeshBuilder::_transformWorldSpaceNormals()
{
	File2Shaders normalMaps;

	const ShaderData *shaderData = m_firstShaderData;
	for (; shaderData != NULL; shaderData = shaderData->next)
	{
		if (!shaderData->normalsBaseFileName.empty())
		{
			normalMaps[std::string(shaderData->normalsBaseFileName)].push_back(shaderData);
		}
	}

	File2Shaders::iterator fi;
	for (fi=normalMaps.begin();fi!=normalMaps.end();++fi)
	{
		_transformWorldSpaceNormals(fi->second);
	}
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::specifyNoMoreData(void)
{
	MESSENGER_INDENT;

	// load up the writer with its data
	ShaderData *sd;
	int         sdIndex;
	bool        result;

	for (sd = m_firstShaderData, sdIndex = 0; sd; sd = sd->next, ++sdIndex)
	{
		result = sd->build(m_setWriter, m_shaderTemplateCount - sdIndex - 1);
		MESSENGER_REJECT(!result, ("failed to write shader %d's face data (\"%s\")\n", sdIndex, sd->shaderTemplateName));
	}

	// If we have no extent for the mesh, create one.
	if (getExtent()==NULL)
	{
		Vector min;
		Vector max;

		m_setWriter->getBoundingBox(&min,&max);
		
		Sphere sphere = m_setWriter->getBoundingSphere();

		BoxExtent * B = new BoxExtent(min,max,sphere);

		attachExtent(B);
	}

	_transformWorldSpaceNormals();

	return true;
}

// ----------------------------------------------------------------------

void StaticMeshBuilder::getDot3TextureCoordinateInfo(const char *shaderTemplateName, bool &hasDot3Coordinate, int &textureCoordinateIndex) const
{
	if (!shaderTemplateName || !*shaderTemplateName)
	{
		MESSENGER_LOG_WARNING(("getDot3TextureCoordinateInfo(): passed a NULL or zero-length shaderTemplateName.\n"));
		hasDot3Coordinate = false;

		return;
	}

	//-- Find the shader data with the given shaderTemplateName.
	const ShaderData *shaderData = m_firstShaderData;
	for (; shaderData != NULL; shaderData = shaderData->next)
	{
		const bool foundIt = (!shaderData->shaderTemplateName.empty() && (strcmp(shaderTemplateName, shaderData->shaderTemplateName.c_str()) == 0));
		if (foundIt)
		{
			hasDot3Coordinate      = shaderData->hasDot3TextureCoordinate;
			textureCoordinateIndex = shaderData->dot3TextureCoordinateIndex;

			return;
		}
	}

	//-- Didn't find a matching shader.
	MESSENGER_LOG_WARNING(("getDot3TextureCoordinateInfo(): specified shaderTemplateName [%s] is unknown to StaticMeshBuilder.\n", shaderTemplateName));
	hasDot3Coordinate = false;
}

// ----------------------------------------------------------------------

Sphere StaticMeshBuilder::getSphere (void) const
{
	return m_setWriter->getBoundingSphere();
}

// ----------------------------------------------------------------------

bool StaticMeshBuilder::write(Iff & iff, Vector &minVector, Vector &maxVector) const
{
	MESSENGER_INDENT;

	// write the data
	iff.insertForm(TAG_MESH);

		iff.insertForm(TAG_0005);

			MeshBuilder::write(iff);

			bool writeResult = m_setWriter->write(&iff);
			if(!writeResult)
			{
				MESSENGER_LOG_ERROR(("No Shader defined\n"));
				return false;
			}

			{
				m_setWriter->getBoundingBox(&minVector, &maxVector);
				
				Sphere sphere = getSphere();

				ExporterLog::setExtentForCurrentMesh(sphere.getCenter(), sphere.getRadius(), minVector, maxVector);
			}

		iff.exitForm(TAG_0005);

	iff.exitForm(TAG_MESH);

	return true;
}

// ======================================================================
