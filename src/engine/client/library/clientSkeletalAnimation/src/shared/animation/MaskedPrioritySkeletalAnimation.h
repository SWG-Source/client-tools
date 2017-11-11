// ======================================================================
//
// MaskedPrioritySkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MaskedPrioritySkeletalAnimation_H
#define INCLUDED_MaskedPrioritySkeletalAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"

class TransformMask;
class TransformNameMap;

// ======================================================================

class MaskedPrioritySkeletalAnimation: public SkeletalAnimation
{
public:

	MaskedPrioritySkeletalAnimation(SkeletalAnimation &animation, const TransformNameMap &transformNameMap, const TransformMask &transformMask, int inGroupTransformPriority, int outGroupTransformPriority, int locomotionPriority);

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

	typedef stdvector<int>::fwd  IntVector;

private:

	virtual ~MaskedPrioritySkeletalAnimation();

	// Disabled.
	MaskedPrioritySkeletalAnimation();
	MaskedPrioritySkeletalAnimation(const MaskedPrioritySkeletalAnimation&);
	MaskedPrioritySkeletalAnimation &operator =(const MaskedPrioritySkeletalAnimation&);

private:

	SkeletalAnimation &      m_animation;
	IntVector         *const m_transformPriorities;
	const int                m_locomotionPriority;

};

// ======================================================================

#endif
