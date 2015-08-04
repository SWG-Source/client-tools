// ======================================================================
//
// AnimationNotification.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationNotification.h"

#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

#include <string>

// ======================================================================

#ifdef _DEBUG

#define VALIDATE_CHANNEL(channel) (s_log ? validateChannel(channel) : NOP)

#else

#define VALIDATE_CHANNEL(channel) NOP

#endif

// ======================================================================

namespace AnimationNotificationNamespace
{
	char const *const cs_stateNames[] =
		{
			"NS_noAnimationAttached",
			"NS_attached",
			"NS_inQueue",
			"NS_playInProgress",
			"NS_MARKER_noMorePlay",
			"NS_stoppedNormal",
			"NS_stoppedExplicitlyStopped",
			"NS_stoppedInterrupted",
			"NS_stoppedEjectedFromQueue",
			"NS_stoppedSameTrackTrumped",
			"NS_stoppedMisbehaving"
		};

	int const cs_stateNameCount = sizeof(cs_stateNames) / sizeof(cs_stateNames[0]);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_log;
}

using namespace AnimationNotificationNamespace;

// ======================================================================

class AnimationNotification::ChannelData
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit ChannelData(SkeletalAnimation *animation);
	        ~ChannelData();

	void     setControllerInfo(TrackAnimationController *controller, AnimationTrackId const &trackId);

	void     setState(NotificationState state);
	void     stopAnimation();

	SkeletalAnimation *fetchAnimation();
	NotificationState  getState() const;

	void               debugDump() const;

private:

	// Disabled.
	ChannelData();

private:

	SkeletalAnimation        *m_animation;
	NotificationState         m_state;
	TrackAnimationController *m_controller;
	AnimationTrackId          m_trackId;

};

// ======================================================================
// class AnimationNotification::ChannelData
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(AnimationNotification::ChannelData, true, 0, 0, 0);

// ======================================================================

AnimationNotification::ChannelData::ChannelData(SkeletalAnimation *animation):
	m_animation(animation),
	m_state((animation != NULL) ? NS_attached : NS_noAnimationAttached),
	m_controller(NULL),
	m_trackId()
{
	if (animation)
		animation->fetch();
}

// ----------------------------------------------------------------------

AnimationNotification::ChannelData::~ChannelData()
{
	m_controller = NULL;

	if (m_animation)
	{
		m_animation->setNotification(NULL, 0);
		m_animation->release();
		m_animation = NULL;
	}
}

// ----------------------------------------------------------------------

void AnimationNotification::ChannelData::setControllerInfo(TrackAnimationController *controller, AnimationTrackId const &trackId)
{
	FATAL(m_controller, ("AnimationNotification::ChannelData::setControllerInfo(): controller info already set, illegal."));
	FATAL(!controller, ("setControllerInfo(): controller arg is NULL"));
	FATAL(trackId == AnimationTrackId::cms_invalid, ("AnimationNotification::ChannelData::setControllerInfo(): invalid track id."));

	m_controller = controller;
	m_trackId    = trackId;
}

// ----------------------------------------------------------------------

void AnimationNotification::ChannelData::setState(NotificationState state)
{
	m_state = state;
}

// ----------------------------------------------------------------------

void AnimationNotification::ChannelData::stopAnimation()
{
	FATAL(!m_controller, ("AnimationNotification::ChannelData::stopAnimation(): controller info never set."));
	m_controller->stopAnimation(m_trackId);
}

// ----------------------------------------------------------------------

SkeletalAnimation *AnimationNotification::ChannelData::fetchAnimation()
{
	if (m_animation)
	{
		//-- Fetch reference for caller.
		m_animation->fetch();
	}

	return m_animation;
}

// ----------------------------------------------------------------------

inline AnimationNotification::NotificationState AnimationNotification::ChannelData::getState() const
{
	return m_state;
}

// ----------------------------------------------------------------------

void AnimationNotification::ChannelData::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("AN::CD:animation =[%p].\n", m_animation));
	DEBUG_REPORT_LOG(true, ("AN::CD:state     =[%s].\n", cs_stateNames[static_cast<int>(m_state)]));
	DEBUG_REPORT_LOG(true, ("AN::CD:controller=[%p].\n", m_controller));
	DEBUG_REPORT_LOG(true, ("AN::CD:track id  =[%d].\n", m_trackId.getTrackIdValue()));
}

// ======================================================================
// class AnimationNotification: PUBLIC STATIC
// ======================================================================

