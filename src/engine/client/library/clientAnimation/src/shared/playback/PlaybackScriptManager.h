// ======================================================================
//
// PlaybackScriptManager.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PlaybackScriptManager_H
#define INCLUDED_PlaybackScriptManager_H

// ======================================================================

class Object;
class PlaybackScript;

// ======================================================================

class PlaybackScriptManager
{
public:

	typedef stdvector<const PlaybackScript*>::fwd  ConstPlaybackScriptVector;

public:

	static void                  install();
	static void                  cleanup();

	static void                  update(float deltaTime);

	static void                  addPlaybackScript(PlaybackScript *playbackScript);
	static void                  killPlaybackScript(const PlaybackScript *playbackScript);

	static int                   getPlaybackScriptCount();
	static const PlaybackScript *getPlaybackScript(int index);

	static void                  getPlaybackScriptsForActor(const Object *object, ConstPlaybackScriptVector &playbackScripts);
	static const PlaybackScript *getPlaybackScriptForActorAtIndex(int actorIndex, const Object *object);

private:

	typedef stdvector<PlaybackScript*>::fwd  PlaybackScriptVector;

private:

	static void                  remove();

private:

	// disabled
	PlaybackScriptManager();

private:

	static PlaybackScriptVector  ms_playbackScripts;

#ifdef _DEBUG
	static bool                  ms_installed;
#endif

};

// ======================================================================

#endif
