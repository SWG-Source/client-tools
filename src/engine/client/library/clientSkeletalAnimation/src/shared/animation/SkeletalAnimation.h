// ======================================================================
//
// SkeletalAnimation.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletalAnimation_H
#define INCLUDED_SkeletalAnimation_H

// ======================================================================

class AnimationNotification;
class CrcLowerString;
class CrcString;
class Quaternion;
class SkeletalAnimationTemplate;
class TransformNameMap;
class Vector;

// ======================================================================

class SkeletalAnimation
{
public:

	static void install();

	static int compareAnimationPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs);
	static int compareAnimationTransformPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs);
	static int compareAnimationLocomotionPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs);

public:

	void fetch() const;
	void release() const;

	void                             getMinMaxTransformPriority(int &min, int &max) const;
	const CrcString                 &getLeafAnimationTemplateName() const;

	void                             setNotification(AnimationNotification *notification, int channel);
	bool                             hasNotification() const;

	void                             setStateInQueue(int channel);
	void                             setStatePlayInProgress(int channel);
	void                             setStateStoppedNormal(int channel);
	void                             setStateStoppedExplicitlyStopped(int channel);
	void                             setStateStoppedInterruped(int channel);
	void                             setStateStoppedEjectedFromQueue(int channel);
	void                             setStateSameTrackTrumped(int channel);
	void                             setStateStoppedMisbehaving(int channel);

	void                             handleAnimationMessage(int channel, CrcString const &messageName, float elapsedTimeSinceMessageTrigger);

	/**
	 * Increment the animation for as much as the specified amount of time.
	 *
	 * It is the caller's responsibility to fetch non-NULL replacement animation if it
	 * will be used.  This function does not do a fetch on that value.
	 * 
	 * @param deltaTime             The total amount of time that should be applied to animations.
	 *                              The actual amount applied during this call is deltaTime - deltaTimeRemaining.
	 * @param replacementAnimation  If the function returns false, the animation returned in this parameter should 
	 *                              replace the current animation.  Otherwise, disregard the return value in this parameter.
	 *                              This return value has not been pre-fetched.
	 * @param deltaTimeRemaining    If the value returned in this parameter is greater than 0.0f, the caller should
	 *                              either (1) reset the current animation for a new loop (via startNewCycle()) or
	 *                              (2) stop the current animation and apply the detailTimeRemaining to a new animation.
	 *
	 * @return  true if the current animation has not died on its own; false if the current animation has died on its own.
	 *          When returning true, the caller can optionally choose to loop or stop the animation as described above.  When returning
	 *          false, the caller should stop the animation without question.
	 */
	virtual bool                     alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining) = 0;
	virtual void                     startNewCycle() = 0;

	virtual int                      getTransformCount() const = 0;
	virtual void                     evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation) = 0;

	virtual int                      getTransformPriority(int index) const = 0;
	virtual int                      getLocomotionPriority() const = 0;

	virtual void                     getScaledLocomotion(Quaternion &rotation, Vector &translation) const = 0;

	const SkeletalAnimationTemplate *getSkeletalAnimationTemplate() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Functions needed for locomotion support.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Retrieve the distance of locomotion that occurs in a single cycle of 
	 * animation, accounting for Appearance scaling.
	 *
	 * The distance considered is the distance between the start and end
	 * point of one cycle of the specified animation.  It ignores any
	 * staggering in the animation cycle (i.e. it is not counting non-linear
	 * path length, only the linear distance between the start and end of
	 * the animation for one animation cycle).
	 *
	 * @return  the distance of locomotion that occurs in a single cycle of animation.
	 */
	virtual float                    getCycleScaledLocomotionDistance() const = 0;
	virtual int                      getFrameCount() const = 0;
	virtual float                    getRecordedFramesPerSecond() const = 0;

	virtual void                     setPlaybackFramesPerSecond(float playbackFramesPerSecond) = 0;
	virtual float                    getPlaybackFramesPerSecond() const = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Animation message support.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual int                      getMessageCount() const = 0;
	virtual const CrcLowerString    &getMessageName(int index) const = 0;
	virtual void                     getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const = 0;

	/**
	 * Resolve this skeletal animation to the next child level closer to the leaf
	 * of the SkeletalAnimation tree.
	 *
	 * This function will make any choices required to choose a child animation.
	 * Only one level is traversed.  For multi-level animation hierarchies, multiple
	 * calls to this function will be required to traverse to a leaf.  Leaves typically
	 * are KeyframeSkeletalAnimation (.ans player) nodes.
	 *
	 * This function is not the normal root to resolving an animation --- typically
	 * an animation is played by calling alterSingleCycle() and then retrieving
	 * data via the other interface functions.  This interface function is used
	 * to walk down the tree and (eventually) hit an .ans animation or possibly
	 * some specific SkeletalAnimation-derived class along the way.
	 *
	 * For example, the SWG combat system will traverse the attacker's action
	 * animation's animation tree searching for an AttackRegionSkeletalAnimation,
	 * which knows how to choose a random attack targeted at a specific skeletal
	 * region.  The combat system needs to know which attack is chosen by the
	 * attacker so that the defender can be setup to do the corresponding
	 * block for the given attack.
	 *
	 * @return  if not at the leaf, returns the skeletal animation at the next child 
	 *          level closer to the leaf; returns NULL if at the leaf.
	 */
	virtual SkeletalAnimation       *resolveSkeletalAnimation() = 0;

protected:

	explicit SkeletalAnimation(const SkeletalAnimationTemplate *skeletalAnimationTemplate);
	virtual ~SkeletalAnimation();

private:

	// disabled
	SkeletalAnimation();
	SkeletalAnimation(const SkeletalAnimation&);
	SkeletalAnimation &operator = (const SkeletalAnimation&);

private:

	mutable int                      m_referenceCount;
	const SkeletalAnimationTemplate *m_skeletalAnimationTemplate;
	AnimationNotification           *m_notification;

};

// ======================================================================

inline void SkeletalAnimation::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline const SkeletalAnimationTemplate *SkeletalAnimation::getSkeletalAnimationTemplate() const
{
	return m_skeletalAnimationTemplate;
}

// ======================================================================

#endif
