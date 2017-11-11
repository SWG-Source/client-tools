// ======================================================================
//
// ExportSkeletalMeshGenerator.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef EXPORT_SKELETAL_MESH_GENERATOR_H
#define EXPORT_SKELETAL_MESH_GENERATOR_H

// ======================================================================

#include "MayaPerPixelLighting.h"
#include "MayaUtility.h"

#include "maya/MPxCommand.h"

class CrcLowerString;
class LessPointerComparator;
class MayaMeshWeighting;
class Messenger;
class OccludedFaceMapGenerator;
class SkeletalMeshGeneratorWriter;
class Transform;

namespace boost
{
	template<class T>
	class shared_ptr;
}

class MArgList;
class MDagPath;
class MFloatPointArray;
class MFloatVectorArray;
class MFnMesh;
class MIntArray;
class MObjectArray;
class MString;
class MStringArray;

// ======================================================================

class ExportSkeletalMeshGenerator: public MPxCommand
{
public:

	typedef stdvector<MObject>::fwd  MObjectVector;

public:

	static void install(Messenger *newMessenger);
	static void remove(void);

	static void *creator(void);

public:

	ExportSkeletalMeshGenerator();
	~ExportSkeletalMeshGenerator();

	MStatus doIt(const MArgList &args);

private:

	struct BlendShapeWeight;
	struct HardpointInfo;

	typedef stdmap<const CrcLowerString*, boost::shared_ptr<HardpointInfo>, LessPointerComparator>::fwd  HardpointInfoMap;
	typedef stdvector<Vector>::fwd                                                                       VectorVector;

private:

	static bool checkShaderForOccludableFaces(const OccludedFaceMapGenerator &occludedFaceMapGenerator, const MIntArray &polyShaderAssignment, int shaderIndex, bool &returnHasOccludableFaces);
	static bool getZoneNamesMeshOccludes(const MDagPath &meshParentDagPath, stdvector<boost::shared_ptr<CrcLowerString> >::fwd &zoneNamesThisOccludes);
	static bool getMeshLayer(const MDagPath &meshParentDagPath, int &layer);
	static bool exportOcclusionData(SkeletalMeshGeneratorWriter &writer, const OccludedFaceMapGenerator &occludedFaceMapGenerator, const MDagPath &meshParentDagPath);

	static bool addMeshPositions(const MFloatPointArray &mayaPositions, const MayaMeshWeighting &meshWeighting, SkeletalMeshGeneratorWriter *writer);
	static bool addMeshNormals(const MFloatVectorArray &mayaNormals, SkeletalMeshGeneratorWriter *writer);
	static bool addMeshDot3Values(const MayaPerPixelLighting::FloatVector &dot3Values, SkeletalMeshGeneratorWriter *writer);

	static void removeShapeSuffix(MString &name);

	static bool addTransformsToWriter(const MayaMeshWeighting &meshWeighting, SkeletalMeshGeneratorWriter *writer, MStringArray *skeletonTemplateNodeNames);
	static bool captureBlendDeltas(const MString &gameBlendTargetName, const MFloatPointArray &mayaUnblendedPositions, const MFloatPointArray &mayaBlendedPositions, const MFloatVectorArray &mayaUnblendedNormals, const MFloatVectorArray &mayaBlendedNormals, const MayaPerPixelLighting::Dot3DifferenceVector &dot3DifferenceVector, SkeletalMeshGeneratorWriter *writer);

	static bool setSkeletonsToBindPose(const MStringArray &skeletonTemplateNodeNames, int alternateBindPoseFrameNumber);

	static bool getFaceSets(const MDagPath &meshDagPath, MObjectVector &faceSets);

