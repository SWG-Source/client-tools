// ======================================================================
//
// AlignmentTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AlignmentTransformModifier_H
#define INCLUDED_AlignmentTransformModifier_H

// ======================================================================

#include "clientSkeletalAnimation/TransformModifier.h"
#include "sharedMath/Vector.h"

// ======================================================================

class AlignmentTransformModifier: public TransformModifier
{
public:

	explicit AlignmentTransformModifier(Vector const &alignedTargetDirection_l);

	virtual bool    modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o);
	virtual Vector  getCurrentTargetDirection_w() const = 0;

	Vector const    getAlignedTargetDirection_l() const;
	void            setAlignedTargetDirection_l(Vector const &alignedTargetDirection_l);

private:

	// Disabled.
	AlignmentTransformModifier();
	AlignmentTransformModifier(AlignmentTransformModifier const &);
	AlignmentTransformModifier &operator =(AlignmentTransformModifier const &);

private:

	Vector  m_alignedTargetDirection_l;
	float   m_cosAlignmentTolerance;

};

// ======================================================================

#endif
