// ======================================================================
//
// SwgCuiCommandParserDefault.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserDefault.h"

#include "LocalizedStringTable.h"
#include "UIUtils.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "clientGame/AlarmManager.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/NetworkScene.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/Species.h"
#include "clientGame/WhoManager.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiAliasHandler.h"
#include "clientUserInterface/CuiChatParser.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiSocialsParser.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsAlarm.h"
#include "clientUserInterface/CuiStringIdsAwayFromKeyBoard.h"
#include "clientUserInterface/CuiStringIdsWho.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "fileInterface/StdioFile.h"
#include "sharedCommandParser/CommandParserHistory.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/VTune.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/PvpData.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/FileName.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiCommandParserChatRoom.h"
#include "swgClientUserInterface/SwgCuiCommandParserCombat.h"
#include "swgClientUserInterface/SwgCuiCommandParserNebula.h"
#include "swgClientUserInterface/SwgCuiCommandParserShip.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiCommandParserVoice.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <deque>
#include <ctime>

// ======================================================================

namespace SwgCuiCommandParserDefaultNamespace
{
	namespace Commands
	{
#define MAKE_COMMAND(a) const char * const a = #a

		MAKE_COMMAND (fatal);
		MAKE_COMMAND (exception);
		MAKE_COMMAND (reloadClientTemplate);
		MAKE_COMMAND (vtune);
		MAKE_COMMAND (breakPoint);
		MAKE_COMMAND (serverhelp);
		MAKE_COMMAND (info);
		MAKE_COMMAND (who);
		MAKE_COMMAND (history);
		MAKE_COMMAND (version);
		MAKE_COMMAND (quit);
		MAKE_COMMAND (help);
		MAKE_COMMAND (spatialChat);
		MAKE_COMMAND (alias);
		MAKE_COMMAND (unalias);
		MAKE_COMMAND (save_alias);
		MAKE_COMMAND (load_alias);
		MAKE_COMMAND (mood);
		MAKE_COMMAND (moods);
		MAKE_COMMAND (chatTypes);
		MAKE_COMMAND (emoticons);
		MAKE_COMMAND (bug);
		MAKE_COMMAND (memoryReport);
		MAKE_COMMAND (memoryVerify);
		MAKE_COMMAND (garbageCollect);
		MAKE_COMMAND (brightnessContrastGamma);
		MAKE_COMMAND (resize);
		MAKE_COMMAND (earthTime);
		MAKE_COMMAND (gameTime);
		MAKE_COMMAND (alarmAddIn);
		MAKE_COMMAND (alarmAddAt);
		MAKE_COMMAND (alarmAddInRepeat);
		MAKE_COMMAND (alarmAddAtRepeat);
		MAKE_COMMAND (alarm);
		MAKE_COMMAND (alarmRemove);
		MAKE_COMMAND (alarmSnooze);
		MAKE_COMMAND (filter);
		MAKE_COMMAND (afk);
		MAKE_COMMAND (afktime);
		MAKE_COMMAND (afkmessage);
		MAKE_COMMAND (match);
		MAKE_COMMAND (flushGraphicsResources);
		MAKE_COMMAND (copyCrashReportInformation);
		MAKE_COMMAND (reloadTextures);
		MAKE_COMMAND (createMail);
#if PRODUCTION == 0
		MAKE_COMMAND (cancelTickets);
		MAKE_COMMAND (enablePopupDebugMenu);
		MAKE_COMMAND (showDebugWindow);

		//Temporary prototype thing:
		MAKE_COMMAND (mahjong);

#endif // PRODUCTION

#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
#if PRODUCTION == 0
		{ Commands::fatal,                       1, "yes",                                "Terminate with extreme prejudice."},
		{ Commands::exception,                   1, "yes",                                "Cause an access violation exception."},
		{ Commands::reloadClientTemplate,        1, "<templatename>",                     "Reloads the given client object template."},
		{ Commands::vtune,                       0, "",                                   "Toggle enable/disable vtune"},
		{ Commands::breakPoint,                  0, "",                                   "Hit breakpoint."},
		{ Commands::serverhelp,                  0, "",                                   "List serverside commands."},
		{ Commands::memoryReport,                0, "",                                   "Call MemoryManager::report ()"},
		{ Commands::memoryVerify,                0, "",                                   "Call MemoryManager::verify()"},
		{ Commands::garbageCollect,              0, "",                                   "Call Game::garbageCollect ()"},
		{ Commands::resize,                      2, "width height",                       "Resize the display"},
		{ Commands::createMail,                  1, "[1...128]",                          "Creates bulk persistent messages for yourself"},
		{ Commands::cancelTickets,               0, "",                                   "Cancels all customer service tickets"},
		{ Commands::enablePopupDebugMenu,        0, "",                                   "Enables the popup debug menu"},
		{ Commands::showDebugWindow,             1, "<0|1>",                              "Shows or hides the debug window"},
		{ Commands::mahjong,                    0, "",                                   "Open a minigame"},
#endif
		{ Commands::help,                        0, "[command]",                          "List clientside commands."},
		{ Commands::info,                        0, "",                                   "Get info."  },
		{ Commands::who,                         0, "<name> <lfg> <roleplay> <helper>",   "List players based on filters."},
		{ Commands::history,                     0, "",                                   "List command history." },
		{ Commands::version,                     0, "",                                   "Show version info." },
		{ Commands::quit,                        0, "",                                   "Quit the game." },
		{ Commands::spatialChat,                 1, "<message>",                          "Spatial chat."},
		{ Commands::alias,                       0, "[<key> [<values...>]]",              "Set or show an alias."},
		{ Commands::unalias,                     1, "<key>",                              "Remove an alias."},
		{ Commands::save_alias,                  0, "[filename]",                         "Save your aliases."},
		{ Commands::load_alias,                  0, "[filename]",                         "Load your aliases."},
		{ Commands::mood,                        0, "[mood]",                             "Set or check your mood."},
		{ Commands::moods,                       0, "",                                   "List available moods."},
		{ Commands::chatTypes,                   0, "",                                   "List available chat types."},
		{ Commands::emoticons,                   0, "",                                   "List known emoticons."},
		{ Commands::bug,                         0, "",                                   "Submit a bug"},
		{ Commands::brightnessContrastGamma,     3, "<brightness> <contrast> <gamma>",    "Set the brightness, contrast, and gamma"},
		{ Commands::alarmAddIn,                  3, "<hours> <minutes> <message> Ex. /alarmAddIn 0 30 Pizza is ready!", "Set an alarm to go off in the specified time"},
		{ Commands::alarmAddAt,                  3, "<hour> <minute> <message> Ex. /alarmAddAt 0 0 Time to go to bed...it is midnight!", "Set an alarm to go off at the specified military time"},
		{ Commands::alarmAddInRepeat,            3, "<hours> <minutes> <message> Ex. /alarmAddInRepeat 1 0 Laundry is ready!", "Set an alarm to go off in the specified time. The alarm is reset to the initial duration each time the alarm expires."},
		{ Commands::alarmAddAtRepeat,            3, "<hour> <minute> <message> Ex. /alarmAddAtRepeat 0 0 Time to go to bed...it is midnight!", "Set an alarm to go off at the specified military time every day"},
		{ Commands::alarm,                       0, "",                                   "Displays all pending alarms"},
		{ Commands::alarmRemove,                 1, "<index> Ex. /alarmRemove 0",       "Stops the specifed alarm. Type /alarm for a list of indexed alarms."},
		{ Commands::alarmSnooze,                 1, "<alarmId> <hours> <minutes> Ex. /alarmSnooze 1 0 15", "Extends the alarm time. Type /alarm for a list of alarms and ids."},
		{ Commands::earthTime,                   0, "",                                   "Displays the time on your computer"},
		{ Commands::gameTime,                    0, "",                                   "Displays the time in the game"},
		{ Commands::filter,                      0, "",                                   "Toggle the profanity filter"},
		{ Commands::afk,                         0, "",                                   "Toggle away from keyboard status"},
		{ Commands::afktime,                     0, "<minutes>",                          "Set the time in minutes until you are automatically set to away from keyboard"},
		{ Commands::afkmessage,                  0, "<message>",                          "Set the auto-response message to people who message you while you are away from the keyboard"},
		{ Commands::match,                       1, "[match] <category or type>",         "Quick single parameter matchmaking"},
		{ Commands::flushGraphicsResources,      1, "<fullReset>",                        "Flush the graphics resources, or perform a full reset"},
		{ Commands::copyCrashReportInformation,  0, "",                                   "Copy the test that would be sent with a client crash to the windows clipboard"},
		{ Commands::reloadTextures,              0, "",                                   "Reload all textures from disk"},
		{ "",                                    0, "",                                   ""} // this must be last
	};


