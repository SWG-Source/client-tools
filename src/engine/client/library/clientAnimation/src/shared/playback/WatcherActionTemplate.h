// ======================================================================
//
// WatcherActionTemplate.h
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_WatcherActionTemplate_H
#define INCLUDED_WatcherActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"

class Iff;

// ======================================================================
/**
 * Provides a PlaybackScript system trigger template that is signaled
 * when a specified Object dies.
 */

class WatcherActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

private:

	static void remove();

	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit WatcherActionTemplate(Iff &iff);

	// disabled
	WatcherActionTemplate();
	WatcherActionTemplate(const WatcherActionTemplate&);
	WatcherActionTemplate &operator =(const WatcherActionTemplate&);

private:

#ifdef _DEBUG
	static bool  ms_installed;
#endif

};

// ======================================================================

#endif
