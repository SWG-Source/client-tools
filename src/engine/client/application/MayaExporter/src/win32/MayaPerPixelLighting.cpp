// ======================================================================
//
// MayaPerPixelLighting.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaPerPixelLighting.h"

#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnMesh.h"
#include "maya/MFnPhongShader.h"
#include "maya/MIntArray.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "MayaConversions.h"
#include "MayaLightMeshReader.h"
#include "MayaUtility.h"
#include "MeshBuilder.h"
#include "Messenger.h"

#include <map>
#include <vector>

// ======================================================================
// Implementation-detail statics.
// ======================================================================

const float cs_dot3ComponentEpsilon = .001f;

namespace
{
	Messenger *messenger;
}

// ======================================================================
/**
 * A helper class that partners with MayaLightMeshReader and MeshBuilder
 * to build the dot3 vector used for per-pixel normal map lighting.
 */

class MayaPerPixelLighting::Dot3Builder
{
public:

	static bool buildDot3Values(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount);

public:

	Dot3Builder(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount);

	bool        postProcessResults();

private:

	typedef std::vector<Vector>  VectorVector;

private:

	static bool callback(void *context, int shaderIndex, int polyIndex, const int polyVertexIndex[3]);

private:

	bool        processTriangle(int shaderIndex, int polyIndex, const int polyVertexIndex[3]);
	bool        processVertex(int positionIndex, int normalIndex, int shaderIndex, const MeshBuilder::Face &face, int faceVertexIndex);

	// Disabled.
	Dot3Builder();

private:

	MItMeshPolygon                 m_polygonIterator;
	const ShaderRequestInfoVector &m_shaderRequestInfoVector;
	const Dot3KeyMap              &m_dot3KeyMap;
	FloatVector                   &m_dot3ComponentVector;

	VectorVector                   m_positionVectors;
	VectorVector                   m_normalVectors;

	std::vector<int>               m_dot3UnflippedProcessCount;
	std::vector<int>               m_dot3FlippedProcessCount;

	VectorVector                   m_dot3DuVectors;
	VectorVector                   m_dot3DvVectors;

	VectorVector                   m_dot3FlippedDuVectors;
	VectorVector                   m_dot3FlippedDvVectors;

	VectorVector                   m_dot3NormalVectors;
};

// ======================================================================
/**
 * A helper class that partners with MayaLightMeshReader to build a map
 * of all unique {position index, normal index, shader index} combinations
 * that occur in a specified mesh.
 *
 * Each unique key is assigned a unique, 0-based sequential number that
 * can be used for a sequential container storing the mesh dot3 vector
 * values.
 */

class MayaPerPixelLighting::Mapper
{
public:

	static bool mapVertices(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount);

public:

	Mapper(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount);
	int getProcessedVertexCount() const;

private:

	static bool callback(void *context, int shaderIndex, int polyIndex, const int polyVertexIndex[3]);

private:

	bool handleTriangle(int shaderIndex, int polyIndex, const int polyVertexIndex[3]);

	// Disabled.
	Mapper();

private:

	MItMeshPolygon                 m_polygonIterator;
	const ShaderRequestInfoVector &m_shaderRequestInfoVector;
	Dot3KeyMap                    &m_dot3KeyMap;
	int                           &m_dot3ValueCount;
	int                            m_processedVertexCount;

};

// ======================================================================
// class MayaPerPixelLighting::Dot3Builder
// ======================================================================

bool MayaPerPixelLighting::Dot3Builder::buildDot3Values(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount)
{
	// Create the context class.
	Dot3Builder builder(mesh, shaderRequestInfoVector, dot3KeyMap, dot3ComponentVector, dot3ValueCount);

	// Process the triangles.
	const bool enumerateSuccess = MayaLightMeshReader::enumerateTriangles(mesh, &builder, callback);
	MESSENGER_REJECT(!enumerateSuccess, ("MayaLightMeshReader::enumerateTriangles() failed.\n"));

	// Post process the result.
	const bool postProcessSuccess = builder.postProcessResults();
	MESSENGER_REJECT(!postProcessSuccess, ("builder.postProcessResults() failed.\n"));

	// Success.
	return true;
}

