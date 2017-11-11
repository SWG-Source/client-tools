// ======================================================================
//
// MayaMeshWeighting.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaMeshWeighting.h"

#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MFloatArray.h"
#include "maya/MFn.h"
#include "maya/MFnMesh.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MFnSkinCluster.h"
#include "maya/MObject.h"
#include "maya/MObjectArray.h"
#include "maya/MStatus.h"
#include "maya/MString.h"
#include "Messenger.h"
#include <vector>
#include <algorithm>

// ======================================================================

Messenger *MayaMeshWeighting::messenger;
bool       MayaMeshWeighting::ms_installed;

// ======================================================================

struct MayaMeshWeighting::JointData
{
public:

	JointData(const MDagPath &dagPath, int index, const MString &name);

public:

	MDagPath  m_dagPath;
	int       m_index;
	MString   m_name;

private:

	JointData();

};

// ======================================================================

struct MayaMeshWeighting::JointDataContainer
{
public:

	// wanted this to be a map like this:
	//    typedef std::map<MDagPath, JointData>  Container;
	// but there is no less-than order of MDagPath or MObjects,
	// and the ways I can think of to brew my own are expensive.
	// Just use the vector
	typedef std::vector<JointData>  Container;

public:

	Container  m_container;

};

// ======================================================================

struct MayaMeshWeighting::TransformWeightData
{
public:

	class Less
	{
	public:

		bool operator ()(const TransformWeightData &lhs, const TransformWeightData &rhs);
	};

public:

	static bool less(const TransformWeightData &lhs, const TransformWeightData &rhs);

public:

	TransformWeightData(int vertexIndex, int transformIndex, real transformWeight);

public:

	int   m_vertexIndex;
	int   m_transformIndex;
	real  m_transformWeight;

private:

	TransformWeightData();

};

// ======================================================================

struct MayaMeshWeighting::TransformWeightDataContainer
{
public:

	typedef std::vector<TransformWeightData>  Container;

public:

	Container  m_container;

};

// ======================================================================

struct MayaMeshWeighting::VertexData
{
public:

	VertexData();

public:

	/// index into m_transformWeightData
	int m_firstIndex;

	/// number of transforms affecting this vertex
	int m_transformCount;
};

// ======================================================================

struct MayaMeshWeighting::VertexDataContainer
{
public:

	typedef std::vector<VertexData>  Container;

public:

	Container  m_container;

};

// ======================================================================
// class MayaMeshWeighting::JointData

inline MayaMeshWeighting::JointData::JointData(const MDagPath &dagPath, int index, const MString &name)
:
	m_dagPath(dagPath),
	m_index(index),
	m_name(name)
{
}


// ======================================================================
// class MayaMeshWeighting::TransformWeightData::Less

inline bool MayaMeshWeighting::TransformWeightData::Less::operator ()(const TransformWeightData &lhs, const TransformWeightData &rhs)
{
	return less(lhs, rhs);
}

// ======================================================================
// class MayaMeshWeighting::TransformWeightData

bool MayaMeshWeighting::TransformWeightData::less(const TransformWeightData &lhs, const TransformWeightData &rhs)
{
	if (lhs.m_vertexIndex < rhs.m_vertexIndex)
		return true;
	else if (lhs.m_vertexIndex > rhs.m_vertexIndex)
		return false;
	else 
	{
		// same vertex index, now order by transform index
		if (lhs.m_transformIndex <= rhs.m_transformIndex)
			return true;
		else
			return false;
	}
}

// ----------------------------------------------------------------------

inline MayaMeshWeighting::TransformWeightData::TransformWeightData(int vertexIndex, int transformIndex, real transformWeight)
:
	m_vertexIndex(vertexIndex),
	m_transformIndex(transformIndex),
	m_transformWeight(transformWeight)
{
}

// ======================================================================
// class MayaMeshWeighting::VertexData

inline MayaMeshWeighting::VertexData::VertexData()
:
	m_firstIndex(-1),
	m_transformCount(0)
{
}

// ======================================================================
// class MayaMeshWeighting

void MayaMeshWeighting::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaMeshWeighting already installed"));

	NOT_NULL(newMessenger);

	messenger    = newMessenger;
	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaMeshWeighting::remove()
{
	DEBUG_FATAL(!ms_installed, ("MayaMeshWeighting not installed"));

	messenger    = 0;
	ms_installed = true;
}

// ======================================================================

