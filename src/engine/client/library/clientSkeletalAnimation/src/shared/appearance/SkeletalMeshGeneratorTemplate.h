// ======================================================================
//
// SkeletalMeshGeneratorTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletalMeshGeneratorTemplate_H
#define INCLUDED_SkeletalMeshGeneratorTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

#include <vector>

class Appearance;
class CrcLowerString;
class CustomizationData;
class Iff;
class IndexedTriangleList;
class MemoryBlockManager;
class MeshConstructionHelper;
class OcclusionZoneSet;
class Quaternion;
class ShaderPrimitive;
class SkeletalMeshGenerator;
class Skeleton;
class TextureRenderer;
class TextureRendererTemplate;
class TransformNameMap;
class Vector;

// ======================================================================
/**
 *
 * Generate mesh data that supports skinning and construction-time morphing.
 */

class SkeletalMeshGeneratorTemplate: public BasicMeshGeneratorTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	friend class SkeletalMeshGenerator;

public:

	struct BlendVector;
	class  PerShaderData;

	typedef stdvector<int>::fwd                IntVector;
	typedef stdvector<ShaderPrimitive *>::fwd  ShaderPrimitiveVector;

public:

	static void install();

public:

	static const int ms_blendTargetNameSize;

public:

	virtual MeshGenerator         *createMeshGenerator() const;
	virtual void                   preloadAssets() const;
	virtual void                   garbageCollect() const;
	virtual bool                   hasOnlyNonCollidableShaderTemplates() const;

	int                            getBlendVariableCount() const;
	const std::string             &getBlendVariableName(int index) const;

	int                            getOcclusionLayer() const;
	void                           applySkeletonModifications(const IntVector *blendValues, Skeleton &skeleton) const;
	void                           addShaderPrimitives(Appearance &appearance, int lodIndex, CustomizationData *customizationData, const IntVector *blendValues, const TransformNameMap &transformNameMap, const OcclusionZoneSet &zonesCurrentlyOccluded, OcclusionZoneSet &zonesOccludedByThisLayer, ShaderPrimitiveVector &shaderPrimitives) const;

	Appearance                    *createAppearance() const;

	int                            getReferencedSkeletonTemplateCount() const;
	const  CrcLowerString         &getReferencedSkeletonTemplateName(int index) const;

	int                            getTextureRendererTemplateCount() const;
	const TextureRendererTemplate *fetchTextureRendererTemplate(int index) const;

	void                           addCustomizationVariables(CustomizationData &customizationData) const;

	bool                           isLoaded() const;

	void                           fillIndexedTriangleList(IndexedTriangleList &list) const;

	class  BlendTarget;
	class  Hardpoint;

private:

	struct Dot3Vector;
	struct TextureRendererEntry;
	struct TextureRendererHeader;
	struct TransformWeightData;

	typedef stdvector<Dot3Vector>::fwd              Dot3VectorVector;
	typedef stdvector<TextureRenderer*>::fwd        TextureRendererVector;
	typedef stdvector<TextureRendererEntry>::fwd    TREntryContainer;
	typedef stdvector<TextureRendererHeader*>::fwd  TRHeaderContainer;

	typedef stdvector<IntVector>::fwd               IntVectorVector;

	typedef stdvector<CrcLowerString>::fwd          CrcLowerStringVector;
	typedef stdvector<Vector>::fwd                  VectorVector;

	typedef stdvector<TransformWeightData>::fwd     TransformWeightDataVector;

	typedef stdvector<PerShaderData*>::fwd          PerShaderDataVector;
	typedef stdvector<BlendTarget*>::fwd            BlendTargetVector;
	typedef stdvector<Hardpoint*>::fwd              HardpointVector;

	typedef stdvector<Quaternion>::fwd              QuaternionVector;
	typedef stdvector<SkeletalMeshGenerator*>::fwd  MeshGeneratorVector;

private:

	static void                   remove();
	static MeshGeneratorTemplate *create(Iff *iff, CrcString const &name);
	static void                   asynchronousLoadCallback(void *data);

	static void                   loadHardpoints(Iff &iff, HardpointVector &hardpoints);

private:

	SkeletalMeshGeneratorTemplate(Iff *iff, CrcString const &name);
	virtual ~SkeletalMeshGeneratorTemplate();

	void load(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);
	void load_0004(Iff &iff);

	void asynchronousLoadCallback();

	void fillMeshConstructionHelper(MeshConstructionHelper &meshConstructionHelper, const IntVector *blendValues, const TransformNameMap &transformNameMap, const OcclusionZoneSet &zonesCurrentlyOccluded, OcclusionZoneSet &zonesOccludedByThisLayer) const;
	const Dot3VectorVector *prepareDot3Vectors(const IntVector *blendValues) const;

	void removeAsynchronouslyLoadedMeshGenerator(SkeletalMeshGenerator *meshGenerator) const;


	// Disabled.
	SkeletalMeshGeneratorTemplate();
	SkeletalMeshGeneratorTemplate(const SkeletalMeshGeneratorTemplate&);
	SkeletalMeshGeneratorTemplate &operator =(const SkeletalMeshGeneratorTemplate&);

private:

	static bool                              ms_installed;
	static MeshConstructionHelper           *ms_meshConstructionHelper;

	static IntVector                         ms_localToOutputTransformIndices;

	static VectorVector                      ms_dynamicHardpointPositions;
	static QuaternionVector                  ms_dynamicHardpointRotations;

	static Dot3VectorVector                 *ms_blendedDot3Vectors;

private:

	bool                         m_isLoaded;
	mutable bool                 m_asynchronousLoadInProgress;
	mutable MeshGeneratorVector *m_uninitializedMeshGenerators;

	int                        m_maxTransformsPerVertex;
	int                        m_maxTransformsPerShader;

	CrcLowerStringVector       m_skeletonTemplateNames;
	CrcLowerStringVector       m_transformNames;

	VectorVector               m_positions;
	IntVector                  m_transformWeightCounts;

	TransformWeightDataVector  m_transformWeightData;

	VectorVector               m_normals;

	BlendTargetVector         *m_blendTargets;

	PerShaderDataVector        m_perShaderData;

	TRHeaderContainer         *m_textureRendererHeaders;
	TREntryContainer          *m_textureRendererEntries;

	IntVector                 *m_fullyOccludedZoneCombination;
	IntVectorVector            m_occlusionZoneCombinations;

	IntVector                  m_zonesThisOccludes;

	int                        m_occlusionLayer;

	HardpointVector           *m_staticHardpoints;
	HardpointVector           *m_dynamicHardpoints;

	Dot3VectorVector          *m_dot3Vectors;

};

// ======================================================================

#endif
