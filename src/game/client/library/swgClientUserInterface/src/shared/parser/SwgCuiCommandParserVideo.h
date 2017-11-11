// ======================================================================
//
// SwgCuiCommandParserVideo.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserVideo_H
#define INCLUDED_SwgCuiCommandParserVideo_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserVideo : public CommandParser
{
public:

	SwgCuiCommandParserVideo();

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserVideo(const SwgCuiCommandParserVideo &rhs);
	SwgCuiCommandParserVideo & operator=(const SwgCuiCommandParserVideo &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserVideo_H