	SwgCuiCommandParserDefault::AliasMap_t s_aliases;
	typedef stdvector<SwgCuiCommandParserDefault *>::fwd ParserVector;
	ParserVector s_parsers;

	//-----------------------------------------------------------------

	bool loadAliases (const char * const filename)
	{
		AbstractFile * const fl = new StdioFile(filename, "rb");

		if (!fl->isOpen ())
		{
			delete fl;
			return false;
		}

		const int len = fl->length ();
		char * cbuf = new char [static_cast<size_t>(len) + 1];
		const int num_read = fl->read (cbuf, len);
		UNREF (num_read);
		DEBUG_FATAL (num_read != len, ("invalid read.\n"));
		cbuf [len] = 0;

		const std::string sbuf (cbuf);
		delete[] cbuf;
		cbuf = 0;

		size_t endpos = 0;
		size_t pos = 0;
		std::string token;
		int linenum = 0;

		while (Unicode::getFirstToken (sbuf, pos, endpos, token, "\r\n"))
		{
			++linenum;
			std::string key;

			size_t mid = 0;

			if (!Unicode::getFirstToken (token, 0, mid, key) || mid == token.npos) //lint !e737 //stlport bug
			{
				DEBUG_FATAL (true, ("bogus alias file %s, line %d: '%s'.\n", filename, linenum, token.c_str ()));
				delete fl; //lint !e527 //stfu noob
				return false;
			}

			const std::string value = token.substr (mid + 1);

			SwgCuiCommandParserDefault::setAliasStatic(key, value);

			if (endpos != sbuf.npos) //lint !e737 //stlport bug
				pos = endpos + 1;
			else
				break;
		}

		delete fl;
		return true;
	}

	//-----------------------------------------------------------------

	bool saveAliases (const char * const filename)
	{
		AbstractFile * const fl = new StdioFile(filename, "wb");

		if (!fl->isOpen ())
		{
			delete fl;
			return false;
		}

		for (SwgCuiCommandParserDefault::AliasMap_t::const_iterator it = s_aliases.begin (); it != s_aliases.end (); ++it)
		{
			const Unicode::String & key = (*it).first;
			const Unicode::String & value = (*it).second;

			IGNORE_RETURN (fl->write (static_cast<int>(key.length ()), Unicode::wideToNarrow (key).c_str ()));
			IGNORE_RETURN (fl->write (1, " "));
			IGNORE_RETURN (fl->write (static_cast<int>(value.length ()), Unicode::wideToNarrow (value).c_str ()));
			IGNORE_RETURN (fl->write (1, "\n"));
		}

		delete fl;
		return true;
	}

	//----------------------------------------------------------------------

	Unicode::String s_aliasHelpPrefix = Unicode::narrowToWide ("\\#aaffaaalias\\#ffffff: ");

	//----------------------------------------------------------------------

	bool isIntTypeAndInRange(Unicode::String const & wideS, int const min, int const max, int & value)
	{
		std::string const narrowS = Unicode::wideToNarrow(wideS);

		size_t count = narrowS.size();
		for (size_t i = 0; i < count; ++i)
		{
			if (!isdigit(narrowS[i]))
			{
				return false;
			}
		}

		value = atoi(narrowS.c_str());
		if ((value >= min) && (value <= max))
			return true;

		return false;
	}

	//----------------------------------------------------------------------

	// data for Commands::who command handling
	std::map<Unicode::String, LfgDataTable::LfgNode const *> s_mapLowercaseSingleArgToSearchAttribute;
	std::map<std::pair<Unicode::String, Unicode::String>, LfgDataTable::LfgNode const *> s_mapLowercaseDoubleArgToSearchAttribute;
	std::vector<LfgDataTable::LfgNode const *> s_vectorAllTraderSubProfessions;

	//----------------------------------------------------------------------
}

using namespace SwgCuiCommandParserDefaultNamespace;

//-----------------------------------------------------------------

/**
* Predicate for searching for a command by abbreviation.
*/

struct StrEqualsAbbrevNoCase
{
	const CommandParser::String_t & str;

	explicit                StrEqualsAbbrevNoCase (const CommandParser::String_t & theStr) : str (theStr) {}

	bool                    operator() (const SwgCuiCommandParserDefault::AliasMap_t::value_type & t) const
	{
		return CommandParser::isAbbrev (str, t.first);
	}

	                        StrEqualsAbbrevNoCase (const StrEqualsAbbrevNoCase & rhs) : str (rhs.str) {}

private:
	StrEqualsAbbrevNoCase & operator= (const StrEqualsAbbrevNoCase & rhs); //lint !e754
	                        StrEqualsAbbrevNoCase ();
};

//-----------------------------------------------------------------

/**
* Predicate for searching for a command by fullname in the alias list.
*/

struct StrEqualsNoCase
{
	const CommandParser::String_t & str;

	explicit                StrEqualsNoCase (const CommandParser::String_t & theStr) : str (theStr) {}

	bool                    operator() (const SwgCuiCommandParserDefault::AliasMap_t::value_type & t) const
	{
		return !_wcsicmp (str.c_str (), t.first.c_str ());
	}

	                        StrEqualsNoCase (const StrEqualsNoCase & rhs) : str (rhs.str) {}

private:
	StrEqualsNoCase &       operator= (const StrEqualsNoCase & rhs); //lint !e754
	                        StrEqualsNoCase ();
};

