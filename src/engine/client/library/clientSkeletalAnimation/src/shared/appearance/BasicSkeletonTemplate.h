// ======================================================================
//
// BasicSkeletonTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BasicSkeletonTemplate_H
#define INCLUDED_BasicSkeletonTemplate_H

// ==================================================================

#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplateDef.h"

class CrcString;
class Iff;
class LessPointerComparator;
class PersistentCrcString;
class Quaternion;
class Skeleton;
class Transform;
class TransformAnimationResolver;
class TransformNameMap;
class Vector;

// ==================================================================

class BasicSkeletonTemplate: public SkeletonTemplate
{
friend class SkeletonTemplateList;

public:

	struct ModelToJointInfo;
	typedef stdvector<ModelToJointInfo*>::fwd  ModelToJointInfoVector;

	/**
	 * Defines the chain of skeleton segment attachments occuring from the
	 * absolute root of the skeleton to this skeleton template.
	 *
	 * This is used by the bind-pose meshToBone transform management.  The
	 * bind-pose mesh => bone transform is constant for a given skeleton segment
	 * hierarchy configuration.  Rather than duplicating this data in each
	 * skeleton for non-trivial memory duplication (roughly 1KB per 20 transforms),
	 * this data is managed by the skeleton template.
	 *
	 * AttachmentChain::first is the name of the skeleton template.
	 * AttachmentChain::second is the name of the transform to which the
	 * skeleton template is attached.
	 *
	 * Here's how the entries work:
	 *   Skeleton Template 0 in hierarchy: corresponds to AttachmentChain[-1] (i.e. nothing)
	 *   Skeleton Template 1 in hierarchy: corresponds to AttachmentChain[0]
	 *   Skeleton Template n in hierarchy: corresponds to AttachmentChain[n-1]
	 *
	 * The last entry in AttachmentChain indicates how this BasicSkeletonTemplate attaches
	 * to the skeleton segment chain.
	 */

	typedef stdvector<std::pair<std::string, std::string> >::fwd  AttachmentChain;
	typedef stdvector<Transform>::fwd                             TransformVector;

public:

	static void               install();

	// useful for std::accumulate
	static int                pointerJointCountAccumulator(int currentCount, const BasicSkeletonTemplate *skeletonTemplate);
	static int                referenceJointCountAccumulator(int currentCount, const BasicSkeletonTemplate &skeletonTemplate);

public:

	virtual int                          getDetailCount() const;
	virtual const BasicSkeletonTemplate *fetchBasicSkeletonTemplate(int detailIndex) const;

	Skeleton                 *createSkeleton(TransformAnimationResolver &animationResolver) const;

	int                       getJointCount() const;

	int                       getJointIndex(CrcString const &jointName) const;
	CrcString const          &getJointName(int jointIndex) const;
	void                      findJointIndex(CrcString const &jointName, int *jointIndex, bool *found) const;

	const int                *getJointParentIndexArray() const;

	//-- the following are all joint space to parent space transformation components
	const Quaternion         *getPreMultiplyRotations() const;
	const Quaternion         *getPostMultiplyRotations() const;

	const Vector             *getBindPoseTranslations() const;
	const Quaternion         *getBindPoseRotations() const;

	const ModelToJointInfo   *fetchBindPoseModelToJointTransforms(const AttachmentChain &attachmentChain, const Transform &parentModelToJointTransform, const TransformVector **bindPoseModelToJointTransforms) const;
	void                      releaseBindPoseModelToJointTransforms(const ModelToJointInfo *modelToJointInfo) const;

	const JointRotationOrder *getJointRotationOrderArray() const;

	const TransformNameMap   &getTransformNameMap() const;

#ifdef _DEBUG
	// diagnositics
	void        dumpJointNameIndexMap() const;
#endif

private:  

	typedef stdvector<PersistentCrcString*>::fwd                            PersistentCrcStringVector;
	typedef stdvector<int>::fwd                                             IntVector;
	typedef stdmap<const CrcString*, int, LessPointerComparator>::fwd       JointNameIndexMap;
	typedef stdvector<JointRotationOrder>::fwd                              JointRotationOrderVector;
	typedef stdvector<Quaternion>::fwd                                      QuaternionVector;
	typedef stdvector<Vector>::fwd                                          VectorVector;

private:

	static void              remove();
	static SkeletonTemplate *createSkeletonTemplate(Iff &iff, CrcString const &name);

private:

	BasicSkeletonTemplate(Iff *iff, CrcString const &name);
	virtual ~BasicSkeletonTemplate();

	void  load_0001(Iff *iff);
	void  load_0002(Iff *iff);

	int   getReferenceCount() const;

	void  buildModelToJointTransforms(const Transform &inheritedModelToRootTransform, TransformVector &modelToJointTransforms) const;

private:

	static bool                     ms_installed;

private:

	int                             m_jointCount;

	PersistentCrcStringVector      *m_jointNames;
	JointNameIndexMap              *m_jointNameIndexMap;
	IntVector                      *m_jointParentIndices;

	QuaternionVector               *m_preMultiplyRotations;
	QuaternionVector               *m_postMultiplyRotations;

	VectorVector                   *m_bindPoseTranslations;
	QuaternionVector               *m_bindPoseRotations;

	JointRotationOrderVector       *m_jointRotationOrder;

	mutable ModelToJointInfoVector *m_modelToJointInfo;

	mutable TransformNameMap       *m_transformNameMap;

private:

	// disabled
	BasicSkeletonTemplate();
	BasicSkeletonTemplate(const BasicSkeletonTemplate&);
	BasicSkeletonTemplate &operator =(const BasicSkeletonTemplate&);

};

// ======================================================================
/**
 * Retrieve the number of joint transforms exported by this instance.
 *
 * @return  the number of joint transforms exported by this instance.
 */

inline int BasicSkeletonTemplate::getJointCount() const
{
	return m_jointCount;
}

// ==================================================================

#endif