// ======================================================================

MayaPerPixelLighting::Dot3Builder::Dot3Builder(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount) :
	m_polygonIterator(mesh.object()),
	m_shaderRequestInfoVector(shaderRequestInfoVector),
	m_dot3KeyMap(dot3KeyMap),
	m_dot3ComponentVector(dot3ComponentVector),
	m_positionVectors(),
	m_normalVectors(),
	m_dot3UnflippedProcessCount(static_cast<std::vector<int>::size_type>(dot3ValueCount)),
	m_dot3FlippedProcessCount(static_cast<std::vector<int>::size_type>(dot3ValueCount)),
	m_dot3DuVectors(static_cast<VectorVector::size_type>(dot3ValueCount)),
	m_dot3DvVectors(static_cast<VectorVector::size_type>(dot3ValueCount)),
	m_dot3FlippedDuVectors(static_cast<VectorVector::size_type>(dot3ValueCount)),
	m_dot3FlippedDvVectors(static_cast<VectorVector::size_type>(dot3ValueCount)),
	m_dot3NormalVectors(static_cast<VectorVector::size_type>(dot3ValueCount))
{
	//-- Prepare the destination arrays.
	m_dot3ComponentVector.reserve(static_cast<FloatVector::size_type>(4 * dot3ValueCount));
	m_dot3ComponentVector.clear();

	//-- Retrieve the mesh position values, convert to game coordinates.
	MFloatPointArray  mayaPositions;

	// Get the positions.
	MStatus status = mesh.getPoints(mayaPositions, MSpace::kWorld);
	if (!status)
		MESSENGER_LOG_ERROR(("Dot3Builder(): mesh.getPoints() failed [%s].\n", status.errorString().asChar()));

	// Convert to game positions.
	{
		const unsigned length = mayaPositions.length();
		m_positionVectors.reserve(length);

		for (unsigned i = 0; i < length; ++i)
			m_positionVectors.push_back(MayaConversions::convertPoint(mayaPositions[i]));
	}

	//-- Retrieve the mesh normal values.
	MFloatVectorArray  mayaNormals;

	// Get the normals.
	status = mesh.getNormals(mayaNormals, MSpace::kWorld);
	if (!status)
		MESSENGER_LOG_ERROR(("Dot3Builder(): mesh.getNormals() failed [%s].\n", status.errorString().asChar()));

	// Convert to game normals.
	{
		const unsigned length = mayaNormals.length();
		m_normalVectors.reserve(length);

		for (unsigned i = 0; i < length; ++i)
			m_normalVectors.push_back(MayaConversions::convertVector(mayaNormals[i]));
	}
}

// ----------------------------------------------------------------------

bool MayaPerPixelLighting::Dot3Builder::postProcessResults()
{
	VectorVector::size_type  vectorCount     = m_dot3DuVectors.size();

	for (VectorVector::size_type i = 0; i < vectorCount; ++i)
	{
		// Ensure data has been captured for this dot3 vector.
		MESSENGER_REJECT((m_dot3UnflippedProcessCount[i] < 1) && (m_dot3FlippedProcessCount[i] < 1), ("failed to collect any dot3 transform data for unique dot3 vector index [%i].\n", static_cast<int>(vectorCount)));

		// Figure out whether to use flipped vector.
		const bool useFlipped = (m_dot3FlippedProcessCount[i] >= m_dot3UnflippedProcessCount[i]);

		// Post process the du/dv vectors.
		Vector &du     = useFlipped ? m_dot3FlippedDuVectors[i] : m_dot3DuVectors[i];
		Vector &dv     = useFlipped ? m_dot3FlippedDvVectors[i] : m_dot3DvVectors[i];
		Vector &normal = m_dot3NormalVectors[i];

		IGNORE_RETURN(du.normalize());
		IGNORE_RETURN(dv.normalize());

		if (useFlipped)
		{
			// j = k.cross(i);
			dv = -normal.cross(du);

			// i = j.cross(k);
			du = -dv.cross(normal);
		}
		else
		{
			// j = k.cross(i);
			dv = normal.cross(du);

			// i = j.cross(k);
			du = dv.cross(normal);
		}

		IGNORE_RETURN(du.normalize());
		IGNORE_RETURN(dv.normalize());

		// Write out the dot3 vector.
		m_dot3ComponentVector.push_back(du.x);
		m_dot3ComponentVector.push_back(du.y);
		m_dot3ComponentVector.push_back(du.z);
		m_dot3ComponentVector.push_back((useFlipped ? -1.0f : 1.0f));
	}

	//-- Indicate success.
	return true;
}

