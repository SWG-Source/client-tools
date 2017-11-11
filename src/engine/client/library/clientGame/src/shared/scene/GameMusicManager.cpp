// ======================================================================
//
// GameMusicManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameMusicManager.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PreloadedAssetManager.h"
#include "clientGame/ShipObject.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/EnvironmentBlock.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

#include <algorithm>
#include <vector>

// ======================================================================
// GameMusicManagerNamespace
// ======================================================================

namespace GameMusicManagerNamespace
{
	const char* ms_defaultConfigFilename = "scene/game_music_manager";
	const float ms_soundFadeOutTime = 1.f;

	const Tag TAG_DETH = TAG (D,E,T,H);
	const Tag TAG_EXIT = TAG (E,X,I,T);
	const Tag TAG_GMUS = TAG (G,M,U,S);
	const Tag TAG_INCP = TAG (I,N,C,P);
	const Tag TAG_WATR = TAG (W,A,T,R);
	Tag const TAG_SENT = TAG(S,E,N,T);
	Tag const TAG_SEXI = TAG(S,E,X,I);

	ConstWatcher<Object> ms_referenceObject;
	
	const char*   ms_currentAmbient1SoundTemplateName;
	SoundId       ms_currentAmbient1SoundId;
	const char*   ms_currentAmbient2SoundTemplateName;
	SoundId       ms_currentAmbient2SoundId;
	const char*   ms_currentSoundSoundTemplateName;
	SoundId       ms_currentSoundSoundId;
	const char*   ms_currentMusicSoundTemplateName;
	SoundId       ms_currentMusicSoundId;
	const char*   ms_currentWaterSoundTemplateName;
	SoundId       ms_currentWaterSoundId;
	bool ms_overrideMusic;

	typedef std::vector<const EnvironmentBlock*> EnvironmentBlockList;
	EnvironmentBlockList ms_environmentBlockFirstPlayedList;

	typedef std::vector<const InteriorEnvironmentBlock*> InteriorEnvironmentBlockList;
	InteriorEnvironmentBlockList ms_interiorEnvironmentBlockFirstPlayedList;

	typedef std::vector<PersistentCrcString*> StringList;
	StringList    ms_combatDeathList;
	StringList    ms_combatEnterList;
	StringList    ms_combatExitList;
	StringList    ms_combatIncapacitationList;
	StringList ms_spaceCombatEnterList;
	StringList ms_spaceCombatExitList;

	PersistentCrcString* ms_waterSoundTemplateName;

	enum Event
	{
		E_none,
		E_enter,
		E_exit,
		E_spaceEnter,
		E_spaceExit,
		E_death,
		E_incapacitation,
		E_sunrise,
		E_sunset
	};

	Event ms_event;

	Postures::Enumerator ms_lastPosture;
	bool ms_lastCombatState;
	bool ms_lastDay;
	bool ms_lastSpaceCombatState;
	float const s_spaceCombatMusicMinDistance = 1024.0f;
	Timer ms_spaceCombatStateTimer(5.0f);
	size_t ms_lastSpaceCombatEnterIndex = 0;
	
	typedef std::map<CachedNetworkId, Timer> AttackerMap;
	AttackerMap ms_attackerMap;

	float computeWaterVolume ();
	bool determineSpaceCombatState(float const elapsedTime);
	bool verifySoundTemplate(char const * soundTemplateName);
}

using namespace GameMusicManagerNamespace;

// ======================================================================
// STATIC PUBLIC GameMusicManager
// ======================================================================

