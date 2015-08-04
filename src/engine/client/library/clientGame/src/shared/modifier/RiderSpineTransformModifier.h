// ======================================================================
//
// RiderSpineTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RiderSpineTransformModifier_H
#define INCLUDED_RiderSpineTransformModifier_H

// ======================================================================

#include "clientSkeletalAnimation/AlignmentTransformModifier.h"

// ======================================================================

class RiderSpineTransformModifier: public AlignmentTransformModifier
{
public:

	explicit RiderSpineTransformModifier(Vector const &alignedTargetDirection_l);

	virtual Vector  getCurrentTargetDirection_w() const;
	
private:

	// Disabled.
	RiderSpineTransformModifier();
	RiderSpineTransformModifier(RiderSpineTransformModifier const&);
	RiderSpineTransformModifier &operator =(RiderSpineTransformModifier const &);

};

// ======================================================================

#endif
