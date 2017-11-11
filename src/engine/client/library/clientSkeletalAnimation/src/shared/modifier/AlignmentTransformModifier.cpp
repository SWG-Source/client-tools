// ======================================================================
//
// AlignmentTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AlignmentTransformModifier.h"

#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace AlignmentTransformModifierNamespace
{
	Transform s_alignmentTransform;
}

using namespace AlignmentTransformModifierNamespace;

// ======================================================================

AlignmentTransformModifier::AlignmentTransformModifier(Vector const &alignedTargetDirection_l):
	TransformModifier(),
	m_alignedTargetDirection_l(alignedTargetDirection_l),
	m_cosAlignmentTolerance(cos(1.0f * PI_OVER_180))
{
}

// ----------------------------------------------------------------------

bool AlignmentTransformModifier::modifyTransform(float /* elapsedTime */, Skeleton const &skeleton, CrcString const & /* transformName */, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o)
{
	//-- Get the object for the skeleton's associated appearance so we can get to world space.
	Object const *object = skeleton.getSkeletalAppearance().getOwner();
	if (!object)
	{
		DEBUG_WARNING(true, ("AlignmentTransformModifier::modifyTransform(): appearance template [%s] had a NULL owner object, cannot modify transform.", skeleton.getSkeletalAppearance().getAppearanceTemplateName()));
		return false;
	}

	//-- Get target direction in world space, convert into local joint space.
	Vector const currentTargetDirection_w = getCurrentTargetDirection_w();
	Vector const currentTargetDirection_o = object->getTransform_o2w().rotate_p2l(currentTargetDirection_w);

	transform_l2o.multiply(transform_p2o, transform_l2p);
	Vector const currentTargetDirection_l = transform_l2o.rotate_p2l(currentTargetDirection_o);

	//-- Determine if target direction's deviation from desired modifier is small enough to ignore.
	float const cosAngleBetween = m_alignedTargetDirection_l.dot(currentTargetDirection_l);
	bool const  ignoreAlignment = (cosAngleBetween >= m_cosAlignmentTolerance);
	if (ignoreAlignment)
	{
		// Skip --- the deviation from the desired alignment is too small.
		return false;
	}

	//-- Find an axis perpendicular to the plane between both vectors.  We'll rotate around this vector.
	Vector const rotationAxis_l = m_alignedTargetDirection_l.cross(currentTargetDirection_l);

	//-- Determine how much we plan to rotate toward the target direction.
	// @todo use a constant or accelerating rate.  This currently snaps to the desired target rotation.
	float const      angleBetween = acos(cosAngleBetween);
	Quaternion const alignmentRotation_l(angleBetween, rotationAxis_l);
	alignmentRotation_l.getTransformPreserveTranslation(&s_alignmentTransform);

	//-- Incorporate the rotation.
	transform_l2o.multiply(transform_l2o, s_alignmentTransform);

	return true;
}

// ----------------------------------------------------------------------

Vector const AlignmentTransformModifier::getAlignedTargetDirection_l() const
{
	return m_alignedTargetDirection_l;
}

// ----------------------------------------------------------------------

void AlignmentTransformModifier::setAlignedTargetDirection_l(Vector const &alignedTargetDirection_l)
{
	m_alignedTargetDirection_l = alignedTargetDirection_l;
}

// ======================================================================