void AnimationNotification::install()
{
	ChannelData::install();

#ifdef _DEBUG
	DebugFlags::registerFlag(s_log, "ClientSkeletalAnimation", "logAnimationNotification");
#endif
}

// ----------------------------------------------------------------------

char const *AnimationNotification::getStateName(NotificationState state)
{
	ValidateRangeInclusiveExclusive(0, static_cast<int>(state), cs_stateNameCount, "getStateName(): state");
	return cs_stateNames[static_cast<int>(state)];
}

// ======================================================================
// class AnimationNotification: PUBLIC
// ======================================================================

AnimationNotification::AnimationNotification():
	m_referenceCount(0),
	m_channelDataVector(),
	m_ownerObject()
{
}

// ----------------------------------------------------------------------

void AnimationNotification::associateObject(Object *object)
{
	if (object == m_ownerObject)
		return;

	FATAL((object != NULL) && (m_ownerObject != NULL), ("AnimationNotification::associateObject(): associating new owner id=[%s],template=[%s]; old owner id=[%s],template=[%s].",
		object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName(),
		m_ownerObject->getNetworkId().getValueString().c_str(), m_ownerObject->getObjectTemplateName()));
	
	m_ownerObject = object;
}

// ----------------------------------------------------------------------

void AnimationNotification::fetch() const
{
	++m_referenceCount;
	FATAL(m_referenceCount <= 0, ("AnimationNotification::fetch(): bad reference count handling."));
}

// ----------------------------------------------------------------------

void AnimationNotification::release() const
{
	FATAL(m_referenceCount <= 0, ("AnimationNotification::release(): bad reference count handling, already deleted."));
	--m_referenceCount;
	if (m_referenceCount == 0)
		delete const_cast<AnimationNotification*>(this);
}

// ----------------------------------------------------------------------

int AnimationNotification::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

int AnimationNotification::getChannelCount() const
{
	return static_cast<int>(m_channelDataVector.size());
}

// ----------------------------------------------------------------------

AnimationNotification::NotificationState AnimationNotification::getState(int channel) const
{
	ChannelData const *channelData = getChannel(channel);
	if (channelData)
		return channelData->getState();
	else
	{
		DEBUG_WARNING(s_log, ("AnimationNotification::getState(%d): NULL channel.", channel));
		return NS_noAnimationAttached;
	}
}

// ----------------------------------------------------------------------

bool AnimationNotification::isAnimationDone(int channel) const
{
	int const stateIndex = static_cast<int>(getState(channel));
	return (stateIndex >= static_cast<int>(NS_MARKER_noMorePlay));
}

// ----------------------------------------------------------------------

void AnimationNotification::stopAnimation(int channel)
{
	if (m_ownerObject == NULL)
	{
		// Don't allow stopping animation for a deleted object.  If the owner object is deleted, the controller data is stale.
		return;
	}

	ChannelData *channelData = getChannel(channel);
	if (channelData)
		channelData->stopAnimation();
	else
		DEBUG_WARNING(s_log, ("AnimationNotification::stopAnimation(%d): NULL channel data.", channel));
}

// ----------------------------------------------------------------------

SkeletalAnimation *AnimationNotification::fetchAnimation(int channel)
{
	ChannelData *channelData = getChannel(channel);
	if (channelData)
	{
		SkeletalAnimation *result = channelData->fetchAnimation();
		if(!result)
		{
			DEBUG_WARNING(true, ("AN:fetchAnimation, channelData returned NULL, dump follows"));
			channelData->debugDump();
		}
		return result;
	}
	else
	{
		DEBUG_WARNING(s_log, ("AnimationNotification::fetchAnimation(%d): NULL channel data.", channel));
		return 0;
	}
	
}

// ----------------------------------------------------------------------

void AnimationNotification::handleAnimationMessage(int /* channel */, CrcString const & /* animationMessage */, float /* elapsedTimeSinceTrigger */)
{
	// Default implementation does nothing, no need to chain down.
}

// ======================================================================
// class AnimationNotification: PROTECTED
// ======================================================================

AnimationNotification::~AnimationNotification()
{
	ChannelDataVector::iterator const endIt = m_channelDataVector.end();
	for (ChannelDataVector::iterator it = m_channelDataVector.begin(); it != endIt; ++it)
		delete *it;

	ChannelDataVector().swap(m_channelDataVector);
}

// ======================================================================
// class AnimationNotification: PRIVATE
// ======================================================================

