// ======================================================================
//
// WatcherAction.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_WatcherAction_H
#define INCLUDED_WatcherAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class Object;
class WatcherActionTemplate;

template <typename T>
class Watcher;

// ======================================================================

class WatcherAction: public PlaybackAction
{
friend class WatcherActionTemplate;

public:

	WatcherAction();
	virtual ~WatcherAction();

	virtual bool  update(float deltaTime, PlaybackScript &script);

	void          setWatcherObject(Object *object);
	Object       *getWatcherObject() const;

private:

	explicit WatcherAction(const WatcherActionTemplate &watcherTriggerTemplate);

	// Disabled.
	WatcherAction(const WatcherAction&);
	WatcherAction &operator =(const WatcherAction&);

private:

	Watcher<Object> *m_watcher;

};

// ======================================================================

#endif
