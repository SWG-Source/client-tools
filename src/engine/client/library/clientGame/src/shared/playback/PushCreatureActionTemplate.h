// ======================================================================
//
// PushCreatureActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PushCreatureActionTemplate_H
#define INCLUDED_PushCreatureActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class PushCreatureActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int  getDefenderActorIndex() const;
	int  getAttackerActorIndex() const;
	Tag  getPushDistanceVariable() const;
	Tag  getPushTimeVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	PushCreatureActionTemplate(Iff &iff);

	void load_0000(Iff &iff);

	// Disabled.
	PushCreatureActionTemplate();
	PushCreatureActionTemplate(const PushCreatureActionTemplate&);
	PushCreatureActionTemplate &operator =(const PushCreatureActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object to be pushed.
	int  m_defenderActorIndex;

	/// Actor index for the object that is doing the pushing.
	int  m_attackerActorIndex;

	/// The distance the defender will be pushed. [float variable]
	Tag  m_distanceVariable;

	/// The amount of time (in secs) the push should take. [float variable]
	Tag  m_timeVariable;

};

// ======================================================================

inline int PushCreatureActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

inline int PushCreatureActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline Tag PushCreatureActionTemplate::getPushDistanceVariable() const
{
	return m_distanceVariable;
}

// ----------------------------------------------------------------------

inline Tag PushCreatureActionTemplate::getPushTimeVariable() const
{
	return m_timeVariable;
}

// ======================================================================

#endif
