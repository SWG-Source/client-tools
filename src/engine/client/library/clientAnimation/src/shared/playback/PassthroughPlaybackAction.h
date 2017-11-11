// ======================================================================
//
// PassthroughPlaybackAction.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDE_PassthroughPlaybackAction_H
#define INCLUDE_PassthroughPlaybackAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class PlaybackActionTemplate;

// ======================================================================
/**
 * This PlaybackAction passes through all calls to its associated
 * PlaybackActionTemplate.
 *
 * If your PlaybackActionTemplate is capable of handling all
 * action activity, then use PassthroughPlaybackAction
 * for the associated PlaybackAction.  You'll be able to do this
 * when your action doesn't need to store per-PlaybackScript data.
 */

class PassthroughPlaybackAction: public PlaybackAction
{
public:

	PassthroughPlaybackAction(const PlaybackActionTemplate &playbackActionTemplate);
	virtual ~PassthroughPlaybackAction();

	virtual bool update(float deltaTime, PlaybackScript &script);
	
};

// ======================================================================

#endif
