// ============================================================================
// 
// WhoManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WhoManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/Game.h"
#include "clientGame/Species.h"
#include "clientUserInterface/CuiStringIdsWho.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedGame/PvpData.h"
#include "sharedMath/PackedRgb.h"

#include "UIData.h"
#include "UITableModelDefault.h"
#include "UIUtils.h"

// ============================================================================
//
// WhoManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace WhoManagerNamespace
{ 
	typedef std::vector<MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo> GroupVector;
	typedef std::map<NetworkId, std::vector<MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo> > GroupMap;

	static void buildResultString(LfgCharacterSearchResultData const &matchingCharacterData, Unicode::String &result);
	static void addPlayerToTable (LfgCharacterSearchResultData const &matchingCharacterData, UITableModelDefault * const tableModel, int const index, MatchMakingCharacterResult const &result);

	Unicode::unicode_char_t const space = Unicode::unicode_char_t(' ');
	Unicode::String const newLine = Unicode::narrowToWide(" \n");
	Unicode::String const whiteColor = Unicode::narrowToWide ("\\#ffffff");
	Unicode::String const yellowColor = Unicode::narrowToWide ("\\#ffff00");
	Unicode::String const redColor = Unicode::narrowToWide ("\\#ff0000");
	Unicode::String const greenColor = Unicode::narrowToWide ("\\#00ff00");
};

using namespace WhoManagerNamespace;

//-----------------------------------------------------------------------------
void WhoManagerNamespace::buildResultString(LfgCharacterSearchResultData const &matchingCharacterData, Unicode::String &result)
{
	Unicode::String whiteColorCode(ClientTextManager::getColorCode(PackedRgb::solidWhite));

	result.clear();

	BitArray const & matchingAttributes = matchingCharacterData.characterInterests;

	// Friend

	if (LfgDataTable::testBit("friend", matchingAttributes))
	{
		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_tags);
		result += CuiStringIdsWho::who_friend.localize();
		result += ' ';
	}

	// Name

	result += ClientTextManager::getColorCode(ClientTextManager::TT_who_player_name);

	if (!matchingCharacterData.characterName.empty())
	{
		result += matchingCharacterData.characterName;
	}
#ifdef _DEBUG
	else
	{
		result += Unicode::narrowToWide("noName");
	}
#endif // _DEBUG

	// Guild

	if (!matchingCharacterData.guildName.empty())
	{
		result += whiteColorCode;
		result += Unicode::narrowToWide(" <");

		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_guild);
		result += Unicode::narrowToWide(matchingCharacterData.guildName.c_str());

		if (!matchingCharacterData.guildAbbrev.empty())
		{
			result += Unicode::narrowToWide(", ");
			result += Unicode::narrowToWide(matchingCharacterData.guildAbbrev);
		}

		result += whiteColorCode;
		result += Unicode::narrowToWide(">");
	}

	// Species

	Unicode::String const &species = Species::getLocalizedName(matchingCharacterData.species);
	
	if (!species.empty ())
	{
		result += whiteColorCode;
		result += Unicode::narrowToWide(" the ");

		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_species);
		result += species;
	}

	result += Unicode::narrowToWide(" (");

	// Level

	Unicode::String level;
	UIUtils::FormatLong(level, static_cast<long>(matchingCharacterData.level));
	result += level;

	// Faction

	if (PvpData::isImperialFactionId(matchingCharacterData.faction))
	{
		result += Unicode::narrowToWide(", ");
		result += StringId("lfg_n", "imperial").localize();
	}
	else if (PvpData::isRebelFactionId(matchingCharacterData.faction))
	{
		result += Unicode::narrowToWide(", ");
		result += StringId("lfg_n", "rebel").localize();
	}
	else if (PvpData::isNeutralFactionId(matchingCharacterData.faction))
	{
		result += Unicode::narrowToWide(", ");
		result += StringId("lfg_n", "neutral").localize();
	}

	// Profession

	if ((static_cast<int>(matchingCharacterData.profession) > static_cast<int>(LfgCharacterData::Prof_Unknown)) && (static_cast<int>(matchingCharacterData.profession) < static_cast<int>(LfgCharacterData::Profession_MAX)))
	{
		result += Unicode::narrowToWide(", ");
		result += LfgCharacterData::getProfessionDisplayString(matchingCharacterData.profession);
	}

	result += Unicode::narrowToWide(")");

	// Planet in the galaxy

	if (!matchingCharacterData.locationPlanet.empty())
	{
		result += whiteColorCode;
		result += Unicode::narrowToWide(" on ");

		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_planet);
		result += StringId("planet_n", matchingCharacterData.locationPlanet).localize();

		// Region in the planet

		if (!matchingCharacterData.locationRegion.empty())
		{
			result += Unicode::narrowToWide(": ");
			result += ClientTextManager::getColorCode(ClientTextManager::TT_who_region);
			result += StringId::decodeString(Unicode::narrowToWide(matchingCharacterData.locationRegion));
		}
	}

	// Player city

	if (!matchingCharacterData.locationPlayerCity.empty())
	{
		result += whiteColorCode;
		result += Unicode::narrowToWide(" in ");

		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_region);
		result += Unicode::narrowToWide(matchingCharacterData.locationPlayerCity);
	}

	// Looking for group

	if (LfgDataTable::testBit("lfg", matchingAttributes))
	{
		result += ' ';
		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_tags);
		result += CuiStringIdsWho::looking_for_group.localize();
	}
	
	// Helper

	if (LfgDataTable::testBit("helper", matchingAttributes))
	{
		result += ' ';
		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_tags);
		result += CuiStringIdsWho::helper.localize();
	}
	
	// Roleplay

	if (LfgDataTable::testBit("rp", matchingAttributes))
	{
		result += ' ';
		result += ClientTextManager::getColorCode(ClientTextManager::TT_who_tags);
		result.append(CuiStringIdsWho::roleplay.localize());
	}
}