void AnimationNotification::setChannelCount(int channelCount)
{
	FATAL(!m_channelDataVector.empty(), ("AnimationNotification::setChannelCount() called multiple times, illegal."));
	FATAL(channelCount < 0, ("AnimationNotification::setChannelCount(): called with invalid channelCount [%d].", channelCount));

	m_channelDataVector.resize(static_cast<ChannelDataVector::size_type>(channelCount));
}

// ----------------------------------------------------------------------

void AnimationNotification::attachAnimation(int channel, SkeletalAnimation *animation)
{
	ValidateRangeInclusiveExclusive(0, channel, getChannelCount(), "attachAnimation(): channel");
	FATAL(m_channelDataVector[static_cast<ChannelDataVector::size_type>(channel)] != NULL, ("AnimationNotification::attachAnimation(): called multiple times for same channel, illegal."));

	m_channelDataVector[static_cast<ChannelDataVector::size_type>(channel)] = new ChannelData(animation);
}

// ----------------------------------------------------------------------

void AnimationNotification::setControllerInfo(int channel, TrackAnimationController *controller, AnimationTrackId const &trackId)
{
	ChannelData *channelData = getChannel(channel);

	if (channelData)
		channelData->setControllerInfo(controller, trackId);
	else
		FATAL(true, ("AnimationNotification::setControllerInfo(%d): NULL channelData.", channel));
}

// ----------------------------------------------------------------------

void AnimationNotification::setState(int channel, NotificationState state)
{
	ChannelData *channelData = getChannel(channel);
	if (channelData)
		channelData->setState(state);
	else
		FATAL(true, ("AnimationNotification::setState(%d): NULL channelData.", channel));
}

// ----------------------------------------------------------------------

AnimationNotification::ChannelData *AnimationNotification::getChannel(int index)
{
	FATAL((index < 0) || (index >= getChannelCount()),
		("AnimationNotification::getChannel(%d) called on object [%s], template [%s], valid range is [0..%d]",
		index,
		m_ownerObject.getPointer() ? m_ownerObject.getPointer()->getNetworkId().getValueString().c_str() : "<null owner object>",
		m_ownerObject.getPointer() ? m_ownerObject.getPointer()->getObjectTemplateName() : "<null owner object>"
		));

	ValidateRangeInclusiveExclusive(0, index, getChannelCount(), "getChannel(): index");

	//-- Return data if it exists; otherwise, create.
	ChannelData *channelData = m_channelDataVector[static_cast<ChannelDataVector::size_type>(index)];
	VALIDATE_CHANNEL(channelData);
	
	return channelData;
}

// ----------------------------------------------------------------------

AnimationNotification::ChannelData const *AnimationNotification::getChannel(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getChannelCount());

	//-- Return data if it exists; otherwise, create.
	ChannelData const *channelData = m_channelDataVector[static_cast<ChannelDataVector::size_type>(index)];
	VALIDATE_CHANNEL(channelData);
	
	return channelData;
}

// ----------------------------------------------------------------------

void AnimationNotification::validateChannel(ChannelData const *channel) const
{
	if (!channel)
	{
		DEBUG_WARNING(s_log, ("AnimationNotification::validateChannel(): channel not valid, dumping notification."));
		debugDump();
	}
}

// ----------------------------------------------------------------------

void AnimationNotification::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("AN:[BEGIN DUMP].\n"));
	DEBUG_REPORT_LOG(true, ("AN:reference count=[%d]\n", m_referenceCount));
	
	if (m_ownerObject != NULL)
		DEBUG_REPORT_LOG(true, ("AN:owner object id=[%s],template=[%s].\n", m_ownerObject->getNetworkId().getValueString().c_str(), m_ownerObject->getObjectTemplateName()));
	else
		DEBUG_REPORT_LOG(true, ("AN:owner object [NULL].\n"));

	int const channelCount = getChannelCount();
	DEBUG_REPORT_LOG(true, ("AN:channel count=[%d].\n", channelCount));

	for (int i = 0; i < channelCount; ++i)
	{
		ChannelData const *const channelData = m_channelDataVector[static_cast<ChannelDataVector::size_type>(i)];
		if (channelData)
		{
			DEBUG_REPORT_LOG(true, ("AN:channel [%d]: dump follows.\n", i));
			channelData->debugDump();
		}
		else
			DEBUG_REPORT_LOG(true, ("AN:channel [%d]: NULL.\n", i));
	}

	DEBUG_REPORT_LOG(true, ("AN:[END DUMP].\n"));
}

// ======================================================================
