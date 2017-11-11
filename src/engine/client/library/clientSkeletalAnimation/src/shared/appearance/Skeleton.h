// ======================================================================
//
// Skeleton.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_Skeleton_H
#define INCLUDED_Skeleton_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class BoxExtent;
class CrcString;
class Object;
class SkeletalAppearance2;
class BasicSkeletonTemplate;
class SkeletonTransformNameMap;
class MemoryBlockManager;
class Transform;
class TransformAnimationResolver;
class TransformModifier;
class TransformNameMap;
class Vector;
class PoseModelTransform;

// ======================================================================

class Skeleton
{
friend class BasicSkeletonTemplate;

	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	enum TransformType
	{
		TT_joint,
		TT_hardpoint
	};

	class Hardpoint;

	typedef stdvector<Transform>::fwd  TransformVector;

public:

	static void             install();

public:

	~Skeleton();

	void                    beginSkeletonModification();
	void                    endSkeletonModification();

	// these functions must be called between {begin/end}SkeletonModification()
	int                     attachSkeletonSegment(const BasicSkeletonTemplate &skeletonTemplate, CrcString const &attachmentTransformName);
	void                    removeSkeletonSegment(int skeletonSegmentId);
	void                    removeAllSegments();

	void                    addHardpoint(CrcString const &hardpointName, CrcString const &parentName, const Transform &hardpointToParent);
	void                    removeAllHardpoints();

	void                    findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const;
	const Transform *       findTransform     (CrcString const &name) const;
	int                     getTransformIndex(CrcString const &name) const;
	int                     getParentTransformIndex(int childIndex) const;

	CrcString const        &getTransformName(int transformIndex) const;
	const TransformNameMap &getTransformNameMap() const;

	int                       getTransformCount() const;
	const Transform          *getJointToRootTransformArray() const;
	const PoseModelTransform *getBindPoseModelToRootTransforms() const;

	void                    addShaderPrimitives(const SkeletalAppearance2 &appearance) const;
	void                    drawNow(const Transform &skeletonToWorld, const Vector &scale) const;

	const BoxExtent        *getExtent() const;

	const TransformAnimationResolver &getAnimationResolver() const;
	TransformAnimationResolver       &getAnimationResolver();

	void                              setScale(float scale);
	float                             getScale() const;

	void                              clearAllTransformModifiers();
	void                              attachTransformModifier(int transformIndex, TransformModifier *transformModifier);

	SkeletalAppearance2              &getSkeletalAppearance();
	SkeletalAppearance2 const        &getSkeletalAppearance() const;


public:
	class LocalShaderPrimitive;
	class Segment;
	friend class Segment;

private:

	typedef stdmap<int, TransformModifier *>::fwd  TransformModifierMap;
	typedef stdvector<Segment*>::fwd               SegmentVector;

private:

	static void             remove();

private:

	Skeleton(const BasicSkeletonTemplate &rootSkeletonTemplate, TransformAnimationResolver &animationResolver);

	int                     findSegmentIndex(int globalTransformIndex) const;
	void                    findSegmentLocalTransformIndex(CrcString const &transformName, int &segmentIndex, int &segmentLocalTransformIndex, bool &foundIt) const;

	void                    rebuildLinkedSkeletonDefinition();

	void                    drawJointFramesNow(const Transform &skeletonToWorld, const Vector &scale) const;
	void                    calculateJointToRootTransforms() const;
	void                    calculateBindPoseModelToRootTransforms() const;
	void                    calculateExtent() const;

	void                    allocateTransformArrays(int transformCount);

private:

	TransformAnimationResolver   &m_animationResolver;
	int                           m_transformCount;
	SegmentVector                *m_skeletonSegments;
	uint8                        *m_jointToRootTransformBackingStore;
	mutable Transform            *m_jointToRootTransforms;
	uint8                        *m_bindPoseModelToRootTransformBackingStore;
	mutable PoseModelTransform   *m_bindPoseModelToRootTransforms;

	mutable int                   m_frameLastJointToRootCalculate;
	mutable int                   m_frameLastBindPoseModelToRootCalculate;
	mutable int                   m_frameLastExtentCalculate;

	SkeletonTransformNameMap     *m_transformNameMap;
	mutable BoxExtent            *m_extent;

	bool                          m_modifyingSkeleton;

	mutable LocalShaderPrimitive *m_shaderPrimitive;

	Transform                    *m_scaleTransform;
	float                         m_scale;

	TransformModifierMap         *m_transformModifierMap;

private:

	// disabled
	Skeleton();
	Skeleton(const Skeleton&);
	Skeleton &operator =(const Skeleton&);

};

// ======================================================================

inline int Skeleton::getTransformCount() const
{
	return m_transformCount;
}

// ----------------------------------------------------------------------

inline const TransformAnimationResolver &Skeleton::getAnimationResolver() const
{
	return m_animationResolver;
}

// ----------------------------------------------------------------------

inline TransformAnimationResolver &Skeleton::getAnimationResolver()
{
	return m_animationResolver;
}

// ----------------------------------------------------------------------

inline float Skeleton::getScale() const
{
	return m_scale;
}

// ======================================================================

#endif
