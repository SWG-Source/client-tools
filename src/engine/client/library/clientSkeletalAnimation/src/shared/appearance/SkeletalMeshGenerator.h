// ======================================================================
//
// SkeletalMeshGenerator.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletalMeshGenerator_H
#define INCLUDED_SkeletalMeshGenerator_H

// ======================================================================

#include "clientSkeletalAnimation/MeshGenerator.h"

class CrcLowerString;
class CustomizationData;
class Iff;
class MeshConstructionHelper;
class Quaternion;
class MemoryBlockManager;
class SkeletalMeshGeneratorTemplate;
class TextureRenderer;
class Vector;

// ======================================================================
/**
 * Generate mesh data that supports skinning and construction-time morphing.
 */

class SkeletalMeshGenerator: public MeshGenerator
{
friend class SkeletalMeshGeneratorTemplate;

public:

	static void install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

public:

	virtual int            getOcclusionLayer() const;
	virtual void           applySkeletonModifications(Skeleton &skeleton) const;
	virtual void           addShaderPrimitives(Appearance &appearance, int lodIndex, CustomizationData *customizationData, const TransformNameMap &transformNameMap, const OcclusionZoneSet &zonesCurrentlyOccluded, OcclusionZoneSet &zonesOccludedByThisLayer, ShaderPrimitiveVector &shaderPrimitives) const;
	virtual void           setCustomizationData(CustomizationData *customizationData);

	virtual Appearance    *createAppearance() const;

	virtual void           addCustomizationVariables(CustomizationData &customizationData) const;

	virtual bool           isReadyForUse() const;

	int                    getReferencedSkeletonTemplateCount() const;
	const  CrcLowerString &getReferencedSkeletonTemplateName(int index) const;

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	static void remove();

	static void handleCustomizationModificationStatic(const CustomizationData &customizationData, const void *context);

private:

	SkeletalMeshGenerator(const SkeletalMeshGeneratorTemplate *meshGeneratorTemplate);
	virtual ~SkeletalMeshGenerator();

	void                                 create();
	const SkeletalMeshGeneratorTemplate &getSkeletalMeshGeneratorTemplate() const;

	void                                 handleCustomizationModification(const CustomizationData &customizationData);

	// disabled
	SkeletalMeshGenerator();
	SkeletalMeshGenerator(const SkeletalMeshGenerator&);
	SkeletalMeshGenerator &operator =(const SkeletalMeshGenerator&);

private:

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;

private:

	CustomizationData *m_customizationData;
	IntVector         *m_blendValues;

};

// ======================================================================

#endif
