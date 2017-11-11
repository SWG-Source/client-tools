// ======================================================================
//
// BasePriorityBlendAnimation.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BasePriorityBlendAnimation_H
#define INCLUDED_BasePriorityBlendAnimation_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"

class TransformNameMap;

// ======================================================================

class BasePriorityBlendAnimation: public SkeletalAnimation
{
public:

	enum BlendState
	{
		BS_playA,
		BS_blendToB,
		BS_playB,
		BS_blendToA
	};

public:

	static void                      install();
	static float                     getBlendTime();

public:

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

	BasePriorityBlendAnimation(SkeletalAnimationTemplate const *animationTemplate, TransformNameMap const &transformNameMap);
	virtual ~BasePriorityBlendAnimation();

	virtual int                      getAnimationCount() const = 0;
	virtual SkeletalAnimation       *getAnimation(int index) = 0;
	virtual SkeletalAnimation const *getAnimation(int index) const = 0;

	virtual SkeletalAnimation       *getPrimaryAnimation();
	virtual SkeletalAnimation const *getPrimaryAnimation() const;

	virtual bool                     doAlterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining) = 0;
	virtual void                     doEvaluateTransformComponents(int animationIndex, int transformIndex, Quaternion &rotation, Vector &translation);

	virtual bool                     allBlendStatesMatch(BlendState blendState) const;

private:

	struct BlendData;
	typedef stdvector<BlendData>::fwd  BlendDataVector;
	
private:

	virtual bool             alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining);
	void                     updateBlenderData(float elapsedTime);

	SkeletalAnimation       *getLocomotionPriorityAnimation();
	SkeletalAnimation const *getLocomotionPriorityAnimation() const;

	// disabled
	BasePriorityBlendAnimation();
	BasePriorityBlendAnimation(const BasePriorityBlendAnimation&);
	BasePriorityBlendAnimation &operator = (const BasePriorityBlendAnimation&);

private:

	int const        m_transformCount;
	BlendDataVector *m_blendDataVector;

};

// ======================================================================

#endif