// ======================================================================

bool MayaPerPixelLighting::Dot3Builder::callback(void *context, int shaderIndex, int polyIndex, const int polyVertexIndex[3])
{
	NOT_NULL(context);

	Dot3Builder *const builder = reinterpret_cast<Dot3Builder*>(context);
	return builder->processTriangle(shaderIndex, polyIndex, polyVertexIndex);
}

// ======================================================================

bool MayaPerPixelLighting::Dot3Builder::processTriangle(int shaderIndex, int polyIndex, const int polyVertexIndex[3])
{
	//-- Check if this shader uses dot3.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, static_cast<int>(m_shaderRequestInfoVector.size()));
	
	const ShaderRequestInfo &info = m_shaderRequestInfoVector[static_cast<ShaderRequestInfoVector::size_type>(shaderIndex)];
	if (!info.isDot3Requested())
	{
		// Skip this triangle, it isn't per-pixel lit.
		return true;
	}

	// @todo make this work --- currently this value is ignored and the calculation is always completed off of the
	//       first maya UV set.
	// const int dot3CalculationUvSetIndex = info.getCalculationTextureCoordinateSet();

	//-- Set the iterator to the specified face.
	int previousIndex;

	MStatus status = m_polygonIterator.setIndex(polyIndex, previousIndex);
	STATUS_REJECT(status, "failed to set polygon iterator index");

	//-- Get the face data (position index and value, normal index and value, UV values for dot3 calculation.
	const int polygonVertexCount = m_polygonIterator.polygonVertexCount(&status);
	STATUS_REJECT(status, "m_polygonIterator.getPolygonVertexCount() failed");
	UNREF(polygonVertexCount);

	const int          faceUvSetIndex = 0;

	Vector             faceNormal;
	MeshBuilder::Face  face;
	int                positionIndex[3];
	int                normalIndex[3];
	float              mayaUv[2];

	Zero(face);

	for (int i = 0; i < 3; ++i)
	{
		const int localVertexIndex = polyVertexIndex[i];
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localVertexIndex, polygonVertexCount);

		//-- Retrieve key fields for unique dot3 vector coordinate {position index, normal index, shader index}
		// Get the object-relative position index for this poly vertex.
		positionIndex[i] = m_polygonIterator.vertexIndex(localVertexIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get position index for poly index [%d] local vertex index [%d] [%s].\n", polyIndex, polyVertexIndex[i], status.errorString().asChar()));

		// Get the object-relative normal index this poly vertex.
		normalIndex[i] = m_polygonIterator.normalIndex(localVertexIndex, &status);
		MESSENGER_REJECT(!status, ("failed to get normal index for poly index [%d] local vertex index [%d] [%s].\n", polyIndex, polyVertexIndex[i], status.errorString().asChar()));

		//-- Build face data for du/dv calculation.
		// Get position vector.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, positionIndex[i], static_cast<int>(m_positionVectors.size()));
		face.positionArray[i] = m_positionVectors[static_cast<VectorVector::size_type>(positionIndex[i])];

		// Get normal vector.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, normalIndex[i], static_cast<int>(m_normalVectors.size()));
		face.vertexNormalArray[i] = m_normalVectors[static_cast<VectorVector::size_type>(normalIndex[i])];

		//-- Sum up normal vectors to compute average face normal.
		faceNormal += face.vertexNormalArray[i];

		// Get UV values for UV calculation set.
		status = m_polygonIterator.getUV(localVertexIndex, mayaUv);
		MESSENGER_REJECT(!status, ("failed to get uv for poly index [%d] local vertex index [%d]\n", polyIndex, localVertexIndex));

		// Convert Maya UVs to game UVs.
		// @todo we do not always want to fold UVs into the 0..1 range.
		MayaConversions::convertMayaUvToZeroOneRange(mayaUv, face.uArray[faceUvSetIndex][i], face.vArray[faceUvSetIndex][i]);
	}

	//-- Fixup face for processing.
	IGNORE_RETURN(faceNormal.normalize());
	face.faceNormal = faceNormal;
	face.uvCount    = 1;

	//-- Calculate Face dot3 vector.
	MeshBuilder::computeDot3Transform(face, 0);
	if (!face.dot3TransformMatrixValid)
	{
		MESSENGER_LOG_ERROR(("MeshBuilder::computeDot3Transform() failed to compute valid dot3 transform matrix data.\n"));
		return false;
	}
	
	//-- Incorporate face dot3 for each vert {position index, normal index, shader index} in the face.
	//   We add these all up for now and keep track of the count.  The values are normalized in the
	//   postProcessResults step.
	for (int j = 0; j < 3; ++j)
	{
		const bool success = processVertex(positionIndex[j], normalIndex[j], shaderIndex, face, j);
		MESSENGER_REJECT(!success, ("failed to process vertex with position index [%d], normal index [%d], shader index [%d].\n", positionIndex[j], normalIndex[j], shaderIndex));
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Process per-pixel-lighting dot3 vector given face dot3 transform info.
 *
 * @param positionIndex  the object-relative position index for the vertex.
 * @param normalIndex    the object-relative normal index for the vertex.
 * @param shaderIndex    the object-relative shader index for the vertex.
 * @param face           the face containing the calculated dot3 vector for that particular face.
 */

bool MayaPerPixelLighting::Dot3Builder::processVertex(int positionIndex, int normalIndex, int shaderIndex, const MeshBuilder::Face &face, int faceVertexIndex)
{
	//-- Find the dot3 index for this vertex.
	const Dot3Key key(positionIndex, normalIndex, shaderIndex);

	const Dot3KeyMap::const_iterator findIt = m_dot3KeyMap.find(key);
	MESSENGER_REJECT(findIt == m_dot3KeyMap.end(), ("Mesh vertex position index [%d], normal index [%d], shader index [%d] does not have a dot3 mapping.", positionIndex, normalIndex, shaderIndex));

	const int                     dot3Index  = findIt->second;
	const VectorVector::size_type dot3IndexU = static_cast<VectorVector::size_type>(dot3Index);

	//-- Check if this is a new value.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, dot3Index, static_cast<int>(m_dot3UnflippedProcessCount.size()));

	// This is the first time this dot3 value is getting processed.

	// Capture the vertex normal.
	m_dot3NormalVectors[dot3IndexU] = face.vertexNormalArray[faceVertexIndex];

	// Capture the du, dv vectors.
	if (face.dot3TransformMatrixFlipped)
	{
		m_dot3FlippedDuVectors[dot3IndexU] += face.du;
		m_dot3FlippedDvVectors[dot3IndexU] += face.dv;

		++m_dot3FlippedProcessCount[dot3IndexU];
	}
	else
	{
		m_dot3DuVectors[dot3IndexU] += face.du;
		m_dot3DvVectors[dot3IndexU] += face.dv;

		++m_dot3UnflippedProcessCount[dot3IndexU];
	}

	//-- Indicate success.
	return true;
}

// ======================================================================
// class MayaPerPixelLighting::Mapper
// ======================================================================

bool MayaPerPixelLighting::Mapper::mapVertices(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount)
{
	// Create the context class.
	Mapper  mapper(mesh, shaderRequestInfoVector, dot3KeyMap, dot3ValueCount);

	// Process the triangles.
	const bool result = MayaLightMeshReader::enumerateTriangles(mesh, &mapper, callback);

#if 1
	if (dot3ValueCount < 1)
	{
		MESSENGER_LOG(("MayaPerPixelLighting::Mapper: no vertices tagged for dot3.\n"));
	}
	else
	{
		const int   vertexCount      = mapper.getProcessedVertexCount();
		const float shareCompression = (vertexCount > 0) ? 100.0f * static_cast<float>(dot3ValueCount) / static_cast<float>(vertexCount) : 0.0f;

		MESSENGER_LOG(("MayaPerPixelLighting::Mapper: processed         [%d] vertices.\n", vertexCount));
		MESSENGER_LOG(("MayaPerPixelLighting::Mapper: mapped            [%d] dot3 transforms.\n", dot3ValueCount));
		MESSENGER_LOG(("MayaPerPixelLighting::Mapper: share compression [%.0f] percent.\n", shareCompression));
	}
#endif

	return result;
}

// ======================================================================

MayaPerPixelLighting::Mapper::Mapper(const MFnMesh &mesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount) :
	m_polygonIterator(mesh.object()),
	m_shaderRequestInfoVector(shaderRequestInfoVector),
	m_dot3KeyMap(dot3KeyMap),
	m_dot3ValueCount(dot3ValueCount),
	m_processedVertexCount(0)
{
	// Ensure there are no entries in the key map.
	m_dot3KeyMap.clear();
	m_dot3ValueCount = 0;	
}

// ----------------------------------------------------------------------

int MayaPerPixelLighting::Mapper::getProcessedVertexCount() const
{
	return m_processedVertexCount;
}

// ======================================================================

bool MayaPerPixelLighting::Mapper::callback(void *context, int shaderIndex, int polyIndex, const int polyVertexIndex[3])
{
	NOT_NULL(context);

	//-- Ignore calls for triangles on the default shader.
	if (shaderIndex < 0)
		return true;

	//-- Handle mapping.
	Mapper *const mapper = reinterpret_cast<Mapper*>(context);
	return mapper->handleTriangle(shaderIndex, polyIndex, polyVertexIndex);
}

// ======================================================================

bool MayaPerPixelLighting::Mapper::handleTriangle(int shaderIndex, int polyIndex, const int polyVertexIndex[3])
{
	//-- Check if this shader uses dot3.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, static_cast<int>(m_shaderRequestInfoVector.size()));
	
	const ShaderRequestInfo &info = m_shaderRequestInfoVector[static_cast<ShaderRequestInfoVector::size_type>(shaderIndex)];
	if (!info.isDot3Requested())
	{
		// Skip this triangle, it isn't per-pixel lit.
		return true;
	}

	//-- Set the iterator to the specified face.
	int previousIndex;

	MStatus status = m_polygonIterator.setIndex(polyIndex, previousIndex);
	STATUS_REJECT(status, "failed to set polygon iterator index");

	//-- Get the position and normal indices.
	const int polygonVertexCount = m_polygonIterator.polygonVertexCount(&status);
	STATUS_REJECT(status, "m_polygonIterator.getPolygonVertexCount() failed");
	UNREF(polygonVertexCount);

	for (int i = 0; i < 3; ++i)
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, polyVertexIndex[i], polygonVertexCount);

		//-- Retrieve key fields for unique dot3 vector coordinate {position index, normal index, shader index}
		// Get the object-relative position index for this poly vertex.
		const int positionIndex = m_polygonIterator.vertexIndex(polyVertexIndex[i], &status);
		MESSENGER_REJECT(!status, ("failed to get position index for poly index [%d] local vertex index [%d] [%s].\n", polyIndex, polyVertexIndex[i], status.errorString().asChar()));

		// Get the object-relative normal index this poly vertex.
		const int normalIndex = m_polygonIterator.normalIndex(polyVertexIndex[i], &status);
		MESSENGER_REJECT(!status, ("failed to get normal index for poly index [%d] local vertex index [%d] [%s].\n", polyIndex, polyVertexIndex[i], status.errorString().asChar()));

		//-- Add a mapping entry if one does not yet exist.
		const Dot3Key key(positionIndex, normalIndex, shaderIndex);

		Dot3KeyMap::iterator lowerBoundResult = m_dot3KeyMap.lower_bound(key);
		if ((lowerBoundResult == m_dot3KeyMap.end()) || m_dot3KeyMap.key_comp()(key, lowerBoundResult->first))
		{
			// Create a mapping for this unique dot3 vector key.  Assign a new slot in the dot3 vector array.
			IGNORE_RETURN(m_dot3KeyMap.insert(lowerBoundResult, Dot3KeyMap::value_type(key, m_dot3ValueCount)));
			++m_dot3ValueCount;
		}

		++m_processedVertexCount;
	}

	//-- Success.
	return true;
}

