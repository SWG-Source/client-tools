// ======================================================================
//
// SwgCuiCommandParserNet.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserNet.h"

#include "clientGame/ConnectionManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Production.h"
#include "sharedNetworkMessages/ChatInstantMessageToCharacter.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "sharedNetworkMessages/RequestGalaxyLoopTimes.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	 {"install",    0, "",                  "Install the GameNetwork."},
	 {"remove",     0, "",                  "Remove the GameNetwork."},
	 {"im", 2, "<player> <message>", "Send an instant message to the player on the current cluster"},
	 {"galaxyim", 3, "<galaxy> <player> <message>",  "Send an instant message to the player in the specified galaxy"},
	 {"testpm", 1, "<player>", "Test persistent message"},
	 {"galaxyClock", 0, "", "Get the last and current loop barrier times for the galaxy."},
	 {"task", 0, "<comand>", "Send a command to the task manager."},
#if PRODUCTION == 0
	 {"dumpClusterInfo", 0, "", "Display information for all clusters received from LoginServer."},
	 {"setOverrideHostForCluster", 2, "<cluster id> <hostname>", "Sets the host to be used to connect to the specified cluster."},
	 {"setOverridePortForCluster", 2, "<cluster id> <port>", "Sets the port to be used to connect to the specified cluster."},
