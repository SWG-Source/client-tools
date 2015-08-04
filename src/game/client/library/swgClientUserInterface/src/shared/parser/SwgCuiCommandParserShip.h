// ======================================================================
//
// SwgCuiCommandParserShip.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserShip_H
#define INCLUDED_SwgCuiCommandParserShip_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

class SwgCuiCommandParserShip : public CommandParser
{
public:

	SwgCuiCommandParserShip();

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserShip(const SwgCuiCommandParserShip &rhs);
	SwgCuiCommandParserShip & operator=(const SwgCuiCommandParserShip &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserShip_H
