// ======================================================================
//
// FireProjectileAction.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FireProjectileAction_H
#define INCLUDED_FireProjectileAction_H

// ======================================================================

class CallbackAnimationNotification;

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"

// ======================================================================

class FireProjectileAction: public PlaybackAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	friend class FireProjectileActionTemplate;

public:

	virtual ~FireProjectileAction();

	virtual bool                  update(float deltaTime, PlaybackScript &script);

	virtual void                  cleanup(PlaybackScript &playbackScript);
	virtual void                  stop(PlaybackScript &script);

private:

	void static                   handleAnimationMessageStatic(void *context, int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);
	
private:

	FireProjectileAction(FireProjectileActionTemplate const &actionTemplate, CallbackAnimationNotification *notification, char const *messageName);

	void                          releaseResources();
	void                          handleAnimationMessage(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);

	// Disabled.
	FireProjectileAction();
	FireProjectileAction(FireProjectileAction const&);
	FireProjectileAction &operator =(FireProjectileAction const&);

private:

	CallbackAnimationNotification *m_notification;
	TemporaryCrcString             m_messageName;
	bool                           m_messageReceived;
	float                          m_elapsedTimeSinceTrigger;

};

// ======================================================================

#endif