MayaMeshWeighting::MayaMeshWeighting(const MDagPath &meshDagPath, MStatus *status)
:
	m_isValid(false),
	m_isDirty(true),
	m_meshObject(0),
	m_meshDagPath(meshDagPath),
	m_meshCompleteComponent(0),
	m_meshPositionCount(0),
	m_jointData(0),
	m_transformWeightData(0),
	m_vertexData(0),
	m_singleAffectorAdded(false)
{
	DEBUG_FATAL(!ms_installed, ("MayaMeshWeighting not installed\n"));

	MStatus  localStatus;

	//-- make sure given dag path truly is a mesh
	MFn::Type argType = meshDagPath.apiType(&localStatus);
	if (argType != MFn::kMesh)
	{
		// error, exit
		if (status)
			*status = MStatus(MStatus::kFailure);

		MESSENGER_LOG_ERROR(("tried to create MayaMeshWeighting with a non mesh\n"));
		return;
	}

	//-- setup data members
	m_meshObject  = NON_NULL(new MObject);
	*m_meshObject = meshDagPath.node(&localStatus);
	if (!localStatus)
	{
		MESSENGER_LOG_ERROR(("failed to get node from dag path\n"));
		return;
	}

	//-- setup "complete" component for mesh
	// note: this is an object that simply says: I'm referring to all the
	// vertices in the mesh.  think of it like a mask for the mesh vertices.

	// get number of positions in the mesh
	MFnMesh fnMesh(*m_meshObject, &localStatus);
	if (!localStatus)
	{
		MESSENGER_LOG_ERROR(("failed to set MFnMesh\n"));
		return;
	}

	m_meshPositionCount = static_cast<int>(fnMesh.numVertices(&localStatus));
	if (!localStatus)
	{
		MESSENGER_LOG_ERROR(("failed to get vertex count\n"));
		return;
	}

	MESSENGER_LOG(("mesh [%s] has [%d] positions\n", m_meshDagPath.partialPathName().asChar(), m_meshPositionCount));

	// create vertex component object
	MFnSingleIndexedComponent fnComponent;

	m_meshCompleteComponent  = NON_NULL(new MObject());
	*m_meshCompleteComponent = fnComponent.create(MFn::kMeshVertComponent, &localStatus);
	if (!localStatus)
	{
		MESSENGER_LOG_ERROR(("failed to create mesh vertex component object\n"));
		return;
	}

	// set component to complete: add all verts
	localStatus = fnComponent.setCompleteData(m_meshPositionCount);
	if (!localStatus)
	{
		MESSENGER_LOG_ERROR(("failed to set vert component 'complete data' state\n"));
		return;
	}

	//-- create containers
	m_jointData           = NON_NULL(new JointDataContainer());
	m_transformWeightData = NON_NULL(new TransformWeightDataContainer());
	m_vertexData          = NON_NULL(new VertexDataContainer());

	//-- size containers
	m_vertexData->m_container.resize(static_cast<size_t>(m_meshPositionCount));

	//-- success
	m_isValid = true;
}

// ----------------------------------------------------------------------

