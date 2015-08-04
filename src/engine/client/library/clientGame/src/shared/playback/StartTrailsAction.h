// ======================================================================
//
// StartTrailsAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StartTrailsAction_H
#define INCLUDED_StartTrailsAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class Object;
class StartTrailsActionTemplate;
class VectorArgb;

template<class T>
class Watcher;

// ======================================================================

class StartTrailsAction: public PlaybackAction
{
friend class StartTrailsActionTemplate;

public:

	static void install();

	virtual ~StartTrailsAction();

	virtual void  cleanup(PlaybackScript &playbackScript);
	virtual bool  update(float deltaTime, PlaybackScript &script);

	void          killTrailAppearances();

private:

	typedef Watcher<Object>                 ObjectWatcher;
	typedef stdvector<ObjectWatcher*>::fwd  ObjectWatcherVector;

private:

	static void createTrailObjects(uint32 trailBitfield, ObjectWatcherVector &objectWatcherVector, Object &parentObject, const std::string &swooshFilename, VectorArgb const &color);

private:

	StartTrailsAction(const StartTrailsActionTemplate &actionTemplate);

	StartTrailsActionTemplate const &getOurActionTemplate() const;
	void                             getSwooshFilename(PlaybackScript const &script, std::string swooshFilename);

	// Disabled.
	StartTrailsAction();
	StartTrailsAction(const StartTrailsAction&);
	StartTrailsAction &operator =(const StartTrailsAction&);

private:

	ObjectWatcherVector *m_objectWatcherVector;

};

// ======================================================================

#endif
