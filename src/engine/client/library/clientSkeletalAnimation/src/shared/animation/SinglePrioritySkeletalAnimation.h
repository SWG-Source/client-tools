// ======================================================================
//
// SinglePrioritySkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SinglePrioritySkeletalAnimation_H
#define INCLUDED_SinglePrioritySkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class MemoryBlockManager;

// ======================================================================

class SinglePrioritySkeletalAnimation: public SkeletalAnimation
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	SinglePrioritySkeletalAnimation(SkeletalAnimation &animation, int uniformTransformPriority, int locomotionPriority);

	virtual int                      getTransformPriority(int index) const;
	virtual int                      getLocomotionPriority() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// The following interface functions are forwarded to the target animation
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual bool                     alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	virtual void                     startNewCycle();

	virtual int                      getTransformCount() const;
	virtual void                     evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation);

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

private:

	virtual ~SinglePrioritySkeletalAnimation();

	// Disabled.
	SinglePrioritySkeletalAnimation();
	SinglePrioritySkeletalAnimation(const SinglePrioritySkeletalAnimation&);
	SinglePrioritySkeletalAnimation &operator =(const SinglePrioritySkeletalAnimation&);

private:

	SkeletalAnimation &m_animation;
	const int          m_uniformTransformPriority;
	const int          m_locomotionPriority;

};

// ======================================================================

#endif
