// ======================================================================
//
// CallbackAnimationNotification.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

struct CallbackAnimationNotification::CallbackInfo
{
public:

	CallbackInfo(Callback callback, void *context);

	bool operator ==(CallbackInfo const &rhs) const;

public:

	Callback  m_callback;
	void     *m_context;

private:

	// Disabled.
	CallbackInfo();

};

// ======================================================================

inline CallbackAnimationNotification::CallbackInfo::CallbackInfo(Callback callback, void *context):
	m_callback(callback),
	m_context(context)
{
}

// ----------------------------------------------------------------------

inline bool CallbackAnimationNotification::CallbackInfo::operator ==(CallbackInfo const &rhs) const
{
	return (rhs.m_callback == m_callback) && (rhs.m_context == m_context);
}

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(CallbackAnimationNotification, true, 0, 0, 0);

// ======================================================================

CallbackAnimationNotification::CallbackAnimationNotification():
	AnimationNotification(),
	m_callbackInfoVector()
{
}

// ----------------------------------------------------------------------

void CallbackAnimationNotification::addCallback(Callback callback, void *context)
{
	DEBUG_FATAL(std::find(m_callbackInfoVector.begin(), m_callbackInfoVector.end(), CallbackInfo(callback, context)) != m_callbackInfoVector.end(), ("CallbackAnimationNotification: tried to add identical entry multiple times, illegal."));
	NOT_NULL(callback);

	m_callbackInfoVector.push_back(CallbackInfo(callback, context));
}

// ----------------------------------------------------------------------

void CallbackAnimationNotification::removeCallback(Callback callback, void *context)
{
	CallbackInfoVector::iterator findIt = std::find(m_callbackInfoVector.begin(), m_callbackInfoVector.end(), CallbackInfo(callback, context));
	DEBUG_FATAL(findIt == m_callbackInfoVector.end(), ("CallbackAnimationNotification: tried to remove callback that was already removed or was never added."));

	IGNORE_RETURN(m_callbackInfoVector.erase(findIt));
}

// ----------------------------------------------------------------------

void CallbackAnimationNotification::handleAnimationMessage(int channel, CrcString const &animationMessage, float elapsedTimeSinceTrigger)
{
	CallbackInfoVector::iterator endIt = m_callbackInfoVector.end();
	for (CallbackInfoVector::iterator it = m_callbackInfoVector.begin(); it != endIt; ++it)
	{
		CallbackInfo &callbackInfo = *it;
		(*callbackInfo.m_callback)(callbackInfo.m_context, channel, animationMessage, elapsedTimeSinceTrigger);
	}
}

// ======================================================================
