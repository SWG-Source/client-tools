// ======================================================================
//
// HomingTargetActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HomingTargetActionTemplate_H
#define INCLUDED_HomingTargetActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================
/**
 * Provide a PlaybackActionTemplate to drive and set an animation target
 * to be used for simulating firing closer and closer to the defender
 * along a line from the attacker to the defender.
 *
 * An action derived from this template is intended to be run in a separate
 * PlaybackScript thread.  It will stall (i.e. will not die) until the
 * given length of time elapses.
 *
 * The position of the target will travel along a line starting at
 * a start fraction along the line from attacker to defender.  It will
 * progress toward the end fraction along the line between attacker and
 * defender, over the specified time interval.
 */

class HomingTargetActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	Tag                     getTravelTimeVariable() const;
	Tag                     getStartDistanceFractionVariable() const;
	Tag                     getEndDistanceFractionVariable() const;

	int                     getAttackerActorIndex() const;
	int                     getDefenderActorIndex() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit HomingTargetActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	// Disabled.
	HomingTargetActionTemplate();
	HomingTargetActionTemplate(const HomingTargetActionTemplate&);
	HomingTargetActionTemplate &operator =(const HomingTargetActionTemplate&);

private:

	static bool  ms_installed;

private:

	Tag     m_travelTimeVariable;
	Tag     m_startDistanceFractionVariable;
	Tag     m_endDistanceFractionVariable;

	int     m_attackerActorIndex;
	int     m_defenderActorIndex;

};

// ======================================================================

inline Tag HomingTargetActionTemplate::getTravelTimeVariable() const
{
	return m_travelTimeVariable;
}

// ----------------------------------------------------------------------

inline Tag HomingTargetActionTemplate::getStartDistanceFractionVariable() const
{
	return m_startDistanceFractionVariable;
}

// ----------------------------------------------------------------------

inline Tag HomingTargetActionTemplate::getEndDistanceFractionVariable() const
{
	return m_endDistanceFractionVariable;
}

// ----------------------------------------------------------------------

inline int HomingTargetActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline int HomingTargetActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ======================================================================

#endif
