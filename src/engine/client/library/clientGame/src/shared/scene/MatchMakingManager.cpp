// ============================================================================
//
// MatchMakingManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/MatchMakingManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingResult.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include <map>
#include <set>
#include <vector>

// ============================================================================
namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const MatchMakingManager::Messages::MatchFoundResult::MatchMakingResult &, MatchMakingManager::Messages::MatchFoundResult>
			matchFoundResult;

		MessageDispatch::Transceiver<const MatchMakingManager::Messages::MatchFoundResultString::MatchMakingResultString &, MatchMakingManager::Messages::MatchFoundResultString>
			matchFoundResultString;

		MessageDispatch::Transceiver<const MatchMakingManager::Messages::QuickMatchFoundResultString::QuickMatchResultString &, MatchMakingManager::Messages::QuickMatchFoundResultString>
			quickMatchFoundResultString;
	}
}

// ============================================================================
//
// MatchMakingManagerNamespace
//
// ============================================================================

namespace MatchMakingManagerNamespace
{
	typedef std::set<NetworkId>                           DisplayedPlayerMatches;
	typedef std::map<NetworkId, std::pair<float, float> > QueuedPlayerMatches;
	typedef std::vector<NetworkId>                        ObjectVector;
	//typedef std::map<NetworkId, Object *> Particles;

	DisplayedPlayerMatches     s_displayedPlayerMatches;                  // Player matches already displayed to the player
	QueuedPlayerMatches        s_queuedPlayerMatches;                     // Player matches not yet displayed to the player
	ObjectVector               s_purgeList;                               // Used to delete items
	//Particles                  s_particles;                                // Current particles on the matched players
	float const                s_matchDisplayWait = 20.0f;                // Time between displaying successive matches
	float                      s_matchDisplayTimer = s_matchDisplayWait;
	unsigned int const         s_maxQueuedMatches = 100;
	float const                s_cleanerUpdateTime = 4.0f;
	float                      s_cleanerTimer = s_cleanerUpdateTime;
	float                      s_matchMakingUpdateTime = 0.0f;
	float                      s_matchMakingTimer = 0.0f;
	float                      s_matchMakingSensitivity;
	MatchMakingId              s_preferenceId;
	std::string                s_preferenceIdIntString;
	MatchMakingManager::Categories s_categories;

	void resetMatches();
};

using namespace MatchMakingManagerNamespace;

//-----------------------------------------------------------------------------
void MatchMakingManagerNamespace::resetMatches()
{
	s_displayedPlayerMatches.clear();
	s_queuedPlayerMatches.clear();
}