//-----------------------------------------------------------------

/**
* Predicate for searching for a command by fullname in the guard set
*/

struct StrEqualsNoCaseSet
{
	const CommandParser::String_t & str;

	explicit                StrEqualsNoCaseSet (const CommandParser::String_t & theStr) : str (theStr) {}

	bool                    operator() (const SwgCuiCommandParserDefault::AliasGuardSet_t::value_type & t) const
	{
		return !_wcsicmp (str.c_str (), t.c_str ());
	}

	                        StrEqualsNoCaseSet (const StrEqualsNoCaseSet & rhs) : str (rhs.str) {}

private:
	StrEqualsNoCaseSet &       operator= (const StrEqualsNoCaseSet & rhs); //lint !e754
	                        StrEqualsNoCaseSet ();
};

//-----------------------------------------------------------------
//--
//--
//-----------------------------------------------------------------
/**
* The Command parser DOES NOT take ownership of the history.
* The Command parser DOES NOT take ownership of the alias handler.
*/

SwgCuiCommandParserDefault::SwgCuiCommandParserDefault (CommandParserHistory * history) :
CommandParser   ("", 0, "...", "all commands", 0),
m_history       (history),
m_aliasGuard    (0),
m_aliasGuardSet (new AliasGuardSet_t),
m_aliasFilename ("aliases.txt"),
m_aliasHandler  (0)
{
	createDelegateCommands (cmds);

	s_parsers.push_back(this);

#if PRODUCTION == 0
	setAliasStatic ("warpme",         "/scene warpme");
	setAliasStatic ("warp",           "/scene warp");
	setAliasStatic ("drawNetworkIds", "/scene drawNetworkIds");
#else
	removeAliasStatic (Unicode::narrowToWide ("warpme"));
	removeAliasStatic (Unicode::narrowToWide ("warp"));
	removeAliasStatic (Unicode::narrowToWide ("drawNetworkIds"));
#endif

	setAliasStatic ("browser", "/ui browser");
	setAliasStatic ("url", "/ui url");

	addSubCommand (new SwgCuiCommandParserChatRoom ());
	addSubCommand(new SwgCuiCommandParserShip);
	addSubCommand(new SwgCuiCommandParserNebula);
	addSubCommand(new SwgCuiCommandParserVoice);

	loadAliases(m_aliasFilename.c_str ());

	// begin data setup for Commands::who command handling
	if (s_mapLowercaseSingleArgToSearchAttribute.empty())
	{
		// (lfg, rp, helper) search group
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("lfg")] = LfgDataTable::getLfgLeafNodeByName("lfg");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("roleplay")] = LfgDataTable::getLfgLeafNodeByName("rp");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("rp")] = LfgDataTable::getLfgLeafNodeByName("rp");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("newbiehelper")] = LfgDataTable::getLfgLeafNodeByName("helper");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("helper")] = LfgDataTable::getLfgLeafNodeByName("helper");

		// (friend) search group
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("friend")] = LfgDataTable::getLfgLeafNodeByName("friend");

		// (imperial, rebel, neutral) search group
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("imperial")] = LfgDataTable::getLfgLeafNodeByName("imperial");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("imp")] = LfgDataTable::getLfgLeafNodeByName("imperial");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("rebel")] = LfgDataTable::getLfgLeafNodeByName("rebel");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("reb")] = LfgDataTable::getLfgLeafNodeByName("rebel");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("neutral")] = LfgDataTable::getLfgLeafNodeByName("neutral");

		// species search group
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_human))] = LfgDataTable::getLfgLeafNodeByName("human");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_rodian))] = LfgDataTable::getLfgLeafNodeByName("rodian");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_trandoshan))] = LfgDataTable::getLfgLeafNodeByName("trandoshan");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("moncalamari")] = LfgDataTable::getLfgLeafNodeByName("mon_calamari");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("moncal")] = LfgDataTable::getLfgLeafNodeByName("mon_calamari");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_wookiee))] = LfgDataTable::getLfgLeafNodeByName("wookiee");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_bothan))] = LfgDataTable::getLfgLeafNodeByName("bothan");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_twilek))] = LfgDataTable::getLfgLeafNodeByName("twilek"); // Twi'lek
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("twilek")] = LfgDataTable::getLfgLeafNodeByName("twilek");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_zabrak))] = LfgDataTable::getLfgLeafNodeByName("zabrak");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_ithorian))] = LfgDataTable::getLfgLeafNodeByName("ithorian");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::toLower(Species::getLocalizedName(SharedCreatureObjectTemplate::SP_sullustan))] = LfgDataTable::getLfgLeafNodeByName("sullustan");

		// profession search group
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("bh")] = LfgDataTable::getLfgLeafNodeByName("prof_bh");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("commando")] = LfgDataTable::getLfgLeafNodeByName("prof_commando");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("entertainer")] = LfgDataTable::getLfgLeafNodeByName("prof_entertainer");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("jedi")] = LfgDataTable::getLfgLeafNodeByName("prof_jedi");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("medic")] = LfgDataTable::getLfgLeafNodeByName("prof_medic");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("officer")] = LfgDataTable::getLfgLeafNodeByName("prof_officer");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("smuggler")] = LfgDataTable::getLfgLeafNodeByName("prof_smuggler");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("spy")] = LfgDataTable::getLfgLeafNodeByName("prof_spy");
		
		// "trader" profession is handled by special case code in the parser
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("domestics")] = LfgDataTable::getLfgLeafNodeByName("prof_trader_domestics");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("engineering")] = LfgDataTable::getLfgLeafNodeByName("prof_trader_engineering");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("munitions")] = LfgDataTable::getLfgLeafNodeByName("prof_trader_munitions");
		s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("structures")] = LfgDataTable::getLfgLeafNodeByName("prof_trader_structures");

		s_vectorAllTraderSubProfessions.clear();
		s_vectorAllTraderSubProfessions.push_back(s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("domestics")]);
		s_vectorAllTraderSubProfessions.push_back(s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("engineering")]);
		s_vectorAllTraderSubProfessions.push_back(s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("munitions")]);
		s_vectorAllTraderSubProfessions.push_back(s_mapLowercaseSingleArgToSearchAttribute[Unicode::narrowToWide("structures")]);

		for (std::map<Unicode::String, LfgDataTable::LfgNode const *>::const_iterator iter = s_mapLowercaseSingleArgToSearchAttribute.begin(); iter != s_mapLowercaseSingleArgToSearchAttribute.end(); ++iter)
			FATAL((iter->second == NULL), ("no leaf LfgDataTable::LfgNode found for /who search attribute (%s)", Unicode::wideToNarrow(iter->first).c_str()));

		for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iter2 = s_vectorAllTraderSubProfessions.begin(); iter2 != s_vectorAllTraderSubProfessions.end(); ++iter2)
			FATAL((*iter2 == NULL), ("no leaf LfgDataTable::LfgNode found for /who for an entry in the trader sub-professions list search attribute"));
	}

	if (s_mapLowercaseDoubleArgToSearchAttribute.empty())
	{
		// species search group
		s_mapLowercaseDoubleArgToSearchAttribute[std::make_pair(Unicode::narrowToWide("mon"), Unicode::narrowToWide("calamari"))] = LfgDataTable::getLfgLeafNodeByName("mon_calamari");
		s_mapLowercaseDoubleArgToSearchAttribute[std::make_pair(Unicode::narrowToWide("mon"), Unicode::narrowToWide("cal"))] = LfgDataTable::getLfgLeafNodeByName("mon_calamari");

		// profession search group
		s_mapLowercaseDoubleArgToSearchAttribute[std::make_pair(Unicode::narrowToWide("bounty"), Unicode::narrowToWide("hunter"))] = LfgDataTable::getLfgLeafNodeByName("prof_bh");

		for (std::map<std::pair<Unicode::String, Unicode::String>, LfgDataTable::LfgNode const *>::const_iterator iter = s_mapLowercaseDoubleArgToSearchAttribute.begin(); iter != s_mapLowercaseDoubleArgToSearchAttribute.end(); ++iter)
			FATAL((iter->second == NULL), ("no leaf LfgDataTable::LfgNode found for /who search attribute (%s %s)", Unicode::wideToNarrow(iter->first.first).c_str(), Unicode::wideToNarrow(iter->first.second).c_str()));
	}
	// begin data setup for Commands::who command handling
}

