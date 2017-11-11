// ======================================================================
//
// PlayParticleSystemAction.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PlayParticleSystemAction_H
#define INCLUDED_PlayParticleSystemAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

#include "clientAudio/SoundId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

class CellObject;
class PlaybackScript;
class PlayParticleSystemActionTemplate;

// ======================================================================

class PlayParticleSystemAction: public PlaybackAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	friend class PlayParticleSystemActionTemplate;

public:

	virtual bool  update(float deltaTime, PlaybackScript &script);

	virtual void  cleanup(PlaybackScript &playbackScript);
	virtual void  stop(PlaybackScript &script);

private:

	explicit PlayParticleSystemAction(PlayParticleSystemActionTemplate const &actionTemplate);

	PlayParticleSystemActionTemplate const &getActionTemplate() const;

	bool  startup(PlaybackScript &script);
	void  shutdown(PlaybackScript const &script);

	Object       *getEmitterObject(PlaybackScript &script) const;
	Object const *getEmitterConstObject(PlaybackScript const &script) const;
	float         getTimeDuration(PlaybackScript const &script) const;
	std::string   getAppearanceFilename(PlaybackScript const &script) const;
	std::string   getHardpointName(PlaybackScript const &script) const;

	std::string   getBeginSoundName(PlaybackScript const &script) const;
	std::string   getLoopSoundName(PlaybackScript const &script) const;
	std::string   getEndSoundName(PlaybackScript const &script) const;

	Vector        getTargetLocation(PlaybackScript const &script) const;
	const CellObject *getTargetCell(PlaybackScript const &script) const;

	// Disabled.
	PlayParticleSystemAction();
	PlayParticleSystemAction(PlayParticleSystemAction const &);
	PlayParticleSystemAction &operator =(PlayParticleSystemAction const &);

private:

	void          internalPlayLoopingSound(const char *soundName, Object *obj, const char *hardpointName);
	void          internalStopLoopingSound(const char *soundName);
	bool          internalGetSoundIsLooping(const char *soundName);

	Timer            m_startupWaitTimer;

	bool             m_initialized; /// Set true the first time through update.
	Timer            m_timer;       /// Timer: when this expires, the action should be shut down.
	Watcher<Object>  m_object;      /// Object watcher for the particle system hardpoint object.
	
	bool             m_useLocation;
	Vector           m_emitterLocation;
};

// ======================================================================

#endif
