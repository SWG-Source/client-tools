// ============================================================================
//
// PlayerMusicManager.cpp
// Copyright Sony Online Entertainment Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerMusicManager.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupManager.h"
#include "clientGame/PlayerMusicManager_Musician.h"
#include "clientGame/PlayerMusicManager_ParticleSystem.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/DataTable.h"

#include <list>
#include <map>
#include <set>
#include <vector>

#ifdef _DEBUG
#include "sharedDebug/DebugFlags.h"
#endif // _DEBUG

// ============================================================================
//
// PlayerMusicManagerNamespace
//
// ============================================================================

namespace PlayerMusicManagerNamespace
{
	class PerformanceDataTableRow
	{
	public:

		PerformanceDataTableRow();

		std::string m_intro;
		std::string m_mainLoop;
		std::string m_flourish[8];
		std::string m_outro;
		int         m_instrumentId;
	};

	typedef std::set<PlayerMusicManager::Musician *>                               Musicians;
	typedef std::map<PlayerMusicManager::ParticleType, std::string>                MusicParticleSystemPaths;
	typedef std::map<int, std::string>                                             FlourishParticleSystemPaths;
	typedef std::vector<PerformanceDataTableRow>                                   PerformanceDataTableRows;
	typedef std::set<CreatureObject *>                                             ListenerGroupMembers;
	typedef std::map<CreatureObject const *, PlayerMusicManager::ParticleSystem *> PerformanceParticleSystem;

	bool                        s_installed = false;
	Musicians                   s_musicians;
	Musicians                   s_removeMusicians;
	MusicParticleSystemPaths    s_musicParticleSystemPaths;
	FlourishParticleSystemPaths s_flourishLowParticleSystemPaths;
	FlourishParticleSystemPaths s_flourishMediumParticleSystemPaths;
	FlourishParticleSystemPaths s_flourishHighParticleSystemPaths;
	ListenerGroupMembers        s_listenerGroupMembers;
	PerformanceParticleSystem   s_performanceParticleSystem;
	PerformanceDataTableRows    s_performanceDataTableRows;
	bool                        s_playerMusicManagerDebug = false;
	int                         s_flourishCount = 8;
	bool                        s_sampleFinished = true;
	PerformanceTimer *          s_sampleFinishedTime = NULL;
	float                       s_startPerformanceTimer = 0.0f;

	void        addMusician(CreatureObject *creatureObject);
	void        startMusic();
	void        nextFrame();
	void        sampleFinished();
	std::string getParticleSystemPath(DataTable const &dataTable, std::string const &name);
	void        loadFlourishParticlePaths(FlourishParticleSystemPaths &flourishParticleSystemPaths, std::string const &path);

#ifdef _DEBUG
	void showDebug();
#endif // _DEBUG
}

using namespace PlayerMusicManagerNamespace;

// ============================================================================
//
// PlayerMusicManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::sampleFinished()
{
	s_sampleFinished = true;
	s_sampleFinishedTime->start();
}