// ======================================================================
// class MayaPerPixelLighting::Dot3Key
// ======================================================================

bool MayaPerPixelLighting::Dot3Key::operator <(const Dot3Key &rhs) const
{
	//-- Check position index (most significant portion of key because most likely to be different).
	if (m_positionIndex < rhs.m_positionIndex)
		return true;
	else if (m_positionIndex > rhs.m_positionIndex)
		return false;

	//-- Positions identical, now check normal index.
	if (m_normalIndex < rhs.m_normalIndex)
		return true;
	else if (m_normalIndex > rhs.m_normalIndex)
		return false;

	//-- Positions and normals identical, now check shader index.
	if (m_shaderIndex < rhs.m_shaderIndex)
		return true;
	else
		return false;
}

// ======================================================================
// class MayaPerPixelLighting: public static member functions
// ======================================================================

void MayaPerPixelLighting::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void MayaPerPixelLighting::remove()
{
}

// ----------------------------------------------------------------------
/**
 * Retrieve information for the mesh indicating which shader mesh data should
 * make use of per-pixel lighting, and if so, which existing texture coordinate set
 * should be used to generate the dot3 lighting vector.
 *
 * @param fnMesh                   the mesh to be analyzed.
 * @param shaderRequestInfoVector  the per-shader dot3 lighting request entries are returned in this container.
 *
 * @return  true if the operation completed successfully; false if there was some kind of error.
 *          In the event of an error, the results returned in shaderRequestInfoVector are undefined.
 */

