// ======================================================================
//
// RiderSpineTransformModifier.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/RiderSpineTransformModifier.h"

// ======================================================================

RiderSpineTransformModifier::RiderSpineTransformModifier(Vector const &alignedTargetDirection_l):
	AlignmentTransformModifier(alignedTargetDirection_l)
{
}

// ----------------------------------------------------------------------

Vector RiderSpineTransformModifier::getCurrentTargetDirection_w() const
{
	//-- Assume the terrain up direction is always unitY in world coordinates.
	return Vector::unitY;
}

// ======================================================================
