// ======================================================================
//
// TransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TransformModifier_H
#define INCLUDED_TransformModifier_H

// ======================================================================

class CrcString;
class Skeleton;
class Transform;

// ======================================================================

class TransformModifier
{
public:

	virtual ~TransformModifier();

	virtual bool modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o) = 0;

protected:

	TransformModifier();

private:

	// Disabled.
	TransformModifier(TransformModifier const&);
	TransformModifier &operator =(TransformModifier const&);

};

// ======================================================================

#endif