bool MayaPerPixelLighting::getShaderRequestInfo(const MFnMesh &fnMesh, ShaderRequestInfoVector &shaderRequestInfoVector)
{
	MStatus  status;

	//-- Retrieve the shaders attached to the specified mesh.  Always assume the exporter handles
	//   instance 0 of the mesh.
	const unsigned     instanceIndex = 0;
	MObjectArray       shaderGroupArray;
	MIntArray          shaderPolyMapping;

	status = fnMesh.getConnectedShaders(instanceIndex, shaderGroupArray, shaderPolyMapping);
	STATUS_REJECT(status, "fnMesh.getConnectedShaders() failed");

	//-- Prepare return vector.
	const unsigned int shaderGroupCount = shaderGroupArray.length();

	shaderRequestInfoVector.reserve(shaderGroupCount);
	shaderRequestInfoVector.clear();

	//-- Collect info from each shader.
	MObjectArray       objectArray;
	MFnDependencyNode  shaderDependencyNode;

	for (unsigned int shaderGroupIndex = 0; shaderGroupIndex < shaderGroupCount; ++shaderGroupIndex)
	{
		const MObject shaderGroupObject = shaderGroupArray[shaderGroupIndex];
		int           calculationTcSet  = 0;
		bool          useDot3           = false;

		//-- Get the surface shader node for the shader group.
		status = shaderDependencyNode.setObject(shaderGroupObject);
		STATUS_REJECT(status, "shaderDependencyNode.setObject() failed on attached shader object");

		if (MayaUtility::findSourceObjects(shaderDependencyNode, "surfaceShader", &objectArray) && objectArray.length() == 1)
		{
			MFnPhongShader phongShader(objectArray[0], &status);
			if (status)
			{
				MObject normalMap = phongShader.attribute ("soe_textureName_NRML", &status);
				UNREF(normalMap);

				if (status != MS::kSuccess)
				{
					// check to see if it's a compressed normal map
					normalMap = phongShader.attribute ("soe_textureName_CNRM", &status);
				}
				if (status) 
				{
					MObject texCoordSet = phongShader.attribute ("soe_texCoordSet_NRML_uv", &status);
					if (status)
					{
						MPlug uvPlug = phongShader.findPlug(texCoordSet, &status);
						if (status)
						{
							status = uvPlug.getValue(calculationTcSet);
							if (status)
								useDot3 = true;
						}
					}
				}
			}
		}

		//-- Crate shader request info for this shader.
		shaderRequestInfoVector.push_back(ShaderRequestInfo(useDot3, calculationTcSet));
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool MayaPerPixelLighting::generateDot3KeyMap(const MFnMesh &fnMesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount)
{
	return Mapper::mapVertices(fnMesh, shaderRequestInfoVector, dot3KeyMap, dot3ValueCount);
}

// ----------------------------------------------------------------------

bool MayaPerPixelLighting::computeDot3Values(const MFnMesh &fnMesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount)
{
	//-- Retrieve the max slot index from the key map.
	return Dot3Builder::buildDot3Values(fnMesh, shaderRequestInfoVector, dot3KeyMap, dot3ComponentVector, dot3ValueCount);
}

// ----------------------------------------------------------------------

bool MayaPerPixelLighting::computeDot3DifferenceVector(const FloatVector &baseDot3Vector, const FloatVector &targetDot3Vector, Dot3DifferenceVector &differenceVector)
{
	//-- Validate arguments.
	FloatVector::size_type size = baseDot3Vector.size();
	MESSENGER_REJECT(size != targetDot3Vector.size(), ("dot3 vector size mismatch: base=[%d], target=[%d].\n", static_cast<int>(size), static_cast<int>(targetDot3Vector.size())));
	MESSENGER_REJECT((size % 4) != 0, ("dot3 vectors are not a multiple of 4 [%d].\n", static_cast<int>(size)));

	//-- Compare all elements looking for differences.
	for (FloatVector::size_type i = 0; i < size;)
	{
		const float delta    = targetDot3Vector[i] - baseDot3Vector[i];
		const bool  noChange = (delta >= -cs_dot3ComponentEpsilon) && (delta <= cs_dot3ComponentEpsilon);

		if (noChange)
		{
			// Move on to the next element.
			++i;
		}
		else
		{
			//const int componentIndex = static_cast<int>(i % 4);
			const int vectorIndex    = static_cast<int>(i / 4);

			// Ensure the fourth vector component has not changed.  This represents the winding direction of the vert in UV space.
			// It should always be -1.0 or 1.0 and should not change due to application of a blend target --- we can't
			// blend between these two discrete states.
			float const deltaW = targetDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 3)] - baseDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 3)];
			if (abs(deltaW) >= 0.001f)
			{
				// -TRF- We suppress this message now.  Note it is an indication of some kind of lighting error
				//       that is due to some combination of bad code, bad algorithm and bad data.  Since we haven't
				//       been able to detect the error when ignoring the blend shape's dot3-morphing effect, we are
				//       ignoring this issue.
				// MESSENGER_LOG_WARNING(("Dot3 vector [%d]'s uv winding direction differs, skipping this blend shape's dot3 vector changes.  Possibly a blend target texturing issue.\n", vectorIndex));
			}
			else
			{
				// Create the difference vector.
				differenceVector.push_back(Dot3Difference(
					vectorIndex,
					targetDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 0)] - baseDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 0)],
					targetDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 1)] - baseDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 1)],
					targetDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 2)] - baseDot3Vector[static_cast<FloatVector::size_type>(4 * vectorIndex + 2)],
					deltaW
					));
			}

			// Increment loop counter to first element of the next 4-component dot3 vector.
			i = static_cast<FloatVector::size_type>(vectorIndex + 1) * 4;
		}
	}

	//-- Success.
	return true;
}

// ======================================================================
