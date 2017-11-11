// ============================================================================
//
// PlayerMusicManager_Musician.h
// Copyright Sony Online Entertainment Inc.
// 
// ============================================================================

#ifndef INCLUDED_PlayerMusicManager_Musician_H
#define INCLUDED_PlayerMusicManager_Musician_H

#include "clientAudio/SoundId.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerMusicManager_ParticleSystem.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class CreatureObject;

//-------------------------------------------------------------------------
class PlayerMusicManager::Musician
{
public:

	Musician();
	explicit Musician(CreatureObject *creatureObject);
	~Musician();

	void alter(float const deltaTime);
	void nextFrame(bool const lowerVolume);
	void startPlaying();
	void addFlourish(int const flourishIndex);
	void startParticleSystem(std::string const &path);

	CreatureObject const *getCreatureObject() const;
	int                   getSampleTime() const;
	SoundId const &       getSoundId() const;
	int                   getQueuedFlourishCount() const;
	int                   getSongPlaying() const;
	float                 getTimeSinceFlourish() const;
	int                   getConsecutiveFlourishes() const;

	bool                  isAboutToPerformFlourish() const;
	bool                  isPerforming() const;
	bool                  isStartingPerformance() const;
	bool                  isDonePerforming() const;
	bool                  isParticleSystemEnabled() const;
	bool                  isPlayingOutro() const;

private:

	typedef stdlist<int>::fwd Flourishes;

	Watcher<CreatureObject>            m_creatureObject;
	Flourishes *                       m_queudFlourishes;
	SoundId                            m_soundId;
	bool                               m_playingIntro;
	bool                               m_playingOutro;
	float                              m_timeSinceFlourish;
	int                                m_consecutiveFlourishCount;
	PlayerMusicManager::ParticleSystem m_particleSystem1;
	PlayerMusicManager::ParticleSystem m_particleSystem2;

	void stopPlaying();
};

// ============================================================================

#endif // INCLUDED_PlayerMusicManager_Musician_H
