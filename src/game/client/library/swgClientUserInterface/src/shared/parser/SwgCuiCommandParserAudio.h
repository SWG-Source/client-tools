// ======================================================================
//
// SwgCuiCommandParserAudio.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserAudio_H
#define INCLUDED_SwgCuiCommandParserAudio_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserAudio : public CommandParser
{
public:

	SwgCuiCommandParserAudio();
	~SwgCuiCommandParserAudio();

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserAudio(const SwgCuiCommandParserAudio &rhs);
	SwgCuiCommandParserAudio & operator=(const SwgCuiCommandParserAudio &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserAudio_H
