// PRIVATE

// ======================================================================
//
// ThrowSwordAction.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ThrowSwordAction_H
#define INCLUDED_ThrowSwordAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class MemoryBlockManager;
class Object;
class ThrowSwordActionTemplate;

// ======================================================================

class ThrowSwordAction: public PlaybackAction
{
	friend class ThrowSwordActionTemplate;
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	virtual bool                  update(float deltaTime, PlaybackScript &script);
	virtual void                  cleanup(PlaybackScript &script);

	virtual float                 getMaxReasonableUpdateTime() const;

private:

	explicit ThrowSwordAction(ThrowSwordActionTemplate const &actionTemplate);

	bool                          initializeScriptData(PlaybackScript &script);

	bool                          swordWithinDestinationRange() const;
	void                          onDestinationReached(PlaybackScript &script) const;

	bool                          swordWithinCatchRange() const;
	void                          playCatchAnimation(PlaybackScript &script) const;
	bool                          swordWithinAttachRange() const;

	// Disabled.
	ThrowSwordAction();
	ThrowSwordAction(const ThrowSwordAction&);
	ThrowSwordAction &operator =(const ThrowSwordAction&);

private:

	bool                m_initializedScriptData;
	bool                m_defenderIsHit;
	TemporaryCrcString  m_attackerHardpointName;

	bool                m_headingOutward;  // true if sword is traveling toward defender; false if coming back toward attacker.
	bool                m_playedCatchAnimation;

	Watcher<Object>     m_attackerChildWatcher;
	Watcher<Object>     m_defenderChildWatcher;
	Watcher<Object>     m_swordWatcher;

	float               m_destinationReachedDistanceSquared;
	float               m_playCatchAnimationDistanceSquared;
	float               m_destroySwordDistanceSquared;

};

// ======================================================================

#endif
