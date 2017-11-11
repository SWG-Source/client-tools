// ======================================================================
//
// PriorityPlaybackScriptManager.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PriorityPlaybackScriptManager_H
#define INCLUDED_PriorityPlaybackScriptManager_H

// ======================================================================

class Object;
class PlaybackScript;

// ======================================================================
/**
 * This manager builds on top of the PlaybackScriptManager to provide
 * managing the playback scripts in which an actor is active or inactive.
 *
 * An actor is never active in more than one PlaybackScript at any given
 * time.  An actor is set active in a script if the actor has the highest
 * priority in the given script when it is submitted for playback.  Once
 * an actor is set inactive for a script, the actor will never be switched
 * over to active even when all higher priority scripts terminate.  An
 * actor can be transitioned from active to inactive if another script
 * with higher-priority is started involving the actor.
 *
 * Later on this class may queue up PlaybackScript instances that are
 * played on the same two actors if both actors are active in the same
 * script at the current time.  This could help smooth out network
 * latency-induced issues.
 */

class PriorityPlaybackScriptManager
{
public:

	typedef stdvector<int>::fwd  IntVector;

public:

	static bool submitPlaybackScript(PlaybackScript *playbackScript, bool alwaysPlayScript = false);
	static void	setPlaybackPriorityIgnore(bool ignorePriority);

private:

	typedef stdvector<const Object*>::fwd          ObjectVector;
	typedef stdvector<const PlaybackScript*>::fwd  PlaybackScriptVector;

private:

	static void        getPlaybackScriptActors(const PlaybackScript &playbackScript, ObjectVector &actorObjects);
	static void        getPlaybackScriptsWithActiveActors(const ObjectVector &actorObjects, PlaybackScriptVector &playbackScripts);
	static int         getMaxPlaybackScriptPriority(const PlaybackScriptVector &playbackScripts);

private:

	// Disabled.
	PriorityPlaybackScriptManager();

private:

	static ObjectVector          ms_actorObjects;
	static PlaybackScriptVector  ms_playbackScripts;

};

// ======================================================================

#endif
