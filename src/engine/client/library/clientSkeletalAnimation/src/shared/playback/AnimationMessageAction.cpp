// ======================================================================
//
// AnimationMessageAction.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationMessageAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientSkeletalAnimation/AnimationMessageActionTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationController.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"

#include <string>

// ======================================================================
// inlines
// ======================================================================

inline const AnimationMessageActionTemplate &AnimationMessageAction::getAnimationMessageActionTemplate() const
{
	return *(safe_cast<const AnimationMessageActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// public member functions
// ======================================================================

AnimationMessageAction::AnimationMessageAction(const char *messageName, int actorIndex) :
	PlaybackAction(0),
	m_messageName(new CrcLowerString(messageName)),
	m_animationController(0),
	m_callbackId(-1),
	m_messageReceived(false),
	m_useTemplate(false),
	m_actorIndex(actorIndex)
{
}

// ----------------------------------------------------------------------

AnimationMessageAction::~AnimationMessageAction()
{
	DEBUG_WARNING(m_callbackId >= 0, ("leaked an object message callback"));
	m_animationController = 0;

	delete m_messageName;
}

// ----------------------------------------------------------------------

void AnimationMessageAction::notifyActorAdded(PlaybackScript &playbackScript, int actorIndex)
{
	// check if we're dealing with our target actor.
	if (actorIndex != m_actorIndex)
	{
		return;
	}

	// sanity check: we shouldn't be setting this multiple times without intervening notifyRemovingActor() calls
	DEBUG_FATAL((m_callbackId != -1) || m_animationController, ("notifyActorAdded() appears to have been called multiple times for the target actor index %d", m_actorIndex));

	//-- retrieve the actor's animation controller.
	Object *const actor = playbackScript.getActor(actorIndex);
	if (!actor)
	{
		DEBUG_WARNING(!actor, ("specified actor was added by playbackScript reported as null"));
		m_messageReceived = true;
		return;
	}

	// get the base appearance
	Appearance *const baseAppearance = actor->getAppearance();
	if (!baseAppearance)
	{
		DEBUG_WARNING(true, ("actor object has no appearance, signaling trigger"));
		m_messageReceived = true;
		return;
	}

	// get the skeletal appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*>(baseAppearance);
	if (!skeletalAppearance)
	{
		DEBUG_WARNING(true, ("actor object does not have a skeletal appearance, signaling trigger"));
		m_messageReceived = true;
		return;
	}

	// get the animation controller
	m_animationController = skeletalAppearance->getAnimationController();
	if (!m_animationController)
	{
		DEBUG_WARNING(true, ("skeletal appearance has no animation controller, signaling trigger"));
		m_messageReceived = true;
		return;
	}

	//-- add this instance as a listener for animation messages.  until
	//   this listener is removed, we'll receive a callback for every
	//   animation message emitted.
	m_callbackId = m_animationController->addAnimationMessageListener(receiveAnimationMessageCallback, this);
	DEBUG_FATAL(m_callbackId == -1, ("addAnimationMessageListener failed()"));
}

// ----------------------------------------------------------------------

void AnimationMessageAction::notifyRemovingActor(PlaybackScript &playbackScript, int actorIndex)
{
	UNREF(playbackScript);

	// check if we're dealing with the target actor
	if (actorIndex != m_actorIndex)
		return;

	// if we're listening for animation messages, cancel that
	if (m_callbackId != -1)
	{
		NOT_NULL(m_animationController);
		m_animationController->removeAnimationMessageListener(m_callbackId);
		m_callbackId = -1;
	}
} //lint !e1764 // Info -- Reference parameter 'playbackScript' could be declared const ref // needed for other classes in hierarchy

// ----------------------------------------------------------------------

bool AnimationMessageAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	//-- trivially handle when we've already received the message
	if (m_messageReceived)
	{
		// move forward
		return false;
	}

	//-- retrieve the target message name if not yet received.
	// -TRF- perhaps this should be in a "notifyPreFirstScriptUpdate()" --- there is a race condition right now.
	if (!m_messageName)
	{
		DEBUG_FATAL(!m_useTemplate, ("shouldn't get here if not using a template."));

		//-- get the message name from the message variable
		std::string  messageName;

		const bool result = script.getStringVariable(getAnimationMessageActionTemplate().getMessageNameVariable(), messageName);
		UNREF(result);
	#if _DEBUG
		if (!result)
		{
			char buffer[5];
			ConvertTagToString(getAnimationMessageActionTemplate().getMessageNameVariable(), buffer);
			DEBUG_FATAL(true, ("animation message trigger trying to initilialize with non-existent variable [%s]\n", buffer));
		}
	#endif

		//-- create the animation message name we'll scan for.
		m_messageName = new CrcLowerString(messageName.c_str());
	}

	//-- we haven't received the message yet, stall.
	return true;
} //lint !e1764 // Info -- Reference parameter 'script' could be declared const ref // needed for other classes in hierarchy

// ======================================================================
// private static member functions
// ======================================================================

void AnimationMessageAction::receiveAnimationMessageCallback(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController *controller)
{
	UNREF(controller);

	// context should be set to the AnimationMessageAction instance pointer.
	NOT_NULL(context);
	AnimationMessageAction *const trigger = reinterpret_cast<AnimationMessageAction*>(context);

	// do the real trigger handling
	trigger->handleAnimationMessage(animationMessageName);
}

// ======================================================================
// private member functions
// ======================================================================

AnimationMessageAction::AnimationMessageAction(const AnimationMessageActionTemplate &triggerTemplate) :
	PlaybackAction(&triggerTemplate),
	m_messageName(0),
	m_animationController(0),
	m_callbackId(-1),
	m_messageReceived(false),
	m_useTemplate(true),
	m_actorIndex(triggerTemplate.getActorIndex())
{
}

// ----------------------------------------------------------------------

void AnimationMessageAction::handleAnimationMessage(const CrcLowerString &messageName)
{
	// check if we've had our update called yet.  if the message name isn't set,
	// the update hasn't been called yet.  if not, we ignore the message.
	if (!m_messageName)
		return;

	// signal the trigger if the emitted message name is our target message name.
	if (messageName == *m_messageName)
		m_messageReceived = true;
}

// ======================================================================
