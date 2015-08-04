// ======================================================================
//
// SwgCuiCommandParserUI.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserUI_H
#define INCLUDED_SwgCuiCommandParserUI_H

#include "sharedCommandParser/CommandParser.h"
#include "UITypes.h"

// ======================================================================
class SwgCuiCommandParserUI : public CommandParser
{
public:
	                       SwgCuiCommandParserUI ();
	virtual bool           performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

private:
	                       SwgCuiCommandParserUI (const SwgCuiCommandParserUI & rhs);
	SwgCuiCommandParserUI &  operator= (const SwgCuiCommandParserUI & rhs);

	//-----------------------------------------------------------------

	class MessageBoxParser;
	class ShowParser;
	class ChatBubbleParser;
};

// ======================================================================

#endif
