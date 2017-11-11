// ======================================================================
//
// PlaybackAction.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackAction.h"

#include "clientAnimation/PlaybackActionTemplate.h"

// ======================================================================
// public member functions
// ======================================================================

PlaybackAction::~PlaybackAction()
{
	if (m_playbackActionTemplate)
	{
		m_playbackActionTemplate->release();
		m_playbackActionTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void PlaybackAction::notifyActorAdded(PlaybackScript& /* playbackScript */, int actorIndex)
{
	// default implementation: do nothing.
	UNREF(actorIndex);
}

// ----------------------------------------------------------------------

void PlaybackAction::notifyRemovingActor(PlaybackScript& /* playbackScript */, int actorIndex)
{
	// default implementation: do nothing.
	UNREF(actorIndex);
}

// ----------------------------------------------------------------------
/**
 * Called when a PlaybackScript is about to delete the action, prior to
 * calling notifyRemovingActor().
 *
 * Derived class implementation note: this function does not perform
 * any functionality.  Do not bother chaining to this function in a derived
 * class implementation.
 *
 * @param playbackScript  the PlaybackScript instance to which this Action was bound.
 */

void PlaybackAction::cleanup(PlaybackScript & /* playbackScript */)
{
}

// ----------------------------------------------------------------------
/**
 * Call this function when an active PlaybackAction-derived instance should
 * stop.
 *
 * During this function call, any state associated with the running action
 * can be released.  Also, calling this function should ensure that the next
 * call of update() returns false.  I plan on having a PlaybackScript::stop(threadIndex, actionIndex)
 * function that also ensures we don't run the update at all after this call,
 * but ensuring that stop() forces update() to return false next is the safest
 * thing to do.
 *
 * Derived class implementation note: this function chains the stop() call
 * to the PlaybackActionTemplate-derived class.  Only call this if you
 * want such behavior; otherwise, it's a waste as things currently stand.
 *
 * @param script  the PlaybackScript instance to which this Action was bound.
 */

void PlaybackAction::stop(PlaybackScript &script)
{
	// Default implementation passes the stop call to the playback action template.
	PlaybackActionTemplate const *const actionTemplate = getPlaybackActionTemplate();
	if (actionTemplate)
		actionTemplate->stop(script);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the maximum accumulated update time this action can
 * consume before the PlaybackScript system should assume this action
 * is hanging/stuck/needs to be stopped.
 *
 * Implementer Note: derived classes should make this number as tight as
 * reasonable so that lock ups are detected and dealt with as swiftly as
 * possible.  Update time is only accumulated for an action if it requires
 * more than a single update() call; therefore, all PlaybackAction instances
 * will receive at least one update() call before this return value can
 * cause a lock-up detection.
 *
 * @return  number of seconds this script can consume in update before
 *          the PlaybackScript system assumes the action is locked up.
 */

float PlaybackAction::getMaxReasonableUpdateTime() const
{
	return 5.0f;
}

// ======================================================================
// protected member functions
// ======================================================================

PlaybackAction::PlaybackAction(const PlaybackActionTemplate *playbackActionTemplate) :
	m_playbackActionTemplate(playbackActionTemplate)
{
	if (m_playbackActionTemplate)
	{
		//-- Keep local reference.
		m_playbackActionTemplate->fetch();
	}
}

// ======================================================================
