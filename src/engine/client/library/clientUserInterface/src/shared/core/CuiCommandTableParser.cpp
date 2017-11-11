// ======================================================================
//
// CuiCommandTableParser.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCommandTableParser.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/NetworkScene.h"
#include "clientGame/PlayerObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Object.h"
#include <map>

// ======================================================================

namespace
{

	const Unicode::unicode_char_t      s_geChar        = '>';
	const Unicode::unicode_char_t      s_targetChar    = s_geChar;

	/**
	* The command parser node that represents a command table command.  It is a normal
	* command parser with an added command piece of data.
	*/

	class CommandTableParserNode: public CommandParser
	{
	public:
		CommandTableParserNode(Command const &command, const String_t &cmd, size_t minArgs, const String_t &args, const String_t &help, CommandParser *delegate) :
			CommandParser(cmd, minArgs, args, help, delegate),
			m_command(command)
		{
		}

		Command const &getCommand() const
		{
			return m_command;
		}

	private:
		CommandTableParserNode();
		CommandTableParserNode(CommandTableParserNode const &rhs);
		CommandTableParserNode &operator=(CommandTableParserNode const &rhs);

	private:
		Command const &m_command;
	};

	bool s_showHiddenCommands = false;

	bool s_showHiddenCommandsRegistered = false;

	
}

//-----------------------------------------------------------------

CuiCommandTableParser::CuiCommandTableParser () :
CommandParser ("commands", 0, "...", "all commands", 0),
m_callback (new MessageDispatch::Callback)
{
	resetCommands ();

	m_callback->connect (*this, &CuiCommandTableParser::onPlayerSetup,               static_cast<CreatureObject::Messages::PlayerSetup *>  (0));
	m_callback->connect (*this, &CuiCommandTableParser::onJediStateChanged,          static_cast<PlayerObject::Messages::JediStateChanged*>(0));
}

//----------------------------------------------------------------------

CuiCommandTableParser::~CuiCommandTableParser ()
{
	m_callback->disconnect (*this, &CuiCommandTableParser::onPlayerSetup,                static_cast<CreatureObject::Messages::PlayerSetup *>   (0));
	m_callback->disconnect (*this, &CuiCommandTableParser::onJediStateChanged,           static_cast<PlayerObject::Messages::JediStateChanged *>(0));
	delete m_callback;
}

//----------------------------------------------------------------------

bool CuiCommandTableParser::performParsing(const NetworkId &, const StringVector_t &, const String_t &originalCommand, String_t &result, const CommandParser *node)
{
	CommandTableParserNode const *commandNode = NON_NULL(dynamic_cast<const CommandTableParserNode *>(node));
	performCommand(commandNode->getCommand(), originalCommand, result);	
	return true;
}

//----------------------------------------------------------------------

void CuiCommandTableParser::performCommand(Command const &command, Unicode::String const &originalCommand, String_t &)
{
	Unicode::String token;
	size_t curPos = 0, endPos = 0;
	if (Unicode::getFirstToken(originalCommand, curPos, endPos, token))
	{
		curPos = Unicode::skipWhitespace(originalCommand, endPos);
		if (curPos != Unicode::String::npos)
			ClientCommandQueue::enqueueCommand(command, NetworkId::cms_invalid, Unicode::getTrim(originalCommand.substr(curPos)));
		else
			ClientCommandQueue::enqueueCommand(command, NetworkId::cms_invalid, Unicode::String());
	}
}

//----------------------------------------------------------------------

void CuiCommandTableParser::onPlayerSetup (const CreatureObject::Messages::PlayerSetup::Payload & payload)
{
	if (static_cast<const Object *>(&payload) == Game::getPlayer ())
	{
		resetCommands ();
	}
}

//----------------------------------------------------------------------

void CuiCommandTableParser::onJediStateChanged (const PlayerObject::Messages::JediStateChanged::Payload & payload)
{
	if (static_cast<const Object *>(&payload) == Game::getConstPlayerObject ())
	{
		resetCommands ();
	}
}

//----------------------------------------------------------------------

void CuiCommandTableParser::resetCommands ()
{
	deleteSubCommands ();

	Unicode::String dummy;
	
	const PlayerObject * const playerObject = Game::getConstPlayerObject ();

	bool isCsr = ConfigClientGame::getCSR ();

#if !PRODUCTION
	isCsr = true;
#endif

	bool isJedi = false;	
	if (playerObject && playerObject->getJediState())	//Player has a non-zero jedi state
			isJedi = true;

	if (!s_showHiddenCommandsRegistered)
	{
		DebugFlags::registerFlag(s_showHiddenCommands,  "ClientUserInterface", "showHiddenCommands");
		s_showHiddenCommandsRegistered = true;
	}

	static const uint32 s_displayGroupJedi   = Crc::normalizeAndCalculate ("jedi");
	static const uint32 s_displayGroupHidden = Crc::normalizeAndCalculate ("hidden");
	static const uint32 s_displayGroupCsr    = Crc::normalizeAndCalculate ("csr");

	CommandTable::CommandMap const &commandMap = CommandTable::getCommandMap();
	for (CommandTable::CommandMap::const_iterator it = commandMap.begin(); it != commandMap.end(); ++it)
	{
		const Command &command = (*it).second;
		const uint32 displayGroup = command.m_displayGroup;

		if (!s_showHiddenCommands && displayGroup == s_displayGroupHidden)
			continue;

		if (!isJedi && displayGroup == s_displayGroupJedi)
			continue;

		if (!isCsr && displayGroup == s_displayGroupCsr)
			continue;

		addSubCommand(new CommandTableParserNode (command, Unicode::narrowToWide(command.m_commandName), 0, dummy, dummy, this));
	}
}

// ======================================================================
