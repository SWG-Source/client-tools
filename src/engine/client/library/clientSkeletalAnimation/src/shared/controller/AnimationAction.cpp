// ======================================================================
//
// AnimationAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationAction.h"

#include "sharedFoundation/CrcLowerString.h"

// ======================================================================
// struct AnimationAction::LessNameCrcOrderPointerComparator
// ======================================================================

bool AnimationAction::LessNameCrcOrderPointerComparator::operator ()(const AnimationAction *lhs, const AnimationAction *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getName() < rhs->getName();
}

// ----------------------------------------------------------------------

bool AnimationAction::LessNameCrcOrderPointerComparator::operator ()(const CrcLowerString &lhs, const AnimationAction *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getName();
}

// ----------------------------------------------------------------------

bool AnimationAction::LessNameCrcOrderPointerComparator::operator ()(const AnimationAction *lhs, const CrcLowerString &rhs) const
{
	NOT_NULL(lhs);

	return lhs->getName() < rhs;
}

// ======================================================================
// struct AnimationAction::LessNameAbcOrderPointerComparator
// ======================================================================

bool AnimationAction::LessNameAbcOrderPointerComparator::operator ()(const AnimationAction *lhs, const AnimationAction *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return (strcmp(lhs->getName().getString(), rhs->getName().getString()) < 0);
}

// ======================================================================
// class AnimationAction
// ======================================================================

AnimationAction::AnimationAction()
{
}

// ----------------------------------------------------------------------

AnimationAction::~AnimationAction()
{
}

// ======================================================================