// ============================================================================
//
// WhoManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void WhoManager::requestWhoMatch(MatchMakingCharacterPreferenceId const &id)
{
	uint32 const hash = Crc::normalizeAndCalculate("requestCharacterMatch");

	std::string value;
	id.packString(value);

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::narrowToWide(value)));
}

//-----------------------------------------------------------------------------
void WhoManager::updateWhoId(MatchMakingId const &matchMakingId)
{
	// Send the server the new match making id

	uint32 const hash = Crc::normalizeAndCalculate("setMatchMakingCharacterId");
	std::string value;
	matchMakingId.packIntString(value);

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::String(Unicode::narrowToWide(value))));
}

//-----------------------------------------------------------------------------
void WhoManager::buildResultStringList(MatchMakingCharacterResult const &matchMakingCharacterResultList, UnicodeStringList &unicodeStringList)
{
	Unicode::String yourself;
	Unicode::String whoString;

	unicodeStringList.clear();

	std::vector<LfgCharacterSearchResultData>::const_iterator iterMatchMakingCharacterResultList = matchMakingCharacterResultList.m_matchingCharacterData.begin();
	for (; iterMatchMakingCharacterResultList != matchMakingCharacterResultList.m_matchingCharacterData.end(); ++iterMatchMakingCharacterResultList)
	{
		LfgCharacterSearchResultData const & matchingCharacterData = *iterMatchMakingCharacterResultList;

		if (   (Game::getClientPlayer() != NULL)
		    && (Game::getClientPlayer()->getLocalizedName() == matchingCharacterData.characterName))
		{
			buildResultString(matchingCharacterData, yourself);
		}
		else
		{
			buildResultString(matchingCharacterData, whoString);
			unicodeStringList.push_back(whoString);
		}
	}

	// Yourself
	if (!yourself.empty())
		unicodeStringList.push_back(yourself);
}

//-----------------------------------------------------------------------------

void WhoManager::buildResultTable (MatchMakingCharacterResult const &matchMakingCharacterResultList, UITableModelDefault * const tableModel)
{
	if (!tableModel)
		return;

	int whoResultCount = 0;

	std::vector<LfgCharacterSearchResultData>::const_iterator iterMatchMakingCharacterResultList = matchMakingCharacterResultList.m_matchingCharacterData.begin();
	for (; iterMatchMakingCharacterResultList != matchMakingCharacterResultList.m_matchingCharacterData.end(); ++iterMatchMakingCharacterResultList, ++whoResultCount)
	{
		addPlayerToTable(*iterMatchMakingCharacterResultList, tableModel, whoResultCount, matchMakingCharacterResultList);
	}
}

