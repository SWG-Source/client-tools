// ======================================================================
//
// TrackAnimationController_AddAnimation.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_AddAnimation_H
#define INCLUDED_TrackAnimationController_AddAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"

class TransformNameMap;

// ======================================================================

class TrackAnimationController::AddAnimation: public SkeletalAnimation
{
public:

	AddAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController, const AnimationTrackId &trackId1, const AnimationTrackId &trackId2);

	virtual bool                     alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	virtual void                     startNewCycle();

	virtual int                      getTransformCount() const;
	virtual void                     evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation);

	virtual int                      getTransformPriority(int index) const;
	virtual int                      getLocomotionPriority() const;

	virtual void                     getScaledLocomotion(Quaternion &rotation, Vector &translation) const;

	const SkeletalAnimationTemplate *getSkeletalAnimationTemplate() const;

	virtual float                    getCycleScaledLocomotionDistance() const;
	virtual int                      getFrameCount() const;
	virtual float                    getRecordedFramesPerSecond() const;

	virtual void                     setPlaybackFramesPerSecond(float playbackFramesPerSecond);
	virtual float                    getPlaybackFramesPerSecond() const;

	virtual void                     setFrameNumber(float frameNumber);

	virtual int                      getMessageCount() const;
	virtual const CrcLowerString    &getMessageName(int index) const;
	virtual void                     getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const;

	virtual SkeletalAnimation       *resolveSkeletalAnimation();

private:

	virtual ~AddAnimation();

	Track                   &getTrack1();
	Track                   &getTrack2();

	SkeletalAnimation       *getAnimation1();
	SkeletalAnimation       *getAnimation2();

	const SkeletalAnimation *getAnimation1() const;
	const SkeletalAnimation *getAnimation2() const;


	// disabled
	AddAnimation();
	AddAnimation(const AddAnimation&);
	AddAnimation &operator = (const AddAnimation&);

private:

	const int                 m_transformCount;
	TrackAnimationController &m_controller;
	const AnimationTrackId    m_trackId1;
	const AnimationTrackId    m_trackId2;

	SkeletalAnimation        *m_animation1;
	SkeletalAnimation        *m_animation2;

};

// ======================================================================

#endif