//----------------------------------------------------------------------

SwgCuiCommandParserDefault::~SwgCuiCommandParserDefault ()
{
	s_parsers.erase(std::remove(s_parsers.begin(), s_parsers.end(), this), s_parsers.end());

	delete m_aliasGuardSet;
	m_aliasGuardSet = 0;
	m_history = 0;
	m_aliasHandler = 0;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserDefault::setAliasStatic(const std::string & key, const std::string & value)
{
	return setAliasStatic(Unicode::narrowToWide(key), Unicode::narrowToWide(value));
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserDefault::setAliasStatic(const String_t & key, const String_t & value)
{
	//-- see if it currently exists with possibly alternate capitalization

	const AliasMap_t::iterator find_iter = std::find_if (s_aliases.begin (), s_aliases.end (), StrEqualsNoCase (key));

	// it exists
	if (find_iter != s_aliases.end ())
	{
		const Unicode::String & old_key = find_iter->first;

		//-- they differ by capitalization, remove the alias first
		if (old_key != key)
		{
			//-- make a copy of the old key here, because the iterator can be invalidated by removeAlias ()
			Unicode::String  const old_key_copy = old_key;
			const bool retval = removeAliasStatic(old_key_copy);
			WARNING (!retval, ("Unable to remove existing alias '%s'", Unicode::wideToNarrow (old_key_copy).c_str ()));
		}
	}

	//-- apply the alias to all the existing parsers
	for (ParserVector::iterator it = s_parsers.begin(); it != s_parsers.end(); ++it)
	{
		SwgCuiCommandParserDefault * const parser = *it;
		if (NULL != parser)
		{
			if (!parser->setAliasInternal(key, value))
			{
				WARNING (true, ("Unable to set alias: '%s'", Unicode::wideToNarrow(key).c_str ()));

				//-- this failed, so cancel the entire operation
				removeAliasStatic(key);
				return false;
			}
		}
	}

	s_aliases[key] = value;

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserDefault::setAliasInternal(const std::string & key, const std::string & value)
{
	return setAliasInternal(Unicode::narrowToWide (key), Unicode::narrowToWide (value));
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserDefault::setAliasInternal(const String_t & key, const String_t & value)
{
	//-- check for existance of the non-aliased command in the command parser
	{
		StringVector_t vec;
		vec.push_back(key);
		CommandParser const * const commandParser = findParser(vec, 0);
		if (NULL != commandParser)
		{
			if (commandParser->getDelegate() != this ||
				commandParser->getHelp().find(s_aliasHelpPrefix) == Unicode::String::npos)
			{
				//-- we found a subcommand that doesn't look like a proper alias
				return false;
			}
			else
				return true;
		}
	}

	//-- otherwise just go ahead and setup the alias

	StringVector_t vec;
	vec.push_back (key);

	const String_t aliasString (s_aliasHelpPrefix + value);

	if (!(addSubCommand ( new CommandParser (key, 0, String_t (), aliasString, this))))
	{
		WARNING(true, ("SwgCuiCommandParserDefault unable to addSubCommand for alias [%s]", Unicode::wideToNarrow(key).c_str()));
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserDefault::removeAliasStatic(const Unicode::String & key)
{
	AliasMap_t::iterator iter = std::find_if (s_aliases.begin (), s_aliases.end (), StrEqualsNoCase (key));

	if (iter != s_aliases.end ())
		s_aliases.erase (iter);

	for (ParserVector::iterator it = s_parsers.begin(); it != s_parsers.begin(); ++it)
	{
		SwgCuiCommandParserDefault * const parser = *it;
		if (NULL != parser)
		{
			IGNORE_RETURN(parser->removeAliasInternal(key));
		}
	}

	return true;
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserDefault::removeAliasInternal (const String_t & key)
{

	StringVector_t vec;
	vec.push_back (key);

	CommandParser * parser = findParser (vec, 0);

	if (parser != 0)
	{
		if (!removeParser (parser))
			return false;

		delete parser;
	}

	return true;
}


//-----------------------------------------------------------------

bool SwgCuiCommandParserDefault::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (originalCommand);
	UNREF (userId);

	//-----------------------------------------------------------------

	if (isCommand( argv [0], Commands::help))
	{
		CommandParser * parser = findParser (argv, 1);

		if (parser == 0)
		{
			String_t str;

			for (size_t i = 1; i < argv.size (); ++i)
			{
				IGNORE_RETURN (str.append (argv [i]).append (1, ' '));
			}

			result += getErrorMessage ( str, ERR_CMD_NOT_FOUND);
		}
		else
			parser->showHelp (result);
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::serverhelp))
	{
		Unicode::NarrowString s;

		for  (size_t i = 0; i < argv.size (); ++i)
		{
			IGNORE_RETURN (s.append (argv [i].begin (), argv[i].end ()).append (1, ' '));
		}

		ConGenericMessage m (s);
		GameNetwork::send(m, true);
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::history))
	{
		if (m_history)
		{
			result = Unicode::narrowToWide ("History:\n"
												"-----------------------------------------------\n");

			const CommandParserHistory::HistoryList_t & historyList = m_history->getHistoryList ();

			size_t index = historyList.size ();
			for (CommandParserHistory::HistoryList_t::const_reverse_iterator hiter = historyList.rbegin ();
			hiter != historyList.rend (); //lint !e55 !e81 // bad type, struct/union equality
			++hiter)
			{
				char buf [1024];
				sprintf (buf, "%3d ]  %s\n", --index, Unicode::wideToNarrow(*hiter).c_str ());
				result += Unicode::narrowToWide (buf);
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::version))
	{
		result += Unicode::narrowToWide (ApplicationVersion::getInternalVersion ());
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::breakPoint))
	{
		assert (false); //lint !e1924 !e1776
		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::memoryReport))
	{
		MemoryManager::report ();

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::memoryVerify))
	{
		MemoryManager::verify(true, true);

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::garbageCollect))
	{
		Game::garbageCollect (true);

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::brightnessContrastGamma))
	{
		const float brightness = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		const float contrast   = static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ()));
		const float gamma      = static_cast<float> (atof (Unicode::wideToNarrow (argv [3]).c_str ()));

		Graphics::setBrightnessContrastGamma(brightness, contrast, gamma);

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::resize))
	{
		const int width  = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
		const int height = static_cast<int> (atoi (Unicode::wideToNarrow (argv [2]).c_str ()));

		static int resolutions [5][2] =
		{
			800, 600,
			1024, 768,
			1280, 960,
			1280, 1024,
			1600, 1200
		};

		bool set = false;

		{
			int i;
			for (i = 0; i < 5; ++i)
			{
				if (resolutions [i][0] == width && resolutions [i][1] == height)
				{
					set = true;
					Graphics::resize (width, height);
					break;
				}
			}
		}

		if (!set)
		{
			result += Unicode::narrowToWide ("Valid resolutions are:\n");

			int i;
			for (i = 0; i < 5; ++i)
			{
				char buffer [128];
				sprintf (buffer, "  %i %i\n", resolutions [i][0], resolutions [i][1]);
				result += Unicode::narrowToWide (buffer);
			}
		}

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::createMail))
	{
		if (argv.size() > 1)
		{
			int const mailCount = clamp(1, atoi(Unicode::wideToNarrow(argv[1]).c_str()), 128);

			// Send a bunch of test email to ourself

			ClientObject *playerClientObject = Game::getClientPlayer();

			if (playerClientObject != NULL)
			{
				static int count = 0;

				for (int index = 0; index < mailCount; ++index)
				{
					char s[256];
					char b[256];

					snprintf(s, sizeof(s), "subject: %d", count++);
					snprintf(b, sizeof(b), "body: %d", count);

					std::string const recepient(Unicode::wideToNarrow(playerClientObject->getLocalizedFirstName()));

					Unicode::String const subject = Unicode::narrowToWide(s);
					Unicode::String const body = Unicode::narrowToWide(b);
					Unicode::String const &outOfBand = Unicode::emptyString;

					CuiPersistentMessageManager::sendMessage(recepient,subject, body, outOfBand);
				}
			}
			else
			{
				DEBUG_WARNING(true, ("The Game::getClientPlayer() is NULL"));
			}
		}

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], Commands::cancelTickets))
	{
		CustomerServiceManager::cancelTickets();

		return true; //lint !e527
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], Commands::enablePopupDebugMenu))
	{
		Os::enablePopupDebugMenu();
		return true; //lint !e527
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], Commands::showDebugWindow))
	{
		int const show = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		if (show)
			DebugMonitor::show();
		else
			DebugMonitor::hide();
		return true; //lint !e527
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::fatal))
	{
		CrashReportInformation::addStaticText("fatal from UI\n");
		FATAL (true, ("Terminating from UI.\n"));
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::exception))
	{
		CrashReportInformation::addStaticText("exception from UI\n");
		static_cast<int*>(0)[0] = 0;
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::quit))
	{
		if (!Game::isGodClient ())
			Game::quit ();

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::spatialChat))
	{
		Unicode::String s;

		for  (size_t i = 1; i < argv.size (); ++i)
		{
			IGNORE_RETURN (s.append (argv [i]).append (1, ' '));
		}

		if (!s.empty ())
			IGNORE_RETURN (CuiChatParser::parse (s, result, 0, false));
	}

	//-----------------------------------------------------------------
	else if (isCommand (argv [0], Commands::alias))
	{
		static const Unicode::String tabStop (Unicode::narrowToWide ("  \\@02 - "));
		// list all aliases
		if (argv.size () < 2)
		{
			result += Unicode::narrowToWide ("Aliases:\n");

			for (AliasMap_t::const_iterator iter = s_aliases.begin (); iter != s_aliases.end (); ++iter)
			{
				const Unicode::String & key = (*iter).first;
				const Unicode::String & value = (*iter).second;

				IGNORE_RETURN (result.append (key));
				IGNORE_RETURN (result.append (tabStop));
				IGNORE_RETURN (result.append (value));
				IGNORE_RETURN (result.append (1, '\n'));
			}
		}

		// list all aliases matching abbrev pattern
		else if (argv.size () < 3)
		{
			result += Unicode::narrowToWide ("Matching aliases (abbrev):\n");

			for (AliasMap_t::const_iterator iter = s_aliases.begin (); iter != s_aliases.end (); ++iter)
			{
				if (CommandParser::isAbbrev (argv[1], iter->first))
				{
					const Unicode::String & key = (*iter).first;
					const Unicode::String & value = (*iter).second;

					IGNORE_RETURN (result.append (key));
					IGNORE_RETURN (result.append (tabStop));
					IGNORE_RETURN (result.append (value));
					IGNORE_RETURN (result.append (1, '\n'));
				}
			}
		}
		else
		{
			String_t str;

			for (size_t i = 2; i < argv.size (); ++i)
			{
				if (argv[i].find (' ') != static_cast<String_t::size_type>(argv[i].npos))
				{
					IGNORE_RETURN (str.append (1, '\"'));
					IGNORE_RETURN (str.append (argv[i]));
					IGNORE_RETURN (str.append (1, '\"'));
				}
				else
					IGNORE_RETURN (str.append (argv[i]));

				IGNORE_RETURN (str.append (1, ' '));
			}

			if (!setAliasStatic(argv[1], str))
				result += Unicode::narrowToWide ("Unable to set alias.");
			else
			{
				result += Unicode::narrowToWide ("Alias set.");
				IGNORE_RETURN (saveAliases (m_aliasFilename.c_str ()));
			}
		}
	}
	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::unalias))
	{
		for (size_t i = 1; i < argv.size (); ++i)
		{
			result += Unicode::narrowToWide("Alias '") + argv[i] + (removeAliasStatic (argv[i]) ? Unicode::narrowToWide ("' removed") : Unicode::narrowToWide("' not found")) + Unicode::narrowToWide(".\n");
		}

		IGNORE_RETURN (saveAliases (m_aliasFilename.c_str ()));

	}
	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::save_alias))
	{
		if (argv.size () > 1 && !argv [1].empty ())
			m_aliasFilename = Unicode::wideToNarrow (argv [1]);

		if (saveAliases (m_aliasFilename.c_str ()))
			result += Unicode::narrowToWide( std::string("Aliases saved to: ") + m_aliasFilename);
		else
			result += Unicode::narrowToWide( std::string("FAILED to save aliases to: ") + m_aliasFilename);
	}
	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::load_alias))
	{
		if (argv.size () > 1 && !argv [1].empty ())
			m_aliasFilename = Unicode::wideToNarrow (argv [1]);

		if (loadAliases (m_aliasFilename.c_str ()))
			result += Unicode::narrowToWide( std::string("Aliases loaded from: ") + m_aliasFilename);
		else
			result += Unicode::narrowToWide( std::string("FAILED to load aliases from: ") + m_aliasFilename);
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::mood))
	{
		CreatureObject * const player = dynamic_cast<CreatureObject *>(Game::getClientPlayer ());

		static const Unicode::String s_noneString (Unicode::narrowToWide ("none"));

		if (player)
		{
			uint32 mood = 0;

			if (argv.size () > 1)
			{
				if (Unicode::caseInsensitiveCompare (s_noneString, argv [1]))
				{
					player->requestServerSetMood (0);
				}
				else
				{
					mood = MoodManagerClient::getMoodByAbbrevNoCase (argv [1]);

					if (mood)
					{
						player->requestServerSetMood  (mood);
					}
					else
					{
						result += Unicode::narrowToWide ("No such mood.");
						return true;
					}
				}
			}
			else
				mood = player->getMood ();

			result += Unicode::narrowToWide ("Your mood is: ");
			Unicode::String moodName;
			if (MoodManagerClient::getMoodName (mood, moodName))
				result += moodName;
			else
				result += s_noneString;
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::moods))
	{
		// @todo localize this
		result += Unicode::narrowToWide ("--- Mood Types: ---");

		Unicode::String abbrev;
		if (argv.size () > 1)
			abbrev = argv [1];

		typedef LocalizedStringTable::NameMap_t NameMap_t;
		const LocalizedStringTable * const table = MoodManagerClient::getMoodsTable ();

		if (table)
		{
			static const int NUM_COLS = 4;
			int col = 0;

			static const Unicode::String colTabstops [NUM_COLS] =
			{
				Unicode::String (),
					Unicode::narrowToWide ("\\@2"),
					Unicode::narrowToWide ("\\@4"),
					Unicode::narrowToWide ("\\@6")
			};

			const NameMap_t & nameMap           = table->getNameMap ();
			const NameMap_t::const_iterator end = nameMap.end ();
			for (NameMap_t::const_iterator it = nameMap.begin (); it != end; ++it)
			{
				const std::string & canonicalName  = (*it).first;

				const Unicode::String & str = Unicode::narrowToWide (canonicalName);

				const uint32 mood = MoodManager::getMoodByCanonicalName (canonicalName);

				std::string tmp;
				const bool hasAnim = MoodManagerClient::getMoodEmoteAnimation  (mood, tmp);

				if (!abbrev.empty ())
				{
					if (abbrev [0] == '*')
					{
						if (!hasAnim || mood == 0)
							continue;
					}
					else if (!CommandParser::isAbbrev (abbrev, str))
						continue;
				}

				if (col >= NUM_COLS)
					col = 0;

				if (col == 0)
					IGNORE_RETURN (result.append (1, '\n'));

				IGNORE_RETURN (result.append (colTabstops [col]));

				IGNORE_RETURN (result.append (4, ' '));

				if (hasAnim)
					IGNORE_RETURN (result.append (1, '*'));
				else
					IGNORE_RETURN (result.append (1, ' '));

				IGNORE_RETURN (result.append (2, ' '));

				result += str;

				++col;
			}
		}
		else
		{
			result += Unicode::narrowToWide ("No moods table available.\n");
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::chatTypes))
	{
		// @todo localize this
		result += Unicode::narrowToWide ("--- Chat Types: ---\n");

		Unicode::String abbrev;
		if (argv.size () > 1)
			abbrev = argv [1];


		CuiSpatialChatManager::StringVector sv;
		CuiSpatialChatManager::getLocalizedChatTypeNames (sv);

		static const int NUM_COLS = 4;
		int col = 0;

		static const Unicode::String colTabstops [NUM_COLS] =
		{
			Unicode::String (),
				Unicode::narrowToWide ("\\@2    "),
				Unicode::narrowToWide ("\\@4    "),
				Unicode::narrowToWide ("\\@6    ")
		};

		for (CuiSpatialChatManager::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const Unicode::String & str = *it;

			if (!abbrev.empty () && !CommandParser::isAbbrev (abbrev, str))
				continue;

			if (col >= NUM_COLS)
				col = 0;

			if (col == 0)
				IGNORE_RETURN (result.append (1, '\n'));

			IGNORE_RETURN (result.append (colTabstops [col]));

			result += str;
			++col;
		}
		IGNORE_RETURN (result.append (1, '\n'));

		return true;

	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], Commands::emoticons))
	{
		// @todo localize this
		result += Unicode::narrowToWide ("--- Emoticon Types: ---\n");

		Unicode::String abbrev;
		if (argv.size () > 1)
			abbrev = argv [1];
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::reloadClientTemplate))
	{
		FileName templateName(FileName::P_object, Unicode::wideToNarrow(argv[1]).c_str());
		std::string tmpString(templateName);
		Iff templateFile;
		if (!templateFile.open(tmpString.c_str(), true))
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		if (ObjectTemplateList::fetch(templateFile) == NULL)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::vtune))
	{
#if PRODUCTION == 0
		static bool s_vtuneEnabled = false;

		s_vtuneEnabled = !s_vtuneEnabled;

		result += Unicode::narrowToWide ("vtune ");

		if (s_vtuneEnabled)
		{
			VTune::resume ();
			result += Unicode::narrowToWide ("enabled");
		}
		else
		{
			VTune::pause ();
			result += Unicode::narrowToWide ("disabled");
		}
#endif

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::who))
	{
		MatchMakingCharacterPreferenceId matchMakingCharacterPreferenceId;
		BitArray & searchAttribute = matchMakingCharacterPreferenceId.getSearchAttribute();

		static LfgDataTable::LfgNode const * const levelLfgNode = LfgDataTable::getLfgNodeByName("level");
		static int const minLevel = (levelLfgNode ? levelLfgNode->minValue : 1);
		static int const maxLevel = (levelLfgNode ? levelLfgNode->maxValue : 90);

		static Unicode::String traderProfession = Unicode::narrowToWide("trader");

		for (unsigned int i = 1; i < argv.size(); ++i)
		{
			Unicode::String const argLowercase = Unicode::toLower(argv[i]);
			std::map<Unicode::String, LfgDataTable::LfgNode const *>::const_iterator iterFindSingle = s_mapLowercaseSingleArgToSearchAttribute.find(argLowercase);
			if (iterFindSingle != s_mapLowercaseSingleArgToSearchAttribute.end())
			{
				LfgDataTable::setBit(*(iterFindSingle->second), searchAttribute);
				continue;
			}

			// special handling for "trader" profession
			if (argLowercase == traderProfession)
			{
				for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iterTraderSubProfession = s_vectorAllTraderSubProfessions.begin(); iterTraderSubProfession != s_vectorAllTraderSubProfessions.end(); ++iterTraderSubProfession)
					LfgDataTable::setBit(**iterTraderSubProfession, searchAttribute);

				continue;
			}

			if (argv.size() > (i + 1))
			{
				Unicode::String const nextArgLowercase = Unicode::toLower(argv[i+1]);
				
				std::map<std::pair<Unicode::String, Unicode::String>, LfgDataTable::LfgNode const *>::const_iterator iterFindDouble = s_mapLowercaseDoubleArgToSearchAttribute.find(std::make_pair(argLowercase, nextArgLowercase));
				if (iterFindDouble != s_mapLowercaseDoubleArgToSearchAttribute.end())
				{
					LfgDataTable::setBit(*(iterFindDouble->second), searchAttribute);

					++i;
					continue;
				}
				
				// see if it's a pair of numbers for specifying a level range
				int lowLevel, highLevel;
				if (levelLfgNode && isIntTypeAndInRange(argLowercase, minLevel, maxLevel, lowLevel) && isIntTypeAndInRange(nextArgLowercase, minLevel, maxLevel, highLevel) && (lowLevel <= highLevel))
				{
					LfgDataTable::setLowHighValue(*levelLfgNode, static_cast<unsigned long>(lowLevel), static_cast<unsigned long>(highLevel), searchAttribute);
					++i;
					continue;
				}
			}

			// Sub string text, only take the first sub-string
			if (matchMakingCharacterPreferenceId.getSubString().empty() && !argLowercase.empty())
				matchMakingCharacterPreferenceId.setSubString(Unicode::wideToNarrow(argLowercase));
		}

		// default All/Any value for the lfg/helper/rp match group is All
		LfgDataTable::setAnyOrAllBit("general", false, searchAttribute);

		WhoManager::requestWhoMatch(matchMakingCharacterPreferenceId);

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::bug))
	{
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::bugReport, Unicode::emptyString));
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::alarmAddIn) ||
	         isCommand(argv[0], Commands::alarmAddInRepeat))
	{
		int const hours = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		int const minutes = atoi(Unicode::wideToNarrow(argv[2]).c_str());
		Unicode::String message;

		for (unsigned int i = 3; i < argv.size(); ++i)
		{
			message += argv[i];

			if (i < (argv.size() - 1))
			{
				message += Unicode::narrowToWide(" ");
			}
		}

		int const soundId = 0;
		bool const repeatDaily = isCommand(argv[0], Commands::alarmAddInRepeat);
		int const alarmId = AlarmManager::addAlarmIn(hours, minutes, message, soundId, repeatDaily);

		if (AlarmManager::isAlarmIdValid(alarmId))
		{
			Unicode::String alarmExpireTimeString;
			AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

			result += alarmExpireTimeString;

			Game::gameOptionChanged();
		}
		else
		{
			DEBUG_FATAL(true, ("Alarm parameters invalid <hours> %d <minutes> %d", hours, minutes));
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::alarmAddAt) ||
	         isCommand(argv[0], Commands::alarmAddAtRepeat))
	{
		int const hour = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		int const minute = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		Unicode::String message;

		for (unsigned int i = 3; i < argv.size(); ++i)
		{
			message += argv[i];

			if (i < (argv.size() - 1))
			{
				message.append(1, ' ');
			}
		}

		int const soundId = 0;
		bool const repeatDaily = isCommand(argv[0], Commands::alarmAddAtRepeat);
		int alarmId = AlarmManager::addAlarmAt(hour, minute, message, soundId, repeatDaily);

		if (AlarmManager::isAlarmIdValid(alarmId))
		{
			Unicode::String alarmExpireTimeString;
			AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

			result.append(alarmExpireTimeString);

			Game::gameOptionChanged();
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::alarm))
	{
		AlarmManager::AlarmIdVector alarms;
		AlarmManager::getAlarms(alarms);

		// Title

		result += CuiStringIdsAlarm::alarm_title.localize();
		result += Unicode::narrowToWide("\n");

		if (!alarms.empty())
		{
			// Alarm list

			AlarmManager::AlarmIdVector::const_iterator iterAlarms = alarms.begin();
			int count = 1;

			for (; iterAlarms != alarms.end(); ++iterAlarms)
			{
				int const alarmId = (*iterAlarms);
				Unicode::String alarmExpireTimeString;

				AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

				char text[256];
				sprintf(text, "%2d ", count);
				result.append(Unicode::narrowToWide(text));
				result.append(alarmExpireTimeString);

				if (iterAlarms != alarms.end())
				{
					result.append(1, '\n');
				}

				++count;
			}
		}
		else
		{
			// No alarms

			result.append(CuiStringIdsAlarm::alarm_none.localize());
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::alarmRemove))
	{
		// Figure out the alarm id

		int const alarmIndex = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1;
		AlarmManager::AlarmIdVector alarms;
		AlarmManager::getAlarms(alarms);

		int alarmId = -1;

		boolean validIndex = ((alarmIndex >= 0) &&
								(alarmIndex < static_cast<int>(alarms.size())));
		if (validIndex)
		{
			alarmId = alarms[alarmIndex];
		}

		if (AlarmManager::isAlarmIdValid(alarmId))
		{
			// Send a message that the alarm was stopped

			Unicode::String alarmExpireTimeString;
			AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

		    if (AlarmManager::removeAlarm(alarmId))
			{
				Unicode::String removeString;
				removeString.append(CuiStringIdsAlarm::alarm_remove.localize());
				removeString.append(1, ' ');
				removeString.append(alarmExpireTimeString);

				result.append(removeString);

				Game::gameOptionChanged();
			}
			else
			{
				DEBUG_WARNING(true, ("Unable to remove alarm: id %d", alarmId));
			}
		}
		else if(alarms.empty())
		{
			// No alarms

			result.append(CuiStringIdsAlarm::alarm_none.localize());
		}
		else
		{
			// Invalid alarmIndex

			CuiStringVariablesData data;
			data.digit_i = alarmIndex + 1;

			Unicode::String alarmIdInvalidString;
			CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_id_invalid, data, alarmIdInvalidString);

			result.append(alarmIdInvalidString);
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::alarmSnooze))
	{
		// Figure out the alarm id

		int const alarmIndex = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1;
		AlarmManager::AlarmIdVector alarms;
		AlarmManager::getAlarms(alarms);

		int alarmId = -1;

		if(alarms.empty())
		{
			result.append(CuiStringIdsAlarm::alarm_none.localize());
			return true;
		}

		if ((alarmIndex >= 0) &&
		    (alarmIndex < static_cast<int>(alarms.size())))
		{
			alarmId = alarms[alarmIndex];
		}

		if ((alarmId >= 0) && (argv.size() >= 4))
		{
			int const hours = atoi(Unicode::wideToNarrow(argv[2]).c_str());
			int const minutes = atoi(Unicode::wideToNarrow(argv[3]).c_str());

			if (AlarmManager::snoozeAlarm(alarmId, hours, minutes))
			{
				Unicode::String alarmExpireTimeString;
				AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

				result += alarmExpireTimeString;

				Game::gameOptionChanged();
			}
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::earthTime))
	{
		// Display the time on the computer

		Unicode::String earthTime;

		IGNORE_RETURN(CuiUtils::FormatDate(earthTime, CuiUtils::GetSystemSeconds()));

		result += CuiStringIds::earth_time.localize();
		result += Unicode::narrowToWide(" ");
		result += earthTime;

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::gameTime))
	{
		if (TerrainObject::getInstance())
		{
			int hour;
			int minute;

			TerrainObject::getInstance()->getTime(hour, minute);

#if PRODUCTION == 0
			result += Unicode::narrowToWide(FormattedString<64>().sprintf("Game Time: %02d:%02d (%f)\n",
				hour, minute, TerrainObject::getInstance()->getTime()));
#else
			result += Unicode::narrowToWide(FormattedString<64>().sprintf("Game Time: %02d:%02d\n",
				hour, minute));
#endif
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::filter))
	{
		Game::setProfanityFiltered(!Game::isProfanityFiltered());

		if (Game::isProfanityFiltered())
		{
			result += CuiStringIds::profanity_filter_on.localize();
		}
		else
		{
			result += CuiStringIds::profanity_filter_off.localize();
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::afk))
	{
		AwayFromKeyBoardManager::toggleAwayFromKeyBoard();

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::afktime))
	{
		bool argumentValid = true;

		if (argv.size() > 1)
		{
			int const automaticAwayFromKeyBoardMinutes = atoi(Unicode::wideToNarrow(argv[1]).c_str());

			// Set the new automatic away from keyboard time

			if (automaticAwayFromKeyBoardMinutes == 0)
			{
				AwayFromKeyBoardManager::setAutoAwayFromKeyBoardEnabled(false);
				Game::gameOptionChanged();
			}
			else if ((automaticAwayFromKeyBoardMinutes >= 1) &&
			         (automaticAwayFromKeyBoardMinutes <= 30))
			{
				AwayFromKeyBoardManager::setAutoAwayFromKeyBoardEnabled(true);
				AwayFromKeyBoardManager::setAutomaticAwayFromKeyBoardMinutes(automaticAwayFromKeyBoardMinutes);
				Game::gameOptionChanged();
			}
			else
			{
				argumentValid = false;
				result += CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_time_invalid.localize();
			}
		}

		if (argumentValid)
		{
			// Display the current away from keyboard time

			int const automaticAwayFromKeyBoardMinutes = AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutes();

			if (!AwayFromKeyBoardManager::isAutoAwayFromKeyBoardEnabled())
			{
				result += CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_disabled.localize();
			}
			else if ((automaticAwayFromKeyBoardMinutes >= 1) &&
					 (automaticAwayFromKeyBoardMinutes <= 30))
			{
				CuiStringVariablesData data;
				data.digit_i = automaticAwayFromKeyBoardMinutes;

				Unicode::String automaticAwayFromKeyBoardTimeString;

				if (automaticAwayFromKeyBoardMinutes == 1)
				{
					CuiStringVariablesManager::process(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_time_one, data, automaticAwayFromKeyBoardTimeString);
				}
				else
				{
					CuiStringVariablesManager::process(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_time_many, data, automaticAwayFromKeyBoardTimeString);
				}

				result += automaticAwayFromKeyBoardTimeString;
			}
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::afkmessage))
	{
		if (argv.size() > 1)
		{
			Unicode::String awayFromKeyBoardMessage;

			for (unsigned int i = 1; i < argv.size(); ++i)
			{
				awayFromKeyBoardMessage += argv[i];

				if (i < (argv.size() - 1))
				{
					awayFromKeyBoardMessage += ' ';
				}
			}

			AwayFromKeyBoardManager::setAutomaticResponseMessage(awayFromKeyBoardMessage);
			Game::gameOptionChanged();
		}

		// Display the current automatic response message

		result += AwayFromKeyBoardManager::getAutomaticResponseMessagePrefix();
		result += AwayFromKeyBoardManager::getAutomaticResponseMessage();

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], Commands::match))
	{
		if (argv.size() > 1)
		{
			Unicode::String string;

			for (unsigned int i = 1; i < argv.size(); ++i)
			{
				string += argv[i];

				if (i != (argv.size() - 1))
				{
					string += ' ';
				}
			}

			MatchMakingManager::findQuickMatch(string);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::flushGraphicsResources))
	{
		char const c = Unicode::wideToNarrow(argv[1]).c_str()[0];
		bool const fullReset = (c == 't' || c == 'T' || c == 'y' || c == 'Y' || c == '1');
		Graphics::flushResources(fullReset);

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::copyCrashReportInformation))
	{
		// build up all the text into a single buffer
		std::string crashReportInformation;
		for (int i = 0; ; ++i)
		{
			char const * entry = CrashReportInformation::getEntry(i);
			if (!entry)
				break;
			crashReportInformation += std::string(entry);
		}

		// copy it to the clipboard
		Os::copyTextToClipboard(crashReportInformation.c_str());

		return true; //lint !e527
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], Commands::reloadTextures))
	{
		TextureList::reloadTextures();
		return true; //lint !e527
	}

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand(argv[0], Commands::mahjong))
	{
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_Mahjong);
	}
