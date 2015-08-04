// ======================================================================
//
// UseLightningAction.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_UseLightningAction_H
#define INCLUDED_UseLightningAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "clientAudio/SoundId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

#include <vector>

class AppearanceTemplate;
class PlaybackScript;
class UseLightningActionTemplate;

// ======================================================================

class UseLightningAction: public PlaybackAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;
	friend class UseLightningActionTemplate;

public:

	static void install();

public:

	virtual ~UseLightningAction();

	virtual void cleanup(PlaybackScript &script);
	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	static void remove();

private:

	explicit UseLightningAction(UseLightningActionTemplate const &actionTemplate);

	UseLightningActionTemplate const &getOurActionTemplate() const;

	void realCleanup();
	bool initialize(PlaybackScript &script);
	bool updateBolt(int boltIndex, float startFraction, float endFraction);

	// Disabled.
	UseLightningAction();
	UseLightningAction(UseLightningAction const&);
	UseLightningAction &operator =(UseLightningAction const&);

private:

	typedef std::vector<float>             FloatVector;

private:

	bool   m_started;

	float  m_currentTime;
	float  m_totalTime;

	float  m_growEndTime;
	float  m_deathStartTime;

	int                  m_level;

	ConstWatcher<Object> m_attackerWatcher;
	ConstWatcher<Object> m_defenderWatcher;

	int                  m_hardpointCount;
	int                  m_boltCount;
	TemporaryCrcString   m_emitterHardpointNameArray[2];
	Watcher<Object>      m_boltObject;

	float                m_boltRotationRadius;
	FloatVector          m_boltRadianAngleOffsetArray;
	float                m_boltBaseRotationInRadians;
	float                m_rotationRateInRadiansPerSecond;

	SoundId              m_loopSoundId;

	AppearanceTemplate const *m_appearanceTemplate;

};

// ======================================================================

#endif