#endif
	 {"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

SwgCuiCommandParserNet::SwgCuiCommandParserNet  ():
CommandParser ("net", 0, "...", "Network commands", 0)
{
	createDelegateCommands (cmds);
	IGNORE_RETURN (addSubCommand (new DisconnectParser ()));  //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new ConnectParser ()));     //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new AddParser ()));         //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new SetParser ()));         //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new ShowParser ()));        //lint !e1524 // new in ctor w no explicit dtor
}
//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (isAbbrev (argv[0], "install"))
	{
		result += Unicode::narrowToWide ("Installing GameNetwork.");
		return true;
	}
	else if (isAbbrev (argv[0], "remove"))
	{
		result += Unicode::narrowToWide ("Removing GameNetwork.");
		return true;
	}
	else if (isAbbrev (argv[0], "im"))
	{
		if (Game::isPlayerSquelched())
		{
			result += Unicode::narrowToWide("Not sent because you are currently squelched");
			return true;
		}

		std::string characterName = Unicode::wideToNarrow(argv[1]);
		size_t i;
		Unicode::String msg;
		for(i = 2; i < argv.size(); ++i)
		{
			msg += argv[i];
			if(i < argv.size())
				msg += Unicode::narrowToWide(" ");
		}
		Unicode::String oob;
		ChatInstantMessageToCharacter m(0, ChatAvatarId (characterName), msg, oob);
		GameNetwork::send(m, true);
		result += Unicode::narrowToWide("Sent");
		return true;
	}
	else if (isAbbrev (argv[0], "galaxyim"))
	{
		if (Game::isPlayerSquelched())
		{
			result += Unicode::narrowToWide("Not sent because you are currently squelched");
			return true;
		}

		std::string characterName = Unicode::wideToNarrow(argv[2]);
		std::string galaxyName = Unicode::wideToNarrow(argv[1]);
		size_t i;
		Unicode::String msg;
		for(i = 3; i < argv.size(); ++i)
		{
			msg += argv[i];
			if(i < argv.size())
				msg += Unicode::narrowToWide(" ");
		}
		Unicode::String oob;
		ChatInstantMessageToCharacter m(0, ChatAvatarId (galaxyName, characterName), msg, oob);
		GameNetwork::send(m, true);
		result += Unicode::narrowToWide("Sent");
		return true;
	}
	else if (isAbbrev (argv[0], "galaxyClock"))
	{
		RequestGalaxyLoopTimes request;
		GameNetwork::send(request, true);
		return true;
	}
	else if (isAbbrev (argv[0], "testpm"))
	{
		if (Game::isPlayerSquelched())
		{
			result += Unicode::narrowToWide("Not sent because you are currently squelched");
			return true;
		}

		std::string characterName = Unicode::wideToNarrow(argv[1]);
		Unicode::String subject(Unicode::narrowToWide("Test Subject"));
		Unicode::String body(Unicode::narrowToWide("This is a test message body\nWith a line break!\n"));
		Unicode::String oob;
		ChatPersistentMessageToServer m(0, ChatAvatarId (characterName), subject, body, oob);
		GameNetwork::send(m, true);
		return true;
	}
	else if (isAbbrev (argv[0], "task"))
	{
		Unicode::NarrowString s;
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
		ConGenericMessage c(s);
		GameNetwork::sendTaskCommand(c);
	}
#if PRODUCTION == 0
	else if (isAbbrev (argv[0], "dumpClusterInfo"))
	{
		CuiLoginManager::ClusterInfoVector civ;
		CuiLoginManager::getClusterInfo (civ);

		for (CuiLoginManager::ClusterInfoVector::const_iterator it = civ.begin (); it != civ.end (); ++it)
		{
			const CuiLoginManagerClusterInfo & clusterInfo = *it;
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("id=(%lu), name=(%s), branch=(%s), version=(%lu), host=(%s), port=(%hu), pingPort=(%hu)\n", clusterInfo.id, clusterInfo.name.c_str(), clusterInfo.branch.c_str(), clusterInfo.version, clusterInfo.getHost().c_str(), clusterInfo.getPort(), clusterInfo.getPingPort()));
		}

		return true;
	}
	else if (isAbbrev (argv[0], "setOverrideHostForCluster"))
	{
		uint32 clusterId = static_cast<uint32>(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
		std::string hostName = Unicode::wideToNarrow(argv[2]);

		const CuiLoginManagerClusterInfo * ci = CuiLoginManager::findClusterInfo(clusterId);
		if (!ci)
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("could not find cluster with id=(%lu)\n", clusterId));
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("setting host for cluster=(%lu, %s) from (%s) to (%s)\n", clusterId, ci->name.c_str(), ci->getHost().c_str(), hostName.c_str()));
			CuiLoginManager::setOverrideHostForCluster(clusterId, hostName);
		}

		return true;
	}
	else if (isAbbrev (argv[0], "setOverridePortForCluster"))
	{
		uint32 clusterId = static_cast<uint32>(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
		uint16 port = static_cast<uint16>(atoi(Unicode::wideToNarrow(argv[2]).c_str()));

		const CuiLoginManagerClusterInfo * ci = CuiLoginManager::findClusterInfo(clusterId);
		if (!ci)
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("could not find cluster with id=(%lu)\n", clusterId));
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("setting port for cluster=(%lu, %s) from (%hu) to (%hu)\n", clusterId, ci->name.c_str(), ci->getPort(), port));
			CuiLoginManager::setOverridePortForCluster(clusterId, port);
		}

		return true;
	}
#endif

	return false;
}

//-----------------------------------------------------------------
// DisconnectParser
//-----------------------------------------------------------------

