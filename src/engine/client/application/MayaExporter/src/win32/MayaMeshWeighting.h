// ======================================================================
//
// MayaMeshWeighting.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef MAYA_MESH_WEIGHTING_H
#define MAYA_MESH_WEIGHTING_H

// ======================================================================
// forward declarations

class MDagPath;
class Messenger;
class MFnSkinCluster;
class MObject;
class MStatus;
class MString;

// ======================================================================

class MayaMeshWeighting
{
public:

	struct VertexData;

public:

	static void install(Messenger *newMessenger);
	static void remove();

public:

	explicit MayaMeshWeighting(const MDagPath &meshDagPath, MStatus *status);
	~MayaMeshWeighting();

	bool addSkinClusterAffector(const MObject &skinCluster);
	bool addSingleAffector(const MDagPath &affectorDagPath);
	bool processAffectors();

	bool getAffectingTransformCount(int *transformCount) const;
	bool getAffectingTransform(int transformIndex, MString *transformName, MDagPath *affectorDagPath) const;

	bool getAffectedPositionCount(int *vertexCount) const;
	bool getVertexData(int vertexIndex, const VertexData **vertexData) const;
	bool getVertexDataAffectorCount(const VertexData *vertexData, int *transformCount) const;
	bool getVertexDataTransformWeightData(const VertexData *vertexData, int affectorIndex, int *transformIndex, real *weight) const;

	bool debugDump() const;

private:

	bool addAffectorInfo(const MDagPath &affectorDagPath, int *transformIndex, bool &isNew);
	bool removeAffectorInfo(int transformIndex);
	bool addTransformData(MFnSkinCluster &fnSkinCluster, const MDagPath &influenceDagPath,const unsigned influenceObjectIndex);

private:

	struct JointData;
	struct JointDataContainer;

	struct TransformWeightData;
	struct TransformWeightDataContainer;

	struct VertexDataContainer;

private:

	static Messenger *messenger;
	static bool       ms_installed;

private:

	bool                          m_isValid;
	bool                          m_isDirty;

	MObject                      *m_meshObject;
	const MDagPath               &m_meshDagPath;
	MObject                      *m_meshCompleteComponent;
	int                           m_meshPositionCount;

	JointDataContainer           *m_jointData;
	TransformWeightDataContainer *m_transformWeightData;
	VertexDataContainer          *m_vertexData;

	bool                          m_singleAffectorAdded;

private:

	MayaMeshWeighting();
	MayaMeshWeighting(const MayaMeshWeighting&);
	MayaMeshWeighting &operator =(const MayaMeshWeighting&);

};

// ======================================================================

#endif