// ============================================================================
//
// MatchMakingManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void MatchMakingManager::install()
{
	InstallTimer const installTimer("MatchMakingManager::install");

	CurrentUserOptionManager::registerOption(s_matchMakingSensitivity, "ClientGame", "MatchMakingSensitivity");
	CurrentUserOptionManager::registerOption(s_matchMakingUpdateTime, "ClientGame", "MatchMakingUpdateTime");
	CurrentUserOptionManager::registerOption(s_preferenceIdIntString, "ClientGame", "MatchMakingPersonalPreferenceId");

	// Load up the categories and types

	char const matchMakingFile[] = "datatables/matchmaking/matchmaking.iff";
	Iff matchMakingIff;

	if (matchMakingIff.open(matchMakingFile, true))
	{
		DataTable dataTable;
		dataTable.load(matchMakingIff);

		int const rowCount = dataTable.getNumRows();

		// Get all the categories from the data table

		for (int i = 0; i < rowCount; ++i)
		{
			int const row = i;

			std::string category(Unicode::wideToNarrow(Unicode::toLower(StringId("ui_cmnty", dataTable.getStringValue("category", row)).localize())));
			std::string type(Unicode::wideToNarrow(Unicode::toLower(StringId("ui_cmnty", dataTable.getStringValue("type", row)).localize())));

			Categories::iterator iterCategories = s_categories.find(category);

			if (iterCategories != s_categories.end())
			{
				TypeNameToBitMap::iterator iterTypeNameToBitMap = iterCategories->second.find(type);

				if (iterTypeNameToBitMap != iterCategories->second.end())
				{
					DEBUG_WARNING(true, ("Dupliate type entry in the matchmaking data table."));
				}
				else
				{
					iterCategories->second.insert(std::make_pair(type, row));
				}
			}
			else
			{
				TypeNameToBitMap typeNameToBitMap;
				typeNameToBitMap.insert(std::make_pair(type, row));

				s_categories.insert(std::make_pair(category, typeNameToBitMap));
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the matchmaking data table: %s", matchMakingFile));
	}

	//DEBUG_REPORT_LOG(true, ("MatchMakingManager::install() - <id> %s\n", s_preferenceId.getDebugIntString().c_str()));

	ExitChain::add(MatchMakingManager::remove, "MatchMakingManager::remove", 0, false);
}

//-----------------------------------------------------------------------------
void MatchMakingManager::remove()
{
	s_displayedPlayerMatches.clear();
	s_queuedPlayerMatches.clear();
	s_purgeList.clear();

	//// Delete any outstanding particles
	//
	//Particles::iterator iterParticles = s_particles.begin();
	//
	//for (; iterParticles != s_particles.end(); ++iterParticles)
	//{
	//	delete iterParticles->second;
	//	iterParticles->second = NULL;
	//}
	//
	//s_particles.clear();
}

//-----------------------------------------------------------------------------
void MatchMakingManager::alter(float const deltaTime)
{
	bool checkMatch = false;

	if (s_matchMakingUpdateTime > 0.0f)
	{
		s_matchMakingTimer -= deltaTime;

		if (s_matchMakingTimer <= 0.0f)
		{
			// Reset the timer

			s_matchMakingTimer = s_matchMakingUpdateTime;
			checkMatch = true;
		}
	}

	if (checkMatch)
	{
		Object *object = Game::getPlayer();

		if (object != NULL)
		{
			findMatches(object);
		}
	}

	s_cleanerTimer -= deltaTime;

	if (s_cleanerTimer <= 0.0f)
	{
		s_cleanerTimer = s_cleanerUpdateTime;

		// See if we need to remove any objects because they have become NULL (not on the client anymore)

		DisplayedPlayerMatches::iterator iterDisplayedPlayerMatches = s_displayedPlayerMatches.begin();

		for (; iterDisplayedPlayerMatches != s_displayedPlayerMatches.end(); ++iterDisplayedPlayerMatches)
		{
			NetworkId const &networkId = (*iterDisplayedPlayerMatches);
			Object *object = NetworkIdManager::getObjectById(networkId);

			if (object == NULL)
			{
				// Get the match ready for deletion

				s_purgeList.push_back(networkId);

				//// Find the associated particles
				//
				//Particles::iterator iterParticles = s_particles.find(iterPlayerMatches->first);
				//
				//if (iterParticles != s_particles.end())
				//{
				//	ParticleEffectAppearance *particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(iterParticles->second->getAppearance());
				//	particleEffectAppearance->setEnabled(false);
				//}
			}
		}

		// Remove any old matches

		ObjectVector::iterator iterPurgeList = s_purgeList.begin();

		for (; iterPurgeList != s_purgeList.end(); ++iterPurgeList)
		{
			NetworkId const &networkId = (*iterPurgeList);
			DisplayedPlayerMatches::iterator iterDisplayedPlayerMatches = s_displayedPlayerMatches.find(networkId);

			if (iterDisplayedPlayerMatches != s_displayedPlayerMatches.end())
			{
				s_displayedPlayerMatches.erase(iterDisplayedPlayerMatches);
			}
		}

		s_purgeList.clear();
	}

	{
		// See if we need to display some new matches

		s_matchDisplayTimer -= deltaTime;

		if (s_matchDisplayTimer <= 0.0f)
		{
			// Reset the timer

			s_matchDisplayTimer = s_matchDisplayWait;

			if (s_queuedPlayerMatches.size() > 0)
			{
				// Find a random entry

				QueuedPlayerMatches::iterator iterQueuedPlayerMatches = s_queuedPlayerMatches.begin();
				int const index = rand() % static_cast<int>(s_queuedPlayerMatches.size());

				for (int i = 0; i < index; ++i)
				{
					++iterQueuedPlayerMatches;
				}

				NetworkId const &networkId = (iterQueuedPlayerMatches->first);

				ClientObject *clientObject = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(networkId));

				if (clientObject != NULL)
				{
					// Send a system message only the first time there is a new match

					CuiStringVariablesData data;
					float const percent = iterQueuedPlayerMatches->second.first;
					data.digit_i = static_cast<int>(percent * 100.0f);
					data.targetName = clientObject->getLocalizedName();

					Unicode::String matchMakingResultString;
					CuiStringVariablesManager::process(CuiStringIdsCommunity::match_found_prose, data, matchMakingResultString);
					//CuiSystemMessageManager::sendFakeSystemMessage(resultStr);
					Transceivers::matchFoundResultString.emitMessage(matchMakingResultString);

					MatchMakingResult matchMakingResult;

					matchMakingResult.setPlayerName(clientObject->getLocalizedName());
					matchMakingResult.setSystemTime(CuiUtils::GetSystemSeconds());
					matchMakingResult.setPreferenceToProfileMatchPercent(iterQueuedPlayerMatches->second.first);
					matchMakingResult.setProfileMatchPercent(iterQueuedPlayerMatches->second.second);

					Transceivers::matchFoundResult.emitMessage(matchMakingResult);

					//// Add a particle system
					//
					//Object *newObject = new Object();
					//NOT_NULL(newObject);
					//
					//RenderWorld::addObjectNotifications(*newObject);
					//creatureObject->addChildObject_o(newObject);
					//
					//ParticleEffectAppearance *particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(AppearanceTemplateList::createAppearance("appearance/pt_music_loop_01.prt"));
					//particleEffectAppearance->setAutoDelete(false);
					//newObject->setAppearance(particleEffectAppearance);
					//s_particles.insert(std::make_pair(creatureObject->getNetworkId(), newObject));
				}

				// Mark this network id as displayed

				s_displayedPlayerMatches.insert(networkId);

				// Remove the displayed item from the queued list

				s_queuedPlayerMatches.erase(iterQueuedPlayerMatches);
			}
		}

	}

	//{
	//	// Particles
	//
	//	Particles::iterator iterParticles = s_particles.begin();
	//
	//	for (; iterParticles != s_particles.end(); ++iterParticles)
	//	{
	//		ParticleEffectAppearance *particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(iterParticles->second->getAppearance());
	//
	//		if (particleEffectAppearance->isDeletable())
	//		{
	//			s_purgeList.push_back(iterParticles->first);
	//		}
	//	}
	//}
	//
	//{
	//	// Remove any old particles
	//
	//	CreatureObjectVector::iterator iterPurgeList = s_purgeList.begin();
	//
	//	for (; iterPurgeList != s_purgeList.end(); ++iterPurgeList)
	//	{
	//		Particles::iterator iterParticles = s_particles.find(*iterPurgeList);
	//
	//		if (iterParticles != s_particles.end())
	//		{
	//			s_particles.erase(iterParticles);
	//		}
	//	}
	//
	//	s_purgeList.clear();
	//}
}

//-----------------------------------------------------------------------------
void MatchMakingManager::findMatches(Object *object)
{
	CreatureObject *playerCreatureObject = dynamic_cast<CreatureObject *>(object);

	if (playerCreatureObject != NULL)
	{
		const int objectCount = ClientWorld::getNumberOfObjects(WOL_Tangible);

		for (int i = 0; i < objectCount; ++i)
		{
			Object const * const currentObject = const_cast<Object const * const>(ClientWorld::getObject(WOL_Tangible, i));
			CreatureObject const * const creatureObject = dynamic_cast<CreatureObject const * const>(currentObject);

			if ((creatureObject != NULL) &&
				(creatureObject != playerCreatureObject) &&
				(creatureObject->getPlayerObject() != NULL) &&
				(playerCreatureObject->getPlayerObject() != NULL))
			{
				const float meToThemMatchPercent = MatchMakingManager::getPreferenceId().getMatchPercent(creatureObject->getPlayerObject()->getMatchMakingPersonalId());

				//DEBUG_REPORT_LOG(true, ("MatchMakingManager::findMatches() - <source> %s <target> %s <percent> %d\n", MatchMakingManager::getPreferenceId().getDebugIntString().c_str(), creatureObject->getPlayerObject()->getMatchMakingPersonalId().getDebugIntString().c_str(), static_cast<int>(meToThemMatchPercent * 100.0f)));
				//DEBUG_REPORT_LOG(true, ("MatchMakingManager::findMatches() - <target> %s\n", creatureObject->getPlayerObject()->getMatchMakingPersonalId().isBitSet(MatchMakingId::B_anonymous) ? "anonymous" : "searchable"));

				if (meToThemMatchPercent >= s_matchMakingSensitivity &&
				    !CommunityManager::isIgnored(creatureObject->getLocalizedName()))
				{
					// Only send a match message if the creature object has not alredy been announced

					QueuedPlayerMatches::iterator iterQueuedPlayerMatches = s_queuedPlayerMatches.find(creatureObject->getNetworkId());
					DisplayedPlayerMatches::iterator iterDisplayedPlayerMatches = s_displayedPlayerMatches.find(creatureObject->getNetworkId());

					if ((iterDisplayedPlayerMatches == s_displayedPlayerMatches.end()) &&
						(iterQueuedPlayerMatches == s_queuedPlayerMatches.end()) &&
						(s_queuedPlayerMatches.size() < s_maxQueuedMatches))
					{
						float themToMeMatchPercent = playerCreatureObject->getPlayerObject()->getMatchMakingPersonalId().getMatchPercent(creatureObject->getPlayerObject()->getMatchMakingPersonalId());

						// Add the new match to the list queue

						s_queuedPlayerMatches.insert(std::make_pair(currentObject->getNetworkId(), std::make_pair(meToThemMatchPercent, themToMeMatchPercent)));
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void MatchMakingManager::setUpdateTime(const float seconds)
{
	if ((seconds >= 0.0f) &&
	    (s_matchMakingUpdateTime != seconds))
	{
		s_matchMakingUpdateTime = seconds;
		s_matchMakingTimer = seconds;
	}
}

//-----------------------------------------------------------------------------
float MatchMakingManager::getUpdateTime()
{
	return s_matchMakingUpdateTime;
}

// ----------------------------------------------------------------------

void MatchMakingManager::setSensitivity(const float percent)
{
	s_matchMakingSensitivity = clamp(0.0f, percent, 1.0f);
}

// ----------------------------------------------------------------------

float MatchMakingManager::getSensitivity()
{
	return s_matchMakingSensitivity;
}

//-----------------------------------------------------------------------------

bool MatchMakingManager::isSearchableByCtsSourceGalaxy()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isSearchableByCtsSourceGalaxy();
	}

	return result;
}

//-----------------------------------------------------------------------------

bool MatchMakingManager::isDisplayLocationInSearchResults()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isDisplayLocationInSearchResults();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isAnonymous()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isAnonymous();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isRolePlay()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isRolePlay();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isHelper()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isHelper();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isLookingForGroup()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isLookingForGroup();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isLookingForWork()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isLookingForWork();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isAwayFromKeyBoard()
{
	bool result = false;
	PlayerObject const *playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isAwayFromKeyBoard();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingManager::isDisplayingFactionRank()
{
	bool result = false;
	PlayerObject const * const playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		result = playerObject->isDisplayingFactionRank();
	}

	return result;
}


//-----------------------------------------------------------------------------
void MatchMakingManager::setPreferenceId(MatchMakingId const &matchMakingId)
{
	s_preferenceId = matchMakingId;

	s_preferenceId.packIntString(s_preferenceIdIntString);

	resetMatches();
}

//-----------------------------------------------------------------------------
MatchMakingId const &MatchMakingManager::getPreferenceId()
{
	return s_preferenceId;
}

//-----------------------------------------------------------------------------
void MatchMakingManager::findQuickMatch(Unicode::String const &quickMatchString)
{
	MatchMakingId quickMatchMakingId;

	std::string string(Unicode::wideToNarrow(Unicode::toLower(quickMatchString)));

	// Check for an exact category match

	MatchMakingManager::Categories::const_iterator iterCategories = s_categories.find(string);

	if (iterCategories != s_categories.end())
	{
		// A category match was found, so mark all the bits for the types in
		// this category

		TypeNameToBitMap::const_iterator iterTypeNameToBitMap = iterCategories->second.begin();

		for (; iterTypeNameToBitMap != iterCategories->second.end(); ++iterTypeNameToBitMap)
		{
			int const bit = iterTypeNameToBitMap->second;

			quickMatchMakingId.setBit(static_cast<MatchMakingId::Bit>(bit));
		}
	}
	else
	{
		// No exact category match was found so search for an exact type match

		int bit = -1;
		iterCategories = s_categories.begin();

		for (; iterCategories != s_categories.end(); ++iterCategories)
		{
			TypeNameToBitMap::const_iterator iterTypeNameToBitMap = iterCategories->second.find(string);

			if (iterTypeNameToBitMap != iterCategories->second.end())
			{
				bit = iterTypeNameToBitMap->second;
				break;
			}
		}

		if (bit != -1)
		{
			// An exact type match was found

			quickMatchMakingId.setBit(static_cast<MatchMakingId::Bit>(bit));
		}
		else
		{
			// No exact type match was found so search for a partial category match

			iterCategories = s_categories.begin();

			for (; iterCategories != s_categories.end(); ++iterCategories)
			{
				char const *category = iterCategories->first.c_str();

				if (strstr(category, string.c_str()) != NULL)
				{
					break;
				}
			}

			if (iterCategories != s_categories.end())
			{
				// A category match was found, so mark all the bits for the types in
				// this category

				TypeNameToBitMap::const_iterator iterTypeNameToBitMap = iterCategories->second.begin();

				for (; iterTypeNameToBitMap != iterCategories->second.end(); ++iterTypeNameToBitMap)
				{
					quickMatchMakingId.setBit(static_cast<MatchMakingId::Bit>(iterTypeNameToBitMap->second));
				}
			}
			else
			{
				// No category partial match was found, so do a type partial match

				iterCategories = s_categories.begin();

				for (; iterCategories != s_categories.end(); ++iterCategories)
				{
					TypeNameToBitMap::const_iterator iterTypeNameToBitMap = iterCategories->second.begin();

					for (; iterTypeNameToBitMap != iterCategories->second.end(); ++iterTypeNameToBitMap)
					{
						char const *type = iterTypeNameToBitMap->first.c_str();

						if (strstr(type, string.c_str()) != NULL)
						{
							// Found a partial type match

							quickMatchMakingId.setBit(static_cast<MatchMakingId::Bit>(iterTypeNameToBitMap->second));
							break;
						}
					}
				}
			}
		}
	}

	Unicode::String resultString;

	{
		CuiStringVariablesData data;
		data.sourceName = quickMatchString;

		Unicode::String quickMatchTitleString;
		CuiStringVariablesManager::process(CuiStringIdsCommunity::quick_match_title, data, quickMatchTitleString);

		resultString += quickMatchTitleString;
		resultString += '\n';
	}

	if (quickMatchMakingId.getSetBitCount() > 0)
	{
		int matchCount = 0;
		//DEBUG_REPORT_LOG(true, ("Quick Match - <parameters> %s\n", quickMatchMakingId.getDebugIntString().c_str()));
		PlayerObject const * const localPlayerObject = Game::getConstPlayerObject();

		if (localPlayerObject != NULL)
		{
			const int objectCount = ClientWorld::getNumberOfObjects(WOL_Tangible);

			for (int i = 0; i < objectCount; ++i)
			{
				Object const * const currentObject = const_cast<Object const * const>(ClientWorld::getObject(WOL_Tangible, i));
				CreatureObject const * const creatureObject = dynamic_cast<CreatureObject const * const>(currentObject);

				if ((creatureObject != NULL) &&
					(creatureObject->getPlayerObject() != NULL))
				{
					// See if we are ignoring the player

					if (!CommunityManager::isIgnored(creatureObject->getLocalizedName()))
					{
						float matchPercent = quickMatchMakingId.getMatchPercent(creatureObject->getPlayerObject()->getMatchMakingPersonalId());

						if (matchPercent > 0.0f)
						{
							//DEBUG_REPORT_LOG(true, ("Quick Match - <name> %s\n", Unicode::wideToNarrow(creatureObject->getLocalizedName()).c_str()));
							resultString += creatureObject->getLocalizedName();
							resultString += '\n';
							++matchCount;
						}
					}
				}
			}
		}

		if (matchCount <= 0)
		{
			resultString += CuiStringIdsCommunity::quick_match_none.localize();
		}
		else if (matchCount == 1)
		{
			resultString += CuiStringIdsCommunity::quick_match_one.localize();
		}
		else
		{
			// More than one match

			CuiStringVariablesData data;
			data.digit_i = matchCount;

			Unicode::String manyMatchString;
			CuiStringVariablesManager::process(CuiStringIdsCommunity::quick_match_many, data, manyMatchString);

			resultString += manyMatchString;
		}
	}
	else
	{
		resultString += CuiStringIdsCommunity::quick_match_bad_parameters.localize();
		//DEBUG_REPORT_LOG(true, ("Quick Match - %s", "no matches\n"));
	}

	Transceivers::matchFoundResultString.emitMessage(resultString);
}

//-----------------------------------------------------------------------------
void MatchMakingManager::loadUserSettings()
{
	s_preferenceId.unPackIntString(s_preferenceIdIntString);
}

// ============================================================================
