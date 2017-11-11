//===================================================================
//
// SwgCuiCommandParserCombat.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserCombat.h"

#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/Production.h"
#include "swgSharedUtility/States.def"

#include <cstdio>
#include <vector>

//===================================================================

namespace
{
	//-- console interface
	const char* const ms_consoleDisplayActionNames      = "displayActionNames";
	const char* const ms_consoleActionName              = "actionName";
	const char* const ms_consoleAttackerPostureEndIndex = "attackerPostureEndIndex";
	const char* const ms_consoleDefenderPostureEndIndex = "defenderPostureEndIndex";
	const char* const ms_consoleDefenderDefense         = "defenderDefense";
	const char* const ms_consoleClearTarget             = "clearTarget";
	const char* const ms_consoleConceal                 = "concealShotRenderTest";

	const CommandParser::CmdInfo cmds[] =
	{
#if PRODUCTION == 0
		{ ms_consoleDisplayActionNames,      0, "",               "show action names"},
		{ ms_consoleActionName,              1, "<action name>",  "set action name (i.e. homing_test)"},
		{ ms_consoleAttackerPostureEndIndex, 1, "<0,1,2>",        "0=standing, 1=kneeling, 2=prone"},
		{ ms_consoleDefenderPostureEndIndex, 1, "<0,1,2>",        "0=standing, 1=kneeling, 2=prone"},
		{ ms_consoleDefenderDefense,         1, "<0,1,2,3>",      "0=none, 1=hit, 2=evade, 3=block"},
		{ ms_consoleClearTarget,             0, "",               "clear all combat targets, taking player out of combat"},
		{ ms_consoleConceal,                 2, "<0,1> <0,1>",    "<0,1> = visible <0,1> = authoritative"},
#endif
		{ "",                                0, "",               "" } // this must be last
	};	

	void getCombatState (Unicode::String& result)
	{
		char buffer [1024];

		sprintf (buffer, "actionName = %s\n", CuiCombatManager::getConsoleActionName ().c_str ());
		result += Unicode::narrowToWide (buffer);

		sprintf (buffer, "attackerPostureEndIndex = %i\n", CuiCombatManager::getConsoleAttackerPostureEndIndex ());
		result += Unicode::narrowToWide (buffer);

		sprintf (buffer, "defenderPostureEndIndex = %i\n", CuiCombatManager::getConsoleDefenderPostureEndIndex ());
		result += Unicode::narrowToWide (buffer);

		sprintf (buffer, "defenderDefense = %i\n", CuiCombatManager::getConsoleDefenderDefense ());
		result += Unicode::narrowToWide (buffer);
	}

	void displayActionNames (Unicode::String& result)
	{
		typedef std::set<std::string> Actions;
		Actions actions;
		
		int i;
		for (i = 0; i < ClientCombatPlaybackManager::getCombatActionKeyCount (); ++i)
		{
			actions.insert(ClientCombatPlaybackManager::getCombatActionKeyString (i));
		}

		Actions::const_iterator iterActions = actions.begin();

		for (; iterActions != actions.end(); ++iterActions)
		{
			result += Unicode::narrowToWide ("  ");
			result += Unicode::narrowToWide (iterActions->c_str ());
			result += Unicode::narrowToWide ("\n");
		}
	}
}

//===================================================================

SwgCuiCommandParserCombat::SwgCuiCommandParserCombat () : 
	CommandParser ("combat", 0, "...", "combat commands", 0)
{
	createDelegateCommands (cmds);
}

//===================================================================

bool SwgCuiCommandParserCombat::performParsing (const NetworkId & , const StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);

	if (isCommand (argv[0], ms_consoleDisplayActionNames))
	{
		displayActionNames (result);
		getCombatState (result);

		return true;
	}
	else if (isCommand (argv[0], ms_consoleActionName))
	{
		const std::string name = Unicode::wideToNarrow (argv [1]);
		CuiCombatManager::setConsoleActionName (name.c_str ());
		getCombatState (result);

		return true;
	}
	else if (isCommand (argv[0], ms_consoleAttackerPostureEndIndex))
	{
		const std::string name = Unicode::wideToNarrow (argv [1]);
		CuiCombatManager::setConsoleAttackerPostureEndIndex (atoi (name.c_str ()));
		getCombatState (result);

		return true;
	}	
	else if (isCommand (argv[0], ms_consoleDefenderPostureEndIndex))
	{
		const std::string name = Unicode::wideToNarrow (argv [1]);
		CuiCombatManager::setConsoleDefenderPostureEndIndex (atoi (name.c_str ()));
		getCombatState (result);

		return true;
	}	
	else if (isCommand (argv[0], ms_consoleDefenderDefense))
	{
		const std::string name = Unicode::wideToNarrow (argv [1]);
		CuiCombatManager::setConsoleDefenderDefense (atoi (name.c_str ()));
		getCombatState (result);

		return true;
	}	
	else if (isCommand (argv[0], ms_consoleClearTarget))
	{
		CreatureObject *const player = Game::getPlayerCreature ();
		if (player)
		{
			if (Game::getSinglePlayer())
			{
				player->setState(States::Combat, false);
				return true;
			}
		}
	}
	else if (isCommand (argv[0], ms_consoleConceal))
	{
		if (Game::getSinglePlayer())
		{
			std::string visible = Unicode::wideToNarrow(argv[1]);
			std::string local = Unicode::wideToNarrow(argv[2]);
			CreatureObject *const player = Game::getPlayerCreature ();
			if (player)
			{
				player->setCoverVisibility(atoi(visible.c_str()) != 0, atoi(local.c_str()) != 0);
				return true;
			}
		}
	}

	return false;
}

//======================================================================
