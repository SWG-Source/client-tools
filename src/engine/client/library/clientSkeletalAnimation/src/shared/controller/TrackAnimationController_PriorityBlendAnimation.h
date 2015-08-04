// ======================================================================
//
// TrackAnimationController_PriorityBlendAnimation.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_PriorityBlendAnimation_H
#define INCLUDED_TrackAnimationController_PriorityBlendAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/BasePriorityBlendAnimation.h"

class TransformNameMap;

// ======================================================================

class TrackAnimationController::PriorityBlendAnimation: public BasePriorityBlendAnimation
{
public:

	PriorityBlendAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController, const AnimationTrackId &trackId1, const AnimationTrackId &trackId2);

	const Track                     &getLocomotionPriorityTrack() const;

	virtual void                     startNewCycle();

protected:

	virtual int                      getAnimationCount() const;
	virtual SkeletalAnimation       *getAnimation(int index);
	virtual SkeletalAnimation const *getAnimation(int index) const;

	virtual bool                     doAlterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	virtual void                     doEvaluateTransformComponents(int animationIndex, int transformIndex, Quaternion &rotation, Vector &translation);

private:

	virtual ~PriorityBlendAnimation();

	bool  readyToDisableTrack() const;
	void  disableTrack();

	// Disabled.
	PriorityBlendAnimation();
	PriorityBlendAnimation(const PriorityBlendAnimation&);
	PriorityBlendAnimation &operator = (const PriorityBlendAnimation&);

private:

	TrackAnimationController &m_controller;
	const AnimationTrackId    m_trackId1;
	const AnimationTrackId    m_trackId2;

	Track                    *m_track1;
	Track                    *m_track2;

	/// The optional (non-primary, blend in track) track, 0 = track1, 1 = track2.
	int                       m_optionalTrackIndex;
};

// ======================================================================

#endif