void GameMusicManager::install ( const char* buildoutAreaName )
{
	InstallTimer const installTimer("GameMusicManager::install");

	char filename[256];

	sprintf(filename, "%s_%s.iff", ms_defaultConfigFilename, buildoutAreaName);

	Iff iff;
	bool opened = iff.open (filename, true);

	if (!opened)
	{
		sprintf(filename, "%s.iff", ms_defaultConfigFilename);
		opened = iff.open(filename, true);
	}

	if (opened)
	{
		iff.enterForm (TAG_GMUS);

			iff.enterForm (TAG_0000);

				while (iff.getNumberOfBlocksLeft ())
				{
					switch (iff.getCurrentName ())
					{
					case TAG_DETH:
						{
							iff.enterChunk (TAG_DETH);
								char* soundTemplateName = iff.read_string ();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_combatDeathList.push_back (new PersistentCrcString (soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk ();
						}
						break;

					case TAG_ENTR:
						{
							iff.enterChunk (TAG_ENTR);
								char* soundTemplateName = iff.read_string ();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_combatEnterList.push_back (new PersistentCrcString (soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk ();
						}
						break;

					case TAG_EXIT:
						{
							iff.enterChunk (TAG_EXIT);
								char* soundTemplateName = iff.read_string ();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_combatExitList.push_back (new PersistentCrcString (soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk ();
						}
						break;

					case TAG_INCP:
						{
							iff.enterChunk (TAG_INCP);
								char* soundTemplateName = iff.read_string ();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_combatIncapacitationList.push_back (new PersistentCrcString (soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk ();
						}
						break;

					case TAG_SENT:
						{
							iff.enterChunk(TAG_SENT);
								char * const soundTemplateName = iff.read_string();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_spaceCombatEnterList.push_back(new PersistentCrcString(soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk();
						}
						break;

					case TAG_SEXI:
						{
							iff.enterChunk(TAG_SEXI);
								char * const soundTemplateName = iff.read_string();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								ms_spaceCombatExitList.push_back(new PersistentCrcString(soundTemplateName, true));
								delete [] soundTemplateName;
							iff.exitChunk();
						}
						break;

					case TAG_WATR:
						{
							iff.enterChunk (TAG_WATR);
								char* soundTemplateName = iff.read_string ();
								PreloadedAssetManager::addSoundTemplate(soundTemplateName);
								if (ms_waterSoundTemplateName)
									delete ms_waterSoundTemplateName;
								ms_waterSoundTemplateName = new PersistentCrcString (soundTemplateName, true);
								delete [] soundTemplateName;
							iff.exitChunk ();
						}
						break;

					default:
						{
#ifdef _DEBUG
							char tagString [5];
							ConvertTagToString (iff.getCurrentName (), tagString);
							DEBUG_WARNING (true, ("GameMusicManager: [%s] - unknown chunk type %s, expecting DETH, EXIT, GMUS, INCP, or WATR", iff.getFileName (), tagString));
#endif

							if (iff.isCurrentForm ())
							{
								iff.enterForm ();
								iff.exitForm (true);
							}
							else
							{
								iff.enterChunk ();
								iff.exitChunk (true);
							}
						}
						break;
					}
				}

			iff.exitForm (TAG_0000);

		iff.exitForm (TAG_GMUS);
	}

	DEBUG_WARNING(ms_spaceCombatEnterList.size() != ms_spaceCombatExitList.size(), ("GameMusicManager::install(): [scene/game_music_manager.iff] space combat enter and exit events have a mismatched number of sound templates"));
}

// ----------------------------------------------------------------------

void GameMusicManager::remove ()
{
	ms_referenceObject = 0;

	ms_currentAmbient1SoundTemplateName = 0;

	if (ms_currentAmbient1SoundId.isValid ())
		Audio::stopSound (ms_currentAmbient1SoundId, ms_soundFadeOutTime);

	ms_currentAmbient2SoundTemplateName = 0;

	if (ms_currentAmbient2SoundId.isValid ())
		Audio::stopSound (ms_currentAmbient2SoundId, ms_soundFadeOutTime);

	ms_currentSoundSoundTemplateName = 0;

	if (ms_currentSoundSoundId.isValid ())
		Audio::stopSound (ms_currentSoundSoundId, ms_soundFadeOutTime);

	ms_currentMusicSoundTemplateName = 0;

	if (ms_currentMusicSoundId.isValid ())
		Audio::stopSound (ms_currentMusicSoundId, ms_soundFadeOutTime);

	ms_currentWaterSoundTemplateName = 0;

	if (ms_currentWaterSoundId.isValid ())
		Audio::stopSound (ms_currentWaterSoundId, ms_soundFadeOutTime);

	ms_environmentBlockFirstPlayedList.clear ();
	ms_interiorEnvironmentBlockFirstPlayedList.clear ();

	std::for_each (ms_combatDeathList.begin (), ms_combatDeathList.end (), PointerDeleter ());
	ms_combatDeathList.clear ();

	std::for_each (ms_combatEnterList.begin (), ms_combatEnterList.end (), PointerDeleter ());
	ms_combatEnterList.clear ();

	std::for_each (ms_combatExitList.begin (), ms_combatExitList.end (), PointerDeleter ());
	ms_combatExitList.clear ();

	std::for_each (ms_combatIncapacitationList.begin (), ms_combatIncapacitationList.end (), PointerDeleter ());
	ms_combatIncapacitationList.clear ();

	std::for_each(ms_spaceCombatEnterList.begin(), ms_spaceCombatEnterList.end(), PointerDeleter());
	ms_spaceCombatEnterList.clear();

	std::for_each(ms_spaceCombatExitList.begin(), ms_spaceCombatExitList.end(), PointerDeleter());
	ms_spaceCombatExitList.clear();

	delete ms_waterSoundTemplateName;
	ms_waterSoundTemplateName = 0;

	ms_event = E_none;
	ms_lastPosture = Postures::Invalid;
	ms_lastCombatState = false;
	ms_lastDay = false;
	ms_lastSpaceCombatState = false;
	ms_overrideMusic = false;
}

// ----------------------------------------------------------------------

void GameMusicManager::setReferenceObject (const Object* const referenceObject)
{
	ms_referenceObject = referenceObject;
}

//-------------------------------------------------------------------

void GameMusicManager::setMusicTrackOverride(char const * const soundTemplateName)
{
	if (verifySoundTemplate(soundTemplateName))
	{
		if (ms_currentMusicSoundId.isValid())
			Audio::stopSound(ms_currentMusicSoundId, 1.f);

		if (soundTemplateName && *soundTemplateName)
			ms_currentMusicSoundId = Audio::playSound(soundTemplateName, NULL);

		ms_overrideMusic = true;
	}
}

//-------------------------------------------------------------------

void GameMusicManager::cancelMusicTrackOverride()
{
	if (ms_overrideMusic)
	{
		if (ms_currentMusicSoundId.isValid())
			Audio::stopSound(ms_currentMusicSoundId, 1.f);

		ms_overrideMusic = false;
	}
}

// ----------------------------------------------------------------------

void GameMusicManager::setSoundTrackOverride(char const * const soundTemplateName)
{
	if (verifySoundTemplate(soundTemplateName))
	{
		if (ms_currentSoundSoundId.isValid ())
			Audio::stopSound (ms_currentSoundSoundId, 1.f);

		if (soundTemplateName && *soundTemplateName)
			ms_currentSoundSoundId = Audio::playSound (soundTemplateName, NULL);
	}
}

// ----------------------------------------------------------------------

void GameMusicManager::update(float const elapsedTime)
{
	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	if (!terrainObject)
		return;

	const ClientProceduralTerrainAppearance* const clientProceduralTerrainAppearance = dynamic_cast<const ClientProceduralTerrainAppearance*> (terrainObject->getAppearance ());
	EnvironmentBlock const * const currentEnvironmentBlock = clientProceduralTerrainAppearance ? clientProceduralTerrainAppearance->getCurrentEnvironmentBlock() : 0;
	InteriorEnvironmentBlock const * const currentInteriorEnvironmentBlock = clientProceduralTerrainAppearance ? clientProceduralTerrainAppearance->getCurrentInteriorEnvironmentBlock() : 0;

	//-- desired ambience 1
	if (currentEnvironmentBlock && currentInteriorEnvironmentBlock)
	{
		const char* playAmbientSoundTemplateName = 0;
		if (ms_referenceObject->isInWorldCell ())
			playAmbientSoundTemplateName = (terrainObject->isDay ()) ? currentEnvironmentBlock->getDay1AmbientSoundTemplateName ()->getString () : currentEnvironmentBlock->getNight1AmbientSoundTemplateName ()->getString ();
		else
			playAmbientSoundTemplateName = (terrainObject->isDay ()) ? currentInteriorEnvironmentBlock->getDayAmbientSoundTemplateName ()->getString () : currentInteriorEnvironmentBlock->getNightAmbientSoundTemplateName ()->getString ();

		if (ms_currentAmbient1SoundTemplateName != playAmbientSoundTemplateName)
		{
			if (ms_currentAmbient1SoundId.isValid ())
				Audio::stopSound (ms_currentAmbient1SoundId, ms_soundFadeOutTime);

			ms_currentAmbient1SoundTemplateName = playAmbientSoundTemplateName;
			ms_currentAmbient1SoundId  = playAmbientSoundTemplateName && istrlen (playAmbientSoundTemplateName) ? Audio::playSound (playAmbientSoundTemplateName, ms_referenceObject->getParentCell()) : SoundId (0);
		}
	}

	//-- desired ambience 2
	if (currentEnvironmentBlock && currentInteriorEnvironmentBlock)
	{
		const char* playAmbientSoundTemplateName = 0;
		if (ms_referenceObject->isInWorldCell ())
			playAmbientSoundTemplateName = (terrainObject->isDay ()) ? currentEnvironmentBlock->getDay2AmbientSoundTemplateName ()->getString () : currentEnvironmentBlock->getNight2AmbientSoundTemplateName ()->getString ();

		if (ms_currentAmbient2SoundTemplateName != playAmbientSoundTemplateName)
		{
			if (ms_currentAmbient2SoundId.isValid ())
				Audio::stopSound (ms_currentAmbient2SoundId, ms_soundFadeOutTime);

			ms_currentAmbient2SoundTemplateName = playAmbientSoundTemplateName;
			ms_currentAmbient2SoundId  = playAmbientSoundTemplateName && istrlen (playAmbientSoundTemplateName) ? Audio::playSound (playAmbientSoundTemplateName, ms_referenceObject->getParentCell()) : SoundId (0);
		}
	}

/*
	//-- desired water
	{
		const char* playWaterSoundTemplateName = 0;
		float waterVolume = computeWaterVolume ();
		if (ms_referenceObject->isInWorldCell () && waterVolume > 0.f)
			playWaterSoundTemplateName = ms_waterSoundTemplateName ? ms_waterSoundTemplateName->getString () : 0;
		else
			playWaterSoundTemplateName = 0;

		if (ms_currentWaterSoundTemplateName != playWaterSoundTemplateName)
		{
			if (ms_currentWaterSoundId.isValid ())
				Audio::stopSound (ms_currentWaterSoundId, 2.5f);

			ms_currentWaterSoundTemplateName = playWaterSoundTemplateName;
			ms_currentWaterSoundId = playWaterSoundTemplateName && istrlen (playWaterSoundTemplateName) ? Audio::playSound (playWaterSoundTemplateName, ms_referenceObject->getParentCell()) : SoundId (0);
		}
	}
*/

	//-- desired music
	if (ms_overrideMusic)
	{
		if (!ms_currentMusicSoundId.isValid() || !Audio::isSoundPlaying(ms_currentMusicSoundId))
			ms_overrideMusic = false;
	}
	else
	{
		//-- see if we've had an event or if we're no longer playing music
		{
			if (Game::isSpace())
			{
				bool const spaceCombatState = determineSpaceCombatState(elapsedTime);

				if (spaceCombatState != ms_lastSpaceCombatState)
				{
					if (spaceCombatState)
						ms_event = E_spaceEnter;
					else
						ms_event = E_spaceExit;
				}

				ms_lastSpaceCombatState = spaceCombatState;
			}
			else
			{
				const CreatureObject* const creatureObject = dynamic_cast<const CreatureObject*> (ms_referenceObject.getPointer());
				if (creatureObject)
				{
					const Postures::Enumerator posture = creatureObject->getVisualPosture ();
					const bool combatState = creatureObject->getState (States::Combat);
					const bool day = terrainObject->isDay ();

					if (posture == Postures::Incapacitated && posture != ms_lastPosture)
						ms_event = E_incapacitation;
					else
						if (posture == Postures::Dead && posture != ms_lastPosture)
							ms_event = E_death;
						else
						{
							if (combatState != ms_lastCombatState)
							{
								if (combatState)
									ms_event = E_enter;
								else
									ms_event = E_exit;
							}
							else
								if (!combatState && (day != ms_lastDay))
								{
									if (day)
										ms_event = E_sunrise;
									else
										ms_event = E_sunset;
								}
						}

					ms_lastPosture = posture;
					ms_lastCombatState = combatState;
					ms_lastDay = day;
				}
			}
		}

		if (ms_event != E_none || !(ms_currentMusicSoundId.isValid () && Audio::isSoundPlaying (ms_currentMusicSoundId)))
		{
			const char* playMusicSoundTemplateName = 0;

			switch (ms_event)
			{
			case E_none:
				{
					if (currentEnvironmentBlock && currentInteriorEnvironmentBlock)
					{
						if (ms_referenceObject->isInWorldCell ())
						{
							EnvironmentBlockList::iterator iter = std::find (ms_environmentBlockFirstPlayedList.begin (), ms_environmentBlockFirstPlayedList.end (), currentEnvironmentBlock);
							if (iter == ms_environmentBlockFirstPlayedList.end ())
							{
								ms_environmentBlockFirstPlayedList.push_back (currentEnvironmentBlock);

								const CrcString* const soundTemplateName = currentEnvironmentBlock->getFirstMusicSoundTemplateName ();
								playMusicSoundTemplateName = (soundTemplateName && soundTemplateName->getString () && *soundTemplateName->getString ()) ? soundTemplateName->getString () : 0;
							}
						}
						else
						{
							InteriorEnvironmentBlockList::iterator iter = std::find (ms_interiorEnvironmentBlockFirstPlayedList.begin (), ms_interiorEnvironmentBlockFirstPlayedList.end (), currentInteriorEnvironmentBlock);
							if (iter == ms_interiorEnvironmentBlockFirstPlayedList.end ())
							{
								ms_interiorEnvironmentBlockFirstPlayedList.push_back (currentInteriorEnvironmentBlock);

								const CrcString* const soundTemplateName = currentInteriorEnvironmentBlock->getFirstMusicSoundTemplateName ();
								playMusicSoundTemplateName = (soundTemplateName && soundTemplateName->getString () && *soundTemplateName->getString ()) ? soundTemplateName->getString () : 0;
							}
						}
					}
				}
				break;

			case E_death:
				{
					if (!ms_combatDeathList.empty ())
						playMusicSoundTemplateName = ms_combatDeathList.size () > 1 ? ms_combatDeathList [Random::random (0, ms_combatDeathList.size () - 1)]->getString () : ms_combatDeathList [0]->getString ();
				}
				break;

			case E_enter:
				{
					if (!ms_combatEnterList.empty ())
						playMusicSoundTemplateName = ms_combatEnterList.size () > 1 ? ms_combatEnterList [Random::random (0, ms_combatEnterList.size () - 1)]->getString () : ms_combatEnterList [0]->getString ();
				}
				break;

			case E_exit:
				{
					if (!ms_combatExitList.empty ())
						playMusicSoundTemplateName = ms_combatExitList.size () > 1 ? ms_combatExitList [Random::random (0, ms_combatExitList.size () - 1)]->getString () : ms_combatExitList [0]->getString ();
				}
				break;

			case E_spaceEnter:
				{
					if (!ms_spaceCombatEnterList.empty())
					{
						ms_lastSpaceCombatEnterIndex = Random::random(0, ms_spaceCombatEnterList.size() - 1);
						playMusicSoundTemplateName = ms_lastSpaceCombatEnterIndex < ms_spaceCombatEnterList.size() ? ms_spaceCombatEnterList[ms_lastSpaceCombatEnterIndex]->getString() : ms_spaceCombatEnterList[0]->getString();
					}
				}
				break;

			case E_spaceExit:
				{
					if (!ms_spaceCombatExitList.empty())
						playMusicSoundTemplateName = ms_lastSpaceCombatEnterIndex < ms_spaceCombatExitList.size() ? ms_spaceCombatExitList[ms_lastSpaceCombatEnterIndex]->getString() : ms_spaceCombatExitList[0]->getString();
				}
				break;

			case E_incapacitation:
				{
					if (!ms_combatIncapacitationList.empty ())
						playMusicSoundTemplateName = ms_combatIncapacitationList.size () > 1 ? ms_combatIncapacitationList [Random::random (0, ms_combatIncapacitationList.size () - 1)]->getString () : ms_combatIncapacitationList [0]->getString ();
				}
				break;

			case E_sunrise:
				{
					if (currentEnvironmentBlock)
					{
						const CrcString* const soundTemplateName = currentEnvironmentBlock->getSunriseMusicSoundTemplateName ();
						playMusicSoundTemplateName = (soundTemplateName && soundTemplateName->getString () && *soundTemplateName->getString ()) ? soundTemplateName->getString () : 0;
					}
				}
				break;

			case E_sunset:
				{
					if (currentEnvironmentBlock)
					{
						const CrcString* const soundTemplateName = currentEnvironmentBlock->getSunsetMusicSoundTemplateName ();
						playMusicSoundTemplateName = (soundTemplateName && soundTemplateName->getString () && *soundTemplateName->getString ()) ? soundTemplateName->getString () : 0;
					}
				}
				break;
			}

			if (ms_currentMusicSoundId.isValid ())
				Audio::stopSound (ms_currentMusicSoundId, ms_soundFadeOutTime);

			ms_currentMusicSoundTemplateName = playMusicSoundTemplateName;
			ms_currentMusicSoundId  = playMusicSoundTemplateName && istrlen (playMusicSoundTemplateName) ? Audio::playSound (playMusicSoundTemplateName, ms_referenceObject->getParentCell()) : SoundId (0);

			ms_event = E_none;
		}
	}
}

// ======================================================================

float GameMusicManagerNamespace::computeWaterVolume ()
{
	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
		return 0.f;

	const int chunkX = terrainObject->calculateChunkX (ms_referenceObject->getPosition_w ());
	const int chunkZ = terrainObject->calculateChunkZ (ms_referenceObject->getPosition_w ());

	int total = 0;
	int count = 0;
	int x;
	int z;
	for (z = chunkZ - 2; z <= chunkZ + 2; ++z)
		for (x = chunkX - 2; x <= chunkX + 2; ++x)
		{
			++total;

			if (terrainObject->getWater (x, z))
				++count;
		}

	const int total_2 = total / 2;
	if (total_2 == 0)
		return 0.f;

	if (count > total_2)
		count = total_2;

	return static_cast<float> (count) / total_2;
}

// ----------------------------------------------------------------------

bool GameMusicManagerNamespace::determineSpaceCombatState(float const elapsedTime)
{
	ShipObject * const playerContainingShip = Game::getPlayerContainingShip();

	if (!playerContainingShip)
		return false;

	// Update all the attackers to see who we can remove
	{
		AttackerMap::iterator iterAttackerMap = ms_attackerMap.begin();

		for (; iterAttackerMap != ms_attackerMap.end();)
		{
			Object * const attacker = iterAttackerMap->first.getObject();
			Timer & timer = iterAttackerMap->second;

			if (   (attacker == NULL)
			    || timer.updateNoReset(elapsedTime)
			    || (attacker->getPosition_w().magnitudeBetweenSquared(playerContainingShip->getPosition_w()) > sqr(s_spaceCombatMusicMinDistance)))
			{
				// The attacker is no longer valid

				ms_attackerMap.erase(iterAttackerMap++);
			}
			else
			{
				++iterAttackerMap;
			}
		}
	}

	//-- determine if enemies are targeting player
	{
		ShipObject::ShipVector::const_iterator iterShipsTargetingPlayer = ShipObject::getShipsTargetingPlayer().begin();

		for (; iterShipsTargetingPlayer != ShipObject::getShipsTargetingPlayer().end(); ++iterShipsTargetingPlayer)
		{
			ShipObject * const attacker = (*iterShipsTargetingPlayer);

			if (   (attacker != NULL)
			    && attacker->isEnemy())
			{
				// Make sure they are close by

				float const enemyDistanceToPlayer = attacker->getPosition_w().magnitudeBetweenSquared(playerContainingShip->getPosition_w());

				if (enemyDistanceToPlayer < sqr(s_spaceCombatMusicMinDistance))
				{
					GameMusicManager::startCombatMusic(attacker->getNetworkId());
					break;
				}
			}
		}
	}

	// If we have an attacker, play combat music
	{
		if (!ms_attackerMap.empty())
		{
			return true;
		}
	}

	bool enemiesTargetingPlayer = false;

	// Determine if the player is targeted by an AI (ship or turret)
	{
		ShipObject * const playerContainingShip = Game::getPlayerContainingShip();

		if (   (playerContainingShip != NULL)
		    && playerContainingShip->hasCondition(TangibleObject::C_spaceCombatMusic))
		{
			enemiesTargetingPlayer = true;
		}
	}

	if (ms_lastSpaceCombatState)
	{
		if (enemiesTargetingPlayer)
			ms_spaceCombatStateTimer.reset();
		else
			return false;
	}
	else
	{
		if (enemiesTargetingPlayer)
		{
			if (ms_spaceCombatStateTimer.updateZero(elapsedTime))
			{
				ms_spaceCombatStateTimer.reset();
				return true;
			}
		}
		else
			ms_spaceCombatStateTimer.reset();
	}

	return ms_lastSpaceCombatState;
}

// ----------------------------------------------------------------------

void GameMusicManager::startCombatMusic(NetworkId const & attacker)
{
	AttackerMap::iterator iterAttacksMap = ms_attackerMap.find(CachedNetworkId(attacker));

	if (iterAttacksMap != ms_attackerMap.end())
	{
		// Attacker already exists, reset the timer

		Timer & timer = iterAttacksMap->second;
		timer.reset();
	}
	else
	{
		// New attacker

		ms_attackerMap.insert(std::make_pair(CachedNetworkId(attacker), Timer(10.0f)));
	}
}

// ----------------------------------------------------------------------

bool GameMusicManagerNamespace::verifySoundTemplate(char const * const soundTemplateName)
{
	SoundTemplate const * const soundTemplate = SoundTemplateList::fetch(soundTemplateName);
	if (!soundTemplate)
	{
		DEBUG_WARNING(true, ("GameMusicManager::verifySoundTemplate(%s): sound template does not exist", soundTemplateName));
		return false;
	}

	bool const infiniteLooping = soundTemplate->isInfiniteLooping();
	bool const is2d = soundTemplate->getAttenuationMethod() == Audio::AM_none;
	SoundTemplateList::release(soundTemplate);

	if (infiniteLooping)
	{
		DEBUG_WARNING(true, ("GameMusicManager::verifySoundTemplate(%s): refusing to play an infinite looping sound template from script", soundTemplateName));
		return false;
	}

	if (!is2d)
	{
		DEBUG_WARNING(true, ("GameMusicManager::verifySoundTemplate(%s): refusing to play a non-2d sound template from script", soundTemplateName));
		return false;
	}

	return true;
}

// ======================================================================
