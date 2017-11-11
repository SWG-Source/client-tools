// ======================================================================
//
// SwgCuiCommandParserLocale.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserLocale_H
#define INCLUDED_SwgCuiCommandParserLocale_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserLocale : public CommandParser
{
public:
	                           SwgCuiCommandParserLocale ();
	virtual bool               performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

private:
	                           SwgCuiCommandParserLocale (const SwgCuiCommandParserLocale & rhs);
	SwgCuiCommandParserLocale &  operator= (const SwgCuiCommandParserLocale & rhs);

	//-----------------------------------------------------------------

	class StringsParser : public CommandParser
	{
	public:
		StringsParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
		
	private:
		StringsParser (const StringsParser & rhs);
		StringsParser & operator= (const StringsParser & rhs);
	};
};

// ======================================================================

#endif
