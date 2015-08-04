// ======================================================================
//
// MayaMeshReader.cpp
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

//precompiled header includes
#include "FirstMayaExporter.h"

//module include
#include "MayaMeshReader.h"

//engine shared includes
#include "sharedMath/PositionVertexIndexer.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

//local mayaExporter includes
#include "ExporterLog.h"
#include "MayaShaderTemplateBuilder.h"
#include "MayaShaderReader.h"
#include "MayaUtility.h"
#include "MeshBuilder.h"
#include "Messenger.h"
//#include "PluginMain.h"
#include "SetDirectoryCommand.h"

//maya SDK includes
#include "maya/MAnimControl.h"
#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnMesh.h"
#include "maya/MFnPhongShader.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MObject.h"
#include "maya/MPlug.h"
#include "maya/MSelectionList.h"
#include "maya/MTime.h"


//system / STL includes
#include <map>
#include <set>
#include <io.h>

// ======================================================================

namespace MayaMeshReaderNamespace
{
	Messenger *messenger;

	PositionVertexIndexer * ms_globalVertexIndexer;
}

using namespace MayaMeshReaderNamespace;

// ======================================================================

const Vector MayaMeshReader::globallyMergeVertexPosition(const Vector& position)
{
	NOT_NULL(ms_globalVertexIndexer);
	int const index = ms_globalVertexIndexer->addVertex(position);

	return ms_globalVertexIndexer->getVertex(index);
}

// ----------------------------------------------------------------------

void MayaMeshReader::clearGlobalVertexIndexer()
{
	NOT_NULL(ms_globalVertexIndexer);
	ms_globalVertexIndexer->clear();
}

// ======================================================================

