// ======================================================================
//
// PriorityBlendAnimation.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PriorityBlendAnimation_H
#define INCLUDED_PriorityBlendAnimation_H

// ======================================================================

class PriorityBlendAnimationTemplate;

#include "clientSkeletalAnimation/BasePriorityBlendAnimation.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================
/**
 * An animation that has two component animations that are priority blended
 * together.
 *
 * The TrackAnimationController::PriorityBlendAnimation differs from this
 * animation.  This animation has two component animations that are not
 * accessible to a caller.  The TrackAnimationController version pulls
 * its animation data off of two other tracks, and the content of the tracks
 * is accessible to the user through the TrackAnimationController.
 *
 * This animation type is added to the .LAT file and handles priority blending
 * between two animations without using two tracks --- it can play entirely on
 * a loop track or an action track.
 */

class PriorityBlendAnimation: public BasePriorityBlendAnimation
{
	friend class PriorityBlendAnimationTemplate;
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	enum
	{
		MAX_ANIMATION_COUNT = 2
	};

protected:

	virtual int                      getAnimationCount() const;
	virtual SkeletalAnimation       *getAnimation(int index);
	virtual SkeletalAnimation const *getAnimation(int index) const;

	virtual SkeletalAnimation       *getPrimaryAnimation();
	virtual SkeletalAnimation const *getPrimaryAnimation() const;

	virtual bool                     doAlterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);

private:

	PriorityBlendAnimation(PriorityBlendAnimationTemplate const &animationTemplate, TransformNameMap const &transformNameMap, SkeletalAnimation *animation0, SkeletalAnimation *animation1, int primaryAnimationIndex);
	virtual ~PriorityBlendAnimation();

	SkeletalAnimation       *getAnimationArrayEntry(int index);
	SkeletalAnimation const *getAnimationArrayEntry(int index) const;

	// Disabled.
	PriorityBlendAnimation();
	PriorityBlendAnimation(PriorityBlendAnimation const&);
	PriorityBlendAnimation &operator =(PriorityBlendAnimation const&);

private:

	int                m_primaryAnimationIndex;
	SkeletalAnimation *m_animationArray[MAX_ANIMATION_COUNT];

};

// ======================================================================

#endif
