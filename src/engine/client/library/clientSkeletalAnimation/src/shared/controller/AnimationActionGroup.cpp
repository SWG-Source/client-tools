// ======================================================================
//
// AnimationActionGroup.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationActionGroup.h"

#include "sharedFoundation/CrcLowerString.h"

// ======================================================================
// struct AnimationActionGroup::LessNameComparator
// ======================================================================

bool AnimationActionGroup::LessNameComparator::operator ()(const AnimationActionGroup *lhs, const AnimationActionGroup *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getName() < rhs->getName();
}

// ----------------------------------------------------------------------

bool AnimationActionGroup::LessNameComparator::operator ()(const CrcLowerString &lhs, const AnimationActionGroup *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getName();
}

// ----------------------------------------------------------------------

bool AnimationActionGroup::LessNameComparator::operator ()(const AnimationActionGroup *lhs, const CrcLowerString &rhs) const
{
	NOT_NULL(lhs);

	return lhs->getName() < rhs;
}

// ======================================================================

AnimationActionGroup::AnimationActionGroup()
{
}

// ----------------------------------------------------------------------

AnimationActionGroup::~AnimationActionGroup()
{
}

// ======================================================================
