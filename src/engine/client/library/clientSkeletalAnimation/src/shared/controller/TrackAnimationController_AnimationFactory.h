// ======================================================================
//
// TrackAnimationController_AnimationFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_AnimationFactory_H
#define INCLUDED_TrackAnimationController_AnimationFactory_H

// ======================================================================

#include "clientSkeletalAnimation/TrackAnimationController.h"

class SkeletalAnimation;
class TransformNameMap;

// ======================================================================

class TrackAnimationController::AnimationFactory
{
public:

	virtual ~AnimationFactory();
	virtual SkeletalAnimation *fetchSkeletalAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const = 0;

};

// ======================================================================
// Generally this is a no-no (inlined virtual function).  I do this here
// because I don't care to make a .cpp solely for an empty destructor.

inline TrackAnimationController::AnimationFactory::~AnimationFactory()
{
}

// ======================================================================

#endif
