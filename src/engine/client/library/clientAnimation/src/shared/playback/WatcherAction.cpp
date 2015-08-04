// ======================================================================
//
// WatcherAction.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/WatcherAction.h"
#include "clientAnimation/WatcherActionTemplate.h"

#include "sharedObject/Object.h"
#include "sharedFoundation/Watcher.h"

// ======================================================================
// public member functions
// ======================================================================

WatcherAction::WatcherAction() :
	PlaybackAction(0),
	m_watcher(0)
{
}

// ----------------------------------------------------------------------

WatcherAction::~WatcherAction()
{
	delete m_watcher;
}

// ----------------------------------------------------------------------

bool WatcherAction::update(float /* deltaTime */, PlaybackScript & /* script */)
{
	//-- ensure watcher has been set
	//-- stall if watch object is still alive
	// @todo -TRF- revisit
	return m_watcher && m_watcher->getPointer();
}

// ----------------------------------------------------------------------

void WatcherAction::setWatcherObject(Object *object)
{
	if (!m_watcher)
	{
		// create the watcher for the object
		m_watcher = new Watcher<Object>(object);
	}
	else
	{
		// assign object to the watcher
		*m_watcher = object;
	}
}

// ----------------------------------------------------------------------

Object *WatcherAction::getWatcherObject() const
{
	//-- ensure watcher has been set
	NOT_NULL(m_watcher);

	return m_watcher->getPointer();
}

// ======================================================================
// private member functions
// ======================================================================

WatcherAction::WatcherAction(const WatcherActionTemplate &watcherTriggerTemplate)
:	PlaybackAction(&watcherTriggerTemplate),
	m_watcher(0)
{
}

// ======================================================================