MayaMeshReader::MayaMeshData::MayaMeshData(const MDagPath &meshDagPath)
:	hasVertexAlphaArray(0),
	isValid(0),
	dagPath(meshDagPath),
	pointArray(),
	normalArray(),
	shaderGroupArray(),
	polyShaderGroupMap(),
	uvSetNameArray(),
	shaderGroupPolyCount()
{
	MESSENGER_INDENT;

	MStatus  status;

	MFnMesh fnMesh(meshDagPath, &status);
	if (!status)
		return;

	// get the position array in world space (global object space), fix it up
	status = fnMesh.getPoints(pointArray, MSpace::kObject);
	if (!status)
		return;
	const unsigned int pointCount = pointArray.length();
	for (unsigned int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
	{
		MFloatPoint &point = pointArray[pointIndex];
		point.x = -point.x;
	}

	// get the normal array in world space, fix it up
	status = fnMesh.getNormals(normalArray, MSpace::kObject);
	if (!status)
		return;
	const unsigned int normalCount = normalArray.length();
	for (unsigned int normalIndex = 0; normalIndex < normalCount; ++normalIndex)
	{
		MFloatVector &normal = normalArray[normalIndex];
		normal.x = -normal.x;
	}

	//
	// get the uvs for this mesh, fix up
	//

	//-- get the uv set names
	status = fnMesh.getUVSetNames (uvSetNameArray);
	MESSENGER_REJECT_VOID(!status, ("fnMesh.getUVSetNames () failed, \"%s\"\n", status.errorString().asChar()));

	unsigned int i;
	for (i = 0; i < uvSetNameArray.length (); i++)
		MESSENGER_LOG(("uvSetNameArray [%u] = %s\n", i, uvSetNameArray [i].asChar ()));

	//-- exporter log
	ExporterLog::setSourceNumberOfVertices (fnMesh.numVertices ());
	ExporterLog::setSourceNumberOfPolygons (fnMesh.numPolygons ());
	ExporterLog::setSourceNumberOfUVSets (fnMesh.numUVSets ());
	ExporterLog::setUVSetsForCurrentMesh(fnMesh.numUVSets());

	// get the shader group objects for shader groups used by the mapping between polygons and shader groups
	status = fnMesh.getConnectedShaders(0, shaderGroupArray, polyShaderGroupMap);
	if (!status)
		return;

	unsigned int l = shaderGroupArray.length();
	unsigned int x;
	for (x=0; x<l; ++x)
	{
		shaderGroupPolyCount.add(0);
	}
	for (x=0; x<polyShaderGroupMap.length(); ++x)
	{
		int index = polyShaderGroupMap[x];
		if(index >= 0)
			shaderGroupPolyCount[index]++;
	}

	// create the hasVertexAlphaArray, one entry per shader group referenced by the maya mesh
	hasVertexAlphaArray = new bool[shaderGroupArray.length()];
	memset(hasVertexAlphaArray, 0, sizeof(bool) * shaderGroupArray.length());

	if (!_buildHasVertexAlphaArray())
		return;

	// we're valid if we've made it this far
	isValid = true;
}

// ----------------------------------------------------------------------

MayaMeshReader::MayaMeshData::~MayaMeshData(void)
{
	delete [] hasVertexAlphaArray;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::MayaMeshData::_buildHasVertexAlphaArray(void)
{
	// -TRF- I hate to do this here, but I need to know if the maya mesh has any non-unity alpha
	MStatus status;
	MItMeshPolygon  iter(dagPath, MObject::kNullObj, &status);
	if (!status)
		return false;

	// loop over all polys, all vertices to check if they've specified any non-unity alpha
	bool    isDoneFlag;
	MColor  color;

	do
	{
		// check if any of the poly's vertices have alpha set
		const bool hasAnyColor = iter.hasColor(&status);
		if (!status)
			return false;

		if (hasAnyColor)
		{
			// at least one of the polygon's vertices has color
			const long vertexCount = iter.polygonVertexCount(&status);
			for (long vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				const bool hasVertexColor = iter.hasColor(static_cast<int>(vertexIndex), &status);
				if (!status)
					return false;

				if (hasVertexColor)
				{
					// the vertex has color, check if it's alpha is not full opacity
					status = iter.getColor(color, static_cast<int>(vertexIndex));
					if (!status)
						return false;
					if (color.a != 1.0f)
					{
						// we've got a non-unity alpha that was definitely specified by the user.
						// remember that this shader has vertex alpha
						const int polyIndex = iter.index(&status);
						if (!status)
							return false;
						const int shaderTemplateIndex = polyShaderGroupMap[static_cast<unsigned int>(polyIndex)];
						hasVertexAlphaArray[shaderTemplateIndex] = true;

						// we can bail out now that we've found one
						return true;
					}
				}
			}
		}

		// move forward
		status = iter.next();
		if (!status)
			return false;

		// check if we're done
		isDoneFlag = iter.isDone(&status);
		if (!status)
			return false;
	} while (!isDoneFlag);

	return true;
}

// ----------------------------------------------------------------------
/**
 * -TRF- this code assumes that, if UVs are present on a polygon, they
 * should be recorded and the associated ShaderTemplate requires one UV.
 * this should probably change to something that validates how many uvs are
 * required for a given ShaderTemplate.
 */

bool MayaMeshReader::MayaMeshData::addFaces(int shaderGroupIndex, MeshBuilder *builder) const
{
	if (!builder)
		return false;

	messenger->logMessage("Adding faces for group %d: ", shaderGroupIndex);

	MStatus status;
	MItMeshPolygon  iter(dagPath, MObject::kNullObj, &status);
	if (!status)
		return false;

	// loop over all polys, all vertices to check if they've specified any non-unity alpha
	bool               isDoneFlag;
	MColorArray        colorArray;
	MVector            mayaFaceNormal;
	Vector             faceNormal;
	MeshBuilder::Face  face;
	int faces_added=0;

	do
	{
		//ensure that we have an empty array for the following functions (esp. iter.hasColor, which seems to assume either 1: emtpy array, or 2: an array large enough)
		IGNORE_RETURN(colorArray.clear());

		// check if poly uses specified shader group
		const int polyIndex = iter.index(&status);
		MESSENGER_REJECT(!status, ("failed to get polygon index [%s].\n", status.errorString().asChar()));

		MESSENGER_REJECT(polyIndex >= static_cast<int>(polyShaderGroupMap.length()), ("poly index %d exceeds polyShaderGroupMap size %d.\n", polyIndex, static_cast<int>(polyShaderGroupMap.length())));
		if (shaderGroupIndex == polyShaderGroupMap[static_cast<unsigned int>(polyIndex)])
		{
			const long  vertexCount = iter.polygonVertexCount(&status);
			MESSENGER_REJECT(!status, ("failed to get polygon vertex count [%s].\n", status.errorString().asChar()));

			// handle the colors for this polygon
			const bool polygonHasColor = iter.hasColor (&status);
			MESSENGER_REJECT(!status, ("failed to test for polygon color [%s].\n", status.errorString().asChar()));

			if (polygonHasColor)
			{
				status = iter.getColors(colorArray);
				MESSENGER_REJECT(!status, ("failed to get polygon colors [%s].\n", status.errorString().asChar()));

				for (long colorIndex = 0; colorIndex < vertexCount; ++colorIndex)
				{
					const bool vertexHasColor = iter.hasColor(static_cast<int>(colorIndex), &status);
					MESSENGER_REJECT(!status, ("checking for vertex color component failed [%s].\n", status.errorString().asChar()));

					if (!vertexHasColor)
					{
						// default unset colors to pure white, completely opaque
						IGNORE_RETURN(colorArray.set (static_cast<unsigned int>(colorIndex), 1.0f, 1.0f, 1.0f, 1.0f));
					}
				}
			}

			// get the face normal
			status = iter.getNormal(mayaFaceNormal, MSpace::kObject);
			if (!status)
				return false;
			faceNormal.x = CONST_REAL(-mayaFaceNormal.x);
			faceNormal.y = CONST_REAL(mayaFaceNormal.y);
			faceNormal.z = CONST_REAL(mayaFaceNormal.z);

			// triangulate (flip vertex order for culling)
			const int v0 = 0;
			int v1;
			int v2;

			// get v0 data (static per this polygon)
			const unsigned int meshV0 = static_cast<unsigned int>(iter.vertexIndex(v0, &status));
			if (!status)
				return false;
			const MFloatPoint  &v0Position    = pointArray[meshV0];
			const int           v0NormalIndex = iter.normalIndex(v0, &status);
			MESSENGER_REJECT(!status, ("iter.normalIndex() failed,\"%s\"\n", status.errorString().asChar()));

			const MFloatVector &v0Normal      = normalArray[static_cast<unsigned int>(v0NormalIndex)]; 
			const MColor       &v0Color       = colorArray[static_cast<unsigned int>(v0)];

			// setup static face data
			face.faceNormal           = faceNormal;
			face.positionArray[0]     = MayaMeshReader::globallyMergeVertexPosition(Vector(v0Position.x, v0Position.y, v0Position.z));
			face.vertexNormalArray[0] = Vector(v0Normal.x, v0Normal.y, v0Normal.z);
			face.diffuseColorsValid   = false;

			if (polygonHasColor)
			{
				face.diffuseColorsValid = true;
				face.vertexDiffuseColorArray[0]  = VectorArgb(v0Color.a, v0Color.r, v0Color.g, v0Color.b);
			}

			//-- get uv sets
			{
				int uvSetsAssigned = 0;

				//-- go through all of the uvsets and see if this polygon uses it
				uint i;
				for (i = 0; i < uvSetNameArray.length (); i++)
				{
					bool hasUVSet = iter.hasUVs (uvSetNameArray [i], &status);
					MESSENGER_REJECT(!status, ("iter.hasUvs () failed looking for %s, \"%s\"\n", uvSetNameArray [i].asChar (), status.errorString().asChar()));

					if (hasUVSet)
					{
						MString name = uvSetNameArray [i];
						float uv [2];

						status = iter.getUV (v0, uv, &name);
						MESSENGER_REJECT(!status, ("iter.getUV() failed, \"%s\"\n", status.errorString().asChar()));

						face.uArray[uvSetsAssigned][0] = uv [0];
						face.vArray[uvSetsAssigned][0] = CONST_REAL (1) - uv [1];

						uvSetsAssigned++;
					}
				}

				face.uvCount = uvSetsAssigned;
				MESSENGER_REJECT (uvSetsAssigned > MAX_UV_COUNT, ("uvSetsAssigned (%u) > (%u)\n", uvSetsAssigned, MAX_UV_COUNT));
			}

			for (v1 = vertexCount-1, v2 = v1-1; v2 > 0; --v1, --v2)
			{
				// get v1 data
				const unsigned int meshV1 = static_cast<unsigned int>(iter.vertexIndex(v1, &status));
				if (!status)
					return false;
				const MFloatPoint  &v1Position    = pointArray[meshV1];
				const int           v1NormalIndex = iter.normalIndex(v1, &status);
				MESSENGER_REJECT(!status, ("iter.normalIndex() failed,\"%s\"\n", status.errorString().asChar()));

				const MFloatVector &v1Normal      = normalArray[static_cast<unsigned int>(v1NormalIndex)]; 
				const MColor       &v1Color       = colorArray[static_cast<unsigned int>(v1)];

				// get v2 data
				const unsigned int meshV2 = static_cast<unsigned int>(iter.vertexIndex(v2, &status));
				if (!status)
					return false;
				const MFloatPoint  &v2Position    = pointArray[meshV2];
				const int           v2NormalIndex = iter.normalIndex(v2, &status);
				MESSENGER_REJECT(!status, ("iter.normalIndex() failed,\"%s\"\n", status.errorString().asChar()));

				const MFloatVector &v2Normal      = normalArray[static_cast<unsigned int>(v2NormalIndex)]; 
				const MColor       &v2Color       = colorArray[static_cast<unsigned int>(v2)];

				// setup per-triangle face data
				face.positionArray[1]     = MayaMeshReader::globallyMergeVertexPosition(Vector(v1Position.x, v1Position.y, v1Position.z));
				face.vertexNormalArray[1] = Vector(v1Normal.x, v1Normal.y, v1Normal.z);

				if (polygonHasColor)
				{
					face.diffuseColorsValid = true;
					face.vertexDiffuseColorArray[1]  = VectorArgb(v1Color.a, v1Color.r, v1Color.g, v1Color.b);
				}
				
				face.positionArray[2]     = MayaMeshReader::globallyMergeVertexPosition(Vector(v2Position.x, v2Position.y, v2Position.z));
				face.vertexNormalArray[2] = Vector(v2Normal.x, v2Normal.y, v2Normal.z);

				if (polygonHasColor)
				{
					face.diffuseColorsValid = true;
					face.vertexDiffuseColorArray[2]  = VectorArgb(v2Color.a, v2Color.r, v2Color.g, v2Color.b);
				}

				// handle uvs
				{
					int uvSetsAssigned = 0;

					//-- go through all of the uvsets and see if this polygon uses it
					uint i;
					for (i = 0; i < uvSetNameArray.length (); i++)
					{
						bool hasUVSet = iter.hasUVs (uvSetNameArray [i], &status);
						MESSENGER_REJECT(!status, ("iter.hasUVs() failed, \"%s\"\n", status.errorString().asChar()));

						if (hasUVSet)
						{
							float uv [2];

							MString name = uvSetNameArray [i];

							status = iter.getUV(v1, uv, &name);
							MESSENGER_REJECT(!status, ("iter.getUV() failed, \"%s\"\n", status.errorString().asChar()));

							face.uArray[uvSetsAssigned][1] = uv [0];
							face.vArray[uvSetsAssigned][1] = CONST_REAL (1) - uv [1];

							status = iter.getUV(v2, uv, &name);
							MESSENGER_REJECT(!status, ("iter.getUVIndex() failed, \"%s\"\n", status.errorString().asChar()));

							face.uArray[uvSetsAssigned][2] = uv [0];
							face.vArray[uvSetsAssigned][2] = CONST_REAL (1) - uv [1];

							uvSetsAssigned++;
						}
					}

					MESSENGER_REJECT (uvSetsAssigned != face.uvCount, ("uvSetsAssigned (%u) != face.uvCount (%u)\n", uvSetsAssigned, face.uvCount));
				}

				// submit the face to the builder
				if (builder->validateFace(face))
				{
					builder->processDot3Transform(face);
					if (!builder->addTriangleFace(face))
						return false;

					++faces_added;
				}
				else
				{
					MIntArray vertList;
					iter.getVertices(vertList);
					MESSENGER_LOG_WARNING(("Additional Zero Sum Triangle Info: Face Index [%u] Vert Indices [%d] [%d] [%d]\n", iter.index(), vertList[0], vertList[1], vertList[2]));
				}
			}
		}

		// move forward
		status = iter.next();
		if (!status)
			return false;

		// check if we're done
		isDoneFlag = iter.isDone(&status);

		if (!status)
			return false;
	} while (!isDoneFlag);
	
	messenger->logMessage("added %d faces\n", faces_added);
	return faces_added > 0;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::MayaMeshData::usesShader(int shaderGroupIndex) const
{
	return shaderGroupPolyCount[shaderGroupIndex]>0; 
}

// ======================================================================

MayaMeshReader::ShaderMeshNode::ShaderMeshNode(const MayaMeshData *newMayaMeshData, int newShaderGroupIndex)
:	mayaMeshData(newMayaMeshData),
	shaderGroupIndex(newShaderGroupIndex)
{
}

// ----------------------------------------------------------------------

MayaMeshReader::ShaderMeshNode::~ShaderMeshNode(void)
{
	mayaMeshData = NULL;
}

// ======================================================================

MayaMeshReader::PerShaderData::PerShaderData(const char *newName, bool newDot3, int newnormalMapTextureCoordinateIndex)
:	name(newName),
	shaderGroupObject(),
	shaderMeshNodes(),
	dot3(newDot3),
	normalMapTextureCoordinateIndex(newnormalMapTextureCoordinateIndex)
{
}

// ----------------------------------------------------------------------

MayaMeshReader::PerShaderData::~PerShaderData(void)
{	
}

// ----------------------------------------------------------------------

bool MayaMeshReader::PerShaderData::hasVertexAlpha(void) const
{
	MESSENGER_INDENT;

	std::vector<ShaderMeshNode>::const_iterator meshNode;
	for (meshNode=shaderMeshNodes.begin(); meshNode!=shaderMeshNodes.end(); ++meshNode)
	{
		const MayaMeshData *meshData = meshNode->mayaMeshData;
		MESSENGER_REJECT(!meshData->hasVertexAlphaArray, ("mayaMeshData has null hasVertexAlphaArray\n"));
		if (meshData->hasVertexAlphaArray[meshNode->shaderGroupIndex])
			return true;
	}

	// none of the maya meshes associated with this ShaderTemplate has vertex alpha
	return false;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::PerShaderData::addMayaMeshData(const MayaMeshData *newMayaMeshData, int shaderGroupIndex)
{
	MESSENGER_INDENT;

	MESSENGER_REJECT(!newMayaMeshData, ("null newMayaMeshData arg\n"));
	MESSENGER_REJECT(static_cast<unsigned int>(shaderGroupIndex) >= newMayaMeshData->shaderGroupArray.length(), ("shaderGroupIndex %d out of valid range [0-%d)\n", shaderGroupIndex, newMayaMeshData->shaderGroupArray.length()));

	if (newMayaMeshData->usesShader(shaderGroupIndex))
	{
		// -TRF- could do a debug check to make sure newMayaMeshData doesn't already exist for this PerShaderData

		if (shaderMeshNodes.empty()) 			// first mesh for ShaderTemplate
		{
			// hang on to the ShaderGroup object
			shaderGroupObject = newMayaMeshData->shaderGroupArray[static_cast<unsigned int>(shaderGroupIndex)];
		}
		ShaderMeshNode temp(newMayaMeshData, shaderGroupIndex);
		shaderMeshNodes.push_back(temp);
	}
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::PerShaderData::construct(MeshBuilder *builder, const char *shaderTemplateReferenceDir, const char *textureReferenceDirectory) const
{
	MESSENGER_INDENT;

	MESSENGER_REJECT(!builder, ("null builder arg\n"));

	// construct complete runtime ShaderTemplate name
	char shaderTemplateReferenceName[MAX_PATH];
	sprintf(shaderTemplateReferenceName, "%s%s.sht", shaderTemplateReferenceDir, name.c_str()); 

	//-----------------------------------------------
	// set the ShaderTemplate
	int  shaderTemplateIndex;
	bool result;

	result = builder->setShaderTemplate(shaderTemplateReferenceName, &shaderTemplateIndex, dot3, normalMapTextureCoordinateIndex);
	MESSENGER_REJECT(!result, ("builder's setShaderTemplate() failed\n"));
	//-----------------------------------------------

	//-----------------------------------------------
	// set normal information for the shader template
	if (shaderGroupObject.apiType() == MFn::kShadingEngine)
	{
		MayaShaderReader sr(shaderGroupObject);
		if (!sr.failed())
		{
			float heightMapScale;
			if (!sr.getFloatAttribute(heightMapScale, "soe_heightMapScale"))
			{
				heightMapScale=1;
			}

			MayaShaderReader::TextureAttributeList textureList;
			if (  sr.getTextureAttributes(textureList, "soe_textureName_NRML", textureReferenceDirectory) 
				|| sr.getTextureAttributes(textureList, "soe_textureName_CNRM", textureReferenceDirectory)
			)
			{
				const MayaShaderReader::TextureAttribute &tex = textureList.front();

				const std::string &tangentNormalsFileName = tex.m_sourceFileName;

				std::string worldNormalsFileName;
				std::string scratchMapFileName;
				std::string::size_type underscore;

				underscore = tangentNormalsFileName.find_last_of('_');
				if (underscore!=std::string::npos)
				{
					worldNormalsFileName.reserve(tangentNormalsFileName.size() + 1);
					worldNormalsFileName.insert(0, tangentNormalsFileName, 0, underscore);
					worldNormalsFileName.append("_wn.tga");
					if (_access(worldNormalsFileName.c_str(), 04)!=0)
					{
						worldNormalsFileName.clear();
					}

					scratchMapFileName.reserve(tangentNormalsFileName.size() + 1);
					scratchMapFileName.insert(0, tangentNormalsFileName, 0, underscore);
					scratchMapFileName.append("_hm.tga");
					if (_access(scratchMapFileName.c_str(), 04)!=0)
					{
						scratchMapFileName.clear();
					}
				}

				if (  !worldNormalsFileName.empty()
					|| !scratchMapFileName.empty()
					)
				{
					std::string baseName;
					baseName.insert(0, tangentNormalsFileName, 0, underscore);

					builder->setShaderNormalsInfo(
						baseName.c_str(), 
						tex.m_compressType,
						tex.m_wrapU,
						tex.m_wrapV,
						heightMapScale
						);
				}
			}
		}
	}
	//-----------------------------------------------

	// tell each mesh to add faces for this ShaderTemplate
	int             meshIndex;

	std::vector<ShaderMeshNode>::const_iterator meshNode;
	for (meshNode = shaderMeshNodes.begin(), meshIndex = 0; meshNode!=shaderMeshNodes.end(); ++meshNode, ++meshIndex)
	{
		result = meshNode->mayaMeshData->addFaces(meshNode->shaderGroupIndex, builder);
		MESSENGER_REJECT(!result, ("failed to add faces for meshNode index %d\n", meshIndex));
	}

	return true;
}

// ======================================================================

void MayaMeshReader::install(Messenger *newMessenger)
{
	messenger = newMessenger;

	ms_globalVertexIndexer = new PositionVertexIndexer();
	ms_globalVertexIndexer->reserve(8192);
}

// ----------------------------------------------------------------------

void MayaMeshReader::remove(void)
{
	messenger = 0;

	delete ms_globalVertexIndexer;
	ms_globalVertexIndexer = 0;
}

// ======================================================================
// construct the MayaMeshReader from the specified objectArray
//
// Remarks:
//
//   This routine expects transformList to contain one to many transform DAG nodes.
//   Each non-intermediate mesh associated with one of the specified transform
//   nodes will be lumped together into the single logical mesh this reader represents.
//
//   Use buildMesh() to construct a mesh-related object using a MeshBuilder.
//
//   Use generateShaderTemplateData() to generate ShaderTemplate data files
//   for the shader groups accessed by the Maya mesh objects.
//
//   The MayaMeshReader constructor should not fail.  Error is instead indicated
//   by a call to MayaMeshReader::isValid().  Call that function to determine
//   if an error occurred during construction of this class.
//
//   The logical mesh contained by this routine may be composed
//   of multiple Maya meshes (see constructor documentation).  Maya world space will be the
//   local space of the logical mesh.  The vertices of each Maya mesh 
//   in the logical mesh will be transformed into the scene's world space before
//   being presented to the builder.
//
//   When multiple Maya meshes are passed to the routine, the routine will
//   aggregate polys by shader group such that all polys for the same ShaderTemplate
//   will be presented to the builder with one call to MeshBuilder::setShaderTemplate().

MayaMeshReader::MayaMeshReader(const MSelectionList& selectionList)
:	isValidFlag(false),
	mayaMeshData(),
	perShaderData()
{
	MESSENGER_INDENT;

	bool  result;

	// build the list of MayaMeshData objects from given objectArray
	result = buildMayaMeshDataList(selectionList);
	MESSENGER_REJECT_VOID(!result, ("failed to build list of MayaMeshData\n"));

	result = buildPerShaderDataList();
	MESSENGER_REJECT_VOID(!result, ("failed to build list of PerShaderData\n"));

	// we're valid if we made it through the above
	isValidFlag = true;
}

// ----------------------------------------------------------------------

MayaMeshReader::MayaMeshReader(const MDagPath &meshObject)
:	isValidFlag(false),
	mayaMeshData(),
	perShaderData()
{
	MESSENGER_INDENT;

	bool  result;

	// build the list of MayaMeshData objects from given objectArray
	result = buildMayaMeshDataList(meshObject);
	MESSENGER_REJECT_VOID(!result, ("failed to build list of MayaMeshData\n"));

	result = buildPerShaderDataList();
	MESSENGER_REJECT_VOID(!result, ("failed to build list of PerShaderData\n"));

	// we're valid if we made it through the above
	isValidFlag = true;
}

// ----------------------------------------------------------------------
/**
 * destroy the MayaMeshReader.
 */

MayaMeshReader::~MayaMeshReader(void)
{
	MESSENGER_INDENT;

	while (!mayaMeshData.empty())
	{
		delete mayaMeshData.back();
		mayaMeshData.pop_back();
	}
	
	while (!perShaderData.empty())
	{
		delete perShaderData.back();
		perShaderData.pop_back();
	}
}

// ----------------------------------------------------------------------
/**
 * build a MayaMeshData for each mesh associated with the list of
 * transform nodes.
 * 
 * For each transform node in objectArray, this routine will create
 * a MayaMeshData for all direct descendant non-intermediate mesh nodes of
 * the transform node.
 */

bool MayaMeshReader::buildMayaMeshDataList(const MSelectionList &transformList)
{
	const unsigned int transformCount = transformList.length();
	if (!transformCount)
		return false;

	MDagPath      transformDagPath;
	MDagPath      meshDagPath;
	MObject       childObject;
	MStatus       status;

	for (unsigned int transformIndex = 0; transformIndex < transformCount; ++transformIndex)
	{
		// get dag path for transform
		status = transformList.getDagPath(transformIndex, transformDagPath);

		if (transformDagPath.hasFn(MFn::kTransform))
		{
			// check each child of transform to see if its a non-intermediate mesh
			const unsigned int childCount = transformDagPath.childCount();
			for (unsigned int childIndex = 0; childIndex < childCount; ++childIndex)
			{
				childObject = transformDagPath.child(childIndex, &status);
				if (!status)
					return false;

				if (childObject.apiType() == MFn::kMesh)
				{
					// create the dag path to the child mesh
					meshDagPath = transformDagPath;
					status      = meshDagPath.push(childObject);
					if (!status)
						return false;

					// check if the node is an intermediate object
					MFnDagNode  meshDagNode(meshDagPath, &status);
					if (!status)
						return false;
					const bool isIntermediateMesh = meshDagNode.isIntermediateObject(&status);
					if (!status)
						return false;

					// if intermediate, skip
					if (isIntermediateMesh)
						continue;

					// build the Maya mesh object
					MayaMeshData *newMayaMeshData = new MayaMeshData(meshDagPath);
					mayaMeshData.push_back(newMayaMeshData);

					// if its invalid, bail on the process (we do this after appending to list so we clean it up properly)
					if (!newMayaMeshData->isValid)
						return false; //lint !e429, "pointer newMayaMeshData not freed", we store it and free it later
				}
			}
		}
		else if (transformDagPath.hasFn(MFn::kMesh))
		{
			// check if the node is an intermediate object
			MFnMesh  meshDagNode(transformDagPath);
			const bool isIntermediateMesh = meshDagNode.isIntermediateObject(&status);
			if (!status)
				return false;

			if (!isIntermediateMesh  && (meshDagNode.numPolygons() > 0))
			{
				// build the Maya mesh object
				MayaMeshData *newMayaMeshData = new MayaMeshData(transformDagPath);
				mayaMeshData.push_back(newMayaMeshData);
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::buildMayaMeshDataList (const MDagPath &meshDagPath)
{
	MayaMeshData *newMayaMeshData = new MayaMeshData(meshDagPath);
	mayaMeshData.push_back(newMayaMeshData);

	// if its invalid, bail on the process (we do this after appending to list so we clean it up properly)
	if (!newMayaMeshData->isValid)
		return false; //lint !e429, "pointer newMayaMeshData not freed", we store it and free it later

	return true;    //lint !e429, "pointer newMayaMeshData not freed", we store it and free it later
}

// ----------------------------------------------------------------------
/**
 * find PerShaderData node for the given ShaderTemplate base name,
 * creating as necessary.
 */

MayaMeshReader::PerShaderData *MayaMeshReader::getPerShaderData(const char *shaderTemplateBaseName, bool dot3, int normalMapTextureCoordinateIndex)
{
	if (!shaderTemplateBaseName || !*shaderTemplateBaseName)
		return 0;

	const std::string baseName = std::string (shaderTemplateBaseName);

	// search through list for target ShaderTemplate
  // Try to find an existing shader template (always add a new one for portals)
	std::vector<PerShaderData *>::iterator psd;
	for (psd = perShaderData.begin(); (psd!=perShaderData.end()); ++psd)
	{
		if ((*psd)->name == baseName && (*psd)->dot3 == dot3 && (*psd)->normalMapTextureCoordinateIndex == normalMapTextureCoordinateIndex) 
			break;
	}

	if (psd == perShaderData.end()) // add a new one
	{
		perShaderData.push_back(new PerShaderData(shaderTemplateBaseName, dot3, normalMapTextureCoordinateIndex));
		return perShaderData.back();
	}
	return *psd;
}

// ----------------------------------------------------------------------

bool MayaMeshReader::buildPerShaderDataList(void)
{
	// loop through all the maya meshes.  for each mesh, for each shader group, add the mesh to the
	// PerShaderData struct.

	char shaderTemplateBaseName[MAX_PATH];

	std::vector<MayaMeshData *>::iterator meshData;
	for (meshData = mayaMeshData.begin(); meshData != mayaMeshData.end(); ++meshData)
	{
		const unsigned int shaderGroupCount = (*meshData)->shaderGroupArray.length();
		for (unsigned int shaderGroupIndex = 0; shaderGroupIndex < shaderGroupCount; ++shaderGroupIndex)
		{
			const MObject shaderGroupObject = (*meshData)->shaderGroupArray[shaderGroupIndex];
			if (!MayaUtility::getNodeName(shaderGroupObject, shaderTemplateBaseName, sizeof(shaderTemplateBaseName)))
				return false;

			// chop off the optional trailing SG
			const size_t nameLength = strlen(shaderTemplateBaseName);
			if ((shaderTemplateBaseName[nameLength-2] == 'S') && (shaderTemplateBaseName[nameLength-1] == 'G'))
				shaderTemplateBaseName[nameLength-2] = 0;

			int               normalMapTextureCoordinateIndex = 0;
			bool              dot3 = false;
			MStatus           status;
			MObjectArray      objectArray;
			MFnDependencyNode shaderDependencyNode(shaderGroupObject, &status);

			if (MayaUtility::findSourceObjects(shaderDependencyNode, "surfaceShader", &objectArray) && objectArray.length() == 1)
			{
				MFnPhongShader phongShader(objectArray[0], &status);
				if (status == MS::kSuccess)
				{
					MObject normalMap = phongShader.attribute ("soe_textureName_NRML", &status);
					if (status != MS::kSuccess)
					{
						// check to see if it's a compressed normal map
						normalMap = phongShader.attribute ("soe_textureName_CNRM", &status);
					}
					if (status == MS::kSuccess) 
					{
						MObject texCoordSet = phongShader.attribute ("soe_texCoordSet_NRML_uv", &status);
						if (status == MS::kSuccess)
						{
							MPlug uvPlug = phongShader.findPlug(texCoordSet, &status);
							if (status == MS::kSuccess)
							{
								status = uvPlug.getValue(normalMapTextureCoordinateIndex);
								if (status == MS::kSuccess)
									dot3 = true;
							}
						}
					}
				}
			}

			PerShaderData *const psd = getPerShaderData(shaderTemplateBaseName, dot3, normalMapTextureCoordinateIndex);
			if (!psd->addMayaMeshData(*meshData, static_cast<int>(shaderGroupIndex)))
				return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * generate the ShaderTemplates for the Maya shader groups
 * associated with the logical mesh.
 * 
 * This routine lets the client specify what game runtime reference directories
 * should be used for textures and shader effects (necessary information
 * when generating a ShaderTemplate data file).  In addition, the directory
 * where the ShaderTemplate data files will be written is specified.
 * 
 * ShaderTemplate base names are predetermined by the name of the shader group.  The ShaderTemplate
 * base name will be the shader group's name, removing the final "SG" if present,
 * with the extension ".sht" appended.
 * 
 * All specified directories should contain a trailing backslash character (e.g. "some_directory\\").
 * 
 * @param textureReferenceDir     [IN] reference directory for shader template's textures (e.g. "texture\\")
 * @param effectReferenceDir      [IN] reference directory for shader template's effect (e.g. "effect\\")
 * @param shaderTemplateWriteDir  [IN] directory where generated ShaderTemplate data files will be written
 * @param textureWriteDir         [IN] directory where generated textures should be written
 */

bool MayaMeshReader::generateShaderTemplateData(
	const MeshBuilder &builder, 
	const char *textureReferenceDir, 
	const char *effectReferenceDir, 
	const char *shaderTemplateReferenceDir, 
	const char *shaderTemplateWriteDir, 
	const char *textureWriteDir
	) const
{
	MayaUtility::TextureSet referencedTextures;

	//------------------------------------------------------------------
	//-- export shader templates
	std::vector<PerShaderData *>::const_iterator psi;
	for (psi = perShaderData.begin(); psi!=perShaderData.end(); ++psi)
	{
		bool result;
		char shaderTemplateWriteName[MAX_PATH];

		const PerShaderData *psd = *psi;

		// create ShaderTemplate full write path
		sprintf(shaderTemplateWriteName, "%s%s.sht", shaderTemplateWriteDir, psd->name.c_str ());

		ExporterLog::addSourceShader(psd->name.c_str());

		// generate the shader template if necessary
		if (!psd->shaderMeshNodes.empty())
		{
			std::vector<std::pair<std::string, Tag> > referencedTextureRenderers;
			const char *const textureRendererReferenceDir = "";

			//-- Retrieve the dot3 bump mapping information associated with this shader.  This information
			//   comes from the Builder because the builder does all the work of figuring out what data is put
			//   in which texture coordinate sets.  This whole system feels extremely clunky.
			bool  hasDot3TextureCoordinate                  = false;
			int   dot3TextureCoordinateIndex                = -1;
			char  shaderTemplateReferenceName[2 * MAX_PATH];

			//-- create shader template name string
			sprintf(shaderTemplateReferenceName, "%s%s.sht", shaderTemplateReferenceDir, psd->name.c_str());

			//-- Use name to get dot3 info from builder.
			builder.getDot3TextureCoordinateInfo(shaderTemplateReferenceName, hasDot3TextureCoordinate, dot3TextureCoordinateIndex);

			//-- Create the ShaderTemplate data.
			result = MayaShaderTemplateBuilder::buildShaderTemplate(
				shaderTemplateWriteName, 
				psd->shaderGroupObject, 
				psd->hasVertexAlpha(), 
				referencedTextureRenderers, 
				textureReferenceDir, 
				textureRendererReferenceDir, 
				effectReferenceDir, 
				referencedTextures, 
				hasDot3TextureCoordinate, 
				dot3TextureCoordinateIndex
			);

			MESSENGER_REJECT (!result, ("MayaShaderTemplateBuilder::buildShaderTemplate() failed for \"%s\"\n", shaderTemplateWriteName));
		}
	}
	//------------------------------------------------------------------

	bool exportSuccess = true;

	//------------------------------------------------------------------
	//-- export textures
	// -TRF- fill up existing crc map from ExporterLog
	MayaUtility::FileCrcMap existingCrcMap;
	MayaUtility::loadFileCrcMap(&existingCrcMap, referencedTextures);

	// export source-modified textures
	MayaUtility::FileCrcMap revisedCrcMap;

	const bool textureExportSuccess = MayaUtility::generateTextures(&referencedTextures, &existingCrcMap, textureWriteDir, &revisedCrcMap, false);

	//don't fatal if this fails, just report the failure
	if (!textureExportSuccess)
		exportSuccess = false;

	// tell exporter log about new crc values
	MayaUtility::saveFileCrcMap(existingCrcMap, revisedCrcMap, referencedTextures);
	//------------------------------------------------------------------

	return exportSuccess;
}

// ----------------------------------------------------------------------

const char* MayaMeshReader::getFirstShaderTemplateName (void) const
{
	if (perShaderData.empty()) return 0;
	else return perShaderData.front()->name.c_str ();
}

// ----------------------------------------------------------------------
/**
 * construct a mesh object using the given MeshBuilder object.
 * 
 * @param builder  [IN] the construction process will use this builder to perform the mesh construction
 * @param shaderTemplateReferenceDir  [IN] specifies the game runtime reference directory for ShaderTemplate data, required to properly pass ShaderTemplate names to the MeshBuilder
 */

bool MayaMeshReader::buildMesh(MeshBuilder *builder, const char *shaderTemplateReferenceDir, const char *textureReferenceDir) const
{
	MESSENGER_INDENT;

	MESSENGER_REJECT(!builder, ("null builder arg\n"));
	MESSENGER_REJECT(!shaderTemplateReferenceDir, ("null shaderTemplateReferenceDir\n"));

	bool result;

	// construction process
	if (builder->hasFrames ())
	{
		// specify frame information to builder
		const MTime  minTime     = MAnimControl::minTime();
		const MTime  maxTime     = MAnimControl::maxTime();
		const MTime  currentTime = MAnimControl::currentTime();

		// validate units
		real              framesPerSecond  = CONST_REAL(24.0);
		bool              goodTimeUnit     = true;
		const MTime::Unit unit             = currentTime.unit();

		//lint -save -e1015 -e1013 -e788 // shut off enum-related warnings, lint's getting confused
		switch (unit)
		{
			case MTime::kFilm:
				framesPerSecond = CONST_REAL(24.0);
				break;
			case MTime::kNTSCFrame:
				framesPerSecond = CONST_REAL(30.0);
				break;
			default:
				goodTimeUnit = false;
		}
		//lint -restore
		MESSENGER_REJECT(!goodTimeUnit, ("time units must be Film or NTSC Frame\n"));

		// -TRF- may want to pass this data into the MayaMeshReader
		// assumes MTime is in unit that is integral-based
		const int    minFrameNumber     = static_cast<int>(minTime.value());
		const int    maxFrameNumber     = static_cast<int>(maxTime.value());
		const int    currentFrameNumber = static_cast<int>(currentTime.value());

		MESSENGER_LOG(("exporting frame number %d, range %d - %d\n", currentFrameNumber, minFrameNumber, maxFrameNumber));

		result = builder->setFrame(currentFrameNumber, minFrameNumber, maxFrameNumber, framesPerSecond);
		MESSENGER_REJECT(!result, ("failed to set builder's frame number\n"));
	}

	// construct each shader's set of maya meshes
	int            perShaderIndex;

	std::vector<PerShaderData *>::const_iterator psd;
	perShaderIndex = 0;
	for (psd = perShaderData.begin(); psd != perShaderData.end(); ++psd)
	{
		result = (*psd)->construct(builder, shaderTemplateReferenceDir, textureReferenceDir);
		MESSENGER_REJECT(!result, ("failed to construct PerShaderData index %d\n", perShaderIndex));
		++perShaderIndex;
	}
	MESSENGER_LOG(("buildMesh(): constructed mesh with %d unique ShaderTemplates\n", perShaderIndex));

	return true;
}

// ======================================================================
