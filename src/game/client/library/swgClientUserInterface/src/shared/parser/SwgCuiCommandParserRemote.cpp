// ======================================================================
//
// SwgCuiCommandParserRemote.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserRemote.h"

#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "clientGame/GameNetwork.h"


// ======================================================================

//-----------------------------------------------------------------

SwgCuiCommandParserRemote::SwgCuiCommandParserRemote  ():
CommandParser ("remote", 0, "<commands>", "remote gameserver commands", 0)
{

}
//-----------------------------------------------------------------

bool SwgCuiCommandParserRemote::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);
	
	Unicode::NarrowString s;
	
	//-----------------------------------------------------------------
	//-- ask for help by default

	if (argv.size () == 1)
	{
		s = "serverhelp";
	}
	else
	{
		for  (size_t i = 1; i < argv.size (); ++i)
		{
			Unicode::NarrowString temp(argv[i].begin(), argv[i].end());
			bool hasSpace = false;
			if (temp.find(' ') != std::string::npos)
			{
				hasSpace = true;
				IGNORE_RETURN (s.append (1, '"'));
			}
			IGNORE_RETURN (s.append (argv [i].begin (), argv[i].end ()));
			if (hasSpace)
				IGNORE_RETURN (s.append (1, '"'));
			IGNORE_RETURN (s.append(1, ' '));
		}
	}

	ConGenericMessage m (s);
	GameNetwork::send(m, true);
	
	result += Unicode::narrowToWide ("Command: ") + Unicode::narrowToWide (s) + Unicode::narrowToWide (" sent to server.\n");

	return true;
}

//-----------------------------------------------------------------
