// ============================================================================
//
// PlayerMusicManager_Musician.cpp
// Copyright Sony Online Entertainment Inc.
// 
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerMusicManager_Musician.h"

#include "clientAudio/Audio.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/CrcLowerString.h"
#include "swgSharedUtility/Postures.h"

#include <list>

// ============================================================================
//
// PlayerMusicManager::Musician
//
// ============================================================================

//-----------------------------------------------------------------------------
PlayerMusicManager::Musician::Musician()
 : m_creatureObject(NULL)
 , m_particleSystem1()
 , m_particleSystem2()
 , m_queudFlourishes(NULL)
 , m_soundId()
 , m_playingIntro(false)
 , m_playingOutro(false)
 , m_timeSinceFlourish(0.0f)
 , m_consecutiveFlourishCount(0)
{
	m_queudFlourishes = new Flourishes;
}

//-----------------------------------------------------------------------------
PlayerMusicManager::Musician::Musician(CreatureObject *creatureObject)
 : m_creatureObject(creatureObject)
 , m_particleSystem1()
 , m_particleSystem2()
 , m_queudFlourishes(NULL)
 , m_soundId()
 , m_playingIntro(false)
 , m_playingOutro(false)
 , m_timeSinceFlourish(0.0f)
 , m_consecutiveFlourishCount(0)
{
	m_queudFlourishes = new Flourishes;
}

