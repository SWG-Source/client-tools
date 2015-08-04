// ======================================================================
//
// AnimationMessageAction.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationMessageAction_H
#define INCLUDED_AnimationMessageAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class AnimationMessageActionTemplate;
class CrcLowerString;
class Object;
class TransformAnimationController;

// ======================================================================

class AnimationMessageAction: public PlaybackAction
{
friend class AnimationMessageActionTemplate;

public:

	AnimationMessageAction(const char *messageName, int actorIndex);
	virtual ~AnimationMessageAction();

	virtual void  notifyActorAdded(PlaybackScript &playbackScript, int actorIndex);
	virtual void  notifyRemovingActor(PlaybackScript &playbackScript, int actorIndex);

	virtual bool  update(float deltaTime, PlaybackScript &script);

private:

	static void   receiveAnimationMessageCallback(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController *controller);

private:

	explicit AnimationMessageAction(const AnimationMessageActionTemplate &triggerTemplate);

	void                                   handleAnimationMessage(const CrcLowerString &messageName);
	const AnimationMessageActionTemplate &getAnimationMessageActionTemplate() const;

	// disabled
	AnimationMessageAction();
	AnimationMessageAction(const AnimationMessageAction&);
	AnimationMessageAction &operator =(const AnimationMessageAction&);

private:

	CrcLowerString               *m_messageName;
	TransformAnimationController *m_animationController;

	int                           m_callbackId;

	bool                          m_messageReceived;

	bool                          m_useTemplate;
	int                           m_actorIndex;

};

// ======================================================================

#endif
