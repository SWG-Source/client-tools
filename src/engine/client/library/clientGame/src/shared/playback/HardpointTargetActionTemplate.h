// PRIVATE

// ======================================================================
//
// HardpointTargetActionTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HardpointTargetActionTemplate_H
#define INCLUDED_HardpointTargetActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;
class MemoryBlockManager;

// ======================================================================
/**
 * Provide a PlaybackActionTemplate to drive and set an animation target
 * to be used for simulating fire over the head of a defender.
 *
 * An action derived from this template is intended to be run in a separate
 * PlaybackScript thread.  It will stall (i.e. will not die) until the
 * given length of time elapses.
 */

class HardpointTargetActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int                     getAttackerActorIndex() const;
	int                     getDefenderActorIndex() const;
	Tag                     getHardpointNameVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit HardpointTargetActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	// Disabled.
	HardpointTargetActionTemplate();
	HardpointTargetActionTemplate(const HardpointTargetActionTemplate&);
	HardpointTargetActionTemplate &operator =(const HardpointTargetActionTemplate&);

private:

	int     m_attackerActorIndex;
	int     m_defenderActorIndex;
	Tag     m_hardpointNameVariable;

};

// ======================================================================

inline int HardpointTargetActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

inline int HardpointTargetActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

inline Tag HardpointTargetActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ======================================================================

#endif
