// ======================================================================
//
// CuiCommandTableParser.h
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiCommandTableParser_H
#define INCLUDED_CuiCommandTableParser_H

// ======================================================================

#include "sharedCommandParser/CommandParser.h"

/**
* The CuiCommandTableParser interprets an input string, determining target if appropriate, and
* tells the ClientCommandQueue to execute it if needed.
* 
*/

namespace MessageDispatch
{
	class Callback;
}

class Command;

class CreatureObject;
class PlayerObject;

//----------------------------------------------------------------------

class CuiCommandTableParser : public CommandParser
{
public:

	CuiCommandTableParser ();
	virtual ~CuiCommandTableParser();

	virtual bool performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node);

	static void performCommand(Command const &command, Unicode::String const &originalCommand, String_t &result);

	void onPlayerSetup (const CreatureObject & payload);
	void onJediStateChanged (const PlayerObject & creature);
	void resetCommands ();

private:
	CuiCommandTableParser(CuiCommandTableParser const &rhs);
	CuiCommandTableParser &operator=(CuiCommandTableParser const &rhs);

	MessageDispatch::Callback * m_callback;
};

// ======================================================================

#endif