SwgCuiCommandParserNet::DisconnectParser::DisconnectParser () :
CommandParser ("disconnect", 0, "...", "Disconnect from a server.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("gameserver", 0, "", "Disconnect from gameserver.", this)));  //lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::DisconnectParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);
	
	if (isAbbrev (argv[0], "gameserver"))
	{
//		GameNetwork::getConnectionManager ().disconnectGameServer ();
		result += Unicode::narrowToWide ("Disconnecting from gameserver.");
	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------
// ConnectParser
//-----------------------------------------------------------------

SwgCuiCommandParserNet::ConnectParser::ConnectParser () :
CommandParser ("connect", 1, "...", "Connect to a server.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("gameserver", 0, "[<host> <port>]", "Connect to a gameserver.", this)));//lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::ConnectParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);
	
	if (isAbbrev (argv[0], "gameserver"))
	{
		result += Unicode::narrowToWide ("No implementation.");
	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}
	
	return true;
}

//-----------------------------------------------------------------
// SetParser
//-----------------------------------------------------------------

SwgCuiCommandParserNet::SetParser::SetParser () :
CommandParser ("set", 0, "...", "Set a property.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("character", 1, "<name>", "Set your selected character.", this)));//lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::SetParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (isAbbrev (argv[0], "character"))
	{ 
		result += Unicode::narrowToWide ("Not implemented.");
	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}



//-----------------------------------------------------------------
// ShowParser
//-----------------------------------------------------------------

SwgCuiCommandParserNet::ShowParser::ShowParser () :
CommandParser ("show", 0, "...", "Show information.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("characters",  0, "", "Show list of characters.",  this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("clusters",    0, "", "Show list of clusters.",    this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("gameservers", 0, "", "Show list of gameservers.", this)));//lint !e1524 // new in ctor w no explicit dtor

}

//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::ShowParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (isAbbrev (argv[0], "characters"))
	{ 
//		const ConnectionManager::CharacterMap_t & characterMap = GameNetwork::getConnectionManager ().getCharacterMap ();
		result += Unicode::narrowToWide (
			"           Character             Location           Appearance:\n"
			"---------------------------------------------------------------\n");


#if 0 // reworking ConnectionManager

		for (ConnectionManager::CharacterMap_t::const_iterator iter = characterMap.begin (); iter != characterMap.end (); ++iter)
		{
		
			const ConnectionManager::CharacterData & data = iter->second;

			char buf[128];

			sprintf (buf, "%20s %20s %20s\n", data.name.c_str (), data.location.c_str (), data.objectTemplate.c_str ());
			result += Unicode::narrowToWide (buf);
		}
#endif // reworking ConnectionManager
	}
		
	else if (isAbbrev (argv[0], "clusters"))
	{ 
//		const ConnectionManager::ServerMap_t & serverMap = GameNetwork::getConnectionManager ().getClusterServerMap ();
		result += Unicode::narrowToWide ("Clusters:\n");
#if 0 // reworking ConnectionManager
		for (ConnectionManager::ServerMap_t::const_iterator iter = serverMap.begin (); iter != serverMap.end (); ++iter)
		{
			result.append ((*iter).first.begin (), (*iter).first.end ());
			result.append (1, '\n');
		}
#endif // reworking ConnectionManager
	}
	else if (isAbbrev (argv[0], "gameservers"))
	{ 
//		const ConnectionManager::ServerMap_t & serverMap = GameNetwork::getConnectionManager ().getGameServerMap ();
		result += Unicode::narrowToWide ("GameServers:\n");
#if 0 // reworking ConnectionManager
		for (ConnectionManager::ServerMap_t::const_iterator iter = serverMap.begin (); iter != serverMap.end (); ++iter)
		{
			result.append ((*iter).first.begin (), (*iter).first.end ());
			result.append (1, '\n');
		}
#endif // reworking ConnectionManager
	}

	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------
// AddParser
//-----------------------------------------------------------------

SwgCuiCommandParserNet::AddParser::AddParser () :
CommandParser ("add", 0, "...", "Add a character/server.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("character", 2, "<name> <appearance>", "Add a character to the list.", this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("cluster", 3, "<name> <host> <port>", "Add a cluster to the list.", this)));//lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserNet::AddParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (isAbbrev (argv[0], "character"))
	{

		// TODO: these should use strings
//		GameNetwork::getConnectionManager ().addCharacterToList ( argv [1], "", Unicode::wideToNarrow (argv [2]).c_str ());
		result += Unicode::narrowToWide ("Character added.");
	}
	if (isAbbrev (argv[0], "cluster"))
	{
		// TODO: these should use strings
//		GameNetwork::getConnectionManager ().addClusterToList (Unicode::wideToNarrow (argv[1]).c_str (), Network::Address (Unicode::wideToNarrow (argv[2]).c_str (), static_cast<unsigned short>(atoi (Unicode::wideToNarrow (argv[3]).c_str ()))));
		result += Unicode::narrowToWide ("Cluster added.");
	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------

// ======================================================================
