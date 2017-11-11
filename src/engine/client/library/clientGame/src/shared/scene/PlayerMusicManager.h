// ============================================================================
//
// PlayerMusicManager.h
// Copyright Sony Online Entertainment Inc.
// 
// ============================================================================

#ifndef INCLUDED_PlayerMusicManager_H
#define INCLUDED_PlayerMusicManager_H

class CreatureObject;

//-----------------------------------------------------------------------------
class PlayerMusicManager
{
public:

	enum ParticleType
	{
		PT_musicLoopBoring,
		PT_musicLoopEntertaining,
		PT_musicPaused,
		PT_musicNotListening,
		PT_none
	};

public:

	static void        install();
	static void        alter(float const deltaTime);
	static void        queueFlourish(CreatureObject const *creatureObject, const int flourishIndex);
	static void        startPerformance(CreatureObject &musician);
	static void        stopPerformance(CreatureObject &musician);
	static void        checkStatus(CreatureObject const * const creatureObject);

	static std::string getMusicParticleSystemPath(ParticleType const particleType);
	static std::string getFlourishParticleSystemPath(int const skillMod, int const flourishCount);

	static std::string getIntroSoundPath(int const songIndex);
	static std::string getMainLoopSoundPath(int const songIndex);
	static std::string getFlourishSoundPath(const int songIndex, int const flourishIndex);
	static std::string getOutroSoundPath(int const songIndex);

	static int         getMaximumFlourishCount();
	static int         getInstrumentId(CreatureObject const * const creatureObject);

	class Musician;
	class ParticleSystem;

private:

	static void remove();

private:

	PlayerMusicManager();
	~PlayerMusicManager();
	PlayerMusicManager(PlayerMusicManager const &);
	PlayerMusicManager &operator =(PlayerMusicManager const &);
};

// ============================================================================

#endif // INCLUDED_PlayerMusicManager_H
