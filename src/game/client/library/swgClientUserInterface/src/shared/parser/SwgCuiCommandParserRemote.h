// ======================================================================
//
// SwgCuiCommandParserRemote.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserRemote_H
#define INCLUDED_SwgCuiCommandParserRemote_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserRemote : public CommandParser
{
public:
	                       SwgCuiCommandParserRemote ();
	virtual bool           performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

private:
	                       SwgCuiCommandParserRemote (const SwgCuiCommandParserRemote & rhs);
	SwgCuiCommandParserRemote &  operator= (const SwgCuiCommandParserRemote & rhs);
};

//-----------------------------------------------------------------

#endif