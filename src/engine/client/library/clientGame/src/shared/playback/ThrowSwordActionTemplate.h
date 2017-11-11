// PRIVATE

// ======================================================================
//
// ThrowSwordActionTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ThrowSwordActionTemplate_H
#define INCLUDED_ThrowSwordActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class Iff;
class MemoryBlockManager;

// ======================================================================

class ThrowSwordActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int  getAttackerActorIndex() const;
	int  getDefenderActorIndex() const;

	Tag  getHardpointNameVariable() const;
	Tag  getYawRateVariable() const;
	Tag  getThrowSpeedVariable() const;

	Tag  getDamageVariable() const;
	Tag  getMissExtentMultiplierVariable() const;
	Tag  getHitExtentMultiplierVariable() const;
	Tag  getHitEventNameVariable() const;
	Tag  getHitDefenderActionNameVariable() const;

	Tag  getCatchTimeDurationVariable() const;
	Tag  getCatchSwordActionNameVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit ThrowSwordActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	// Disabled.
	ThrowSwordActionTemplate();
	ThrowSwordActionTemplate(const ThrowSwordActionTemplate&);
	ThrowSwordActionTemplate &operator =(const ThrowSwordActionTemplate&);

private:

	int  m_attackerActorIndex;
	int  m_defenderActorIndex;

	Tag  m_hardpointNameVariable;
	Tag  m_yawRateVariable;
	Tag  m_throwSpeedVariable;

	Tag  m_damageVariable;
	Tag  m_missExtentMultiplierVariable;
	Tag  m_hitExtentMultiplierVariable;
	Tag  m_hitEventNameVariable;
	Tag  m_hitDefenderActionNameVariable;

	Tag  m_catchTimeDurationVariable;
	Tag  m_catchSwordActionNameVariable;

};

// ======================================================================

#endif
