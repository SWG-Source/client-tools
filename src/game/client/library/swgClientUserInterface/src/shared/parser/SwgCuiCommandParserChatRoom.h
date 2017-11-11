//======================================================================
//
// SwgCuiCommandParserChatRoom.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCommandParserChatRoom_H
#define INCLUDED_SwgCuiCommandParserChatRoom_H

#include "sharedCommandParser/CommandParser.h"

class CommandParserHistory;

// ======================================================================

class SwgCuiCommandParserChatRoom : 
public CommandParser
{
public:


	explicit                     SwgCuiCommandParserChatRoom ();
	bool                         performParsing             (const NetworkId & userId, const StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);

	static void                  setCurrentChatRoom (uint32 id);

private:

	                             SwgCuiCommandParserChatRoom (const SwgCuiCommandParserChatRoom & rhs);
	SwgCuiCommandParserChatRoom & operator=                  (const SwgCuiCommandParserChatRoom & rhs);

private:

};


//======================================================================

#endif
