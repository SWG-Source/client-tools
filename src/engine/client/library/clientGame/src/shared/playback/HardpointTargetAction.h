// PRIVATE

// ======================================================================
//
// HardpointTargetAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class CrcString;
class CreatureController;
class Object;

// ======================================================================

class HardpointTargetAction: public PlaybackAction
{
	friend class HardpointTargetActionTemplate;
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	virtual bool update(float deltaTime, PlaybackScript &script);
	virtual void stop(PlaybackScript &script);
	virtual void cleanup(PlaybackScript &playbackScript);

private:

	explicit HardpointTargetAction(const HardpointTargetActionTemplate &actionTemplate);
	virtual ~HardpointTargetAction();

	const HardpointTargetActionTemplate *getOurTemplate() const;
	Object                              *getAttacker(PlaybackScript &script) const;
	Object                              *getDefender(PlaybackScript &script) const;
	CreatureController                  *getAttackerController(PlaybackScript &script) const;
	void                                 getHardpointName(const PlaybackScript &script, CrcString &hardpointName) const;

	Object                              *createTarget(PlaybackScript &script) const;
	void                                 killTarget();

private:

	HardpointTargetAction();
	HardpointTargetAction(const HardpointTargetAction&);
	HardpointTargetAction &operator =(const HardpointTargetAction&);

private:

	bool             m_targetCreated;
	Watcher<Object>  m_targetWatcher;

	bool             m_hasCreatureController;
};

// ======================================================================
