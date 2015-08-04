// ======================================================================
//
// SwgCuiCommandParserParticle.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserParticle_H
#define INCLUDED_SwgCuiCommandParserParticle_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserParticle : public CommandParser
{
public:

	SwgCuiCommandParserParticle();

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserParticle(const SwgCuiCommandParserParticle &rhs);
	SwgCuiCommandParserParticle & operator=(const SwgCuiCommandParserParticle &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserParticle_H
