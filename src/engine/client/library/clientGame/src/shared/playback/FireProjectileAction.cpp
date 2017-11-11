// ======================================================================
//
// FireProjectileAction.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FireProjectileAction.h"

#include "clientGame/FireProjectileActionTemplate.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(FireProjectileAction, true ,0, 0, 0);

// ======================================================================

FireProjectileAction::~FireProjectileAction()
{
	releaseResources();
} //lint !e1740 // m_notification not directly freed or zero'd // It is in releaseResources().

// ----------------------------------------------------------------------

bool FireProjectileAction::update(float /* deltaTime */, PlaybackScript &script)
{
	//-- Ensure we have a notification.
	if (!m_notification)
	{
		DEBUG_WARNING(true, ("FireProjectileAction: ignoring action because the notification is NULL."));
		return false;
	}

	//-- Check if we received the trigger message.  If so, allow template to process the message.
	if (m_messageReceived)
		return NON_NULL(getPlaybackActionTemplate())->update(m_elapsedTimeSinceTrigger, script);

	//-- Check if the animation has stopped.  If so, so do we.
	if (m_notification->isAnimationDone(0))
	{
		// The animation finished without us getting triggered.  Stop.
		return false;
	}

	//-- Waiting for message or animation terminiation.
	return true;
}

// ----------------------------------------------------------------------

void FireProjectileAction::cleanup(PlaybackScript & /* playbackScript */)
{
	releaseResources();
}

// ----------------------------------------------------------------------

void FireProjectileAction::stop(PlaybackScript & /* script */)
{
	releaseResources();
}

// ======================================================================

void FireProjectileAction::handleAnimationMessageStatic(void *context, int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger)
{
	//-- Forward message to class instance.
	NOT_NULL(context);
	FireProjectileAction *const action = static_cast<FireProjectileAction*>(context);
	action->handleAnimationMessage(channel, animationMessage, elapsedTimeSinceTrigger);
}

// ======================================================================

FireProjectileAction::FireProjectileAction(FireProjectileActionTemplate const &actionTemplate, CallbackAnimationNotification *notification, char const *messageName):
	PlaybackAction(&actionTemplate),
	m_notification(notification),
	m_messageName(messageName, true),
	m_messageReceived(false),
	m_elapsedTimeSinceTrigger(0.0f)
{
	NOT_NULL(m_notification);
	m_notification->fetch();

	//-- Tell callback animation notification that we want a callback for this specific animation.
	m_notification->addCallback(handleAnimationMessageStatic, this);
}

// ----------------------------------------------------------------------

void FireProjectileAction::releaseResources()
{
	if (m_notification)
	{
		m_notification->removeCallback(handleAnimationMessageStatic, this);
		m_notification->release();
		m_notification = 0;
	}
}

// ----------------------------------------------------------------------

void FireProjectileAction::handleAnimationMessage(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger)
{
	//-- Check if this message is for the body (channel 0) and matches our particular fire message name.
	if ((channel == 0) && (animationMessage == m_messageName))
	{
		// Warn if we see a message more than once.
		DEBUG_WARNING(m_messageReceived, ("FireProjectileAction: received fire trigger animation message [%s] multiple times, should only see once, time-based bolt placement probably will be wrong.", animationMessage.getString()));

		// Remember that we saw the message so we can process it next update.
		m_messageReceived         = true;
		m_elapsedTimeSinceTrigger = elapsedTimeSinceTrigger;
	}
}

// ======================================================================