	static bool collectHardpoints(const MString &hardpointPrefix, const MString &meshName, HardpointInfoMap &hardpointInfoMap, int lodIndex);
	static bool extractHardpointTransform(const MObject &transformObject, Vector &translation, Quaternion &rotation, std::string *parentTransformName);
	static bool processHardpointInfo(const HardpointInfoMap &hardpointInfoMap, SkeletalMeshGeneratorWriter &writer, bool printDebugInfo, int lodIndex);
	static bool exportHardpoints(const char *meshName, SkeletalMeshGeneratorWriter &writer, int lodIndex);

	static bool processArguments(const MArgList &args, 
		MString *authorName, 
		MString *generatorOutputDirectory, 
		MDagPath *targetNode, 
		bool &ignoreShaders, 
		bool &ignoreBlendTargets, 
		int &bindPoseFrameNumber, 
		bool &ignoreTextures, 
		bool &interactive, 
		bool &commitToSourceControl, 
		bool &createNewChangelist, 
		bool &independentExport, 
		bool &lock, 
		bool &unlock, 
		bool &showViewerAfterExport,
		MString &branch
	);

	static bool getShapePathFromParent(const MDagPath &parentPath, MDagPath &shapePath);
	static bool getLodPath(const MDagPath &lodGroupPath, int lodIndex, MDagPath &lodPath);

	static bool extractShaderNames(bool useDefaultShader, const MObject &mayaShadingEngineObject, std::string &shaderShortName, std::string &shaderTemplateReferenceName);
	static bool arePolysAssignedToShader(int mayaShaderIndex, const MIntArray &polyShaderAssignment);

	static bool isZeroAreaTriangle(const Vector &position1, const Vector &position2, const Vector &position3);

	static bool getGamePositions(const MFnMesh &mesh, VectorVector &gamePositions);

private:

	bool addPerShaderData(
			const MFnMesh                                       &fnMesh, 
			SkeletalMeshGeneratorWriter                         *writer, 
			const MObject                                       &mayaShadingEngineObject, 
			const MIntArray                                     &polyShaderAssignment, 
			int                                                  mayaShaderIndex, 
			const MStringArray                                  &mayaUvSetNames, 
			MayaUtility::TextureSet                             *referencedTextures, 
			const OccludedFaceMapGenerator                      &occludedFaceMapGenerator, 
			const MayaPerPixelLighting::ShaderRequestInfoVector &dot3ShaderRequestInfoVector, 
			const MayaPerPixelLighting::Dot3KeyMap              &dot3KeyMap,
			const VectorVector                                  &meshGamePositions
			);

	bool handlePositionsAndNormals(
			const MDagPath                                      &meshDagPath, 
			const MayaMeshWeighting                             &meshWeighting, 
			const MayaPerPixelLighting::ShaderRequestInfoVector &dot3ShaderRequestInfoVector,
			const MayaPerPixelLighting::Dot3KeyMap              &dot3KeyMap,
			int                                                  dot3ValueCount,
			SkeletalMeshGeneratorWriter                         *writer
			) const;

	bool performSingleExport(int bindPoseFrameNumber, const MDagPath &targetDagPath);
	bool performLodExport(int bindPoseFrameNumber, const MDagPath &targetDagPath);

	bool exportMeshGeneratorTemplate(int bindPoseFrameNumber, const MDagPath &shapeDagPath, const char *meshName, SkeletalMeshGeneratorWriter &writer, int lodIndex);

private:

	static const int   ms_initialIffSize;
	static const int   ms_skeletonTemplateNameComponentIndex;
	static const int   ms_maxSupportedPolygonVertexCount;
	static const real  ms_blendPositionEpsilon;
	static const real  ms_blendNormalEpsilon;

	static bool        ms_installed;

private:

	int   m_meshTriangleCount;
	bool  m_ignoreShaders;
	bool  m_ignoreBlendTargets;
	bool  m_ignoreTextures;

private:

	// disabled
	ExportSkeletalMeshGenerator(const ExportSkeletalMeshGenerator&);
	ExportSkeletalMeshGenerator &operator =(const ExportSkeletalMeshGenerator&);

};

// ======================================================================

#endif