MayaMeshWeighting::~MayaMeshWeighting()
{
	if (!m_isValid)
	{
		MESSENGER_LOG_ERROR(("deleting invalid MayaMeshWeighting [0x%08x]\n", this));
	}

	delete m_vertexData;
	delete m_transformWeightData;
	delete m_jointData;
	delete m_meshCompleteComponent;
	delete m_meshObject;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::addAffectorInfo(const MDagPath &affectorDagPath, int *transformIndex, bool &isNew)
{
	NOT_NULL(m_jointData);

	MESSENGER_INDENT;
	MStatus status;

	const size_t transformCount = m_jointData->m_container.size();
	bool         haveTransform  = false;

	size_t searchTransformIndex;
	for (searchTransformIndex = 0; searchTransformIndex < transformCount; ++searchTransformIndex)
	{
		const JointData &jointData = m_jointData->m_container[searchTransformIndex];
		if (jointData.m_dagPath == affectorDagPath)
		{
			// found it
			haveTransform = true;
			break;
		}
	}

	if (haveTransform)
	{
		*transformIndex = static_cast<int>(searchTransformIndex);
		isNew = false;
	}
	else
	{
		// this is a new influence for us, record and enter data for it
		*transformIndex = static_cast<int>(m_jointData->m_container.size());

		// convert to dependency node so we can get just the node name
		const MObject affectorObject = affectorDagPath.node(&status);
		MESSENGER_REJECT(!status, ("failed to get affector object\n"));

		MFnDependencyNode fnDependencyNode(affectorObject, &status);
		MESSENGER_REJECT(!status, ("failed to assign transform/joint object to MFnDependencyNode\n"));
		
		// create the joint data
		m_jointData->m_container.push_back(JointData(affectorDagPath, *transformIndex, fnDependencyNode.name()));

		isNew = true;
	}

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::removeAffectorInfo(int transformIndex)
{
	DEBUG_FATAL(transformIndex >= static_cast<int>(m_jointData->m_container.size()), ("transformIndex %d out of range [0..%u)\n", transformIndex, m_jointData->m_container.size()));

	JointDataContainer::Container::iterator it = m_jointData->m_container.begin();
	it += static_cast<size_t>(transformIndex);

	IGNORE_RETURN(m_jointData->m_container.erase(it));

	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::addTransformData(MFnSkinCluster &fnSkinCluster, const MDagPath &influenceDagPath,const unsigned influenceObjectIndex)
{
	MESSENGER_INDENT;
	// @todo add to a flag: too much spam for typical case but is useful info.
	// MESSENGER_LOG(("addTransformData(): processing node [%s]:\n", influenceDagPath.partialPathName().asChar()));

	NOT_NULL(m_jointData);
	NOT_NULL(m_meshCompleteComponent);
	NOT_NULL(m_transformWeightData);

	MStatus status;

	//-- check the type of the node
	MFn::Type nodeType = influenceDagPath.apiType(&status);
	MESSENGER_REJECT(!status, ("addTransformData(): failed to get dag path api type\n"));
	MESSENGER_REJECT(nodeType != MFn::kJoint, ("addTransformData(): skin cluster influence type [%s] not supported\n", influenceDagPath.partialPathName().asChar()));

	//-- add influence to the list of transforms
	int  transformIndex = -1;
	bool affectorIsNew  = false;

	const bool aaiSuccess = addAffectorInfo(influenceDagPath, &transformIndex, affectorIsNew);
	MESSENGER_REJECT(!aaiSuccess, ("addAffectorInfo() failed for [%s]\n", influenceDagPath.partialPathName().asChar()));

	//-- get the points affected by this influence (joint/transform)
	MFloatArray  weights;

	status = fnSkinCluster.getWeights(m_meshDagPath, *m_meshCompleteComponent, influenceObjectIndex, weights);
	MESSENGER_REJECT(!status, ("addTransformData(): failed to get weights associated with joint [%s]\n", influenceDagPath.partialPathName().asChar()));

	//-- add transform weighting information
	// validate # weights returned
	const unsigned weightCount = weights.length();
	MESSENGER_REJECT(!status, ("failed to get weights length\n"));
	MESSENGER_REJECT(static_cast<int>(weightCount) != m_meshPositionCount, ("unexpected: skin cluster joint did not return weights for all positions [%u/%d]\n", weightCount, m_meshPositionCount));

	// reserve more transformWeightData
	m_transformWeightData->m_container.reserve(m_transformWeightData->m_container.size() + static_cast<size_t>(weightCount));

	// add non-zero transform weight data
	const real epsilon = CONST_REAL(0.005);

	int weightedPositionCount = 0;

	for (unsigned weightIndex = 0; weightIndex < weightCount; ++weightIndex)
	{
		const real weight        = static_cast<real>(weights[weightIndex]);
		const int  positionIndex = static_cast<int>(weightIndex);  // yes, in this context the position index is the weight index

		// only add non-zero weights
		if (!WithinEpsilonInclusive(CONST_REAL(0), weight, epsilon))
		{
			// add transform weight info
			m_transformWeightData->m_container.push_back(TransformWeightData(positionIndex, transformIndex, weight));
			++weightedPositionCount;
		}
	}

#if 0
	MESSENGER_LOG(("-- [%d out of %d affected]\n", weightedPositionCount, weightCount));
#endif

	if ((weightedPositionCount < 1) && affectorIsNew)
	{
		//-- remove this transform, it does not affect the mesh
		// @todo add to a flag: too much spam for typical case but is useful info.
		// MESSENGER_LOG(("-- removing, does not affect the mesh\n"));
		
		const bool raiResult = removeAffectorInfo(transformIndex);
		MESSENGER_REJECT(!raiResult, ("removeAffectorInfo() failed\n"));
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::addSkinClusterAffector(const MObject &skinCluster)
{
	NOT_NULL(m_meshObject);

	MESSENGER_INDENT;
	MESSENGER_REJECT(m_singleAffectorAdded, ("already added single affector, nothing else should be added\n"));

	MStatus  status;

	MFnSkinCluster fnSkinCluster(skinCluster, &status);
	MESSENGER_REJECT(!status, ("addSkinCluster(): object not a skin cluster deformer\n"));

	//-- check if skin cluster deforms our geometry
	// note: in Maya, we select/work with the output of the skin cluster deformer.
	// Maya hides the input node from normal viewing, but you'll see it in the
	// HyperGraph.
	MObjectArray  outputGeometryArray;

	status = fnSkinCluster.getOutputGeometry(outputGeometryArray);
	MESSENGER_REJECT(!status, ("failed to get skin cluster deformer output geometry\n"));

	bool      deformsTargetMesh  = false;
	const unsigned geometryCount = outputGeometryArray.length();

	for (unsigned i = 0; i < geometryCount; ++i)
	{
		const MObject &geometryObject = outputGeometryArray[i];
		if (geometryObject == *m_meshObject)
		{
			// this skin cluster deforms our mesh
			deformsTargetMesh = true;
			break;
		}
	}
	MESSENGER_REJECT(!deformsTargetMesh, ("addSkinCluster: skin cluster [%s] does not deform target mesh\n", fnSkinCluster.name().asChar()));

	//-- add skin cluster joints
	MDagPathArray   influenceObjects;

	const unsigned influenceObjectCount = fnSkinCluster.influenceObjects(influenceObjects, &status);
	MESSENGER_REJECT(!status, ("addSkinCluster(): failed to retrieve influence objects\n"));

	for (unsigned transformIndex = 0; transformIndex < influenceObjectCount; ++transformIndex)
	{
		const MDagPath &influenceDagPath = influenceObjects[transformIndex];

		const bool success = addTransformData(fnSkinCluster, influenceDagPath,transformIndex);
		MESSENGER_REJECT(!success, ("addSkinCluster(): failed to add transform data for influence object\n"));
	}

	//-- success
	m_isDirty = true;
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::addSingleAffector(const MDagPath &affectorDagPath)
{
	NOT_NULL(m_jointData);
	NOT_NULL(m_transformWeightData);

	MESSENGER_INDENT;
	MESSENGER_REJECT(m_singleAffectorAdded, ("already added single affector, nothing else should be added\n"));
	MESSENGER_REJECT(m_jointData->m_container.size(), ("already added other joint data, adding single affector doesn't make sense\n"));

	//-- record affector data
	int   transformIndex = -1;
	bool  affectorIsNew  = false;

	const bool aaiResult = addAffectorInfo(affectorDagPath, &transformIndex, affectorIsNew);
	MESSENGER_REJECT(!aaiResult, ("addAffectorInfo() failed for [%s]\n", affectorDagPath.partialPathName().asChar()));

	//-- for each vertex in our mesh, add single weighting
	m_transformWeightData->m_container.reserve(static_cast<size_t>(m_meshPositionCount));

	// add non-zero transform weight data
	const real weight = CONST_REAL(1.0);

	for (int positionIndex = 0; positionIndex < m_meshPositionCount; ++positionIndex)
	{
		m_transformWeightData->m_container.push_back(TransformWeightData(positionIndex, transformIndex, weight));
	}

	//-- no errors
	m_singleAffectorAdded = true;
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::processAffectors()
{
	//-- sort transform weight data
	// this will sort the vertex data first by vertex index, then by affecting joint index.
	// both fields are sorted in ascending numerical order.
	std::sort(m_transformWeightData->m_container.begin(), m_transformWeightData->m_container.end(), TransformWeightData::Less());

	// fill in vertex data container
	int         currentPositionIndex = -1;
	VertexData *currentVertexData    = 0;

	const size_t transformWeightDataCount = m_transformWeightData->m_container.size();
	for (size_t transformWeightDataIndex = 0; transformWeightDataIndex < transformWeightDataCount; ++transformWeightDataIndex)
	{
		const TransformWeightData &transformWeightData = m_transformWeightData->m_container[transformWeightDataIndex];

		if (transformWeightData.m_vertexIndex != currentPositionIndex)
		{
			// we're transitioning to transforms covering a new vertex position
			currentPositionIndex = transformWeightData.m_vertexIndex;
			currentVertexData    = &( m_vertexData->m_container[static_cast<size_t>(transformWeightData.m_vertexIndex)] );
			currentVertexData->m_firstIndex = static_cast<int>(transformWeightDataIndex);
		}

		NOT_NULL(currentVertexData);
		++(currentVertexData->m_transformCount);
	}

	//-- success
	m_isDirty = false;
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getAffectingTransformCount(int *transformCount) const
{
	NOT_NULL(transformCount);
	NOT_NULL(m_jointData);

	*transformCount = static_cast<int>(m_jointData->m_container.size());
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getAffectingTransform(int transformIndex, MString *transformName, MDagPath *affectorDagPath) const
{
	NOT_NULL(transformName);
	NOT_NULL(affectorDagPath);
	NOT_NULL(m_jointData);

	MESSENGER_REJECT((transformIndex < 0) || (transformIndex >= static_cast<int>(m_jointData->m_container.size())),
		("getTransform(): transformIndex [%d] out of valid range [0..%u]\n", transformIndex, m_jointData->m_container.size()));

	const JointData &jointData = m_jointData->m_container[static_cast<size_t>(transformIndex)];
	*transformName             = jointData.m_name;
	*affectorDagPath           = jointData.m_dagPath;

	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getAffectedPositionCount(int *vertexCount) const
{
	NOT_NULL(vertexCount);

	*vertexCount = m_meshPositionCount;
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getVertexData(int vertexIndex, const VertexData **vertexData) const
{
	NOT_NULL(vertexData);
	NOT_NULL(m_vertexData);

	MESSENGER_REJECT(m_isDirty, ("getVertexData(): MayaMeshWeighting currently dirty, needs processAffectors() call\n"));
	MESSENGER_REJECT((vertexIndex < 0) || (vertexIndex >= static_cast<int>(m_vertexData->m_container.size())), 
		("getVertexAffectorCount: vertexIndex arg [%d] out of valid range [0..%u]\n", vertexIndex, m_vertexData->m_container.size()));

	*vertexData = &(m_vertexData->m_container[static_cast<size_t>(vertexIndex)]);
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getVertexDataAffectorCount(const VertexData *vertexData, int *transformCount) const
{
	NOT_NULL(vertexData);
	NOT_NULL(transformCount);

	*transformCount = vertexData->m_transformCount;
	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::getVertexDataTransformWeightData(const VertexData *vertexData, int affectorIndex, int *transformIndex, real *weight) const
{
	NOT_NULL(m_transformWeightData);
	NOT_NULL(vertexData);
	NOT_NULL(transformIndex);
	NOT_NULL(weight);

	const size_t transformWeightDataIndex = static_cast<size_t>(vertexData->m_firstIndex + affectorIndex);
	MESSENGER_REJECT(transformWeightDataIndex >= m_transformWeightData->m_container.size(), ("affector index out of range [%u/%u]\n", transformWeightDataIndex, m_transformWeightData->m_container.size()));

	const TransformWeightData &transformWeightData = m_transformWeightData->m_container[transformWeightDataIndex];

	*transformIndex = transformWeightData.m_transformIndex;
	*weight         = transformWeightData.m_transformWeight;

	return true;
}

// ----------------------------------------------------------------------

bool MayaMeshWeighting::debugDump() const
{
	MESSENGER_INDENT;

	NOT_NULL(m_vertexData);
	NOT_NULL(m_transformWeightData);
	NOT_NULL(m_jointData);

	MESSENGER_REJECT(m_isDirty, ("getVertexData(): MayaMeshWeighting currently dirty, needs processAffectors() call\n"));

	MESSENGER_LOG(("MayaMeshWeighting::debugDump(): %d positions\n", m_meshPositionCount));

	const real epsilon = CONST_REAL(0.01);

	for (size_t positionIndex = 0; positionIndex < static_cast<size_t>(m_meshPositionCount); ++positionIndex)
	{
		const VertexData &vertexData = m_vertexData->m_container[positionIndex];
		real              weightSum  = 0;

		for (int transformIndex = 0; transformIndex < vertexData.m_transformCount; ++transformIndex)
		{
			const size_t               transformWeightDataIndex = static_cast<size_t>(vertexData.m_firstIndex + transformIndex);
			const TransformWeightData &transformWeightData      = m_transformWeightData->m_container[transformWeightDataIndex];
			const MString             &transformName            = m_jointData->m_container[static_cast<size_t>(transformWeightData.m_transformIndex)].m_name;

			MESSENGER_LOG(("p(%u) w(%.3f) t(%s)\n", positionIndex, transformWeightData.m_transformWeight, transformName.asChar()));

			weightSum += transformWeightData.m_transformWeight;
		}

		// check for weightSum to be one
		if (!WithinEpsilonInclusive(CONST_REAL(1), weightSum, epsilon))
			MESSENGER_LOG_WARNING(("WARNING: position weighting does not sum to one [%.4f]\n", weightSum));
	}

	return true;	
}

// ======================================================================
