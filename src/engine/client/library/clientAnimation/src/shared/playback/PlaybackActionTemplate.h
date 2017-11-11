// ======================================================================
//
// PlaybackActionTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_PlaybackActionTemplate_H
#define INCLUDE_PlaybackActionTemplate_H

// ======================================================================

class PlaybackAction;
class PlaybackScript;

// ======================================================================

class PlaybackActionTemplate
{
public:

	virtual PlaybackAction *createPlaybackAction() const = 0;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;
	virtual void            stop(PlaybackScript &script) const;

	void                    fetch() const;
	void                    release() const;

protected:

	PlaybackActionTemplate();
	virtual ~PlaybackActionTemplate();

private:

	// Disabled.
	PlaybackActionTemplate(const PlaybackActionTemplate&);
	PlaybackActionTemplate &operator =(const PlaybackActionTemplate&);

private:

	mutable int  m_referenceCount;

};

// ======================================================================

inline void PlaybackActionTemplate::fetch() const
{
	++m_referenceCount;
}

// ======================================================================

#endif
