// ======================================================================
//
// SwgCuiCommandParserNebula.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserNebula_H
#define INCLUDED_SwgCuiCommandParserNebula_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

class SwgCuiCommandParserNebula : public CommandParser
{
public:

	SwgCuiCommandParserNebula();

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserNebula(const SwgCuiCommandParserNebula &rhs);
	SwgCuiCommandParserNebula & operator=(const SwgCuiCommandParserNebula &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserNebula_H
