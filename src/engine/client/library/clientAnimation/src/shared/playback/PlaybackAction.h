// ======================================================================
//
// PlaybackAction.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_PlaybackAction_H
#define INCLUDE_PlaybackAction_H

// ======================================================================

class PlaybackActionTemplate;
class PlaybackScript;

// ======================================================================

class PlaybackAction
{
friend class PlaybackActionTemplate;

public:

	virtual ~PlaybackAction();

	virtual void                  notifyActorAdded(PlaybackScript &playbackScript, int actorIndex);
	virtual void                  notifyRemovingActor(PlaybackScript &playbackScript, int actorIndex);
	virtual void                  cleanup(PlaybackScript &playbackScript);

	virtual bool                  update(float deltaTime, PlaybackScript &script) = 0;
	virtual void                  stop(PlaybackScript &script);

	virtual float                 getMaxReasonableUpdateTime() const;

	const PlaybackActionTemplate *getPlaybackActionTemplate() const;

protected:

	explicit PlaybackAction(const PlaybackActionTemplate *playbackActionTemplate);

	// disabled
	PlaybackAction();
	PlaybackAction(const PlaybackAction&);
	PlaybackAction &operator =(const PlaybackAction&);

private:

	const PlaybackActionTemplate *m_playbackActionTemplate;

};

// ======================================================================

inline const PlaybackActionTemplate *PlaybackAction::getPlaybackActionTemplate() const
{
	return m_playbackActionTemplate;
}

// ======================================================================

#endif