#endif //PRODUCTION

	else
	{
		// look for aliases
		AliasMap_t::iterator find_iter = std::find_if (s_aliases.begin (), s_aliases.end (), StrEqualsAbbrevNoCase (argv[0]));

		// no alias found
		if (find_iter == s_aliases.end ())
		{
			result += getErrorMessage (argv [0], ERR_NO_HANDLER);
		}

		// execute the alias
		else
		{
			if (m_aliasGuard > 0)
			{
				if (std::find_if (m_aliasGuardSet->begin (), m_aliasGuardSet->end (), StrEqualsNoCaseSet (find_iter->first)) != m_aliasGuardSet->end ())
				{
					result += Unicode::narrowToWide ("Alias recursion guard hit.  Stopping alias execution.");
					return true;
				}
			}
			else
				m_aliasGuardSet->clear ();

			String_t str((*find_iter).second);
			IGNORE_RETURN (str.append (1, ' '));

			for (size_t i = 1; i < argv.size (); ++i)
			{
				IGNORE_RETURN (str.append (argv[i]).append (1, ' '));
			}

			result += Unicode::narrowToWide ("Executing alias '") + (*find_iter).first + Unicode::narrowToWide ("' -> '") + (*find_iter).second + Unicode::narrowToWide ("'...\n");

			++m_aliasGuard;
			IGNORE_RETURN (m_aliasGuardSet->insert (find_iter->first));

			bool b = false;

			if (m_aliasHandler)
				b = (m_aliasHandler->handleAlias (str, result) != ERR_NO_HANDLER);
			else
				b = (parse (NetworkId::cms_invalid, str, result) != ERR_NO_HANDLER);
			--m_aliasGuard;
			return b;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCommandParserDefault::setAliasHandler            (CuiAliasHandler * aliasHandler)
{
	m_aliasHandler = aliasHandler;
}


// ======================================================================
