// ======================================================================
//
// ZeroTranslationTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ZeroTranslationTransformModifier_H
#define INCLUDED_ZeroTranslationTransformModifier_H

// ======================================================================

#include "clientSkeletalAnimation/TransformModifier.h"

// ======================================================================

class ZeroTranslationTransformModifier: public TransformModifier
{
public:

	ZeroTranslationTransformModifier();

	virtual bool modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o);

private:

	// Disabled.
	ZeroTranslationTransformModifier(ZeroTranslationTransformModifier const&);
	ZeroTranslationTransformModifier &operator =(ZeroTranslationTransformModifier const&);
};

// ======================================================================

#endif
