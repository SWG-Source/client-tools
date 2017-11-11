// ======================================================================
//
// GrenadeLobActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_GrenadeLobActionTemplate_H
#define INCLUDED_GrenadeLobActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class Iff;
class ObjectTemplate;
class SkeletalAppearance2;

// ======================================================================
/**
 * An action that will take the attacker weapon Object, clone it,
 * and stick the cloned version on the specified hardpoint of the
 * attacker.
 */

class GrenadeLobActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	bool                    getHardpointName(const PlaybackScript &script, CrcLowerString &name) const;

	int                     getAttackerActorIndex() const;
	int                     getDefenderActorIndex() const;
	Tag                     getHardpointNameVariable() const;
	Tag                     getGravitationalForceVariable() const;
	Tag                     getAirTimeVariable() const;
	Tag                     getSitTimeVariable() const;
	Tag                     getIsHitVariable() const;
	Tag                     getThrowAtCenterVariable() const;
	Tag                     getTrackTargetVariable() const;

	int                     getThrowAnimationThreadIndex() const;
	int                     getThrowAnimationActionIndex() const;
	Tag                     getThrowAnimationMessageNameVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	GrenadeLobActionTemplate(Iff &iff);

	void                  load_0000(Iff &iff);
	void                  load_0001(Iff &iff);
	void                  load_0002(Iff &iff);
	void                  load_0003(Iff &iff);

	// Disabled.
	GrenadeLobActionTemplate();
	GrenadeLobActionTemplate(const GrenadeLobActionTemplate&);
	GrenadeLobActionTemplate &operator =(const GrenadeLobActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object that is holding the grenade object (the attacker).
	int  m_attackerActorIndex;

	/// Actor index for the object that is being targeted for a hit (the defender).
	int  m_defenderActorIndex;

	/// Variable (string) containing the name of the hardpoint where the grenade object is attached on the specified attacker object.
	Tag  m_hardpointNameVariable;

	/// Variable (float) containing the gravitational force (in m/s^2) along the y axis, where +y is up (so this must be a negative number).
	Tag  m_gravitationalForceVariable;

	/// Variable (float) containing the duration of time for the grenade to lob through the air.
	Tag  m_airTimeVariable;

	/// Variable (float) containing the duration of time during which the grenade will sit on the ground after landing before the grenade explosion client effect goes off.
	Tag  m_sitTimeVariable;

	// Variable (int) containing a number > 0 if hit, <= 0 if missed.
	Tag  m_isHitVariable;

	/// Variable (int) indicating if the throw should go to the center of the object (1) or the ground at its feet(0)
	Tag m_throwAtCenterVariable;

	// Variable (int) indicating if the throw should follow the target object (1) or go to the point where it was when the action started
	Tag m_trackTargetVariable;

	int  m_throwAnimationThreadIndex;
	int  m_throwAnimationActionIndex;
	Tag  m_throwAnimationMessageNameVariable;
};

// ======================================================================

inline int GrenadeLobActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline int GrenadeLobActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getGravitationalForceVariable() const
{
	return m_gravitationalForceVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getAirTimeVariable() const
{
	return m_airTimeVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getSitTimeVariable() const
{
	return m_sitTimeVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getIsHitVariable() const
{
	return m_isHitVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getThrowAtCenterVariable() const
{
	return m_throwAtCenterVariable;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getTrackTargetVariable() const
{
	return m_trackTargetVariable;
}

// ----------------------------------------------------------------------

inline int GrenadeLobActionTemplate::getThrowAnimationThreadIndex() const
{
	return m_throwAnimationThreadIndex;
}

// ----------------------------------------------------------------------

inline int GrenadeLobActionTemplate::getThrowAnimationActionIndex() const
{
	return m_throwAnimationActionIndex;
}

// ----------------------------------------------------------------------

inline Tag GrenadeLobActionTemplate::getThrowAnimationMessageNameVariable() const
{
	return m_throwAnimationMessageNameVariable;
}

// ======================================================================

#endif