//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::addMusician(CreatureObject *creatureObject)
{
	if (creatureObject != NULL)
	{
		// Add the musician to the list if they are not already in the list

		bool found = false;
		Musicians::iterator iterMusicians = s_musicians.begin();

		for (; iterMusicians != s_musicians.end(); ++iterMusicians)
		{
			if ((*iterMusicians)->getCreatureObject() == creatureObject)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			s_musicians.insert(new PlayerMusicManager::Musician(creatureObject));

			// Turn off the "not listening to" particles if we are listening to
			// this musician

			PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.find(creatureObject);

			if (iterPerformanceParticleSystem != s_performanceParticleSystem.end())
			{
				iterPerformanceParticleSystem->second->setEnabled(false);
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Trying to add a NULL musican."));
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::startMusic()
{
	//DEBUG_REPORT_LOG(true, ("start music\n"));

	GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
	CreatureObject *creatureObject = Game::getPlayerCreature();
	SoundId silentSoundId;

	if (creatureObject != NULL)
	{
		silentSoundId = Audio::attachSound("player_music/sound/no_sound.snd", creatureObject);
		Audio::setSoundVolume(silentSoundId, 0.0f);
		Audio::alter(0.0f, (groundScene ? groundScene->getSoundObject() : NULL));
	}

	Musicians::iterator iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		PlayerMusicManager::Musician &musician = *(*iterMusicians);

		if (musician.isPerforming())
		{
			// Start the intro

			musician.startPlaying();

			Audio::setEndOfSampleCallBack(musician.getSoundId(), PlayerMusicManagerNamespace::sampleFinished);
		}
	}

	Audio::alter(0.0f, (groundScene ? groundScene->getSoundObject() : NULL));
	Audio::stopSound(silentSoundId);
}

//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::nextFrame()
{
	//DEBUG_REPORT_LOG(true, ("next frame\n"));

	// Determine if any of the musicians are flourishing so we can lower the
	// volume of musicians who are not flourishing

	bool musicianIsFlourishing = false;

	Musicians::iterator iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		PlayerMusicManager::Musician &musician = *(*iterMusicians);

		if (musician.isAboutToPerformFlourish())
		{
			musicianIsFlourishing = true;
			break;
		}
	}

	GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
	CreatureObject *creatureObject = Game::getPlayerCreature();
	SoundId silentSoundId;

	if (creatureObject != NULL)
	{
		silentSoundId = Audio::attachSound("player_music/sound/no_sound.snd", creatureObject);
		Audio::setSoundVolume(silentSoundId, 0.0f);
		Audio::alter(0.0f, (groundScene ? groundScene->getSoundObject() : NULL));
	}

	iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		PlayerMusicManager::Musician &musician = *(*iterMusicians);

		if (musician.isPerforming())
		{
			musician.nextFrame(musicianIsFlourishing);

			if (!musician.isPlayingOutro())
			{
				Audio::setEndOfSampleCallBack(musician.getSoundId(), PlayerMusicManagerNamespace::sampleFinished);
			}
		}
	}

	Audio::alter(0.0f, (groundScene ? groundScene->getSoundObject() : NULL));
	Audio::stopSound(silentSoundId);
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManagerNamespace::getParticleSystemPath(DataTable const &dataTable, std::string const &name)
{
	std::string result;
	int const rowCount = dataTable.getNumRows();

	for (int index = 0; index < rowCount; ++index)
	{
		std::string particleName(dataTable.getStringValue("name", index));

		if (particleName == name)
		{
			result = dataTable.getStringValue("path", index);
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::loadFlourishParticlePaths(FlourishParticleSystemPaths &flourishParticleSystemPaths, std::string const &path)
{
	// Flourish particles data table

	Iff iff;

	if (iff.open(path.c_str(), true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		for (int row = 0; row < dataTable.getNumRows(); ++row)
		{
			int const index = dataTable.getIntValue("index", row);
			std::string const &path = dataTable.getStringValue("path", row);

			flourishParticleSystemPaths.insert(std::make_pair(index, path));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the player music flourish data table: %s", path.c_str()));
	}
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
void PlayerMusicManagerNamespace::showDebug()
{
	DEBUG_REPORT_PRINT(true, ("Musician List Count -        %d\n", static_cast<int>(s_musicians.size())));
	DEBUG_REPORT_PRINT(true, ("Listener Group Size -        %d\n", static_cast<int>(s_listenerGroupMembers.size())));
	DEBUG_REPORT_PRINT(true, ("Performance Particle Count - %d\n", static_cast<int>(s_performanceParticleSystem.size())));
	DEBUG_REPORT_PRINT(true, ("Sample Finished -            %s\n", s_sampleFinished ? "yes" : "no"));

	DEBUG_REPORT_PRINT(true, ("** Player Musician List **\n"));

	Musicians::const_iterator iterMusicians = s_musicians.begin();

	int referenceTime = 0;

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		if ((*iterMusicians)->isPerforming())
		{
			referenceTime = (*iterMusicians)->getSampleTime();
			break;
		}
	}

	int musicianNumber = 1;
	iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		if ((*iterMusicians)->getCreatureObject() != NULL)
		{
			PlayerMusicManager::Musician &musician = *(*iterMusicians);

			bool const idle = !musician.isPerforming();
			int const time = musician.getSampleTime();
			TemporaryCrcString currentSample;
			Audio::getCurrentSample(musician.getSoundId(), currentSample);
			char const *trimmedSample = strstr(currentSample.getString(), "player_music/sample/");

			if (trimmedSample != NULL)
			{
				trimmedSample += strlen("player_music/sample/");
			}

			int const timeDifference = (time - referenceTime);
			ClientObject *listenTargetClientObject = static_cast<ClientObject *>(NetworkIdManager::getObjectById((*iterMusicians)->getCreatureObject()->getPerformanceListenTarget()));
			std::string const musicianName = Unicode::wideToNarrow((*iterMusicians)->getCreatureObject()->getObjectName());
			std::string const listenTargetName = (listenTargetClientObject != NULL) ? Unicode::wideToNarrow(listenTargetClientObject->getObjectName()) : "No One";
			std::string const listenTargetNetworkId = (listenTargetClientObject != NULL) ? listenTargetClientObject->getNetworkId().getValueString() : "";
			int const consecutiveFlourishes = musician.getConsecutiveFlourishes();

			DEBUG_REPORT_PRINT(true, ("%2d %s is listening to %s (%s)\n", musicianNumber, musicianName.c_str(), listenTargetName.c_str(), listenTargetNetworkId.c_str()));

			if (!idle)
			{
				DEBUG_REPORT_PRINT(true, ("    %4d (%5d) %s\n", time, timeDifference, (trimmedSample != NULL) ? trimmedSample : currentSample.getString()));
				DEBUG_REPORT_PRINT(true, ("    flourishes queued: %d song: %d time since flourish: %.2f consecutive flourish %d\n", musician.getQueuedFlourishCount(), musician.getSongPlaying(), musician.getTimeSinceFlourish(), consecutiveFlourishes));
			}
			else
			{
				DEBUG_REPORT_PRINT(true, ("    IDLE\n"));
			}


			++musicianNumber;
		}
	}
}
#endif // _DEBUG

// ============================================================================
//
// PerformanceDataTableRow
//
// ============================================================================

//-----------------------------------------------------------------------------
PerformanceDataTableRow::PerformanceDataTableRow()
 : m_intro()
 , m_mainLoop()
 , m_flourish()
 , m_outro()
 , m_instrumentId(0)
{
}

// ============================================================================
//
// PlayerMusicManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void PlayerMusicManager::install()
{
	InstallTimer const installTimer("PlayerMusicManager::install");

	DEBUG_FATAL(s_installed, ("The PlayerMusicManager is already installed."));
	s_installed = true;

	delete s_sampleFinishedTime;
	s_sampleFinishedTime = new PerformanceTimer;
	s_sampleFinishedTime->start();

	// Performance data table

	Iff performanceIff;

	char const performanceFile[] = "datatables/performance/performance.iff";

	if (performanceIff.open(performanceFile, true))
	{
		DataTable dataTable;

		dataTable.load(performanceIff);

		int const rowCount = dataTable.getNumRows();
		int const musicHash = static_cast<int>(Crc::calculate("music"));

		for (int index = 0; index < rowCount; ++index)
		{
			int const hash = dataTable.getIntValue("type", index);

			if (hash == musicHash)
			{
				PerformanceDataTableRow performanceDataTableRow;

				performanceDataTableRow.m_intro = dataTable.getStringValue("intro", index);
				performanceDataTableRow.m_mainLoop = dataTable.getStringValue("mainloop", index);
				performanceDataTableRow.m_outro = dataTable.getStringValue("outro", index);
				performanceDataTableRow.m_instrumentId = dataTable.getIntValue("instrumentAudioId", index);
				performanceDataTableRow.m_flourish[0] = dataTable.getStringValue("flourish1", index);
				performanceDataTableRow.m_flourish[1] = dataTable.getStringValue("flourish2", index);
				performanceDataTableRow.m_flourish[2] = dataTable.getStringValue("flourish3", index);
				performanceDataTableRow.m_flourish[3] = dataTable.getStringValue("flourish4", index);
				performanceDataTableRow.m_flourish[4] = dataTable.getStringValue("flourish5", index);
				performanceDataTableRow.m_flourish[5] = dataTable.getStringValue("flourish6", index);
				performanceDataTableRow.m_flourish[6] = dataTable.getStringValue("flourish7", index);
				performanceDataTableRow.m_flourish[7] = dataTable.getStringValue("flourish8", index);

				s_performanceDataTableRows.push_back(performanceDataTableRow);
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the player music performance data table: %s", performanceFile));
	}

	// Music particles data table

	Iff musicParticlesIff;

	char const musicParticlesFile[] = "datatables/player_music/music_particles.iff";

	if (musicParticlesIff.open(musicParticlesFile, true))
	{
		DataTable dataTable;

		dataTable.load(musicParticlesIff);

		s_musicParticleSystemPaths.insert(std::make_pair(PT_musicLoopBoring, PlayerMusicManagerNamespace::getParticleSystemPath(dataTable, "musicLoopBoring")));
		s_musicParticleSystemPaths.insert(std::make_pair(PT_musicLoopEntertaining, PlayerMusicManagerNamespace::getParticleSystemPath(dataTable, "musicLoopEntertaining")));
		s_musicParticleSystemPaths.insert(std::make_pair(PT_musicPaused, PlayerMusicManagerNamespace::getParticleSystemPath(dataTable, "musicPaused")));
		s_musicParticleSystemPaths.insert(std::make_pair(PT_musicNotListening, PlayerMusicManagerNamespace::getParticleSystemPath(dataTable, "musicNotListening")));
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the player music performance data table: %s", performanceFile));
	}

	loadFlourishParticlePaths(s_flourishLowParticleSystemPaths, "datatables/player_music/flourish_low_particles.iff");
	loadFlourishParticlePaths(s_flourishMediumParticleSystemPaths, "datatables/player_music/flourish_medium_particles.iff");
	loadFlourishParticlePaths(s_flourishHighParticleSystemPaths, "datatables/player_music/flourish_high_particles.iff");

	// Cache all the sound templates

	{
		// Build a unique list of all the files to cache

		typedef std::set<std::string> SoundTemplateFileNames;
		SoundTemplateFileNames soundTemplateFileNames;

		PerformanceDataTableRows::const_iterator iterPerformanceDataTableRows = s_performanceDataTableRows.begin();

		for (; iterPerformanceDataTableRows != s_performanceDataTableRows.end(); ++iterPerformanceDataTableRows)
		{
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_intro);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_mainLoop);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_outro);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[0]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[1]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[2]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[3]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[4]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[5]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[6]);
			soundTemplateFileNames.insert(iterPerformanceDataTableRows->m_flourish[7]);
		}

		if (ConfigClientGame::getPreloadPlayerMusicManager ())
		{
			SoundTemplateFileNames::const_iterator iterSoundTemplateNames = soundTemplateFileNames.begin();

			for (; iterSoundTemplateNames != soundTemplateFileNames.end(); ++iterSoundTemplateNames)
			{
				char const *path = iterSoundTemplateNames->c_str();

				SoundTemplateList::preload(path);
			}
		}
	}

#ifdef _DEBUG
	DebugFlags::registerFlag(s_playerMusicManagerDebug, "ClientGame", "playerMusicManagerDebug", PlayerMusicManagerNamespace::showDebug);
#endif // _DEBUG

	ExitChain::add(PlayerMusicManager::remove, "PlayerMusicManager::remove", 0, false);
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::remove()
{
	DEBUG_FATAL(!s_installed, ("The PlayerMusicManager is not installed."));
	s_installed = false;

	sampleFinished();

	while (!s_musicians.empty())
	{
		PlayerMusicManager::Musician const *musician = *s_musicians.begin();

		s_musicians.erase(s_musicians.begin());

		delete musician;
		musician = NULL;
	}

	while (!s_performanceParticleSystem.empty())
	{
		PlayerMusicManager::ParticleSystem const *particleSystem = s_performanceParticleSystem.begin()->second;

		s_performanceParticleSystem.erase(s_performanceParticleSystem.begin());

		delete particleSystem;
		particleSystem = NULL;
	}

	delete s_sampleFinishedTime;
	s_sampleFinishedTime = NULL;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::queueFlourish(CreatureObject const *creatureObject, const int flourishIndex)
{
	Musicians::iterator iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		if (creatureObject == (*iterMusicians)->getCreatureObject())
		{
			(*iterMusicians)->addFlourish(flourishIndex);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::alter(float const deltaTime)
{
	// Remove musicians whose creature objects are not valid anymore

	s_removeMusicians.clear();
	Musicians::iterator iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		if ((*iterMusicians)->getCreatureObject() == NULL)
		{
			s_removeMusicians.insert(*iterMusicians);
		}
	}

	iterMusicians = s_removeMusicians.begin();

	for (; iterMusicians != s_removeMusicians.end(); ++iterMusicians)
	{
		PlayerMusicManager::Musician const *musician = (*iterMusicians);

		IGNORE_RETURN(s_musicians.erase(s_musicians.find(*iterMusicians)));

		delete musician;
		musician = NULL;

		//DEBUG_REPORT_LOG(true, ("Musician removed due to NULL creature object - Musician count: %d\n", static_cast<int>(s_musicians.size())));
	}

	if ((s_removeMusicians.size() > 0) &&
	    (s_musicians.size() == 0))
	{
		s_startPerformanceTimer = 0.0f;
		sampleFinished();
	}
	else if ((s_musicians.size() > 0) &&
	          s_sampleFinished)
	{
		s_sampleFinished = false;

		// Disable all the rest of the end of sample callbacks since one of
		// the musicians already finished

		iterMusicians = s_musicians.begin();

		for (; iterMusicians != s_musicians.end(); ++iterMusicians)
		{
			PlayerMusicManager::Musician &musician = *(*iterMusicians);

			Audio::setEndOfSampleCallBack(musician.getSoundId(), NULL);
		}

		// See if there is a musician in the band already playing

		iterMusicians = s_musicians.begin();
		bool bandIsStartingPerformance = true;

		for (; iterMusicians != s_musicians.end(); ++iterMusicians)
		{
			PlayerMusicManager::Musician &musician = *(*iterMusicians);

			if (musician.isPerforming() &&
			    !musician.isStartingPerformance())
			{
				bandIsStartingPerformance = false;
				break;
			}
		}

		// Find the first musician that is playing to see what the band needs
		// to be doing since all musicians are synched to each other

		iterMusicians = s_musicians.begin();

		for (; iterMusicians != s_musicians.end(); ++iterMusicians)
		{
			PlayerMusicManager::Musician &musician = *(*iterMusicians);

			if (musician.isPerforming())
			{
				if (bandIsStartingPerformance)
				{
					// This timer allows all the clients to get synched up for
					// the start of the music

					s_startPerformanceTimer += deltaTime;

					if (s_startPerformanceTimer > 2.0f)
					{
						startMusic();
					}
					else
					{
						s_sampleFinished = true;
					}
				}
				else
				{
					s_startPerformanceTimer = 0.0f;
					nextFrame();
				}

				break;
			}
		}
	}

	// Update the player musicians

	iterMusicians = s_musicians.begin();

	for (; iterMusicians != s_musicians.end(); ++iterMusicians)
	{
		PlayerMusicManager::Musician &musician = *(*iterMusicians);

		if (musician.isPerforming())
		{
			musician.alter(deltaTime);
		}
	}

	// Clean up any performance particle systems (musicians you are not listening to)

	PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.begin();

	for (; iterPerformanceParticleSystem != s_performanceParticleSystem.end(); ++iterPerformanceParticleSystem)
	{
		if (iterPerformanceParticleSystem->second->isDeletable())
		{
			PlayerMusicManager::ParticleSystem const *particleSystem = iterPerformanceParticleSystem->second;

			s_performanceParticleSystem.erase(iterPerformanceParticleSystem);

			delete particleSystem;
			particleSystem = NULL;
			break;
		}
	}

	// This catches the case that an end of sample fails to get called
	// while the band is playing

	if ((!Audio::isEnabled() &&
	     (s_sampleFinishedTime->getSplitTime() > 5.0f)) ||
	    ((s_musicians.size() > 0) &&
         (s_sampleFinishedTime->getSplitTime() > 20.0f)))
	{
		s_sampleFinishedTime->start();
		s_sampleFinished = true;
	}
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getMusicParticleSystemPath(ParticleType const particleType)
{
	std::string result;

	MusicParticleSystemPaths::const_iterator iterParticleSystemPaths = s_musicParticleSystemPaths.find(particleType);

	if (iterParticleSystemPaths != s_musicParticleSystemPaths.end())
	{
		result = iterParticleSystemPaths->second;
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getIntroSoundPath(int const songIndex)
{
	std::string result;

	if ((songIndex >= 0) &&
	    (songIndex < static_cast<int>(s_performanceDataTableRows.size())))
	{
		result = s_performanceDataTableRows[static_cast<unsigned int>(songIndex)].m_intro;
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getMainLoopSoundPath(int const songIndex)
{
	std::string result;

	if ((songIndex >= 0) &&
	    (songIndex < static_cast<int>(s_performanceDataTableRows.size())))
	{
		result = s_performanceDataTableRows[static_cast<unsigned int>(songIndex)].m_mainLoop;
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getOutroSoundPath(int const songIndex)
{
	std::string result;

	if ((songIndex >= 0) &&
	    (songIndex < static_cast<int>(s_performanceDataTableRows.size())))
	{
		result = s_performanceDataTableRows[static_cast<unsigned int>(songIndex)].m_outro;
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getFlourishSoundPath(int const songIndex, int const flourishIndex)
{
	std::string result;

	if ((songIndex >= 0) &&
	    (songIndex < static_cast<int>(s_performanceDataTableRows.size())) &&
		(flourishIndex >= 0) &&
		(flourishIndex <= getMaximumFlourishCount()))
	{
		result = s_performanceDataTableRows[songIndex].m_flourish[flourishIndex - 1];
	}

	return result;
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::getMaximumFlourishCount()
{
	return s_flourishCount;
}

//-----------------------------------------------------------------------------
int PlayerMusicManager::getInstrumentId(CreatureObject const * const creatureObject)
{
	int result = 0;

	if (creatureObject != NULL)
	{
		int const index = creatureObject->getPerformanceType() - 1;

		if ((index >= 0) &&
		    (index < static_cast<int>(s_performanceDataTableRows.size())))
		{
			result = s_performanceDataTableRows[index].m_instrumentId;
		}
		else
		{
			DEBUG_WARNING(true, ("Invalid performance type index: %d", index));
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::getFlourishParticleSystemPath(int const skillMod, int const flourishCount)
{
	std::string result;

	FlourishParticleSystemPaths *flourishParticleSystemPaths = NULL;

	// Determine which list of paths to use

	if ((skillMod < 50) &&
	    !s_flourishLowParticleSystemPaths.empty())
	{
		flourishParticleSystemPaths = &s_flourishLowParticleSystemPaths;
	}
	else if ((skillMod < 100) &&
	         !s_flourishMediumParticleSystemPaths.empty())
	{
		flourishParticleSystemPaths = &s_flourishMediumParticleSystemPaths;
	}
	else if (!s_flourishHighParticleSystemPaths.empty())
	{
		flourishParticleSystemPaths = &s_flourishHighParticleSystemPaths;
	}

	// Find the actual path

	if (flourishParticleSystemPaths != NULL)
	{
		FlourishParticleSystemPaths::const_iterator iterFlourishParticleSystemPaths = flourishParticleSystemPaths->begin();
		FlourishParticleSystemPaths::const_iterator iterFound = flourishParticleSystemPaths->begin();

		for (; iterFlourishParticleSystemPaths != flourishParticleSystemPaths->end(); ++iterFlourishParticleSystemPaths)
		{
			if (flourishCount > iterFlourishParticleSystemPaths->first)
			{
				iterFound = iterFlourishParticleSystemPaths;
			}
			else
			{
				break;
			}
		}

		result = iterFound->second;
	}

	DEBUG_WARNING(result.empty(), ("The player music flourish particle effect does not exist."));

	return result;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::startPerformance(CreatureObject &musician)
{
	if ((musician.getPerformanceType() > 0) &&
	    (musician.getPerformanceType() <= static_cast<int>(s_performanceDataTableRows.size())))
	{
		// All musicians get particles representing we are not listening to them
		// but they are playing

		PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.find(&musician);

		if (iterPerformanceParticleSystem == s_performanceParticleSystem.end())
		{
			PlayerMusicManager::ParticleSystem *particleSystem = new PlayerMusicManager::ParticleSystem;
			particleSystem->create(&musician, getMusicParticleSystemPath(PT_musicNotListening).c_str());
			particleSystem->setAllowDelete(false);
			particleSystem->setEnabled(true);
			s_performanceParticleSystem.insert(std::make_pair(&musician, particleSystem));
		}

		// See if the musician needs to be audible

		CreatureObject *creatureObject = Game::getPlayerCreature();

		if (creatureObject != NULL)
		{
			checkStatus(creatureObject);
		}
	}
}


//-----------------------------------------------------------------------------
void PlayerMusicManager::stopPerformance(CreatureObject &musician)
{
	//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::stopPerformance()\n"));

	// See who needs to stop being audible

	CreatureObject *creatureObject = Game::getPlayerCreature();

	if (creatureObject != NULL)
	{
		checkStatus(creatureObject);
	}

	// Stop the music particles for this musician

	PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.find(&musician);

	if (iterPerformanceParticleSystem != s_performanceParticleSystem.end())
	{
		iterPerformanceParticleSystem->second->setAllowDelete(true);
		iterPerformanceParticleSystem->second->setEnabled(false);
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::checkStatus(CreatureObject const * const creatureObject)
{
	if (creatureObject == NULL)
	{
		return;
	}

	// See if I am listening to someone

	bool const listening = creatureObject->getPerformanceListenTarget().isValid();

	if (listening)
	{
		CreatureObject * const listenTargetCreatureObject = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(creatureObject->getPerformanceListenTarget()));

		if (listenTargetCreatureObject != NULL)
		{
			// Get the current listener group members

			GroupManager::getGroupMembers(*listenTargetCreatureObject, s_listenerGroupMembers);

			//DEBUG_REPORT_LOG(true, ("Listener group size: %d\n", static_cast<int>(s_listenerGroupMembers.size())));

			// Build a list of musicians that are not in the group anymore or have stopped playing
			// music

			s_removeMusicians.clear();

			Musicians::iterator iterMusicians = s_musicians.begin();

			for (; iterMusicians != s_musicians.end(); ++iterMusicians)
			{
				PlayerMusicManager::Musician &musician = *(*iterMusicians);
				CreatureObject const *musicianCreatureObject = musician.getCreatureObject();

				if (musicianCreatureObject != NULL)
				{
					int const performanceType = musicianCreatureObject->getPerformanceType();
					int const musicPerformanceCount = static_cast<int>(s_performanceDataTableRows.size());

					if ((s_listenerGroupMembers.find(const_cast<CreatureObject *>(musicianCreatureObject)) == s_listenerGroupMembers.end()) ||
						(performanceType > musicPerformanceCount) ||
						(!musician.isPerforming()))
					{
						// A creature that was in the group is not in the current group, so mark
						// it for removal

						IGNORE_RETURN(s_removeMusicians.insert(*iterMusicians));

						//DEBUG_REPORT_LOG(true, ("Removing musician: %s total count: %d\n", Unicode::wideToNarrow(musicianCreatureObject->getLocalizedName()).c_str(), static_cast<int>(s_removeMusicians.size())));
					}
				}
				else
				{
					// The musicians creature object is NULL so delete the musician

					IGNORE_RETURN(s_removeMusicians.insert(*iterMusicians));

					//DEBUG_REPORT_LOG(true, ("Removing musician: %s total count: %d\n", Unicode::wideToNarrow(musicianCreatureObject->getLocalizedName()).c_str(), static_cast<int>(s_removeMusicians.size())));
				}
			}

			// Remove all the musicians that are not in the group anymore

			//DEBUG_REPORT_LOG(true, ("Musician count: %d\n", static_cast<int>(s_musicians.size())));

			Musicians::iterator iterRemoveMusicians = s_removeMusicians.begin();

			for (; iterRemoveMusicians != s_removeMusicians.end(); ++iterRemoveMusicians)
			{
				PlayerMusicManager::Musician const *musician = (*iterRemoveMusicians);

				// If they were a musician and are still playing, then make sure they have
				// the not-listening particles

				PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.find(musician->getCreatureObject());

				if (iterPerformanceParticleSystem != s_performanceParticleSystem.end())
				{
					//DEBUG_REPORT_LOG(true, ("not-listening particles enabled\n"));
					iterPerformanceParticleSystem->second->setEnabled(true);
				}

				IGNORE_RETURN(s_musicians.erase(s_musicians.find(*iterRemoveMusicians)));

				delete musician;
				musician = NULL;

				//DEBUG_REPORT_LOG(true, ("Musician count: %d\n", static_cast<int>(s_musicians.size())));
			}

			// No one is currently playing, so signal that all music has stopped

			if ((s_removeMusicians.size() > 0) &&
			    (s_musicians.size() == 0))
			{
				sampleFinished();
			}

			// Add new musicians to the group, if they are already in the group, they
			// are not added twice.

			ListenerGroupMembers::iterator iterGroupMembers = s_listenerGroupMembers.begin();

			for (; iterGroupMembers != s_listenerGroupMembers.end(); ++iterGroupMembers)
			{
				CreatureObject *groupMember = (*iterGroupMembers);

				if (groupMember != NULL)
				{
					int const performanceType = groupMember->getPerformanceType();
					int const musicPerformanceCount = static_cast<int>(s_performanceDataTableRows.size());

					if ((performanceType > 0) &&
					    (performanceType <= musicPerformanceCount))
					{
						addMusician(groupMember);
					}
				}
			}
		}
	}
	else
	{
		bool const musiciansExisted = !s_musicians.empty();

		// Not listening to anyone so remove all the musicians

		while (!s_musicians.empty())
		{
			PlayerMusicManager::Musician const *musician = (*s_musicians.begin());

			// If they were a musician and are still playing, then make sure they have
			// the not-listening particles

			PerformanceParticleSystem::iterator iterPerformanceParticleSystem = s_performanceParticleSystem.find(musician->getCreatureObject());

			if (iterPerformanceParticleSystem != s_performanceParticleSystem.end())
			{
				//DEBUG_REPORT_LOG(true, ("not-listening particles enabled\n"));
				iterPerformanceParticleSystem->second->setEnabled(true);
			}

			IGNORE_RETURN(s_musicians.erase(s_musicians.begin()));

			delete musician;
			musician = NULL;
		}

		if (musiciansExisted)
		{
			sampleFinished();
		}

		s_listenerGroupMembers.clear();
	}
}

// ============================================================================
