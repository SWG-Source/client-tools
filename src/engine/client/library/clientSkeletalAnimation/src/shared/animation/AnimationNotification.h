// ======================================================================
//
// AnimationNotification.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationNotification_H
#define INCLUDED_AnimationNotification_H

// ======================================================================

class AnimationTrackId;
class CrcString;
class Object;
class SkeletalAnimation;
class TrackAnimationController;
class TransformAnimationResolver;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"
#include <vector>

// ======================================================================

class AnimationNotification
{
	friend class SkeletalAnimation;
	friend class TrackAnimationController;
	friend class TransformAnimationResolver;

public:

	enum NotificationState
	{
		NS_noAnimationAttached,
		NS_attached,
		NS_inQueue,
		NS_playInProgress,
		NS_MARKER_noMorePlay, // This value will never be set as a state.  It is a marker used to see if an animation is still playing or waiting to be played.  Any value numerically larger than this marker indicates the animation is done for whatever reason.
		NS_stoppedNormal,
		NS_stoppedExplicitlyStopped,
		NS_stoppedInterrupted,
		NS_stoppedEjectedFromQueue,
		NS_stoppedSameTrackTrumped,
		NS_stoppedMisbehaving,
	};

public:

	static void        install();
	static char const *getStateName(NotificationState state);

public:

	AnimationNotification();

	void               associateObject(Object *object); // must be called before any of the SkeletalAnimation/TrackAnimationController functions may be set.

	void               fetch() const;
	void               release() const;
	int                getReferenceCount() const;

	int                getChannelCount() const;
	NotificationState  getState(int channel) const;
	bool               isAnimationDone(int channel) const;

	void               stopAnimation(int channel);
	SkeletalAnimation *fetchAnimation(int channel);

	virtual void       handleAnimationMessage(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);
	void               debugDump() const;

protected:

	virtual ~AnimationNotification();

private:

	class ChannelData;
	friend class ChannelData;

	typedef stdvector<ChannelData*>::fwd  ChannelDataVector;

private:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Interface used directly and only by SkeletalAnimation and TrackAnimationController
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  setChannelCount(int channelCount);
	void  attachAnimation(int channel, SkeletalAnimation *animation);
	void  setControllerInfo(int channel, TrackAnimationController *controller, AnimationTrackId const &trackId);
	void  setState(int channel, NotificationState state);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ChannelData       *getChannel(int index);
	ChannelData const *getChannel(int index) const;

	void               validateChannel(ChannelData const *channel) const;

	//-- Disabled.
	AnimationNotification(AnimationNotification const&);
	AnimationNotification &operator =(AnimationNotification const&);

private:

	int                mutable  m_referenceCount;
	ChannelDataVector           m_channelDataVector;
	Watcher<Object>             m_ownerObject;

};

// ======================================================================

#endif
