// ======================================================================
//
// TrackAnimationController_Track.h
// Copyright 2002-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_Track_H
#define INCLUDED_TrackAnimationController_Track_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"

// This is a private inner class included almost nowhere, so I do not mind including 
// implementation details (headers) to avoid some heap usage.
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <vector>

class AnimationNotification;
class CrcString;

// ======================================================================

class TrackAnimationController::Track
{
public:

	Track(TrackAnimationController &controller, const PhysicalTrackTemplate &physicalTrackTemplate);
	~Track();

	void               alter(float deltaTime, bool processAnimationMessages = true);
	void               evaluateTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation);
	void               getMostRecentAnimationTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation);

	int                playAnimation(SkeletalAnimation *skeletalAnimation, PlayMode playMode, bool loop, BlendMode transitionBlendMode, float blendInTime, AnimationNotification *notification);
	void               stopAnimation();

	int                getMostRecentlyCompletedAnimationId() const;
	bool               getCurrentlyPlayingAnimationId(int &animationId) const;

	void               getObjectLocomotion(Quaternion &rotation, Vector &translation) const;

	bool               isIdle() const;

	SkeletalAnimation *fetchCurrentAnimation();
	SkeletalAnimation const *fetchCurrentAnimation() const;

	void clearLocomotionTranslation();
	void clearLocomotionRotation();


	const Track       &getLocomotionPriorityTrack() const;

	const CrcString   &getTrackName() const;

private:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct QueueEntry
	{
	public:

		QueueEntry(SkeletalAnimation *skeletalAnimation, bool loop, BlendMode blendMode, float blendInTime, int animationId);
		~QueueEntry();

	public:

		SkeletalAnimation *m_skeletalAnimation;
		bool               m_loop;
		BlendMode          m_blendMode;
		float              m_blendInTime;
		int                m_animationId;

	private:
		// disabled
		QueueEntry();

		//lint -esym(754, QueueEntry::QueueEntry)  // unreferenced // defensive hiding
		//lint -esym(1714, QueueEntry::QueueEntry)
		QueueEntry(const QueueEntry&);              

		//lint -esym(754, QueueEntry::operator=)  // unreferenced // defensive hiding
		//lint -esym(1714, QueueEntry::operator=)
		QueueEntry &operator =(const QueueEntry&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<QueueEntry*>::fwd  QueueEntryVector;
	typedef stdvector<Quaternion>::fwd   QuaternionVector;
	typedef stdvector<Vector>::fwd       VectorVector;

private:

	void                         clearQueue();
	void                         transitionToNextQueueEntry();

	void                         playNow(SkeletalAnimation *skeletalAnimation, bool loop, BlendMode transitionBlendMode, float blendInTime, int animationId);

	const PhysicalTrackTemplate &getPhysicalTrackTemplate() const;

	void                         preExitIdleHook();
	void                         postEnterIdleHook();

	void                         createInitialAnimation();

	// disabled
	Track();

	//lint -esym(754, Track::Track)  // unreferenced // defensive hiding
	//lint -esym(1714, Track::Track) // unreferenced
	Track(const Track&);             

	//lint -esym(754, Track::operator=)  // unreferenced // defensive hiding
	//lint -esym(1714, Track::operator=) // unreferenced
	Track &operator =(const Track&); 

private:

	TrackAnimationController    &m_controller;
	const PhysicalTrackTemplate &m_physicalTrackTemplate;

	SkeletalAnimation           *m_currentAnimation;

	bool                         m_loop;
	QueueEntryVector             m_queue;
	bool                         m_isIdle;

	Vector                       m_locomotionTranslation;
	Quaternion                   m_locomotionRotation;

	QuaternionVector            *m_mostRecentRotations;
	VectorVector                *m_mostRecentTranslations;

	int                          m_currentAnimationId;
	int                          m_nextAnimationId;

};

// ======================================================================

inline SkeletalAnimation *TrackAnimationController::Track::fetchCurrentAnimation()
{
	if (m_currentAnimation)
		m_currentAnimation->fetch();
	return m_currentAnimation;
}

// ----------------------------------------------------------------------

inline SkeletalAnimation const *TrackAnimationController::Track::fetchCurrentAnimation() const
{
	if (m_currentAnimation)
		m_currentAnimation->fetch();
	return m_currentAnimation;
}

// ----------------------------------------------------------------------

inline bool TrackAnimationController::Track::isIdle() const
{
	return m_isIdle;
}

// ======================================================================

#endif
