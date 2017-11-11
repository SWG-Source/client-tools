// ======================================================================
//
// FireTurretActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FireTurretActionTemplate_H
#define INCLUDED_FireTurretActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class Iff;
class ObjectTemplate;
class SkeletalAppearance2;

// ======================================================================
/**
 * An action for instructing a TurretObject to fire at a specified
 * target.
 */

class FireTurretActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	FireTurretActionTemplate(Iff &iff);

	void                           load_0000(Iff &iff);

	int                            getAttackerActorIndex() const;
	int                            getDefenderActorIndex() const;
	int                            getWatcherThreadIndex() const;
	int                            getWatcherActionIndex() const;
	Tag                            getIsHitVariable() const;

	// Disabled.
	FireTurretActionTemplate();
	FireTurretActionTemplate(const FireTurretActionTemplate&);
	FireTurretActionTemplate &operator =(const FireTurretActionTemplate&);

private:

	static bool  ms_installed;

private:

	int  m_attackerActorIndex;
	int  m_defenderActorIndex;

	int  m_watcherThreadIndex;
	int  m_watcherActionIndex;

	Tag  m_isHitVariable;

};

// ======================================================================

inline int FireTurretActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline int FireTurretActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

inline int FireTurretActionTemplate::getWatcherThreadIndex() const
{
	return m_watcherThreadIndex;
}

// ----------------------------------------------------------------------

inline int FireTurretActionTemplate::getWatcherActionIndex() const
{
	return m_watcherActionIndex;
}

// ----------------------------------------------------------------------

inline Tag FireTurretActionTemplate::getIsHitVariable() const
{
	return m_isHitVariable;
}

// ======================================================================

#endif
