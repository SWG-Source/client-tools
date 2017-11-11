// ======================================================================
//
// ArcTargetActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ArcTargetActionTemplate_H
#define INCLUDED_ArcTargetActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================
/**
 * Provide a PlaybackActionTemplate to drive and set an animation target
 * to be used for simulating fire over the head of a defender.
 *
 * An action derived from this template is intended to be run in a separate
 * PlaybackScript thread.  It will stall (i.e. will not die) until the
 * given length of time elapses.
 */

class ArcTargetActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	Tag                     getArcTimeVariable() const;
	Tag                     getArcDistanceMultipleVariable() const;
	Tag                     getArcHeightMultipleVariable() const;

	int                     getAttackerActorIndex() const;
	int                     getDefenderActorIndex() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit ArcTargetActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);
	void     load_0001(Iff &iff);

	// Disabled.
	ArcTargetActionTemplate();
	ArcTargetActionTemplate(const ArcTargetActionTemplate&);
	ArcTargetActionTemplate &operator =(const ArcTargetActionTemplate&);

private:

	static bool  ms_installed;

private:

	Tag     m_arcTimeVariable;
	Tag     m_distanceMultipleVariable;
	Tag     m_heightMultipleVariable;
	int     m_attackerActorIndex;
	int     m_defenderActorIndex;

};

// ======================================================================

inline Tag ArcTargetActionTemplate::getArcTimeVariable() const
{
	return m_arcTimeVariable;
}

// ----------------------------------------------------------------------

inline Tag ArcTargetActionTemplate::getArcDistanceMultipleVariable() const
{
	return m_distanceMultipleVariable;
}

// ----------------------------------------------------------------------

inline Tag ArcTargetActionTemplate::getArcHeightMultipleVariable() const
{
	return m_heightMultipleVariable;
}

// ----------------------------------------------------------------------

inline int ArcTargetActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline int ArcTargetActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ======================================================================

#endif
