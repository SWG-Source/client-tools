// PRIVATE

// ======================================================================
//
// TimeScaleSkeletalAnimation.h
// Copyright 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TimeScaleSkeletalAnimation_H
#define INCLUDED_TimeScaleSkeletalAnimation_H

// ======================================================================

class TimeScaleSkeletalAnimationTemplate;

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class TimeScaleSkeletalAnimation: public SkeletalAnimation
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;
	friend class TimeScaleSkeletalAnimationTemplate;

public:

	virtual bool                     alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	virtual void                     startNewCycle();

	virtual int                      getTransformCount() const;
	virtual void                     evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation);

	virtual int                      getTransformPriority(int index) const;
	virtual int                      getLocomotionPriority() const;

	virtual void                     getScaledLocomotion(Quaternion &rotation, Vector &translation) const;
	virtual float                    getCycleScaledLocomotionDistance() const;
	virtual int                      getFrameCount() const;
	virtual float                    getRecordedFramesPerSecond() const;

	virtual void                     setPlaybackFramesPerSecond(float playbackFramesPerSecond);
	virtual float                    getPlaybackFramesPerSecond() const;

	virtual int                      getMessageCount() const;
	virtual const CrcLowerString    &getMessageName(int index) const;
	virtual void                     getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const;

	virtual SkeletalAnimation       *resolveSkeletalAnimation();

protected:

	virtual ~TimeScaleSkeletalAnimation();

private:

	TimeScaleSkeletalAnimation(TimeScaleSkeletalAnimationTemplate const *skeletalAnimationTemplate, SkeletalAnimation *baseSkeletalAnimation);

	TimeScaleSkeletalAnimationTemplate const &getTimeScaleSkeletalAnimationTemplate() const;

	// Disabled.
	TimeScaleSkeletalAnimation();
	TimeScaleSkeletalAnimation(const TimeScaleSkeletalAnimation&);
	TimeScaleSkeletalAnimation &operator = (const TimeScaleSkeletalAnimation&);

private:

	SkeletalAnimation *m_baseAnimation;

};

// ======================================================================

#endif