//-----------------------------------------------------------------------------
PlayerMusicManager::Musician::~Musician()
{
	Audio::stopSound(m_soundId);

	delete m_queudFlourishes;
	m_queudFlourishes = NULL;

	m_creatureObject = NULL;
	m_playingIntro = false;
	m_playingOutro = false;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::startParticleSystem(std::string const &path)
{
	if ((m_creatureObject != NULL) &&
	    !path.empty())
	{
		if (!m_particleSystem1.isEnabled() ||
		    (m_particleSystem1.getParticleSystemPath() == path))
		{
			m_particleSystem2.setEnabled(false);
			m_particleSystem1.create(m_creatureObject, path);
		}
		else if (!m_particleSystem2.isEnabled() ||
		         (m_particleSystem2.getParticleSystemPath() == path))
		{
			m_particleSystem1.setEnabled(false);
			m_particleSystem2.create(m_creatureObject, path);
		}
	}

	DEBUG_FATAL((m_particleSystem1.isEnabled() && m_particleSystem2.isEnabled()), ("Both player music particle systems should not be enabled."));
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::Musician::isParticleSystemEnabled() const
{
	bool result = false;

	if (m_particleSystem1.isEnabled() ||
	    m_particleSystem2.isEnabled())
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::startPlaying()
{
	if ((m_creatureObject == NULL) ||
	    isPlayingOutro())
	{
		return;
	}

	m_playingIntro = true;

	// Start the music particles

	startParticleSystem(PlayerMusicManager::getMusicParticleSystemPath(PlayerMusicManager::PT_musicLoopEntertaining));

	// Start the music playing

	if (Audio::isSoundValid(m_soundId))
	{
		Audio::setAutoDelete(m_soundId, true);
		Audio::stopSound(m_soundId);
	}

	std::string music(PlayerMusicManager::getIntroSoundPath(getSongPlaying()));

	if (!music.empty())
	{
		m_soundId = Audio::attachSound(music.c_str(), m_creatureObject);
		Audio::setAutoDelete(m_soundId, true);
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::alter(float const deltaTime)
{
	if (m_creatureObject->getPerformanceType() == 0)
	{
		m_playingOutro = true;
		stopPlaying();
	}

	if (getQueuedFlourishCount() <= 0)
	{
		m_timeSinceFlourish += deltaTime;
	}
	else
	{
		m_timeSinceFlourish = 0.0f;
	}

	if (isPlayingOutro() &&
	    !Audio::isSoundValid(m_soundId) &&
	    (m_particleSystem1.isEnabled() ||
	     m_particleSystem2.isEnabled()))
	{
		// If the player stopped playing their outro then shut off the
		// particles

		stopPlaying();
	}

	if (isPlayingOutro() &&
	    m_particleSystem1.isDeletable() &&
	    m_particleSystem2.isDeletable())
	{
		if ((m_creatureObject != NULL) &&
		    (m_creatureObject->getPerformanceType() == 0))
		{
			m_creatureObject = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::nextFrame(bool const lowerVolume)
{
	if ((m_creatureObject == NULL) ||
	    isPlayingOutro())
	{
		return;
	}

	if (m_queudFlourishes->size() > 0)
	{
		// Play the next flourish

		int const flourishIndex = *m_queudFlourishes->begin();

		m_queudFlourishes->pop_front();

		if (flourishIndex == -1)
		{
			if (Audio::isEnabled())
			{
				m_playingOutro = true;

				// We are playing the outro

				startParticleSystem(PlayerMusicManager::getMusicParticleSystemPath(PlayerMusicManager::PT_musicLoopEntertaining));
			
				// Stop the previous track
			
				Audio::setAutoDelete(m_soundId, true);
				Audio::stopSound(m_soundId);
			
				// Start the outro
			
				std::string music(PlayerMusicManager::getOutroSoundPath(getSongPlaying()));
			
				if (!music.empty())
				{
					m_soundId = Audio::attachSound(music.c_str(), m_creatureObject);
					Audio::setAutoDelete(m_soundId, true);
				}
			}
			else
			{
				stopPlaying();
			}
		}
		else if ( flourishIndex == 0)
		{
			// Pause for a frame

			if (--m_consecutiveFlourishCount < 1)
			{
				m_consecutiveFlourishCount = 1;
			}

			startParticleSystem(PlayerMusicManager::getMusicParticleSystemPath(PlayerMusicManager::PT_musicPaused));
			
			if(Audio::isEnabled())
			{

				// Stop the current sound

				Audio::setAutoDelete(m_soundId, true);
				Audio::stopSound(m_soundId);

				// Start a silent track

				std::string music(PlayerMusicManager::getMainLoopSoundPath(getSongPlaying()));

				if (!music.empty())
				{
					m_soundId = Audio::attachSound(music.c_str(), m_creatureObject);
					Audio::setSoundVolume(m_soundId, 0.0f);
				}
			}
		}
		else if (flourishIndex <= PlayerMusicManager::getMaximumFlourishCount())
		{
			++m_consecutiveFlourishCount;
			
			// Get the players skill in music

			CreatureObject::SkillModMap::const_iterator iterSkillModMap = m_creatureObject->getSkillModMap().find("healing_music_ability");
			int skillMod = 0;

			if (iterSkillModMap != m_creatureObject->getSkillModMap().end())
			{
				skillMod = iterSkillModMap->second.first + iterSkillModMap->second.second;
			}

			startParticleSystem(PlayerMusicManager::getFlourishParticleSystemPath(skillMod, m_consecutiveFlourishCount));

			// Play the flourish animation

			SkeletalAppearance2 *appearance = dynamic_cast<SkeletalAppearance2 *>(m_creatureObject->getAppearance());

			if (appearance != NULL)
			{
				char text[256];
				sprintf(text, "skill_action_%d", flourishIndex);

				int animationId;
				bool animationIsAdd;
				appearance->getAnimationResolver().playAction(CrcLowerString(text), animationId, animationIsAdd, NULL);
			}

			// Stop the current sound
			if(Audio::isEnabled())
			{
				Audio::setAutoDelete(m_soundId, true);
				Audio::stopSound(m_soundId);
	

				// Start the flourish music

				std::string music(PlayerMusicManager::getFlourishSoundPath(getSongPlaying(), flourishIndex));

				if (!music.empty())
				{
					m_soundId = Audio::attachSound(music.c_str(), m_creatureObject);
				}
			}
		}
	}
	else
	{
		bool const justFinishedFlourishes = (getQueuedFlourishCount() == 0);
		bool const justJoiningTheBand = !m_playingIntro;

		if (justFinishedFlourishes ||
	        m_playingIntro ||
			justJoiningTheBand)
		{
			if (--m_consecutiveFlourishCount < 1)
			{
				m_consecutiveFlourishCount = 1;
			}

			// Start the main loop

			m_playingIntro = false;

			// Stop the flourish particles

			if (m_timeSinceFlourish < 60.0f)
			{
				startParticleSystem(PlayerMusicManager::getMusicParticleSystemPath(PlayerMusicManager::PT_musicLoopEntertaining));
			}
			else
			{
				startParticleSystem(PlayerMusicManager::getMusicParticleSystemPath(PlayerMusicManager::PT_musicLoopBoring));
			}

			if(Audio::isEnabled())
			{
				// Start up the music loop

				if (Audio::isSoundValid(m_soundId))
				{
					Audio::setAutoDelete(m_soundId, true);
					Audio::stopSound(m_soundId);
				}

				std::string music(PlayerMusicManager::getMainLoopSoundPath(getSongPlaying()));

				if (!music.empty())
				{
					m_soundId = Audio::attachSound(music.c_str(), m_creatureObject);
					if (lowerVolume)
					{
						Audio::setSoundVolume(m_soundId, 0.8f);
					}
					Audio::setAutoDelete(m_soundId, false);
				}
			}
		}
	}

	Audio::setCurrentSoundTime(m_soundId, 0);
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::Musician::isAboutToPerformFlourish() const
{
	bool result = false;

	if ((m_queudFlourishes->size() > 0) &&
	    (*m_queudFlourishes->begin() > 0))
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::Musician::isPerforming() const
{
	bool result = false;

	if ((m_creatureObject != NULL) &&
	    (m_creatureObject->getPerformanceType() != 0))
	{
		result = true;
	}
	
	return result;
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::Musician::getQueuedFlourishCount() const
{
	return static_cast<int>(m_queudFlourishes->size());
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::Musician::getSongPlaying() const
{
	int result = m_creatureObject->getPerformanceType() - 1;

	DEBUG_FATAL((result < 0), ("Invalid song index: %d", result));

	return result;
}

//-----------------------------------------------------------------------------
float PlayerMusicManager::Musician::getTimeSinceFlourish() const
{
	return m_timeSinceFlourish;
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::Musician::getConsecutiveFlourishes() const
{
	return m_consecutiveFlourishCount;
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::Musician::isStartingPerformance() const
{
	bool result = false;

	if (!isPlayingOutro() &&
	    isPerforming() &&
	    !isParticleSystemEnabled())
	{
		const int instrumentAudioId = PlayerMusicManager::getInstrumentId(m_creatureObject);

		if (instrumentAudioId > 0)
		{
			result = true;
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::Musician::getSampleTime() const
{
	int milliSecond = 0;

	IGNORE_RETURN(Audio::getCurrentSoundTime(m_soundId, milliSecond));

	return milliSecond;
}

//-----------------------------------------------------------------------------
CreatureObject const *PlayerMusicManager::Musician::getCreatureObject() const
{
	return m_creatureObject;
}

//-----------------------------------------------------------------------------
SoundId const &PlayerMusicManager::Musician::getSoundId() const
{
	return m_soundId;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::addFlourish(int const flourishIndex)
{
	if (flourishIndex == -1)
	{
		m_queudFlourishes->clear();
	}

	if (!isPlayingOutro())
	{
		m_queudFlourishes->push_back(flourishIndex);
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::Musician::stopPlaying()
{
	m_playingOutro = true;

	// Stop all the music sounds

	Audio::setAutoDelete(m_soundId, true);
	Audio::stopSound(m_soundId);

	// Disable the particle effects

    m_particleSystem1.setAllowDelete(true);
    m_particleSystem2.setAllowDelete(true);
	m_particleSystem1.setEnabled(false);
	m_particleSystem2.setEnabled(false);

	// Clear the queued flourishes

	m_queudFlourishes->clear();

	if (m_creatureObject != NULL)
	{
		// Change the musicians posture on the local client

		m_creatureObject->setVisualPosture(Postures::Upright);

		// Only send the posture change request to the server for the local client

		if (Game::getPlayerCreature() == m_creatureObject)
		{
			m_creatureObject->requestServerPostureChange(Postures::Upright);
		}
	}
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::Musician::isPlayingOutro() const
{
	return m_playingOutro;
}

// ============================================================================
