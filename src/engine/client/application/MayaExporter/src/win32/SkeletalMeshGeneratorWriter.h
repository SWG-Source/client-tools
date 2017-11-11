// ======================================================================
//
// SkeletalMeshGeneratorWriter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef SKELETAL_MESH_GENERATOR_WRITER_H
#define SKELETAL_MESH_GENERATOR_WRITER_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class  CrcLowerString;
class  Iff;
class  LessPointerComparator;
class  PackedArgb;
class  Quaternion;
class  Vector;

namespace boost
{
	template <typename T>
	class shared_ptr;
}

// ======================================================================

class SkeletalMeshGeneratorWriter
{
public:

	struct BlendVector;
	struct BlendTarget;
	struct PerShaderData;

public:

	SkeletalMeshGeneratorWriter();
	~SkeletalMeshGeneratorWriter();

	void write(Iff *iff);

	void addReferencedSkeletonTemplateName(const char *name);
	int  addTransformName(const char *name);

	void           addPosition(const Vector &position);
	void           addPositionWeight(int positionIndex, int transformIndex, real weight);
	int            getPositionCount() const;

	void           addNormal(const Vector &normal);
	int            getNormalCount() const;

	void           addDot3Value(float x, float y, float z, float w);

	int            addStaticHardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation);
	int            addDynamicHardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation);

	BlendTarget   *addBlendTarget(const char *variableName);
	BlendTarget   *findBlendTarget(const char *variableName) const;
	void           debugDumpBlendTargets() const;

	void           addBlendTargetPosition(BlendTarget *blendTarget, int positionIndex, const Vector &deltaVector) const;
	void           addBlendTargetNormal(BlendTarget *blendTarget, int normalIndex, const Vector &deltaVector) const;
	void           addBlendTargetDot3Value(BlendTarget *blendTarget, int dot3VectorIndex, float deltaX, float deltaY, float deltaZ, float deltaW) const;
	void           addBlendTargetHardpointTarget(BlendTarget *blendTarget, int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation) const;

	PerShaderData &addShaderTemplateByName(const char *name);

	void           setShaderTextureCoordinateSetCount(PerShaderData &perShaderData, int count) const;
	void           setShaderTextureCoordinateSetDimensionality(PerShaderData &perShaderData, int setIndex, int dimensionality) const;

	int            addShaderVertex(PerShaderData &perShaderData, int positionIndex) const;
	void           setShaderVertexNormal(PerShaderData &perShaderData, int normalIndex) const;
	void           setShaderVertexDot3ValueIndex(PerShaderData &perShaderData, int dot3VectorIndex) const;
	void           setShaderVertexDiffuseColor(PerShaderData &perShaderData, const PackedArgb &color) const;
	void           setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0) const;
	void           setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0, real tc1) const;
	void           setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0, real tc1, real tc2) const;

	void           beginIndexedTriList(PerShaderData &perShaderData, int triCountHint = 256);
	void           addIndexedTriListTri(int index0, int index1, int index2);
	void           endIndexedTriList();

	void           beginOccludableIndexedTriList(PerShaderData &perShaderData, int triCountHint = 256);
	void           addOccludableIndexedTriListTri(int occlusionZoneCombinationIndex, int index0, int index1, int index2);
	void           endOccludableIndexedTriList();

	void           addTextureRenderer(const PerShaderData &perShaderData, const char *textureRendererTemplateName, Tag shaderTextureTag);

	void           addOcclusionZone(const char *name);

	void           setFullyOccludedZoneCombination(const stdvector<int>::fwd &combination);
	void           addOcclusionZoneCombination(const stdvector<int>::fwd &combination);

	void           setZonesThisOccludes(const stdvector<int>::fwd &zonesThisOccludes);

	void           setLayer(int meshGeneratorLayer);

private:

	class DrawPrimitive;
	class Hardpoint;
	class IndexedTriListPrimitive;
	class OccludableIndexedTriListPrimitive;

	struct BlendTargetContainer;
	struct PerShaderDataContainer;
	struct TextureRendererData;
	struct TransformWeight;
	struct TransformWeightContainer;
	struct TransformWeightHeaderContainer;
	struct VectorContainer;

	typedef stdvector<boost::shared_ptr<CrcLowerString> >::fwd                         CrcLowerStringVector;
	typedef stdvector<float>::fwd                                                      FloatVector;
	typedef stdvector<Hardpoint*>::fwd                                                 HardpointVector;
	typedef stdvector<int>::fwd                                                        IntVector;
	typedef stdvector<boost::shared_ptr<IntVector> >::fwd                              SharedIntVectorVector;
	typedef stdmap<CrcLowerString*, TextureRendererData*, LessPointerComparator>::fwd  TextureRendererContainer;

private:

	static int   addHardpoint(HardpointVector &hardpoints, const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation);
	static void  writeHardpoints(Iff &iff, const HardpointVector &hardpoints);

private:

	int                                  m_maxTransformsPerVertex;
	int                                  m_maxTransformsPerShader;

	CrcLowerStringVector                *m_skeletonTemplateNames;
	CrcLowerStringVector                *m_transformNames;

	VectorContainer                     *m_positions;
	TransformWeightHeaderContainer      *m_transformWeightHeaders;
	TransformWeightContainer            *m_transformWeightData;

	VectorContainer                     *m_normals;
	FloatVector                         *m_dot3Values;

	BlendTargetContainer                *m_blendTargets;

	PerShaderDataContainer              *m_perShaderData;
	TextureRendererContainer            *m_textureRenderers;

	IndexedTriListPrimitive             *m_currentIndexedTriList;
	OccludableIndexedTriListPrimitive   *m_currentOccludableIndexedTriList;

	CrcLowerStringVector                *m_occlusionZoneNames;

	IntVector                           *m_fullyOccludedZoneCombination;
	SharedIntVectorVector               *m_occlusionZoneCombinations;

	IntVector                           *m_zonesThisOccludes;

	int                                  m_occlusionLayer;

	HardpointVector                     *m_staticHardpoints;
	HardpointVector                     *m_dynamicHardpoints;

private:

	// disabled
	SkeletalMeshGeneratorWriter(const SkeletalMeshGeneratorWriter&);
	SkeletalMeshGeneratorWriter &operator =(const SkeletalMeshGeneratorWriter&);

};

// ======================================================================

#endif
