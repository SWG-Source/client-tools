// ======================================================================
//
// ZeroTranslationTransformModifier.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ZeroTranslationTransformModifier.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

// ======================================================================

ZeroTranslationTransformModifier::ZeroTranslationTransformModifier():
	TransformModifier()
{
}

// ----------------------------------------------------------------------

bool ZeroTranslationTransformModifier::modifyTransform(float /* elapsedTime */, Skeleton const & /* skeleton */, CrcString const & /* transformName */, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o)
{
	//-- Zero out this joint's translation relative to it's parent, return the new localToObject transform.
	Transform newTransform_l2p(transform_l2p);
	newTransform_l2p.setPosition_p(Vector::zero);

	transform_l2o.multiply(transform_p2o, newTransform_l2p);
	return true;
}

// ======================================================================
