// ======================================================================
//
// CallbackAnimationNotification.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CallbackAnimationNotification_H
#define INCLUDED_CallbackAnimationNotification_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationNotification.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include <vector>

// ======================================================================

class CallbackAnimationNotification: public AnimationNotification
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	typedef void (*Callback)(void *context, int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);

public:

	CallbackAnimationNotification();

	void          addCallback(Callback callback, void *context);
	void          removeCallback(Callback callback, void *context);

	virtual void  handleAnimationMessage(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger);

private:

	struct CallbackInfo;
	typedef std::vector<CallbackInfo>  CallbackInfoVector;

private:

	// Disabled.
	CallbackAnimationNotification(CallbackAnimationNotification const&);
	CallbackAnimationNotification &operator =(CallbackAnimationNotification const&);

private:

	CallbackInfoVector  m_callbackInfoVector;

};

// ======================================================================

#endif
