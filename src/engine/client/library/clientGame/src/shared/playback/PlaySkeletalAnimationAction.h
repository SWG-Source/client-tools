// ======================================================================
//
// PlaySkeletalAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PlaySkeletalAnimationAction_H
#define INCLUDED_PlaySkeletalAnimationAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class CallbackAnimationNotification;
class CreatureObject;
class Object;
class PlaybackScript;
class PlaySkeletalAnimationActionTemplate;
class SkeletalAppearance2;

// ======================================================================

class PlaySkeletalAnimationAction: public PlaybackAction
{
public:

	static void install();

public:

	PlaySkeletalAnimationAction(const PlaySkeletalAnimationActionTemplate &actionTemplate);
	virtual ~PlaySkeletalAnimationAction();

	virtual void cleanup(PlaybackScript &playbackScript);
	virtual bool update(float deltaTime, PlaybackScript &script);

	CallbackAnimationNotification *fetchNotification();

	void debugDump() const;

private:

	static SkeletalAppearance2                *getActorAppearance(PlaybackScript &script, int actorIndex);

private:

	const PlaySkeletalAnimationActionTemplate &getOurTemplate() const;
	bool                                       initialize(PlaybackScript &script);
	void                                       handlePostureChange(Object &actor, int newPosture, bool skipTraversal) const;
	void                                       makeMountOrientForCombatAsNecessary(PlaybackScript &script, PlaySkeletalAnimationActionTemplate const &actionTemplate, Object &riderObject, CreatureObject &mountObject);

	// Disabled.
	PlaySkeletalAnimationAction();
	PlaySkeletalAnimationAction(const PlaySkeletalAnimationAction&);
	PlaySkeletalAnimationAction &operator =(const PlaySkeletalAnimationAction&);

private:

	bool                           m_initialized;

	int                            m_animationId;
	bool                           m_animationIsAdd;

	CallbackAnimationNotification *m_notification;

	SkeletalAppearance2 *          m_initialAppearance;

};

// ======================================================================

#endif