//-----------------------------------------------------------------------------

void WhoManagerNamespace::addPlayerToTable(LfgCharacterSearchResultData const &characterData, UITableModelDefault * const tableModel, int const index, MatchMakingCharacterResult const &result)
{
	std::string const rowID = characterData.characterId.getValueString();
	Unicode::String const playerProf = LfgCharacterData::getProfessionDisplayString(characterData.profession);
	Unicode::String const playerSpecies = Species::getLocalizedName(characterData.species);
	Unicode::String playerGuild = Unicode::narrowToWide(characterData.guildName);

	if (!playerGuild.empty() && !characterData.guildAbbrev.empty())
	{
		playerGuild += Unicode::narrowToWide(", ");
		playerGuild += Unicode::narrowToWide(characterData.guildAbbrev);
	}

	Unicode::String playerName = characterData.characterName;
	Unicode::String::size_type pos = playerName.find(space);

	if (pos != Unicode::String::npos)
		playerName = playerName.substr(0, pos);

	Unicode::String playerLocation;

	// Planet in the galaxy
	if (!characterData.locationPlanet.empty())
	{
		playerLocation += StringId("planet_n", characterData.locationPlanet).localize();

		// Region in the planet
		if (!characterData.locationRegion.empty())
		{
			playerLocation += Unicode::narrowToWide(": ");
			playerLocation += StringId::decodeString(Unicode::narrowToWide(characterData.locationRegion));
		}
	}

	// Player city
	if (!characterData.locationPlayerCity.empty())
	{
		if (!playerLocation.empty())
			playerLocation += Unicode::narrowToWide(", ");

		playerLocation += Unicode::narrowToWide(characterData.locationPlayerCity);
	}

	Unicode::String playerLevel;
	UIUtils::FormatLong(playerLevel, characterData.level);
	
	Unicode::String playerFaction;
	Unicode::String factionIcon;	
	
	if (PvpData::isImperialFactionId(characterData.faction))
	{
		factionIcon = Unicode::narrowToWide("/Styles.Icon.Faction.imperial_logo");
		playerFaction = StringId("lfg_n", "imperial").localize();
	}
	else if (PvpData::isRebelFactionId(characterData.faction))
	{
		factionIcon = Unicode::narrowToWide("/Styles.Icon.Faction.rebel_logo");
		playerFaction = StringId("lfg_n", "rebel").localize();
	}
	else
	{
		playerFaction = StringId("lfg_n", "neutral").localize();
	}

	UIData * cellData = tableModel->AppendCell(0, rowID.c_str(), playerName);

	if (!cellData)
		return;

	UIData * cellDataPlayerLevel = tableModel->AppendCell(1, rowID.c_str(), playerLevel);
	if (cellDataPlayerLevel)
	{
		if ((characterData.level >= 0) && (characterData.level <= 9))
			cellDataPlayerLevel->SetProperty(UITableModelDefault::DataProperties::SortValue, Unicode::narrowToWide("0") + playerLevel);
		else
			cellDataPlayerLevel->SetProperty(UITableModelDefault::DataProperties::SortValue, playerLevel);
	}

	tableModel->AppendCell(2, rowID.c_str(), factionIcon);
	tableModel->AppendCell(3, rowID.c_str(), playerProf);
	tableModel->AppendCell(4, rowID.c_str(), playerLocation);

	cellData->SetProperty(UILowerString("FirstName"), playerName);

	Unicode::String tooltip;
	
	tooltip += yellowColor + Unicode::narrowToWide("Name: \t\t") + whiteColor + playerName + newLine;
	tooltip += yellowColor + Unicode::narrowToWide("Profession: \t") + whiteColor + playerProf + newLine;
	tooltip += yellowColor + Unicode::narrowToWide("Species: \t") + whiteColor + playerSpecies + newLine;
	tooltip += yellowColor + Unicode::narrowToWide("Level: \t\t") + whiteColor + playerLevel + newLine;
	
	if (!playerGuild.empty())
		tooltip += yellowColor + Unicode::narrowToWide("Guild: \t\t") + whiteColor + playerGuild + newLine;

	tooltip += yellowColor + Unicode::narrowToWide("Faction: \t") + whiteColor + playerFaction + newLine;

	if (!characterData.ctsSourceGalaxy.empty())
	{
		Unicode::String ctsSourceGalaxy;
		for (std::set<std::string>::const_iterator iter = characterData.ctsSourceGalaxy.begin(); iter != characterData.ctsSourceGalaxy.end(); ++iter)
		{
			if (!ctsSourceGalaxy.empty())
				ctsSourceGalaxy += Unicode::narrowToWide(", ");

			ctsSourceGalaxy += Unicode::narrowToWide(*iter);
		}

		tooltip += yellowColor + Unicode::narrowToWide("Transferred From: ") + whiteColor + ctsSourceGalaxy + newLine;
	}

	if (!playerLocation.empty())
		tooltip += yellowColor + Unicode::narrowToWide("Location: \t") + whiteColor + playerLocation + newLine;

	GroupMap::const_iterator groupIter = result.m_matchingCharacterGroup.find(characterData.groupId); 

	if (groupIter != result.m_matchingCharacterGroup.end())
	{
		tooltip += newLine + redColor + Unicode::narrowToWide("Group Members: ") + newLine + whiteColor;

		GroupVector const & groupMembers = (groupIter->second);
		GroupVector::const_iterator memberIter = groupMembers.begin();

		Unicode::String numMembers;
		UIUtils::FormatLong(numMembers, groupMembers.size());

		for (; memberIter != groupMembers.end(); memberIter++)
		{
			if ((*memberIter).groupMemberId == characterData.characterId)
			{
				if (memberIter == groupMembers.begin())
					playerName = redColor + Unicode::narrowToWide("(L ")  + numMembers + Unicode::narrowToWide(") ") + whiteColor + playerName;
				else
					playerName = greenColor + Unicode::narrowToWide("(G ")  + numMembers + Unicode::narrowToWide(") ") + whiteColor + playerName;

				cellData->SetProperty (UITableModelDefault::DataProperties::Value, playerName);
				cellData->SetPropertyBoolean(UILowerString("IsGrouped"), true);
				continue;
			}

			Unicode::String memberName = Unicode::narrowToWide((*memberIter).groupMemberName);
			
			Unicode::String::size_type pos = memberName.find(space);

			if (pos != Unicode::String::npos)
				memberName = memberName.substr(0, pos);

			if (memberIter == groupMembers.begin())
				memberName = redColor + Unicode::narrowToWide("(L)") + whiteColor + memberName;

			Unicode::String const memberProf = LfgCharacterData::getProfessionDisplayString(static_cast<LfgCharacterData::Profession>((*memberIter).groupMemberProfession));
			Unicode::String memberLevel;
			
			if ((*memberIter).groupMemberLevel > 0)
				UIUtils::FormatLong(memberLevel, (*memberIter).groupMemberLevel);

			if (memberProf == Unicode::narrowToWide("NPC"))
			{
				StringId::LocUnicodeString npcDisplayName;
				if (StringId((*memberIter).groupMemberName).localize(npcDisplayName))
					memberName = npcDisplayName;
			}
						
			tooltip += memberName + space + space + yellowColor + memberLevel + space + space + whiteColor + memberProf + newLine; 
		}
	}
	
	std::map<NetworkId, Unicode::String>::const_iterator bioIter = result.m_matchingCharacterBiography.find(characterData.characterId);

	if (bioIter != result.m_matchingCharacterBiography.end())
		cellData->SetProperty(UILowerString("PlayerBio"), (*bioIter).second);

	for (int i = 0; i < tableModel->GetColumnCount(); i++ )
	{
		UIData * const data = tableModel->GetCellDataLogical (index, i);

		if (data)
		{
			data->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tooltip);
		}	
	}
}

// ============================================================================
